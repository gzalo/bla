/*
	Bla_Pronunciacion:
		Convierte una palabra en su pronunciación: string con fonemas separados por espacios
 
		Varias reglas de pronunciación fueron obtenidas mediante prueba y error, puede que
		algunas palabras fallen. Se testeó con un diccionario generado por el SRI con 38000 palabras
		sin ningún problema (exceptuando palabras extranjeras o pronunciaciones de letras)
		
		Gracias Patricia Pelle por contestar dudas!
*/

#include <bla.h>
#include <bla_pronunciacion.h>
#include <bla_modelo.h>
#include <stdbool.h>
#include <limits.h>

#if CHAR_MAX > 127
	/*Los chars no tienen signo (CORTEX M4, por ej)*/
	#define ATILDE (256-31)
	#define ETILDE (256-23)
	#define ITILDE (256-19)
	#define OTILDE (256-13)
	#define UTILDE (256-6)
	#define ENIE (256-15)
	#define UDIER (256-4)
#else
	/*Los chars tienen signo (IA32, por ej)*/
	#define ATILDE (-31)
	#define ETILDE (-23)
	#define ITILDE (-19)
	#define OTILDE (-13)
	#define UTILDE (-6)
	#define ENIE (-15)
	#define UDIER (-4)
#endif

static bool esVocal(char v){
	if(v == 'a' || v == 'e' || v == 'i' || v == 'o' || v == 'u' || 
	v == ATILDE || v == ETILDE || v == ITILDE || v == OTILDE || v == UTILDE)
		return true;
	
	return false;
}

static bool esVocalAbierta(char v){
	if(v == 'a' || v == 'e' || v == 'o' ||
	v == ATILDE || v == ETILDE || v == OTILDE)
		return true;
	
	return false;
}

static bool esAOU(char v){
	if(v == 'a' || v == 'u' || v == 'o' ||
	v == ATILDE || v == UTILDE || v == OTILDE)
		return true;
	
	return false;
}

