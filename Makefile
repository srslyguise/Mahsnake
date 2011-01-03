CC=g++
LIBS=-lSDL -lSDL_ttf

all: mahsnake

mahsnake: mahsnake.o
	$(CC) -o mahsnake mahsnake.o $(LIBS)

mahsnake.o: mahsnake.cpp
	$(CC) -c -o mahsnake.o mahsnake.cpp

clean:
	rm -f mahsnake.o
	rm -f mahsnake
