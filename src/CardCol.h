#ifndef CARDCOL_H
#define CARDCOL_H

//$Id$

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

#include <string>

#include <gtk--/table.h>
#include <gtk--/button.h>
#include <gtk--/statusbar.h>

#include <Thread.h>


#include <CardSet.h>
#include <CardPile.h>
#include <CardImgs.h>

#include <XApplication.h>

class Game;
class ICarddeckSelectDlg;
enum ICarddeckSelectDlg::commands;


// Class to handle the Rovhult-cardgame
class CardgameCollection : public XApplication {
 public:
   // Manager functions
   CardgameCollection ();
   ~CardgameCollection ();

 private:
   // IDs for menus
   enum { NEW, EXIT, DEBUG, ROVHULT, TWOPART, CHGDECKS, SAVESET, ABOUT };

   // Protected manager functions
   CardgameCollection (const CardgameCollection&);
   const CardgameCollection& operator= (const CardgameCollection&);

   // Event-handling
   virtual void command (int menu);

   void changeCards (void* opt);
   void loadCards ();
   void userWants2End (unsigned int input);
   void startGame ();

   void changeDecks (ICarddeckSelectDlg::commands cmd);

   static XApplication::MenuEntry CardgameCollection::menuItems[];

   static const char* xpmGame[];
   static const char* xpmAuthor[];

   Gtk::Statusbar status;

   MenuItem* pMenuNew;

   CardImages cardFaces;
   CardSet cards;

   typedef OThread<CardgameCollection> THRDAPPL;
   THRDAPPL* pThread;

   ICarddeckSelectDlg* dlgChgDecks;
   std::string pathDeck;
   std::string pathBack;

   typedef enum { NONE, GROVHULT, GTWOPART } games;
   games typeGame;

   Game* game;

   static const std::string NAME_INIFILE;

   static const unsigned int USED_CARDS = 52;

   static const unsigned int WIDTH = 760;
   static const unsigned int HEIGHT = 700;

   static games oldGame;
};

#endif
