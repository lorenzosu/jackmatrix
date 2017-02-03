bin_PROGRAMS = jackmatrix
jackmatrix_SOURCES = jackmatrix.c jackmatrix.h \
                     callbacks.c callbacks.h \
                     dialogues.c dialogues.h

AM_CPPFLAGS = $(GTK_CFLAGS) $(JACK_CFLAGS)

jackmatrix_LDADD = $(GTK_LIBS) $(JACK_LIBS)

AM_CFLAGS =	-Wall

debug:
	$(MAKE) all "CXXFLAGS=-g -DDEBUG"
