## Process this file with automake to produce Makefile.in

bin_PROGRAMS=CardCol

CardCol_SOURCES=CardCol.cpp Twopart.cpp Rovhult.cpp Hearts.cpp HeartsScore.cpp


INCLUDES = @INCLUDES@ -I../Common `gtkmm-config --cflags`

CardCol_LDFLAGS=-L../Common -lCard  `gtkmm-config --libs` -lgthread

EXTRA_DIST=Options.meta Twopart.h Rovhult.h Hearts.h HeartsScore.h Options.h\
	   CardCol.h

Options.meta: Options.h
	mgeni Options.h Options.meta
