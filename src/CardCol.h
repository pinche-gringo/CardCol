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


#include <vector>

#include <gtkmm/table.h>
#include <gtkmm/button.h>
#include <gtkmm/statusbar.h>

#include <Mutex.h>
#include <Thread.h>

#include <CardSet.h>
#include <CardPile.h>
#include <CardImgs.h>

#include <ConnMgr.h>
#include <IVIOAppl.h>
#include <XApplication.h>

class Game;
class Options;
class ICarddeckSelectDlg;


// Class to handle the Rovhult-cardgame
class CardgameCollection : public XApplication {
 public:
   typedef enum { NONE = -1, GROVHULT = 0, GTWOPART, GHEARTS, GBURACO, GLAST } games;

   // Manager functions
   CardgameCollection (Options& opts);
   ~CardgameCollection ();

   Gtk::Box& getClient () { return *XApplication::getClient (); }
   Gtk::Statusbar& getStatusbar () { return status; }
   CardSet& getCards () { return cards; }
   const std::vector<Player*>& getPlayer () const;
   ConnectionMgr& getConnectionMgr () { return cmgr; };
   Mutex& getClientMutex () { return mxSerMsgs; }
   const unsigned int getPlayerPosition () const { return playerPos; }

 private:
   // IDs for menus
   enum { NEW = LAST, END, CONNECT, EXIT, DEBUG, ROVHULT, TWOPART, HEARTS,
          BURACO, CHGDECKS, CHGNAMES, SAVESET };

   // Protected manager functions
   CardgameCollection (const CardgameCollection&);
   const CardgameCollection& operator= (const CardgameCollection&);

   // Event-handling
   void closeDialog (int, const Gtk::Dialog* dlg);
   virtual void command (int menu);
   virtual void gameEvents (unsigned int status);
   virtual void showAboutbox ();
   virtual const char* getHelpfile ();

   void* changeCards (void* opt);
   void* loadCards (void*);
   void userWants2End ();
   void startGame ();

   void changeDecks (const ICarddeckSelectDlg& dialog);
   void changePlayernames ();

   void initCommunication ();
   void makePlayer ();
   void* waitForMessages (void*);
   bool handleErrorMessage (unsigned int player, char* msg);
   bool handleMessage (unsigned int player, char* msg);
   bool showMessage (char* msg);

   static XApplication::MenuEntry CardgameCollection::menuItems[];

   static const char* xpmGame[];
   static const char* xpmAuthor[];

   Gtk::Statusbar status;

   CardImages cardFaces;
   CardSet cards;

   typedef OThread<CardgameCollection> THRDAPPL;
   THRDAPPL* pThread;
   THRDAPPL* pCommThread;

   Options& options;
   Mutex mxSerMsgs;
   ConnectionMgr cmgr;
   std::vector<Player*> aPlayer;

   unsigned int playerPos;

   games oldGame;
   unsigned int restart;

   Game* game;

   static const unsigned int WIDTH;
   static const unsigned int HEIGHT;
};

#endif
