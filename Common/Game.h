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

#include <gtk--/table.h>

// Forward declarations
namespace Gtk {
   class Box;
   class Menu;
   class Statusbar;
};
class CardSet;
class ICardPile;


// Class to select the card decks to use
class Game : public Gtk::Table {
 public:
   enum { INITIALIZING, STOPPED, TOSTOP, PLAYING, LAST };

   Game (Gtk::Box& parent, Gtk::Statusbar& statusbar, CardSet& cardset,
         const vector<string>& playerNames, unsigned int rows, unsigned int columns);
   virtual ~Game ();

   // Managing
   virtual void start ();
   virtual void stop ();
   virtual void end (bool startNew);
   virtual void playOpen (bool) { }
   virtual void control (unsigned int status) const { }
   virtual void clean ();
   virtual const char* name () = 0;

   // Status handling
   bool isRunning () const { return statGame >= PLAYING; }
   virtual bool canBeStopped () const { return !actPlayer; }

   unsigned int gameStatus () const { return statGame; }
   void setGameStatus (unsigned int newStatus);

 protected:
   virtual int enableHuman ();
   virtual void disableHuman ();

   unsigned int currentPlayer () const { return actPlayer; }
   void setNextPlayer (unsigned int player) { actPlayer = player; }

   void flipCards2Play (ICardPile& pile, unsigned int& start, unsigned int& end);
   void displayTurn (unsigned int player);
   void displayTurn (unsigned int player, const std::string& preText);
   void makeNextMoves ();
   virtual int makeMove (unsigned int player) = 0;

   void randomizeCardsToPile (ICardPile& pile) const;
   static void movePile (ICardPile& dest, ICardPile& source,
                         unsigned int start = 0, int end = -1);

   // Handling of won cards (if any)
   gint wonCardsSelected (GdkEvent *event);
   void showWonCards (bool show = true);
   int  enableWonCards (ICardPile& pile) {
      pWonPile = &pile;
      return enableActWonCards (); }
   void disableWonCards ();

   void sortWonByNumber ();
   void sortWonByColor ();

   Gtk::Statusbar& status;
   CardSet& cards;

   vector<Gtk::Connection> activeCards;
   const vector<string>& names;

 private:
   int makeComputerMove ();

   unsigned int statGame;

   int actPlayer;                   // Player who is in turn (needed for timer)
   bool restart;

   vector<Gtk::Connection> wonCards;           // Connections to show won cards
   ICardPile*              pWonPile;
   Gtk::Menu*              pMenuPopSort;

   int  enableActWonCards ();
};


// Specialized Game to inform controler about status-changes
// The Controller must support a statusbar (accessed by getStatusbar), a cardset
// (accessed by getCards), a Gtk::Box, which can be accessed by getClient ()
// and a vector of names (accessed by getNames ())
// Parent must be derived from Game
template <class Parent, class Controller>
class TGame : public Parent {
 public:
   typedef void (Controller::*PCALLBACK) (unsigned int);

   TGame (Controller& controller, PCALLBACK callback)
      : Parent (controller.getClient (), controller.getStatusbar (),
                controller.getCards (), controller.getNames ())
      , obj (controller), pCallback (callback) { }
   virtual ~TGame () { }

   virtual void control (unsigned int status) const {
      (obj.*pCallback) (status);
   }

 private:
   Controller & obj;
   PCALLBACK pCallback;
};


#endif
