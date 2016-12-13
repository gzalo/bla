/*
	BLA: Biblioteca liviana para reconocimiento de palabras aisladas
	
	Detalles técnicos:
	
		Adquisición y almacenamiento de datos de señal de audio:
			Para usar los modelos provistos, se requieren señales
			muestreadas a 16 KSPS (KHz)
			
			El almacenamiento se hace en buffer circular, donde los datos son
			enteros de 16 bits con signo
	
		Detección comienzo/fin de palabras:
			Basado en energía de una ventana deslizante de 100ms
			Si la energía es mayor a un umbral fijo, se supone que
			alguien está hablando
		
		Preprocesamiento:
			La señal se separa en ventanas de largo 25 ms, tomadas cada 10 ms
			A cada bloque se le remueve el valor medio, se le realiza una preénfasis (filtro pasa altos)
			posteriormente se lo ventanea usando una ventana de Hamming
			
			Se calcula la FFT de cada bloque
			Se calcula el módulo al cuadrado de cada elemento
			Se realiza un banco de filtros de forma triangular, separados en la escala de frecuencia Mel
			Con la salidas de cada filtro, se realiza una transformada coseno discreta inversa (tipo 3)
			
			Todos los pasos anteriores obtienen los MFCC (Mel Frequency Cepstral Coefficients) de cada bloque
			
		Reconocimiento:
			Por cada palabra posible, se realiza el algoritmo de Viterbi para hallar una probabilidad del modelo
			Se toman las siguientes suposiciones:
				Observaciones se comportan como mezclas de Gausianas multivariables de varianza diagonal
				Las probabilidades de transición se obtienen de los modelos de cada fonema, y de la pronunciación
				de cada palabra (mappeo palabra->fonema)
				Para evitar underflows, debido a que las probabilidades suelen ser números muy pequeños, se realizan
				las cuentas del algoritmo usando los logaritmos de las probabilidades. 
			
			Se obtiene un puntaje (probabilidad de observación de cada salida) de cada palabra en un vector
			Se ordena el vector y se copian los N resultados más probables a un vector provisto por el usuario.

	Octubre-Noviembre 2016 - Gonzalo Ávila Alterach - http://gzalo.com
*/

#include <bla.h>
#include <bla_config.h>
#include <bla_viterbi.h>
#include <bla_mfcc.h>
#include <bla_pronunciacion.h>
#include <bla_probabilidad.h>
#include <bla_modelo.h>
#include <string.h>
#include <math.h>
#include <stdint.h>
#include <stdbool.h>
#include <stdlib.h>
#include <arm_math.h>

/*Buffer de audio, definido en RAM2 para que entre en un solo banco*/
__BSS(RAM2) static volatile int16_t blaBufferAudio[BLA_LARGO_BUFFER_AUDIO];
static volatile uint32_t blaBufferAudioIdx = 0;

/*Duración de la palabra que se está pronunciando en este momento*/
static volatile int blaLargoAudio = 0;

/*Acumulador de energía de los últimas muestras 
  Va aumentando y disminuyendo de forma proporcional al cuadrado de las muestras
  presentes en una ventana definida en bla_config.h*/
static volatile uint64_t blaAcumuladorEnergia = 0;

/*Estado de la detección 
	ESPERANDO_COMIENZO:
		Se está esperando que comienze una palabra: que el nivel de energía supere el umbral.
	ESPERANDO_FINAL:
		Se está esperando que termine una palabra: que el nivel de energía sea inferior al umbral.
	ESPERANDO_TIEMPO_EXTRA:
		Se está esperando un tiempo extra, para asegurarse que se incluye un poco más de lo muestreado
		luego de detectado el final (para que no se corte muy abruptamente).
*/
enum {BLA_ESPERANDO_COMIENZO, BLA_ESPERANDO_FINAL, BLA_ESPERANDO_TIEMPO_EXTRA} blaEstadoDeteccion = BLA_ESPERANDO_COMIENZO;
static uint16_t blaContadorTiempoExtra = 0;

