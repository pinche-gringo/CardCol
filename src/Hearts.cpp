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


#include <string.h>

#include <Check.h>
#include <Trace_.h>

#include <gdk/gdk.h>

#include <gtk--/menu.h>
#include <gtk--/menuitem.h>
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
     , pos2Play (-1U), pScoreDlg (NULL) {
   TRACE9 ("Hearts::Hearts (Box&, Statusbar&, CardSet&)");

   unsigned int width (cards.getCard (0).getImageWidth ());
   unsigned int height (cards.getCard (0).getImageHeight ());

   // Show and attach card-piles
   for (int i (0); i < NUM_PLAYERS; ++i) {
      players[i].won.show ();
      attach (players[i].won, COLS_PLAYER[i],
              COLS_PLAYER[i] + ((i & 1) ? 1 : 5),
              ROWS_PLAYER[i] + ((i == 2) ? 2 : -2),
              ROWS_PLAYER[i] + ((i == 2) ? 2 : -2) + 1, GTK_EXPAND);

      TRACE9 ("Hearts::Hearts () - Set at: "
              << COLS_PLAYER[i] << '/' << ROWS_PLAYER[i] + ((i == 2) ? 2 : -2));

      players[i].hand.show ();
      attach (players[i].hand, COLS_PLAYER[i],
              COLS_PLAYER[i] + ((i & 1) ? 1 : 5), ROWS_PLAYER[i],
              ROWS_PLAYER[i] + 1, GTK_EXPAND);
      TRACE9 ("Hearts::Hearts () - 2nd set at: "
              << COLS_PLAYER[i] << '/' << ROWS_PLAYER[i]);

      players[i].won.setShowOption (ICardPile::SHOWBACK);
      players[i].hand.setShowOption (i ? ICardPile::SHOWBACK : ICardPile::SHOWFACE);

      players[i].won.set_usize (width + 12 * 7, height + 5);
      players[i].hand.set_usize (width + 12 * 18, height + 5);

      players[i].hand.setStyle (i ? ICardPile::QUITE_COMPRESSED : ICardPile::COMPRESSED);
      players[i].won.setStyle (ICardPile::VERY_COMPRESSED);
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
   delete pScoreDlg;
}


/*--------------------------------------------------------------------------*/
//Purpose   : Makes the move for the next player.
//Parameters: player: Actual player
//Returns   : int: Next player or -1 if end of game
/*--------------------------------------------------------------------------*/
int Hearts::makeMove (unsigned int player) {
   TRACE5 ("Hearts::makeMove () - Turn of player " << player);
   Check1 (gameStatus () == PLAYING);
   Check3 (player);

   if (pos2Play == -1U) {
      pos2Play = findPos2Play (player);
      TRACE8 ("Hearts::makeMove (unsigned int) - Going to play card at pos " << pos2Play);
      flipCards2Play (players[player].hand, pos2Play, pos2Play);
   }
   else {
      TRACE9 ("Hearts::makeMove (unsigned int) - Playing card at pos " << pos2Play);
      ICardPile& pile (players[player].hand);
      aPlayed[pile.at (pos2Play).color ()]++;
      movePile (played, pile, pos2Play, pos2Play);
      pos2Play = -1U;
      player = check4Winner (player);
   }
   return player;
}

/*--------------------------------------------------------------------------*/
//Purpose   : Starts the game by dealing the cards
/*--------------------------------------------------------------------------*/
void Hearts::start () {
   TRACE9 ("Hearts::start ()");
   Game::start ();

   // Hide won pile again (if not in debug-mode)
#if TRACELEVEL > 0
   if (players[1].won.getShowOption () == ICardPile::SHOWBACK)
#endif
      showWonCards (false);

   Check2 (!played.numberOfCards ());
   cards.shuffle ();
   for (unsigned int i (0); i < NUM_PLAYERS; ++i)
      for (unsigned int j (0); j < (cards.numberOfCards () / NUM_PLAYERS); ++j)
         players[i].hand.insertColorSorted
            (cards.getCard (i * (cards.numberOfCards () / NUM_PLAYERS) + j));

   setGameStatus (EXCHANGE);

   status.pop (1);
   status.push (1, _("Select 3 cards to give away"));
   enableHuman ();
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
   disableHuman ();
   Game::clean ();
}

/*--------------------------------------------------------------------------*/
//Purpose   : Shows or hides the cards of the computer player
//Parameters: open: Flag if cards should be shown or hidden
/*--------------------------------------------------------------------------*/
void Hearts::playOpen (bool open) {
   for (int i (1); i < NUM_PLAYERS; ++i) {
      players[i].hand.setShowOption (open ? ICardPile::SHOWFACE : ICardPile::SHOWBACK);
      players[i].hand.setStyle (open ? ICardPile::COMPRESSED : ICardPile::QUITE_COMPRESSED);
      players[i].won.setShowOption (open ? ICardPile::SHOWFACE : ICardPile::SHOWBACK);
      players[i].won.setStyle (open ? ICardPile::COMPRESSED : ICardPile::VERY_COMPRESSED);
   }
   players[0].won.setShowOption (open ? ICardPile::SHOWFACE : ICardPile::SHOWBACK);
   players[0].won.setStyle (open ? ICardPile::COMPRESSED : ICardPile::VERY_COMPRESSED);
}

/*--------------------------------------------------------------------------*/
//Purpose   : Enables the cards of the human player
//Returns   : 0
//Remarks   : Depending of the status of the game (PLAYING2) also the top
//            card of the played pile is enabled
/*--------------------------------------------------------------------------*/
int Hearts::enableHuman () {
   Check1 (activeCards.empty ());
   Check1 ((gameStatus () == PLAYING) || (gameStatus () == EXCHANGE));

   TRACE2 ("Hearts::enableHuman () - Human has " << players[0].hand.numberOfCards ()
           << " cards");

   for (int i (players[0].hand.numberOfCards ()); i;)
      activeCards.push_back
         (players[0].hand.at (--i).clicked.connect_after
           (bind (slot (this, (&Hearts::cardSelected)), i)));

   if (gameStatus () == EXCHANGE)
      for (int i (played.numberOfCards ()); i;)
         activeCards.push_back
            (played.at (--i).clicked.connect_after
             (bind (slot (this, (&Hearts::takeCard)), i)));

   return Game::enableHuman ();
}

/*--------------------------------------------------------------------------*/
//Purpose   : Callback after clicking on a card in the played field
//Parameters: iCard: Offset of card in hand
/*--------------------------------------------------------------------------*/
void Hearts::takeCard (unsigned int iCard) {
   TRACE9 ("Hearts::takeCard (unsigned int) - Picking up card " << iCard);
   Check1 (iCard < played.numberOfCards ());
   Check1 (gameStatus () == EXCHANGE);

   movePile (players[0].hand, played, iCard, iCard);
   players[0].hand.sortByColor ();
   makeNextMoves ();
}

/*--------------------------------------------------------------------------*/
//Purpose   : Callback after clicking on a card in hand
//Parameters: iCard: Offset of card in hand
/*--------------------------------------------------------------------------*/
void Hearts::cardSelected (unsigned int iCard) {
   TRACE5 ("Hearts::cardSelected (unsigned int) - Position " << iCard);
   Check1 (iCard < players[0].hand.numberOfCards ());
   Check1 ((gameStatus () == PLAYING) || (gameStatus () == EXCHANGE));

   // Hide won pile again (if not in debug-mode)
#if TRACELEVEL > 0
   if (players[1].won.getShowOption () == ICardPile::SHOWBACK)
#endif
      showWonCards (false);

   if (moveSelectedCardToPlayed (0, iCard)) {
      if (gameStatus () == PLAYING)
         setNextPlayer (check4Winner (0));
      else {
         Check3 (gameStatus () == EXCHANGE);
         if (played.numberOfCards () == 3) {
            memset (aPlayed, 0, sizeof (aPlayed));

            // Exchange the cards in pre-play
            TRACE7 ("Hearts::cardSelected (unsigned int) - Finished exchange");
            exchangeCards ();
            movePile (players[1].hand, played);
            players[1].hand.sortByColor ();
            setGameStatus (PLAYING);

            // Search for startplayer
            unsigned int nextPlayer (0);
            for (unsigned int i (1); i < NUM_PLAYERS; ++i)
               if ((players[i].hand.at (0).number () == CardWidget::TWO)
                   && (players[i].hand.at (0).color () == CardWidget::CLUBS)) {
                  TRACE7 ("Hearts::cardSelected (unsigned int) - Start with player " << i);
                  setNextPlayer (nextPlayer = i);
                  flipCards2Play (players[i].hand, pos2Play = 0, pos2Play);
                  break;
               }
            displayTurn (nextPlayer);
         }
      }
      makeNextMoves ();
   }
}

/*--------------------------------------------------------------------------*/
//Purpose   : Checks if the round is at end and gives the cards to winner if so
//Parameters: player: ID of player who did the last turn
//Returns   : Next player
/*--------------------------------------------------------------------------*/
unsigned int  Hearts::check4Winner (unsigned int player) {
   if (played.numberOfCards () == NUM_PLAYERS) {
      // Everyone played its card: Search for winner of played pile;
      // clear it and continue with winner
      CardWidget::COLORS color (played.at (0).color ());
      CardWidget::NUMBERS highest (CardWidget::TWO);
      unsigned int pos (0);
      for (unsigned int i (0); i < NUM_PLAYERS; ++i)
         if ((played.at (i).color () == color)
             && (played.at (i).number () > highest)) {
            pos = i;
            highest = played.at (i).number ();
            TRACE9 ("Hearts::check4Winner (unsinged int, unsinged int) - "
                    "New high card at " << i);
         }
      setNextPlayer (player = ((player - NUM_PLAYERS + pos + 1) & 0x3));;
      TRACE4 ("Hearts::check4Winner (unsinged int, unsinged int) - "
              "Continuing with player " << player);
      movePile (players[player].won, played);
   }
   else
      player = ((player + 1) & 0x3);

   if (!players[player].hand.numberOfCards ()) {
      player = -1U;
      setGameStatus (STOPPED);
      if (!pScoreDlg)
         pScoreDlg = HeartsScoreDlg::perform ();

      unsigned int aScore[NUM_PLAYERS];
      for (unsigned int i (0); i < NUM_PLAYERS; ++i) {
         aScore[i] = pointsOfPile (players[i].won);
         if (aScore[i] == 26) {
            aScore[0] = aScore[1] = aScore[2] = aScore[3] = 26;
            aScore[i] = 0;
            break;
         }
      }

      pScoreDlg->addPoints (aScore);
      pScoreDlg->show ();

      status.pop (1);
      status.push (1, _("Round ended"));
   }
   else
      displayTurn (player);

   if (!player)
      enableWonCards (players[0].won);
   return player;
}

/*--------------------------------------------------------------------------*/
//Purpose   : Moves the selected card to the played pile
//Parameters: player: ID of player
//            card: Offset of card to play
//Returns   : Status of moving; true: Card could be moved; false else
/*--------------------------------------------------------------------------*/
bool Hearts::moveSelectedCardToPlayed (unsigned int player, unsigned int card) {
   TRACE5 ("Hearts::moveSelectedCardToPlayed (unsigned int, unsigned int) - Player: "
           << player << " at position " << card);
   Check1 (player < NUM_PLAYERS);
   Check1 (card < players[player].hand.numberOfCards ());
   Check1 ((gameStatus () == PLAYING) || (gameStatus () == EXCHANGE));

   if (gameStatus () == PLAYING) {
      CardWidget& card (players[player].hand.at (card));
      CardWidget::COLORS playColor (card.color ());
      unsigned int cardsPlayed (0);
      for (unsigned int i (0); i < NUM_PLAYERS; ++i)
         cardsPlayed += players[i].won.numberOfCards ();

      if (played.numberOfCards ()) {
         // The same color must be played again (if available)
         CardWidget::COLORS color (played.at (0).color ());
         if ((playColor != color) && players[player].hand.exists (color)) {
            XMessageBox::Show (_("Play first cards with an equal color as "
                                 "the first played one!"), PACKAGE " - Hearts",
                               XMessageBox::ERROR);
            return false;
         }
      }
      else {
         // The game must be started with the two of clubs
         if (!cardsPlayed) {
            if ((card.color () != CardWidget::CLUBS)
                && (card.number () != CardWidget::TWO)) {
               XMessageBox::Show (_("The game must be started with the two of clubs!"),
                                  PACKAGE " - Hearts", XMessageBox::ERROR);
               return false;
            }
         }

         // One can start with a heart only if there has been one played before
         if (((playColor == CardWidget::HEARTS) && !aPlayed[CardWidget::HEARTS])
             && (players[player].hand.at (0).color () != CardWidget::HEARTS)) {
            XMessageBox::Show (_("You can't start with a heart, if they have"
                                 " not been played before!"), PACKAGE " - Hearts",
                               XMessageBox::ERROR);
            return false;
         }
      }

      // The queen of spades can't be played in the first round
      if (!cardsPlayed) {
         if ((card.color () == CardWidget::SPADES)
             && (card.number () == CardWidget::QUEEN)) {
            XMessageBox::Show (_("The queen of spades can't be played in the first"
                                 " round"), PACKAGE " - Hearts", XMessageBox::ERROR);
            return false;
         }

         if (((playColor == CardWidget::HEARTS) && !aPlayed[CardWidget::HEARTS])
              && (players[player].hand.at (0).color () != CardWidget::HEARTS)) {
            XMessageBox::Show (_("Hearts can't be played in the first round"),
                               PACKAGE " - Hearts", XMessageBox::ERROR);
            return false;
         }
      }

      Check3 (playColor < (sizeof (aPlayed) / sizeof (aPlayed[0])));
      aPlayed[playColor]++;
   }

   movePile (played, players[player].hand, card, card);
   return true;
}

/*--------------------------------------------------------------------------*/
//Purpose   : Exchanges the cards of the computer players
//
//            Get rid of cards according the following algorithm:
//              - If nr. of clubs or diamonds are < 3 -> Use them
//              - If nr. of spades < 5 get rid of high spades (especially the queen)
//              - Get rid of high hearts
//              - Get rid of high cards
/*--------------------------------------------------------------------------*/
void Hearts::exchangeCards () {
   TRACE9 ("Hearts::exchangeCards ()");

   for (unsigned int i (3); i; --i) {
      int posColors[4];
      ICardPile& pile (players[i].hand);
      getPositionOfColors (pile, posColors);

      unsigned int moved (0);
      unsigned int cCards (numberOfCards (posColors, CardWidget::CLUBS));
      // If nr. of clubs or diamonds are < 3 -> Use them
      if (cCards && (cCards < 3)) {
         TRACE3 ("Hearts::exchangeCards () - Getting rid of all clubs: 0 - "
                 << cCards - 1);
         movePile (players[(i + 1) & 0x3].hand, pile, 0, posColors[0]);
         moved = cCards;
      }
      cCards = numberOfCards (posColors, CardWidget::DIAMONDS);
      if (cCards && (cCards) < (3 - moved)) {
         TRACE3 ("Hearts::exchangeCards () - Getting rid of all diamonds: "
                 << posColors[1] - moved  - cCards + 1 << " - " 
                 << posColors[1] - moved);
         movePile (players[(i + 1) & 0x3].hand, pile,
                   posColors[1] - moved - cCards + 1, posColors[1] - moved);
         moved += cCards;
      }

      // If nr. of spades < 5 get rid of high spades (especially the queen)
      cCards = numberOfCards (posColors, CardWidget::SPADES);
      if (cCards && (cCards < 5)) {
         // Search for the queen of spades and get rid of cards equal or bigger
         unsigned int start (posColors[2] - moved - cCards + 1);
         while (start <= (posColors[2] - moved - 1)
                && (pile.at (start).number () < CardWidget::QUEEN)) {
            TRACE9 ("Hearts::exchangeCards () - Checking spades at " << start);
            Check3 (pile.at (start).color () == CardWidget::SPADES);
            ++start;
         }
         
         cCards = (((posColors[2] - moved - start) > 3)
                   ? 3 : posColors[2] - moved - start);
         TRACE3 ("Hearts::exchangeCards () - Getting rid of all high spades: "
                 << start << " - " << start + cCards - 1);
         movePile (players[(i + 1) & 0x3].hand, pile, start, start + cCards);
         moved += cCards + 1;
      }

      // Get rid of high cards
      int cardPos;
      for (unsigned int nr (CardWidget::ACE); moved < 3; --nr) {
         Check3 (nr > CardWidget::TWO);
         TRACE8 ("Hearts::exchangeCards () - Getting rid of high cards");
         if ((cardPos = pile.find (CardWidget::NUMBERS (nr))) != -1) {
            TRACE3 ("Hearts::exchangeCards () - Getting rid of all high card at "
                    << cardPos);
            movePile (players[(i + 1) & 0x3].hand, pile, cardPos, cardPos);
            moved++;
         }
      }
      players[(i + 1) & 0x3].hand.sortByColor ();
   }
}

/*--------------------------------------------------------------------------*/
//Purpose   : Retrieve the last position of each color in the pile
//Parameters: pile: Pile to inspect
//            result: Array of position of last cards of earch color
/*--------------------------------------------------------------------------*/
void Hearts::getPositionOfColors (ICardPile& pile, int result[4]) {
   memset (result, (char)-1, sizeof (int[4]));
   for (unsigned int i (0); i < (pile.numberOfCards () - 1); ++i)
      if (pile.at (i).color () != pile.at (i + 1).color ())
         result[pile.at (i).color ()] = i;
   result[pile.at (pile.numberOfCards () - 1).color ()] = pile.numberOfCards () - 1;

   TRACE9 ("Hearts::getPositionOfColors (ICardPile&, unsigned int) - Pos. of "
           "cards: " << result[0] << ", " << result[1]
           << ", " << result[2] << ", " << result[3]);
}

/*--------------------------------------------------------------------------*/
//Purpose   : Calculate the number of cards out of the positions
//Parameters: aPositions: Array of positions
//            color: Color whose number should be calculated
//Returns   : int: Number of cards for color
/*--------------------------------------------------------------------------*/
unsigned int Hearts::numberOfCards (int aPositions[4], CardWidget::COLORS color) {
   Check1 (color <= CardWidget::HEARTS);
   unsigned int nr (0);
   if (aPositions[color] != -1) {
      nr = aPositions[color] + 1;
      if (color && aPositions[color - 1] != -1)
         nr -= aPositions[color - 1] + 1;
   }

   TRACE9 ("Hearts::numberOfCards (int, CardWidget::COLORS) - Cards: " << nr);
   return nr;
}

/*--------------------------------------------------------------------------*/
//Purpose   : Calculate the number of cards for each color
//Parameters: pile: Pile to inspect
//            result: Array of number of cards for earch color
/*--------------------------------------------------------------------------*/
unsigned int Hearts::findPos2Play (unsigned int player) {
   Check1 (player < NUM_PLAYERS);
   TRACE8 ("Hearts::findPos2Play (unsigned int)");

   int aPos[4];
   getPositionOfColors (players[player].hand, aPos);

   ICardPile& pile (players[player].hand);
   if (played.numberOfCards ()) {
      CardWidget::COLORS color (played.at (0).color ());
      // Check if cards of the same color are available
      if (aPos[color] == -1) {
         // Player doesn't have color -> Play queen of spades or hearts
         // or a high card.
         unsigned int cardsPlayed (0);
         for (unsigned int i (0); i < NUM_PLAYERS; ++i)
            cardsPlayed += players[i].won.numberOfCards ();

         if (cardsPlayed) {
            TRACE5 ("Hearts::findPos2Play (unsigned int) - Searching for SQ");
            if (aPos[2] > 0)
               for (unsigned int pos ((aPos[1] >= 0)
                                      ? aPos[1] + 1
                                      : ((aPos[0] >= 0) ? aPos[0] + 1: 0));
                    pos <= aPos[2]; ++pos) {
                  TRACE9 ("Hearts::findPos2Play (unsigned int) - Searching for "
                          "SQ at position " << pos);
                  Check3 (pile.at (pos).color () == CardWidget::SPADES);
                  TRACE5 ("Hearts::findPos2Play (unsigned int) - Searching for SQ");
                  if (pile.at (pos).number () >= CardWidget::QUEEN)
                     return pos;
               }

            // No high spade found: Try to play a heart (after the first round)
            TRACE5 ("Hearts::findPos2Play (unsigned int) - Searching for hearts");
            if (pile.at (pile.numberOfCards () - 1).color () == CardWidget::HEARTS)
               return pile.numberOfCards () - 1;
         }

         // If everything else failes: Play a high card
         TRACE5 ("Hearts::findPos2Play (unsigned int) - Searching for high cards");
         int pos (0);
         for (int card (CardWidget::ACE); card >= CardWidget::TWO;
              --card) {
            pos = 0;
            while ((pos = pile.find (CardWidget::NUMBERS (card), pos))
                   != -1) {
               CardWidget::COLORS color (pile.at (pos).color ());
               TRACE2 ("Hearts::findPos2Play (unsigned int) - Checking card "
                       << pile.at (pos) << " at pos " << pos << " against "
                       << aPlayed[color] << " cards");
               if (((aPlayed[color] + numberOfCards (aPos, color))
                    < (cards.numberOfCards () / NUM_PLAYERS))
                   // ... but a heart or the queen of spades in round 1
                   && (cardsPlayed
                       || (color != CardWidget::HEARTS)
                       || ((color == CardWidget::SPADES)
                           && card != CardWidget::QUEEN)))
                  return pos;
               ++pos;
            }
         }
      }
      else {
         // Play high card of the color, if last player and pile contains no
         // counting card, except if that would mean to play the queen of spades.
         if ((played.numberOfCards () == (NUM_PLAYERS - 1))
             && !pointsOfPile (played)
             && ((color != CardWidget::SPADES)
                 || (players[player].hand.at (aPos[CardWidget::SPADES]).number ()
                     != CardWidget::QUEEN)))
            return aPos[color];
         else {
            // Play highest card lower than the previously played onces
            // First get highest played card (with the color of the first)
            int card (0);
            Check3 (played.numberOfCards ());
            CardWidget::NUMBERS highest (played.at (0).number ());
            while (++card < played.numberOfCards ())
               if ((played.at (card).number () > highest)
                   && (played.at (card).color () == color)) {
                  highest = played.at (card).number ();
                  TRACE9 ("Hearts::findPos2Play (unsigned int) - Try to be below "
                          << played.at (card));
               }

            // Then search for a lower card
            card = aPos[color]; Check3 (card >= 0);
            do {
               if (pile.at (card).number () < highest) {
                  TRACE5 ("Hearts::findPos2Play (unsigned int) - Playing card at "
                          << card  << ": " << pile.at (card));
                  return card;
               }
               --card;
            } while ((card >= 0) && (pile.at (card).color () == color));
            TRACE5 ("Hearts::findPos2Play (unsigned int) - Forced to play card at "
                    << card + 1 << ": " << pile.at (card + 1));
            return ((played.numberOfCards () == (NUM_PLAYERS - 1))
                    ? aPos[color] : card + 1);
         }
      }
   }
   else {
      // Player starts the round: If he has loads of spades: Play them
      if (((cards.numberOfCards () / NUM_PLAYERS) + 1 - aPlayed[CardWidget::SPADES])
          < numberOfCards (aPos, CardWidget::SPADES)) {
         unsigned int pos ((aPos[1] >= 0)
                           ? aPos[1] + 1
                           : ((aPos[0] >= 0) ? aPos[0] + 1: 0));
         TRACE5 ("Hearts::findPos2Play (unsigned int) - Starting with spade at "
                 << pos << " (" << pile.at (pos) << ')');
         return pos;
      }

      // Else: Search for a low card
      int pos (0);
      for (unsigned int card (CardWidget::TWO); card <= CardWidget::ACE;
           ++card) {
         pos = 0;
         while ((pos = pile.find (CardWidget::NUMBERS (card), pos)) != -1) {
            CardWidget::COLORS color (pile.at (pos).color ());
            // Play the lowest card, if there are still cards of that color
            // owned by other players and - if it is a heart - there are
            // already played hearts.
            if ((aPlayed[color] + numberOfCards (aPos, color))
                < (cards.numberOfCards () / NUM_PLAYERS)) {
               TRACE9 ("Hearts::findPos2Play (unsigned int) - Considering to "
                       "play " << pile.at (pos) << "; Played: " << aPlayed[color]);

                if ((color != CardWidget::HEARTS)
                    || aPlayed[CardWidget::HEARTS]) {
                   TRACE5 ("Hearts::findPos2Play (unsigned int) - Starting with "
                           << pile.at (pos));
                   return pos;
                }
            }
            ++pos;
         }
      }
      TRACE1 ("Hearts::findPos2Play (unsigned int) - All cards for player " << player);
   }
   return 0;
}

/*--------------------------------------------------------------------------*/
//Purpose   : Counts the points in the passed pile.
//
//            The queen of spades counts 13 points and every heart 1 point
//Parameters: pile: Pile to inspect
//Returns   : unsigned int: Number of points
/*--------------------------------------------------------------------------*/
unsigned int Hearts::pointsOfPile (ICardPile& pile) {
   unsigned int points (0);
   for (unsigned int i (0); i < pile.numberOfCards (); ++i) {
      CardWidget::COLORS color (pile.at (i).color ());
      if (color == CardWidget::HEARTS)
         ++points;
      else
         if ((color == CardWidget::SPADES)
             && pile.at (i).number () == CardWidget::QUEEN)
            points += 13;
   }
   TRACE7 ("Hearts::pointsOfPile (ICardPile&) - Number of points: " << points);
   return points;
}
