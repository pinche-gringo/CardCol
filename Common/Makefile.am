## Process this file with automake to produce Makefile.in

SUBDIRS=Grafix

lib_LTLIBRARIES=libCard.la

libCard_la_LDFLAGS=`gtkmm-config --libs` -version-info $(LIBVERSION) -release $(LIBRELEASE) -no-undefined

libCard_la_SOURCES=CardWidget.cpp CardSet.cpp CardPile.cpp

include_HEADERS=CardWidget.h CardSet.h CardPile.h

INCLUDES = @INCLUDES@ `gtkmm-config --cflags`
DEFS= @DEFS@ -DPKGDIR=\"$(pkgdatadir)\"
