//$Id$

//PROJECT     : Cardgames
//SUBSYSTEM   : Rovhult
//REFERENCES  :
//TODO        : - MsgBox displaying looser?
//BUGS        :
//REVISION    : $Revision$
//AUTHOR      : Markus Schwab
//CREATED     : 28.3.2002
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
#include "Rovhult.h"


GtkTargetEntry RovhultAppl::dndTypeHand  = { "icon/card/hand", GTK_TARGET_SAME_APP, 0 };
GtkTargetEntry RovhultAppl::dndTypeTable = { "icon/card/table", GTK_TARGET_SAME_APP, 1 };


const unsigned int RovhultAppl::COLS_PLAYER[NUM_PLAYERS] = { 7, 13, 7, 1 };
const unsigned int RovhultAppl::ROWS_PLAYER[NUM_PLAYERS] = { 4, 7, 13, 7 };

// Pixmap for program
const char* RovhultAppl::xpmRovhult[] = {
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
const char* RovhultAppl::xpmAuthor[] = {
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
XApplication::MenuEntry RovhultAppl::menuItems[] = {
    { (initI18n (PACKAGE, LOCALEDIR),
      _("_Game")),    _("<alt>G"), 0,     BRANCH },
    { _("_New"),      _("<ctl>N"), NEW,   ITEM },
    { "",             "",          0,     SEPARATOR },
    { _("E_xit"),     _("<ctl>Q"), EXIT,  ITEM },
    { _("_Options"),  _("<alt>O"), 0,     BRANCH },
    { _("_Debug"),    _("<ctl>D"), DEBUG, CHECKITEM },
    { _("_Help"),     _("<alt>H"), 0,     LASTBRANCH },
    { _("_About..."), _("<ctl>a"), ABOUT, ITEM } };


/*--------------------------------------------------------------------------*/
//Purpose   : Defaultconstructor; all widget are created
/*--------------------------------------------------------------------------*/
RovhultAppl::RovhultAppl ()
   : XApplication (PACKAGE " - Rovhult V" PRG_RELEASE), status (), actPlayer (0)
     , tblTable (16, 19), cardFaces (USED_CARDS), cards (), pThread (NULL)
     , staple (ICardPile::VERY_COMPRESSED), played (ICardPile::VERY_COMPRESSED) {
   set_usize (WIDTH, HEIGHT);

   addMenu (menuItems[0]);
   pMenuNew = addMenu (menuItems[1]); Check3 (pMenuNew);
   pMenuNew->set_sensitive (false);

   tblTable.show ();
   getClient ()->pack_start (tblTable, true, true, 5);

   status.show ();
   getClient ()->pack_start (status, false);

   staple.show ();
   tblTable.attach (staple, 3, 4, 2, 5, 0, 0);

   tblTable.set_col_spacings (2);
   tblTable.set_row_spacings (2);

   show ();

   // Load cards in background
   pThread = THRDAPPL::create (*this, (THRDAPPL::THREAD_OBJMEMBER)&RovhultAppl::loadCards,
                               NULL);
   TRACE9 ("RovhultAppl::RovhultAppl () - Thread-ID = " << pThread->getID ());

   // Create controls
   addMenus (menuItems + 2, sizeof (menuItems) / sizeof (menuItems[0]) - 2);

   // Show and attach card-piles
   for (int i (0); i < NUM_PLAYERS; ++i) {
      for (int j (0); j < 3; ++j) {
         players[i].reserve[j].setStyle (ICardPile::QUITE_COMPRESSED);
         players[i].reserve[j].show ();
         tblTable.attach (players[i].reserve[j], COLS_PLAYER[i] + (j << 1),
                          COLS_PLAYER[i] + 1 + (j << 1), ROWS_PLAYER[i],
                          ROWS_PLAYER[i] + 2, 0, 0, 1);

         TRACE9 ("RovhultAppl::RovhultAppl () - Set at: "
                 << COLS_PLAYER[i]  + (j << 1) << '/' << ROWS_PLAYER[i]);
      }

      players[i].hand.setStyle (i ? ICardPile::QUITE_COMPRESSED : ICardPile::NORMAL);
      players[i].hand.setShowOption (i ? ICardPile::SHOWBACK : ICardPile::SHOWFACE);
      players[i].hand.show ();
      tblTable.attach (players[i].hand, COLS_PLAYER[i],
                       COLS_PLAYER[i] + 5,
                       ROWS_PLAYER[i] + (i ? 3 : -3),
                       ROWS_PLAYER[i] + (i ? 3 : -3) + 2
                       , GTK_FILL|GTK_EXPAND, GTK_FILL|GTK_EXPAND, 1);
      TRACE9 ("RovhultAppl::RovhultAppl () - 2nd set at: "
              << COLS_PLAYER[i] + (i << 1) << '/'
              << ROWS_PLAYER[i] + (i ? 3 : -3));
   }

   tblTable.attach (played, 7, 11, 5, 14, 0, 0, 1);

   //   players[0].hand.showTips (false);
}

/*--------------------------------------------------------------------------*/
//Purpose   : Destructor
/*--------------------------------------------------------------------------*/
RovhultAppl::~RovhultAppl () {
   TRACE9 ("RovhultAppl::~RovhultAppl ()");
}


/*--------------------------------------------------------------------------*/
//Purpose   : Command-handler
//Parameters: menu: ID of command (menu)
/*--------------------------------------------------------------------------*/
void RovhultAppl::command (int menu) {
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

      XAbout* about (new XAbout (ver, PACKAGE " - Rovhult V" VERSION));
      about->setIconProgram (xpmRovhult);
      about->setIconAuthor (xpmAuthor); }
      break;

   case EXIT:
      delete_event_impl (0);
      break;

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

   default:
      Check3 (0);
   } // end-switch
}

