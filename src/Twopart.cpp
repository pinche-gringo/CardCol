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

#include <fstream>

#include <Check.h>
#include <Trace_.h>

#include <XAbout.h>
#include <INIFile.h>
#include <XMessageBox.h>
#include <Cardset-config.h>

#include <CardWidget.h>
#include "Twopart.h"


const std::string Twopart::NAME_INIFILE = PathSearch::expandNode ("~/.cardgames");

const unsigned int Twopart::COLS_PLAYER[NUM_PLAYERS] = { 7, 13, 7, 1 };
const unsigned int Twopart::ROWS_PLAYER[NUM_PLAYERS] = { 4,  7, 8, 7 };

char Twopart::sortOrder[4];

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
      _("_Game")),            _("<alt>G"), 0,        BRANCH },
    { _("_New"),              _("<ctl>N"), NEW,      ITEM },
    { _("_End"),              _("<ctl>E"), END,      ITEM },
    { "",                     "",          0,        SEPARATOR },
    { _("E_xit"),             _("<ctl>Q"), EXIT,     ITEM },
    { _("_Options"),          _("<alt>O"), 0,        BRANCH },
    { _("_Change decks ..."), _("<ctl>C"), CHGDECKS, ITEM },
    { _("_Save settings"),    _("<ctl>S"), SAVESET,  ITEM },
#if TRACELEVEL > 0
    { _("_Debug"),            _("<ctl>D"), DEBUG,    CHECKITEM },
#endif
    { _("_Help"),             _("<alt>H"), 0,        LASTBRANCH },
    { _("_About..."),         _("<ctl>A"), ABOUT,    ITEM } };


/*--------------------------------------------------------------------------*/
//Purpose   : Defaultconstructor; all widget are created
/*--------------------------------------------------------------------------*/
Twopart::Twopart ()
   : XApplication (PACKAGE " - Twopart V" PRG_RELEASE), status ()
     , tblTable (11, 7), cardFaces (USED_CARDS), cards (), pThread (NULL)
     , staple (ICardPile::VERY_COMPRESSED, ICardPile::SHOWBACK)
     , played (ICardPile::COMPRESSED, ICardPile::SHOWFACE)
     , bfPlayers ((1 << NUM_PLAYERS) - 1), pTrump (NULL), offPos (0)
     , bfOldPlayers (bfPlayers), statGame (INITIALIZING) {
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
   pMenuEnd = addMenu (menuItems[2]); Check3 (pMenuEnd);
   pMenuEnd->set_sensitive (false);
   addMenus (menuItems + 3, sizeof (menuItems) / sizeof (menuItems[0]) - 3);

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

   for (unsigned int i (0); i < NUM_PLAYERS; ++i) {
      players[i].won.setShowOption (ICardPile::SHOWBACK);
      players[i].hand.setShowOption (i ? ICardPile::SHOWBACK : ICardPile::SHOWFACE);
   }
}

/*--------------------------------------------------------------------------*/
//Purpose   : Destructor
/*--------------------------------------------------------------------------*/
Twopart::~Twopart () {
   TRACE9 ("Twopart::~Twopart ()");
}

/*--------------------------------------------------------------------------*/
//Purpose   : Starts the game
/*--------------------------------------------------------------------------*/
void Twopart::startGame () {
   statGame = PLAYING;
   pMenuEnd->set_sensitive (true);
   cleanTable ();
   fillStaple ();
   dealCards ();
}

/*--------------------------------------------------------------------------*/
//Purpose   : Command-handler
//Parameters: menu: ID of command (menu)
/*--------------------------------------------------------------------------*/
void Twopart::command (int menu) {
   switch (menu) {
   case NEW:
      if (statGame >= PLAYING) {
         restart = true;
         XMessageDialog<Twopart>::Show (*this, &Twopart::userWants2End,
                                        _("A game is already running. Do you really"
                                          " want to end it and start another?"),
                                        PACKAGE " - Twopart",
                                        XMessageBox::QUESTION | XMessageBox::YESNO);
      }
      else
         startGame ();
      break;

   case END:
      Check3 (statGame >= PLAYING);
      restart = false;
      XMessageDialog<Twopart>::Show (*this, &Twopart::userWants2End,
                                     _("Do you really want to end the game?"),
                                     PACKAGE " - Twopart",
                                     XMessageBox::QUESTION | XMessageBox::YESNO);
      break;

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

   case CHGDECKS:
      dlgChgDecks = CarddeckSelectDlg<Twopart>::create (*this, &Twopart::changeDecks,
                                                        CARDSET_PATH,
                                                        pathDeck, pathBack);
      break;

   case SAVESET: {
      TRACE2 ("Twopart::command () - Save file " << NAME_INIFILE);
      ofstream inifile (NAME_INIFILE.c_str ());
      
      inifile << "[Decks]\nFront=" << pathDeck << "\nBack=" << pathBack << '\n';
      break;
      }

#if TRACELEVEL > 0
   case DEBUG: {
      ICardPile::ShowOpt show (players[0].won.getShowOption () == ICardPile::SHOWFACE
                               ? ICardPile::SHOWBACK : ICardPile::SHOWFACE);
      for (int i (0); i < NUM_PLAYERS; ++i) {
         players[i].won.setShowOption (show);
         players[i].won.setStyle ((show == ICardPile::SHOWFACE)
                                  ? ICardPile::COMPRESSED
                                  : ICardPile::VERY_COMPRESSED);
         players[i].hand.setShowOption (i ? show : ICardPile::SHOWFACE);
      }
      }
      break;

#endif
   default:
      Check3 (0);
   } // end-switch
}

