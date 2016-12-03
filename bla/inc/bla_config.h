/*
	BLA - Gonzalo �vila Alterach (http://gzalo.com)
	Biblioteca liviana para realizar reconocimiento de habla 
	
	Archivo de configuraci�n
*/

#ifndef BLA_CONFIG_H
#define BLA_CONFIG_H

/*!Par�metros modificables*/

/*Descomentar para agregar varios chequeos extra en valores, y printfs para debuggear,
  solamente �til para Windows/Linux*/
/* #define BLA_DEBUG */

/*M�xima cantidad de fonemas por palabra*/
#define BLA_MAX_CANT_FONEMAS 10

/*M�xima cantidad de palabras a reconocer*/
#define BLA_MAX_PALABRAS (10)

/*Rango del ADC (se supone que va entre -BLA_ADC_RANGO-1 y BLA_ADC_RANGO)
  Es requerido para calcular correctamente el coeficiente 0, ya que de lo contrario
  queda escalado por un valor y puede influir negativamente en el reconocimiento*/
#ifndef BLA_ADC_RANGO
#define BLA_ADC_RANGO (511*8)
#endif

/*Longitud del buffer principal de audio, de 16 bits por muestra (t�picamente 1 segundo)*/
#define BLA_LARGO_BUFFER_AUDIO (16000)	

/*Longitud (en muestras) de la ventana que acumula la energ�a para detectar
  comienzos y finales de palabras (t�picamente 0.1 segundos) */
#define BLA_ACUMULADOR_ENERGIA_VENTANA (1600)

/*Valor de energ�a a partir del cual se supone que se est� hablando
  puede ser necesario modificarlo en funci�n del tipo de micr�fono, la distancia al hablante
  y el ruido de fondo (del conversor anal�gico digital y del entorno)*/
#define BLA_UMBRAL_ENERGIA_INICIO 	(20000000)
#define BLA_UMBRAL_ENERGIA_FIN 		(10000000)

/* Cuantas muestras extras agrego para el reconocimiento */
#define BLA_OFFSET_MUESTRAS (1000)

/*----------------------------------------------------------------------*/

/*!Par�metros del c�lculo de coeficientes MFCC
	! La modificaci�n requiere reentrenar el modelo usando HERest 
	! y una base de datos de frases junto a sus transcripciones*/
	
/*Para m�s informaci�n, cursar 86.53 "Procesamiento del Habla" :D o revisar el HTK Book*/

/*Bloques de 25 ms (400 muestras) tomados cada 10 ms (160 muestras)
  elegidos de forma tal que las ventanas se superponen, pudiendo suavizar transitorios abruptos*/
#define BLA_LARGOVENTANA (400)
#define BLA_LARGOBLOQUE (160)

/*Cantidad de filtros triangulares a usar*/
#define BLA_NUMCHANS (26)
/*Cantidad de coeficientes cepstrum que se almacenan*/
#define BLA_NUMCEPS (12)
/*Longitud del liftrado cepstral (amplificaci�n de los coeficientes m�s altos para mejorar SNR)*/
#define BLA_CEPLIFTER (22)

/* Cantidad de bloques totales, teniendo en cuenta que no se pueden extaer 
	coeficientes del final porque no da la ventana (Se saldr�a del buffer) */ 
#define BLA_CANTIDAD_BLOQUES (((BLA_LARGO_BUFFER_AUDIO-BLA_LARGOVENTANA)/BLA_LARGOBLOQUE)+1)

#define BLA_ENERGIA_TIEMPO_EXTRA (3500)

#endif

