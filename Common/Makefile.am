## Process this file with automake to produce Makefile.in

lib_LTLIBRARIES=libCard.la

libCard_la_LDFLAGS=@CARDGAME_LIBS@ -version-info $(LIBVERSION) -release $(LIBRELEASE) -no-undefined

libCard_la_SOURCES=CardWidget.cpp CardImgs.cpp CardPile.cpp CardSet.cpp \
                   DeckSelect.cpp Game.cpp PlayerDlg.cpp

include_HEADERS=CardWidget.h CardImgs.h CardPile.h CardSet.h DeckSelect.h \
                Game.h PlayerDlg.h

INCLUDES = @INCLUDES@ @CARDGAME_CFLAGS@
DEFS= @DEFS@ -DPKGDIR=\"$(pkgdatadir)\"