/*--------------------------------------------------------------------------*/
//Purpose   : Callback to change the carddecks
//Parameters: cmd: 
/*--------------------------------------------------------------------------*/
void Twopart::changeDecks (ICarddeckSelectDlg::commands cmd) {
   TRACE2 ("Twopart::changeDecks (ICarddeckSelectDlg::commands) - Command "
           << cmd);
   Check3 (dlgChgDecks);

   if (cmd != ICarddeckSelectDlg::CANCEL) {
      std::string deck, back;
      dlgChgDecks->getSelection (deck, back);
      TRACE3 ("Twopart::changeDecks (ICarddeckSelectDlg::commands) - Use "
              << deck << " and " << back);

      unsigned int opt (0);
      if (deck != pathDeck) {
         pathDeck = deck;
         opt |= 1;
      }
      if (back != pathBack) {
         pathBack = back;
         opt |= 2;
      }
      
      if (opt) {
         pThread = THRDAPPL::create (*this,
                                     (THRDAPPL::THREAD_OBJMEMBER)&Twopart::changeCards,
                                     (void*)opt);
         TRACE9 ("Twopart::Twopart () - Thread-ID = " << pThread->getID ());
      }
   }

   if (cmd != ICarddeckSelectDlg::APPLY)
      dlgChgDecks = NULL;
}

/*--------------------------------------------------------------------------*/
//Purpose   : Checks the user-input after asking if he wants to end the game;
//            depending on the answer either stops or continues
//Parameters: input: Button pressed by the user
/*--------------------------------------------------------------------------*/
void Twopart::userWants2End (unsigned int input) {
   if (input == XMessageBox::YES) {
      pMenuEnd->set_sensitive (false);
      disableLastPlayer ();

      if (statGame >= PLAYING) {
         status.pop (1);
         status.push (1, _("User canceled"));

         if (actPlayer)
            statGame = TOSTOP;
         else {
            statGame = STOPPED;
         
            if (restart)
               startGame ();
            else
               disableLastPlayer ();
         }
      }
   }
}

/*--------------------------------------------------------------------------*/
//Purpose   : Enables the cards of the actual player
/*--------------------------------------------------------------------------*/
int Twopart::enableActPlayer () {
   enablePlayer (actPlayer);
   return 0;
}

/*--------------------------------------------------------------------------*/
//Purpose   : Enables the cards of the passed player
//Parameters: player: Player to enable
//Remarks   : Depending of the status of the game (PLAYING2) also the top
//            card of the played pile are enabled
/*--------------------------------------------------------------------------*/
void Twopart::enablePlayer (unsigned int player) {
   Check3 (activeCards.empty ());
   Check3 (statGame >= PLAYING);

   TRACE2 ("Twopart::enablePlayer (unsigned int) - player "
           << player << " has " << players[player].hand.numberOfCards ()
           << " cards");

   for (int i (players[player].hand.numberOfCards ()); i;)
      activeCards.push_back
         (players[player].hand.at (--i).clicked.connect_after
          (bind (slot (this, (&Twopart::cardSelected)), player, i)));

   if ((statGame == PLAYING2)
       && (played.numberOfCards ()))
      pileTop = played.getTopCard ().clicked.connect_after
         (bind (slot (this, (&Twopart::playedSelected)), player));
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
   Check9 (activeCards.empty ());

   pileTop.disconnect ();
}

/*--------------------------------------------------------------------------*/
//Purpose   : Moving played cards (of last person) to the passed player
//Parameters: player: ID of player who should get the cards
//Requieres : Only for part 2 of the game
/*--------------------------------------------------------------------------*/
void Twopart::pickUpPlayedPile (unsigned int player) {
   TRACE3 ("Twopart::pickUpPlayedPile (unsigned int) - Player " << player
           << " picks up played pile");
   Check3 (statGame == PLAYING2);
   Check3 (bfPlayers);

   // Move played cards to player
   Check3 (offPos > 0); Check3 (offPos < NUM_PLAYERS);
   movePlayedCardsToPlayer (player, startPos[--offPos]);

   // Re-enable next two players (having cards); continue with first of them
   removePlayer (player);
   unsigned int next;
   unsigned int cAdded (0);
   for (unsigned int i (0); i < NUM_PLAYERS; ++i)
      if ((!(bfPlayers & (1 << (next = (actPlayer + i + 1) & 0x3))))
          && players[next].hand.numberOfCards ()) {
         TRACE5 ("Twopart::pickUpPlayedPile (unsigned int) - Re-adding player "
                 << next);
         addPlayer (next);
         if (++cAdded == 2)
            break;
      }
   actPlayer = findNextPlayer (player);

   TRACE7 ("Twopart::pickUpPlayedPile (unsigned int) - Continuing with player "
           << actPlayer);
}

/*--------------------------------------------------------------------------*/
//Purpose   : Callback after clicking the top card of the played pile
//Parameters: player: ID of player
//Requieres : Only for part 2 of the game
/*--------------------------------------------------------------------------*/
void Twopart::playedSelected (unsigned int player) {
   TRACE3 ("Twopart::playedSelected (unsigned int) - Player " << player
           << " picks up played pile");
   Check3 (statGame == PLAYING2);
   Check3 (bfPlayers);

   pickUpPlayedPile (player);
   makeNextMoves ();
   disableLastPlayer ();
}