static int blaPalabrasAReconocer[BLA_MAX_PALABRAS][BLA_MAX_CANT_FONEMAS];
static int blaPalabrasCantFonemas[BLA_MAX_PALABRAS];
static int blaCantidadPalabras;

static volatile int blaReconociendo = 0;

uint64_t blaLeerAcumuladorEnergia(void){
	return blaAcumuladorEnergia;
}

/*Código de error, elige qué mensaje se devuelve al llamar a blaLeerError*/
static uint8_t blaIdError = BLA_OK;

int blaInicializarBiblioteca(const char *palabrasAReconocer[], int cantidadPalabras){
	int i;
	
	if(cantidadPalabras > BLA_MAX_PALABRAS){
		blaIdError = BLA_LIMITE_PALABRAS; /*Demasiadas palabras*/
		return 1;
	}
	
	for(i=0;i<cantidadPalabras;i++){
		if(blaPronunciacion(palabrasAReconocer[i], blaPalabrasAReconocer[i], BLA_MAX_CANT_FONEMAS, &(blaPalabrasCantFonemas[i])) != 0){
			blaIdError = BLA_PALABRAS_LARGAS; /*Palabra[s] muy larga[s]*/
			return 1;
		}
		blaImprimirPronunciacion(blaPalabrasAReconocer[i], blaPalabrasCantFonemas[i]);
	}
	blaCantidadPalabras = cantidadPalabras;
	

	
	blaProbabilidadInicializar();
	
	return 0;
}

/*Rota circularmente los elementos del buffer 
  No usa memoria extra, se realiza en el mismo array en O(N) intercambios
  Inspirado en http://codegolf.stackexchange.com/questions/17415/rotate-an-integer-array-with-an-on-algorithm */
static int blaRotarBuffer(int rotacion){
	size_t base=0, actual=0;
	int i;
	if(rotacion < 0){
		rotacion += BLA_LARGO_BUFFER_AUDIO;
	}
	if(rotacion > BLA_LARGO_BUFFER_AUDIO)
		rotacion %= BLA_LARGO_BUFFER_AUDIO;
	for(i=0;i<BLA_LARGO_BUFFER_AUDIO;i++){
		actual = (actual+rotacion)%BLA_LARGO_BUFFER_AUDIO;
		if(actual == base){
			actual++;
			base++;
		}
		int16_t temp = blaBufferAudio[actual];
		blaBufferAudio[actual] = blaBufferAudio[base];
		blaBufferAudio[base] = temp;
	}
	return 0;
}

int blaDeteccionPeriodica(void){
	/*Manejo la máquina de estados de la detección de palabras*/
	switch(blaEstadoDeteccion){
		case BLA_ESPERANDO_COMIENZO:
			if(blaAcumuladorEnergia > BLA_UMBRAL_ENERGIA_INICIO){
				blaEstadoDeteccion = BLA_ESPERANDO_FINAL;
				blaLargoAudio = 0; /*Reseteamos el contador "largo de la palabra actual"*/
				return BLA_COMIENZO_PALABRA;
			}
			break;
		case BLA_ESPERANDO_FINAL:
			if(blaAcumuladorEnergia < BLA_UMBRAL_ENERGIA_FIN){
				blaEstadoDeteccion = BLA_ESPERANDO_TIEMPO_EXTRA;
				blaContadorTiempoExtra = blaLargoAudio;
				return BLA_OK;
			}
			break;
		case BLA_ESPERANDO_TIEMPO_EXTRA:
			/*
				Esperamos que pasen alrededor de 180 ms más (2880 muestras)
				Sin la segunda condición, puede quedarse trabado esperando el final
			*/
			
			/* Volvió a subir la energía, seguramente sea el final
				de una palabra luego de un silencio (por ejemplo, en la palabra siete), 
				Volvemos a esperar que baje la energía */
			if(blaAcumuladorEnergia > BLA_UMBRAL_ENERGIA_FIN){
				blaEstadoDeteccion = BLA_ESPERANDO_COMIENZO; /*TODO: Arreglar este bug*/
				return BLA_FIN_PALABRA;
			}
			
			if(blaLargoAudio >= blaContadorTiempoExtra+BLA_ENERGIA_TIEMPO_EXTRA || 
				blaLargoAudio >= BLA_LARGO_BUFFER_AUDIO){
					
				blaEstadoDeteccion = BLA_ESPERANDO_COMIENZO;
				return BLA_FIN_PALABRA;
			}

			break;
	}
	return BLA_OK;
}

