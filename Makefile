CC	= /usr/bin/g++
CFLAGS	= -ffast-math -std=c++14 `freetype-config --cflags`
LDFLAGS	= -lm -lGL -lGLU -lglut -lSOIL -lftgl `freetype-config --libs`

default: src/main.cpp
	$(CC) src/main.cpp -O $(CFLAGS) -o ponydefense $(LDFLAGS)

debug: src/main.cpp
	$(CC) src/main.cpp -ggdb -Wall $(CFLAGS) -o ponydefense $(LDFLAGS)