/*--------------------------------------------------------------------------*/
//Purpose   : Callback after clicking on a card in hand
//Parameters: player: ID of player
//            iCard: Offset of card in hand
//Returns   : bool: Status of moving; true: Card could be moved; false else
/*--------------------------------------------------------------------------*/
bool Twopart::moveSelectedCardToPlayed (unsigned int player, unsigned int pos) {
   Check3 (player <= NUM_PLAYERS);
   Check3 (pos <= players[player].hand.numberOfCards ());
   TRACE5 ("Twopart::moveSelectedCardToPlayed (unsigned int, unsigned int) - Player: "
           << player << " at position " << pos);

   Check3 (statGame >= PLAYING);

   if (statGame == PLAYING) {
      CardWidget& card (players[player].hand.remove (pos));
      TRACE9 ("Twopart::moveSelectedCardToPlayed (unsigned int, unsinged int) - Player "
              << player << "; Card at " << pos << " = " << card);
      played.append (card);

      if (staple.numberOfCards ()) {
         CardWidget& card (staple.removeShownTopCard ());
         players[player].hand.insertSorted (card);

         if (!staple.numberOfCards ()) {
            Check3 (!pTrump);
            pTrump = new CardWidget (card); Check3 (pTrump);
            staple.hide ();
         }
      }
   }
   else {
      CardWidget* card (&players[player].hand.at (pos));
      unsigned int nr (card->number ());
      CardWidget::COLORS  color (card->color ());
      
      // Perform validity-check in part 2: Card must have the same color and be
      // bigger than the last played card or be a (bigger) trump
      Check3 (pTrump);
      if (played.numberOfCards ()
          && ((color == pTrump->color ())
              ? ((played.getTopCard ().color () == pTrump->color ())
                 && (played.getTopCard ().number () >= nr))
              : ((played.getTopCard ().color () != color)
                 || (played.getTopCard ().number () >= nr)))) {
         XMessageBox::Show (_("Played card(s) must have the same color and must be "
                              "bigger (or be a trump)!"), PACKAGE " - Twopart",
                            XMessageBox::ERROR);
         return false;
      }

      unsigned int posIns (played.numberOfCards ());
      if (offPos < (NUM_PLAYERS - 1))
         startPos[offPos++] = posIns;
#if TRACELEVEL > 8
      for (unsigned int i (0); i < (NUM_PLAYERS - 1); ++i)
         TRACE ("Twopart::moveSelectedCardToPlayed (unsigned int, unsinged int) - "
                << i << ". Position: " << startPos[i]);
#endif
      played.append (players[player].hand.remove (pos));
      TRACE3 ("Twopart::moveSelectedCardToPlayed (unsigned int, unsinged int) - Player "
              << player << "; Card at " << pos << " = " << *card);

      while (pos
             && ((card = &players[player].hand.at (--pos)),
                 (card->number ()) == (nr - 1))
             && (card->color () == color)) {
         TRACE9 ("Twopart::moveSelectedCardToPlayed (unsigned int, unsinged int) - Player "
                 << player << "; Card at " << pos << " = " << *card);

         nr = card->number ();
         played.insert (players[player].hand.remove (pos), posIns);
      }
   }
   return true;
}

/*--------------------------------------------------------------------------*/
//Purpose   : Callback after clicking on a card in hand
//Parameters: player: ID of player
//            iCard: Offset of card in hand
/*--------------------------------------------------------------------------*/
void Twopart::cardSelected (unsigned int player, unsigned int pos) {
   TRACE5 ("Twopart::cardSelected (unsigned int, unsigned int) - Player: "
           << player << " at position " << pos);
   Check3 (player <= NUM_PLAYERS);
   Check3 (pos <= players[player].hand.numberOfCards ());
   Check3 (statGame >= PLAYING);

   executeMove (player, pos);
   makeNextMoves ();
   disableLastPlayer ();
}

/*--------------------------------------------------------------------------*/
//Purpose   : Executes a move out of a hand
//Parameters: player: ID of player
//            iCard: Offset of card in hand
/*--------------------------------------------------------------------------*/
void Twopart::executeMove (unsigned int player, unsigned int pos) {
   TRACE5 ("Twopart::executeMove (unsigned int, unsigned int) - Player: "
           << player << " at position " << pos);
   Check3 (player <= NUM_PLAYERS);
   Check3 (pos <= players[player].hand.numberOfCards ());

   if (!moveSelectedCardToPlayed (player, pos))
      return;

   // Check if every player still in game or has already played; end round if so
   // or calculate next player if not
   TRACE7 ("Twopart::executeMove (unsigned int, unsigned int) - Players: "
           << hex << bfPlayers << dec);
   removePlayer (actPlayer = player);
   int newPlayer (actPlayer);
   if (bfPlayers)
      newPlayer = findNextPlayer (player);
   else {
      // Show trump if not already visible
      if (pTrump && !pTrump->is_visible ()) {
         pTrump->showFace ();
         pTrump->show ();
         tblTable.attach (*pTrump, 2, 3, 2, 3, 0, 0, 5, 5);
      }

      newPlayer = endRound ();
   }

   // Check if the actual part is terminated
   if ((statGame == PLAYING)
       ? (newPlayer < 0)
       : (newPlayer == findNextPlayerWithCards (newPlayer))) {
      actPlayer = (statGame == PLAYING) ? ~newPlayer : newPlayer;

      std::string str ((statGame == PLAYING)
                       ? _("First part ended; Part 2 starts player %1")
                       : _("Player %1 lost"));
      str.replace (str.find ("%1"), 2, (char)(actPlayer + '0'));
      status.pop (1);
      status.push (1, str);

      if (statGame == PLAYING)
         startPartTwo (actPlayer);
      else
         statGame = STOPPED;
   }
   else {
      actPlayer = newPlayer;

      status.pop (1);
      std::string stat ( _("Turn of player %1"));
      stat.replace (stat.find ("%1"), 2, (char)(actPlayer + '0'));
      status.push (1, stat);
   }
}

