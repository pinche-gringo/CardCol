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


#include <cardgames-cfg.h>

#include <vector>

#include <gtkmm/button.h>
#include <gtkmm/statusbar.h>

#include <YGP/Mutex.h>
#include <YGP/Thread.h>
#include <YGP/ConnMgr.h>
#include <YGP/Exception.h>

#include <CardSet.h>
#include <CardPile.h>
#include <CardImgs.h>

#include <XGP/XApplication.h>


namespace Gtk {
   class Dialog;
}

class Game;
class Player;
class Options;
class ChatDlg;
class ICarddeckSelectDlg;


/**Class to handle the cardgame collection
 */
class CardgameCollection : public XGP::XApplication {
 public:
   // Manager functions
   CardgameCollection (Options& opts);
   ~CardgameCollection ();

   Gtk::Box& getClient () { return *XGP::XApplication::getClient (); }
   Gtk::Statusbar& getStatusbar () { return status; }
   CardSet& getCards () { return cards; }
   const std::vector<Player*>& getPlayer () const { return aPlayer; }
   YGP::ConnectionMgr& getConnectionMgr () { return cmgr; };
   YGP::Mutex& getClientMutex () { return mxThreadCmd; }
   const unsigned int getPlayerPosition () const { return playerPos; }

 private:
   // IDs for menus
   enum { NEW = 0, END,
#ifdef HAVE_LIBPTHREAD
	  CONNECT, CHAT,
#endif
	  STARTGAMES_MENU = CHAT,
#ifdef WITH_BURACO
	  BURACO,
#endif
#ifdef WITH_HEARTS
	  HEARTS,
#endif
#ifdef WITH_MACHIAVELLI
	  MACHIAVELLI,
#endif
#ifdef WITH_ROVHULT
	  ROVHULT,
#endif
#ifdef WITH_SGTMAYOR
	  SGTMAYOR,
#endif
#ifdef WITH_TWOPART
	  TWOPART,
#endif
	  LAST };
   Glib::RefPtr<Gtk::Action> apMenus[LAST];

   // Protected manager functions
   CardgameCollection (const CardgameCollection&);
   const CardgameCollection& operator= (const CardgameCollection&);

   // Event-handling
   void newGame ();
   void endGame ();
#ifdef HAVE_LIBPTHREAD
   void autoConnect (const Options& options);
   void connect ();
   void showChatDlg ();
   void closeChat (int);
   bool stopClientWaiting ();
   void removeCommThreads ();
   void initCommunication ();
   void* waitForMessages (void*);
   int handleGlobalMessage (unsigned int player, const std::string& msg) throw (YGP::ParseError);
   bool handleMessage (unsigned int player, const std::string msg);
   void sendMessage (const Glib::ustring& msg);
   void broadcastMsg (const std::string& msg, unsigned int exclude = -1U);
   void broadcastNames ();
#endif
   void exit ();
   void changeGame (int game);
   void showChangeDeckDlg ();
   void changeNames ();
   void editPreferences ();
   void savePreferences ();
#if TRACELEVEL >= 0
   void toggleDebug ();
#endif

   bool showMessage (const std::string msg);
   static void closeDialog (int, const Gtk::Dialog* dlg);
   virtual void gameEvents (unsigned int status);
   virtual void showAboutbox ();
   virtual const char* getHelpfile ();

   void* changeCards (void* opt);
   void loadCards ();
   bool restartGame ();
   void startGame ();
   void doStartGame ();

   bool terminateGameAndExit ();
   bool wait4EndGameAndExit ();

   void changeDecks (const std::string& deck, const std::string& back);
   void changePlayernames ();
   void makePlayer ();

   static const char* xpmGame[];
   static const char* xpmAuthor[];

   Gtk::Statusbar status;

   CardImages cardFaces;
   CardSet cards;

#ifdef HAVE_LIBPTHREAD
   typedef YGP::OThread<CardgameCollection> THRDAPPL;
   std::vector<THRDAPPL*> aCommThreads;
   YGP::Mutex mxThreadCmd;
   YGP::Mutex mxGuiCmd;
   YGP::ConnectionMgr cmgr;
   unsigned int playerPos;

   ChatDlg* dlgChat;
#endif

   Options& options;
   std::vector<Player*> aPlayer;

   int oldGame, actGame;
   unsigned int restart;

   Game* game;

   static const unsigned int WIDTH;
   static const unsigned int HEIGHT;
};

#endif
