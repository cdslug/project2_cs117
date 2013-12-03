IDIR =../include
CC=gcc
CFLAGS=-I$(IDIR)

ODIR=obj
LDIR =../lib

LIBS=-lm

_DEPS = packet118.h cwnd118.h probability.h tcp118.h parse.h response.h
DEPS = $(patsubst %,$(IDIR)/%,$(_DEPS))

_OBJ =  packet118.o cwnd118.o probability.o tcp118.o parse.o response.o
OBJ = $(patsubst %,$(ODIR)/%,$(_OBJ))


$(ODIR)/%.o: %.c $(DEPS)
	$(CC) -c -o $@ $< $(CFLAGS)

all: server client

server: $(OBJ) server.o
	gcc -o $@ $^ $(CFLAGS) $(LIBS)

client: $(OBJ) client.o
	gcc -o $@ $^ $(CFLAGS) $(LIBS)

.PHONY: clean

clean:
	rm -f $(ODIR)/*.o *~ core $(INCDIR)/*~ 