/*--------------------------------------------------------------------------*/
//Purpose   : Makes the move for the next player.
//Returns   : int: Flag for timer, if it should continue (0: no; else: yes)
/*--------------------------------------------------------------------------*/
int Twopart::makeNextMove () {
   TRACE5 ("Twopart::makeNextMove () - Turn of player " << actPlayer);
   Check3 (actPlayer); Check3 (actPlayer < NUM_PLAYERS);
   if (statGame == TOSTOP) {
      TRACE8 ("Twopart::makeNextMove () - End game ");
      statGame = STOPPED;
      if (restart)
         startGame ();
      return 0;
   }

   Check3 (statGame >= PLAYING);
   if (pos2Play == -1) {
      pos2Play = findPos2Play (actPlayer);
      if (pos2Play != -1) {
         unsigned int pos (pos2Play);
         // Flip card(s) to play
         if (statGame == PLAYING)
            players[actPlayer].hand.at (pos).showFace ();
         else
            do {
               players[actPlayer].hand.at (pos).showFace ();
               if (pos != (players[actPlayer].hand.numberOfCards () - 1))
                   players[actPlayer].hand.resize (pos, ICardPile::COMPRESSED);
            } while (pos
                     && ((players[actPlayer].hand.at (pos2Play).number ()
                          - players[actPlayer].hand.at (--pos).number ()
                          == (pos2Play - pos)))
                     && (players[actPlayer].hand.at (pos).color ()
                         == players[actPlayer].hand.at (pos2Play).color ()));
         return 1;
      }
      else {
         unsigned int oldPlayer (actPlayer);
         pickUpPlayedPile (actPlayer);

         status.pop (1);
         std::string stat ( _("Player %1 can't continue -> Picking up last cards;"
                              " Turn of player %2"));
         stat.replace (stat.find ("%1"), 2, (char)(oldPlayer + '0'));
         stat.replace (stat.find ("%2"), 2, (char)(actPlayer + '0'));
         status.push (1, stat);
      }
   }
   else
      executeMove (actPlayer, pos2Play);

   if (statGame >= PLAYING) {
      // If turn of human player: Stop computer playing
      if (!actPlayer) {
         TRACE5 ("Twopart::makeNextMove () - Enable human");
         enablePlayer (0);
      }

      pos2Play = -1;
      return actPlayer;
   }
   else
      return 0;
}

