//$Id$

//PROJECT     : Cardgames
//SUBSYSTEM   : Twopart
//REFERENCES  :
//TODO        :
//BUGS        :
//REVISION    : $Revision$
//AUTHOR      : Markus Schwab
//CREATED     : 20.7.2002
//COPYRIGHT   : Anticopyright (A) 2002

// This program is free software; you can redistribute it and/or modify
// it under the terms of the GNU General Public License as published by
// the Free Software Foundation; either version 2 of the License, or
// (at your option) any later version.

// This program is distributed in the hope that it will be useful,
// but WITHOUT ANY WARRANTY; without even the implied warranty of
// MERCHANTABILITY or FITNESS FOR A PARTICULAR PURPOSE.  See the
// GNU General Public License for more details.

// You should have received a copy of the GNU General Public License
// along with this program; if not, write to the Free Software
// Foundation, Inc., 59 Temple Place - Suite 330, Boston, MA 02111-1307, USA.

#include <cardgames-cfg.h>

#include <time.h>
#include <stdlib.h>
#include <locale.h>
#include <unistd.h>

#include <glib.h>

#define CHECK 3
#define TRACELEVEL 9
#include <Check.h>
#include <Trace_.h>

#include <XAbout.h>
#include <XMessageBox.h>

#include <CardWidget.h>
#include "Twopart.h"


const unsigned int Twopart::COLS_PLAYER[NUM_PLAYERS] = { 7, 13, 7, 1 };
const unsigned int Twopart::ROWS_PLAYER[NUM_PLAYERS] = { 4,  7, 8, 7 };

