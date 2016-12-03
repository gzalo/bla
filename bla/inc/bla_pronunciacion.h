/*
	Bla_Pronunciacion:
		Convierte una palabra en su pronunciación: string con fonemas separados por espacios
 
*/

#ifndef BLA_PRONUNCIACION
#define BLA_PRONUNCIACION

int blaPronunciacion(const char *palabra, int *pronunciacion, const int largoBuffer, int *cantidadFonemas);
int blaImprimirPronunciacion(int *pron, int largo);

#endif