/*Coeficientes MFCC de los 100 bloques analizados
  0 hasta BLA_NUMCEPS (0 hasta 12): Coeficientes MFCC + Coef 0
  BLA_NUMCEPS+1 hasta 2*BLA_NUMCEPS+1 (13 hasta 25): Coeficientes delta
  2*BLA_NUMCEPS+2 hasta 3*BLA_NUMCEPS+2 (26 hasta 38): Coeficientes aceleración (o delta-delta)*/

float blaMFCCBloques[BLA_CANTIDAD_BLOQUES][BLA_NUMCEPS*3+3];

int blaCalcularDeltas(int offsetEntrada, int offsetSalida){
	int i,j;

	for(i=0;i<BLA_CANTIDAD_BLOQUES;i++){
		/*Para más detalles, revisar sección 5.9 del HTKBook
		  DELTAWINDOW = 2 por default, se repiten los bordes si "se va del rango" 
		
		  No lo dice en ningún lado el HTKBook, pero la forma de 
		  diferenciar numéricamente usada se llama Low-Noise Lanczos:
		  http://www.holoborodko.com/pavel/numerical-methods/numerical-derivative/lanczos-low-noise-differentiators/ */
		
		float coefMenos1, coefMenos2, coefMas1, coefMas2;
		
		/*También calculamos la delta al ultimo elemento (coef 0)*/
		for(j=0;j<=BLA_NUMCEPS;j++){
			
			/*Repetimos los elementos que se van del vector*/
			if(i==0){
				coefMenos1 = blaMFCCBloques[i  ][j+offsetEntrada];
			}else{
				coefMenos1 = blaMFCCBloques[i-1][j+offsetEntrada];
			}
			if(i==BLA_CANTIDAD_BLOQUES-1){
				coefMas1   = blaMFCCBloques[i  ][j+offsetEntrada];
			}else{
				coefMas1   = blaMFCCBloques[i+1][j+offsetEntrada];
			}
			
			if(i==0){
				coefMenos2 = blaMFCCBloques[i  ][j+offsetEntrada];
			}else if(i==1){
				coefMenos2 = blaMFCCBloques[i-1][j+offsetEntrada];
			}else{
				coefMenos2 = blaMFCCBloques[i-2][j+offsetEntrada];
			}
			
			if(i==BLA_CANTIDAD_BLOQUES-1){
				coefMas2   = blaMFCCBloques[i  ][j+offsetEntrada];
			}else if(i==BLA_CANTIDAD_BLOQUES-2){
				coefMas2   = blaMFCCBloques[i+1][j+offsetEntrada];
			}else{
				coefMas2   = blaMFCCBloques[i+2][j+offsetEntrada];
			}

			blaMFCCBloques[i][offsetSalida+j] = (coefMas1-coefMenos1+2.0f*(coefMas2-coefMenos2))/10.0f;
		}
	}
	return 0;
}

/*Compara dos estructuras blaListado (que poseen idPalabra y logProb) y los ordena según logProb (decreciente) */
static int blaCompararListado(const void *_a, const void *_b){
	const blaListado *a = (blaListado*) _a;
	const blaListado *b = (blaListado*) _b;
	/*Es una comparación entre floats, por lo que no comparo si son iguales*/
	if(a->probabilidad > b->probabilidad) return -1; /*A tiene que ir antes que B*/
	if(a->probabilidad < b->probabilidad) return 1; /*B tiene que ir antes de A*/
	return 0;
}
#ifdef BLA_DEBUG
	#include <stdio.h>
	
