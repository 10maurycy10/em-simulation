#!/bin/sh
SIM=$1

[[ -z "$SIM" ]] && echo "Please pass a simulation (hint, check ./sims)"
[[ -z "$SIM" ]] && exit 1

gcc $SIM math.c render.c maxwell.c -o embox -Wall -std=c99 -lSDL2 -lm -gdwarf -O3 -lSDL2_image -fopenmp

exit 0
