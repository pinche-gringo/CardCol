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
   , cards (cardset), restart (false) {
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
   TRACE9 ("Game::start ()");
   Check3 ((statGame == INITIALIZING) || (statGame == STOPPED));
   if (statGame != INITIALIZING)
      clean ();

   setGameStatus (PLAYING);
   actPlayer = 0;
}

/*--------------------------------------------------------------------------*/
//Purpose   : Terminates the game and cleans the table
/*--------------------------------------------------------------------------*/
void Game::stop () {
   TRACE9 ("Game::stop ()");
   clean ();
   setGameStatus (STOPPED);
}

/*--------------------------------------------------------------------------*/
//Purpose   : End the current game as soon as possible
//Parameters: startNew: Flag, if game should be restarted
/*--------------------------------------------------------------------------*/
void Game::end (bool startNew) {
   TRACE9 ("Game::end () - Restart: " << (startNew ? "Yes" : "No"));

   restart = startNew;
   if (canBeStopped ()) {
      setGameStatus (STOPPED);
      actPlayer = 0;
      disableLastPlayer ();
   }
   else
      setGameStatus (TOSTOP);
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
void Game::movePile (ICardPile& dest, ICardPile& source, unsigned int start,
                     int end) {
   TRACE3 ("Game::movePile (ICardPile&, ICardPile&, unsigned int, int) - "
           "moving from pos " << start << " to " << end);
   Check3 (source.numberOfCards ());
   Check3 (start < source.numberOfCards ());
   
   if (end == -1)
      end = source.numberOfCards () - 1;
   Check3 (end < source.numberOfCards ()); Check3 (start <= end);

   while (end-- >= (int)start)
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
   if (actPlayer >= 0) {
      TRACE9 ("Game::makeComputerMoves () - *** Start timer ***");
      Gtk::Main::timeout.connect (slot (this, (actPlayer
                                               ? &Game::makeComputerMove
                                               : &Game::enableActPlayer)),
                                  actPlayer ? 700 : 50);
      disableLastPlayer ();
   }
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
      setGameStatus (STOPPED);
      if (restart)
         start ();
      return 0;
   }

   actPlayer = makeMove (actPlayer);
   TRACE9 ("Game::makeCompuerMove () - Next player: " << actPlayer);

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

/*--------------------------------------------------------------------------*/
//Purpose   : Changes the game-status
//Parameters: newStatus: Status to set
/*--------------------------------------------------------------------------*/
void Game::setGameStatus (unsigned int newStatus) {
   statGame = newStatus;
   control (statGame);
}

/*--------------------------------------------------------------------------*/
//Purpose   : Flips the cards the user is about to play
//Parameters: pile: Pile to manipulate
//            start: Position of first card to play; update to reflect moving
//            start: Position of last card to play; update to reflect moving
//Returns   : unsigned int: Changed position to play
/*--------------------------------------------------------------------------*/
void Game::flipCards2Play (ICardPile& pile, unsigned int& start, unsigned int& end) {
   TRACE2 ("Game::flipCards2Play (ICardPile&, unsigned int, unsigned int) - "
           "Cards from " << start << " to " << end);
   Check3 (end < pile.numberOfCards ());
   Check3 (start <= end);

   bool bFollow (false);
   do {
      CardWidget& card (pile.at (start));
      pile.move (pile.numberOfCards () - 1, start);
      card.showFace ();

      if ((pile.getStyle () != ICardPile::NORMAL)
          && bFollow) {
         Check3 (pile.numberOfCards () > 1);
         pile.resize (pile.numberOfCards () - 2, ICardPile::COMPRESSED);
      }
      bFollow = true;
   } while (end-- && (start <= end));

   start += pile.numberOfCards () - end - 2;
   end =  pile.numberOfCards () - 1;
   TRACE8 ("Game::flipCards2Play (ICardPile&, unsigned int, unsigned int) - "
           "New positions " << start << " and " << end);
}
