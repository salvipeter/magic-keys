#!/bin/sh

if [ $# -ne 2 ] ; then
    echo "Usage: $0 <Neptun1> <Neptun2>"
    exit 1
fi

FLAGS="-std=c11 -Wall -pedantic -g -O0 -fsanitize=address"

sed "s/NEPTUN1/$1/g" magic-keys.c | sed "s/NEPTUN2/$2/g" > tmp.c
if [ "$1" = "$2" ]; then
    gcc -o magic-keys $FLAGS tmp.c init_$1.c
else
    gcc -o magic-keys $FLAGS tmp.c init_$1.c init_$2.c
fi    
rm tmp.c
