CC = gcc
JACKMATRIX = jackmatrix
JACKMATRIX_SOURCE = jackmatrix.c

CFLAGS	+= -Wall -g $(shell pkg-config --cflags gtk+-2.0)
LIBS 	+= $(shell pkg-config --libs jack)
LIBS 	+= $(shell pkg-config --libs gtk+-2.0) 

all: $(JACKMATRIX)

%: %.c 
	$(CC) $(JACKMATRIX_SOURCE) $(CFLAGS) -o $(JACKMATRIX) $(LIBS)

clean:
	@rm -rf $(JACKMATRIX)