/*--------------------------------------------------------------------------*/
//Purpose   : Callback after finishing card-exchange
/*--------------------------------------------------------------------------*/
void RovhultAppl::finishedExchange () {
   played.show ();

   status.pop (1);
   status.push (1, "");

   exchangeAutoplayerCards ();

   // Remove drag´n´drop abilities and compress cards
   for (int i (0); i < NUM_PLAYERS; ++i)
      players[i].hand.sortByNumber ();

   for (int i (0); i < 3; ++i) {
      CardWidget& card (players[0].hand.at (i));
      unregisterDND (card);
      unregisterDND (players[0].reserve[i].getTopCard ());
   }

   players[0].hand.setStyle (ICardPile::COMPRESSED);
   enablePlayer (0);
   staple.getTopCard ().remove_accelerator (*get_accel_group (), ' ', 0);
   pileTop.disconnect ();
}

/*--------------------------------------------------------------------------*/
//Purpose   : Compares two cards according the rules of Rovhult
//Parameters: lhs, rhs: Cards to compare
//Returns   : int: >0, if number of lhs is smaller; 0 if equal or >0 if bigger
/*--------------------------------------------------------------------------*/
int RovhultAppl::compareCards (const CardWidget& lhs, const CardWidget& rhs) {
   unsigned int lhsValue ((lhs.number () == CardWidget::TWO) ? CardWidget::ACE + 1
                          : (lhs.number () == CardWidget::TEN) ? CardWidget::ACE + 2 :
                          lhs.number ());
   unsigned int rhsValue ((rhs.number () == CardWidget::TWO) ? CardWidget::ACE + 1
                          : (rhs.number () == CardWidget::TEN) ? CardWidget::ACE + 2 :
                          rhs.number ());
   TRACE3 ("RovhultAppl::compareCards (const CardWidget&, const CardWidget&) - "
           << lhsValue << " - " << rhsValue << " = " << (int)(lhsValue - rhsValue));

   return lhsValue - rhsValue;
}

/*--------------------------------------------------------------------------*/
//Purpose   : Exchanges the cards of the computer-players
//Parameters: player: Not really a void*, but actually the (next computer)player
/*--------------------------------------------------------------------------*/
void RovhultAppl::exchangeAutoplayerCards () {
   for (unsigned int i (1); i < NUM_PLAYERS; ++i) {
      for (unsigned int j (0); j < 3; ++j) {
         unsigned int posPile (0);
         unsigned int posHand (0);

         // Search for smallest card in pile and biggest in hand
         for (unsigned int k (1); k < 3; ++k) {
            if (compareCards (players[i].reserve[k].getTopCard (),
                              players[i].reserve[posPile].getTopCard ()) < 0)
               posPile = k;

            if (compareCards (players[i].hand.at (k), players[i].hand.at (posHand)) > 0)
               posHand = k;
         }

         // and exchange them, if hand is bigger than pile
         if (compareCards (players[i].reserve[posPile].getTopCard (),
                           players[i].hand.at (posHand)) < 0) {
            CardWidget& cardPile (players[i].reserve[posPile].removeTopCard ());
            CardWidget& cardHand (players[i].hand.remove (posHand, true));

            TRACE3 ("RovhultAppl::exchangeAutoplayerCards () - exchanging card "
                    << cardHand << " in hand (" << posHand << ") with card on pile "
                    << posPile << " (" << cardPile << ')');
            // Swap cards
            players[i].reserve[posPile].setTopCard (cardHand);
            players[i].hand.insert (cardPile, posHand);
         }
      }

      // Sort cards on piles
      for (int j (0); j < 2; ++j) {
         for (int k (j); k >= 0; --k) {
            if (compareCards (players[i].reserve[k + 1].getTopCard (),
                              players[i].reserve[k].getTopCard ()) < 0) {
               CardWidget& low (players[i].reserve[k + 1].removeTopCard ());
               CardWidget& high (players[i].reserve[k].removeTopCard ());

               TRACE3 ("RovhultAppl::exchangeAutoplayerCards () - exchanging card "
                       << low << " on pile " << (k + 1) << " with card " << high
                       << " on pile " << k);

               players[i].reserve[k + 1].setTopCard (high);
               players[i].reserve[k].setTopCard (low);
            }
         }
      }
   }
}

/*--------------------------------------------------------------------------*/
//Purpose   : Makes a move for a computer controlled player. If the next
//            player is human, enable its cards
/*--------------------------------------------------------------------------*/
int RovhultAppl::makeComputerMove () {
   static bool inTurn (false);
   if (inTurn) {
      Check (!"in Turn");
      return true;
   }

   inTurn = true;

   TRACE2 ("RovhultAppl::makeComputerMove (void*) - Start with player "
           << actPlayer);

   actPlayer = makeTurn (actPlayer);

   TRACE2 ("RovhultAppl::makeComputerMove (void*) - Next player: "
           << actPlayer);

   if (!actPlayer)
      enablePlayer (0);

   // Continue with computer-moves (means: let timer enabled), if computer
   // controlled players are on turn
   inTurn = false;
   return actPlayer > 0;
}

/*--------------------------------------------------------------------------*/
//Purpose   : Enables the cards of the passed player
//Parameters: player: Player to enable
/*--------------------------------------------------------------------------*/
void RovhultAppl::enablePlayer (unsigned int player) {
   disableLastPlayer (); Check3 (activeCards.empty ());

   if (players[player].hand.numberOfCards ()) {
      TRACE2 ("RovhultAppl::enablePlayer (unsigned int) - Hand of player "
           << player << " has " << players[player].hand.numberOfCards () << " card(s)");

      for (int i (players[player].hand.numberOfCards ()); i;)
         activeCards.push_back
            (players[player].hand.at (--i).clicked.connect_after
             (bind (slot (this, &RovhultAppl::handSelected), player, i)));
   }
   else {
      TRACE2 ("RovhultAppl::enablePlayer (unsigned int) - Enable reserve of player "
              << player);

      for (int i (0); i < 3; ++i)
         if (players[player].reserve[i].numberOfCards ()) {
            TRACE8 ("RovhultAppl::enablePlayer (unsigned int) - Pile " << i << " has "
                    << players[player].reserve[i].numberOfCards () << " card(s)");
            activeCards.push_back
               (players[player].reserve[i].getTopCard ().clicked.connect_after
                (bind (slot (this, &RovhultAppl::pileSelected), player, i)));
         }
   }

   if (played.numberOfCards ()) {
      TRACE2 ("RovhultAppl::enablePlayer (unsigned int) - Enable last played card for "
              "player " << player);
      activeCards.push_back (played.getTopCard ().clicked.connect_after
                             (bind (slot (this, &RovhultAppl::takeCards),
                                    player)));
   }
}

