## Process this file with automake to produce Makefile.in

bin_PROGRAMS=CardCol

CardCol_SOURCES=Twopart.cpp Rovhult.cpp Hearts.cpp Buraco.cpp Buraco.h \
		Twopart.h Rovhult.h Hearts.h Options.h CardCol.h BuracoPile.cpp \
		BuracoPile.h Machiavelli.cpp Machiavelli.h MachiPile.cpp \
		MachiPile.h SgtMayor.cpp SgtMayor.h CardCol.cpp

INCLUDES = @INCLUDES@ -I../Common @CARDGAME_CFLAGS@

CardCol_LDFLAGS=-L../Common -lCard @CARDGAME_LIBS@ @LTLIBINTL@
DEFS = @DEFS@ -Wall

EXTRA_DIST=Options.meta

Options.meta: Options.h
	mgeni.pl Options.h>Options.meta
