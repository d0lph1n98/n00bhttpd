CC = gcc
CFLAGS = -c -Wall -Wextra -Werror -pedantic
LDFLAGS =
INSTALL = install
PREFIX = /usr/local/
PROG = n00bhttpd
OBJS = ./src/n00bhttpd.o
SUBDIRS = doc clean all install
DOC = ./doc/n00bhttpd.8
MAN = /usr/man/man8/
TEST = ./test/index.html

all: $(PROG)

n00bhttpd: $(OBJS)
	@$(CC) $(OBJS) -o ./bin/$(PROG)

n00bhttpd.o: ./src/n00bhttpd.c
	@$(CC) $(CFLAGS) ./src/n00bhttpd.c

install: $(PROG)
	@mv $(PROG) $(PREFIX)/bin/
	@gzip $(DOC); cp $(DOC).gz $(MAN)

test: $(PROG)
	@./bin/$(PROG) 1234 $(TEST)

.PHONY: $(SUBDIRS)

clean:
	rm -f ./src/*.o ./bin/$(PROG)