/*--------------------------------------------------------------------------*/
//Purpose   : Searches for the card(s) to play by analyzing the previously
//            played cards
//            to him if so. Else enable the players which can continue
//Parameters: player: ID of player to analyze
//Returns   : int: Position to play; or -1 if player can't continue
/*--------------------------------------------------------------------------*/
int Twopart::findPos2Play (unsigned int player) const {
   Check3 (actPlayer); Check3 (actPlayer < NUM_PLAYERS);
   Check3 (statGame >= PLAYING);
   TRACE5 ("Twopart::findPos2Play (unsigned int) - Player " << player);

   if (statGame == PLAYING) {
      unsigned int points (0);
      unsigned int cHigh (0);

      // Analyze played staple
      for (unsigned int i (0); i < played.numberOfCards (); ++i) {
         CardWidget::NUMBERS nr (played.at (i).number ());
         points += nr;
         if (nr >= CardWidget::TEN)
            ++cHigh;
      }

      TRACE2 ("Twopart::findPos2Play (unsigned int) - Points: " << points
              << "; Avg: " << (played.numberOfCards ()
                               ? (points / played.numberOfCards ()) : 0)
              << "; High: " << cHigh);
      if (played.numberOfCards ())
         points /= played.numberOfCards ();

      int maxNr (-1);
      int maxEqualNr (-1);
      int posMax (-1);
      int posMaxEqual (-1);
      analyzeLastPlayed (*startPos, played.numberOfCards () - *startPos,
                         maxNr, posMax, maxEqualNr, posMaxEqual);

      int pos (-1);
      // Try to get the cards if there are loads of high cards (a third or more)
      // or if the average card played is at least a 8
      if (((played.numberOfCards () / 3) < cHigh)
          || (points >= CardWidget::EIGHT)) {
         // Search for card whose number you own
         for (unsigned int i (*startPos);
              i < players[player].hand.numberOfCards (); ++i)
            if ((played.exists (players[player].hand.at (i).number ()))
                && ((posMaxEqual == -1) 
                    || (played.at (i).number () >= maxEqualNr))) {
               TRACE2 ("Twopart::findPos2Play (unsigned int) - Having equal card at "
                       << i);
               return i;
            }
         
         // Player has no equal card: Play high card if higher (and he is the
         // last player) or 
         if (((!(bfPlayers & ~(1 << player)))
              && (posMaxEqual == -1)
              && ((pos = players[player].hand.findFirstEqualOrBigger
                   (static_cast<CardWidget::NUMBERS> (maxNr))) != -1))
             // or the staple is being fighted for and player has high cards
             || (*startPos
                 && ((pos = players[player].hand.numberOfCards () - 1),
                     ((players[player].hand.at (pos).number ()
                       == CardWidget::ACE))
                     || ((playersInBitfield (bfOldPlayers) < *startPos)
                         && (players[player].hand.at (pos).number () >= maxNr)
                         && (posMaxEqual == -1))))) {
            TRACE2 ("Twopart::findPos2Play (unsigned int) - Playing highest card at "
                    << pos);
            return pos;
         }
      }

      // We don't want the pile; so try not to get it:
      TRACE5 ("Twopart::findPos2Play (unsigned int) - Avoiding pile");
      pos = ((players[player].hand.numberOfCards () > 1)
             && ((played.exists (players[player].hand.at (0).number (), *startPos)
                  && (players[player].hand.at (1).number () < CardWidget::SIX)
                  && !played.exists (players[player].hand.at (1).number (), *startPos))
                 || ((players[player].hand.at (0).number () == maxEqualNr)
                     && !played.exists (players[player].hand.at (1).number (),
                                        *startPos)
                     && (players[player].hand.at (1).number () < CardWidget::SIX))));

      // Final check: If you have to pick up the pile and you're the last,
      // use at least a high card
      if (!pos
          && !(bfPlayers & ~(1 << player))
          && (players[player].hand.numberOfCards () > 1)
          && (players[player].hand.at (pos).number () > maxNr)
          && (posMaxEqual == -1))
         pos = 1;
      return pos;
   }
   else {
      // Find first fitting card
      int pos (played.numberOfCards ()
               ? players[player].hand.find1EqualOrBigger (played.getTopCard (),
                                                          compByColorAccTrumps)
               : findSmallestCard (player));
      TRACE5 ("Twopart::findPos2Play (unsigned int) - First try: " << pos);

      Check3 (pTrump);
      if ((pos == -1)
          || (played.numberOfCards ()
              && (played.getTopCard ().color ()
                  != players[player].hand.at (pos).color ()))) {
         TRACE5 ("Twopart::findPos2Play (unsigned int) - No card found; trying trump");
         Check3 (pTrump);
         if (played.getTopCard ().color () != pTrump->color ()) {
            for (pos = players[player].hand.numberOfCards (); pos; --pos)
               if (players[player].hand.at (pos - 1).color () != pTrump->color ())
                  break;

            if (!pos)
               pos = findEndOfSerie (player, 0);
            else
               return (pos == players[player].hand.numberOfCards ()) ? -1 : pos;
         }
         else
            return -1;
      }
      else
         // Card was found; now search for last card to play (only if not trump
         // or only trump left)
         if (!pos
             || (players[player].hand.at (pos).color () != pTrump->color ()))
            pos = findEndOfSerie (player, pos);
      TRACE5 ("Twopart::findPos2Play (unsigned int) - Playing card at pos " << pos);
      return pos;
   }
}

/*--------------------------------------------------------------------------*/
//Purpose   : Searches for the smallest card in the hand of the passed player
//Parameters: player: Player to inspect
//Returns   : int: Position of smallest card
/*--------------------------------------------------------------------------*/
unsigned int Twopart::findSmallestCard (unsigned int player) const {
   TRACE5 ("Twopart::findSmallestCard (unsigned int) - Inspecting player " << player);
   Check3 (player < NUM_PLAYERS);
   Check3 (pTrump);

   unsigned int nrMin (CardWidget::UNREACHABLE);
   unsigned int cSerie (0);
   unsigned int pos (0);
   for (unsigned int i (0); i < players[player].hand.numberOfCards (); ++i) {
      CardWidget& card (players[player].hand.at (i));

      // Stop searching if a trump was found
      if ((card.color () == pTrump->color ()) && i)
         break;
          
      if (nrMin >= card.number ()) {
         TRACE9 ("Twopart::findSmallestCard (unsigned int) - New smallest card at "
                 << i << "; Cards: " << (findEndOfSerie (player, i) - i));

         unsigned int endPos (findEndOfSerie (player, i));
         if ((card.number () == nrMin) && ((endPos - i) <= cSerie))
               continue;

         cSerie = (endPos - i);
         nrMin = card.number ();
         pos = endPos;
      }
   }

   TRACE5 ("Twopart::findSmallestCard (unsigned int) - Smallest card at " << pos);
   return pos;
}

