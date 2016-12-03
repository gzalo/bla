/*
	Bla_Test_Modelo:
		Convierte de formato un modelo de HTK a un formato propio,
		en forma de un archivo .c que contiene únicamente constantes.
		
		De esta forma se logra que el modelo pueda ser compilado a memoria
		de código, y así evitar tener que cargarlo todo en RAM.
		
		Únicamente funciona para ciertos tipos de modelos (varianza diagonal)
		y mezclas de gaussianas. El código no es muy prolijo, pero funciona.
*/

#include <iostream>
#include <sstream>
#include <fstream>
#include <vector>
#include <cmath>
#include <map>
using namespace std;

struct blaMezcla_t{
	vector <float> medias;
	vector <float> varianzas;
	float constante;
	float peso;
};

struct blaEstado_t{
	vector <blaMezcla_t> mezclas;
};

struct blaFonema_t{
	string nombre;
	vector <blaEstado_t> estados;
	vector <float> probabilidadDiag;
};

string codigoExtra = "";

//Imprime un vector de floats como su representación en código
string stringVectorFloats(vector<float> vec){
	ostringstream out;
	out << "{";
	for(size_t i=0;i<vec.size();i++){
		//Esta operación NO le agrega la "f" al final de los números float que imprimimos
		//suponemos que el compilador los va a compilar como floats 
		out.precision(15);
		out << vec[i];
		if(i != vec.size()-1)
			out << ", ";
	}
	out << "}";
	return out.str();
}
//Imprime un vector de floats como su representación en código, invirtiendo (1/x) cada valor
string stringVectorFloatsInvertidos(vector<float> vec){
	ostringstream out;
	out << "{";
	for(size_t i=0;i<vec.size();i++){
		//Esta operación NO le agrega la "f" al final de los números float que imprimimos
		//suponemos que el compilador los va a compilar como floats 
		out.precision(15);
		out << 1.0/vec[i];
		if(i != vec.size()-1)
			out << ", ";
	}
	out << "}";
	return out.str();
}

//Imprime una estructura de mezcla como su representación en código
string stringMezclas(vector<blaMezcla_t> mezcla){
	ostringstream out;
	out << "\n";
	for(size_t i=0;i<mezcla.size();i++){
		out << "\t\t\t\t\t{\n";
		out << "\t\t\t\t\t\t.medias = " << stringVectorFloats(mezcla[i].medias) << ",\n";
		out << "\t\t\t\t\t\t.varianzas = " << stringVectorFloatsInvertidos(mezcla[i].varianzas) << ",\n";
		out << "\t\t\t\t\t\t.constante = " << mezcla[i].constante << ",\n";
		out << "\t\t\t\t\t\t.logPeso = " << log(mezcla[i].peso) << "\n";
		
		if(i != mezcla.size()-1){

		out << "\t\t\t\t\t},\n";
		}else{
			out << "\t\t\t\t\t}\n";
		}
	}
	return out.str();
}

//Imprime una estructura de estado como su representación en código
string stringEstados(vector<blaEstado_t> estados){
	ostringstream out;
	out << "\n";
	for(size_t i=0;i<estados.size();i++){
		out << "\t\t\t{\n";
		out << "\t\t\t\t.cantidadMezclas = " << estados[i].mezclas.size() << ",\n";
		out << "\t\t\t\t.mezclas = {" << stringMezclas(estados[i].mezclas) << "\t\t\t\t}\n";
		
		if(i != estados.size()-1){
			out << "\t\t\t},\n";
		}else{
			out << "\t\t\t}\n";
		}
	}
	out << "\t\t";
	return out.str();
}

