.PHONY := clean

ARM_PREFIX=arm-linux-gnueabi-
CC := ${ARM_PREFIX}gcc
FTP := /srv/ftp
BIN := clock
SRCS := st_clock.c st_screen.c st_buf.c st_symb.c st_symb_24x40.c st_symb_8x12.c #st_symb_17x30.c 
vpath %.c

OBJS = $(SRCS:.c=.o)

all: $(BIN)

$(BIN): $(OBJS)
	$(CC) $^ -Wall -O0 -s -static -o $@

%.o: %.c
	$(CC) -DDEBUG -Werror -Wall -I./ -I../include -g -c $<

install:
	cp -i ${BIN} ${FTP}/st

clean:
	rm -f *~ *.o $(BIN)

