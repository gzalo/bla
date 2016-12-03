/*
	Bla_App_LPC4337:
		Prueba la biblioteca usando datos obtenidos del ADC, el cual es leído desde las interrupciones del mismo
		
		El clock del ADC está definido por CLK_APB3_ADC0, que en la EDU-CIAA es de 204 MHz. Además cada conversión tarda 11 ciclos.
		Como el divisor del clock es de 8 bits, solamente se puede dividir por 256, lo que NO permite muestrear a 16000.
		Para solucionarlo, se sobremuestrea con un factor de 8x, lo que resulta en fs=128 KHz. Se realiza una promedio de las muestras.

*/

#include "chip.h"
#include "board.h"
#include <bla.h>
#include <stdio.h>
#include <string.h>
#define TICKRATE_HZ (1000)
volatile uint32_t tickCounter = 0;
/*Contador incrementado cada 1 ms */

/* Módulo y canal de ADC utilizado */
#define _ADC_CHANNEL ADC_CH1
#define _LPC_ADC_ID LPC_ADC0
#define _LPC_ADC_IRQ ADC0_IRQn
static ADC_CLOCK_SETUP_T ADCSetup;

/* Muestras promediadas por cada llamada a la biblioteca */
#define ADC_PROMEDIO (8)
static volatile uint32_t idxConversion = 0;
static volatile uint32_t cntConversion = 0;

void SysTick_Handler(void) {
	/*Incrementar el contador*/
	tickCounter++;

	if(tickCounter%100 == 0){
		/*Parpadeamos LED blanco (r+g+b)*/
		Board_LED_Toggle(3);
		Board_LED_Toggle(4);
		Board_LED_Toggle(5);	
	}
}
void delay(uint32_t tk) {
	uint32_t end = tickCounter + tk;
	while(tickCounter < end)
		__WFI();
}

volatile int16_t data;

void ADC0_IRQHandler(void){
	uint16_t dataADC = 0;
	
	NVIC_DisableIRQ(_LPC_ADC_IRQ);
	Chip_ADC_Int_SetChannelCmd(_LPC_ADC_ID, _ADC_CHANNEL, DISABLE);
	
	/*Lectura del dato del ADC del canal correspondiente*/
	Chip_ADC_ReadValue(_LPC_ADC_ID, _ADC_CHANNEL, &dataADC);
	
	/*Pasar el valor a la biblioteca, pasándolo a entero con signo (restando 2^9)*/
	data += (dataADC-512);
	idxConversion++;
	if(idxConversion == ADC_PROMEDIO){
		blaAlmacenarValorMicrofono(data);	
		idxConversion=0;
		data = 0;  /*Reseteamos el acumulador*/
	}
	cntConversion++;
	
	/*Rehabilitar el ADC*/
	NVIC_EnableIRQ(_LPC_ADC_IRQ);
	Chip_ADC_Int_SetChannelCmd(_LPC_ADC_ID, _ADC_CHANNEL, ENABLE);
}

int ADCInit(){
	Board_ADC_Init();
	/*Inicializa rel ADC en el canal correcto, a 16000 muestras por segundo*/
	Chip_ADC_Init(_LPC_ADC_ID, &ADCSetup);
	ADCSetup.burstMode = false;
	Chip_ADC_EnableChannel(_LPC_ADC_ID, _ADC_CHANNEL, ENABLE);
	Chip_ADC_SetResolution(_LPC_ADC_ID, &ADCSetup, ADC_10BITS);
	Chip_ADC_SetSampleRate(_LPC_ADC_ID, &ADCSetup, 16000*ADC_PROMEDIO);
	
	/*Habilitar el modo con interrupciones */
	NVIC_EnableIRQ(_LPC_ADC_IRQ);
	Chip_ADC_Int_SetChannelCmd(_LPC_ADC_ID, _ADC_CHANNEL, ENABLE);
	
	/*Habilitar el modo burst (conversiones una a continuación de la otra, a la tasa mencionada)*/
	Chip_ADC_SetBurstCmd(_LPC_ADC_ID, ENABLE);
	return 0;
}

int ADCSuspender(int apagar){
	if(apagar){
		NVIC_DisableIRQ(_LPC_ADC_IRQ);
	}else{
		NVIC_EnableIRQ(_LPC_ADC_IRQ);
	}
}

__RODATA(Flash2) const char pruebaBanco2[] = "PruebaBanco2";
int main() {
	/*Inicializaciones de la placa*/
	SystemCoreClockUpdate();
	Board_Init();
	
	/*Inicialización del período del systick*/
	SysTick_Config(SystemCoreClock / TICKRATE_HZ);
	
	/*Inicializar el conversor analógico-digital*/
	ADCInit();
	
	printf("Test reconocimiento de palabras (baremetal)\r\nGonzalo Avila Alterach 2016\r\n");
	if(strncmp(pruebaBanco2, "PruebaBanco2", 12) != 0){
		printf("Banco 2 programado incorrectamente\r\n");
		while(1);
	}
	
	const char *palabrasAReconocer[] = {"cero", "uno", "dos", "tres", "cuatro", "cinco", "seis", "siete", "ocho", "nueve"};
	
	const int tamanioLista = 3;
	blaListado listado[tamanioLista];
			
	if(blaInicializarBiblioteca(palabrasAReconocer, sizeof (palabrasAReconocer) / sizeof (*palabrasAReconocer)) != BLA_OK){
		printf("Error inicializando biblioteca: %s\r\n", blaLeerError());
	}else{
		printf("Inicializacion OK\r\n");
		
	}

	Board_LED_Set(3,1);
	Board_LED_Set(4,1);
	Board_LED_Set(5,1);
	
	while (1) {

		/*printf("Energia: %lld\r\n", blaLeerAcumuladorEnergia());*/

		int ret = blaDeteccionPeriodica();
		
		if(ret == BLA_COMIENZO_PALABRA){
			Board_LED_Set(1,0);
		}
		
		if(ret == BLA_FIN_PALABRA){
			Board_LED_Set(1,1);
			Board_LED_Set(0,0);
			
			/*Cortamos las interrupciones del ADC*/
			ADCSuspender(1);
			ret = blaReconocer(listado, tamanioLista);
			ADCSuspender(0);
			
			Board_LED_Set(0,1);
						
			if(ret != BLA_OK){
				printf("Error reconociendo: %s\r\n", blaLeerError());
			}else{
				int i;
				printf("Palabra\tProbabilidad\r\n");
				for(i=0;i<tamanioLista;i++){
					printf("%s\t%d\r\n", palabrasAReconocer[listado[i].idPalabra], (int)listado[i].probabilidad);
					
				}
				printf("----------------------\r\n");
			}
		}
		delay(10);
	}
}