/*--------------------------------------------------------------------------*/
//Purpose   : Searches for the last position of the card which are in a serie
//            (same color; number increasing by 1)
//Parameters: player: Player to inspect
//            start: Position to start
//Returns   : unsinged int: Position of last card in serie
/*--------------------------------------------------------------------------*/
unsigned int Twopart::findEndOfSerie (unsigned int player, unsigned int start) const {
   Check3 (start < players[player].hand.numberOfCards ());

   CardWidget* card (&players[player].hand.at (start));
   CardWidget::NUMBERS nr (card->number ());
   CardWidget::COLORS color (card->color ());

   while ((++start < players[player].hand.numberOfCards ())
          && ((card = &players[player].hand.at (start)),
              (card->number ()) == (nr + 1))
          && (card->color () == color)) {
      TRACE9 ("Twopart::findEndOfSerie (unsigned int, unsigned int) - Next valid card "
              << *card << " at " << start);
      nr = card->number ();
   }

   return start - 1;
}

/*--------------------------------------------------------------------------*/
//Purpose   : Checks if there is a winner for the round and moves played cards
//            to him if so. Else enable the players which can continue
//Returns   : int: Next player; or -1 if there is no next player
/*--------------------------------------------------------------------------*/
int Twopart:: endRound () {
   TRACE8 ("Twopart::endRound ()");
   Check3 (!bfPlayers);

   unsigned int nextPlayer (NUM_PLAYERS);

   if (statGame == PLAYING2) {
      played.clear ();
      bfPlayers = (1 << NUM_PLAYERS) - 1;
      removePlayersWithoutCards ();
      offPos = 0;
      nextPlayer = (!players[actPlayer].hand.numberOfCards ())
         ? findNextPlayer (actPlayer) : actPlayer;
   }
   else {
      bfPlayers = bfOldPlayers;
      unsigned int cPlayers (playersInBitfield (bfPlayers));

      TRACE8 ("Twopart::endRound () - Round has " << cPlayers << " players; Start = "
              << *startPos << " of " << played.numberOfCards () << " cards");
      Check3 ((*startPos + cPlayers) <= played.numberOfCards ());

      int maxNr (-1);
      int maxEqualNr (-1);
      int posMax (-1);
      int posMaxEqual (-1);
      analyzeLastPlayed (*startPos, cPlayers, maxNr, posMax, maxEqualNr, posMaxEqual);

      TRACE4 ("Twopart::endRound () - Player starting round: " << startPlayer
              << "; players: " << cPlayers);
      Check3 ((*startPos + cPlayers) == played.numberOfCards ());

      // Equal cards found
      if (posMaxEqual >= 0) {
         unsigned int bfPlayersOut (0);
         cPlayers = 0;

         // Add players having equal cards and having still cards left
         nextPlayer = startPlayer;
         for (unsigned int i (*startPos); i < played.numberOfCards (); ++i) {
            if ((played.at (i).number () == maxEqualNr)
                && players[pos2Player (i - *startPos)].hand.numberOfCards ()) {
               if (!cPlayers)
                  // Start player is the first who played the highest cards
                  nextPlayer = pos2Player (posMaxEqual - *startPos);
               
               TRACE5 ("Twopart::endRound () - Found equal cards; Player "
                       << pos2Player (i - *startPos)
                       << (cPlayers ? " still in round" : " is winner"));
               ++cPlayers;
            }
            else
               bfPlayersOut |= (1 << pos2Player (i - *startPos));
         } // endfor check for equal cards
         bfPlayers &= ~bfPlayersOut;
         TRACE5 ("Twopart::endRound () - Found equal cards; " << cPlayers
                 << " player(s) still in round (" << hex << bfPlayers << dec << ')');

         // Find player to continue
         if (!players[nextPlayer].hand.numberOfCards ())
            nextPlayer = findNextPlayer (nextPlayer);
         TRACE6 ("Twopart::endRound () - Try to continue with player " << nextPlayer);
         if (cPlayers < 2) {                   // Less than two found: 
            bfPlayers = (1 << NUM_PLAYERS) - 1;
            cPlayers = removePlayersWithoutCards ();

            if (nextPlayer == -1) {
               nextPlayer = bfPlayers ? findNextPlayer (nextPlayer) : ~startPlayer;
               movePlayedCardsToPlayer (startPlayer);
            }
            else
               movePlayedCardsToPlayer (nextPlayer);
         }
#if CHECK > 0
         else
            Check (cPlayers > 1);
#endif
      }
      // All played cards are differnt: Winner is the one with highest card
      else {
         startPlayer = nextPlayer = pos2Player (posMax - *startPos);
         movePlayedCardsToPlayer (nextPlayer);
         TRACE5 ("Twopart::endRound () - Found winner: " << nextPlayer);

         bfPlayers = (1 << NUM_PLAYERS) - 1;   // Set all players (having cards)
         removePlayersWithoutCards ();
         bfOldPlayers = bfPlayers;

         if (!players[nextPlayer].hand.numberOfCards ())
            nextPlayer = findNextPlayer (nextPlayer);
         if (nextPlayer == -1)
            nextPlayer = ~startPlayer;
      }
      *startPos = played.numberOfCards ();
   }

   bfOldPlayers = bfPlayers;
   TRACE8 ("Twopart::endRound () - Continuing with player " << nextPlayer);
   return startPlayer = nextPlayer;
}

