//$Id$

//PROJECT     : Cardgames
//SUBSYSTEM   : Hearts
//REFERENCES  :
//TODO        :
//BUGS        :
//REVISION    : $Revision$
//AUTHOR      : Markus Schwab
//CREATED     : 24.12.2002
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


#define CHECK 9
#define TRACELEVEL 9
#include <Check.h>
#include <Trace_.h>

#include "Hearts.h"


const unsigned int Hearts::COLS_PLAYER[NUM_PLAYERS] = { 3, 9, 3,  1 };
const unsigned int Hearts::ROWS_PLAYER[NUM_PLAYERS] = { 3, 7, 9, 7 };


/*--------------------------------------------------------------------------*/
//Purpose   : Constructor
//Parameters: parent: Parent widget to display the game in
//            statusbar: Status bar widget to display information about the game
//            cardset: Cardset to use
/*--------------------------------------------------------------------------*/
Hearts::Hearts (Box& parent, Statusbar& statusbar, CardSet& cardset)
   : Game (parent, statusbar, cardset, 10, 10)
     , startPlayer (rand () % NUM_PLAYERS) {
   TRACE9 ("Hearts::Hearts (Box&, Statusbar&, CardSet&)");

   unsigned int width (cards.getCard (0).getImageWidth ());
   unsigned int height (cards.getCard (0).getImageHeight ());

   // Show and attach card-piles
   for (int i (0); i < NUM_PLAYERS; ++i) {
      players[i].won.show ();
      attach (players[i].won, COLS_PLAYER[i],
              COLS_PLAYER[i] + ((i & 1) ? 1 : 5),
              ROWS_PLAYER[i] + ((i == 2) ? 2 : -2),
              ROWS_PLAYER[i] + ((i == 2) ? 2 : -2) + 1,
              0, 0, 1);

      TRACE9 ("Hearts::Hearts () - Set at: "
              << COLS_PLAYER[i] << '/' << ROWS_PLAYER[i] + ((i == 2) ? 2 : -2));

      players[i].hand.show ();
      attach (players[i].hand, COLS_PLAYER[i],
              COLS_PLAYER[i] + ((i & 1) ? 1 : 5), ROWS_PLAYER[i],
              ROWS_PLAYER[i] + 1, 0, 0, 1);
      TRACE9 ("Hearts::Hearts () - 2nd set at: "
              << COLS_PLAYER[i] << '/' << ROWS_PLAYER[i]);

      players[i].won.setShowOption (ICardPile::SHOWBACK);
      // players[i].hand.setShowOption (i ? ICardPile::SHOWBACK : ICardPile::SHOWFACE);
      players[i].hand.setShowOption (ICardPile::SHOWFACE);

      players[i].won.set_usize (width + 12 * 7, height + 5);
      players[i].hand.set_usize (width + 12 * 18, height + 5);
   }

   // Show played area
   played.show ();
   attach (played, 3, 4, 5, 8, 0, 0, 0, 5);
   played.set_usize (width + 150, height);
}

/*--------------------------------------------------------------------------*/
//Purpose   : Destructor
/*--------------------------------------------------------------------------*/
Hearts::~Hearts () {
   TRACE9 ("Hearts::~Hearts ()");
}


/*--------------------------------------------------------------------------*/
//Purpose   : Makes the move for the next player.
//Parameters: player: Actual player
//Returns   : int: Next player or -1 if end of game
/*--------------------------------------------------------------------------*/
int Hearts::makeMove (unsigned int player) {
   TRACE5 ("Hearts::makeMove () - Turn of player " << player);
   Check3 (gameStatus () == PLAYING);

   enablePlayer (player);
   return (player + 1) & 0x3;

}

/*--------------------------------------------------------------------------*/
//Purpose   : Starts the game by dealing the cards
/*--------------------------------------------------------------------------*/
void Hearts::start () {
   Game::start ();

   Check1 (!played.numberOfCards ());
   played.hide ();
   randomizeCardsToPile (played);
   for (unsigned int i (0); i < NUM_PLAYERS; ++i) {
      for (unsigned int j (0); j < cards.numberOfCards () / NUM_PLAYERS; ++j)
         players[i].hand.insertColorSorted (played.removeTopCard ());

      players[i].hand.setStyle (ICardPile::COMPRESSED);
      players[i].won.setStyle (ICardPile::QUITE_COMPRESSED);
   }
   played.show ();

   TRACE9 ("Hearts::start () - Player: " << startPlayer);
   displayTurn (startPlayer);
   enablePlayer (startPlayer);
}

/*--------------------------------------------------------------------------*/
//Purpose   : Remove cards from everything which can hold them
/*--------------------------------------------------------------------------*/
void Hearts::clean () {
   for (unsigned int i (0); i < NUM_PLAYERS; ++i) {
      players[i].hand.clear ();
      players[i].hand.clear ();
   }

   played.clear ();
   disableLastPlayer ();
}

/*--------------------------------------------------------------------------*/
//Purpose   : Shows or hides the cards of the computer player
//Parameters: open: Flag if cards should be shown or hidden
/*--------------------------------------------------------------------------*/
void Hearts::playOpen (bool open) {
   // TODO
}

/*--------------------------------------------------------------------------*/
//Purpose   : Enables the cards of the passed player
//Parameters: player: Player to enable
//Remarks   : Depending of the status of the game (PLAYING2) also the top
//            card of the played pile is enabled
/*--------------------------------------------------------------------------*/
void Hearts::enablePlayer (unsigned int player) {
   Check3 (activeCards.empty ());
   Check3 (gameStatus () == PLAYING);

   TRACE2 ("Hearts::enablePlayer (unsigned int) - player "
           << player << " has " << players[player].hand.numberOfCards ()
           << " cards");

   for (int i (players[player].hand.numberOfCards ()); i;)
      activeCards.push_back
         (players[player].hand.at (--i).clicked.connect_after
           (bind (slot (this, (&Hearts::cardSelected)), player, i)));
}

/*--------------------------------------------------------------------------*/
//Purpose   : Callback after clicking on a card in hand
//Parameters: player: ID of player
//            iCard: Offset of card in hand
/*--------------------------------------------------------------------------*/
void Hearts::cardSelected (unsigned int player, unsigned int iCard) {
   TRACE5 ("Hearts::cardSelected (unsigned int, unsigned int) - Player: "
           << player << " at position " << iCard);
   Check3 (player <= NUM_PLAYERS);
   Check3 (iCard <= players[player].hand.numberOfCards ());
   Check3 (gameStatus () == PLAYING);
}
