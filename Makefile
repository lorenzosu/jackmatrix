CC = gcc
SOURCES.c= jackstuff.c callbacks.c  MainWindow.c
INCLUDES = gui.h
CFLAGS	+= -Wall -g $(shell pkg-config --cflags gtk+-2.0)
LIBS 	+= $(shell pkg-config --libs jack)
LIBS 	+= $(shell pkg-config --libs gtk+-2.0) 
PROGRAM = jackmatrix

OBJECTS= $(SOURCES.c:.c=.o)

.KEEP_STATE:

debug := CFLAGS= -g

$(PROGRAM): $(OBJECTS)  $(INCLUDES)
	$(CC) -o $@ -c $(OBJECTS) $(LIBS)

clean:
	rm -f $(PROGRAM) $(OBJECTS)