//Imprime una estructura de fonema como su representación en código
string stringFonema(blaFonema_t fonema){
	ostringstream out;
	out << "\t{\n";
	out << "\t\t.nombre = \"" << fonema.nombre << "\",\n";
	out << "\t\t.cantidadEstados = " << fonema.estados.size() << ",\n";
	out << "\t\t.estados = {" << stringEstados(fonema.estados) << "} ,\n";
	if(fonema.probabilidadDiag.size() != 3){
		//out << "\t\t.probabilidadDiag = {0.0f,0.0f,0.0f},\n";
		out << "\t\t.probabilidadExtendida = blaProbExtendida\n";
		//Esto anda solo una vez, idealmente habria que ponerle un nombre aleatorio o secuencial a la variable
		codigoExtra += "const float blaProbExtendida[] = ";
		codigoExtra += stringVectorFloats(fonema.probabilidadDiag);
		codigoExtra += ";";
	}else{
		vector <float> logProbMismo, logProbTrans;
		logProbMismo.resize(fonema.probabilidadDiag.size());
		logProbTrans.resize(fonema.probabilidadDiag.size());
		for(size_t i=0;i<fonema.probabilidadDiag.size();i++){
			logProbMismo[i] = log(fonema.probabilidadDiag[i]);
			logProbTrans[i] = log(1.0f-fonema.probabilidadDiag[i]);
		}
		out << "\t\t.logProbMismo = " << stringVectorFloats(logProbMismo) << ",\n";
		out << "\t\t.logProbTrans = " << stringVectorFloats(logProbTrans) << ",\n";
		out << "\t\t.probabilidadExtendida = 0\n";
	}
	out << "\t}";
	return out.str();
}
//Imprime un vector de fonemas como su representación en código
string stringVectorFonemas(vector<blaFonema_t> fonemas){
	ostringstream out;
	out << "const uint32_t blaCantidadFonemas = " << fonemas.size() << ";\n";
	out << "__RODATA(Flash2) const blaFonema_t blaFonemas[] = {\n";
	for(size_t i=0;i<fonemas.size();i++){
		out << stringFonema(fonemas[i]);
		
		if(i != fonemas.size()-1){
			out << ",\n";
		}else{
			out << "\n";
		}
	}
	out << "};\n";
	
	return out.str();
}

std::string toupper(const std::string & s){
    std::string ret(s.size(), char());
    for(size_t i = 0; i < s.size(); ++i)
        ret[i] = (s[i] <= 'z' && s[i] >= 'a') ? s[i]-('a'-'A') : s[i];
    return ret;
}

//Imprime un emum de fonemas con sus nombres, en mayusculas, para poder indexar más facilmente
string stringEnumFonemas(vector<blaFonema_t> fonemas){
	ostringstream out;
	out << "enum{\n";
	for(size_t i=0;i<fonemas.size();i++){
		out << "\tBLA_FONEMA_" << toupper(fonemas[i].nombre);
		
		if(i == 0){
			out << "=0";
		}
		if(i != fonemas.size()-1){
			out << ",\n";
		}else{
			out << "\n";
		}
	}
	out << "};\n";
	
	return out.str();
}

int numeroDimensiones = 0, numeroMezclas = 0, numeroEstados = 0;

string imprimirH(){
	ostringstream out_h;
	out_h << "#define NUMERODIMENSIONES (" << numeroDimensiones << ")" << endl;
	out_h << "#define NUMEROMEZCLAS (" << numeroMezclas << ")" << endl;
	out_h << "#define NUMEROESTADOS (" << numeroEstados << ")" << endl;
	out_h << "#define BLA_INV_VARIANZA" << endl;
	out_h << "\
	typedef struct {\n\
		float medias[NUMERODIMENSIONES];\n\
		float varianzas[NUMERODIMENSIONES];\n\
		float constante;\n\
		float logPeso;\n\
	}blaMezcla_t;\n\
	\n\
	typedef struct{\n\
		uint8_t cantidadMezclas;\n\
		blaMezcla_t mezclas[NUMEROMEZCLAS];\n\
	}blaEstado_t;\n\
	\n\
	typedef struct {\n\
		char nombre[4];\n\
		uint8_t cantidadEstados;\n\
		blaEstado_t estados[NUMEROESTADOS];\n\
		float logProbMismo[NUMEROESTADOS];\n\
		float logProbTrans[NUMEROESTADOS];\n\
		const float *probabilidadExtendida;\n\
	}blaFonema_t;\n\
	\n\
	extern const blaFonema_t blaFonemas[];\n\
	extern const uint32_t blaCantidadFonemas;\n\
	extern const float blaProbExtendida[9];" << endl;
	
	return out_h.str();
}