/*--------------------------------------------------------------------------*/
//Purpose   : Disables the cards of the passed player
/*--------------------------------------------------------------------------*/
void RovhultAppl::disableLastPlayer () {
   TRACE2 ("RovhultAppl::disableLastPlayer () - " << activeCards.size () << " cards");

   for (int i (activeCards.size ()); i > 0;) {
      activeCards[--i].disconnect ();
      activeCards.pop_back ();
   }
   Check3 (activeCards.empty ());
}

/*--------------------------------------------------------------------------*/
//Purpose   : Callback after clicking on a card on table
//Parameters: player: ID of player
//            pile: Offset of selected pile
/*--------------------------------------------------------------------------*/
void RovhultAppl::pileSelected (unsigned int player, unsigned int pile) {
   Check3 (player < NUM_PLAYERS); Check3 (pile < 3);

   CardWidget& card (players[player].reserve[pile].getTopCard ());
   TRACE1 ("Rovhult::pileSelected (unsigned int, unsinged int) - Card " << card);

   // If played from bottom of pile (with invisible cards): Flip card first
   if (players[player].reserve[pile].numberOfCards () == 1)
      card.setVisible ();
      
   if (!cardValid (card.number ())) {                 // Check if card is valid
      if (players[player].reserve[pile].numberOfCards () > 1)   // Visible? Ret
         return;
      
      players[player].reserve[pile].removeTopCard ();
      card.set_relief (GTK_RELIEF_NORMAL);
      played.append (card);
      if (executeMove (player, CardWidget::UNREACHABLE) > 0)
         makeComputerMoves ();
      return;
   }

   actPlayer = doPileSelected (player, pile);

   if (actPlayer > 0)
      // Start a timer to perform the computer-moves
      makeComputerMoves ();
   else
      if (!actPlayer)
         enablePlayer (0);
}

/*--------------------------------------------------------------------------*/
//Purpose   : Executes the move from a pile: Moves the cards and enables next
//Parameters: player: ID of player
//            pile: Offset of selected pile
//Returns   : int: player to continue
/*--------------------------------------------------------------------------*/
int RovhultAppl::doPileSelected (unsigned int player, unsigned int pile) {
   TRACE1 ("Rovhult::doPileSelected (unsigned int, unsinged int) - " 
           << player << '/' << pile);

   Check3 (player < NUM_PLAYERS); Check3 (pile < 3);
   Check3 (players[player].reserve[pile].numberOfCards ());

   // Move card (and visible cards with equal number below) from player to
   // played staple
   CardWidget& card (players[player].reserve[pile].removeTopCard ());
   if (card.number () == CardWidget::TEN)
      played.clear ();
   else {
      card.set_relief (GTK_RELIEF_NORMAL);
      played.append (card);
   }

   while (pile--) {
      if (players[player].reserve[pile].numberOfCards ()
          && players[player].reserve[pile].topCardVisible ()
          && (players[player].reserve[pile].getTopCard ().number () == card.number ())) {
         CardWidget& movedCard (players[player].reserve[pile].removeTopCard (true));
         if (movedCard.number () != CardWidget::TEN) {
            card.set_relief (GTK_RELIEF_NORMAL);
            played.append (movedCard);
         }
      }
   }

   return executeMove (player, card.number ());
}


/*--------------------------------------------------------------------------*/
//Purpose   : Check if played card is valid (equal or bigger)
//            The following cards have special meaning:
//              - 2: Can be played always
//              - 7: The next card must be equal or *smaller*
//              - 8: Skips the next player
//              -10: Clears the staple; the same player can continue with cards in hand
//Parameters: nr: Card to check
//            silent: Flag, if error should be displayed
//Returns   : bool: True, if card can be played
/*--------------------------------------------------------------------------*/
bool RovhultAppl::cardValid (CardWidget::NUMBERS nr, bool silent) const {
   switch (nr) {
   case CardWidget::TEN:
   case CardWidget::TWO:
      break;

   default:
      if (played.numberOfCards ()) {
         CardWidget& lastPlayed (played.getTopCard ());

         if (lastPlayed.number () == CardWidget::SEVEN) {
            if (nr > CardWidget::SEVEN) {
               if (!silent)
                  XMessageBox::Show (_("After a 7, the played card must be equal or smaller!"),
                                     _("Invalid move"), XMessageBox::ERROR | XMessageBox::OK);
               return false;
            }
         }
         else
            if (nr < played.getTopCard ().number ()) {
               if (!silent)
                  XMessageBox::Show (_("Played card must be equal or bigger!"),
                                     _("Invalid move"), XMessageBox::ERROR | XMessageBox::OK);
               return false;
            }
      }
   } // end-switch
   return true;
}

/*--------------------------------------------------------------------------*/
//Purpose   : Callback after clicking on a card in hand
//Parameters: player: ID of player
//            iCard: Offset of card in hand
/*--------------------------------------------------------------------------*/
void RovhultAppl::handSelected (unsigned int player, unsigned int pos) {
   TRACE3 ("Rovhult::handSelected (unsigned int, unsinged int) - Checking player "
           << player << "; Card at " << pos);
   Check3 (player < NUM_PLAYERS);
   Check3 (pos <= players[player].hand.numberOfCards ());

   CardWidget& card (players[player].hand.at (pos));
   TRACE1 ("Rovhult::handSelected (unsigned int, unsinged int) - Card " << pos
           << " = " << card);

   if (!cardValid (card.number ()))
       return;

   playCardsFromHand (player, pos);

   actPlayer = executeMove (player, card.number ());

   if (actPlayer > 0) {
      // Start a timer to perform the computer-moves
      makeComputerMoves ();
   }
   else
      if (!actPlayer)
         enablePlayer (0);
}

