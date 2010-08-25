# sploitsh

VERSION = 0.0.1

# includes and libs
LIBS = -lreadline

# flags
CFLAGS = -Wall -O3 ${LIBS} -DVERSION=\"${VERSION}\" -I./include

SRC = src/main.c src/util.c
BIN = sploitsh

all:
	gcc -o $(BIN) $(SRC) $(CFLAGS)

ncurses:
	make -C nsploitshell

clean:
	make -C nsploitshell clean
	rm -f $(BIN)

install:
	cp -f $(BIN) /usr/local/bin/
	make -C nsploitshell install
