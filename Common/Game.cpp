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

#include <cardgames-cfg.h>

#include <gtk--/box.h>
#include <gtk--/main.h>
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
   : Gtk::Table (rows, columns), statGame (INITIALIZING), status (statusbar)
   , cards (cardset), reStart (false) {
   TRACE3 ("Game::Game (Gtk::Box&, Gtk::Statusbar&, Cardset&, unsinged int, unsigned int)");
   Check3 (cardset.numberOfCards ());

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
   clean ();
}


/*--------------------------------------------------------------------------*/
//Purpose   : Starts the game
/*--------------------------------------------------------------------------*/
void Game::start () {
   statGame = PLAYING;

   actPlayer = 0;
}

/*--------------------------------------------------------------------------*/
//Purpose   : Starts the game
/*--------------------------------------------------------------------------*/
void Game::stop () {
   clean ();
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

/*--------------------------------------------------------------------------*/
//Purpose   : Moves cards from one pile to another
//Parameters: dest: Destination pile
//            source: Source pile
/*--------------------------------------------------------------------------*/
void Game::movePile (ICardPile& dest, ICardPile& source, unsigned int start) {
   while (source.numberOfCards () > start)
      dest.append (source.remove (start));
}

/*--------------------------------------------------------------------------*/
//Purpose   : Cleans the table
/*--------------------------------------------------------------------------*/
void Game::clean () {
   TRACE9 ("Game::clean ()");
}

/*--------------------------------------------------------------------------*/
//Purpose   : Activates the computer player
/*--------------------------------------------------------------------------*/
void Game::makeNextMoves () {
   TRACE9 ("Game::makeComputerMoves () - *** Start timer ***");
   Gtk::Main::timeout.connect (slot (this, (actPlayer
                                     ? &Game::makeComputerMove
                                     : &Game::enableActPlayer)),
                               actPlayer ? 1000 : 50);
   disableLastPlayer ();
}

/*--------------------------------------------------------------------------*/
//Purpose   : Enables the cards of the actual player
/*--------------------------------------------------------------------------*/
int Game::enableActPlayer () {
   enablePlayer (actPlayer);
   return 0;
}

/*--------------------------------------------------------------------------*/
//Purpose   : Enables the cards of the passed player
//Parameters: player: Player whose cards should be enabled
/*--------------------------------------------------------------------------*/
void Game::enablePlayer (unsigned int player) {
}

/*--------------------------------------------------------------------------*/
//Purpose   : Makes the move for the next player.
//Returns   : int: Flag for timer, if it should continue (0: no; else: yes)
/*--------------------------------------------------------------------------*/
int Game::makeComputerMove () {
   TRACE5 ("Game::makeComputerMove () - Turn of player " << actPlayer);
   Check3 (actPlayer);

   if (statGame == TOSTOP) {
      TRACE8 ("Game::makeCompuerMove () - End game ");
      stop ();
      return 0;
   }

   actPlayer = makeMove (actPlayer);

   if (!actPlayer)
      enablePlayer (0);
   return actPlayer > 0;
}

/*--------------------------------------------------------------------------*/
//Purpose   : Displays information about whose turn it is
//Parameters: player: Player in turn
/*--------------------------------------------------------------------------*/
void Game::displayTurn (unsigned int player) {
   status.pop (1);
   std::string stat (_("Turn of player %1"));
   stat.replace (stat.find ("%1"), 2, (char)(player + '0'));
   status.push (1, stat);
}

/*--------------------------------------------------------------------------*/
//Purpose   : Displays information about whose turn it is
//Parameters: player: Player in turn
/*--------------------------------------------------------------------------*/
void Game::displayTurn (unsigned int player, const std::string& preText) {
   status.pop (1);
   std::string stat (_("Turn of player %1"));
   stat.replace (stat.find ("%1"), 2, (char)(player + '0'));
   status.push (1, preText + stat);
}
