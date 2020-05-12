CC := g++
LDFLAGS := -lstdc++fs
LDFLAGS += $(shell pkg-config --libs libpulse)
CFLAGS := -g -Wall
CFLAGS += $(shell pkg-config --cflags libpulse)
SOURCES := $(wildcard *.cpp) $(wildcard */*.cpp)
OBJECTS := $(patsubst )
EXECUTABLE := main

all: main

main: build/main.o
	${CC} ${CFLAGS} -o main build/main.o ${LDFLAGS}

build/%.o: %.cpp
	${CC} ${CFLAGS} -c -o $@ $<

clean:
	rm main main.o
