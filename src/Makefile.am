## Process this file with automake to produce Makefile.in

bin_PROGRAMS=CardCol

CardCol_SOURCES=CardCol.cpp Twopart.cpp Rovhult.cpp Hearts.cpp HeartsScore.cpp\
		Burazno.cpp Twopart.h Rovhult.h Hearts.h HeartsScore.h\
		Options.h CardCol.h SigCExt.h


INCLUDES = @INCLUDES@ -I../Common @GTKMM_CFLAGS@

CardCol_LDFLAGS=-L../Common -lCard  @GTKMM_LIBS@ `pkg-config --libs gthread-2.0`

EXTRA_DIST=Options.meta

Options.meta: Options.h
	mgeni Options.h Options.meta