int blaPronunciacion(const char *palabra, int *pronunciacion, const int largoBuffer, int *cantidadFonemas){
	char anterior = ' ';
	unsigned int i=0;
	int pronIdx = 0;
	
	/*Macro que agrega una letra al final del vector
	  Chequea que no nos pasemos del final del buffer
	  No es muy elegante como código pero corre una sola vez, por lo que no importa :P*/
	#define AGREGAR_FONEMA(x) {pronunciacion[pronIdx++] = x; if(pronIdx>=largoBuffer){ pronunciacion[pronIdx-1]=BLA_FONEMA_SIL; return BLA_PALABRAS_LARGAS;}}	
	
	AGREGAR_FONEMA(BLA_FONEMA_SIL);
	
	while(palabra[i]){
		char siguiente = palabra[i+1];
		char sigsig = palabra[i+2];
		
		if(siguiente == 0) 
			siguiente = ' ';
		if(sigsig == 0) 
			sigsig = ' ';
		

		
		switch(palabra[i]){
			case 'a': case ATILDE: 
				AGREGAR_FONEMA(BLA_FONEMA_AA);
			break;
			case 'b': {
				if(anterior == 'm' || anterior == 'n' || anterior == ' ' || siguiente == 'v'){
					AGREGAR_FONEMA(BLA_FONEMA_B);
				}else{
					AGREGAR_FONEMA(BLA_FONEMA_BB);
				}
			}break;
			case 'j':{
				AGREGAR_FONEMA(BLA_FONEMA_HH);
			}break;
			case 'o': case OTILDE: {
				AGREGAR_FONEMA(BLA_FONEMA_OW);

				if(siguiente == 'u' && sigsig == 'r'){ /*couri*/
					i++;
					AGREGAR_FONEMA(BLA_FONEMA_W);

				}
			}break;
			case 'n':{
				if(siguiente == 'n') /*Bonn*/
					i++;
				AGREGAR_FONEMA(BLA_FONEMA_N);
			}break;
			case 'd':{
				if(siguiente == 'd') /*eddy*/
					i++;
					
				if(anterior == ' ' || anterior == 'l' || anterior == 'n' || siguiente == 'd'){
					AGREGAR_FONEMA(BLA_FONEMA_D);
				}else{
					AGREGAR_FONEMA(BLA_FONEMA_DD);
				}
			}break;
			case 'r':{
				if(anterior == ' '){
					AGREGAR_FONEMA(BLA_FONEMA_RR);
				}else if(siguiente == 'r'){
					AGREGAR_FONEMA(BLA_FONEMA_RR);
					i++;
				}else{
					AGREGAR_FONEMA(BLA_FONEMA_RX);
				}
			}break;
			case 'c': {
				if(siguiente == 'h'){
					AGREGAR_FONEMA(BLA_FONEMA_CH);
				}else if(siguiente == 'c'){
					AGREGAR_FONEMA(BLA_FONEMA_K);
					AGREGAR_FONEMA(BLA_FONEMA_S);
					i++;
				}else if(siguiente == 'y'){ /*cyprus*/
					AGREGAR_FONEMA(BLA_FONEMA_K);
					AGREGAR_FONEMA(BLA_FONEMA_IY);

					i++;
				}else{
					if(siguiente == 'e' || siguiente == ETILDE ||
					siguiente == 'i' || siguiente == ITILDE){
						AGREGAR_FONEMA(BLA_FONEMA_S);
					}else{
						AGREGAR_FONEMA(BLA_FONEMA_K);
					}
				}
			}break;
			case 'k':{
				AGREGAR_FONEMA(BLA_FONEMA_K);
			}break;
			case 's':{
				if(siguiente == 'c'){
					AGREGAR_FONEMA(BLA_FONEMA_S);
						
					if(sigsig != 'r' && sigsig != 'o' && sigsig != 'a' && sigsig != 'u' && 
						sigsig != ATILDE && sigsig != UTILDE && sigsig != OTILDE && sigsig != 'l') 
						/*inSCRipción, aSCO, ataSCAdo, eSCLarecer*/
						i++;
				}else if(siguiente == 's'){
					AGREGAR_FONEMA(BLA_FONEMA_S);
					i++;
				}else{
					/*ISA -> S
					  ESG -> Z
					  ESU -> S
					  ASB -> Z
					  abcdefghijklmnopqrstuvwxyz
					  x   xx xx     xxx  xxx*/
					if(
						(anterior == OTILDE && siguiente == 'm') || 
						siguiente == 't' || siguiente == 'i' || 
						siguiente == ' ' || siguiente == 'o' || 
						siguiente == 'a' || siguiente == ATILDE ||
						siguiente == 'u' || siguiente == 'p' || 
						siguiente == 'e' || siguiente == OTILDE || 
						siguiente == ETILDE || siguiente == 'f'|| 
						siguiente == 'q' || siguiente == ITILDE || 
						siguiente == 'h' || siguiente == 'v' || 
						siguiente == 'y' || siguiente == UTILDE ){
						AGREGAR_FONEMA(BLA_FONEMA_S);
					}else{
						AGREGAR_FONEMA(BLA_FONEMA_Z);
					}
				
				}
			}break;
			case 't':{
				if(siguiente == 't') /*betty*/
					i++;
				
				AGREGAR_FONEMA(BLA_FONEMA_T);
			}break;
			case 'e': case ETILDE: {
				AGREGAR_FONEMA(BLA_FONEMA_EY);
			}break;
			case 'i':{
				/*VID -> IY*/
				if(esVocal(siguiente) || esVocalAbierta(anterior)){
					AGREGAR_FONEMA(BLA_FONEMA_Y);
				}else{
					AGREGAR_FONEMA(BLA_FONEMA_IY);
				}
			}break;
			case ITILDE:{
				if(siguiente == 'e' && esVocalAbierta(anterior)){
					AGREGAR_FONEMA(BLA_FONEMA_Y);
				}else{
					AGREGAR_FONEMA(BLA_FONEMA_IY);
				}
			}break;
			case 'm':{
				AGREGAR_FONEMA(BLA_FONEMA_M);
			}break;
			case 'y':{
				if(anterior == 't' || (anterior == 'n' && siguiente == ' ')){
					AGREGAR_FONEMA(BLA_FONEMA_IY);
				}else{
					AGREGAR_FONEMA(BLA_FONEMA_Y);
				}
			} break;
			case 'g': {
				/*ALERGIA -> HH
				  ALARGA -> GG*/
				if((anterior == ' ' && siguiente != 'e' && siguiente != 'i' && siguiente!= ETILDE && siguiente != ITILDE) || 
				(anterior == 'n' && esAOU(siguiente)) || 
				(anterior == 'l' && esAOU(siguiente)) || 
				(anterior == 's' && esAOU(siguiente)) ||	
				(anterior == 'r' && esAOU(siguiente)) || 				
				(anterior == 'z' && esAOU(siguiente)) ||
				(anterior == 'b' && esAOU(siguiente)) ||
				
				siguiente == 'm' || siguiente == 'd' ||
				siguiente == ' ' || anterior == 'z' ||
				anterior == 'f' || siguiente == 'l' || 
				siguiente == 'n' || siguiente == 'r' || 
				siguiente == UDIER  
				
				) {
					AGREGAR_FONEMA(BLA_FONEMA_G);
				}else if(siguiente == 'i' || siguiente == 'e' || siguiente == ITILDE || siguiente == ETILDE){
					AGREGAR_FONEMA(BLA_FONEMA_HH);
				}else{
					AGREGAR_FONEMA(BLA_FONEMA_GG);
				}
			}break;	
			case 'l':{
				if(siguiente == 'l'){
					AGREGAR_FONEMA(BLA_FONEMA_Y);
					i++;
				}else{
					AGREGAR_FONEMA(BLA_FONEMA_L);
				}
			} break;
			case UDIER:{
				AGREGAR_FONEMA(BLA_FONEMA_W);
			}break;
			case 'u':{
				if(anterior == 'g'){
					/*GUA GUO -> 'w'
					 GUE GUI -> mudas*/
					if(siguiente == 'a' || siguiente == 'o' || siguiente == 'u'){
						AGREGAR_FONEMA(BLA_FONEMA_W);
					}else if(siguiente != 'e' && siguiente != 'i' && siguiente != ETILDE && siguiente != ITILDE){
						AGREGAR_FONEMA(BLA_FONEMA_UW);
					}
				}else if(anterior == 'q'){
					
				}else{
					/*AUD*/
						
					if(esVocal(siguiente) || esVocalAbierta(anterior)){
						AGREGAR_FONEMA(BLA_FONEMA_W);
					}else{
						AGREGAR_FONEMA(BLA_FONEMA_UW);
					}
				}
			} break;
			case UTILDE:{
				AGREGAR_FONEMA(BLA_FONEMA_UW);				
			} break;
			case 'h':{
			} break;
			case 'q': {
				AGREGAR_FONEMA(BLA_FONEMA_K);	
			}break;
			case 'p': {
				AGREGAR_FONEMA(BLA_FONEMA_P);	
			}break;
			case 'w': {
				AGREGAR_FONEMA(BLA_FONEMA_W);	
			}break;
			case 'f': {
				if(siguiente == 'f') /*ruffo*/
					i++;
				AGREGAR_FONEMA(BLA_FONEMA_F);	
			}break;
			case 'z':{
				AGREGAR_FONEMA(BLA_FONEMA_S);	
			}break;
			case 'x': {

				AGREGAR_FONEMA(BLA_FONEMA_K);	
				AGREGAR_FONEMA(BLA_FONEMA_S);	
				
				if(siguiente == 'c' && sigsig != 'a') /*eXCede*/
					i++;
			}break;
			case ENIE:{
				AGREGAR_FONEMA(BLA_FONEMA_NY);	
			}break;
			case 'v': {
				if(anterior == 'n' || anterior == ' '){
					AGREGAR_FONEMA(BLA_FONEMA_B);	
				}else{
					AGREGAR_FONEMA(BLA_FONEMA_BB);	
				}
			}break;
			default:
				/* palabra[i] no cae en ningún caso particular */
				return BLA_ERROR_PRONUNCIACION;
				break;
				
		}
		anterior = palabra[i];
		i++;
	}
	/*Agregamos el Fonema Final (SIL)*/
	pronunciacion[pronIdx] = BLA_FONEMA_SIL;
	
	if(cantidadFonemas)
		*cantidadFonemas = pronIdx+1;
	
	return BLA_OK;
}

