//$Id$

//PROJECT     : Cardgames
//SUBSYSTEM   : Common/Game
//REFERENCES  :
//TODO        :
//BUGS        :
//REVISION    : $Revision$
//AUTHOR      : Markus Schwab
//CREATED     : 10.9.2002
//COPYRIGHT   : Anticopyright (A) 2002, 2003

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

#include <glibmm/main.h>

#include <gtkmm/box.h>
#include <gtkmm/menu.h>
#include <gtkmm/statusbar.h>

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
//            playerNames: Vector of player names
//            rows: Number of rows needed by game
//            columns: Number of columns needed by game
/*--------------------------------------------------------------------------*/
Game::Game (Gtk::Box& parent, Gtk::Statusbar& statusbar, CardSet& cardset,
            const std::vector<Glib::ustring>& playerNames, unsigned int rows,
            unsigned int columns)
   : Gtk::Table (rows, columns), statGame (INITIALIZING), status (statusbar)
     , cards (cardset), restart (false), pWonPile (NULL), pMenuPopSort (NULL)
     , names (playerNames) {
   TRACE3 ("Game::Game (Gtk::Box&, Gtk::Statusbar&, Cardset&, unsinged int, unsigned int)");
   Check3 (cardset.size ());

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
   delete pMenuPopSort;
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
      disableHuman ();
   }
   else
      setGameStatus (TOSTOP);
}

/*--------------------------------------------------------------------------*/
//Purpose   : Disables the cards the human player can select
/*--------------------------------------------------------------------------*/
void Game::disableHuman () {
   TRACE2 ("Game::disableHuman () - " << activeCards.size () << " cards");

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
   pile.setTopCards (cards.getCards ());
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
   Check3 (source.size ());
   Check3 (start < source.size ());
   
   if (end == -1)
      end = source.size () - 1;
   Check1 (end < source.size ()); Check1 (start <= end);

   do {
      dest.append (source.remove (start));
   } while ((unsigned int)end-- > start);
}

/*--------------------------------------------------------------------------*/
//Purpose   : Cleans the table
/*--------------------------------------------------------------------------*/
void Game::clean () {
   TRACE9 ("Game::clean ()");
   disableWonCards ();
}

/*--------------------------------------------------------------------------*/
//Purpose   : Activates the next player
/*--------------------------------------------------------------------------*/
void Game::makeNextMoves () {
   if (actPlayer >= 0) {
      TRACE9 ("Game::makeNextMoves () - *** Start timer *** for player " << actPlayer);
      Glib::signal_timeout ().connect
         (slot (*this, (actPlayer
                        ? &Game::makeComputerMove
                        : &Game::enableHuman)), actPlayer ? 700 : 50);
      disableHuman ();
   }
}

/*--------------------------------------------------------------------------*/
//Purpose   : Enables the cards of the human player
//Returns   : int: false
/*--------------------------------------------------------------------------*/
bool Game::enableHuman () {
   TRACE9 ("Game::enableHuman () - enabling player " << actPlayer);
   return false;
}

/*--------------------------------------------------------------------------*/
//Purpose   : Makes the move for the next player.
//Returns   : int: Flag for timer, if it should continue (0: no; else: yes)
/*--------------------------------------------------------------------------*/
bool Game::makeComputerMove () {
   TRACE5 ("Game::makeComputerMove () - Turn of player " << actPlayer);

   if (statGame == TOSTOP) {
      TRACE8 ("Game::makeComputerMove () - End game; Restart: "
              << (restart ? "Yes" : "No"));
      setGameStatus (STOPPED);
      if (restart)
         // Restart the game, when idle (means: *after* this signalhandler
         // terminates)
         Glib::signal_idle ().connect
             (bind_return (slot (*this, &Game::start), false));
      return false;
   }

   actPlayer = makeMove (actPlayer);
   TRACE9 ("Game::makeComputerMove () - Next player: " << actPlayer);
   if (!actPlayer)
      enableHuman ();
   return actPlayer > 0;
}

/*--------------------------------------------------------------------------*/
//Purpose   : Displays information about whose turn it is
//Parameters: player: Player in turn
/*--------------------------------------------------------------------------*/
void Game::displayTurn (unsigned int player) {
   Check1 (player < names.size ());
   status.pop ();
   Glib::ustring stat (_("Turn of %1"));
   stat.replace (stat.find ("%1"), 2, names[player]);
   status.push (stat);
}

