# BLA
Biblioteca liviana para Reconocimiento de Habla en español.

La idea es poder integrar de forma sencilla a una aplicación embebida ya existente el reconocimiento de palabras aisladas. Implementando únicamente lo mínimo requerido y de forma optimizada, se espera lograr tasas de errores muy bajas con consumo bajo de recursos.

Se incluye código de prueba para un microcontrolador de 32 bits LPC4337 (ARM Cortex M4/M0) como el de la Computadora Industrial Abierta Argentina (CIAA), en ejemplos/EDU-CIAA-baremetal.

[Documentación básica de la API](docs/API.md)

El proyecto fue desarrollado como trabajo final para la materia ["Seminario de Sistemas Embebidos"](http://laboratorios.fi.uba.ar/lse/seminario/) de la Facultad de Ingeniería de la UBA (FIUBA), en Buenos Aires, Argentina.

Detalles técnicos:

* Reconocimiento de palabras aisladas, independiente del hablante.
* La estrategia para detecctar comienzo/fin de palabras se basa en una ventana deslizante, a la cual se le toma la energía. Dicha energía se compara con un umbral (con histéresis).
* El reconocimiento se basa en calcular los coeficientes MFCC de la señal capturada, y en base a los modelos ocultos de Markov, encontrar la palabra que es más probable que haya generado las observaciones.
* Los modelos de cada fonema están basados en varios estados que emiten mezclas de Gaussianas, con matrices de covarianza diagonales (internamente almacenada como sus inversas, para usar solo productos y no divisiones)
* Se corre el algoritmo de Viterbi por cada palabra, lo que encuentra la probabilidad del camino más probable. En base a esta probabilidad se ordenan las distintas palabras.
* Los cálculos de probabilidades se implementaron en espacios logarítmicos, para evitar problemas numéricos.
* La señal de audio debe estar muestreada a 16 KHz, de lo contrario es necesario reentrenar los modelos con otros datos de entrenamiento.	

Desarrollador: [Gonzalo Ávila Alterach / gzalo](http://gzalo.com)
