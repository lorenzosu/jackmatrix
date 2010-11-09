CC = gcc
PROGRAM = jackmatrix
PROGRAM_FILES = jackmatrix.c

CFLAGS	+= -Wall -g $(shell pkg-config --cflags gtk+-2.0)
LIBS 	+= $(shell pkg-config --libs jack)
LIBS 	+= $(shell pkg-config --libs gtk+-2.0) 

all: $(PROGRAM)

%: %.c 
	$(CC) $(PROGRAM_FILES) $(CFLAGS) -o $(PROGRAM) $(LIBS)

clean:
	@rm -rf $(PROGRAM)


#jackmatrix: jackmatrix.c
#	gcc -Wall -g ./src/jackmatrix.c -o ./src/jackmatrix `pkg-config --libs jack` `` ``