/*--------------------------------------------------------------------------*/
//Purpose   : Move card (and cards with equal number below) from player to
//            played staple. The cards are replaced, if the staple contains
//            cards
//Parameters: player: ID of player who played the last card
//            pos: Offset of card in hand
//Returns   : CardWidget::NUMBERS: Number of played card
/*--------------------------------------------------------------------------*/
CardWidget::NUMBERS RovhultAppl::playCardsFromHand (unsigned int player, unsigned int pos) {
   CardWidget& card (players[player].hand.at (pos));

   do {
      CardWidget& movedCard (players[player].hand.remove (pos, true));
      if (movedCard.number () == CardWidget::TEN)
         played.clear ();
      else {
         movedCard.set_relief (GTK_RELIEF_NORMAL);
         played.append (movedCard);
      }
   } while (pos-- && (players[player].hand.at (pos).number () == card.number ()));

   // If staple contains cards and no 10 was played (except if hand is empty):
   // Fill up cards til player has 3 (or one, in case of a ten)
   if ((card.number () != CardWidget::TEN) || (!players[player].hand.numberOfCards ()))
      fillUpPile (players[player].hand, card.number () != CardWidget::TEN ? 3 : 1);

   return card.number ();
}

/*--------------------------------------------------------------------------*/
//Purpose   : Executes the move -> Check consequences for next in round and
//            calculate next player
//Parameters: player: ID of player who played the last card
//            nr: Played card
//Returns   : int: The next player
/*--------------------------------------------------------------------------*/
int RovhultAppl::executeMove (unsigned int player, CardWidget::NUMBERS nr) {
   TRACE3 ("RovhultAppl::executeMove (unsigned int, CardWidget::NUMBERS) - Player "
           << player)
   Check3 (player < NUM_PLAYERS);

   status.pop (1);
   std::string stat;

   // If last 4 cards have the same number or ten was played: Don't increase
   // player (except of course, if actual player don't have anymore cards)
   if (!((nr == CardWidget::TEN) || clearPlayedIf4Equal ())
       || (player != nextAvailablePlayer ((player - 1) & 0x3))) {
      if (nr != CardWidget::UNREACHABLE)
         player = nextAvailablePlayer (player);

      if (nextAvailablePlayer (player) == -1) {
         stat = _("Player %1 lost");
         stat.replace (stat.find ("%1"), 2, (char)(player + '0'));
         status.push (1, stat);
         return -1;
      }

      if (nr == CardWidget::EIGHT) {
         stat = _("Skipping player %1; ");
         stat.replace (stat.find ("%1"), 2, (char)(player + '0'));
         player = nextAvailablePlayer (player);
      }

      // Check if next player has fitting card
      if (!playerCanContinue (player, nr)) {
         if (nr != CardWidget::UNREACHABLE) {
            stat = stat + _("Player %1 can't continue -> Getting whole pile. ");
            stat.replace (stat.find ("%1"), 2, (char)(player + '0'));
         }

         movePlayedCardsToLooser (player);
         player = nextAvailablePlayer (player);
      }
   }
   else
      stat = _("Pile cleared; ");

   stat = stat + _("Turn of player %1");
   stat.replace (stat.find ("%1"), 2, (char)(player + '0'));
   status.push (1, stat);
   return player;
}

/*--------------------------------------------------------------------------*/
//Purpose   : Callback after selection top card on played pile -> Moves all
//            its card to the passed player
//Parameters: player: ID of player picking up the cards
/*--------------------------------------------------------------------------*/
void RovhultAppl::takeCards (unsigned int player) {
   TRACE2 ("RovhultAppl::takeCards (unsigned int) - " << player);
   actPlayer = executeMove (player, CardWidget::UNREACHABLE);

   // Start a timer to perform the computer-moves
   makeComputerMoves ();
}

/*--------------------------------------------------------------------------*/
//Purpose   : Checks if the passed pile has a card which can be played
//Parameters: player: ID of player to analyze
//            card: Last played card
/*--------------------------------------------------------------------------*/
bool RovhultAppl::playerCanContinue (unsigned int player, CardWidget::NUMBERS card) const {
   TRACE3 ("RovhultAppl::playerCanContinue (unsigned int, CardWidget::NUMBERS) const - "
           << player << "; Card: " << card);
   Check3 (player < NUM_PLAYERS);

   if (card == CardWidget::UNREACHABLE)
      return false;
 
   if (players[player].hand.numberOfCards ())
      return playerHandCanContinue (players[player].hand, card);

   // Check pile: Analyze only visible cards; if there are none, return true
   bool hasNoVisibleCards (true);
   for (int i (0); i < 3; ++i)
      if (players[player].reserve[i].numberOfCards () > 1) {
         TRACE5 ("RovhultAppl::playerCanContinue (unsigned int, CardWidget::NUMBERS) const"
                 " - Checking pile " << i << "; "
                 << players[player].reserve[i].numberOfCards () << " cards");
         hasNoVisibleCards = false;

         CardWidget::NUMBERS nr (players[player].reserve[i].getTopCard ().number ());
         switch (nr) {
         case CardWidget::TWO:
         case CardWidget::TEN:
            return true;

         default:
            TRACE7 ("RovhultAppl::playerCanContinue (unsigned int, CardWidget::NUMBERS) const"
                    " - Value of card: " << nr);
            if ((card == CardWidget::SEVEN)
                ? (nr <= CardWidget::SEVEN) : (nr >= card))
               return true;
         } // end-switch card
      } // endif pile contains cards

   return hasNoVisibleCards ? (card != CardWidget::UNREACHABLE) : false;
}

/*--------------------------------------------------------------------------*/
//Purpose   : Checks if the passed pile has a card which can be played
//Parameters: player: ID of player to analyze
//            card: Last played card
/*--------------------------------------------------------------------------*/
bool RovhultAppl::playerHandCanContinue (const ICardPile& pile, CardWidget::NUMBERS card) const {
   TRACE3 ("RovhultAppl::playerHandCanContinue (const ICardPile&, CardWidget::NUMBERS) const"
           << " - Card " << card << " in " << pile.numberOfCards () << " cards");
   Check3 (pile.numberOfCards ());

   // Check if first/last is smaller/bigger then passed one
   if (card == CardWidget::SEVEN) {
      if (pile.at (0).number () <= card)
         return true;
   }
   else {
      if (pile.at (pile.numberOfCards () - 1).number () >= card)
         return true;
   }

   TRACE3 ("RovhultAppl::playerHandCanContinue (const ICardPile&, CardWidget::NUMBERS"
           " - Special check");
   // Simple check failed -> Check for special card (2 or 10)
   if (pile.at (0).number () == CardWidget::TWO)
      return true;

   return pile.exists (CardWidget::TEN);
}

