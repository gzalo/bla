/*
	ADVERTENCIA: NO FUNCIONA	
		y además requiere modificaciones varias a CIAA-Firmware para agregar la biblioteca BLA y CMSIS-DSP!
*/

/* Copyright 2014, 2015, 2016 Mariano Cerdeiro
 * Copyright 2014, Gustavo Muro
 * Copyright 2014, Pablo Ridolfi
 * Copyright 2014, Juan Cecconi
 * Copyright 2014, Fernando Beunza
 * Copyright 2016, Gonzalo Ávila Alterach
 * All rights reserved.
 *
 * This file is part of CIAA Firmware.
 *
 * Redistribution and use in source and binary forms, with or without
 * modification, are permitted provided that the following conditions are met:
 *
 * 1. Redistributions of source code must retain the above copyright notice,
 *    this list of conditions and the following disclaimer.
 *
 * 2. Redistributions in binary form must reproduce the above copyright notice,
 *    this list of conditions and the following disclaimer in the documentation
 *    and/or other materials provided with the distribution.
 *
 * 3. Neither the name of the copyright holder nor the names of its
 *    contributors may be used to endorse or promote products derived from this
 *    software without specific prior written permission.
 *
 * THIS SOFTWARE IS PROVIDED BY THE COPYRIGHT HOLDERS AND CONTRIBUTORS "AS IS"
 * AND ANY EXPRESS OR IMPLIED WARRANTIES, INCLUDING, BUT NOT LIMITED TO, THE
 * IMPLIED WARRANTIES OF MERCHANTABILITY AND FITNESS FOR A PARTICULAR PURPOSE
 * ARE DISCLAIMED. IN NO EVENT SHALL THE COPYRIGHT HOLDER OR CONTRIBUTORS BE
 * LIABLE FOR ANY DIRECT, INDIRECT, INCIDENTAL, SPECIAL, EXEMPLARY, OR
 * CONSEQUENTIAL DAMAGES (INCLUDING, BUT NOT LIMITED TO, PROCUREMENT OF
 * SUBSTITUTE GOODS OR SERVICES; LOSS OF USE, DATA, OR PROFITS; OR BUSINESS
 * INTERRUPTION) HOWEVER CAUSED AND ON ANY THEORY OF LIABILITY, WHETHER IN
 * CONTRACT, STRICT LIABILITY, OR TORT (INCLUDING NEGLIGENCE OR OTHERWISE)
 * ARISING IN ANY WAY OUT OF THE USE OF THIS SOFTWARE, EVEN IF ADVISED OF THE
 * POSSIBILITY OF SUCH DAMAGE.
 *
 */

/** \brief Ejemplo de uso de biblioteca BLA
 **
 ** Conectar señal de micrófono ya amplificada a ADC_CH1 
 ** (pin 13 del conector P1 de la EDU-CIAA-NXP)
 ** Mensajes de debug enviados a través de la uart principal via USB 
 **
 **/

/*
 * Initials     Name
 * ---------------------------
 * gzalo        Gonzalo Ávila Alterach
 *
 */

/*
 * modification history (new versions first)
 * -----------------------------------------------------------
 * 20161115 v0.1 gzalo primera versión funcional
 */

/*==================[inclusions]=============================================*/
#include "os.h"
#include "ciaaPOSIX_stdio.h"
#include "ciaaPOSIX_stdlib.h"
#include "ciaak.h"            /* <= ciaa kernel header */
#include "bla_app.h"
#include "bla.h"

/*==================[macros and definitions]=================================*/

#define ADC_PROMEDIO (8)

/*==================[internal data declaration]==============================*/

/*==================[internal functions declaration]=========================*/

/*==================[internal data definition]===============================*/

/** \brief File descriptor for ADC
 *
 * Device path /dev/serial/aio/in/0
 */
static int32_t fd_adc;

/*==================[external data definition]===============================*/

/*==================[internal functions definition]==========================*/

/*==================[external functions definition]==========================*/
/** \brief Main function
 *
 * This is the main entry point of the software.
 *
 * \returns 0
 *
 * \remarks This function never returns. Return value is only to avoid compiler
 *          warnings or errors.
 */
