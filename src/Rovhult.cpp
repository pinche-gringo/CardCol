//$Id$

//PROJECT     : Cardgames
//SUBSYSTEM   : Rovhult
//REFERENCES  :
//TODO        :
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

#define DEBUG 9
#include <Check.h>
#include <Trace_.h>

#include <XAbout.h>

#include "Rovhult.h"


const unsigned int RovhultAppl::COLS_PLAYER[NUM_PLAYERS] = { 7, 13, 7, 1 };
const unsigned int RovhultAppl::ROWS_PLAYER[NUM_PLAYERS] = { 4, 7, 13, 7 };

// Pixmap for program
const char* RovhultAppl::xpmRovhult[] = {
   "56 46 38 1",
   " 	c None",
   ".	c #FFFFFF",
   "+	c #D7D3D7",
   "@	c #000000",
   "#	c #E7E7EF",
   "$	c #515549",
   "%	c #969696",
   "&	c #AEAAAE",
   "*	c #AEA6AE",
   "=	c #AEAEAE",
   "-	c #A69EA6",
   ";	c #B6AEB6",
   ">	c #616161",
   ",	c #A6A2A6",
   "'	c #AEAEB6",
   ")	c #717571",
   "!	c #BEB6BE",
   "~	c #CFC7CF",
   "{	c #303430",
   "]	c #000808",
   "^	c #696971",
   "/	c #EFF3F7",
   "(	c #202020",
   "_	c #182020",
   ":	c #080408",
   "<	c #8E8A8E",
   "[	c #BEC3BE",
   "}	c #9E9A9E",
   "|	c #D7DFE7",
   "1	c #79869E",
   "2	c #AE9E59",
   "3	c #282C30",
   "4	c #BEBABE",
   "5	c #AE6161",
   "6	c #51A249",
   "7	c #E7D79E",
   "8	c #BEC3CF",
   "9	c #8E8679",
   "......................................................+@",
   "..+++++++++++++++++++++++++++++++++++++++++++++++++++#$@",
   "..%&*=%**%**%**%**%**%**%**%**%**%**%**%**%**%**%**=-;>@",
   "..,**&&=&&=&'*=&=&'*=&=&'*=&=&'*=&=&'*=&=&'*=&=&'=*==')@",
   "..-==&&&&&&&&=&&&&&=&&&&&=&&&&&=&&&&&=&&&&&=&&&&&&=&=!)@",
   "..-&&&&&&&&&&&&&&&&&&----------&=!~!~~~!;&&&&&&&&&&&=')@",
   "..,&&&&&&&&&&&&&&&&~............#)@{@]@$=&&&&&&&&&&&&!^@",
   "..&&&&&&&&&&&&&&=!'/............#,(@@@@('!&&&&&&&&&&=!)@",
   "..-&&&;&;&;&&;&!~=$)^$>$$........~{({]@@)&&&&;&&;&&&&;)@",
   "..-&&&&&&&&&&'!=@@@{{{{{{_@@.....+@@@@@@$!'!&&&&&&&&==)@",
   "..,&&&!*=&&..+_@$$$$))_@@($$@{+...~(]@:@{$!=&&;&&&&&&!)@",
   "..&&&=*=&&=.<@@$$==+#[<$=~=&=@@=..!@@@@@@@~;&&&&&;&==;)@",
   "..-&&&=&&;!$_$$#/...'$$$&=...+}@).;]@:@:@@)!==&&&&&&=!^@",
   "..-&&&;&=~@{$>......|)$$}}.....+@1/$@@@@@@$=;;&&&&&==;)@",
   "..,&&&&&!)_${.......#=$_(@!;....~@(@@:@:@@@$};==&&&&=!)@",
   "..&&&&=;~_{$~.....=<)@({@@$$<<}..)@@@@:@]@@_>1!&&&&==;)@",
   "..-&&&&!)@{{...#='}$@@{$@@$$$$$$}&@@:@@:@@@@@(;!=&&&&;)@",
   "..-&&&=;{{$)..#,=;)@@@@@@@$$$$$$$$(:@@:@@:]@@]>;&&&===)@",
   "..,&&&=~{{$..}===!^:@:((}&!~{{${$${@@:@@:@@@@@_};=&&=!)@",
   "..&&&='!{{$/^{;=-;}$(@{$#...#.#+$$(@:@@:@@:]@@@$!!&==;)@",
   "..-&&&&[{{$$$$[=+.~$$>]]+.......+$_:@$:@@:@@]@@]1====!^@",
   "..-&&&'!{{$((@<!~++<>}{@$)^)*,+~[)(@@{@@:@@:@]@@)!!==!)@",
   "..,=&&&;)@$@@@$&=*=*=~$@@@@@$$'!')@@:@:@@:@@:@@@@);==;$@",
   "..&&&&==;{{>$);..,*&,)$$$$$$@@$^/(@@@:@@:@$@@)#%@$!===)@",
   "..-&&&&=!)_$$~+.#=!+<^}=&=)<^)==}@@:@@:@(~$$)!..$$;==!)@",
   "..-=&&=!!;$@1...#$2~+~....~=.../@{@@:@@:@);....#$$!==;)@",
   "..-=&&====[@{$$...=]@@@@@@}}.#+_@((@@:@@&.#....+$$===!^@",
   "..-&&=;===';@@3+..+${@{{$$~+.@@{@)):@(&~..!~.+~,$$!==!)@",
   "..,=&&&&&;&.#{@@'...~{%!....$@@,<3{@@:@,....#=.~@$;==;)@",
   "..&&;&&&&&,..$$]@@@$=..}$@@@@$@@^}(:@{}.;.=...+(]$;==;)@",
   "..-&&&&&&&&+#.$$($@]@@@@@@$]{{(@:$;$).......~#$@@>'==!^]",
   "..-&&&&&&&=&}+!$$$$${${(($@$<});@@,#&<#+...+.=@(@$[===)@",
   "..,=&&;&&&-#;')#$$$$<;)$+.....>)$((,#[)#!.;.,@{@@>'==!)@",
   "..-&&&&=;}.+42%/~{$$3)<%<1<<({{$$>{]^/~)1=.)_@$@@$!&==)@",
   "..-=&&&!}<1<)@@@@@@@@@]@@@@@@@]@({$$@$+#)^$@@@(@@>'=&!)@",
   "..=*=!!-((@@@@@@@@]@]@]@@@@@$@:@@@@@@]$~.=@@@]@@])!==;)@",
   "..-=!%{@@@@@@]@:@:@@@@@]@:@@$$@@@@@@@@@_}.~$:@:@@$!==!)@",
   "..-;}(@@@@@@]@@]@@:@:@:@@@:@:@:@:]@]@@@@@).+)@@@@$;==!^@",
   "..};}^))51))^)6))))1)^51))))6))51)^))${{{@)~~+$<)}!==;)@",
   "..==;;![;!;;!!;!4;!=;!!!;;;;!'4!=!;!;!)$$$@)#{{=!;====)@",
   "..-===&===============================!)$$$@]@{!=====!)@",
   "..-*=&&&&&====&&*==&==========&========;}$$${)=;=&===;)@",
   "..-=&=&&&&=!=*&&;===&=*==!=*=;&&=&&&&===!=$$$$)!===&=!^@",
   "..=!;!;;;!;;;;!;;;!!;!;!;;!;!!;!!;!;!;!!'!7'887=;!;!=+)@",
   "..)<1<<1<191<191<191<<<19191<191<191<191<1112111<191<1^]",
   "#{@@@@@]@@]@]@]@@@@@@@@@@@@@@@@@@@@@@@@@@@]@]@]]@@@@@::@" };

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
    { (RovhultAppl::initI18n (),
      _("_Game")),     _("<alt>G"), 0,     BRANCH },
    { _("_New"),      _("<ctl>N"), NEW,   ITEM },
    { "",             "",          0,     SEPARATOR },
    { _("E_xit"),     _("<ctl>Q"), EXIT,  ITEM },
    { _("_Help"),     _("<alt>H"), 0,     LASTBRANCH },
    { _("_About..."), _("<alt>a"), ABOUT, ITEM } };


