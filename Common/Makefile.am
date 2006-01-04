## Process this file with automake to produce Makefile.in

lib_LTLIBRARIES=libCard.la

libCard_la_LDFLAGS=@DEPENDENCIES_LIBS@ -release $(LIBRELEASE) -no-undefined @LTLIBINTL@

libCard_la_SOURCES=CardWidget.cpp CardImgs.cpp CardPile.cpp CardSet.cpp \
                   DeckSelect.cpp Game.cpp PlayerDlg.cpp ScoreDlg.cpp \
		   ScoreDlg.h PlayerConnDlg.cpp PlayerConnDlg.h Player.cpp \
		   Player.h Human.cpp Human.h RemotePlayer.cpp RemotePlayer.h \
		   ComputerPlayer.cpp ComputerPlayer.h

include_HEADERS=CardWidget.h CardImgs.h CardPile.h CardSet.h DeckSelect.h \
                Game.h PlayerDlg.h

INCLUDES = @INCLUDES@ @DEPENDENCIES_CFLAGS@
DEFS= @DEFS@ -DPKGDIR=\"$(pkgdatadir)\" -Wall
