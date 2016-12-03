CMSIS_DSP_FOLDER=../../../ejemplos/CMSIS_DSPLIB_SRC
CMSIS_DSP_LIB=$(CMSIS_DSP_FOLDER)/lib/libcmsisdsp_x86.a
CMSIS_DSP_INC=$(CMSIS_DSP_FOLDER)/inc/

BLA_INC = ../../../bla/inc
BLA_SRC_FOLDER = ../../../bla/src
BLA_SRC=$(wildcard $(BLA_SRC_FOLDER)/*.c) 
 
CFLAGS = -O0 -Wall -Wextra -Wno-implicit-function-declaration -Wno-unused-function -std=c99 -pedantic -Wall -Wshadow -Wpointer-arith -fno-omit-frame-pointer -ggdb

#-march=native -mfpmath=sse -flto  -funroll-loops -ffast-math -pg

all: bla_test_comparacion.exe

bla_test_comparacion.exe: bla_test_comparacion.c $(CMSIS_DSP_LIB) $(BLA_SRC)
	gcc $< $(BLA_SRC) -o $@ -I$(BLA_INC) -I$(CMSIS_DSP_INC) $(CFLAGS) -L$(CMSIS_DSP_FOLDER)/lib -lcmsisdsp_x86 -DBLA_ADC_RANGO=32767

.PHONY: run clean	
	
run:
	bla_test_comparacion
	
clean:
	del bla_test_comparacion.exe
	