#!/bin/sh
#Crea un master label file, con las "transcripciones" de cada wav
#Código original hecho por el grupo de Procesamiento del Habla?

#Directorio donde se encuentran los wav
dirbase="../datos/wav/"
#archivo de salida
archsalida="../config/prompts.mlf"
`ls $dirbase/*/*.wav > p`

echo "#!MLF!#" >> $archsalida

cat p |\
while read file 
do
    var=`basename ${file%.*}`
    a=`echo $var |cut -c 1,1`
    b=`stat -c %s $file`
    b1=`expr $b - 1024`
    c=`expr $b1 \\/ 2`
    case $a in
	1)
	    echo \"*/$var.lab\" >> $archsalida
	    echo "0 $c uno" >> $archsalida
	    echo . >> $archsalida
	    ;;
	2) 
	    echo \"*/$var.lab\" >> $archsalida
	    echo "0 $c dos" >> $archsalida
	    echo . >> $archsalida
	    ;;
	3) 
	    echo \"*/$var.lab\" >> $archsalida
	     echo "0 $c tres" >> $archsalida
	    echo . >> $archsalida
	    ;;
	4) 
	    echo \"*/$var.lab\" >> $archsalida
	    echo "0 $c cuatro" >> $archsalida
	    echo . >> $archsalida
	    ;;
	5) 
	    echo \"*/$var.lab\" >> $archsalida
	    echo "0 $c cinco" >> $archsalida
	    echo . >> $archsalida
	    ;;
	6) 
	    echo \"*/$var.lab\" >> $archsalida
	    echo "0 $c seis" >> $archsalida
	    echo . >> $archsalida
	    ;;
	7) 
	    echo \"*/$var.lab\" >> $archsalida
	    echo "0 $c siete" >> $archsalida
	    echo . >> $archsalida
	    ;;
	8) 
	    echo \"*/$var.lab\" >> $archsalida
	    echo "0 $c ocho" >> $archsalida
	    echo . >> $archsalida
	    ;;
	9) 
	    echo \"*/$var.lab\" >> $archsalida
	    echo "0 $c nueve" >> $archsalida
	    echo . >> $archsalida
	    ;;
	0) 
	    echo \"*/$var.lab\" >> $archsalida
	    echo "0 $c cero"  >> $archsalida
	    echo . >> $archsalida
	    ;;
    esac
done

rm p

