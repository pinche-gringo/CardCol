## Process this file with automake to produce Makefile.in

SRC_NETWORK=ChatDlg.cpp ChatDlg.h CardColNet.cpp
if HAVE_NETWORK
   USE_NETWORK=$(SRC_NETWORK)
endif

SRC_BURACO=Buraco.cpp Buraco.h BuracoPile.cpp BuracoPile.h BuracoCards.cpp BuracoCards.h
if SUPPORT_BURACO
   USE_BURACO=$(SRC_BURACO)
endif

SRC_HEARTS=Hearts.cpp Hearts.h
if SUPPORT_HEARTS
   USE_HEARTS=$(SRC_HEARTS)
endif

SRC_JABBERWOCKY=Jabberwocky.cpp Jabberwocky.h
if SUPPORT_JABBERWOCKY
   USE_JABBERWOCKY=$(SRC_JABBERWOCKY)
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

CardCol_SOURCES=$(USE_BURACO) $(USE_HEARTS) $(USE_JABBERWOCKY)		\
$(USE_MACHIAVELLI) $(USE_ROVHULT) $(USE_SGTMAYOR) $(USE_TWOPART)	\
Options.h CardOptions.h CardCol.h GameTypes.cpp GameTypes.h		\
Settings.cpp Settings.h CardSizes.cpp CardSizes.h CardColAppl.cpp	\
CardCol.cpp CardColAppl.h $(USE_NETWORK)

INCLUDES = @INCLUDES@ @DEPENDENCIES_CFLAGS@

AM_CPPFLAGS = -Wall -Wextra
AM_CXXFLAGS = -Woverloaded-virtual -Weffc++
CardCol_LDFLAGS=-L../card -lCard @DEPENDENCIES_LIBS@ @LTLIBINTL@
DEFS = @DEFS@

EXTRA_DIST=Options.meta CardOptions.meta $(SRC_BURACO) $(SRC_HEARTS)	\
$(SRC_JABBERWOCKY) $(SRC_MACHIAVELLI) $(SRC_ROVHULT) $(SRC_SGTMAYOR)	\
$(SRC_TWOPART) $(SRC_NETWORK)

Options.meta: Options.h
	mgeni.pl Options.h>Options.meta

CardOptions.meta: CardOptions.h
	mgeni.pl CardOptions.h>CardOptions.meta