/*--------------------------------------------------------------------------*/
//Purpose   : Fills up the passed pile til it contains the specified number
//            of cards
//Parameters: pile: Pile to fill up
//            minCards: Minimal number of cards pile should hold
/*--------------------------------------------------------------------------*/
void RovhultAppl::fillUpPile (ICardPile& pile, unsigned int minCards) {
   TRACE3 ("Rovhult::fillUpPile (ICardPile&, unsinged int) - "
           << pile.numberOfCards () << " -> " << minCards);

   while ((pile.numberOfCards () < minCards) && staple.numberOfCards ()) {
      CardWidget& newCard (staple.removeTopCard ());
      newCard.setVisible ();
      pile.insertSorted (newCard);

      TRACE8 ("Rovhult::fillUpPile (ICardPile&, unsinged int) - Appended card "
              << newCard);
   }
}

/*--------------------------------------------------------------------------*/
//Purpose   : Returns the number of equal cards from the played pile
//Returns   : unsigned int: Number of equal cards
/*--------------------------------------------------------------------------*/
unsigned int RovhultAppl::numberOfEqualTopCards () const {
   TRACE8 ("Rovhult::numberOfEqualTopCards () const");
   unsigned int nrCards (played.numberOfCards ());

   if (nrCards)
      --nrCards;
   else
      return 0;

   int i (1);
   CardWidget& card (played.getTopCard ());
   while (i <= nrCards) {
      TRACE9 ("Rovhult::numberOfEqualTopCards () const - Checking "
              << played.at (nrCards - i) << " with " << card);
      
      if (played.at (nrCards - i).number () != card.number ()) {
         TRACE8 ("Rovhult::numberOfEqualTopCards () const - found " << i);
         break;
      }
      ++i;
   }
   return i;
}

/*--------------------------------------------------------------------------*/
//Purpose   : Clears the played staple if the last 4 cards are equal
//Returns   : bool: True, if 4 equal cards found
/*--------------------------------------------------------------------------*/
bool RovhultAppl::clearPlayedIf4Equal () {
   TRACE8 ("Rovhult::clearPlayedIf4Equal ()");

   int cards (numberOfEqualTopCards ()); Check3 (cards <= 4);
   if (cards < 4)
      return false;

   TRACE7 ("Rovhult::clearPlayedIf4Equal () - found 4");
   played.clear ();
   return true;
}

/*--------------------------------------------------------------------------*/
//Purpose   : Method to move the cards of the actual round to the winner
//Parameters: nrLooser: Nr. of player getting all played cards
/*--------------------------------------------------------------------------*/
void RovhultAppl::movePlayedCardsToLooser (unsigned int nrLooser) {
   TRACE8 ("Rovhult::movePlayedCardsToLooser () - Player " << nrLooser << " gets "
           << played.numberOfCards () << " cards");
   Check3 (nrLooser < NUM_PLAYERS);

   while (played.numberOfCards ())
      players[nrLooser].hand.insertSorted (played.remove (0));
}

/*--------------------------------------------------------------------------*/
//Purpose   : Checks which player has still cards left
//Parameters: actPlayer: ID of actual player
//Returns   : int: ID of player or -1 (if none can continue)
/*--------------------------------------------------------------------------*/
int RovhultAppl::nextAvailablePlayer (unsigned int actPlayer) const {
   // We assume (without checking), that acutal player still has cards
   for (unsigned int i (1); i < NUM_PLAYERS; ++i) {
      unsigned int player ((actPlayer + i) & 0x3);

      if (players[player].hand.numberOfCards ())
         return player;

      for (int j (0); j < 3; ++j)
         if (players[player].reserve[j].numberOfCards ())
            return player;
   }
   return -1;
}

/*--------------------------------------------------------------------------*/
//Purpose   : Loads the cards (from xpm-files)
/*--------------------------------------------------------------------------*/
void RovhultAppl::loadCards () {
   Check3 (staple.is_realized ());

   gdk_threads_enter ();
   status.push (1, _("Loading cardimages ..."));
   gdk_threads_leave ();

   cardFaces.load (staple.get_window ());  // Cards need an realized (!) parent
   cards.addPacket (cardFaces);
   mutexThread.lock ();
   pThread = NULL;
   mutexThread.unlock ();

   gdk_threads_enter ();
   pMenuNew->set_sensitive (true);
   status.pop (1);
   status.push (1, _("Start a new game with Ctrl+N (or Game -> New)"));

   assert (cardFaces.numberOfCards ());
   const Gdk_Pixmap& img (cardFaces.getCardImage (0));
   unsigned int width (const_cast<Gdk_Pixmap&> (img).width ());
   unsigned int height (const_cast<Gdk_Pixmap&> (img).height ());

   for (int i (0); i < NUM_PLAYERS; ++i) {
      for (int j (0); j < 3; ++j)
         players[i].reserve[j].set_usize (width, height + 20);

      players[i].hand.set_usize (width * 3, height);
   }
   played.set_usize (width, height);
   staple.set_usize (width, height);
   gdk_threads_leave ();
}

/*--------------------------------------------------------------------------*/
//Purpose   : Shuffles (Randomizes) the cards onto the staple
/*--------------------------------------------------------------------------*/
void RovhultAppl::fillStaple () {
   // Randomize and put cards onto staple
   cards.shuffle ();
   staple.setTopCards (cards.getCards (), false);
}

/*--------------------------------------------------------------------------*/
//Purpose   : Remove cards from everything which can hold them
/*--------------------------------------------------------------------------*/
void RovhultAppl::cleanTable () {
   staple.clear ();                                             // Clear staple
   for (int i (0); i < NUM_PLAYERS; ++i) {            // Clear cards of players
      for (int j (0); j < 3; ++j) {
         players[i].reserve[j].clear ();
      }

      players[i].hand.clear ();
   }
   played.clear ();

   players[0].hand.setStyle (ICardPile::NORMAL);
   disableLastPlayer ();
}

