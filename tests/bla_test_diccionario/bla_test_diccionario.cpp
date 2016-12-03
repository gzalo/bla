/*
	Bla_Test_Dicc:
		Verifica que el pasaje de palabras a fonemas esté correcto,
		comprobándolo con un diccionario de pronunciaciones de prueba.
		
		Originalmente implementado usando un diccionario provisto por
		SRI (no libre), que no se incluye en el repositorio porque no 
		puede ser distribuido.
 
*/

#include <iostream>
#include <sstream>
#include <fstream>
using namespace std;

#define LARGOFIJO
#include <bla_pronunciacion.h>
#include <bla_modelo.h>

string nombreFonema(int id){
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

string pronunciar(const string &palabra){
	int pronReglas[30], cantidadFonemas;
	if(blaPronunciacion(palabra.c_str(), pronReglas, 30, &cantidadFonemas) != 0){
		cerr << "No se pudo pronunciar" << endl;
		return "";
	}
	
	string pronFinal;
	for(int i=1;i<cantidadFonemas-1;i++){ //Salteamos el primero y el último (SIL)
		
		pronFinal += nombreFonema(pronReglas[i]);
		if(i != cantidadFonemas-2)
			pronFinal += " ";
	}
	return pronFinal;
}

int main(int argc, char **args){
	
	ifstream in("diccionario.txt");
	cout << pronunciar("bilbo") << endl;
	cout << pronunciar("ciaa") << endl;
	cout << pronunciar("arwen") << endl;
	
	int palabrasTotales = 0, palabrasIncorrectas = 0;
	while(!in.eof()){
		palabrasTotales++;
		string todo;
		getline(in, todo);
		
		if(todo=="") break;
		string palabra = todo.substr(0,todo.find(" "));
		
		string pron = todo.substr(todo.rfind("  ")+2);
		
		//Casos atípicos que no siguen las reglas:
		//-Pronunciar una sola letra
		if(palabra.size() == 1) continue;
		
		//-Palabras extranjeras que no pronuncia correctamente
		if(palabra == "hardware") continue;
		if(palabra == "mexicano") continue;
		if(palabra == "mexicanos") continue;
		if(palabra == "larry") continue;
		if(palabra == "software") continue;
		
		string pronFinal = pronunciar(palabra);
		
		if(pron != pronFinal){
			palabrasIncorrectas++;
			if(palabrasIncorrectas == 1){
				cout << palabrasTotales << endl;
				cout << palabra << endl << pron << endl << pronFinal << endl;
			}
		}
	}
	
	cout << "Incorrectas: " << palabrasIncorrectas << " de " << palabrasTotales << " (" << (float)100.0f*palabrasIncorrectas/palabrasTotales << "%)" << endl;
	return 0;
}