/*--------------------------------------------------------------------------*/
//Purpose   : Defaultconstructor; all widget are created
/*--------------------------------------------------------------------------*/
RovhultAppl::RovhultAppl ()
   : XApplication (PACKAGE " - Rovhult V" VERSION), status ()
     , tblTable (16, 19), cards (), staple (CardPile::VERY_COMPRESSED) {
   set_usize (WIDTH, HEIGHT);

   addMenus (menuItems, sizeof (menuItems) / sizeof (menuItems[0]));

   // Create controls
   tblTable.show ();
   getClient ()->pack_start (tblTable, true, true, 5);

   status.push (1, _("Start a new game with Ctrl+N (or Game -> New)"));
   status.show ();
   getClient ()->pack_start (status, false);

   staple.show ();
   tblTable.attach (staple, 3, 4, 2, 4, 0, 0);

   show ();
   cards.load (staple.get_window ());   // Cards need an realized (!) parent be loaded
      fillStaple ();
}

/*--------------------------------------------------------------------------*/
//Purpose   : Destructor
/*--------------------------------------------------------------------------*/
RovhultAppl::~RovhultAppl () {
   TRACE9 ("RovhultAppl::~RovhultAppl ()");
   tblTable.hide ();
   status.hide ();
   hide ();
}


/*--------------------------------------------------------------------------*/
//Purpose   : Command-handler
//Parameters: menu: ID of command (menu)
/*--------------------------------------------------------------------------*/
void RovhultAppl::command (int menu) {
   switch (menu) {
   case NEW: {
      dealCards ();
      break;
   }

   case ABOUT: {
      XAbout* about (new XAbout ("Anticopyright (A) 2002 Markus Schwab"
                                 "\ne-mail: g17m0@lycos.com\n\n"
                                 "Compiled on " __DATE__ " - " __TIME__,
                                 PACKAGE " - Rovhult V" VERSION "." MICRO_VERSION));
      about->setIconProgram (xpmRovhult);
      about->setIconAuthor (xpmAuthor); }
      break;

   case EXIT:
      delete_event_impl (0);
      break;

   default:
      Check3 (0);
   } // end-switch
}

