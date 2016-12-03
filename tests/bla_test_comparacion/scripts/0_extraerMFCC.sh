#!/bin/sh

#Genera la lista de wavs a procesar y los procesa con HCopy (extracción de parámetros MFCC)

#Ruta a los binarios del HTK
RUTA_HTK="/home/gzalo/htk/HTKTools"
 
find ../datos/wav/* -type f >p
cat p |sed 's/wav/mfc/g' >q
paste p q > ../datos/lista_conversion
rm p 
rm q

$RUTA_HTK/HCopy -T 1 -C ../config/config.hcopy -S ../datos/lista_conversion

find ../datos/mfc/* -type f > ../datos/lista_mfcs