// Pixmap for program
const char* Twopart::xpmTwopart[] = {
   "56 46 135 2",
   "  	c None",
   ". 	c #C4C4C4",
   "+ 	c #B1B1B1",
   "@ 	c #898989",
   "# 	c #767662",
   "$ 	c #62624E",
   "% 	c #9D9D9D",
   "& 	c #EBD8B1",
   "* 	c #C4B19D",
   "= 	c #767676",
   "- 	c #626262",
   "; 	c #766262",
   "> 	c #898976",
   ", 	c #9D8976",
   "' 	c #B19D89",
   ") 	c #897676",
   "! 	c #897662",
   "~ 	c #B19D76",
   "{ 	c #766276",
   "] 	c #626276",
   "^ 	c #C4C4B1",
   "/ 	c #141414",
   "( 	c #C4C49D",
   "_ 	c #C4B189",
   ": 	c #272727",
   "< 	c #14143B",
   "[ 	c #14273B",
   "} 	c #3B3B3B",
   "| 	c #76624E",
   "1 	c #4E3B4E",
   "2 	c #624E4E",
   "3 	c #3B274E",
   "4 	c #3B3B4E",
   "5 	c #624E62",
   "6 	c #27273B",
   "7 	c #4E4E4E",
   "8 	c #141427",
   "9 	c #B1B19D",
   "0 	c #271414",
   "a 	c #271427",
   "b 	c #3B2727",
   "c 	c #3B2714",
   "d 	c #3B1414",
   "e 	c #4E3B27",
   "f 	c #4E2714",
   "g 	c #4E2727",
   "h 	c #623B27",
   "i 	c #B1B189",
   "j 	c #272714",
   "k 	c #622714",
   "l 	c #B14E27",
   "m 	c #762714",
   "n 	c #9D3B27",
   "o 	c #893B14",
   "p 	c #9D4E14",
   "q 	c #9D4E27",
   "r 	c #9D6214",
   "s 	c #9D6227",
   "t 	c #9D7627",
   "u 	c #B17627",
   "v 	c #894E14",
   "w 	c #9D3B14",
   "x 	c #893B27",
   "y 	c #892714",
   "z 	c #763B14",
   "A 	c #763B27",
   "B 	c #896214",
   "C 	c #B17614",
   "D 	c #9D7614",
   "E 	c #764E14",
   "F 	c #B16227",
   "G 	c #B18927",
   "H 	c #C48927",
   "I 	c #C47627",
   "J 	c #894E27",
   "K 	c #9D623B",
   "L 	c #B1623B",
   "M 	c #B1764E",
   "N 	c #C4764E",
   "O 	c #B1763B",
   "P 	c #896227",
   "Q 	c #C48962",
   "R 	c #764E27",
   "S 	c #C4894E",
   "T 	c #D88962",
   "U 	c #D8894E",
   "V 	c #D87627",
   "W 	c #9D764E",
   "X 	c #894E3B",
   "Y 	c #C4B176",
   "Z 	c #4E1414",
   "` 	c #B17662",
   " .	c #D88927",
   "..	c #D8764E",
   "+.	c #4E3B3B",
   "@.	c #D8763B",
   "#.	c #B16214",
   "$.	c #624E3B",
   "%.	c #B18962",
   "&.	c #89623B",
   "*.	c #764E3B",
   "=.	c #89624E",
   "-.	c #622727",
   ";.	c #3B273B",
   ">.	c #4E4E3B",
   ",.	c #9D7662",
   "'.	c #B13B27",
   ").	c #621414",
   "!.	c #9D624E",
   "~.	c #9D9D76",
   "{.	c #894E4E",
   "].	c #764E4E",
   "^.	c #C47662",
   "/.	c #C4763B",
   "(.	c #9D763B",
   "_.	c #623B3B",
   ":.	c #27143B",
   "<.	c #B1894E",
   "[.	c #271400",
   "}.	c #B18976",
   "|.	c #C48976",
   "1.	c #00003B",
   "2.	c #B1624E",
   "3.	c #4E4E62",
   "4.	c #C44E27",
   "5.	c #9D4E3B",
   "6.	c #89764E",
   "7.	c #C4893B",
   "8.	c #623B14",
   "9.	c #000027",
   "0.	c #3B1427",
   "a.	c #D8C4B1",
   "b.	c #001427",
   "c.	c #9D9D89",
   "d.	c #D8C49D",
   ". + @ # $ $ $ $ $ $ $ $ $ $ $ $ $ $ $ $ $ $ $ $ $ $ $ $ $ $ $ $ $ $ $ $ $ $ $ $ $ $ $ $ $ $ $ $ $ $ $ $ $ # @ + ",
   "% # & * = - ; ; ; ; - ; > , # # = ' ) # , ) - > * ) # ! # ; # * , , , ~ , # # { # , , # # = ] ] ] ] = = = ^ & - ",
   "/ ( _ : < < < [ < < < < } | ! | ! ) < 1 ; } < 2 ; 3 4 ) 5 6 7 ; 2 ! ; ; ; ) | 8 4 $ - ; 2 } < < [ < < < < < * 9 ",
   "/ * _ 0 < [ 8 a b a c d d c c 0 b b b d e f 0 c 0 0 d 0 0 g d h a c 0 c c 0 d 0 d g g g g 0 d 0 c 0 b < < 8 * _ ",
   "/ * i j < < 8 k l m n o o o o p p q r r s s t u s s s s v p v p o w x n o o w o o o n o w y w z o o h < < 8 * _ ",
   "/ * i c < < 8 0 A z v r s s s u v r B u C r r r r v D r r u r u u u s r v E z k f f k f f d k f E k g < < 8 * _ ",
   "/ * _ c < < 8 k q v v r u u C F t u u u t u u u u u u u C u t t r B u s s u u u G H u u I u s r J z h < < 8 * _ ",
   "/ * i j < < a 0 z u u s F F F K L K K K L L L M N N M M O O O O u u H u s u r v J v v B r C E P P f f < < < * _ ",
   "/ * _ 0 < < 8 f J k v s M Q Q N M K K L L s u u s s s s u F u s H K M N O s s u u F s t s s R f o k g [ 4 } * _ ",
   "/ * ~ b ; ! : c o f m v s M Q N N M M Q Q N O u s s s F s L O O O s s s M K K M N S S N O q o f z k g 6 7 4 * _ ",
   "/ * ~ b | ; b k q k x m p s N M K Q Q Q Q Q T U T Q O I V H L N Q N O M W K J X L N N O v f o f A k e 1 , $ * _ ",
   "/ * Y b | ! b Z x k z m v s M ` M T Q Q Q T Q T N u I  . .I  .H s N T ..T Q Q T N M N u o k o Z m k k 2 , 1 * _ ",
   "/ * ' b 2 ! +.Z A f m k k v O M K T M Q Q Q N u I V  .H V @. .H H I O Q Q Q Q T M M S #.o Z m f k f g | ! } 9 _ ",
   "/ * ' j $.; b k l z n x w q O Q %.S M Q Q Q T L u  . . . .@.I  .H u U T Q Q T T M &.O u p o q m x m h $ , 1 9 _ ",
   "/ * ' : - 2 : d m d k f z s N W M M K Q T Q Q T M u  . .V  . .I O Q T Q Q Q Q N O M K u z d k 0 k d g } } 6 * _ ",
   "/ * ~ b 7 7 : k l o n o s O N &.M Q W h h e *.N Q N u H H H u O Q Q =.b e b *.M M J M N s o n o n y -.2 , 1 * _ ",
   "/ * ~ b 5 | ;.d k d k z u N Q N N =.*.0 c 0 e Q Q Q N F I u M T Q Q =.>.,.*.e S L L N N O r m d k Z g $ , 1 * _ ",
   "/ * ~ : 7 - } k '.z q s M =.K s s K &.0 c 0 A Q Q Q T Q N S Q Q Q Q S $.8 $.%.F u u N N M O q m x z g 2 ! ;.* _ ",
   "/ * ~ a +.} : d m d v O ` %.M u u F X 0 0 0 h Q Q T Q S N N Q T T Q Q *.8 =.Q K s H O K M N t f ).Z g 2 ! } * _ ",
   "/ * ~ a | ; ;.k n k r O !.W s r P O K c 0 0 X T Q K n n n l N Q T Q Q *.8 =.T S r t O M M M u q o m g 7 ! } * _ ",
   "/ * ~.b ,.) +.d z f q M M !.K r D O !.c 0 0 =.` ,.Q K w w l Q T X W T *.8 {.Q O r t K K M N u v k f b < < 8 * _ ",
   "/ * ~.: 7 } : k o k v u S Q S r u u L c 0 0 &.$.e e *.!.M M *.h M Q Q ].8 ].^.s C s M Q N /.v f m k g < < 8 * _ ",
   "/ * ~.: ; ! e k n m n v F O Q O v q L e 0 c !.T Q Q %.*.d 0 +.` %.].%.{.8 =.N B v O M O t p n m o A g < < 8 * _ ",
   "/ * ~.: 2 | ;.Z m c m k o r s L B r (.e 0 : ,.Q Q Q $.0 R Q Q *.8 < : _.:.=.N s r L s s E f k d f Z b < < 8 * _ ",
   "/ * ~.: # ) +.m n z x J F K ^.M r r O h 0 c M Q *.0 g M Q M N M *.h X ].a ].` F D t Q N O s v m o z g < < 8 * _ ",
   "/ * ~.8 1 1 a d z c p O S M (.s B u <.R [.e =.j j *.Q Q O H u S Q Q Q =.a {.Q M u D s M M M u k d Z g 2 # 6 * _ ",
   "/ * ~.: 2 ; +.x '.z u O W M F C K ` Q X c e %.}.Q Q Q M u I u O Q M Q | < ].|.|.M s F K K L O r x x -.$ 7 8 * _ ",
   "/ * ~.: ; # ;.d m f u (.=.M N O O =.Q !.0 h Q Q Q Q Q T M K M T !.8 6 < < < ;.a _./.M M M M u E Z d g } 4 8 * i ",
   "/ * ~.: ! ; b o q z F M Q Q &.M Q !.%.!.0 *.Q Q Q Q Q S N ^.Q Q K 8 < < < < < 1.*.N K M ^.O J k n A g 6 < 8 ( i ",
   "/ * ~.: $ 2 : d m c o s S Q M %.N W Q W c *.Q Q O q l l F N N N N g < < < < < b ` (.W M O u z d f f g 2 - : * _ ",
   "/ * ~./ 2 2 b m x z x z s N 2.&.U W ` =.0 $.Q Q n '.l l '.l L N N M ].6 < 6 2 Q %.!.Q N u o o k y m g 7 7 a ( i ",
   "/ * ~.: ; ! } f x k z m s u L !.T M M ` M M Q T J '.l l l l '.'.l L Q Q ` Q Q N K Q S O v m m k m k -.- 7 a ( i ",
   "/ * ~.a | | : k x k x h J F M W Q Q Q Q Q Q ^.Q J '.l l l l l l '.'.q M Q Q Q Q W M N s z k m f k k -.- 7 8 * i ",
   "/ * ~.: ; 2 ;.k n A x m z s M ` Q Q Q Q Q Q Q L n l '.l l '.'.l l l l L Q Q Q Q /.S /.r o m x m o m h ; 3.a ( i ",
   "/ * ~./ | - : f A f A k z s M M Q Q Q Q ^.N Q K '.'.4.l l 4.4.'.'.'.'.l S T Q T N /.I r m f k f k f g a < 8 * i ",
   "/ * ~.b ! 2 : A l x q x q s N Q Q K M N Q Q Q U K 5.5.J q 5.n J L X M T T T Q T Q Q N s p w n x n x -.2 ; 6 * _ ",
   "/ * ~.: ! ) } d k d m f r O M K N !.=.W N K K K M Q T Q Q T T S %.Q Q T Q %.M M M T Q O v f k d Z d g ; 7 8 * _ ",
   "/ * ~.: ; ! +.o l o q v K ^.T N &.%.6.=.M K K s s q s s s s r r r s s O M =.` Q =.2.M M O s v y o z -.4 4 8 * _ ",
   "/ * ~.8 4 4 8 f v B #.u s F u u u u O /./.M S N M O O O L O M M M M P u u O !.=.N M N T Q O t E v z b < < 8 * i ",
   "/ * ~./ < < : o x v s u u s E s B B B u v r u u u u (.O O O 7.S S S S <.S <.S S /.O O O u I u t v z g < < 8 * _ ",
   "/ * ~.8 < < 8 d J s r p p r v F u u u u u u u D r s s r r r B P P r v r s u s r P v B v v F u P s 8.b < < 8 * i ",
   "/ * ~.8 < < a m x z o m m o m o m z o z z x z v E r r t u u G G u G u u u u u t t t t s v E z 8.z k g < < 8 * _ ",
   "/ * ~.9.< < 8 k z f o k k m f m f m z m m m k o f o k k 8.8.z z A x v z z z o 8.m k z f 8.f h f A k b < < 8 * _ ",
   "/ * ~.9.< < < 0.g e _._.$.].g _.].b _.$.e _.$.+._.].$.$.b b +.]._.+.+.].+.b +.b g g h $.2 ].+.2 6 8 < < < 8 _ i ",
   "} ' a.7 < < < < < 6 ; ;.} ; ;.1 +.< :.7 7 7 $ 7 $ 2 +.2 6 < } =.2 | 2 2 2 2 2 } $ 7 ; 2 7 1 6 1 < b.< < < - & c.",
   ". > > * ( ( ( ( ( ( ( ( ( ( ( ( ( ( ( ( ( ( ( ( ( ( ( ( ( ( ( * ( ( ( ( ( ( ( ( * ( ( ( ( ( ( ( ( ( ( ( d.9 > > "};

