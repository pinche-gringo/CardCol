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

#include <gtk--/statusbar.h>

#include <XMessageBox.h>

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
     , played (ICardPile::COMPRESSED, ICardPile::SHOWFACE)
     , startPlayer (rand () % NUM_PLAYERS), heartsPlayed (false) {
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
      players[i].hand.setShowOption (i ? ICardPile::SHOWBACK : ICardPile::SHOWFACE);

      players[i].won.set_usize (width + 12 * 7, height + 5);
      players[i].hand.set_usize (width + 12 * 18, height + 5);

      players[i].hand.setStyle (i ? ICardPile::QUITE_COMPRESSED : ICardPile::COMPRESSED);
      players[i].won.setStyle (ICardPile::QUITE_COMPRESSED);
   }

   // Show played area
   played.setStyle (ICardPile::COMPRESSED);
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
   Check1 (gameStatus () == PLAYING);

   enablePlayer (player);
   return (player + 1) & 0x3;

}

/*--------------------------------------------------------------------------*/
//Purpose   : Starts the game by dealing the cards
/*--------------------------------------------------------------------------*/
void Hearts::start () {
   TRACE9 ("Hearts::start ()");
   Game::start ();

   Check2 (!played.numberOfCards ());
   cards.shuffle ();
   for (unsigned int i (0); i < NUM_PLAYERS; ++i)
      for (unsigned int j (0); j < (cards.numberOfCards () / NUM_PLAYERS); ++j)
         players[i].hand.insertColorSorted
            (cards.getCard (i * (cards.numberOfCards () / NUM_PLAYERS) + j));

   setGameStatus (EXCHANGE);
   heartsPlayed = false;

   TRACE9 ("Hearts::start () - Player: " << startPlayer);

   status.pop (1);
   status.push (1, _("Select 3 cards to give away"));
   enablePlayer (0);
}

/*--------------------------------------------------------------------------*/
//Purpose   : Remove cards from everything which can hold them
/*--------------------------------------------------------------------------*/
void Hearts::clean () {
   TRACE9 ("Hearts::clean ()");
   for (unsigned int i (0); i < NUM_PLAYERS; ++i) {
      players[i].hand.clear ();
      players[i].won.clear ();
   }

   played.clear ();
   disableLastPlayer ();
}

/*--------------------------------------------------------------------------*/
//Purpose   : Shows or hides the cards of the computer player
//Parameters: open: Flag if cards should be shown or hidden
/*--------------------------------------------------------------------------*/
void Hearts::playOpen (bool open) {
   for (int i (1); i < NUM_PLAYERS; ++i) {
      players[i].hand.setShowOption (open ? ICardPile::SHOWFACE : ICardPile::SHOWBACK);
      players[i].hand.setStyle (open ? ICardPile::COMPRESSED : ICardPile::QUITE_COMPRESSED);
   }
}

/*--------------------------------------------------------------------------*/
//Purpose   : Enables the cards of the passed player
//Parameters: player: Player to enable
//Remarks   : Depending of the status of the game (PLAYING2) also the top
//            card of the played pile is enabled
/*--------------------------------------------------------------------------*/
void Hearts::enablePlayer (unsigned int player) {
   Check1 (activeCards.empty ());
   Check1 ((gameStatus () == PLAYING) || (gameStatus () == EXCHANGE));

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
   Check1 (player < NUM_PLAYERS);
   Check1 (iCard < players[player].hand.numberOfCards ());
   Check1 ((gameStatus () == PLAYING) || (gameStatus () == EXCHANGE));

   if (moveSelectedCardToPlayed (player, iCard)) {
      disableLastPlayer ();

      // Set next player, if either a the player exchanged his 3 cards or
      // played his card.
      if ((gameStatus () == PLAYING)
          || ((gameStatus () == EXCHANGE)
              && (played.numberOfCards () == 3))) {
         // Exchange the cards in pre-play
         if (gameStatus () == EXCHANGE) {
            TRACE7 ("Hearts::cardSelected (unsigned int, unsigned int) - "
                    "Finished exchange");
            exchangeCards ();
            movePile (players[1].hand, played, 0, 2);
            players[1].hand.sortByColor ();
            setGameStatus (PLAYING);

            // Search for startplayer
            for (unsigned int i (0); i < NUM_PLAYERS; ++i)
               if ((players[i].hand.at (0).number () == CardWidget::TWO)
                   && (players[i].hand.at (0).color () == CardWidget::HEARTS)) {
                  setNextPlayer (player = i);
                  break;
               }
         }
         else
            setNextPlayer (player = (player + 1) & 0x3);
         displayTurn (player);
      }
      makeNextMoves ();
   }
}

