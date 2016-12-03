/*	
	gen_hamming	
		Genera los coeficientes de una ventana de Hamming de largo BLA_LARGOVENTANA
*/

#include <iostream>
#include <cmath>
using namespace std;

#define BLA_LARGOVENTANA (400)

int main(int argc, char **args){
	cout << "static const float blaHamming[BLA_LARGOVENTANA] = {";
	for(int i=0;i<BLA_LARGOVENTANA;i++){
		float arg = 2.0f*M_PI*i/(BLA_LARGOVENTANA-1);
		float coef = 0.54f-0.46f*cos(arg);
		
		if(i != BLA_LARGOVENTANA-1){
			cout << coef << ",";
		}else{
			cout << coef << "};";
		}
	}

	return 0;
}