/*--------------------------------------------------------------------------*/
//Purpose   : Prepares the card for drag´n´drop (starting from the table,
//            ending on hand or ending on table, starting from hand)
//Parameters: card: Card to prepare for drag´n´drop
//            player: Number of player holding card
//            pile: Number of pile on reserve holding card
/*--------------------------------------------------------------------------*/
void RovhultAppl::registerTableDND (CardWidget& card, unsigned int player,
                                    unsigned int pile) {
   static Gdk_Colormap color (get_colormap ());
   static Gdk_Bitmap bitmap;

   // Card accepts drops from hand and drags from table
   card.drag_dest_set (GTK_DEST_DEFAULT_ALL, &dndTypeHand, 1, GDK_ACTION_COPY);
   card.drag_source_set (GDK_BUTTON1_MASK, &dndTypeTable, 1, GDK_ACTION_COPY);
   card.drag_source_set_icon
      (color, const_cast<Gdk_Pixmap&> (cardFaces.getCardImage (card.id ())),
       bitmap);
   card.drag_data_received.connect
      (bind (slot (this, &RovhultAppl::cardDroppedOnTable), (player << 16) + pile));
   card.drag_data_get.connect (bind (slot (this, &RovhultAppl::getDropData),
                                     player, pile));
}

/*--------------------------------------------------------------------------*/
//Purpose   : Prepares the card for drag´n´drop (starting from the hand
//            ending on table or ending on hand, starting from table)
//Parameters: card: Card to prepare for drag´n´drop
//            player: Number of player holding card
//            pile: Number of pile on reserve holding card
/*--------------------------------------------------------------------------*/
void RovhultAppl::registerHandDND (CardWidget& card, unsigned int player,
                                    unsigned int iCard) {
   static Gdk_Colormap color (get_colormap ());
   static Gdk_Bitmap bitmap;

   // Card accepts drops from table and drags from hand
   card.drag_dest_set (GTK_DEST_DEFAULT_ALL, &dndTypeTable, 1, GDK_ACTION_COPY);
   card.drag_source_set (GDK_BUTTON1_MASK, &dndTypeHand, 1, GDK_ACTION_COPY);
   card.drag_source_set_icon
      (color, const_cast<Gdk_Pixmap&> (cardFaces.getCardImage (card.id ())),
       bitmap);
   card.drag_data_received.connect
      (bind (slot (this, &RovhultAppl::cardDroppedOnHand), (player << 16) + iCard));
   card.drag_data_get.connect (bind (slot (this, &RovhultAppl::getDropData),
                                     player, iCard));
}

/*--------------------------------------------------------------------------*/
//Purpose   : Stops the drag´n´drop abilities of the passed card
//Parameters: card: Card to unregister of dnd
/*--------------------------------------------------------------------------*/
void RovhultAppl::unregisterDND (CardWidget& card) const {
   card.drag_dest_unset ();
   card.drag_source_unset ();
}

/*--------------------------------------------------------------------------*/
//Purpose   : Deals the cards
/*--------------------------------------------------------------------------*/
void RovhultAppl::dealCards () {
   TRACE9 ("RovhultAppl::dealCards ()");
   Check3 (staple.numberOfCards () > 36);

   // Show cards on table: For all players put 6 cards on table (only the
   // (upper visible) and 3 (visible ones) in hand
   for (unsigned int i (0); i < NUM_PLAYERS; ++i)
      for (unsigned int j (0); j < 3; ++j) {
         for (unsigned int k (0); k < 2; ++k) {           // Set cards on table
            CardWidget& card (staple.removeTopCard ());
            players[i].reserve[j].setTopCard (card, k);

            if (k && !i)      // Enable drag-n-drop for the top-card of human
               registerTableDND (card, i, j);
         } // end-for two cards pro pile (in reserve)

         // Put card into hand
         CardWidget& card (staple.removeTopCard ());
         card.setVisible ();
         players[i].hand.append (card);

         if (!i)
            registerHandDND (card, 0, j);
      }

   played.hide ();

   Check3 (staple.numberOfCards ());
   CardWidget& card (staple.getTopCard ());
   pileTop = card.clicked.connect (slot (this, &RovhultAppl::finishedExchange));
   card.add_accelerator ("clicked", *get_accel_group (), ' ', 0, GtkAccelFlags (0));

   status.pop (1);
   status.push (1, _("Exchange the cards in your hand with the one on the "
                     "table (with drag and drop) - press space (or click on staple) "
                     "if finished"));
}

/*--------------------------------------------------------------------------*/
//Purpose   : Callback after dropping a card onto (cards on) table
//Parameters: pContext: Context of the drag (contains things like source,
//                      target, action, ...)
//            pData: Describes the thing which was dropped
//            info: Describes the type of pData (should be 0)
//            time: Timestamp of the drag
//            player: Number of player
//            pile: Number of pile
//Requieres : pContext, pData not NULL; Expects info to be 0
/*--------------------------------------------------------------------------*/
void RovhultAppl::cardDroppedOnTable (GdkDragContext* pContext, gint, gint,
                                      GtkSelectionData* pData, guint info,
                                      guint32 time, unsigned int playerPile) {
   if (info == 1)
      return;

   Check3 (pContext); Check3 (pData);
   Check3 (!pContext->is_source);
   Check3 (pData->length == (sizeof (int) << 1));
   Check3 (pData->format == 8);

   unsigned int player (playerPile >> 16);
   unsigned int pile (playerPile & 0xffff);

   Check3 (player < NUM_PLAYERS);
   Check3 (pile < 3);
   
   unsigned int* pValues (reinterpret_cast <unsigned int*> (pData->data));
   Check3 (pValues);

   TRACE1 ("RovhultAppl::cardDroppedOnTable (...) - Data = "
           << *pValues << '/' << pValues[1] << " <-> " << player << '/' << pile);

   Gdk_DragContext gdc (pContext);

   // Check if player matches
   if (player != *pValues)
      drag_finish (gdc, false, false, time);
   else {
      CardWidget& cardTable (players[player].reserve[pile].removeTopCard ());
      CardWidget& cardHand (players[player].hand.remove (pValues[1]));

      TRACE1 ("RovhultAppl::cardDroppedOnTable (...) - Exchanging cards "
              << cardHand.id () << "<->" << cardTable.id ());

      // End old dnd
      drag_finish (gdc, true, false, time);

      unregisterDND (cardHand);
      unregisterDND (cardTable);

      players[player].reserve[pile].setTopCard (cardHand);
      players[player].hand.insert (cardTable, pValues[1]);

      registerHandDND (cardTable, player, pValues[1]);
      registerTableDND (cardHand, player, pile);
   }
}

