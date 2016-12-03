#include <bla.h>
#include <bla_config.h>
#include <bla_viterbi.h>
#include <bla_probabilidad.h>
#include <bla_modelo.h>
#include <math.h>
extern volatile uint32_t tickCounter;
static inline float blaLogTransicion(int estadoPrevio, int estadoActual, int numeroEstados, const blaFonema_t *fPrevio, const blaFonema_t *fActual){
	if(estadoActual > 3 && estadoActual < numeroEstados-3 && estadoActual != numeroEstados){
		/*Transiciones de la parte "normal", solo puedo ir para adelante o seguir en mismo estado*/
		if(estadoPrevio == estadoActual){
			/*Transicion de seguir en el mismo estado*/
			return fActual->logProbMismo[estadoActual%3];
		}else if(estadoPrevio == estadoActual-1){
			/*Transicion de "subir" un estado*/
			return fPrevio->logProbTrans[estadoPrevio%3];
		}else{
			/*Otros casos*/
			return LZERO;
		}
	}else if(estadoActual == 0){
		/*Para pasar al primer estado se puede venir del primero o del último*/
		if(estadoPrevio == 0){
			/*Prob de pasar sil_0 a sil_0*/
			return logf(blaProbExtendida[0]);
		}else if(estadoPrevio == 2){
			/*Prob de pasar sil_2 a sil_0*/
			return logf(blaProbExtendida[6]);
		}else{
			return LZERO;
		}
	}else if(estadoActual == 1){
		/*Para pasar al segundo estado se puede venir del segundo o del primero*/
		if(estadoPrevio == 1){
			/*Prob de pasar sil_1 a sil_1*/
			return logf(blaProbExtendida[4]);
		}else if(estadoPrevio == 0){
			/*Prob de pasar sil_0 a sil_1*/
			return logf(blaProbExtendida[1]);
		}else{
			return LZERO;
		}
	}else if(estadoActual == 2){
		/*Para pasar al tercer estado se puede venir de cualquiera de los tres anteriores*/
		if(estadoPrevio == 0){
			/*Prob de pasar sil_0 a sil_2*/
			return logf(blaProbExtendida[2]);
		}else if(estadoPrevio == 1){
			/*Prob de pasar sil_1 a sil_2*/
			return logf(blaProbExtendida[5]);
		}else if(estadoPrevio == 2){
			/*Prob de pasar sil_2 a sil_2*/
			return logf(blaProbExtendida[8]);
		}else{
			return LZERO;
		}
	}else if(estadoActual == 3){
		/*Para pasar al cuarto estado (fon_0) se puede venir del mismo o del anterior (sil_2)*/
		if(estadoPrevio == 2){
			/*Prob de pasar sil_2 a fon_0 (el resto que queda)*/
			return logf(1.0-blaProbExtendida[8]-blaProbExtendida[6]);
		}else if(estadoPrevio == 3){
			/*Prob de pasar fon_0 a fon_0*/
			return fActual->logProbMismo[0];
		}else{
			return LZERO;
		}
	}else if(estadoActual == numeroEstados-1){
		/*Para pasar al último estado se puede venir de cualquiera de los tres anteriores*/
		if(estadoPrevio == numeroEstados-3){
			/*Prob de pasar sil_0 a sil_2*/
			return logf(blaProbExtendida[2]);
		}else if(estadoPrevio == numeroEstados-2){
			/*Prob de pasar sil_1 a sil_2*/
			return logf(blaProbExtendida[5]);
		}else if(estadoPrevio == numeroEstados-1){
			/*Prob de pasar sil_2 a sil_2*/
			return logf(blaProbExtendida[8]);
		}else{
			return LZERO;
		}
	}else if(estadoActual == numeroEstados-2){
		/*Para pasar al segundo estado se puede venir del segundo o del primero*/
		if(estadoPrevio == numeroEstados-2){
			/*Prob de pasar sil_1 a sil_1*/
			return logf(blaProbExtendida[4]);
		}else if(estadoPrevio == numeroEstados-3){
			/*Prob de pasar sil_0 a sil_1*/
			return logf(blaProbExtendida[1]);
		}else{
			return LZERO;
		}
	}else if(estadoActual == numeroEstados-3){
		/*Para pasar al primer estado (sil_0 del final) se puede venir del último (-1), mismo (-3) o anterior (-4)*/
		if(estadoPrevio == numeroEstados-3){
			/*Prob de pasar sil_0 a sil_0*/
			return logf(blaProbExtendida[0]);
		}else if(estadoPrevio == numeroEstados-1){
			/*Prob de pasar sil_2 a sil_0*/
			return logf(blaProbExtendida[6]);
		}else if(estadoPrevio == numeroEstados-4){
			/*Prob de pasar fon_2 a sil_0*/
			return fPrevio->logProbTrans[estadoPrevio%3];
			
		}else{
			return LZERO;
		}
	}else if(estadoActual == numeroEstados){
		if(estadoPrevio == numeroEstados){
			return logf(1); /*Maxima probabilidad de quedarse en este estado*/
		}else if(estadoPrevio == numeroEstados-1){
			return logf(1.0-blaProbExtendida[8]-blaProbExtendida[6]); /*Probabilidad de salir de sil_2*/
		}else{
			return LZERO;
		}
	}
	#ifdef BLA_DEBUG
		printf("Probabilidad de transicion no cae en ningun caso! %d %d %d\n", estadoPrevio, estadoActual, numeroEstados);	
	#endif
	return LZERO;
}