// Pixmap for author
const char* Twopart::xpmAuthor[] = {
   "56 43 5 1",
   " 	c None",
   "!   c #0000FF",
   "@	c #000000",
   "-	c #AEAAAE",
   ".	c #FFFFFF",
   ".......................................................@",
   "..-----------------------------------------------------@",
   "..-----------------------------------------------------@",
   "..----------------------!!!!!!-------------------------@",
   "..--------------------!!!!@@!!!!-----------------------@",
   "..------------------!!!!@@---@@!!!---------------------@",
   "..-----------------!!!@@--------!!!--------------------@",
   "..-----------------!!!@---!!----!!!--------------------@",
   "..----------------!!!@---!!!!@---!!!-------------------@",
   "..----------------!!@---!!!!!!@---!!@------------------@",
   "..----------------!!@----!!!!@----!!@------------------@",
   "..----------------!!!-----!!@----!!!@------------------@",
   "..-----------------!!!-----@-----!!!@------------------@",
   "..-----------------!!!!---------!!!@-------------------@",
   "..------------------!!!!-------!!!@--------------------@",
   "..--------------------!!!!--!!!!@@---------------------@",
   "..----------------------!!!!!!@@-----------------------@",
   "..-----------------------!!!!@-------------------------@",
   "..------------------------!!@--------------------------@",
   "..------------------------!!@--------------------------@",
   "..------------------!!!!!!!!!!!!!!---------------------@",
   "..-----------------!!!!!!!!!!!!!!!!--------------------@",
   "..----------------!!!@@@@@!!@@@@@!!!-------------------@",
   "..----------------!!@-----!!@-----!!@------------------@",
   "..----------------!!@-----!!@-----!!@------------------@",
   "..----------------!!@-----!!@-----!!@------------------@",
   "..----------------!!@-----!!@-----!!@------------------@",
   "..-----------------@@-----!!@------@@------------------@",
   "..------------------------!!@--------------------------@",
   "..--------------------!!!!!!!!!!-----------------------@",
   "..------------------!!!!!!!!!!!!!!---------------------@",
   "..-----------------!!!@@@@@@@@@!!!!--------------------@",
   "..-----------------!!@----------!!!!-------------------@",
   "..----------------!!!@-----------!!!@------------------@",
   "..----------------!!@------------!!!@------------------@",
   "..----------------!!@-------------!!@------------------@",
   "..----------------!!@-------------!!@------------------@",
   "..----------------!@--------------!!@------------------@",
   "..----------------!!@--------------@@------------------@",
   "..-----------------!@----------------------------------@",
   "..------------------@----------------------------------@",
   "..-----------------------------------------------------@",
   "@@@@@@@@@@@@@@@@@@@@@@@@@@@@@@@@@@@@@@@@@@@@@@@@@@@@@@@@" };


