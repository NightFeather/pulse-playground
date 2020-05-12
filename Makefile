CC=gcc
LDFLAGS=$(pkg-config --libs libpulse)
CFLAGS=$(pkg-config --cflags libpulse)

all: main

main: main.o
	g++ -o main main.o ${LDFLAGS}

main.o: main.cpp
	g++ -c ${CFLAGS} -o main.o main.cpp

clean:
	rm main main.o
