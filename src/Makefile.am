## Process this file with automake to produce Makefile.in

bin_PROGRAMS=
bin_PROGRAMS=CardCol

CardCol_SOURCES=CardCol.cpp Twopart.cpp Rovhult.cpp

INCLUDES = @INCLUDES@ -I../Common `gtkmm-config --cflags`

CardCol_LDFLAGS=-L../Common -lCard  `gtkmm-config --libs` -lgthread