// With a very ugly trick initialize I18n before the first use of gettext)
XApplication::MenuEntry Twopart::menuItems[] = {
    { (initI18n (PACKAGE, LOCALEDIR),
      _("_Game")),    _("<alt>G"), 0,     BRANCH },
    { _("_New"),      _("<ctl>N"), NEW,   ITEM },
    { "",             "",          0,     SEPARATOR },
    { _("E_xit"),     _("<ctl>Q"), EXIT,  ITEM },
#if TRACELEVEL > 0
    { _("_Options"),  _("<alt>O"), 0,     BRANCH },
    { _("_Debug"),    _("<ctl>D"), DEBUG, CHECKITEM },
#endif
    { _("_Help"),     _("<alt>H"), 0,     LASTBRANCH },
    { _("_About..."), _("<ctl>a"), ABOUT, ITEM } };


/*--------------------------------------------------------------------------*/
//Purpose   : Defaultconstructor; all widget are created
/*--------------------------------------------------------------------------*/
Twopart::Twopart ()
   : XApplication (PACKAGE " - Twopart V" VERSION), status ()
     , tblTable (11, 7), cardFaces (USED_CARDS), cards (), pThread (NULL)
     , staple (ICardPile::VERY_COMPRESSED, ICardPile::SHOWBACK)
     , played (ICardPile::COMPRESSED, ICardPile::SHOWFACE)
     , bfPlayers ((1 << NUM_PLAYERS) - 1) {
   set_usize (WIDTH, HEIGHT);

   addMenu (menuItems[0]);
   pMenuNew = addMenu (menuItems[1]); Check3 (pMenuNew);
   pMenuNew->set_sensitive (false);
 
   tblTable.show ();
   getClient ()->pack_start (tblTable, true, true, 5);

   status.show ();
   getClient ()->pack_start (status, false);

   staple.show ();
   tblTable.attach (staple, 2, 3, 2, 3, 0, 0, 5, 5);

   tblTable.set_col_spacings (2);
   tblTable.set_row_spacings (2);

   show ();

   // Load cards in background
   pThread = THRDAPPL::create (*this, (THRDAPPL::THREAD_OBJMEMBER)&Twopart::loadCards,
                               NULL);
   TRACE9 ("Twopart::Twopart () - Thread-ID = " << pThread->getID ());

   // Create controls
   addMenus (menuItems + 2, sizeof (menuItems) / sizeof (menuItems[0]) - 2);

   // Show and attach card-piles
   for (int i (0); i < NUM_PLAYERS; ++i) {
      players[i].won.setStyle (ICardPile::VERY_COMPRESSED);
      players[i].won.show ();
      tblTable.attach (players[i].won, COLS_PLAYER[i] + 1,
                       COLS_PLAYER[i] + 2,
                       ROWS_PLAYER[i] + ((i == 2) ? 2 : -2),
                       ROWS_PLAYER[i] + ((i == 2) ? 2 : -2) + 1,
                       0, 0, 1);

      TRACE9 ("Twopart::Twopart () - Set at: "
              << COLS_PLAYER[i] + 1 << '/' << ROWS_PLAYER[i] + ((i == 2) ? 2 : -2));

      players[i].hand.show ();
      tblTable.attach (players[i].hand, COLS_PLAYER[i],
                       COLS_PLAYER[i] + 3, ROWS_PLAYER[i],
                       ROWS_PLAYER[i] + 1, 0, 0, 1);
      TRACE9 ("Twopart::Twopart () - 2nd set at: "
              << COLS_PLAYER[i] << '/' << ROWS_PLAYER[i]);
   }

   played.show ();
   tblTable.attach (played, 3, 11, 5, 8, 0, 0, 0, 5);

   for (unsigned int i (0); i < NUM_PLAYERS; ++i)
      players[i].won.setShowOption (ICardPile::SHOWBACK);
}

