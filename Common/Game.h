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
   void end () { statGame = TOSTOP; }
   virtual void playOpen () { }

   // Status handling
   bool isRunning () const { return statGame >= PLAYING; }
   virtual bool canBeStopped () const { return true; }

 protected:
   enum { INITIALIZING, STOPPED, TOSTOP, PLAYING, LAST };

   virtual void disableLastPlayer ();

   void randomizeCardsToPile (ICardPile& pile) const;

   unsigned int statGame;

   Gtk::Statusbar& status;
   CardSet& cards;

   vector<Gtk::Connection> activeCards;

 private:
};

#endif
