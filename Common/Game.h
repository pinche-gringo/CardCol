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
   class Statusbar;
};
class CardSet;
class ICardPile;


// Class to select the card decks to use
class Game : public Gtk::Table {
 public:
   Game (Gtk::Box& parent, Gtk::Statusbar& statusbar, CardSet& cardset,
         unsigned int rows, unsigned int columns);
   virtual ~Game ();

   // Managing
   virtual void start ();
   virtual void stop ();
   void end (bool restart = false) { reStart = restart; statGame = TOSTOP; }
   virtual void playOpen (bool) { }

   // Status handling
   bool isRunning () const { return statGame >= PLAYING; }
   virtual bool canBeStopped () const { return true; }

 protected:
   enum { INITIALIZING, STOPPED, TOSTOP, PLAYING, LAST };

   virtual int enableActPlayer ();
   virtual void enablePlayer (unsigned int player);
   virtual void disableLastPlayer ();

   unsigned int currentPlayer () const { return actPlayer; }
   void setNextPlayer (unsigned int player) { actPlayer = player; }

   void displayTurn (unsigned int player);
   void displayTurn (unsigned int player, const std::string& preText);
   void makeNextMoves ();
   virtual int makeMove (unsigned int player) = 0;

   virtual void clean ();
   void randomizeCardsToPile (ICardPile& pile) const;
   static void movePile (ICardPile& dest, ICardPile& source,
                         unsigned int start = 0);

   unsigned int statGame;
   bool reStart;

   Gtk::Statusbar& status;
   CardSet& cards;

   vector<Gtk::Connection> activeCards;

 private:
   int makeComputerMove ();

   int actPlayer;                   // Player who is in turn (needed for timer)
};

#endif