#endif

//#define BLA_ESTADISTICA_TIEMPOS

#ifdef BLA_ESTADISTICA_TIEMPOS
	#include <stdio.h>
	extern uint32_t tickCounter;
	
#endif

int blaReconocer(blaListado *listaProbabilidades, int cantidadLista){
	int ret, i;
	#ifdef BLA_ESTADISTICA_TIEMPOS
		uint32_t t1 = tickCounter;
	#endif
	
	blaReconociendo = 1;
	
	/*Roto el buffer para que el principio de la palabra quede al principio,
	  además agrego N segundos más al principio, porque la detección de comienzo
	  de palabra suele tardar un poco en activarse.
	*/
	blaRotarBuffer(BLA_LARGO_BUFFER_AUDIO-blaBufferAudioIdx+blaLargoAudio+BLA_OFFSET_MUESTRAS);
	
	int largo = blaLargoAudio+BLA_OFFSET_MUESTRAS;
	if(largo > BLA_LARGO_BUFFER_AUDIO) /*Puede darse que se pase del largo máximo*/
		largo = BLA_LARGO_BUFFER_AUDIO;
	
	ret = blaReconocerBuffer(listaProbabilidades, cantidadLista, (const int16_t*) blaBufferAudio, largo);
	
	/*Limpiamos el buffer de audio*/
	for(i=0;i<BLA_LARGO_BUFFER_AUDIO;i++)
		blaBufferAudio[i] = 0;
	
	/*Reseteamos el acumulador de energía*/
	blaAcumuladorEnergia = 0;
	blaReconociendo = 0;
	
	#ifdef BLA_ESTADISTICA_TIEMPOS
		uint32_t dt = tickCounter - t1;
	
		printf("Capturadas %d muestras (%d ms)\r\n", blaLargoAudio+BLA_OFFSET_MUESTRAS, (blaLargoAudio+BLA_OFFSET_MUESTRAS)/16);
		printf("Reconocimiento en %u ms (%f ms/ms)\r\n", (int)dt, (double)(dt/((blaLargoAudio+BLA_OFFSET_MUESTRAS)/16.0)));
	#endif
	
	blaLargoAudio = 0;
	
	return ret;
}
int blaReconocerBuffer(blaListado *listaProbabilidades, int cantidadLista, const int16_t *buffer, int largoBuffer){
	
	#ifdef BLA_DEBUG
		FILE *fp = fopen("reconocido.raw","wb");
		fwrite(buffer, sizeof(int16_t), largoBuffer, fp);
		fclose(fp);
		
		uint8_t *bff_8 = (uint8_t*) buffer;
		for(int j=0;j<largoBuffer*2;j++){
			Board_UARTPutChar(bff_8[j]);		
		}
	#endif
			
	
	int cantBloquesLocal = (((largoBuffer-BLA_LARGOVENTANA)/BLA_LARGOBLOQUE)+1);
	int i;
	
	/*Solamente obtenemos los coeficientes de la parte que se va a usar para reconocer*/
	for(i=0;i<cantBloquesLocal;i++){
		blaObtenerCoeficientes(buffer+i*BLA_LARGOBLOQUE, &(blaMFCCBloques[i][0]));
	}

	/*Calcular deltas */
	blaCalcularDeltas(0, BLA_NUMCEPS+1);
	/*Calcular aceleraciones */
	blaCalcularDeltas(BLA_NUMCEPS+1, 2*BLA_NUMCEPS+2);
	
	#ifdef BLA_NORMALIZACION_MEDIA_CEPSTRAL
	/*Resta el valor medio componente a componente
	  - se realiza usando los valores de todos los bloques
	  ayuda a mejorar el reconocimiento si hay un canal convolucional 
	  solamente funciona bien si la rta. al impulso es relativamente corta
	  por ej, no puede "compensar" ecos de varios segundos de duración */
	blaCMN();	
	#endif
	
	/*Calculamos Viterbi con cada una de las palabras*/
	blaListado logProb[blaCantidadPalabras];
	
	for(i=0;i<blaCantidadPalabras;i++){
		logProb[i].idPalabra = i;
		logProb[i].probabilidad = blaViterbi(blaPalabrasAReconocer[i], blaPalabrasCantFonemas[i], cantBloquesLocal);		
	}
	/*Ordenamos el vector con las verosimilitudes de cada modelo (manteniendo los índices) */
	qsort(logProb, blaCantidadPalabras, sizeof(blaListado), blaCompararListado);

	/*Devolvemos al usuario las cantidadLista palabras más probables
	  extrayéndolas del vector ordenado */
	if(cantidadLista > blaCantidadPalabras)
		cantidadLista = blaCantidadPalabras;
	for(i=0;i<cantidadLista;i++){
		listaProbabilidades[i].idPalabra = logProb[i].idPalabra;
		listaProbabilidades[i].probabilidad = logProb[i].probabilidad;
	}
	
	return 0;
}

