# API de la biblioteca BLA

## Inicialización

`int blaInicializarBiblioteca(const char *palabrasAReconocer[], int cantidadPalabras);`

Inicializa el reconocimiento, a la función se le pasa un vector de cadenas con las palabras que se desean reconocer, y su longitud. 

Devuelve BLA_OK si fue inicializada correctamente.

## Muestreado

`int blaAlmacenarValorMicrofono(int16_t valor);`

Cada vez que se captura una nueva muestra es necesario llamar a esta función, a la que se le pasa como argumento el valor de capturado como un número entero. El rango deberá ser definido en una macro (ver más abajo).

En el caso de que las muestras estén disponibles de a bloques, se deberá llamar a la función tantas veces como sea el largo del bloque.

## Detección comienzo/fin de palabra

`int blaDeteccionPeriodica(void);`

Periódicamente se deberá llamar a esta función, que se encarga de verificar si comenzó o terminó una palabra. 

El periódo con el que se llame a esta función deberá ser de aproximadamente 10 ms, no importa si tiene un poco de fluctuaciones. Si es mucho más grande que 10 ms, puede que aumente la tasa de error del reconocimiento.

Si se detecta la finalización de una palabra, su valor de retorno es BLA_FIN_PALABRA.

## Reconocimiento

### Estructura blaListado
Esta estructura se usada para devolver al usuario los resultados del reconocimiento. la misma posee un entero idPalabra que identifica a qué palabra corresponde, pudiendo ser usado como índice sobre el vector de palabras con el que se inicializa la biblioteca.
También tiene un float probabilidad, que es como un puntaje, representa cuán parecido es el modelo de la palabra a los datos que se capturaron.

`int blaReconocer(blaListado *listaProbabilidades, int cantidadLista);`

Luego de detectar el fin de una palabra, la aplicación del usuario puede llamar a esta función para realizar el reconocimiento propiamente dicho. 
Como argumento es necesario pasarle un vector de blaListado, y la longitud de dicho vector. Si solamente se desea encontrar la palabra más probable, dicho largo deberá ser establecido en 1.
La función encuentra y devuelve las cantidadLista palabras más probables, modificando el vector provisto. La más probable queda almacenada en el índice 0, mientras que la menos probable queda en el índice cantidadLista-1.

## Manejo de errores
`const char *blaLeerError(void);`

En caso de que alguna de las funciones llamadas haya fallado (no devolviendo 0/BLA_OK), es posible obtener una cadena de texto con una descripción básica del error, llamando a esta función inmediatamente después de la función que falló.

Las funciones mencionadas tienen sus prototipos en el archivo bla.h. 

# Macros editables

Se definieron las siguientes macros modificables por el usuario, en el archivo bla_config.h:

* BLA_DEBUG: si está definida agrega algunas impresiones por pantalla, para probar diversas partes del reconocimiento. Para mejorar el rendimiento es necesario desactivarla.
* BLA_MAX_CANT_FONEMAS (por defecto, 10): da un límite de cuantos fonemas tiene la palabra más larga que se desea reconocer. Si se usan palabras muy largas, puede ser necesario tener que aumentar el valor.
* BLA_MAX_PALABRAS (por defecto, 10): define un límite de cuántas palabras máximas se pueden reconocer. Para reducir el consumo de memoria, se puede ajustar al valor correcto.
* BLA_ADC_RANGO (por defecto, 511*8): define el rango de la señal que se le pasa a la biblioteca. Es necesario ajustarlo correctamente para que todos los coeficientes sean calculado correctamente.
* BLA_LARGO_BUFFER_AUDIO (por defecto, 16000): define la longitud del buffer principal que guarda las muestras de audio a medida que son capturadas. 
* BLA_ACUMULADOR_ENERGIA_VENTANA (Por defecto, 1600): Define la longitud de la ventana deslizante a la que se le calcula la energía, como parte del algoritmo de detección de comienzo y fines de palabra.
* BLA_UMBRAL_ENERGIA (Por defecto, 8000000): define un umbral de energía, a partir del cual se considera que hay alguien hablando. Puede ser necesario modificarlo en función del tipo de micrófono, la distancia al hablante y el ruido de fondo (del conversor analógico-digital y del entorno)
* BLA_OFFSET_MUESTRAS (Por defecto, 1200): define cuántas muestras de antes de la detección de voz se agregan al reconocimiento. De esta forma si el algoritmo de detección de comienzo tarda un poco más en detectarlo no se trunca el principio del audio.

Además se agregaron otras macros cuya modificación requiere un reentrenamiento de los modelos de habla usados, por lo que no se describirán en este documento. 
