IDIR =../include
CC=gcc
CFLAGS=-I$(IDIR)

ODIR=obj
LDIR =../lib

LIBS=-lm

_DEPS = tcp118.h parse.h response.h
DEPS = $(patsubst %,$(IDIR)/%,$(_DEPS))

_OBJ =  tcp118.o parse.o response.o server.o
OBJ = $(patsubst %,$(ODIR)/%,$(_OBJ))


$(ODIR)/%.o: %.c $(DEPS)
	$(CC) -c -o $@ $< $(CFLAGS)

all: server client

server: $(OBJ)
	gcc -o $@ $^ $(CFLAGS) $(LIBS)

client: $(OBJ)
	gcc -o $@ $^ $(CFLAGS) $(LIBS)

.PHONY: clean

clean:
	rm -f $(ODIR)/*.o *~ core $(INCDIR)/*~ 
