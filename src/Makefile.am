## Process this file with automake to produce Makefile.in

SRC_BURACO=Buraco.cpp Buraco.h BuracoPile.cpp BuracoPile.h BuracoCards.cpp BuracoCards.h
if SUPPORT_BURACO
   USE_BURACO=$(SRC_BURACO)
endif

SRC_HEARTS=Hearts.cpp Hearts.h
if SUPPORT_HEARTS
   USE_HEARTS=$(SRC_HEARTS)
endif

SRC_MACHIAVELLI=Machiavelli.cpp Machiavelli.h MachiPile.cpp MachiPile.h
if SUPPORT_MACHIAVELLI
   USE_MACHIAVELLI=$(SRC_MACHIAVELLI)
endif

SRC_ROVHULT=Rovhult.cpp Rovhult.h CardValue.cpp CardValue.h
if SUPPORT_ROVHULT
   USE_ROVHULT=$(SRC_ROVHULT)
endif

SRC_SGTMAYOR=SgtMayor.cpp SgtMayor.h
if SUPPORT_SGTMAYOR
   USE_SGTMAYOR=$(SRC_SGTMAYOR)
endif

SRC_TWOPART=Twopart.cpp Twopart.h
if SUPPORT_TWOPART
   USE_TWOPART=$(SRC_TWOPART)
endif

bin_PROGRAMS=CardCol

CardCol_SOURCES=$(USE_BURACO) $(USE_HEARTS) $(USE_MACHIAVELLI)		\
$(USE_ROVHULT) $(USE_SGTMAYOR) $(USE_TWOPART) Options.h CardCol.h	\
GameTypes.cpp GameTypes.h Settings.cpp Settings.h ChatDlg.cpp		\
ChatDlg.h CardColNet.cpp CardColAppl.cpp CardCol.cpp CardColAppl.h

INCLUDES = @INCLUDES@ -I../Common @DEPENDENCIES_CFLAGS@

CardCol_LDFLAGS=-L../Common -lCard @DEPENDENCIES_LIBS@ @LTLIBINTL@
DEFS = @DEFS@ -Wall

EXTRA_DIST=Options.meta $(SRC_BURACO) $(SRC_HEARTS) $(SRC_MACHIAVELLI)	\
$(SRC_ROVHULT) $(SRC_SGTMAYOR) $(SRC_TWOPART)

Options.meta: Options.h
	mgeni.pl Options.h>Options.meta