int main(void)
{
   /* Starts the operating system in the Application Mode 1 */
   /* This example has only one Application Mode */
   StartOS(AppMode1);

   /* StartOs shall never returns, but to avoid compiler warnings or errors
    * 0 is returned */
   return 0;
}

/** \brief Error Hook function
 *
 * This fucntion is called from the os if an os interface (API) returns an
 * error. Is for debugging proposes. If called this function triggers a
 * ShutdownOs which ends in a while(1).
 *
 * The values:
 *    OSErrorGetServiceId
 *    OSErrorGetParam1
 *    OSErrorGetParam2
 *    OSErrorGetParam3
 *    OSErrorGetRet
 *
 * will provide you the interface, the input parameters and the returned value.
 * For more details see the OSEK specification:
 * http://portal.osek-vdx.org/files/pdf/specs/os223.pdf
 *
 */
void ErrorHook(void)
{
   ciaaPOSIX_printf("ErrorHook was called\n");
   ciaaPOSIX_printf("Service: %d, P1: %d, P2: %d, P3: %d, RET: %d\n", OSErrorGetServiceId(), OSErrorGetParam1(), OSErrorGetParam2(), OSErrorGetParam3(), OSErrorGetRet());
   ShutdownOS(0);
}

/** \brief Tarea inicial (AppMode1)
 */
TASK(InitTask)
{
   /* init the ciaa kernel */
   ciaak_start();

   /* inicializar biblioteca BLA */
   const char *palabrasAReconocer[] = {"cero", "uno", "dos", "tres", "cuatro", "cinco", "seis", "siete", "ocho", "nueve"};

   if(blaInicializarBiblioteca(palabrasAReconocer, sizeof (palabrasAReconocer) / sizeof (*palabrasAReconocer)) != BLA_OK){
      ciaaPOSIX_printf("Error inicializando biblioteca: %s\n", blaLeerError());
   }   
   
   /* open CIAA ADC */
   fd_adc = ciaaPOSIX_open("/dev/serial/aio/in/0", ciaaPOSIX_O_RDONLY);
   ciaaPOSIX_ioctl(fd_adc, ciaaPOSIX_IOCTL_SET_SAMPLE_RATE, 16000*);
   ciaaPOSIX_ioctl(fd_adc, ciaaPOSIX_IOCTL_SET_CHANNEL, ciaaCHANNEL_1);

   /* end InitTask */
   TerminateTask();
}

/** \brief Read ADC values and send them to BLA
 *
 * This task is activated every 1 ms by the AnalogicAlarm configured in
 * bla_app.oil
 */
TASK(Analogic)
{
   int32_t count;
   uint16_t hr_ciaaAdc[128];

   /* Read ADC. */
   count = ciaaPOSIX_read(fd_adc, &hr_ciaaAdc, sizeof(hr_ciaaAdc));

   if (count > 0)
   {
      int i;
      for(i=0;i<count;i++)
         blaAlmacenarValorMicrofono(hr_ciaaAdc-512);	
   }

   /* end of task */
   TerminateTask();
}

/** \brief Check if a word has ended
 *
 * This task is activated every 100 ms by the AnalogicAlarm configured in
 * bla_app.oil
 */
TASK(Recognition)
{
   int ret = blaDeteccionPeriodica();

   if(ret == BLA_FINPALABRA){
      const int tamanioLista = 3;
      blaListado listado[tamanioLista];

      ret = blaReconocer(listado, tamanioLista);

      if(ret != BLA_OK){
         ciaaPOSIX_printf("Error reconociendo: %s\n", blaLeerError());
      }else{
         int i;
         ciaaPOSIX_printf("Palabra\tProbabilidad\n");
         for(i=0;i<tamanioLista;i++){
            ciaaPOSIX_printf("%s\t%f\n", palabrasAReconocer[listado[i].idPalabra], listado[i].probabilidad);
         }
         ciaaPOSIX_printf("----------------------\n");
      }
  
    }
   TerminateTask();
}


/*==================[end of file]============================================*/

