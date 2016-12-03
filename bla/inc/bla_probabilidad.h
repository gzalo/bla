#include <bla_modelo.h>
#ifndef BLA_PROBABILIDAD
#define BLA_PROBABILIDAD

int blaProbabilidadInicializar(void);
float blaProbabilidadMezclaGaussiana(const blaEstado_t *estado, float *x);

/* Inspirado en Hmodel.c y Hmath.c de la biblioteca HTK */
#define LZERO  (-1.0E10)   /* Aproximadamente log(0) */
#define LSMALL (-0.5E10)   /* Tolerancia usada para redondear */
#define MINMIX  1.0E-5     /* Peso de mezcla mínimo */
#define LMINMIX -11.5129254649702 /* log(MINMIX) */

#endif
