#ifndef GAME_H
#define GAME_H

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

#include <gtkmm/table.h>

#include <YGP/Mutex.h>


// Forward declarations
namespace Gtk {
   class Box;
   class Menu;
   class Dialog;
   class Statusbar;
};
namespace YGP {
class Socket;
class ConnectionMgr;
}
class Player;
class CardSet;
class ICardPile;


/**Abstract base class providing usefull methods for card games.
*/
class Game : public Gtk::Table {
 public:
   /// Stati of the game
   enum { NONE = 0,                        ///< Class created; game not started
          INITIALIZING,          ///< Initialization phase (dealing cards, ...)
          STOPPED,                                     ///< Game has been ended
          TOSTOP,        ///< Game should be ended (but can't be at the moment)
          PLAYING,                                    ///< Game is being played
          LAST };

   Game (Gtk::Box& parent, Gtk::Statusbar& statusbar, CardSet& cardset,
         const std::vector<Player*>& player, unsigned int posPlayer,
         YGP::Mutex& mxSerialize, unsigned int rows, unsigned int columns);
   virtual ~Game ();

   /// \name Managing
   //@{
   virtual void start ();
   virtual void stop ();
   virtual void end (bool startNew);
   virtual void playOpen (bool) { }
   /// Informs the parent about status changes
   virtual void control (unsigned int status) const;
   virtual YGP::ConnectionMgr& getConnectionMgr () const = 0;
   virtual void clean ();
   virtual const char* name () = 0;
   virtual void changeNames (const std::vector<Player*>& newPlayer);
   //@}

   /// \name Carddeck information
   //@{
   virtual unsigned int numberOfDecks () const { return 1; }
   virtual unsigned int numberOfJokers () const { return 0; }
   //@}

   virtual bool handleMessage (unsigned int player, const std::string& msg) throw (std::string);
   bool ignoreMessage ();

   /// \name Status handling
   //@{
   /// Checks if the game is being played
   bool isRunning () const { return statGame >= PLAYING; }
   /// Checks if the game can be stopped at the moment (only when it's the
   /// turn of the human)
   virtual bool canBeStopped () const;

   /// Handling the actual game status
   unsigned int gameStatus () const { return statGame; }
   void setGameStatus (unsigned int newStatus);
   bool isShowingCardsToPlay () const { return pos1Play != -1U; }
   //@}

   void setPlayerPosition (unsigned int posPlayer) { posServer = posPlayer; }

   virtual void disableHuman ();
   void endTurn () { stati.pendingTurn = 0; }

   /// \name Player actions
   //@{
   virtual bool enableHuman ();
   bool makeComputerMove ();
   //@}

   /// \name Methods to make the game load/save before dealing
   //@{
   void setCardOrder (const char* order) { cardOrder = order; data = cardOrder.data (); }
   const char* getCardOrder () const { return cardOrder.data (); }
   void clearCardOrder () { cardOrder.clear () ; data = NULL; }
   //@}

 protected:
   virtual ICardPile* getPileOfPlayer (unsigned int player, unsigned int pile) = 0;
   virtual bool executeRemoteMove (ICardPile& pile, unsigned int target);
   virtual unsigned int getActTarget () const;

   /// \name Communication helper methods
   //@{
   static void writeError (YGP::Socket& socket, unsigned int rc, const std::string& msg);
   static void writeOK (YGP::Socket& socket) { return writeMessage (socket, "Error=0"); }
   static void writeMessage (YGP::Socket& socket, const std::string& msg);
   void broadcastMessage (const std::string& msg) const;
   //@}

   /// Returns the current player
   unsigned int currentPlayer () const { return actPlayer; }
   /// Sets the next player
   void setNextPlayer (unsigned int player);

   void flipCards2Play (ICardPile& pile, unsigned int& start, unsigned int& end);
   void flipCards2Play (ICardPile& pile, const std::string& cards) throw (std::string);
   void displayTurn (unsigned int player);
   void displayTurn (unsigned int player, const Glib::ustring& preText);
   void makeNextMoves ();
   bool endRemoteMove (unsigned int player);
   virtual int makeMove (unsigned int player) = 0;

   bool randomizeCardsToPile (ICardPile& pile) const;
   static void movePile (ICardPile& dest, ICardPile& source,
                         unsigned int start = 0, int end = -1);

   bool performCommand (unsigned int player, const std::string& msg) throw (std::string);
   static bool stringToNumber (unsigned long& number, const char* text);

   // Handling of won cards (if any)
   bool wonCardsSelected (GdkEvent *event);
   void showWonCards (bool show = true);
   int  enableWonCards (ICardPile& pile) {
      pWonPile = &pile;
      return enableActWonCards (); }
   void disableWonCards ();

   void sortWonByNumber ();
   void sortWonByColour ();

   Gtk::Statusbar& status;
   CardSet& cards;

   std::vector<SigC::Connection> activeCards;
   const std::vector<Player*>&   actPlayers;

   YGP::Mutex& mxSerializeMsgs;

   unsigned int posServer;     ///< Position the player occupies for the server

   unsigned int pos2Play;                    ///< Upper border of cards to play
   unsigned int pos1Play;                    ///< Lower border of cards to play

   unsigned int ignoreNextMsg;

 private:
   bool endGame (bool startNew);

   bool enableActWonCards ();

   const char* data;                                   // Data send from server

   unsigned int statGame;

   int actPlayer;                   // Player who is in turn (needed for timer)
   struct {
      int restart : 1;
      int pendingTurn : 1;
   } stati;

   std::vector<SigC::Connection> wonCards;     // Connections to show won cards
   ICardPile*                    pWonPile;
   Gtk::Menu*                    pMenuPopSort;

   std::string cardOrder;
};


/**Specialized Game to inform controler about status-changes.

   The Controller must support a statusbar (accessed by getStatusbar), a
   cardset (accessed by getCards), a Gtk::Box, which can be accessed by
   getClient (), a vector of players (accessed by getPlayer ()) and a method
   called getConnectionMgr to retrieve a ConnectionMgr object.

   \remarks Parent must be derived from Game
*/
template <class Parent, class Controller>
class TGame : public Parent {
 public:
   typedef void (Controller::*PCALLBACK) (unsigned int);

   TGame (Controller& controller, PCALLBACK callback)
      : Parent (controller.getClient (), controller.getStatusbar (),
                controller.getCards (), controller.getPlayer (),
                controller.getPlayerPosition (), controller.getClientMutex ())
      , obj (controller), pCallback (callback) { }
   virtual ~TGame () { }

   /// 
   virtual void control (unsigned int status) const {
      (obj.*pCallback) (status);
   }

   virtual YGP::ConnectionMgr& getConnectionMgr () const {
      return obj.getConnectionMgr ();
   }

 private:
   Controller& obj;
   PCALLBACK pCallback;
};


#endif
