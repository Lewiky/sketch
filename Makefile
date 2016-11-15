# Edit this if your computer has things set up differently.

sketch-program:
	gcc -std=c99 -Wall -O3 -pedantic sketch.c display.c -lSDL2 -o sketch
