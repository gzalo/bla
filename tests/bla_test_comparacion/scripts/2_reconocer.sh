#!/bin/sh

RUTA_HTK="/home/gzalo/htk/HTKTools"

#Genera el grafo con las conexiones a partir de la gramática en formato EBNF
$RUTA_HTK/HParse ../config/gramatica ../config/wdnet

#Reconocer usando el algoritmo de Viterbi
$RUTA_HTK/HVite -A -D -T 1 -l '*'  -o STWM -C ../config/config.common -w ../config/wdnet -i ../config/resultados -S ../datos/lista_mfcs -H ../config/macros_sin_cmn -H ../config/hmmdefs_sin_cmn  ../config/lexicon ../config/fonemas
#-p 0 -s 5.0 no cambia nada

#Mostrar los resultados, comparando con las transcripcciones
$RUTA_HTK/HResults -A -D -T 1 -f -t -I ../config/prompts.mlf ../config/fonemas ../config/resultados
