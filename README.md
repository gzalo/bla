# BLA
Biblioteca liviana para Reconocimiento de Habla en español.

La idea es poder integrar de forma sencilla a una aplicación embebida ya existente el reconocimiento de palabras aisladas. Implementando únicamente lo mínimo requerido y de forma optimizada, se espera lograr tasas de errores muy bajas con consumo bajo de recursos.

Se incluye código de prueba para un microcontrolador de 32 bits LPC4337 (ARM Cortex M4/M0) como el de la Computadora Industrial Abierta Argentina (CIAA), en ejemplos/EDU-CIAA-baremetal.

[Documentación básica de la API](docs/API.md)

El proyecto fue desarrollado como trabajo final para la materia ["Seminario de Sistemas Embebidos"](http://laboratorios.fi.uba.ar/lse/seminario/) de la Facultad de Ingeniería de la UBA (FIUBA), en Buenos Aires, Argentina.

Detalles técnicos:

* Reconocimiento de palabras aisladas, independiente del hablante.
* La estrategia para detectar comienzo/fin de palabras se basa en una ventana deslizante, a la cual se le toma la energía. Dicha energía se compara con un umbral (con histéresis).
* El reconocimiento se basa en calcular los coeficientes MFCC de la señal capturada, y en base a los modelos ocultos de Markov, encontrar la palabra que es más probable que haya generado las observaciones.
* Los modelos de cada fonema están basados en varios estados que emiten mezclas de Gaussianas, con matrices de covarianza diagonales (internamente almacenada como sus inversas, para usar solo productos y no divisiones)
* Se corre el algoritmo de Viterbi por cada palabra, lo que encuentra la probabilidad del camino más probable. En base a esta probabilidad se ordenan las distintas palabras.
* Los cálculos de probabilidades se implementaron en espacios logarítmicos, para evitar problemas numéricos.
* La señal de audio debe estar muestreada a 16 KHz, de lo contrario es necesario reentrenar los modelos con otros datos de entrenamiento.	

Desarrollador: [Gonzalo Ávila Alterach / gzalo](http://gzalo.com)

---

Lightweight library for speech recognition.

The main idea is to integrate to an existing embedded application the ability to recognise isolated words. By implementing just the needed modules, and optimizing the code, low error rate with low resource usage is expected.

Test code for a 32 bits LPC4337 microcontroller (ARM Cortex M4/M0) like the one from the Computadora Industrial Abierta Argentina (CIAA) is included, in the folder ejemplos/EDU-CIAA-baremetal.

[Basic API Documentation (in spanish)](docs/API.md)

This project was developed for the assignature ["Seminario de Sistemas Embebidos"](http://laboratorios.fi.uba.ar/lse/seminario/) of UBA Faculty of Engineering (FIUBA), in Buenos Aires, Argentina.

Specifications:

* Speaker independent, Isolated word recognition.
* Voice activity detection based in energy of a sliding window, which is compared to a threshold (with some histeresis).
* Recognition based in MFCC coefficients, and Hidden Markov Models. The most likely N-words are displayed.
* Per phoneme models, based on Gaussian mixtures, with diagonal covariance matrices.
* Viterbi algorithm is executed once for each word, which finds the probability of the most probable path. Words are sorted by their probability.
* Log-spaces are used to calculate probabilities and to use Viterbi, in order to avoid numerical problems.
* Audio input signal must be sampled at 16 KHz (or a higher rate and downsampled via decimation or averaging, check sample code), otherwise the models must be replaced with others which must be trained with the correct sample rate.

Developer: [Gonzalo Ávila Alterach / gzalo](http://gzalo.com)
