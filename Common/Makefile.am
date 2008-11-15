## Process this file with automake to produce Makefile.in

lib_LTLIBRARIES=libCard.la

libCard_la_LDFLAGS=@DEPENDENCIES_LIBS@ -release $(LIBRELEASE) -no-undefined @LTLIBINTL@

libCard_la_SOURCES=CardWidget.cpp CardImgs.cpp CardPile.cpp	\
CardSet.cpp DeckSelect.cpp Game.cpp PlayerDlg.cpp ScoreDlg.cpp	\
PlayerConnDlg.cpp Player.cpp Human.cpp RemotePlayer.cpp		\
ComputerPlayer.cpp CardWindow.cpp

include_HEADERS=CardWidget.h CardImgs.h CardPile.h CardSet.h		\
CardWindow.h Game.h DeckSelect.h PlayerDlg.h ScoreDlg.h			\
PlayerConnDlg.h Player.h Human.h RemotePlayer.h ComputerPlayer.h

AM_CPPFLAGS = -Wall -Wextra
AM_CXXFLAGS = -Woverloaded-virtual -Weffc++
INCLUDES = @INCLUDES@ @DEPENDENCIES_CFLAGS@
DEFS= @DEFS@ -DPKGDIR=\"$(pkgdatadir)\" -Wall
