## Process this file with automake to produce Makefile.in

bin_PROGRAMS=CardCol

CardCol_SOURCES=CardCol.cpp Twopart.cpp Rovhult.cpp Hearts.cpp Buraco.cpp \
		Buraco.h Twopart.h Rovhult.h Hearts.h Options.h CardCol.h \
		SigCExt.h

INCLUDES = @INCLUDES@ -I../Common @CARDGAME_CFLAGS@

CardCol_LDFLAGS=-L../Common -lCard @CARDGAME_LIBS@

EXTRA_DIST=Options.meta

Options.meta: Options.h
	mgeni Options.h Options.meta
