## Process this file with automake to produce Makefile.in

SUBDIRS=Grafix

lib_LTLIBRARIES=libCard.la

libCard_la_LDFLAGS=`gtkmm-config --libs`

libCard_la_SOURCES=CardWidget.cpp CardSet.cpp

include_HEADERS=CardWidget.h CardSet.h

INCLUDES = `gtkmm-config --cflags` -DPKGDIR=\"$(pkgdatadir)\"