/*--------------------------------------------------------------------------*/
//Purpose   : Callback after dropping a card onto onto hand
//Parameters: pContext: Context of the drag (contains things like source,
//                      target, action, ...)
//            pData: Describes the thing which was dropped
//            info: Describes the type of pData (should be 0)
//            time: Timestamp of the drag
//            player: Number of player
//            card: Number of card
//Requieres : pContext, pData not NULL; Expects info to be 0
/*--------------------------------------------------------------------------*/
void RovhultAppl::cardDroppedOnHand (GdkDragContext* pContext, gint, gint,
                                     GtkSelectionData* pData, guint info,
                                     guint32 time, unsigned int playerCard) {
   if (!info)
      return;

   Check3 (pContext); Check3 (pData);
   Check3 (!pContext->is_source);
   Check3 (pData->length == (sizeof (int) << 1));
   Check3 (pData->format == 8);

   unsigned int player (playerCard >> 16);
   unsigned int card (playerCard & 0xffff);

   Check3 (player < NUM_PLAYERS);
   Check3 (card < players[player].hand.numberOfCards ());

   unsigned int* pValues (reinterpret_cast <unsigned int*> (pData->data));
   Check3 (pValues);

   TRACE1 ("RovhultAppl::cardDroppedOnHand (...) - Data = "
           << *pValues << '/' << pValues[1] << " <-> " << player << '/' << card);

   Gdk_DragContext gdc (pContext);

   // Check if player matches
   if (player != *pValues)
      drag_finish (gdc, false, false, time);
   else {
      CardWidget& cardTable (players[player].reserve[pValues[1]].removeTopCard ());
      CardWidget& cardHand (players[player].hand.remove (card));

      TRACE1 ("RovhultAppl::cardDroppedOnHand (...) - Exchanging cards "
              << cardHand.id () << "<->" << cardTable.id ());

      // End old DND
      drag_finish (gdc, true, false, time);
      unregisterDND (cardHand);
      unregisterDND (cardTable);

      // Swap cards
      players[player].reserve[pValues[1]].setTopCard (cardHand);
      players[player].hand.insert (cardTable, card);

      // Adapt dnd-settigns
      registerHandDND (cardTable, player, card);
      registerTableDND (cardHand, player, pValues[1]);
   }
}

/*--------------------------------------------------------------------------*/
//Purpose   : Callback to query the data to drop
//Parameters: pContext: Context of the drag (contains things like source,
//                      target, action, ...)
//            pData: Describes the thing which was dropped
//            time: Timestamp of the drag
//            player: Number of player
//            cardPos: Position of card (either in hand or pile on table)
//Requieres : pContext, pData not NULL; Expects info to be 0
/*--------------------------------------------------------------------------*/
void RovhultAppl::getDropData (GdkDragContext* pContext, GtkSelectionData* pData,
                               guint info, guint32 time, unsigned int player,
                               unsigned int cardPos) {
   Check3 (pContext); Check3 (pData); Check3 (info < 2);
   Check3 (pContext->is_source);

   unsigned int data[] = { player, cardPos };
   gtk_selection_data_set (pData, pData->target, 8, reinterpret_cast <guchar*> (data),
                           sizeof (data));
}

/*--------------------------------------------------------------------------*/
//Purpose   : Retrieves the card at given position. If there are cards in the
//            hand, they are considered; else the cards in the reserve
//Parameters: player: Player in turn
//            pos: Position of (last) card to play
//Returns   : CardWidget&: Reference to specified card
/*--------------------------------------------------------------------------*/
CardWidget& RovhultAppl::cardAtPos (unsigned int player, unsigned int pos) const {
   Check3 ((players[player].hand.numberOfCards ())
           ? (pos < players[player].hand.numberOfCards ())
           : (pos < 3));

   return ((players[player].hand.numberOfCards ())
           ? players[player].hand.at (pos)
           : players[player].reserve[pos].getTopCard ());
}

/*--------------------------------------------------------------------------*/
//Purpose   : Flips the cards the user is about to play (and the ones with
//            similar numbers below)
//Parameters: player: Player in turn
//            pos: Position of (last) card to play
/*--------------------------------------------------------------------------*/
void RovhultAppl::flipCards2Play (unsigned int player, unsigned int pos) {
   TRACE2 ("RovhultAppl::flipCards2Pplay (unsigned int, unsigned int) - "
           " For player " << player << " at position " << pos);
   Check3 (player < NUM_PLAYERS);
   Check3 ((players[player].hand.numberOfCards ())
           ? (players[player].hand.numberOfCards () > pos)
           : (pos < 3));

   CardWidget* card (&cardAtPos (player, pos));
   do {
      card->setVisible ();
      card->set_relief (GTK_RELIEF_NONE);
      if (card->width () < card->getImageWidth ())
         played.resize (*card, ICardPile::COMPRESSED);
   } while (pos
            && (card->number () == ((card = &cardAtPos (player, --pos))->number ())));
}

