# nsploitsh

VERSION = 0.0.1

# includes and libs
LIBS = -lncurses -lcdk -lpthread

# flags
CFLAGS = -Wall -O3 ${LIBS} -DVERSION=\"${VERSION}\" -I./include

SRC = src/main.c src/ui.c src/util.c
BIN = nsploitsh

all:
	gcc -o $(BIN) $(SRC) $(CFLAGS)

clean:
	rm -f $(BIN)

install:
	cp -f $(BIN) /usr/local/bin/