/*--------------------------------------------------------------------------*/
//Purpose   : Analyzes the played staple and retrieves the highest card(s)
//Parameters: startPos: Position from where to start analyzing
//            max: Highest single card
//            maxPos: Position of highest single card
//            maxEqua: Highest pair
//            maxEquaPos: Position of highest equal card
/*--------------------------------------------------------------------------*/
void Twopart::analyzeLastPlayed (unsigned int startPos, unsigned int cards,
                                 int& max, int& maxPos, int& maxEqual,
                                 int& maxEqualPos) const {
   cards += startPos;
   // Check if card is bigger then all previous
   for (; startPos < cards; ++startPos) {
      if ((int)(played.at (startPos).number ()) > max) {
         TRACE3 ("TwoPart::analyzePlayed () - New highest card " << played.at (startPos)
                 << " at position " << startPos);
         max = (int)played.at (startPos).number ();
         maxPos = startPos;
         }
         Check3 (maxPos < played.numberOfCards ());
         Check3 (max == played.at (maxPos).number ());

         // Check if card has equal cards
         for (unsigned int j (startPos + 1); j < cards; ++j)
            if (played.at (startPos).number () == played.at (j).number ())
               if ((int)(played.at (startPos).number ()) > maxEqual) {
                  TRACE3 ("TwoPart::analyzePlayed () - Found equal "
                          << played.at (startPos).numberStr ()
                          << " at positions " << startPos << " and " << j);
                  maxEqual = (int)played.at (startPos).number ();
                  maxEqualPos = startPos;
                  break;
               } // endif equal card found
   } // end-for all players still in game
}

/*--------------------------------------------------------------------------*/
//Purpose   : Finds the next player having cards
//Parameters: player: Number of player to start with
//Returns   : int: Number of next player (or -1)
/*--------------------------------------------------------------------------*/
int Twopart::findNextPlayerWithCards (unsigned int player) {
   unsigned int i (player);
   do {
      i = (i + 1) & 0x3;
      if (players[i].hand.numberOfCards ())
         return i;
   } while (i != player);

   return -1;
}

/*--------------------------------------------------------------------------*/
//Purpose   : Finds the next player which can continue according to the
//            bfPlayers bitfield
//Parameters: player: Number of player to start with
//Returns   : int: Number of next player (or -1)
/*--------------------------------------------------------------------------*/
int Twopart::findNextPlayer (unsigned int player) {
   if (!bfPlayers)                                // No players left: Return -1
      return -1;

   // Find first player (starting with the passed one) being still in game
   do {
      player = (player + 1) & 0x3;
   } while (!(bfPlayers & (1 << player)));

   return player;
}

/*--------------------------------------------------------------------------*/
//Purpose   : Removes all players having no cards left from the player-bitfield
//Returns   : Number of players left
/*--------------------------------------------------------------------------*/
unsigned int Twopart::removePlayersWithoutCards () {
   unsigned int cPlayers (0);
   for (unsigned int i (0); i < NUM_PLAYERS; ++i)
      if (players[i].hand.numberOfCards ())
         ++cPlayers;
      else
         removePlayer (i);

   return cPlayers;
}

/*--------------------------------------------------------------------------*/
//Purpose   : Converts a position in the played staple into the number of the
//            player
//Parameters: pos: Position to convert
//            start: First player of round
//Returns   : unsigned int: Number of player
/*--------------------------------------------------------------------------*/
unsigned int Twopart::pos2Player (unsigned int pos) const {
   TRACE9 ("Twopart::pos2Player (unsigned int) - Pos to convert: " << pos
           << "; starting with player " << startPlayer);

   unsigned int start (startPlayer);
   while (pos) {
      ++start &= 0x3;
      if (bfPlayers & (1 << start))
         --pos;
   }
   
   TRACE9 ("Twopart::pos2Player (unsigned int) - Calculated player: " << start);
   return start;
}

/*--------------------------------------------------------------------------*/
//Purpose   : Method to move the cards of the actual round to the winner
//Parameters: receiver: Nr. of player getting all played cards
//            start: Startposition of cards to move
/*--------------------------------------------------------------------------*/
void Twopart::movePlayedCardsToPlayer (unsigned int receiver, unsigned int start) {
   TRACE8 ("Twopart::movePlayedCardsToPlayer () - " << played.numberOfCards ()
           << " cards for player " << receiver << " til position " << start);
   Check3 (receiver < NUM_PLAYERS);
   Check3 (start < played.numberOfCards ());

   while (played.numberOfCards () > start)
      if (statGame == PLAYING)
         players[receiver].won.append (played.remove (0));
      else
         players[receiver].hand.append (played.remove (start));

   if (statGame == PLAYING2)
      players[receiver].hand.sort (compByColorAccTrumps);
}

/*--------------------------------------------------------------------------*/
//Purpose   : Loads the cards (from xpm-files) and initializes deck
/*--------------------------------------------------------------------------*/
void Twopart::loadCards () {
   // Cards need an realized (!) parent, so make somehow sure, that the window
   // already exists
   Check3 (staple.is_realized ());

   gdk_threads_enter ();
   status.push (1, _("Loading cardimages ..."));
   gdk_threads_leave ();

   INIFILE (NAME_INIFILE.c_str ());
   INISECTION (Decks);
   INIATTR2 (Decks, std::string, pathDeck, Front);
   INIATTR2 (Decks, std::string, pathBack, Back);

   pathDeck = CARDSET_PATH "/Deck1";
   pathBack = CARDSET_PATH "/back1.xpm";

   try {
      unsigned int rc (INIFILE_READ ());
   }
   catch (std::string& error) {
      TRACE ("'Twopart::loadCards () - Can't read INI-file '"
             << NAME_INIFILE << "'\nReason: " << error);
   }

   cardFaces.load (staple.get_window (), pathDeck, pathBack);
   cards.addPacket (cardFaces);

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

   statGame = STOPPED;
   pThread = NULL;
}