//Parsea los bloques ~0
int parsearO(stringstream &in){
	string idx;
	bool formatoCorrecto = false;
	while(in >> idx){
		if(idx == "<STREAMINFO>"){
			int numeroStreams, numeroVectores;
			in >> numeroStreams >> numeroVectores;
			
			if(numeroStreams != 1){
				cerr << "Soporte para mas de 1 stream no implementado" << endl;
				return -1;
			}

		}
		if(idx == "<VECSIZE>"){
			in >> numeroDimensiones;	
			
		}
		if(idx == "<NULLD><MFCC_D_A_0><DIAGC>"){
			formatoCorrecto = true;
		}
	}
	if(formatoCorrecto){
		cout << "Parseado bloque de opciones" << endl;
		return 0;
	}else{
		cerr << "Formato de modelos incorrecto (no es D_A_0 y DIAGC)." << endl;
		return -1;
	}
	return 0;
}

vector<blaFonema_t> fonemas;

int parsearH(stringstream &in){
	string idx;
	
	blaFonema_t fonemaActual;
	string nombre;
	in >> nombre;
	
	//Saco las comillas (primer y último caracter)
	nombre = nombre.substr(1,nombre.size()-2);
	fonemaActual.nombre = nombre;
	
	int mezclaActual = 0;
	int estadoActual = 0;
	bool tieneBegin = false, tieneEnd = false;
	
	while(in >> idx){
		if(idx == "<NUMMIXES>"){
			in >> numeroMezclas;
						
			fonemaActual.estados[estadoActual].mezclas.resize(numeroMezclas);
			
		}else if(idx == "<MIXTURE>"){
			float mixProb;
			in >> mezclaActual >> mixProb;
			mezclaActual = mezclaActual-1;
			
			fonemaActual.estados[estadoActual].mezclas[mezclaActual].peso = mixProb;
			
		}else if(idx == "<MEAN>"){
			int cantidad;
			in >> cantidad;			
		
			if(cantidad > numeroDimensiones){
				cerr << "Mas dimensiones que las esperadas" << endl;
			}
		
			fonemaActual.estados[estadoActual].mezclas[mezclaActual].medias.resize(cantidad);
					
			for(int i=0;i<cantidad;i++){
				in >> fonemaActual.estados[estadoActual].mezclas[mezclaActual].medias[i];
			}
			
		}else if(idx == "<VARIANCE>"){
			int cantidad;
			in >> cantidad;				
			
			if(cantidad > numeroDimensiones){
				cerr << "Mas dimensiones que las esperadas" << endl;
			}
			
			fonemaActual.estados[estadoActual].mezclas[mezclaActual].varianzas.resize(cantidad);
			
			for(int i=0;i<cantidad;i++){
				in >> fonemaActual.estados[estadoActual].mezclas[mezclaActual].varianzas[i];
			}
		}else if(idx == "<GCONST>"){
		
			in >> fonemaActual.estados[estadoActual].mezclas[mezclaActual].constante;
													
		}else if(idx == "<TRANSP>"){
			int tamanio;
			in >> tamanio;
			
			float val[tamanio*tamanio];
			for(int i=0;i<tamanio*tamanio;i++)
				in >> val[i];

			//Fonemas convencionales de 3 estados
			if(nombre != "sil"){
				fonemaActual.probabilidadDiag.resize(tamanio-2);
				//Es suficiente tener los valores de la diagonal de la matriz de transición
				//para despejar los otros datos
				for(int i=0;i<tamanio-2;i++)
					fonemaActual.probabilidadDiag[i] = val[(i+1)*tamanio+(i+1)];
				
			}else{
				//El caso del fonema de silencio es como el siguiente ejemplo
				/*0.000000e+00 1.000000e+00 0.000000e+00 0.000000e+00 0.000000e+00
				0.000000e+00 8.103214e-01 7.387751e-02 1.158011e-01 0.000000e+00
				0.000000e+00 0.000000e+00 8.705046e-01 1.294954e-01 0.000000e+00
				0.000000e+00 3.576976e-02 0.000000e+00 9.458196e-01 1.841063e-02
				0.000000e+00 0.000000e+00 0.000000e+00 0.000000e+00 0.000000e+00*/
				//Hace falta guardar la matriz de 3x3 del centro
				//El que imprime la matriz se encarga de que entre en la misma estructura
				
				fonemaActual.probabilidadDiag.resize(3*3);
				int j=0;
				for(int y=0;y<3;y++)
					for(int x=0;x<3;x++,j++)
						fonemaActual.probabilidadDiag[j] = val[(y+1)*tamanio+(x+1)];
			}
			
		}else if(idx == "<BEGINHMM>"){
			tieneBegin = true;
		}else if(idx == "<ENDHMM>"){
			tieneEnd = true;
		}else if(idx == "<NUMSTATES>"){
			int cantidadEstados;
			in >> cantidadEstados;
			cantidadEstados -= 2;
			fonemaActual.estados.resize(cantidadEstados); //Remuevo el inicial y final (que no emiten ninguna salida)
			
			numeroEstados = cantidadEstados;
			
		}else if(idx == "<STATE>"){
			in >> estadoActual;	
			estadoActual = estadoActual-2; //Arrancan en 2 los ID de estados -> lo arreglo
		}else if(idx != ""){
			cout << "No parseado: " << idx << endl;
		}
	}
	
	if(!tieneBegin || !tieneEnd){
		cerr << "Bloque " << fonemaActual.nombre << " le falta BEGINHMM o ENDHMM" << endl;
		return -1;
	}
	
	if(nombre != "sp"){
		fonemas.push_back(fonemaActual);
		cout << "Parseado fonema " << fonemaActual.nombre << endl;
	}
	
	return 0;
}

