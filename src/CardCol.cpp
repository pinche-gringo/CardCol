
//$Id$

//PROJECT     : Cardgames
//SUBSYSTEM   : Common
//REFERENCES  :
//TODO        : 
//BUGS        :
//REVISION    : $Revision$
//AUTHOR      : Markus Schwab
//CREATED     : 9.9.2002
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

#include <fstream>

#include <glib.h>

#include <Check.h>
#include <Trace_.h>

#include <INIFile.h>
#include <PathSrch.h>

#include <XAbout.h>
#include <XMessageBox.h>
#include <Cardset-config.h>
#include <DeckSelect.h>

#include <CardWidget.h>
#include <Rovhult.h>
#include <Twopart.h>

#include "CardCol.h"

const std::string CardgameCollection::NAME_INIFILE = PathSearch::expandNode ("~/.cardgames");

// Pixmap for program
const char* CardgameCollection::xpmGame[] = {
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
const char* CardgameCollection::xpmAuthor[] = {
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
XApplication::MenuEntry CardgameCollection::menuItems[] = {
    { (initI18n (PACKAGE, LOCALEDIR),
      _("_Game")),            _("<alt>G"), 0,        BRANCH },
    { _("_New"),              _("<ctl>N"), NEW,      ITEM },
    { _("_End"),              _("<ctl>E"), END,      ITEM },
    { "",                     "",          0,        SEPARATOR },
    { _("E_xit"),             _("<ctl>Q"), EXIT,     ITEM },
    { _("_Options"),          _("<alt>O"), 0,        BRANCH },
    { _("_Change game"),      _("<alt>C"), 0,        SUBMENU },
    {    _("_Rovhult"),       _("<ctl>R"), ROVHULT,  RADIOITEM },
    {    _("_Twopart"),       _("<ctl>T"), TWOPART,  LASTRADIOITEM },
    { "",                     "",          0,        SUBMENUEND },
    { _("_Change decks ..."), _("<ctl>C"), CHGDECKS, ITEM },
    { _("_Save settings"),    _("<ctl>S"), SAVESET,  ITEM },
#if TRACELEVEL >= 0
    { _("_Debug"),            _("<ctl>D"), DEBUG,    CHECKITEM },
#endif
    { _("_Help"),             _("<alt>H"), 0,        LASTBRANCH },
    { _("_About..."),         _("<ctl>A"), ABOUT,    ITEM } };


/*--------------------------------------------------------------------------*/
//Purpose   : Defaultconstructor; all widget are created
/*--------------------------------------------------------------------------*/
CardgameCollection::CardgameCollection ()
   : XApplication (PACKAGE " - Cardgames V" PRG_RELEASE), status ()
     , cardFaces (USED_CARDS), cards (), pThread (NULL), game (NULL)
     , typeGame (GROVHULT), oldGame (NONE), restart (false) {
   set_usize (WIDTH, HEIGHT);

   addMenu (menuItems[0]);
   pMenuNew = dynamic_cast<MenuItem*> (addMenu (menuItems[1])); Check3 (pMenuNew);
   pMenuNew->set_sensitive (false);

   status.show ();
   getClient ().pack_end (status, false);

   show ();

   // Load cards in background
   pThread = THRDAPPL::create (*this, (THRDAPPL::THREAD_OBJMEMBER)&CardgameCollection::loadCards,
                               NULL);
   TRACE9 ("CardgameCollection::CardgameCollection () - Thread-ID = " << pThread->getID ());

   // Create controls
   pMenuEnd = dynamic_cast<MenuItem*> (addMenu (menuItems[2])); Check3 (pMenuEnd);
   pMenuEnd->set_sensitive (false);
   addMenus (menuItems + 3, sizeof (menuItems) / sizeof (menuItems[0]) - 3);
}

/*--------------------------------------------------------------------------*/
//Purpose   : Destructor
/*--------------------------------------------------------------------------*/
CardgameCollection::~CardgameCollection () {
   TRACE9 ("CardgameCollection::~CardgameCollection ()");
}


/*--------------------------------------------------------------------------*/
//Purpose   : Starts the game
/*--------------------------------------------------------------------------*/
void CardgameCollection::startGame () {
   TRACE6 ("CardgameCollection::startGame () - Old game type " << oldGame
           << " -> New: " << typeGame);

   // Check if game has been changed; if so destroy the old one
   if (oldGame != typeGame) {
      if (game) {
         getClient ().remove (*game);
         delete game;
      }

      oldGame = typeGame;
      switch (typeGame) {
      case GROVHULT:
         game = new TRovhult<CardgameCollection> (*this, &CardgameCollection::gameEvents);
         break;

      case GTWOPART:
         game = new TTwopart<CardgameCollection> (*this, &CardgameCollection::gameEvents);
         break;

      default:
         Check (0);
      }
   }

   Check3 (game);
   game->start ();
}

/*--------------------------------------------------------------------------*/
//Purpose   : Command-handler
//Parameters: menu: ID of command (menu)
/*--------------------------------------------------------------------------*/
void CardgameCollection::command (int menu) {
   switch (menu) {
   case NEW:
      if (game && game->isRunning ()) {
         restart = true;
         XMessageDialog<CardgameCollection>
            ::Show (*this, &CardgameCollection::userWants2End,
                    _("A game is already running. Do you really"
                      " want to end it and start another?"),
                    PACKAGE, XMessageBox::QUESTION | XMessageBox::YESNO);
      }
      else
         startGame ();
      break;

   case END:
      Check3 (game && game->isRunning ());
      restart = false;
      XMessageDialog<CardgameCollection>
         ::Show (*this, &CardgameCollection::userWants2End,
                 _("Do you really want to end the game?"),
                 PACKAGE, XMessageBox::QUESTION | XMessageBox::YESNO);
      break;

   case TWOPART:
      typeGame = GTWOPART;
      break;

   case ROVHULT:
      typeGame = GROVHULT;
      break;

   case CHGDECKS:
      dlgChgDecks = CarddeckSelectDlg<CardgameCollection>
         ::create (*this, &CardgameCollection::changeDecks,
                   CARDSET_PATH, pathDeck, pathBack);
      break;

   case SAVESET: {
      TRACE2 ("CardgameCollection::command () - Save file " << NAME_INIFILE);
      ofstream inifile (NAME_INIFILE.c_str ());
      
      inifile << "[Game]\nDefault=" << typeGame << "\n\n[Decks]\nFront="
              << pathDeck << "\nBack=" << pathBack << '\n';
      break;
   }

   case ABOUT: {
      string ver (_("Anticopyright (A) 2002 Markus Schwab"
                    "\ne-mail: g17m0@lycos.com\n\nCompiled on %1 at %2"));
      ver.replace (ver.find ("%1"), 2, __DATE__);
      ver.replace (ver.find ("%2"), 2, __TIME__);

      XAbout* about (new XAbout (ver, PACKAGE " V" VERSION));
      about->setIconProgram (xpmGame);
      about->setIconAuthor (xpmAuthor); }
      break;

   case EXIT:
      if (game && game->isRunning ()
          && (XMessageBox::Show (_("A game is running. Do you really want to quit?"),
                                 PACKAGE, XMessageBox::QUESTION | XMessageBox::YESNO)
              != XMessageBox::YES))
            break;

      delete_event_impl (0);
      break;

#if TRACELEVEL >= 0
   case DEBUG: {
      static bool open = false;
      open = !open;
      if (game)
         game->playOpen (open);
      break; }
#endif

   default:
      Check3 (0);
   } // end-switch
}
/*--------------------------------------------------------------------------*/
//Purpose   : Callback to change the carddecks
//Parameters: cmd: Selected button of dialog
/*--------------------------------------------------------------------------*/
void CardgameCollection::changeDecks (ICarddeckSelectDlg::commands cmd) {
   TRACE2 ("CardgameCollection::changeDecks (ICarddeckSelectDlg::commands) - Command "
           << cmd);
   Check3 (dlgChgDecks);

   if (cmd != ICarddeckSelectDlg::CANCEL) {
      std::string deck, back;
      dlgChgDecks->getSelection (deck, back);
      TRACE3 ("CardgameCollection::changeDecks (ICarddeckSelectDlg::commands) - Use "
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
                                     (THRDAPPL::THREAD_OBJMEMBER)&CardgameCollection::changeCards,
                                     (void*)opt);
         TRACE9 ("CardgameCollection::Twopart () - Thread-ID = " << pThread->getID ());
      }
   }

   if (cmd != ICarddeckSelectDlg::APPLY)
      dlgChgDecks = NULL;
}

/*--------------------------------------------------------------------------*/
//Purpose   : Loads the cards (from xpm-files)
//Parameters: opt: Actually a bit field! Option indicationg what to load
/*--------------------------------------------------------------------------*/
void CardgameCollection::changeCards (void* opt) {
   TRACE2 ("CardgameCollection::changeCards (void*) - Option: " << opt);

   // Cards need an realized (!) parent, so ensure that the window is already
   // shown
   Check3 (this->is_realized ());
   unsigned int option ((unsigned int)opt); Check3 (option);

   if (option & 1)
      cardFaces.loadDecks (get_window (), pathDeck);
   if (option & 2)
      cardFaces.loadBack (get_window (), pathBack);

   gdk_threads_enter ();
   cards.update ();
   if (dlgChgDecks)
      dlgChgDecks->unlock ();
   gdk_threads_leave ();
   pThread = NULL;
}

/*--------------------------------------------------------------------------*/
//Purpose   : Checks the user-input after asking if he wants to end the game;
//             depending on the answer either stops or continues
//Parameters: input: Button pressed by the user
/*--------------------------------------------------------------------------*/
void CardgameCollection::userWants2End (unsigned int input) {
   if (input == XMessageBox::YES) {
      Check3 (game);
      if (game->isRunning ()) {
         status.pop (1);
         status.push (1, _("User canceled"));

         if (game->canBeStopped ()) {
            game->stop ();
            if (restart)
               startGame ();
         }
         else {
            game->end ((typeGame == oldGame) ? restart : false);
            restart = false;
         }
      }
      else
         startGame ();
   }
}

/*--------------------------------------------------------------------------*/
//Purpose   : Loads the cards (from xpm-files)
/*--------------------------------------------------------------------------*/
void CardgameCollection::loadCards () {
   // Cards need an realized (!) parent, so make somehow sure, that the window
   // already exists
   Check3 (is_realized ());

   gdk_threads_enter ();
   status.push (1, _("Loading cardimages ..."));
   gdk_threads_leave ();

   INIFILE (NAME_INIFILE.c_str ());
   INISECTION (Decks);
   INIATTR2 (Decks, std::string, pathDeck, Front);
   INIATTR2 (Decks, std::string, pathBack, Back);
   INISECTION (Game);
   INIATTR2 (Game, unsigned int, (unsigned int)typeGame, Default);

   pathDeck = CARDSET_PATH "/Deck1";
   pathBack = CARDSET_PATH "/back1.xpm";

   try {
      unsigned int rc (INIFILE_READ ());
   }
   catch (std::string& error) {
      TRACE ("'CardgameCollection::loadCards () - Can't read INI-file '"
             << NAME_INIFILE << "'\nReason: " << error);
   }

   cardFaces.load (get_window (), pathDeck, pathBack);
   cards.addPacket (cardFaces);

   gdk_threads_enter ();
   pMenuNew->set_sensitive (true);
   status.pop (1);
   status.push (1, _("Start a new game with Ctrl+N (or Game -> New)"));
   gdk_threads_leave ();

   assert (cardFaces.numberOfCards ());
   pThread = NULL;
}

/*--------------------------------------------------------------------------*/
//Purpose   : Handling of game-events
//Parameters: status: New status of game
/*--------------------------------------------------------------------------*/
void CardgameCollection::gameEvents (unsigned int status) {
   TRACE1 ("CardgameCollection::gameEvents (unsigned int) const - New status: "
           << status);

   switch (status) {
   case Game::PLAYING:
      Check3 (pMenuEnd);
      pMenuEnd->set_sensitive (true);
      break;

   case Game::STOPPED:
      Check3 (pMenuEnd);
      pMenuEnd->set_sensitive (false);

      if (restart)
         startGame ();
      restart = false;
      break;
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
   CardgameCollection win;

   gdk_threads_enter ();
   appl.run ();
   gdk_threads_leave ();
   return 0;
}