/*Algoritmo de Viterbi, usado para encontrar el logaritmo de la probabilidad
  del camino más probable, que está relacionado con la verosimilitud del modelo.*/
  
/*Implementado con probabilidades en espacios logarítmicos, para evitar problemas numéricos
  Más detalles del algoritmo pueden ser encontrados en: 
  http://users-cs.au.dk/cstorm/courses/PRiB_f12/slides/hidden-markov-models-2.pdf */
 #include <stdio.h>
float blaViterbi(const int *pronunciacion, const int cantFonemas, const int cantBloques){
	/*Como no importa el camino tomado, no es necesario guardar toda la matriz, por lo que
	  los dos arrays se van usando de a turnos, de forma similar a un ping-pong buffering:
	  se cargan las probabilidades de uno y se modifica el otro*/
		
	#ifdef BLA_DEBUG
	uint32_t t1 = tickCounter;
	printf("Viterbi: %d bloques, ", cantBloques);
	#endif
	
	int numeroEstados = 3*cantFonemas;
	
	float delta[2][numeroEstados+1]; /*Agrego un estado final (que no emite símbolos)*/
	int idx = 0, i,k,j;
	
	/*Inicializar vector*/
	for(i=0;i<numeroEstados+1;i++){
		/*El primer estado tiene probabilidad 1 (en log, 0), el resto probabilidad 0 (en log, -infinito)*/
		delta[idx][i] = (i==0)?0:-INFINITY; 
	}
			
	/*Recursión de Viterbi:
	  j = bloque, correspodiente al eje del tiempo
	  i = estado actual 
	  k = estado anterior
	*/
	for(j=0;j<cantBloques;j++){ 
		idx++; idx%=2; /*Paso al otro buffer*/
		
		for(i=0;i<numeroEstados+1;i++){
			float maxLogProb = -INFINITY;
			const blaEstado_t *estadoActual = &(blaFonemas[pronunciacion[i/3]].estados[i%3]);
			const blaFonema_t *fActual = &(blaFonemas[pronunciacion[i/3]]);
			
			int inf = i-2;
			if(inf<0)
				inf = 0;
			
			int sup = i+2;
			if(sup>numeroEstados)
				sup = numeroEstados;
			
			for(k=inf;k<sup;k++){
				
				const blaFonema_t *fAnterior = &(blaFonemas[pronunciacion[k/3]]);
				
				float logProbN = delta[1-idx][k] + blaLogTransicion(k,i,numeroEstados, fAnterior, fActual);
				
				if(logProbN > maxLogProb){
					maxLogProb = logProbN;
				}
			}
			if(i == numeroEstados){
				/*El último estado no emite observación, por lo que la probabilidad es solo el primer término */
				delta[idx][i] = maxLogProb ;
			}else{
				float logProbObs = blaProbabilidadMezclaGaussiana(estadoActual,&(blaMFCCBloques[j][0]));
				
				delta[idx][i] = maxLogProb + logProbObs;
			}
		}
	}
	/*Paso final*/
	#if 0
	/*Suponiendo que el estado final puede ser cualquiera*/
	float logProb = -INFINITY;
	for(i=0;i<numeroEstados+1;i++){
		if(delta[idx][i] > logProb){
			logProb = delta[idx][i];
		}
	}
	#endif
	float logProb = delta[idx][numeroEstados]; /*Supongo que el estado final es el último*/
	#ifdef BLA_DEBUG
	printf("%d ms (%f)\r\n", tickCounter-t1,  (float)(tickCounter-t1)/(float)cantBloques);
	#endif
	return logProb;
}
