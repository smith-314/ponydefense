CC	= /usr/bin/g++
CFLAGS	= -ffast-math -std=c++14 `freetype-config --cflags`
LDFLAGS	= -lm -lGL -lGLU -lglut -lSOIL -lftgl `freetype-config --libs`

default: src/main.cpp
	$(CC) src/main.cpp -O $(CFLAGS) -o ponydefense $(LDFLAGS)

debug: src/main.cpp
	$(CC) src/main.cpp -ggdb -Wall $(CFLAGS) -o ponydefense $(LDFLAGS)

# ugly but keeps compatibility to old versions
update: src/main.cpp
	rm -r src tex LICENSE README.md
	wget --quiet https://github.com/Smith-3/ponydefense/archive/master.zip
	unzip master.zip
	mv ponydefense-master/* .
	rm -r master.zip ponydefense-master
	$(CC) src/main.cpp -O $(CFLAGS) -o ponydefense $(LDFLAGS)