/*--------------------------------------------------------------------------*/
//Purpose   : Destructor
/*--------------------------------------------------------------------------*/
Twopart::~Twopart () {
   TRACE9 ("Twopart::~Twopart ()");
}


/*--------------------------------------------------------------------------*/
//Purpose   : Command-handler
//Parameters: menu: ID of command (menu)
/*--------------------------------------------------------------------------*/
void Twopart::command (int menu) {
   switch (menu) {
   case NEW: {
      cleanTable ();
      fillStaple ();
      dealCards ();
      break;
   }

   case ABOUT: {
      string ver (_("Anticopyright (A) 2002 Markus Schwab"
                    "\ne-mail: g17m0@lycos.com\n\nCompiled on %1 at %2"));
      ver.replace (ver.find ("%1"), 2, __DATE__);
      ver.replace (ver.find ("%2"), 2, __TIME__);

      XAbout* about (new XAbout (ver, PACKAGE " - Twopart V" VERSION));
      about->setIconProgram (xpmTwopart);
      about->setIconAuthor (xpmAuthor); }
      break;

   case EXIT:
      delete_event_impl (0);
      break;

#if TRACELEVEL > 0
   case DEBUG:
      for (int i (1); i < NUM_PLAYERS; ++i) {
         ICardPile::ShowOpt show (players[i].hand.getShowOption ());
         show = ((show == ICardPile::SHOWFACE)
                 ? ICardPile::SHOWBACK : ICardPile::SHOWFACE);
         players[i].hand.setShowOption (show);
         players[i].hand.setStyle ((show == ICardPile::SHOWFACE)
                                   ? ICardPile::COMPRESSED
                                   : ICardPile::QUITE_COMPRESSED);
      }
      break;

#endif
   default:
      Check3 (0);
   } // end-switch
}