/*--------------------------------------------------------------------------*/
//Purpose   : Moves the selected card to the played pile
//Parameters: player: ID of player
//            card: Offset of card to play
//Returns   : bool: Status of moving; true: Card could be moved; false else
/*--------------------------------------------------------------------------*/
bool Hearts::moveSelectedCardToPlayed (unsigned int player, unsigned int card) {
   TRACE5 ("Hearts::moveSelectedCardToPlayed (unsigned int, unsigned int) - Player: "
           << player << " at position " << card);
   Check1 (player < NUM_PLAYERS);
   Check1 (card < players[player].hand.numberOfCards ());
   Check1 ((gameStatus () == PLAYING) || (gameStatus () == EXCHANGE));

   if (gameStatus () == PLAYING) {
      CardWidget::COLORS playColor (players[player].hand.at (card).color ());
      if (played.numberOfCards ()) {
         // The same color must be played again (if available)
         CardWidget::COLORS color (played.at (0).color ());
         if ((playColor != color) && players[player].hand.exists (color)) {
            XMessageBox::Show (_("Play first cards with an equal color than "
                                 "the first played one!"), PACKAGE " - Hearts",
                               XMessageBox::ERROR);
            return false;
         }
      }
      else
         // One can start with a heart only if there has been one played before
         if (!heartsPlayed && (playColor == CardWidget::HEARTS)
             && (players[player].hand.at (0).color () != CardWidget::HEARTS)) {
            XMessageBox::Show (_("You can't start with a heart, if they have"
                                 " not been played before!"), PACKAGE " - Hearts",
                               XMessageBox::ERROR);
            return false;
         }

      if (playColor == CardWidget::HEARTS)
         heartsPlayed = true;
   }

   movePile (played, players[player].hand, card, card);
   return true;
}

/*--------------------------------------------------------------------------*/
//Purpose   : Exchanges the cards of the computer players
/*--------------------------------------------------------------------------*/
void Hearts::exchangeCards () {
   TRACE9 ("Hearts::exchangeCards ()");

   for (unsigned int i (3); i; --i) {
      unsigned int nrColors[4] = { 0, 0, 0, 0 };
      ICardPile& pile (players[i].hand);
      getNumberOfColors (pile, nrColors);

      // Now get rid of cards according the following algorithm:
      //   - If nr. of clubs or diamonds are < 3 -> Use them
      //   - If nr. of spades < 5 get rid of high spades (especially the queen)
      //   - Get rid of high cards
      //   - Get rid of high hearts
      unsigned int cards2Move[3];
      unsigned int pos (0);
      if (nrColors[0] < 3)
         TRACE3 ("Hearts::exchangeCards () - Getting rid of all clubs");
         for (unsigned int c (0); c < nrColors[0]; ++c)
            cards2Move[pos++] = c;
      if (nrColors[1] < ((sizeof (cards2Move) / sizeof (cards2Move[0])) - pos)) {
         TRACE3 ("Hearts::exchangeCards () - Getting rid of all diamonds");
         for (unsigned int c (0); c < nrColors[0]; ++c)
            cards2Move[pos++] = nrColors[0] + c;
      }

      if (nrColors[2] < 5) {
         // TODO: Check for queen
         unsigned endSpades (nrColors[0] + nrColors[1] + nrColors[2] - 3 + pos);
         while (pos < 3)
            cards2Move[pos++] = endSpades++;
      }

      int cardPos;
      for (unsigned int nr (CardWidget::ACE);
           (pos < 3) && (nr > (unsigned int)CardWidget::NINE); --nr)
         if ((cardPos = pile.find (CardWidget::NUMBERS (nr))) != -1) {
            cards2Move[pos++] = cardPos;
         }

      cardPos = pile.numberOfCards () - 3 + pos;
      while (pos < 3)
         cards2Move[pos++] = cardPos++;
      TRACE5 ("Hearts::exchangeCards () - Exchanging: " << cards2Move[0] << ", "
              << cards2Move[1] << ", " << cards2Move[2]);

      do {
         --pos;
         movePile (players[(i + 1) & 0x3].hand, pile, pos, pos);
      } while (pos);
      players[(i + 1) & 0x3].hand.sortByColor ();
   }
}

/*--------------------------------------------------------------------------*/
//Purpose   : Calculate the number of cards for each color
//Parameters: pile: Pile to inspect
//            result: Array of number of cards for earch color
/*--------------------------------------------------------------------------*/
void Hearts::getNumberOfColors (ICardPile& pile, unsigned int result[4]) {
   unsigned int oldPos (-1U);

   for (unsigned int i (0); i < (pile.numberOfCards () - 1); ++i)
      if (pile.at (i).color () != pile.at (i + 1).color ()) {
         result[pile.at (i).color ()] = i - oldPos;
         oldPos = i;
      }
   result[pile.at (pile.numberOfCards () - 1).color ()] =
      pile.numberOfCards () - 1 - oldPos;
   TRACE9 ("Hearts::getNumberOfColors (ICardPile&, unsigned int) - Nr. of "
           "cards: " << result[0] << ", " << result[1]
           << ", " << result[2] << ", " << result[3]);
}
