/*
	Bla_Test_Coeficientes:
		Programa para PC (Win/Linux)
		Prueba la parte de extracción de coeficientes MFCC de la biblioteca, cargando un wav usando la biblioteca dr_wav
*/

#include <stdio.h>
#include <string.h>
#include <inttypes.h>
#include <math.h>
#include <bla.h>
#include <bla_config.h>
#include <bla_mfcc.h>

#define DR_WAV_IMPLEMENTATION
#include "dr_wav.h"

const char archivoWav[]="testaudio.wav";
int tickCounter = 0;

float *cargarWav(const char *nombreArchivo, uint64_t *cantidadMuestras){
	unsigned int channels;
	unsigned int sampleRate;
	uint64_t totalSampleCount;
	float *pSampleData = drwav_open_and_read_file_f32(nombreArchivo, &channels, &sampleRate, &totalSampleCount);
	
	if (pSampleData == NULL) {
		printf("No se puede abrir el archivo de audio\n");
		return NULL;
	}
	if(channels != 1){
		printf("Mas de 1 canal presente en el archivo.\n");
		return NULL;
	}
	if(sampleRate != 16000){
		printf("Tasa de muestreo incorrecta: %ud\n", sampleRate);
		return NULL;
	}
	printf("Muestras del archivo: %d\n", totalSampleCount);
	
	*cantidadMuestras = totalSampleCount;
	return pSampleData;
}

int main() {
	uint64_t cantidadMuestras;
	float *muestras = cargarWav(archivoWav,&cantidadMuestras);
	
	const int cantidadBloques = (BLA_LARGO_BUFFER_AUDIO-BLA_LARGOVENTANA)/BLA_LARGOBLOQUE;
	
	for(int i=0;i<=cantidadBloques;i++){
		int16_t buffer[BLA_LARGOVENTANA];
		for(int j=0;j<BLA_LARGOVENTANA;j++){
			buffer[j] = round(muestras[i*BLA_LARGOBLOQUE+j]*(BLA_ADC_RANGO+1));
		}
		
		blaObtenerCoeficientes(buffer, blaMFCCBloques[i]);
	}
	
	//Calcular deltas 
	blaCalcularDeltas(0, BLA_NUMCEPS+1);
	//Calcular aceleraciones
	blaCalcularDeltas(BLA_NUMCEPS+1, 2*BLA_NUMCEPS+2);
	
	//Calcular coeficientes usando el HTK
	const char rutaHTK[] = "C:\\MinGW\\msys\\1.0\\home\\gzalo\\htk\\HTKTools\\";
	char comando[256];
	sprintf(comando,"%sHCopy -T 1 -C config.hcopy testaudio.wav testaudio.mfc", rutaHTK);
	system(comando);
	sprintf(comando,"%sHList -h testaudio.mfc > testaudio.mfc.txt", rutaHTK);
	system(comando);
	
	//Cargamos el archivo con los coeficientes calculados por el HTK
	FILE *fp = fopen("testaudio.mfc.txt","r");
	
	char bufferLinea[128];
	//Salteamos las primeras 5 líneas
	for(int i=0;i<5;i++)
		fgets(bufferLinea, 128, fp);

	int val;
	float bufferLocal[39];
	float deltamax = 0;
	char idxMax[16] = {0};
	
	for(int i=0;i<=cantidadBloques;i++){
		printf("Bloque %d: ", i);
		for(int j=0;j<39;j++){
			if(j%10 == 0){
				fscanf(fp, "%d:", &val);
				if(val != i){
					printf("ERROR: No coinciden los indices!");
					return -1;
				}
			}
			fscanf(fp, "%f", &(bufferLocal[j]));
		}	
		
		
		for(int j=0;j<3*(BLA_NUMCEPS+1);j++){
			float c1 = blaMFCCBloques[i][j];
			float c2 = bufferLocal[j];
			float delta = fabs(c1-c2);
			
			if(delta>deltamax){
				deltamax = delta;
				sprintf(idxMax, "%d %d", i,j);
			}
			
			printf("%f %f  ", c1, c2);
		}
		
		printf("\n");
		
	}
	fclose(fp);
	printf("Max delta: %f\n",deltamax);
	printf("En posicion: %s\n", idxMax);
	return 0;
}