/*--------------------------------------------------------------------------*/
//Purpose   : Enables the cards of the passed player
//Parameters: player: Player to enable
/*--------------------------------------------------------------------------*/
void Twopart::enablePlayer (unsigned int player) {
   Check3 (activeCards.empty ());

   TRACE2 ("Twopart::enablePlayer (unsigned int) - Hand of player "
           << player << " has " << players[player].hand.numberOfCards () << " cards");

   for (int i (players[player].hand.numberOfCards ()); i;)
      activeCards.push_back
         (players[player].hand.at (--i).clicked.connect_after
          (bind (slot (this, &Twopart::handSelected), player, i)));

   status.pop (1);
   std::string stat ( _("Turn of player %1"));
   stat.replace (stat.find ("%1"), 2, (char)(player + '0'));
   status.push (1, stat);
}

/*--------------------------------------------------------------------------*/
//Purpose   : Disables the cards of the passed player
/*--------------------------------------------------------------------------*/
void Twopart::disableLastPlayer () {
   TRACE2 ("Twopart::disableLastPlayer () - " << activeCards.size () << " cards");

   for (int i (activeCards.size ()); i > 0;) {
      activeCards[--i].disconnect ();
      activeCards.pop_back ();
   }
   Check3 (activeCards.empty ());
}

