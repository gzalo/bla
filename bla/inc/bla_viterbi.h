/*
	Bla_Viterbi:
		Realiza el algoritmo de Viterbi y devuelve el logaritmo de la probabilidad del camino más probable
		Usando los modelos de modelos.h
 
*/

#ifndef BLA_VITERBI
#define BLA_VITERBI

float blaViterbi(const int *pronunciacion, const int cantFonemas, const int largoAudio);

#endif