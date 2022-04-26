CC = gcc
CFLAGS = -I ../libs/ -I ../sierpinski -I ../diamond_square

all: main.c
	$(CC) -O3 -o demo $(CFLAGS) main.c ../sierpinski/sierpinski.c ../diamond_square/diamond_square.c ../libs/*.c