/*--------------------------------------------------------------------------*/
//Purpose   : Callback after clicking on a card in hand
//Parameters: player: ID of player
//            iCard: Offset of card in hand
/*--------------------------------------------------------------------------*/
void Twopart::handSelected (unsigned int player, unsigned int pos) {
   Check3 (player <= NUM_PLAYERS);
   Check3 (pos <= players[player].hand.numberOfCards ());

   CardWidget& card (players[player].hand.remove (pos));
   TRACE3 ("Twopart::handSelected (unsigned int, unsinged int) - Checking player "
           << player << "; Card at " << pos << " = " << card);

   played.append (card);

   if (staple.numberOfCards ())
      players[player].hand.insertSorted (staple.removeShownTopCard ());

   // Check if every player still in game already played; end round if so
   // or calculate next player if not
   bfPlayers &= ~(1 << player);
   if (bfPlayers) {
      while (!(bfPlayers & (1 << player))) {
         ++player;
         if (player >= NUM_PLAYERS)
            player %= NUM_PLAYERS;
      }
   }
   else
      player = endRound ();

   disableLastPlayer ();
   enablePlayer (player);
}

/*--------------------------------------------------------------------------*/
//Purpose   : Checks if there is a winner for the round and moves played cards
//            to him if so. Else enable the players which can continue
//Returns   : unsigned int: Next player
/*--------------------------------------------------------------------------*/
unsigned int Twopart:: endRound () {
   TRACE8 ("Twopart::endRound ()");
   Check3 (!bfPlayers);

   unsigned int nextPlayer (NUM_PLAYERS);
   unsigned int i (0);
   unsigned int cPlayers (NUM_PLAYERS);
   bfPlayers = (1 << NUM_PLAYERS) - 1;
   do {
      Check3 ((i + cPlayers) <= played.numberOfCards ());

      int maxNr (-1);
      int maxEqualNr (-1);
      int posMax (-1);
      int posMaxEqual (-1);

      // Check if card is bigger then all previous
      for (unsigned int j (i); j < (cPlayers + i); ++j) {
         if ((int)(played.at (j).number ()) > maxNr) {
            TRACE3 ("TwoPart::endRound () - New highest card " << played.at (j)
                    << " at position " << j);
            maxNr = (int)played.at (j).number ();
            posMax = j;
         }
         Check3 (posMax < played.numberOfCards ());
         Check3 (maxNr == played.at (posMax).number ());

         // Check if card has equal cards
         for (unsigned int k (j + 1); k < (cPlayers + i); ++k)
            if (played.at (j).number () == played.at (k).number ())
               if ((int)(played.at (j).number ()) > maxEqualNr) {
                  TRACE3 ("TwoPart::endRound () - Found equal "
                          << played.at (j).numberStr ()
                          << " at positions " << j << " and " << k);
                  maxEqualNr = (int)played.at (j).number ();
                  posMaxEqual = j;
                  break;
               }
      }

      // Equal cards found
      if (posMaxEqual >= 0) {
         // Start player is the first who played the highest cards
         unsigned int nthPlayer (posMaxEqual - i);
         nextPlayer = 0;
         while (nthPlayer)
            if (bfPlayers & (1 << nextPlayer++))
               --nthPlayer;
         // Set startplayer
         bfPlayers = 1 << nextPlayer;
         TRACE5 ("Twopart::endRound () - Found equal cards; continuing with player "
                 << nextPlayer);

         // Increase position of start card of actual round
         i += cPlayers;
         cPlayers = 1;

         // Add players having equal cards
         for (unsigned int j (posMaxEqual + 1); j < i; ++j)
            if (played.at (j).number () == maxEqualNr) {
               TRACE5 ("Twopart::endRound () - Found equal cards; Player "
                       << (j - posMaxEqual) << " also in round");
               bfPlayers |= (1 << (j - posMaxEqual));
               ++cPlayers;
            }

         TRACE5 ("Twopart::endRound () - Found equal cards; " << cPlayers
                 << " players still in round");
         Check3 (cPlayers > 1);
      }
      else {
         nextPlayer = posMax - i;
         TRACE5 ("Twopart::endRound () - Found winner: " << nextPlayer);

         bfPlayers = (1 << NUM_PLAYERS) - 1;
         Check3 ((i + cPlayers) == played.numberOfCards ());

         movePlayedCardsToPlayer (nextPlayer);
         break;
      }
   } while (i < played.numberOfCards ());

   return nextPlayer;
}

