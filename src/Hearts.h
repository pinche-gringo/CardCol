#ifndef HEARTS_H
#define HEARTS_H

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


#include <CardSet.h>
#include <CardPile.h>

#include <Game.h>

namespace Gtk {
   class Box;
};

using namespace Gtk;


// Class to handle the Hearts cardgame
class Hearts : public Game {
 public:
   Hearts (Box& parent, Statusbar& statusbar, CardSet& cardset);
   virtual ~Hearts ();

   virtual void start ();
   virtual void clean ();
   virtual void playOpen (bool open);
   virtual const char* name () { return "Hearts"; }

 private:
   // Protected manager functions
   Hearts (const Hearts& other);
   const Hearts& operator= (const Hearts& other);

   // Event handling
   void cardSelected (unsigned int player, unsigned int iCard);

   virtual int makeMove (unsigned int player);
   virtual void enablePlayer (unsigned int player);


   static const unsigned int NUM_PLAYERS = 4;              // Number of players

   unsigned int startPlayer;

   struct {
      CardHPile hand;                         // For players: Cards in the hand
      CardHPile won;                            // Reserve-cards (for end-game)
   } players[NUM_PLAYERS];
   CardHPile played;

   static const unsigned int COLS_PLAYER[NUM_PLAYERS];
   static const unsigned int ROWS_PLAYER[NUM_PLAYERS];
};

#endif
