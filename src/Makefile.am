## Process this file with automake to produce Makefile.in

bin_PROGRAMS=CardCol

CardCol_SOURCES=CardCol.cpp Twopart.cpp Rovhult.cpp Hearts.cpp Buraco.cpp \
		Buraco.h Twopart.h Rovhult.h Hearts.h Options.h CardCol.h \
		SigCExt.h BuracoPile.cpp BuracoPile.h Machiavelli.cpp\
		Machiavelli.h MachiPile.cpp MachiPile.h SgtMayor.cpp SgtMayor.h 

INCLUDES = @INCLUDES@ -I../Common @CARDGAME_CFLAGS@

CardCol_LDFLAGS=-L../Common -lCard @CARDGAME_LIBS@ @LTLIBINTL@
DEFS = @DEFS@ -Wall

EXTRA_DIST=Options.meta

Options.meta: Options.h
	mgeni Options.h Options.meta
