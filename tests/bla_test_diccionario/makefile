BLA_INC = ../../bla/inc
BLA_SRC = ../../bla/src

all: bla_test_diccionario.exe

bla_test_diccionario.exe: bla_test_diccionario.cpp $(BLA_SRC)/bla_pronunciacion.c
	g++ $^ -I$(BLA_INC) -o $@ -O2 -Wall 
	
.PHONY: run clean

run:
	bla_test_diccionario.exe

clean:
	del bla_test_diccionario.exe
	
	