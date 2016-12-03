/*	
	gen_tcoseno
		Genera los coeficientes necesarios para la transformada coseno discreta tipo 2
		que es necesaria para calcular coeficientes MFCC
*/


#include <iostream>
#include <cmath>
using namespace std;

#define BLA_LARGOVENTANA (400)

int main(int argc, char **args){
	#define BLA_NUMCHANS (26)
	#define BLA_NUMCEPS (12)
	
	cout << "static const float blaDCT[BLA_NUMCEPS][BLA_NUMCHANS] = {";

	for(int i=0;i<BLA_NUMCEPS;i++){
		cout << "{";
		for(int j=0;j<BLA_NUMCHANS;j++){
			/*El +0.5f originalmente era -0.5f, 
			  debido a que los indices van desde 0 a BLA_NUMCHANS-1, es necesario sumarle 1 al calculo de la DCT*/
			cout << cos(M_PI*(float)(i+1)/BLA_NUMCHANS * ((float)j+0.5f)) * sqrt(2.0f/BLA_NUMCHANS);
			if(j!=BLA_NUMCHANS-1)
				cout << ",";
		}		
		cout << "}" << endl;
		if(i!=BLA_NUMCEPS-1)
			cout << ",";
	}
	cout << "}" << endl;

	return 0;
}
