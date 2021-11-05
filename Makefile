CC = gcc

CFLAGS = -w

LFLAGS = -lSDL2

all: tilegame

tilegame: tilegame.c
	$(CC) tilegame.c $(CFLAGS) $(LFLAGS) -o tilegame
