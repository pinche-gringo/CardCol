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
#include <vector>

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

   Gtk::Box& getClient () { return *XApplication::getClient (); }
   Gtk::Statusbar& getStatusbar () { return status; }
   CardSet& getCards () { return cards; }
   const vector<string>& getNames () const { return names; }

 private:
   // IDs for menus
   enum { NEW = 1, END, EXIT, DEBUG, ROVHULT, TWOPART, HEARTS, CHGDECKS, CHGNAMES,
          SAVESET, ABOUT, CONTENT};

   // Protected manager functions
   CardgameCollection (const CardgameCollection&);
   const CardgameCollection& operator= (const CardgameCollection&);

   // Event-handling
   virtual void command (int menu);
   virtual void gameEvents (unsigned int status);
   virtual gint delete_event_impl (_GdkEventAny*) {
      command (EXIT);
      return 1; }

   void changeCards (void* opt);
   void loadCards ();
   void userWants2End (unsigned int input);
   void startGame ();

   void changeDecks (const ICarddeckSelectDlg& dialog);

   static XApplication::MenuEntry CardgameCollection::menuItems[];

   static const char* xpmGame[];
   static const char* xpmAuthor[];

   Gtk::Statusbar status;

   CardImages cardFaces;
   CardSet cards;

   typedef OThread<CardgameCollection> THRDAPPL;
   THRDAPPL* pThread;

   std::string pathDeck;
   std::string pathBack;

   typedef enum { NONE = -1, GROVHULT = 0, GTWOPART, GHEARTS } games;
   games typeGame;
   games oldGame;
   bool restart;

   Game* game;

   vector<string> names;

   static const std::string NAME_INIFILE;

   static const unsigned int USED_CARDS = 52;

   static const unsigned int WIDTH = 760;
   static const unsigned int HEIGHT = 730;
};

#endif
