## Process this file with automake to produce Makefile.in

SUBDIRS=Grafix

lib_LTLIBRARIES=libCard.la

libCard_la_LDFLAGS=`gtkmm-config --libs` -version-info $(LIBVERSION) -release $(LIBRELEASE) -no-undefined

libCard_la_SOURCES=CardWidget.cpp CardImgs.cpp CardPile.cpp CardSet.cpp

include_HEADERS=CardWidget.h CardImgs.h CardPile.h CardSet.h

INCLUDES = @INCLUDES@ `gtkmm-config --cflags`
DEFS= @DEFS@ -DPKGDIR=\"$(pkgdatadir)\"
