BLA_INC = ../../bla/inc
BLA_SRC_FOLDER = ../../bla/src
BLA_SRC=$(wildcard $(BLA_SRC_FOLDER)/*.c) 
CMSIS_DSP_FOLDER=../../ejemplos/CMSIS_DSPLIB_SRC
CMSIS_DSP_LIB=$(CMSIS_DSP_FOLDER)/lib/libcmsisdsp_x86.a
CMSIS_DSP_INC=$(CMSIS_DSP_FOLDER)/inc/
CFLAGS = -O2 -Wall -Wextra -Wno-implicit-function-declaration -Wno-unused-function -pedantic -Wall -std=c99 -Wshadow -Wpointer-arith

all: bla_test_coeficientes.exe

bla_test_coeficientes.exe: bla_test_coeficientes.c $(CMSIS_DSP_LIB)
	gcc $< $(BLA_SRC) -o $@ -I$(BLA_INC) -I$(CMSIS_DSP_INC) $(CFLAGS) -L$(CMSIS_DSP_FOLDER)/lib -lcmsisdsp_x86 -DBLA_ADC_RANGO=32767

%.o: %.c
	gcc -c $< -O2 -o $@ -I$(CMSIS_DSP_INC) $(CMSIS_GCC_FLAGS)
	
.PHONY: run clean	
	
run:
	bla_test_coeficientes
	
clean:
	del bla_test_coeficientes.exe