/*--------------------------------------------------------------------------*/
//Purpose   : Displays information about whose turn it is
//Parameters: player: Player in turn
/*--------------------------------------------------------------------------*/
void Game::displayTurn (unsigned int player, const Glib::ustring& preText) {
   status.pop ();
   Glib::ustring stat (_("Turn of %1"));
   stat.replace (stat.find ("%1"), 2, names[player]);
   status.push (preText + stat);
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
   Check3 (end < pile.size ());
   Check3 (start <= end);

   unsigned int s (start);
   unsigned int e (end);
   bool bFollow (false);
   do {
      CardWidget& card (*pile[s]);
      pile.move (pile.size () - 1, s);
      card.showFace ();

      if ((pile.getStyle () != ICardPile::NORMAL)
          && bFollow) {
         Check3 (pile.size () > 1);
         pile.resize (pile.size () - 2, ICardPile::COMPRESSED);
      }
      bFollow = true;
   } while (e-- && (s <= e));

   start = pile.size () - 1 - (end - start);
   end =  pile.size () - 1;
   TRACE8 ("Game::flipCards2Play (ICardPile&, unsigned int, unsigned int) - "
           "New positions " << start << " and " << end);
}

/*--------------------------------------------------------------------------*/
//Purpose   : Shows or hides the won cards
//Parameters: show: Flag if to show or to hide the cards
/*--------------------------------------------------------------------------*/
void Game::showWonCards (bool show) {
   if (pWonPile) {
      pWonPile->setShowOption (show ? ICardPile::SHOWFACE : ICardPile::SHOWBACK);
      pWonPile->setStyle (show ? ICardPile::COMPRESSED : ICardPile::VERY_COMPRESSED);
      Glib::signal_timeout ().connect
         (slot (*this, &Game::enableActWonCards), 50);
      disableWonCards ();
   }
}

/*--------------------------------------------------------------------------*/
//Purpose   : Callback for any event for the top of the won cards
//Parameters: event: Caused event
/*--------------------------------------------------------------------------*/
bool Game::wonCardsSelected (GdkEvent* event) {
   TRACE2 ("Game::wonCardsSelected (GdkEvent*) - " << event->type);

   if (event->type == GDK_BUTTON_PRESS) {
      GdkEventButton* bev ((GdkEventButton*)(event));
      switch (bev->button) {
      case 1:
         Check3 (pWonPile);
         showWonCards (pWonPile->getShowOption () == ICardPile::SHOWBACK);
         break;

      case 3: {
         if (!pMenuPopSort) {
            TRACE9 ("Game::wonCardsSelected (GdkEvent*) - Creating menu");
            pMenuPopSort = new Gtk::Menu;
            pMenuPopSort->items ().push_back (Gtk::Menu_Helpers::MenuElem
                                              (_("Sort by number"),
                                               slot (*this, &Game::sortWonByNumber)));
            pMenuPopSort->items ().push_back (Gtk::Menu_Helpers::MenuElem
                                              (_("Sort by colour"),
                                               slot (*this, &Game::sortWonByColour)));
         }
         pMenuPopSort->popup (bev->button, bev->time);
         break; }
      }
      return true;
   }

   return false;
}

/*--------------------------------------------------------------------------*/
//Purpose   : Shows and sorts the won cards by number
/*--------------------------------------------------------------------------*/
void Game::sortWonByNumber () {
   TRACE8 ("Game::sortWonByNumber ()");
   Check3 (pWonPile);
   pWonPile->sortByNumber ();
   showWonCards ();
   Glib::signal_timeout ().connect (slot (*this, &Game::enableActWonCards), 50);
   disableWonCards ();
}

/*--------------------------------------------------------------------------*/
//Purpose   : Shows and sorts the won cards by colour
/*--------------------------------------------------------------------------*/
void Game::sortWonByColour () {
   TRACE8 ("Game::sortWonByColour ()");
   Check3 (pWonPile);
   pWonPile->sortByColour ();
   showWonCards ();
   Glib::signal_timeout ().connect (slot (*this, &Game::enableActWonCards), 50);
   disableWonCards ();
}

/*--------------------------------------------------------------------------*/
//Purpose   : Enables the actual won cards
/*--------------------------------------------------------------------------*/
bool Game::enableActWonCards () {
   disableWonCards ();

   Check3 (pWonPile);
   TRACE9 ("Game::enableActWonCards () - Enabling " << pWonPile->size ()
           << " cards");
   for (int i (pWonPile->size ()); i;)
      wonCards.push_back
         ((*pWonPile)[--i]->signal_event ().connect
          (slot (*this, (&Game::wonCardsSelected))));
   return false;
}

/*--------------------------------------------------------------------------*/
//Purpose   : Disables the won cards
/*--------------------------------------------------------------------------*/
void Game::disableWonCards () {
   TRACE9 ("Game::disableWonCards () - Disabling " << wonCards.size () << " cards");
   for (int i (wonCards.size ()); i > 0;)
      wonCards[--i].disconnect ();
   
   wonCards.clear ();
}

/*--------------------------------------------------------------------------*/
//Purpose   : Changes the names of the playing people
//Parameters: newNames: Array holding the new names of the players
/*--------------------------------------------------------------------------*/
void Game::changeNames (const std::vector<Glib::ustring>& newNames) {
   const_cast<std::vector<Glib::ustring>&> (names) = newNames;
}
