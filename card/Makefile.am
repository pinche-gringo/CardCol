## Process this file with automake to produce Makefile.in

lib_LTLIBRARIES=libCard.la

includedir = ${prefix}/include/card

libCard_la_LDFLAGS=@DEPENDENCIES_LIBS@ -release $(LIBRELEASE) -no-undefined @LTLIBINTL@

libCard_la_SOURCES=Widget.cpp Images.cpp Pile.cpp Set.cpp Game.cpp	\
DeckSelect.cpp PlayerDlg.cpp ScoreDlg.cpp PlayerConnDlg.cpp Player.cpp	\
Human.cpp RemotePlayer.cpp ComputerPlayer.cpp Window.cpp

include_HEADERS=Widget.h Images.h Pile.h Set.h Window.h Game.h Human.h	\
DeckSelect.h PlayerDlg.h ScoreDlg.h PlayerConnDlg.h Player.h		\
RemotePlayer.h ComputerPlayer.h

AM_CPPFLAGS = -Wall -Wextra
AM_CXXFLAGS = -Woverloaded-virtual -Weffc++
INCLUDES = @INCLUDES@ @DEPENDENCIES_CFLAGS@
DEFS= @DEFS@ -DPKGDIR=\"$(pkgdatadir)\" -Wall
