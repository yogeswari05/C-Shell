CC ?= gcc
CFLAGS ?= -std=c11 -D_POSIX_C_SOURCE=200809L -D_DEFAULT_SOURCE -Wall -Wextra
SOURCES = main.c directory.c command_parser.c hop.c reveal.c history.c jobs.c proclore.c seek.c

.PHONY: all clean

all: a.out

a.out: $(SOURCES) shell.h
	$(CC) $(CFLAGS) $(SOURCES) -o $@

clean:
	rm -f a.out