/*--------------------------------------------------------------------------*/
//Purpose   : Finds an executes the turn of a (computer control.ed) player
//Returns   : int: The next player
/*--------------------------------------------------------------------------*/
int RovhultAppl::makeTurn (unsigned int player) {
   TRACE2 ("RovhultAppl::makeTurn (unsigned int) - Player " << player);
   static int pos2Play (-1);

   if (pos2Play == -1) {
      pos2Play = findCard2Play (player);
      flipCards2Play (player, pos2Play);
      return player;
   }
   else {
      TRACE2 ("RovhultAppl::makeTurn (unsigned int) - play card " << pos2Play);

      unsigned int pos (pos2Play);
      pos2Play = -1;
      if (players[player].hand.numberOfCards ()) {
         if (players[player].hand.at (pos).number () == CardWidget::TEN)
            played.clear ();

         CardWidget::NUMBERS nr (playCardsFromHand (player, pos));
         return executeMove (player, nr);
      }
      else {
         // If cards are visible
         if (players[player].reserve[pos].numberOfCards () > 1) {
            doPileSelected (player, pos);                       // Execute move
            return actPlayer;
         }
         else {
            // If card is valid: Play it
            CardWidget& card (players[player].reserve[pos].getTopCard ());
            if (cardValid (card.number (), true)) {
               doPileSelected (player, pos);
               return actPlayer;
            }
            else {
               // Card is not valid: Take up pile
               players[player].reserve[pos].removeTopCard ();
               card.set_relief (GTK_RELIEF_NORMAL);
               played.append (card);
            }
         }

         // Either invisible card was invalid or no visible card is playable
         // (although you shouldn't then reach this part anyway): Take up pile
         return executeMove (player, CardWidget::UNREACHABLE);
      }
   }
}

/*--------------------------------------------------------------------------*/
//Purpose   : Finds the next card to play (for a computer controlled player)
//Parameters: player: Player to inspect
//            pos: Upper position of cards to inspect
//Returns   : int: Position of card to play
/*--------------------------------------------------------------------------*/
bool RovhultAppl::existOnlySpecialCards (unsigned int player, unsigned int pos) const {
   Check3 (player < NUM_PLAYERS);
   Check3 ((players[player].hand.numberOfCards ())
           ? (pos < players[player].hand.numberOfCards ())
           : (pos < 3));

   do {
       CardWidget::NUMBERS nr (cardAtPos (player, pos).number ());
       if ((nr != CardWidget::TEN) && (nr != CardWidget::TWO))
          return false;
   } while (--pos);

   return true;
}

/*--------------------------------------------------------------------------*/
//Purpose   : Finds the next card to play (for a computer controlled player)
//Parameters: player: Player to inspect
//Returns   : int: Position of card to play
/*--------------------------------------------------------------------------*/
int RovhultAppl::findCard2Play (unsigned int player) const {
   TRACE2 ("RovhultAppl::findCard2Play (unsigned int) - Player " << player);
   Check3 (player < NUM_PLAYERS);

   // Search for minimal card to play; this is either a card equal or bigger
   // or - if no previous card is played or the last card played was a 7 -
   // the smallest available
   CardWidget::NUMBERS cardMin (CardWidget::THREE);
   if (played.numberOfCards ()
       && (played.getTopCard ().number () != CardWidget::SEVEN)
       && (played.getTopCard ().number () != CardWidget::TWO))
      cardMin = played.getTopCard ().number ();

   TRACE5 ("RovhultAppl::findCard2Play (unsigned int) - Card to beat " << cardMin);

   // Check if to play from hand or to play from reserve
   unsigned int nrCards (players[player].hand.numberOfCards ());
   if (nrCards) {
      unsigned int pos (players[player].hand.findFirstEqualOrBigger (cardMin));

      // Check if no matching normal card is found or found card is bigger than
      // the played 7. If so, use special card instead
      // We know one card must match as "playerCanContinue" reported this player
      // as valid
      if ((pos == (unsigned int)-1)
          || (played.numberOfCards ()
              && ((played.getTopCard ().number () == CardWidget::SEVEN)
                  && players[player].hand.at (pos).number () > CardWidget::SEVEN))) {
         if (players[player].hand.at (0).number () == CardWidget::TWO)
            pos = 0;
         else {
            pos = players[player].hand.findFirstEqualOrBigger (CardWidget::TEN);
            Check3 (pos != -1);
            Check3 (players[player].hand.at (pos).number () == CardWidget::TEN);
         }
      }
      else {
         TRACE5 ("RovhultAppl::findCard2Play (unsigned int) - Continuing with card "
                 << players[player].hand.at (pos) << " at pos " << pos);
   
         Check3 ((cardMin == CardWidget::SEVEN)
                 ? (players[player].hand.at (pos).number () <= CardWidget::SEVEN)
                 : (players[player].hand.at (pos).number () >= cardMin));

         // The search of CardWidget does not know (and shall not know anything)
         // about the special meaning of the tens, so skip them by yourself
         if (players[player].hand.at (pos).number () == CardWidget::TEN) {
            unsigned int npos = players[player].hand.findLastEqual (pos);
            if ((npos + 1) < nrCards)
               pos = npos + 1;
         }
      }

      // Now find the last of equal cards; get rid of all of them if:
      // - it's not the highest card
      // - it´s the first card
      // - it's a not that high card (up to 9)
      // - they would complete 4
      // - the lower cards are only special cards
      unsigned int npos (players[player].hand.findLastEqual (pos));
      if ((npos != (players[player].hand.numberOfCards () - 1))
          || (pos == 0)
          || (players[player].hand.at (npos).number () < CardWidget::TEN)
          || ((numberOfEqualTopCards () + npos - pos) == 4)
          || (existOnlySpecialCards (player, pos - 1)))
         pos = npos;

      return pos;
   }
   else {
      // Play first visible cards
      bool cardVisible (false);
      unsigned int i (0);
      for (; i < 3; ++i) {
         if (players[player].reserve[i].numberOfCards () > 1) {
            cardVisible = true;

            // If card can be played: Search for last equal card
            CardWidget& actCard (players[player].reserve[i].getTopCard ());
            if (cardValid (actCard.number (), true)) {
               while ((i < 2)
                      && (players[player].reserve[i + 1].getTopCard ().number ()
                          == actCard.number ()))
                  ++i;

               return i;
            }
         }
      }

      // No card visible: Play the first
      if (!cardVisible) {
         unsigned int i (0);
         while (!players[player].reserve[i].numberOfCards ())
            ++i;

         players[player].reserve[i].getTopCard ().setVisible ();
         return i;
      }
   }
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
   RovhultAppl win;

   gdk_threads_enter ();
   appl.run ();
   gdk_threads_leave ();
   return 0;
}
