/*
	Bla_Probabilidad:
		Calcula probabilidades de observaci�n de una vector de coeficientes,
		dado un determinado estado.
		
		Cada estado tiene como funci�n de densidad de probabilidad la mezcla 
		de NUMEROMEZCLAS gaussianas, todas con matriz de covarianza diagonal.
		
		Se calcula directamente el logaritmo de las probabilidades, para 
		simplificar las cuentas (no requiere la funci�n exponencial) y porque
		es m�s estable num�ricamente a la hora de usar el algoritmo Viterbi.
*/
#include <bla_probabilidad.h>
#include <bla_modelo.h>
#include <math.h>
#include <arm_math.h>
#include <arm_const_structs.h>

static float minLogExp;

int blaProbabilidadInicializar(void){
	minLogExp = -logf(-LZERO);
	return 0;
}

inline static float blaLogSaturado(float x){
	if(x<LSMALL)
		return LZERO;
	else
		return logf(x);
}

/*Sumar dos probabilidades(Log)
  Realiza la operaci�n denominada SumLogExp
  aprovecha propiedades del logaritmo para evitar problemas num�ricos*/
inline static float blaSumarProbabilidad(float a, float b){
	if(a < b){
		/*Caso B>A*/
		float diferencia = a-b;
		if(diferencia < minLogExp){
			return b<LSMALL?LZERO:b;
		}
		return b+logf(1.0f + expf(diferencia));
	}else{
		/*Caso A>B*/
		float diferencia = b-a;
		if(diferencia < minLogExp){
			return a<LSMALL?LZERO:a;
		}
		return a+logf(1.0f + expf(diferencia));
	}
}

/*Probabilidad(Log) de una determinada observaci�n x, dada una gaussiana N dimensional dada*/
inline static float blaProbabilidadGaussiana(const blaMezcla_t *mezcla, float *x){
	#ifndef BLA_INV_VARIANZA
		#error Falta escribir el c�digo optimizado para el caso de matrices con inversa de varianza!
	#endif
	
	int i;
	float prob = mezcla->constante;

	
	#if 0
		/*C�digo un poco m�s r�pido, usando funciones de la biblioteca CMSIS-DSp*/
		float32_t buffer[NUMERODIMENSIONES];
		arm_sub_f32((float32_t*)x, (float32_t*)mezcla->medias, buffer, NUMERODIMENSIONES); /*Resto las medias componente a componente*/
		arm_mult_f32(buffer, buffer, buffer, NUMERODIMENSIONES); /* Elevo al cuadrado */
		arm_mult_f32(buffer, (float32_t*)mezcla->varianzas, buffer, NUMERODIMENSIONES); /* Multiplico por las inverzas de las varianzas */
		
		for(i=0;i<NUMERODIMENSIONES;i++)
			prob += buffer[i];
	#endif	
	
	/*C�digo original lento*/
	for(i=0;i<NUMERODIMENSIONES;i++){
		float xSinMedia = x[i] - mezcla->medias[i];
		/* LogProb de una gaussiana con matriz de covarianza diagonal */
		#ifdef BLA_INV_VARIANZA
			prob += xSinMedia*xSinMedia*mezcla->varianzas[i];
		#else
			prob += xSinMedia*xSinMedia/mezcla->varianzas[i];
		#endif
	}

	return -0.5f*prob;
}

/*Probabilidad(Log) de una determinada observaci�n x, dada una mezcla de varias gaussianas*/
inline float blaProbabilidadMezclaGaussiana(const blaEstado_t *estado, float *x){
	float prob = LZERO;
	int i;
	
	for(i=0;i<NUMEROMEZCLAS;i++){
		float logPeso;
		
		/*Supongo que el peso de la mezcla actual no es despreciable*/
		logPeso = estado->mezclas[i].logPeso;
		
		float probabilidadIndividual = blaProbabilidadGaussiana(&(estado->mezclas[i]), x);
		prob = blaSumarProbabilidad(prob,probabilidadIndividual+logPeso);
	}
	
	return prob;
}

/*
Detalles de funciones de HTK (implementados en Hmodel.c)

	FixDiagGConst: Calcula la constante GCONST (la parte que no depende de la observaci�n)
	MOutP: Calcula log-probabilidad de X dada una mezcla[i]
	DOutP: Implementacion de logprob para una mezcla dada para diagonal (tambi�n suma el gconst)
	PrecomputeTMix: Ordena las probabilidades individuales que tira Mout (no lo llama nadie)
	SOutP: Log prob de una dada observaci�n
	LAdd: Suma x+y en escala logar�tmica
	MixLogWeight: calcula el log del peso de una mezcla, redondeandolo si es muy chico
	
*/

