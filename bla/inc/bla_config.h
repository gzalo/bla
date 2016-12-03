/*
	BLA - Gonzalo Ávila Alterach (http://gzalo.com)
	Biblioteca liviana para realizar reconocimiento de habla 
	
	Archivo de configuración
*/

#ifndef BLA_CONFIG_H
#define BLA_CONFIG_H

/*!Parámetros modificables*/

/*Descomentar para agregar varios chequeos extra en valores, y printfs para debuggear,
  solamente útil para Windows/Linux*/
/* #define BLA_DEBUG */

/*Máxima cantidad de fonemas por palabra*/
#define BLA_MAX_CANT_FONEMAS 10

/*Máxima cantidad de palabras a reconocer*/
#define BLA_MAX_PALABRAS (10)

/*Rango del ADC (se supone que va entre -BLA_ADC_RANGO-1 y BLA_ADC_RANGO)
  Es requerido para calcular correctamente el coeficiente 0, ya que de lo contrario
  queda escalado por un valor y puede influir negativamente en el reconocimiento*/
#ifndef BLA_ADC_RANGO
#define BLA_ADC_RANGO (511*8)
#endif

/*Longitud del buffer principal de audio, de 16 bits por muestra (típicamente 1 segundo)*/
#define BLA_LARGO_BUFFER_AUDIO (16000)	

/*Longitud (en muestras) de la ventana que acumula la energía para detectar
  comienzos y finales de palabras (típicamente 0.1 segundos) */
#define BLA_ACUMULADOR_ENERGIA_VENTANA (1600)

/*Valor de energía a partir del cual se supone que se está hablando
  puede ser necesario modificarlo en función del tipo de micrófono, la distancia al hablante
  y el ruido de fondo (del conversor analógico digital y del entorno)*/
#define BLA_UMBRAL_ENERGIA_INICIO 	(20000000)
#define BLA_UMBRAL_ENERGIA_FIN 		(10000000)

/* Cuantas muestras extras agrego para el reconocimiento */
#define BLA_OFFSET_MUESTRAS (1000)

/*----------------------------------------------------------------------*/

/*!Parámetros del cálculo de coeficientes MFCC
	! La modificación requiere reentrenar el modelo usando HERest 
	! y una base de datos de frases junto a sus transcripciones*/
	
/*Para más información, cursar 86.53 "Procesamiento del Habla" :D o revisar el HTK Book*/

/*Bloques de 25 ms (400 muestras) tomados cada 10 ms (160 muestras)
  elegidos de forma tal que las ventanas se superponen, pudiendo suavizar transitorios abruptos*/
#define BLA_LARGOVENTANA (400)
#define BLA_LARGOBLOQUE (160)

/*Cantidad de filtros triangulares a usar*/
#define BLA_NUMCHANS (26)
/*Cantidad de coeficientes cepstrum que se almacenan*/
#define BLA_NUMCEPS (12)
/*Longitud del liftrado cepstral (amplificación de los coeficientes más altos para mejorar SNR)*/
#define BLA_CEPLIFTER (22)

/* Cantidad de bloques totales, teniendo en cuenta que no se pueden extaer 
	coeficientes del final porque no da la ventana (Se saldría del buffer) */ 
#define BLA_CANTIDAD_BLOQUES (((BLA_LARGO_BUFFER_AUDIO-BLA_LARGOVENTANA)/BLA_LARGOBLOQUE)+1)

#define BLA_ENERGIA_TIEMPO_EXTRA (3500)

#endif