int blaAlmacenarValorMicrofono(int16_t valor){
	/*Si estoy haciendo el reconocimiento, no cargo más muestras*/
	if(blaReconociendo)
		return 0;
	
	blaBufferAudio[blaBufferAudioIdx] = valor;
	
	/*Incremento el índice y le doy la vuelta circularmente*/
	blaBufferAudioIdx++;
	blaBufferAudioIdx %= BLA_LARGO_BUFFER_AUDIO;
	
	/*Agrego el valor de energía generado por esta muestra
	  el acumulador es de 64 bits por lo que no hace overflow
	  log2 (512*512*16000) es aproximadamente 32 bits*/
	blaAcumuladorEnergia += valor*valor;
	
	/*Le resto al acumulador de energía el valor correspondiente 
	  a la muestra de hace 100 ms, de esta forma obtengo una 
	  ventana con la energía de las últimas N muestras*/
	int idxAnterior = blaBufferAudioIdx - BLA_ACUMULADOR_ENERGIA_VENTANA;
	if(idxAnterior < 0){
		/*Si dio la vuelta, vuelvo al rango correcto*/
		idxAnterior += BLA_LARGO_BUFFER_AUDIO;
	}
	
	if(blaEstadoDeteccion != BLA_ESPERANDO_COMIENZO){
		blaLargoAudio++;
		if(blaLargoAudio > BLA_LARGO_BUFFER_AUDIO)
			blaLargoAudio = BLA_LARGO_BUFFER_AUDIO;
	}
	
	int16_t valorAnterior = blaBufferAudio[idxAnterior];
	blaAcumuladorEnergia -= valorAnterior*valorAnterior;
	return 0;
}

static const char *blaMensajesErrores[] = {
	"No hubo errores",
	"",
	"",
	"Palabras muy largas"
	"Demasiadas palabras",
	"Palabra impronunciable"
};

const char *blaLeerError(void){
	return blaMensajesErrores[blaIdError];
}

/*Calcula la media de cada uno de los coeficientes,
  y se la resta a todos los bloques */
void blaCMN(void){
	float medias[BLA_NUMCEPS*3+3];
	
	/*Estimo la media de cada coeficiente*/
	for(int j=0;j<BLA_NUMCEPS*3+3;j++){
		medias[j] = 0.0f;
		for(int i=0;i<BLA_CANTIDAD_BLOQUES;i++)
			medias[j] += blaMFCCBloques[i][j];
		
		medias[j] /= BLA_CANTIDAD_BLOQUES;
	}
	
	/*Resto la media correspondiente */
	for(int i=0;i<BLA_CANTIDAD_BLOQUES;i++){
		for(int j=0;j<BLA_NUMCEPS*3+3;j++)
			blaMFCCBloques[i][j] -= medias[j];
	}

}