/*--------------------------------------------------------------------------*/
//Purpose   : Method to move the cards of the actual round to the winner
//Parameters: nrLooser: Nr. of player getting all played cards
/*--------------------------------------------------------------------------*/
void Twopart::movePlayedCardsToPlayer (unsigned int receiver) {
   TRACE8 ("Twopart::movePlayedCardsToPlayer () - " << played.numberOfCards ()
           << " cards for player " << receiver);
   Check3 (receiver < NUM_PLAYERS);

   while (played.numberOfCards ())
      players[receiver].won.insertSorted (played.remove (0));

   players[receiver].hand.sortByNumber ();
}

/*--------------------------------------------------------------------------*/
//Purpose   : Checks which player has still cards left
//Parameters: actPlayer: ID of actual player
//Returns   : int: ID of player or -1 (if none can continue)
/*--------------------------------------------------------------------------*/
int Twopart::nextAvailablePlayer (unsigned int actPlayer) const {
   // We assume (without checking), that acutal player still has cards
   for (unsigned int i (1); i < NUM_PLAYERS; ++i) {
      unsigned int player ((actPlayer + i) & 0x3);

      if (players[player].hand.numberOfCards ())
         return player;
   }
   return -1;
}

/*--------------------------------------------------------------------------*/
//Purpose   : Loads the cards (from xpm-files)
/*--------------------------------------------------------------------------*/
void Twopart::loadCards () {
   Check3 (staple.is_realized ());

   gdk_threads_enter ();
   status.push (1, _("Loading cardimages ..."));
   gdk_threads_leave ();

   cardFaces.load (staple.get_window ());  // Cards need an realized (!) parent
   cards.addPacket (cardFaces);
   pThread = NULL;

   gdk_threads_enter ();
   pMenuNew->set_sensitive (true);
   status.pop (1);
   status.push (1, _("Start a new game with Ctrl+N (or Game -> New)"));

   assert (cardFaces.numberOfCards ());
   const Gdk_Pixmap& img (cardFaces.getCardImage (0));
   unsigned int width (const_cast<Gdk_Pixmap&> (img).width ());
   unsigned int height (const_cast<Gdk_Pixmap&> (img).height ());

   for (int i (0); i < NUM_PLAYERS; ++i) {
      players[i].won.set_usize (width + 20, height + 5);
      players[i].hand.set_usize (width * 3, height + 5);
   }

   played.set_usize (width + 50, height);
   staple.set_usize (width, height);
   gdk_threads_leave ();
}

/*--------------------------------------------------------------------------*/
//Purpose   : Shuffles (Randomizes) the cards onto the staple
/*--------------------------------------------------------------------------*/
void Twopart::fillStaple () {
   // Randomize and put cards onto staple
   cards.shuffle ();
   staple.setTopCards (cards.getCards (), false);
}

/*--------------------------------------------------------------------------*/
//Purpose   : Remove cards from everything which can hold them
/*--------------------------------------------------------------------------*/
void Twopart::cleanTable () {
   staple.clear ();                                             // Clear staple
   for (int i (0); i < NUM_PLAYERS; ++i) {            // Clear cards of players
      players[i].hand.clear ();
   }
   played.clear ();

   disableLastPlayer ();
}

/*--------------------------------------------------------------------------*/
//Purpose   : Deals the cards
/*--------------------------------------------------------------------------*/
void Twopart::dealCards () {
   TRACE9 ("Twopart::dealCards ()");

   // Show cards on table: For all players put 6 cards on table (only the
   // (upper visible) and 3 (visible ones) in hand
   for (unsigned int i (0); i < NUM_PLAYERS; ++i)
      for (unsigned int j (0); j < 3; ++j) {
         // Put card into hand
         CardWidget& card (staple.removeTopCard ());
         card.showFace ();
         players[i].hand.insertSorted (card);
      }

   enablePlayer (0);
   bfPlayers = (1 << NUM_PLAYERS) - 1;
}


/*--------------------------------------------------------------------------*/
//Purpose   : Entrypoint of application
//Parameters: argc: Number of parameters
//            argv: Array with pointer to parameter
//Returns   : int: Status
/*--------------------------------------------------------------------------*/
int main (int argc, char* argv[]) {
   srand (time (NULL));              // Initialize the random number generator

   g_thread_init (NULL);

   Main appl (argc,argv);
   Twopart win;

   gdk_threads_enter ();
   appl.run ();
   gdk_threads_leave ();
   return 0;
}
