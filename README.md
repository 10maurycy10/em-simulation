# Toy electro magnetic simulations

This is a toy simulation of maxwell's equations in 2 dimentions, using the FDFT method.

## Compiling

This requires the SDL2 library and gcc.

Under linux, use the build script `./build.sh` that takes a file in `./sims` as an argument and creates a binary called `embox`

(I should probobly set up a proper build system sometime)

## Current Simulations

- `./sims/doubleslit.c` This is the clasic double slit experiment.
