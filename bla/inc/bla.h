/*
	BLA - Gonzalo Ávila Alterach (http://gzalo.com)
	Biblioteca liviana para realizar reconocimiento de habla 
	
		Funciones llamables por la aplicación del usuario
 
*/
#include <stdint.h>
#include <bla_config.h>

#ifndef BLA
#define BLA

/* Includes específicos a cada plataforma */
#ifdef __linux__
	/* Para evitar el problema en la definición del buffer y de los datos del modelo */
	#define __BSS(RAM2) 
	#define __RODATA(Flash2) 
#elif defined __MINGW32__
	/* Para evitar el problema en la definición del buffer y de los datos del modelo */
	#define __BSS(RAM2) 
	#define __RODATA(Flash2) 
#else
	#include <cr_section_macros.h> 
#endif

int blaInicializarBiblioteca(const char *palabrasAReconocer[], int cantidadPalabras);
int blaDeteccionPeriodica(void);

typedef struct{
	int idPalabra;
	float probabilidad;
}blaListado;

int blaReconocer(blaListado *listaProbabilidades, int cantidadLista);
int blaReconocerBuffer(blaListado *listaProbabilidades, int cantidadLista, const int16_t *buffer, int largoBuffer);

int blaAlmacenarValorMicrofono(int16_t valor);

uint64_t blaLeerAcumuladorEnergia(void);

const char *blaLeerError(void);

enum{
	BLA_OK = 0,
	BLA_FIN_PALABRA,
	BLA_COMIENZO_PALABRA,
	BLA_PALABRAS_LARGAS,
	BLA_LIMITE_PALABRAS,
	BLA_ERROR_PRONUNCIACION
};

extern float blaMFCCBloques[BLA_CANTIDAD_BLOQUES][BLA_NUMCEPS*3+3];

#endif