int main(int argc, char **args){
	
	ifstream in("hmmdefs_16");
	ofstream out_c("../../bla/src/bla_modelo.c");
	ofstream out_h("../../bla/inc/bla_modelo.h");
	
	out_c << "/*ARCHIVO GENERADO AUTOMATICAMENTE - NO EDITAR*/" << endl;
	out_c << "#include <bla_modelo.h>" << endl;
	
	out_h << "/*ARCHIVO GENERADO AUTOMATICAMENTE - NO EDITAR*/" << endl;
	out_h << "#include <stdint.h>" << endl << "#include <bla.h>" << endl << "#ifndef BLA_MODELO_H" << endl << "#define BLA_MODELO_H" << endl;
			
	map <string, string> reemplazos;
	string acumulado;
	char nivel = ' ';
	
	while(!in.eof()){
		char idx;
		idx = in.get();
		
		if(idx == '~'){
			in >> nivel;
			
			if(nivel == 'h' || nivel == 'o'){
				char chr;
				bool fin = false;
				while(!fin){
					if(in.peek() == EOF){
						fin = true;
						continue;
					}
					if(in.peek() == '~'){
						in >> chr;
						if(in.peek() == 's'){
							//Reemplazo
							in >> chr; //'s'
							string nombreRemplazo;
							in >> nombreRemplazo;
							nombreRemplazo = nombreRemplazo.substr(1,nombreRemplazo.size()-2);
							
							if(reemplazos.count(nombreRemplazo) == 0){
								string acumulado_s = "";

								while(in.peek() != '~' && in.peek() != EOF){
									char chr = in.get();
									acumulado_s += chr;
								}
								
								cout << "Creado reemplazo " << nombreRemplazo << endl;
								reemplazos[nombreRemplazo] = acumulado_s;
							}else{
								cout << "Cargado reemplazo " << nombreRemplazo << endl;
								acumulado += reemplazos[nombreRemplazo];
							}
						}else{
							in.putback('~');
							fin = true;
							continue;
						}
					}
					chr = in.get();
					if(chr == '~'){
						in.putback('~');
					}else{
						acumulado += chr;
					}
				}
				stringstream istr(acumulado);
				if(nivel == 'o'){
					if(parsearO(istr) != 0)
						return -1;
				}else if(nivel == 'h'){
					if(parsearH(istr) != 0)
						return -1;
				}
				acumulado = "";	
			}else if(nivel != ' '){
				cerr << "Nivel incorrecto: " << nivel << endl;
				return -1;
			}			
		}	
	}
	
	string salida = stringVectorFonemas(fonemas);
	out_c << codigoExtra << endl;
	out_c << salida;
		
	out_h << imprimirH() << endl;	
	out_h << stringEnumFonemas(fonemas) << endl;
	out_h << "#endif" << endl;
	
	return 0;
}
