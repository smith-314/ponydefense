CC	= /usr/bin/g++
CFLAGS	= -ffast-math -std=c++14 `pkg-config --cflags freetype2 glu ftgl`
LDFLAGS	= -lm -lSOIL -lglut `pkg-config --libs freetype2 glu ftgl`

default: src/main.cpp
	$(CC) src/main.cpp -O $(CFLAGS) -o ponydefense $(LDFLAGS)

debug: src/main.cpp
	$(CC) src/main.cpp -ggdb -Wall $(CFLAGS) -o ponydefense $(LDFLAGS)

clean: ponydefense
	rm ponydefense
