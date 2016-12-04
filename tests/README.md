# Tests y generadores para BLA

## bla_gen_hamming

Genera coeficientes de una ventana de Hamming (de largo BLA_LARGOVENTANA).

## bla_gen_modelo

Convierte de formato un modelo de fonemas de HTK a un formato propio, en forma de un archivo bla_modelo.c y bla_modelo.h que contienen constantes.

Únicamente funciona para ciertos tipos de modelos (varianza diagonal) y mezclas de gaussianas. El código no es muy prolijo, pero funciona relativamente bien.

## bla_gen_tcoseno

Genera una matriz con los coeficientes necesarios para la transformada coseno discreta tipo 2 (que se usa para calcular los coeficientes MFCC).

## bla_test_coeficientes

Compara los coeficientes MFCC obtenidos con la biblioteca propia con los obtenidos mediante el toolkit HTK, cargando archivos de audio wav usando la biblioteca dr_wav.

## bla_test_comparacion

Compara las tasas de error del sistema completo, contra las del toolkit HTK, usando datos de prueba compuestos por aproximadamente 15 pronunciaciones propias de cada dígito, grabadas anteriormente para Procesamiento del Habla (no se incluyen los wavs, solamente los MFCs ya convertidos). 

## bla_test_diccionario

Valida las reglas utilizadas para convertir una palabra en los fonemas correspondientes, utilizando un diccionario de pronunciación provisto por el Grupo de Procesamiento del Habla de la FIUBA. 

Dicho diccionario no se incluye por problemas de licencia (no se puede distribuir libremente).

## bla_test_energia

Pruebas en MATLAB de varias formas para detectar comienzos y fines de palabras.

## bla_test_validacion

Aplicación de ejemplo para PC, usando la biblioteca PortAudio. Permite probar el reconocimiento usando dispositivos de grabación como un micrófono, o la misma salida de audio del sistema, si se activa el dispositivo denominado "Mezcla estéreo".