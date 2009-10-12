#ifndef GAME_H
#define GAME_H

//$Id: Game.h,v 1.1 2009/06/14 07:03:27 g17m0 Exp $

// This file is part of CardCol.
//
// CardCol is free software: you can redistribute it and/or modify
// it under the terms of the GNU General Public License as published by
// the Free Software Foundation, either version 3 of the License, or
// (at your option) any later version.
//
// CardCol is distributed in the hope that it will be useful,
// but WITHOUT ANY WARRANTY; without even the implied warranty of
// MERCHANTABILITY or FITNESS FOR A PARTICULAR PURPOSE.  See the
// GNU General Public License for more details.
//
// You should have received a copy of the GNU General Public License
// along with CardCol.  If not, see <http://www.gnu.org/licenses/>.


#include <cardgames-cfg.h>

#include <string>
#include <vector>

#include <gtkmm/table.h>
#include <gtkmm/uimanager.h>

#include <YGP/Mutex.h>
#include <YGP/Exception.h>

#include <card/Pile.h>


// Forward declarations
namespace Gtk {
   class Box;
   class Menu;
   class Dialog;
   class MenuShell;
   class Statusbar;
}
namespace YGP {
   class Socket;
   class ConnectionMgr;
}
namespace Card {
   class Set;
   class Player;
   class Widget;
   class Window;
   class PileWindow;
   class PileWindows;
}


namespace Card {

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

   Game (Gtk::Box& parent, Gtk::Statusbar& statusbar, Set& cardset,
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
   virtual void addMenus (Glib::RefPtr<Gtk::UIManager> mgrUI);
   virtual void removeMenus (Glib::RefPtr<Gtk::UIManager> mgrUI);
   virtual void resizeCards ();
   //@}

   /// \name Carddeck information
   //@{
   virtual unsigned int numberOfDecks () const { return 1; }
   virtual unsigned int numberOfJokers () const { return 0; }
   //@}

   virtual bool handleMessage (unsigned int player, const std::string& msg) throw (YGP::ParseError, YGP::CommError);
   bool ignoreMessage ();

   /// \name Status handling
   //@{
   /// Checks if the game is being played
   /// \returns bool True, if the game is running (i.e. can't be interrupted at the moment)
   bool isRunning () const { return statGame >= PLAYING; }
   /// Checks if the game can be stopped at the moment (only when it's the
   /// turn of the human)
   virtual bool canBeStopped () const;

   /// Handling the actual game status
   /// \returns unsigned int Status of the game
   unsigned int gameStatus () const { return statGame; }
   void setGameStatus (unsigned int newStatus);
   bool isShowingCardsToPlay () const { return pos1Play != -1U; }
   //@}

   /// Sets the position of the game/player for a network game; this
   /// position is relative to the server (i.e. the player inviting
   /// the (other) players and starting the game
   /// \param posPlayer Position of player as seen from the server
   void setPlayerPosition (unsigned int posPlayer) { posServer = posPlayer; }

   virtual void disableHuman ();
   /// Sets the internal status of the game to turn ended
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
   virtual IPile* getPileOfPlayer (unsigned int player, unsigned int pile) = 0;
   virtual bool executeRemoteMove (IPile& pile, unsigned int target) throw (YGP::ParseError);
   virtual unsigned int getActTarget () const;

   /// \name Communication helper methods
   //@{
   static void writeError (YGP::Socket& socket, unsigned int rc, const std::string& msg);
   static void writeOK (YGP::Socket& socket) { return writeMessage (socket, "Error=0"); }
   static void writeMessage (YGP::Socket& socket, const std::string& msg);
   void broadcastMessage (const std::string& msg) const;
   void broadcastStartPlayer (unsigned int startplayer);
   //@}

   /// Returns the current player
   unsigned int currentPlayer () const { return actPlayer; }
   /// Sets the next player
   void setNextPlayer (unsigned int player);

   void flipCards2Play (IPile& pile, unsigned int& start, unsigned int& end);
   void flipCards2Play (IPile& pile, const std::string& cards) throw (YGP::ParseError);
   void displayTurn (unsigned int player);
   void displayTurn (unsigned int player, const Glib::ustring& preText);
   void makeNextMoves ();
   bool endRemoteMove (unsigned int player);
   virtual void makeMove (unsigned int player) = 0;

   bool randomiseCardsToPile (IPile& pile) const;

   /// \name Animation
   //@{
   Window& animateCard (IPile& dest, IPile& src, unsigned int pos) {
      return animateCard (dest, dest.size (), src, pos); }
   Window& animateCard (IPile& dest, unsigned int posDest, IPile& src, unsigned int pos);
   PileWindow& animateCards (IPile& dest, IPile& src, unsigned int start, unsigned int end) {
      return animateCards (dest, dest.size (), src, start, end); }
   PileWindow& animateCards (IPile& dest, unsigned int posDest, IPile& src,
			     unsigned int start, unsigned int end);
   PileWindows& animateCards2 (IPile& dest, IPile& src, unsigned int start, unsigned int end) {
      return animateCards2 (dest, dest.size (), src, start, end); }
   PileWindows& animateCards2 (IPile& dest, unsigned int posDest, IPile& src,
			       unsigned int start, unsigned int end);
   //@}

   bool performCommand (unsigned int player, const std::string& msg) throw (YGP::ParseError, YGP::CommError);
   static bool stringToNumber (unsigned long& number, const char* text);

   // Handling of won cards (if any)
   bool wonCardsSelected (GdkEvent *event);
   virtual void showWonCards (bool show = true, unsigned int style = -1U);
   int  enableWonCards (IPile& pile) {
      pWonPile = &pile;
      return enableActWonCards (); }
   void disableWonCards ();

   void sortWonByNumber ();
   void sortWonByColour ();

   Gtk::Statusbar& status;
   Set& cards;

   std::vector<sigc::connection> activeCards;
   const std::vector<Player*>&   actPlayers;

   YGP::Mutex& mxSerializeMsgs;

   unsigned int posServer;     ///< Position the player occupies for the server

   unsigned int pos2Play;                    ///< Upper border of cards to play
   unsigned int pos1Play;                    ///< Lower border of cards to play

   unsigned int ignoreNextMsg;       ///< Number of received messages to ignore

 private:
   Game (const Game&);
   Game& operator= (const Game&);

   bool endGame (bool startNew);

   bool enableActWonCards ();

   const char* data;                                   // Data send from server

   unsigned int statGame;

   int actPlayer;                   // Player who is in turn (needed for timer)
   struct {
      int restart : 1;
      int pendingTurn : 1;
   } stati;

   std::vector<sigc::connection> wonCards;     // Connections to show won cards
   IPile*                        pWonPile;
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

   /// Constructor
   /// \param controller Object controlling the game
   /// \param callback Method of object to call in case of changes of status
   TGame (Controller& controller, PCALLBACK callback)
      : Parent (controller.getClient (), controller.getStatusbar (),
                controller.getCards (), controller.getPlayer (),
                controller.getPlayerPosition (), controller.getClientMutex ())
      , obj (controller), pCallback (callback) { }
   /// Destructor
   virtual ~TGame () { }

   /// Callback to inform a controller about status changes
   /// \param status New status of the game
   virtual void control (unsigned int status) const {
      (obj.*pCallback) (status);
   }

   /// Returns the connection-manager
   virtual YGP::ConnectionMgr& getConnectionMgr () const {
      return obj.getConnectionMgr ();
   }

 private:
   Controller& obj;
   PCALLBACK pCallback;
};

}

#endif