static const char *blaNombreFonema(int id){
	switch(id){
		case BLA_FONEMA_B: return "b";
		case BLA_FONEMA_D: return "d";
		case BLA_FONEMA_F:return "f";
		case BLA_FONEMA_G:return "g";
		case BLA_FONEMA_K:return "k";
		case BLA_FONEMA_L:return "l";
		case BLA_FONEMA_M:return "m";
		case BLA_FONEMA_N:return "n";
		case BLA_FONEMA_P:return "p";
		case BLA_FONEMA_S:return "s";
		case BLA_FONEMA_T:return "t";
		case BLA_FONEMA_W:return "w";
		case BLA_FONEMA_Y:return "y";
		case BLA_FONEMA_Z:return "z";
		case BLA_FONEMA_AA:return "aa";
		case BLA_FONEMA_BB:return "bb";
		case BLA_FONEMA_CH:return "ch";
		case BLA_FONEMA_DD:return "dd";
		case BLA_FONEMA_EY:return "ey";
		case BLA_FONEMA_GG:return "gg";
		case BLA_FONEMA_HH:return "hh";
		case BLA_FONEMA_IY:return "iy";
		case BLA_FONEMA_NY:return "ny";
		case BLA_FONEMA_OW:return "ow";
		case BLA_FONEMA_RR:return "rr";
		case BLA_FONEMA_RX:return "rx";
		case BLA_FONEMA_UW:return "uw";
		case BLA_FONEMA_SIL:return "";
	}
	return "";
}

#include <stdio.h>

int blaImprimirPronunciacion(int *pron, int largo){
	for(int i=0;i<largo;i++)
		printf("%s ", blaNombreFonema(pron[i]));
	printf("\r\n");
}