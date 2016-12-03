/*
	Bla_Test_PC:
		Programa para PC (Win/Linux)
		Prueba la biblioteca usando audio del sistema, a través de la biblioteca portaudio
		o carga un wav usando la biblioteca dr_wav
*/

#include <stdio.h>
#include <string.h>
#include <inttypes.h>
#include <math.h>
#include <bla.h>
#include "timing.h"

#include <portaudio.h>

#ifdef LINUX
	#include <unistd.h>
#endif
#ifdef __MINGW32__
	#include <windows.h>
	int getch_noblock() {
		if (_kbhit())
			return _getch();
		else
			return -1;
	}
#endif

void sleepms(int val){
	#ifdef LINUX
		usleep(val * 1000);
	#endif
	#ifdef __MINGW32__
		Sleep(val);
	#endif
}

PaStream *stream;

int AudioCallback(const void *inputBuffer, void *outputBuffer,
                         unsigned long framesPerBuffer,
                         const PaStreamCallbackTimeInfo* timeInfo,
                         PaStreamCallbackFlags statusFlags,
                         void *userData){
	float *dataBff = (float*)inputBuffer;
		
	//La biblioteca devuelve las muestras de micrófono de a "bloques"
	//Se las paso a BLA como si hubiesen llegado en simultáneo (da lo mismo)

	for(size_t i=0;i<framesPerBuffer;i++){
		blaAlmacenarValorMicrofono(round(dataBff[i]*32767));	
	}
	return 0;
}

int AudioInit(void){	
	//Inicializar el sistema de captura de audio usando PortAudio
	
    PaStreamParameters inputParameters;
    PaError err;
		
    err = Pa_Initialize();
    if(err != paNoError){
		fprintf( stderr, "An error occured while using the portaudio stream\n" );
		fprintf( stderr, "Error number: %d\n", err );
		fprintf( stderr, "Error message: %s\n", Pa_GetErrorText( err ) );
		Pa_Terminate();
		return -1;
	}

    inputParameters.device = Pa_GetDefaultInputDevice(); /* default input device */
    if (inputParameters.device == paNoDevice) {
		fprintf(stderr,"Error: No default input device.\n");
		Pa_Terminate();
		return -1;
    }
    inputParameters.channelCount = 1;       /* mono input */
    inputParameters.sampleFormat = paFloat32;
    inputParameters.suggestedLatency = Pa_GetDeviceInfo( inputParameters.device )->defaultLowInputLatency;
    inputParameters.hostApiSpecificStreamInfo = NULL;

    err = Pa_OpenStream(
		  &stream,
		  &inputParameters,
		  NULL,
		  16000,
		  1024,
		  0, /* paClipOff, */  /* we won't output out of range samples so don't bother clipping them */
		  AudioCallback,
		  NULL );
			  
    if( err != paNoError ){
		fprintf( stderr, "An error occured while using the portaudio stream\n" );
		fprintf( stderr, "Error number: %d\n", err );
		fprintf( stderr, "Error message: %s\n", Pa_GetErrorText( err ) );
		Pa_Terminate();
		return -1;
	}

    err = Pa_StartStream( stream );
    if( err != paNoError ){
		fprintf( stderr, "An error occured while using the portaudio stream\n" );
		fprintf( stderr, "Error number: %d\n", err );
		fprintf( stderr, "Error message: %s\n", Pa_GetErrorText( err ) );
		Pa_Terminate();
		return -1;
	}
	
    return 0;

}

int AudioEnd(void){
	//Desinicializar el sistema de captura de audio usando PortAudio

	PaError err;
    err = Pa_CloseStream( stream );
    if( err != paNoError ){
		fprintf( stderr, "An error occured while using the portaudio stream\n" );
		fprintf( stderr, "Error number: %d\n", err );
		fprintf( stderr, "Error message: %s\n", Pa_GetErrorText( err ) );
		return 1;
	}
    Pa_Terminate();
	return 0;
}

int main() {
	const char *palabrasAReconocer[] = {"cero","uno","dos","tres","cuatro","cinco","seis","siete","ocho","nueve"};
	size_t cantPalabras = sizeof (palabrasAReconocer) / sizeof (*palabrasAReconocer);
	
	if(blaInicializarBiblioteca(palabrasAReconocer, cantPalabras) != BLA_OK){
		printf("Error inicializando: %s\n", blaLeerError());
	}
		
	AudioInit();
	int fin = 0;
	int pRec = 0;
	
	char stringReconocido[128] = "";
	
	while (!fin) {
		GotoXy(0,0);
		printf("Esperando (%lld)         \n", blaLeerAcumuladorEnergia());
		printf("Reconocido: %s\n", stringReconocido);
		
		int ret = blaDeteccionPeriodica();
				
		if(ret == BLA_FIN_PALABRA){
			const int tamanioLista = 2;
			blaListado listado[tamanioLista];
			
			ret = blaReconocer(listado, tamanioLista);
			
			if(ret != BLA_OK){
				printf("Error reconociendo: %s\n", blaLeerError());
			}else{
				int i;
				GotoXy(0,pRec+2);
				pRec++;
				for(i=0;i<tamanioLista;i++)
					printf("%s\t%f\t", palabrasAReconocer[listado[i].idPalabra], listado[i].probabilidad);
				
				char rec[2] = {'0'+listado[0].idPalabra,0};
				strcat(stringReconocido, rec);
			}
			
		}
		sleepms(100);
		
		int v = getch_noblock();
		if(v == 'x')
			fin = 1;
	}
	AudioEnd();

}
