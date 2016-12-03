/*ARCHIVO GENERADO AUTOMATICAMENTE - NO EDITAR*/
#include <stdint.h>
#include <bla.h>
#ifndef BLA_MODELO_H
#define BLA_MODELO_H
#define NUMERODIMENSIONES (39)
#define NUMEROMEZCLAS (16)
#define NUMEROESTADOS (3)
#define BLA_INV_VARIANZA
	typedef struct {
		float medias[NUMERODIMENSIONES];
		float varianzas[NUMERODIMENSIONES];
		float constante;
		float logPeso;
	}blaMezcla_t;
	
	typedef struct{
		uint8_t cantidadMezclas;
		blaMezcla_t mezclas[NUMEROMEZCLAS];
	}blaEstado_t;
	
	typedef struct {
		char nombre[4];
		uint8_t cantidadEstados;
		blaEstado_t estados[NUMEROESTADOS];
		float logProbMismo[NUMEROESTADOS];
		float logProbTrans[NUMEROESTADOS];
		const float *probabilidadExtendida;
	}blaFonema_t;
	
	extern const blaFonema_t blaFonemas[];
	extern const uint32_t blaCantidadFonemas;
	extern const float blaProbExtendida[9];

enum{
	BLA_FONEMA_B=0,
	BLA_FONEMA_D,
	BLA_FONEMA_F,
	BLA_FONEMA_G,
	BLA_FONEMA_K,
	BLA_FONEMA_L,
	BLA_FONEMA_M,
	BLA_FONEMA_N,
	BLA_FONEMA_P,
	BLA_FONEMA_S,
	BLA_FONEMA_T,
	BLA_FONEMA_W,
	BLA_FONEMA_Y,
	BLA_FONEMA_Z,
	BLA_FONEMA_AA,
	BLA_FONEMA_BB,
	BLA_FONEMA_CH,
	BLA_FONEMA_DD,
	BLA_FONEMA_EY,
	BLA_FONEMA_GG,
	BLA_FONEMA_HH,
	BLA_FONEMA_IY,
	BLA_FONEMA_NY,
	BLA_FONEMA_OW,
	BLA_FONEMA_RR,
	BLA_FONEMA_RX,
	BLA_FONEMA_UW,
	BLA_FONEMA_SIL
};

#endif
