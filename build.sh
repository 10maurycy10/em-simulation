#!/bin/sh
gcc math.c render.c main.c -o embox -Wall -std=c99 -lSDL2 -lm -gdwarf -O3
