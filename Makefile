CPP=g++
CC=gcc
CFLAGS=-Wall -g
PREFIX ?= /usr/local
MANPREFIX ?= $(PREFIX)/share/man
INSTALL ?= install

BIN = langursh

all: $(BIN) 

langursh: langursh.o parser.o
	$(CC) langursh.o parser.o -o langursh $(CFLAGS)

langursh.o: langursh.c
	$(CC) -c langursh.c $(CFLAGS)

parser.o: parser.c
	$(CC) -c parser.c $(CFLAGS)

man: langursh.1

langursh.1: $(BIN)
	help2man --include langursh.h2m -o langursh.1 ./langursh

install: $(BIN) 
	$(INSTALL) -m 0755 -d $(DESTDIR)$(PREFIX)/bin
	$(INSTALL) -m 0755 $(BIN) $(DESTDIR)$(PREFIX)/bin
	$(INSTALL) -m 0755 -d $(DESTDIR)$(MANPREFIX)/man1
	$(INSTALL) -m 0644 $(BIN).1 $(DESTDIR)$(MANPREFIX)/man1

uninstall:
	$(RM) $(DESTDIR)$(PREFIX)/bin/$(BIN)
	$(RM) $(DESTDIR)$(MANPREFIX)/man1/$(BIN).1

clean:
	$(RM) -f $(BIN) *.o

skip: ;

.PHONY: install uninstall clean