/*--------------------------------------------------------------------------*/
//Purpose   : Shuffles (Randomizes) and deals the cards
/*--------------------------------------------------------------------------*/
void RovhultAppl::fillStaple () {
   // Randomize cards into staple
   for (int i (cards.getCardNumber () - 1); i >= 0;) {
      int nr (rand () % cards.getCardNumber ());

      // Search if card already exist in staple (TODO?: Optimize?)
      CardWidget* card (cards.getCard (nr)); Check3 (card);
      if (staple.existCard (card))
         continue;

      TRACE9 ("RovhultAppl::fillStaple () - Add to staple: " << nr);
      staple.setTopCard (card, false);
      card->show ();
      --i;
   }
}

/*--------------------------------------------------------------------------*/
//Purpose   : Shuffles (Randomizes) and deals the cards
/*--------------------------------------------------------------------------*/
void RovhultAppl::dealCards () {
   TRACE9 ("RovhultAppl::dealCards ()");

   // Show cards on table
   for (int i (0); i < NUM_PLAYERS; ++i) {
      for (int j (0); j < 3; ++j) {
         CardWidget* pCard (staple.removeTopCard ()); Check3 (pCard);
         //         pCard->set_sensitive (false);

         tblTable.attach (*pCard, COLS_PLAYER[i]  + (j << 1),
                          COLS_PLAYER[i] + 1 + (j << 1), ROWS_PLAYER[i],
                          ROWS_PLAYER[i] + 2, 0, 0, 1);
         TRACE9 ("RovhultAppl::dealCards () - 1st set at: "
                 << COLS_PLAYER[i]  + (j << 1) << '/' << ROWS_PLAYER[i]);
      }
   } // endfor all players

   // Show cards in hand
   for (int i (0); i < NUM_PLAYERS; ++i) {
      for (int j (0); j < 3; ++j) {
         CardWidget* pCard (staple.removeTopCard ()); Check3 (pCard);
         pCard->show ();
         tblTable.attach (*pCard, COLS_PLAYER[i] + (j << 1),
                          COLS_PLAYER[i] + (j << 1)  + 1,
                          ROWS_PLAYER[i] + (i ? 3 : -3),
                          ROWS_PLAYER[i] + (i ? 3 : -3) + 2
                          , 0, 0, 1);
         TRACE9 ("RovhultAppl::dealCards () - 2nd set at: "
                 << COLS_PLAYER[i] + (j << 1) << '/'
                 << ROWS_PLAYER[i] + (i ? 3 : -3));
      }
   } // endfor all players
}

/*--------------------------------------------------------------------------*/
//Purpose   : Resize-method of application
/*--------------------------------------------------------------------------*/
void RovhultAppl::size_allocate_impl (GtkAllocation* size) {
   Check3 (size);
   TRACE9 ("RovhultAppl::size_request_impl -> new size: " << size->width << " * "
           << size->height);

   if ((size->width >= WIDTH) || (size->height >= HEIGHT)) {
      //tblTable.set_usize (size->width 600 - XWIDTH);
      XApplication::size_allocate_impl (size);
   }
}

/*--------------------------------------------------------------------------*/
//Purpose   : Initializes the programm for internationalication
/*--------------------------------------------------------------------------*/
void RovhultAppl::initI18n () {
   setlocale (LC_ALL, "");                          // Activate current locale
   bindtextdomain (PACKAGE, LOCALEDIR);     // Specify messagefile for gettext
   textdomain (PACKAGE);
}


/*--------------------------------------------------------------------------*/
//Purpose   : Entrypoint of application
//Parameters: argc: Number of parameters
//            argv: Array with pointer to parameter
//Returns   : int: Status
/*--------------------------------------------------------------------------*/
int main (int argc, char* argv[]) {
   srand (time (NULL));              // Initialize the random number generator

   Main appl (argc,argv);
   RovhultAppl win;
   appl.run ();
   return 0;
}
