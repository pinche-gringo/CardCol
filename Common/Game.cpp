//$Id$

//PROJECT     : Cardgames
//SUBSYSTEM   : Common/Game
//REFERENCES  :
//TODO        :
//BUGS        :
//REVISION    : $Revision$
//AUTHOR      : Markus Schwab
//CREATED     : 10.9.2002
//COPYRIGHT   : Anticopyright (A) 2002

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


#include <gtk--/box.h>
#include <gtk--/statusbar.h>

#include <Check.h>
#include <Trace_.h>

#include "CardSet.h"
#include "CardPile.h"

#include "Game.h"


/*--------------------------------------------------------------------------*/
//Purpose   : Constructor
//Parameters: parent: Parent of widget
//            statusbar: For messages
//            cardset: Cardset
//            rows: Number of rows needed by game
//            columns: Number of columns needed by game
/*--------------------------------------------------------------------------*/
Game::Game (Gtk::Box& parent, Gtk::Statusbar& statusbar, CardSet& cardset,
            unsigned int rows, unsigned int columns)
   : Gtk::Table (rows, columns), statGame (STOPPED), status (statusbar)
   , cards (cardset) {
   TRACE3 ("Game::Game (Gtk::Box&, Gtk::Statusbar&, Cardset&, unsinged int, unsigned int)");

   show ();
   set_col_spacings (2);
   set_row_spacings (2);

   parent.pack_start (*this, true, true, 5);
}

/*--------------------------------------------------------------------------*/
//Purpose   : Destructor
/*--------------------------------------------------------------------------*/
Game::~Game () {
   TRACE9 ("Game::~Game ()");
}


/*--------------------------------------------------------------------------*/
//Purpose   : Starts the game
/*--------------------------------------------------------------------------*/
void Game::start () {
   statGame = PLAYING;
}

/*--------------------------------------------------------------------------*/
//Purpose   : Starts the game
/*--------------------------------------------------------------------------*/
void Game::stop () {
   statGame = STOPPED;
}

/*--------------------------------------------------------------------------*/
//Purpose   : Disables the cards of the last player
/*--------------------------------------------------------------------------*/
void Game::disableLastPlayer () {
   TRACE2 ("Game::disableLastPlayer () - " << activeCards.size () << " cards");

   for (int i (activeCards.size ()); i > 0;)
      activeCards[--i].disconnect ();
   
   activeCards.clear ();
}

/*--------------------------------------------------------------------------*/
//Purpose   : Shuffles (Randomizes) the cards onto the staple
/*--------------------------------------------------------------------------*/
void Game::randomizeCardsToPile (ICardPile& pile) const {
   // Randomize and put cards onto staple
   cards.shuffle ();
   pile.setTopCards (cards.getCards (), false);
}

