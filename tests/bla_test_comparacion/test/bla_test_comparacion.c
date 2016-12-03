/*
	Bla_Test_Comparación:
		Programa para PC (Win/Linux)
		Prueba reconocer todos los archivos de la carpeta de wavs, para calcular las tasas de error
		
	Tiempos medidos: (PC, i5 2400 @ 3.1 GHz, 150 wavs)
		66.92 ms +- 14.83 ms - Con viterbi normal (pronunciación en tabla ints)
		64.96 ms +- 14.31 ms - Con viterbi mirando solo +-2 estados
		
*/

#include <stdio.h>
#include <string.h>
#include <inttypes.h>
#include <math.h>
#include <bla.h>
#include <bla_config.h>
#include <bla_mfcc.h>
#include <dirent.h>

#define DR_WAV_IMPLEMENTATION
#include "dr_wav.h"

#include "timing.h"
int tickCounter;

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

	*cantidadMuestras = totalSampleCount;
	return pSampleData;
}

const char *palabrasAReconocer[] = {"cero","uno","dos","tres","cuatro","cinco","seis","siete","ocho","nueve"};

int main() {
	//Inicializar biblioteca
	size_t cantPalabras = sizeof (palabrasAReconocer) / sizeof (*palabrasAReconocer);
	
	if(blaInicializarBiblioteca(palabrasAReconocer, cantPalabras) != BLA_OK){
		printf("Error inicializando: %s\n", blaLeerError());
	}
	
	//Recorrer carpeta de WAVs
	const char ruta[] = "../datos/wav/";
	DIR *dir;
	struct dirent *ent;
	
	int wavsTotales=0, wavsCorrectos=0;
	uint64_t tiempoTotal = 0;
	uint64_t tiempoTotalCuadrado = 0;
	
	if((dir=opendir(ruta)) != NULL) {
		while((ent = readdir (dir)) != NULL) {
			DIR *subdir;
			struct dirent *subent;
			char nombreCarpeta[128] = "";
			
			if(strcmp(ent->d_name,".") == 0 || strcmp(ent->d_name,"..") == 0)
				continue; 
			
			strcat(nombreCarpeta, ruta);
			strcat(nombreCarpeta, ent->d_name);
			
			subdir = opendir(nombreCarpeta);
			
			if(subdir == NULL)
				continue; //No era una carpeta
		
			printf ("%s\n", ent->d_name);
				
			while((subent = readdir (subdir)) != NULL) {
				if(strcmp(subent->d_name,".") == 0 || strcmp(subent->d_name,"..") == 0)
					continue; 
				
				char nombreArchivo[128] = "";
				strcat(nombreArchivo, nombreCarpeta);
				strcat(nombreArchivo, "/");
				strcat(nombreArchivo, subent->d_name);
				
				uint64_t cantidadMuestras=0;
				float *pSampleData = cargarWav(nombreArchivo, &cantidadMuestras);
				
				printf ("\t%s %d muestras ", subent->d_name, cantidadMuestras);
				
				if(cantidadMuestras > 16000)
					cantidadMuestras = 16000;
			
				int16_t buffer[cantidadMuestras];
				
				for(size_t i=0;i<cantidadMuestras;i++)
					buffer[i] = round(pSampleData[i]*32767.0f);
							
				const int tamanioLista = 10;
				blaListado listado[tamanioLista];
				
				uint64_t t1 = GetTimeMs64();
				blaReconocerBuffer(listado, tamanioLista, buffer, cantidadMuestras); //Fuerzo el reconocimiento
				uint64_t t2 = GetTimeMs64();
				
				tiempoTotal += t2-t1;
				tiempoTotalCuadrado += (t2-t1)*(t2-t1);
				
				printf("%d %d %d %f\n", listado[0].idPalabra, listado[1].idPalabra, listado[2].idPalabra, listado[0].probabilidad);
				
				if(listado[0].idPalabra+'0' == subent->d_name[0])
					wavsCorrectos++;
				
				wavsTotales++;
				drwav_free(pSampleData);
			}
		}
		closedir (dir);
	}

	float desvio = sqrt((tiempoTotalCuadrado-tiempoTotal*tiempoTotal/wavsTotales)/wavsTotales); //Estimador (sesgado) del desvio estándar
	
	printf("Resultados: \n");
	printf("\tCorrectos: %d de %d (%f%%)\n", wavsCorrectos, wavsTotales, 100.0f*(float)wavsCorrectos/(float)wavsTotales);
	printf("\tTiempo de ejecucion: %f ms (promedio) %f ms (desvio estandar)\n",(float)tiempoTotal/(float)wavsTotales, desvio);
	
	return 0;
}