/*--------------------------------------------------------------------------*/
//Purpose   : Loads the cards (from xpm-files)
//Parameters: opt: Actually a bit field! Option indicationg what to load
/*--------------------------------------------------------------------------*/
void Twopart::changeCards (void* opt) {
   TRACE2 ("Twopart::changeCards (void*) - Option: " << opt);

   // Cards need an realized (!) parent, so ensure that staple is already shown
   Check3 (staple.is_realized ());
   unsigned int option ((unsigned int)opt); Check3 (option);

   if (option & 1)
      cardFaces.loadDecks (staple.get_window (), pathDeck);
   if (option & 2)
      cardFaces.loadBack (staple.get_window (), pathBack);

   gdk_threads_enter ();
   cards.update ();
   if (dlgChgDecks)
      dlgChgDecks->unlock ();
   gdk_threads_leave ();
   pThread = NULL;
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
      players[i].hand.setStyle (i ? ICardPile::COMPRESSED : ICardPile::NORMAL);
      players[i].won.clear ();
   }
   played.clear ();

   disableLastPlayer ();
   staple.show ();
   if (pTrump) {
      delete pTrump;
      pTrump = NULL;
   }

   pos2Play = -1;
}

/*--------------------------------------------------------------------------*/
//Purpose   : Deals the cards
/*--------------------------------------------------------------------------*/
void Twopart::dealCards () {
   TRACE9 ("Twopart::dealCards ()");

   // Show cards on table: For all players put 3 cards in hand
   for (unsigned int i (0); i < NUM_PLAYERS; ++i)
      for (unsigned int j (0); j < 3; ++j)
         players[i].hand.insertSorted (staple.removeTopCard ());

   for (unsigned int i (0); i < (NUM_PLAYERS - 1); ++i)
      startPos[i] = 0;
   offPos = 0;

   bfPlayers = bfOldPlayers = (1 << NUM_PLAYERS) - 1;
   enablePlayer (actPlayer = startPlayer = 0);
}

/*--------------------------------------------------------------------------*/
//Purpose   : Starts part two of the game
//Returns   : int: Value indicating if timer should continue
/*--------------------------------------------------------------------------*/
int Twopart::startPartTwoTimerFnc () {
   TRACE8 ("Twopart::startPartTwoTimerFnc ()");
   Check3 (!bfPlayers);
   statGame = PLAYING2;

   // Prepare array for sorting according to trumps
   Check3 (pTrump);
   for (unsigned int i (0); i < 4; ++i)
      sortOrder[i] = (i - pTrump->color () + 3) & 0x3;
   Check3 (sortOrder[pTrump->color ()] == 3);

   startPlayer = (unsigned int)-1;

   unsigned int nrPlayers (0);
   // Check if there are players without cards
   for (unsigned int i (0); i < NUM_PLAYERS; ++i)
      if (!players[i].won.numberOfCards ()) {
         TRACE5 ("Twopart::startPartTwoTimerFnc () - Player " << i << " has no cards");
         bfPlayers |= 1 << i;
         ++nrPlayers;
      }

   // Now move the cards from the played pile to the hand; if there are
   // players without cards give them the cards up to 5
   unsigned int victim (actPlayer);
   for (unsigned int i (0); i < NUM_PLAYERS; ++i) {
      for (unsigned int j (players[i].won.numberOfCards ()); j; --j) {
         CardWidget& card (players[i].won.removeTopCard ());
         TRACE9 ("Twopart::startPartTwoTimerFnc () - Moving cards " << card
                 << " for player " << i);
         if (bfPlayers && (card.number () <= CardWidget::FIVE)) {
            players[pos2Player (++victim)].hand.append (card);
            victim %= nrPlayers;
         }
         else
            players[i].hand.append (card);
      }
   }

   // Finally sort and show the cards
   for (unsigned int i (0); i < NUM_PLAYERS; ++i) {
      players[i].hand.sort (compByColorAccTrumps);
      players[i].hand.setStyle (i ? ICardPile::VERY_COMPRESSED : ICardPile::COMPRESSED);
   }
   
   bfPlayers = (1 << NUM_PLAYERS) - 1;
   if (!(startPlayer = actPlayer))
      enablePlayer (0);

   pos2Play = -1;
   return 0;
}

/*--------------------------------------------------------------------------*/
//Purpose   : Compares the cards in the pile with regard of the color and
//            with special consideration of trumps
//Parameters: a: Card to compare
//            b: Card to compare
//Returns   : bool: True, if a < b
/*--------------------------------------------------------------------------*/
bool Twopart::compByColorAccTrumps (const CardWidget* a, const CardWidget* b) {
   Check3 (a); Check3 (b);

   TRACE9 ("Twopart::compByColorAccTrumps (const CardWidget*, const CardWidget*) - "
           << *a << " < " << *b << " = "
           << ((a->color () == b->color ()) ? a->number () < b->number ()
               : (sortOrder[a->color ()] <sortOrder[b->color ()])));
   return ((a->color () == b->color ())
           ? a->number () < b->number ()
           : (sortOrder[a->color ()] < sortOrder[b->color ()]));
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
