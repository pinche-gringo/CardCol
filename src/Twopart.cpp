//$Id$

//PROJECT     : Cardgames
//SUBSYSTEM   : Twopart
//REFERENCES  :
//TODO        :
//BUGS        :
//REVISION    : $Revision$
//AUTHOR      : Markus Schwab
//CREATED     : 20.7.2002
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
#include <gtk--/proxy.h>
#include <gtk--/statusbar.h>

#include <CardSet.h>

#include <Check.h>
#include <Trace_.h>

#include <XMessageBox.h>

#include <CardWidget.h>
#include "Twopart.h"


using SigC::slot;
using SigC::bind;


const unsigned int Twopart::COLS_PLAYER[NUM_PLAYERS] = { 7, 13, 7, 1 };
const unsigned int Twopart::ROWS_PLAYER[NUM_PLAYERS] = { 4,  7, 8, 7 };

char Twopart::sortOrder[4];


/*--------------------------------------------------------------------------*/
//Purpose   : Defaultconstructor; all widget are created
//Parameters: parent: Parent for cardgame
/*--------------------------------------------------------------------------*/
Twopart::Twopart (Gtk::Box& parent, Gtk::Statusbar& statusbar, CardSet& cardset)
   : Game (parent, statusbar, cardset, 11, 7)
     , played (ICardPile::COMPRESSED, ICardPile::SHOWFACE)
     , staple (ICardPile::VERY_COMPRESSED, ICardPile::SHOWBACK)
     , bfPlayers ((1 << NUM_PLAYERS) - 1), pTrump (NULL), offPos (0)
        , bfOldPlayers (bfPlayers) {
   staple.show ();
   attach (staple, 2, 3, 2, 3, 0, 0, 5, 5);

   unsigned int width (cards.getCard (0).getImageWidth ());
   unsigned int height (cards.getCard (0).getImageHeight ());

   // Show and attach card-piles
   for (int i (0); i < NUM_PLAYERS; ++i) {
      players[i].won.show ();
      attach (players[i].won, COLS_PLAYER[i] + 1,
              COLS_PLAYER[i] + 2,
              ROWS_PLAYER[i] + ((i == 2) ? 2 : -2),
              ROWS_PLAYER[i] + ((i == 2) ? 2 : -2) + 1,
              0, 0, 1);

      TRACE9 ("Twopart::Twopart () - Set at: "
              << COLS_PLAYER[i] + 1 << '/' << ROWS_PLAYER[i] + ((i == 2) ? 2 : -2));

      players[i].hand.show ();
      attach (players[i].hand, COLS_PLAYER[i],
              COLS_PLAYER[i] + 3, ROWS_PLAYER[i],
              ROWS_PLAYER[i] + 1, 0, 0, 1);
      TRACE9 ("Twopart::Twopart () - 2nd set at: "
              << COLS_PLAYER[i] << '/' << ROWS_PLAYER[i]);

      players[i].won.setShowOption (ICardPile::SHOWBACK);
      players[i].hand.setShowOption (i ? ICardPile::SHOWBACK : ICardPile::SHOWFACE);

      players[i].won.set_usize (width + 20, height + 5);
      players[i].hand.set_usize (width * 3, height + 5);
   }

   played.show ();
   attach (played, 3, 11, 5, 8, 0, 0, 0, 5);

   played.set_usize (width + 150, height);
   staple.set_usize (width, height);
}

/*--------------------------------------------------------------------------*/
//Purpose   : Destructor
/*--------------------------------------------------------------------------*/
Twopart::~Twopart () {
   TRACE9 ("Twopart::~Twopart ()");
}

/*--------------------------------------------------------------------------*/
//Purpose   : Starts the game
/*--------------------------------------------------------------------------*/
void Twopart::start () {
   Game::start ();
   randomizeCardsToPile (staple);
   dealCards ();

   for (unsigned int i (1); i < NUM_PLAYERS; ++i) {
      players[i].hand.setStyle (ICardPile::COMPRESSED);
      players[i].won.setStyle (ICardPile::VERY_COMPRESSED);
   }
   players[0].hand.setStyle (ICardPile::NORMAL);
   players[0].won.setStyle (ICardPile::QUITE_COMPRESSED);

   pos1Play = pos2Play = (unsigned int)-1;
}

/*--------------------------------------------------------------------------*/
//Purpose   : Enables the cards of the passed player
//Parameters: player: Player to enable
//Remarks   : Depending of the status of the game (PLAYING2) also the top
//            card of the played pile is enabled
/*--------------------------------------------------------------------------*/
void Twopart::enablePlayer (unsigned int player) {
   Check3 (activeCards.empty ());
   Check3 (gameStatus () >= PLAYING);

   TRACE2 ("Twopart::enablePlayer (unsigned int) - player "
           << player << " has " << players[player].hand.numberOfCards ()
           << " cards");

   for (int i (players[player].hand.numberOfCards ()); i;)
      activeCards.push_back
         (players[player].hand.at (--i).clicked.connect_after
           (bind (slot (this, (&Twopart::cardSelected)), player, i)));

   if ((gameStatus () == PLAYING2)
       && (played.numberOfCards ()))
      activeCards.push_back
         (played.getTopCard ().clicked.connect_after
          (bind (slot (this, (&Twopart::playedSelected)), player)));
}

/*--------------------------------------------------------------------------*/
//Purpose   : Moving played cards (of last person) to the passed player
//Parameters: player: ID of player who should get the cards
//Returns   : unsigned int: Next player
//Requieres : Only for part 2 of the game
/*--------------------------------------------------------------------------*/
unsigned int Twopart::pickUpPlayedPile (unsigned int player) {
   TRACE3 ("Twopart::pickUpPlayedPile (unsigned int) - Player " << player
           << " picks up played pile");
   Check3 (gameStatus () == PLAYING2);
   Check3 (bfPlayers);

   // Move played cards to player
   Check3 (offPos > 0); Check3 (offPos < NUM_PLAYERS);
   movePlayedCardsToPlayer (player, startPos[--offPos]);
   removePlayer (player);

   // Calculate players to re-enable: They are the number of players still
   // in game (with cards) minus the players still in round; but maximal 2
   unsigned int num (0);
   for (unsigned int i (0); i < NUM_PLAYERS; ++i)
      if (players[i].hand.numberOfCards ())
         ++num;
   num -= playersInBitfield (bfPlayers);
   if (num > 2)
      num = 2;
   TRACE8 ("Twopart::pickUpPlayedPile (unsigned int) - Adding " << num
           << " players to left " << playersInBitfield (bfPlayers));

   // Re-enable next two players (having cards); continue with first of them
   unsigned int next;
   for (unsigned int i (0); num; ++i) {
      Check3 (i < NUM_PLAYERS);
      if ((!(bfPlayers & (1 << (next = (player + i + 1) & 0x3))))
          && players[next].hand.numberOfCards ()) {
         TRACE5 ("Twopart::pickUpPlayedPile (unsigned int) - Re-adding player "
                 << next);
         addPlayer (next);
         --num;
      }
   }
   return findNextPlayer (player);
}

/*--------------------------------------------------------------------------*/
//Purpose   : Callback after clicking the top card of the played pile
//Parameters: player: ID of player
//Requieres : Only for part 2 of the game
/*--------------------------------------------------------------------------*/
void Twopart::playedSelected (unsigned int player) {
   TRACE3 ("Twopart::playedSelected (unsigned int) - Player " << player
           << " picks up played pile");
   Check3 (gameStatus () == PLAYING2);
   Check3 (bfPlayers);

   setNextPlayer (pickUpPlayedPile (player));
   makeNextMoves ();
   disableLastPlayer ();
}

/*--------------------------------------------------------------------------*/
//Purpose   : Callback after clicking on a card in hand
//Parameters: player: ID of player
//            start: Offset of first card to play
//            end: Offset of last card to play
//Returns   : bool: Status of moving; true: Card could be moved; false else
/*--------------------------------------------------------------------------*/
bool Twopart::moveSelectedCardToPlayed (unsigned int player,
                                        unsigned int start, unsigned int end) {
   TRACE5 ("Twopart::moveSelectedCardToPlayed (unsigned int, unsigned int) - Player: "
           << player << " at position " << start << " to " << end);
   Check3 (player < NUM_PLAYERS);
   Check3 (end < players[player].hand.numberOfCards ());
   Check3 (start <= end);
   Check3 (gameStatus () >= PLAYING);

   if (gameStatus () == PLAYING) {
      Check3 (start == end);
      TRACE9 ("Twopart::moveSelectedCardToPlayed (unsigned int, unsinged int)"
              " - Player " << player << "; Card at " << end << " = "
              << players[player].hand.at (end));
      movePile (played, players[player].hand, start, end);

      if (staple.numberOfCards ()) {
         CardWidget& card (staple.removeShownTopCard ());
         players[player].hand.insertSorted (card);

         if (!staple.numberOfCards ()) {
            Check3 (!pTrump);
            pTrump = new CardWidget (card); Check3 (pTrump);
            staple.hide ();
         }
      }
   }
   else {
      CardWidget& card (players[player].hand.at (start));
      unsigned int nr (card.number ());
      CardWidget::COLORS color (card.color ());
      
      // Perform validity-check in part 2: Card must have the same color and be
      // bigger than the last played card or be a (bigger) trump
      Check3 (pTrump);
      if (played.numberOfCards ()) {
         CardWidget& top (played.getTopCard ());

         if ((color == pTrump->color ())
             ? ((top.color () == pTrump->color ())
                && (top.number () >= nr))
             : ((top.color () != color)
                || (top.number () >= nr))) {
            XMessageBox::Show (_("Played card(s) must have the same color and must be "
                                 "bigger (or be a trump)!"), PACKAGE " - Twopart",
                               XMessageBox::ERROR);
            return false;
         }
      }

      if (offPos < (NUM_PLAYERS - 1))
         startPos[offPos++] = played.numberOfCards ();
#if TRACELEVEL > 8
      for (unsigned int i (0); i < (NUM_PLAYERS - 1); ++i)
         TRACE ("Twopart::moveSelectedCardToPlayed (unsigned int, unsinged int) - "
                << i << ". Position: " << startPos[i]);
#endif
      movePile (played, players[player].hand, start, end);
   }
   return true;
}

/*--------------------------------------------------------------------------*/
//Purpose   : Callback after clicking on a card in hand
//Parameters: player: ID of player
//            pos: Offset of card in hand
/*--------------------------------------------------------------------------*/
void Twopart::cardSelected (unsigned int player, unsigned int pos) {
   TRACE5 ("Twopart::cardSelected (unsigned int, unsigned int) - Player: "
           << player << " at position " << pos);
   Check3 (player < NUM_PLAYERS);
   Check3 (pos < players[player].hand.numberOfCards ());
   Check3 (gameStatus () >= PLAYING);

   setNextPlayer (executeMove (player, 
                               ((gameStatus () == PLAYING2)
                                ? findStartOfSerie (player, pos) : pos),
                               pos));
   makeNextMoves ();
}

/*--------------------------------------------------------------------------*/
//Purpose   : Executes a move out of a hand
//Parameters: player: ID of player
//            start: Offset of first card to play
//            end: Offset of last card to play
//Returns   : int: Next player or -1 at end
/*--------------------------------------------------------------------------*/
int Twopart::executeMove (unsigned int player, unsigned int start, unsigned int end) {
   TRACE5 ("Twopart::executeMove (unsigned int, unsigned int) - Player: "
           << player << " at position " << start << " to " << end);
   Check3 (player < NUM_PLAYERS);
   Check3 (start <= end);
   Check3 (end < players[player].hand.numberOfCards ());

   if (!moveSelectedCardToPlayed (player, start, end))
      return player;

   // Check if every player still in game or has already played; end round if so
   // or calculate next player if not
   TRACE7 ("Twopart::executeMove (unsigned int, unsigned int) - Players: "
           << hex << bfPlayers << dec);
   removePlayer (player);
   int newPlayer (player);
   if (bfPlayers)
      newPlayer = findNextPlayer (player);
   else {
      // Show trump if not already visible
      if (pTrump && !pTrump->is_visible ()) {
         pTrump->showFace ();
         pTrump->show ();
         attach (*pTrump, 2, 3, 2, 3, 0, 0, 5, 5);
      }

      newPlayer = endRound (player);
   }

   // Check if the actual part is terminated
   if ((gameStatus () == PLAYING)
       ? (newPlayer < 0)
       : (newPlayer == findNextPlayerWithCards (newPlayer))) {
      player = (gameStatus () == PLAYING) ? ~newPlayer : newPlayer;

      std::string str ((gameStatus () == PLAYING)
                       ? _("First part ended; Part 2 starts player %1")
                       : _("Player %1 lost"));
      str.replace (str.find ("%1"), 2, (char)(player + '0'));
      status.pop (1);
      status.push (1, str);

      if (gameStatus () == PLAYING)
         startPartTwo (player);
      else
         setGameStatus (STOPPED);
      return -1;
   }
   else
      displayTurn (player = newPlayer);

   return player;
}

/*--------------------------------------------------------------------------*/
//Purpose   : Makes the move for the next player.
//Parameters: player: Actual player
//Returns   : int: Next player or -1 if end of game
/*--------------------------------------------------------------------------*/
int Twopart::makeMove (unsigned int player) {
   TRACE5 ("Twopart::makeMove () - Turn of player " << player);
   Check3 (gameStatus () >= PLAYING);

   if (pos2Play == (unsigned int)-1) {
      if (findPos2Play (player, pos1Play, pos2Play) != -1) {
         // Flip card(s) to play
         flipCards2Play (players[player].hand, pos1Play, pos2Play);
         return player;
      }
      else {
         unsigned int oldPlayer (player);
         player = pickUpPlayedPile (player);

         std::string stat ( _("Player %1 can't continue -> Picking up last cards; "));
         stat.replace (stat.find ("%1"), 2, (char)(oldPlayer + '0'));
         displayTurn (player, stat);
      }
   }
   else {
      player = executeMove (player, pos1Play, pos2Play);
      pos2Play = (unsigned int)-1;
   }

   return player;
}

/*--------------------------------------------------------------------------*/
//Purpose   : Searches for the card(s) to play by analyzing the previously
//            played cards
//            to him if so. Else enable the players which can continue
//Parameters: player: ID of player to analyze
//            start: Offset of first card to play
//            end: Offset of last card to play
//Returns   : int: Position to play; or -1 if player can't continue
/*--------------------------------------------------------------------------*/
int Twopart::findPos2Play (unsigned int player, unsigned int& start,
                           unsigned int& end) const {
   Check3 (player); Check3 (player < NUM_PLAYERS);
   Check3 (gameStatus () >= PLAYING);
   TRACE5 ("Twopart::findPos2Play (unsigned int) - Player " << player);

   if (gameStatus () == PLAYING) {
      unsigned int points (0);
      unsigned int cHigh (0);

      // Analyze played staple
      for (unsigned int i (0); i < played.numberOfCards (); ++i) {
         CardWidget::NUMBERS nr (played.at (i).number ());
         points += nr;
         if (nr >= CardWidget::TEN)
            ++cHigh;
      }

      TRACE2 ("Twopart::findPos2Play (unsigned int) - Points: " << points
              << "; Avg: " << (played.numberOfCards ()
                               ? (points / played.numberOfCards ()) : 0)
              << "; High: " << cHigh);
      if (played.numberOfCards ())
         points /= played.numberOfCards ();

      int maxNr (-1);
      int maxEqualNr (-1);
      int posMax (-1);
      int posMaxEqual (-1);
      int trumps (0);
      analyzeLastPlayed (*startPos, played.numberOfCards () - *startPos,
                         maxNr, posMax, maxEqualNr, posMaxEqual, trumps);

      start = (unsigned int)-1;
      // Try to get the cards if there are loads of high cards (a third or more)
      // or if the average card played is at least a 8 or there are trumps inside
      if (((played.numberOfCards () / 3) < cHigh)
          || (points >= CardWidget::SEVEN)
          || trumps) {
         end = -1U;
         // Search for card whose number you own
         for (start = 0;
              start < players[player].hand.numberOfCards (); ++start)
            if ((played.exists (players[player].hand.at (start).number (), *startPos))
                && ((posMaxEqual == -1) 
                    || (played.at (start).number () >= maxEqualNr))) {
               TRACE2 ("Twopart::findPos2Play (unsigned int) - Having equal card at "
                       << start);

               // Use card if it's a trump
               if (pTrump
                   && (pTrump->color () == players[player].hand.at (start).color ())) {
                  return end = start;
               }
               if (end == -1U)
                  end = start;
            }
         // Reset start to first found card (or 0)
         Check3 (start == players[player].hand.numberOfCards ());
         start = (end != -1U) ? end : 0;
         TRACE6 ("Twopart::findPos2Play (unsigned int) - First try (I): " << start);

         // Last player plays high card if higher (even if he would have an
         // equal card in case the pile is really good) or ...
         if (((start != end)
              || (cHigh > 1) || (points >= CardWidget::TEN))
             && (posMaxEqual == -1)
             && ((!(bfPlayers & ~(1 << player)))
                 && trumps
                 && ((end = (findBigger
                             (players[player].hand,
                              static_cast<CardWidget::NUMBERS> (maxNr))))
                     != -1U))
             // ... the staple is being fighted for and player has high cards
             || (*startPos
                 && ((end = players[player].hand.numberOfCards () - 1),
                     ((players[player].hand.at (end).number ()
                       == CardWidget::ACE))
                     || ((playersInBitfield (bfOldPlayers) < *startPos)
                         && (players[player].hand.at (end).number () >= maxNr)
                         && (posMaxEqual == -1))))) {
            TRACE2 ("Twopart::findPos2Play (unsigned int) - Playing highest card at "
                    << start);
            start = end;
         }
         return end = start;
      }

      // Try to get the cards if you don't have any close to the end of part 1
      // and you are the last or the pile really sucks and just one is left
      // (of course only if there are no doubles).
      if (!players[player].won.numberOfCards ()
          && (staple.numberOfCards () < 13)
          && (posMaxEqual == -1)
          && (((!(bfPlayers & ~(1 << player)))
               || (!(bfPlayers & ~((1 << player)
                                   | (1 << findNextPlayer ((player + 1) & 0x3))))
                   && (points < CardWidget::SIX)))
              && (maxNr < players[player].hand.at
                  (players[player].hand.numberOfCards () - 1).number ())))
         return end = start = 2;

      // We don't want the pile; so try not to get it. To do so, play
      // the second (biggest) card, if the first card exists in the pile and
      // the second not (though the second card must also small).
      // An exception is also for the last player
      TRACE5 ("Twopart::findPos2Play (unsigned int) - Avoiding pile");
      start = ((players[player].hand.numberOfCards () > 1)
               && played.exists (players[player].hand.at (0).number (), *startPos)
               && !played.exists (players[player].hand.at (1).number (), *startPos)
               && (((!(bfPlayers & ~(1 << player)))
                    && (players[player].hand.at (1).number () < maxNr))
                   || (players[player].hand.at (1).number () <= CardWidget::SEVEN)));
      TRACE5 ("Twopart::findPos2Play (unsigned int) - Avoiding returns " << start);

      // Final check: If you have to pick up the pile and you're the last,
      // use at least a high card (unless there are doubles)
      if (!start
          && (posMaxEqual == -1)
          && !(bfPlayers & ~(1 << player))
          && (players[player].hand.numberOfCards () > 1)
          && (players[player].hand.at (start).number () > maxNr))
         start = 1;

      TRACE5 ("Twopart::findPos2Play (unsigned int) - Playing card at " << start);
      return end = start;
   }
   else {
      // Find first fitting card
      start = (played.numberOfCards ()
               ? players[player].hand.find1EqualOrBigger (played.getTopCard (),
                                                          compByColorAccTrumps)
               : findSmallestCard (player));
      TRACE5 ("Twopart::findPos2Play (unsigned int) - First try (II): " << start);

      Check3 (pTrump);
      if ((start == (unsigned int)-1)
          || (played.numberOfCards ()
              && (played.getTopCard ().color ()
                  != players[player].hand.at (start).color ()))) {
         TRACE5 ("Twopart::findPos2Play (unsigned int) - No card found; trying trump");
         Check3 (pTrump);
         if (played.getTopCard ().color () != pTrump->color ()) {
            for (start = players[player].hand.numberOfCards (); start; --start)
               if (players[player].hand.at (start - 1).color () != pTrump->color ())
                  break;

            if (!start)
               end = findEndOfSerie (player, 0);
            else {
               end = players[player].hand.numberOfCards () - 1;
               // Take up pile if no trump was found or if only a "small amount"
               // of trumps are left (like less than 4 or less than the half)
               // and you are not the last player
               return (((start == players[player].hand.numberOfCards ())
                        || ((bfPlayers & ~(1 << player))
                            && (((end - start) < 4)
                                || (start < (end - start)))))
                       ? (end = (unsigned int)-1) : (end = start));
            }
         }
         else
            return end = (unsigned int)-1;
      }
      else
         // Card was found; now search for last card to play (only if not trump
         // or only trump left)
         end = start;
         if (!start
             || (players[player].hand.at (start).color () != pTrump->color ()))
            end = findEndOfSerie (player, start);
      TRACE5 ("Twopart::findPos2Play (unsigned int) - Playing card at pos " << start);
      return start;
   }
}

/*--------------------------------------------------------------------------*/
//Purpose   : Searches for the smallest card in the hand of the passed player
//Parameters: player: Player to inspect
//Returns   : int: Position of smallest card
/*--------------------------------------------------------------------------*/
unsigned int Twopart::findSmallestCard (unsigned int player) const {
   TRACE5 ("Twopart::findSmallestCard (unsigned int) - Inspecting player " << player);
   Check3 (player < NUM_PLAYERS);
   Check3 (pTrump);

   unsigned int nrMin (CardWidget::UNREACHABLE);
   unsigned int cSerie (0);
   unsigned int pos (0);
   for (unsigned int i (0); i < players[player].hand.numberOfCards (); ++i) {
      CardWidget& card (players[player].hand.at (i));

      // Stop searching if a trump was found
      if ((card.color () == pTrump->color ()) && i)
         break;
          
      if (nrMin >= card.number ()) {
         TRACE9 ("Twopart::findSmallestCard (unsigned int) - New smallest card at "
                 << i << "; Cards: " << (findEndOfSerie (player, i) - i));

         unsigned int endPos (findEndOfSerie (player, i));
         if ((card.number () == nrMin) && ((endPos - i) <= cSerie))
               continue;

         cSerie = (endPos - i);
         nrMin = card.number ();
         pos = i;
         i = endPos - 1;
      }
   }

   TRACE5 ("Twopart::findSmallestCard (unsigned int) - Smallest card at " << pos);
   return pos;
}

/*--------------------------------------------------------------------------*/
//Purpose   : Searches for the last position of the card which are in a serie
//            (same color; number increasing by 1)
//Parameters: player: Player to inspect
//            start: Position to start
//Returns   : unsinged int: Position of last card in serie
/*--------------------------------------------------------------------------*/
unsigned int Twopart::findEndOfSerie (unsigned int player, unsigned int start) const {
   Check3 (start < players[player].hand.numberOfCards ());

   CardWidget* card (&players[player].hand.at (start));
   CardWidget::NUMBERS nr (card->number ());
   CardWidget::COLORS color (card->color ());

   while ((++start < players[player].hand.numberOfCards ())
          && ((card = &players[player].hand.at (start)),
              (card->number ()) == (nr + 1))
          && (card->color () == color)) {
      TRACE9 ("Twopart::findEndOfSerie (unsigned int, unsigned int) - Next valid card "
              << *card << " at " << start);
      nr = card->number ();
   }

   return start - 1;
}

/*--------------------------------------------------------------------------*/
//Purpose   : Searches for the first position of the card which are in a serie
//            (same color; number decreasing by 1)
//Parameters: player: Player to inspect
//            start: Position to start
//Returns   : unsinged int: Position of first card in serie
/*--------------------------------------------------------------------------*/
unsigned int Twopart::findStartOfSerie (unsigned int player, unsigned int start) const {
   Check3 (start < players[player].hand.numberOfCards ());

   CardWidget* card (&players[player].hand.at (start));
   CardWidget::NUMBERS nr (card->number ());
   CardWidget::COLORS color (card->color ());

   while ((--start < players[player].hand.numberOfCards ())
          && ((card = &players[player].hand.at (start)),
              (card->number ()) == (nr - 1))
          && (card->color () == color)) {
      TRACE9 ("Twopart::findStartOfSerie (unsigned int, unsigned int) - Next valid card "
              << *card << " at " << start);
      nr = card->number ();
   }

   return start + 1;
}

/*--------------------------------------------------------------------------*/
//Purpose   : Checks if there is a winner for the round and moves played cards
//            to him if so. Else enable the players which can continue
//Parameters: player: Actual player (ending the round)
//Returns   : int: Next player; or -1 if there is no next player
/*--------------------------------------------------------------------------*/
int Twopart:: endRound (unsigned int player) {
   TRACE8 ("Twopart::endRound (unsigned int)");
   Check3 (!bfPlayers);

   unsigned int nextPlayer (NUM_PLAYERS);

   if (gameStatus () == PLAYING2) {
      played.clear ();
      bfPlayers = (1 << NUM_PLAYERS) - 1;
      removePlayersWithoutCards ();
      offPos = 0;
      nextPlayer = (!players[player].hand.numberOfCards ())
         ? findNextPlayer (player) : player;
   }
   else {
      bfPlayers = bfOldPlayers;
      unsigned int cPlayers (playersInBitfield (bfPlayers));

      TRACE8 ("Twopart::endRound (unsigned int) - Round has " << cPlayers
              << " players; Start = " << *startPos << " of " << played.numberOfCards ()
              << " cards");
      Check3 ((*startPos + cPlayers) <= played.numberOfCards ());

      int maxNr (-1);
      int maxEqualNr (-1);
      int posMax (-1);
      int posMaxEqual (-1);
      int trumps (-1);
      analyzeLastPlayed (*startPos, cPlayers, maxNr, posMax, maxEqualNr,
                         posMaxEqual, trumps);

      TRACE4 ("Twopart::endRound (unsigned int) - Player starting round: " << startPlayer
              << "; players: " << cPlayers);
      Check3 ((*startPos + cPlayers) == played.numberOfCards ());

      // Equal cards found
      if (posMaxEqual >= 0) {
         unsigned int bfPlayersOut (0);
         cPlayers = 0;

         // Add players having equal cards and having still cards left
         nextPlayer = startPlayer;
         for (unsigned int i (*startPos); i < played.numberOfCards (); ++i) {
            if ((played.at (i).number () == maxEqualNr)
                && players[pos2Player (i - *startPos)].hand.numberOfCards ()) {
               if (!cPlayers)
                  // Start player is the first who played the highest cards
                  nextPlayer = pos2Player (posMaxEqual - *startPos);
               
               TRACE5 ("Twopart::endRound (unsigned int) - Found equal cards; Player "
                       << pos2Player (i - *startPos)
                       << (cPlayers ? " still in round" : " is winner"));
               ++cPlayers;
            }
            else
               bfPlayersOut |= (1 << pos2Player (i - *startPos));
         } // endfor check for equal cards
         bfPlayers &= ~bfPlayersOut;
         TRACE5 ("Twopart::endRound (unsigned int) - Found equal cards; " << cPlayers
                 << " player(s) still in round (" << hex << bfPlayers << dec << ')');

         // Find player to continue
         if (!players[nextPlayer].hand.numberOfCards ())
            nextPlayer = findNextPlayer (nextPlayer);
         TRACE6 ("Twopart::endRound (unsigned int) - Try to continue with player " << nextPlayer);
         if (cPlayers < 2) {                   // Less than two found: 
            bfPlayers = (1 << NUM_PLAYERS) - 1;
            cPlayers = removePlayersWithoutCards ();

            if (nextPlayer == -1) {
               nextPlayer = bfPlayers ? findNextPlayer (nextPlayer) : ~startPlayer;
               movePlayedCardsToPlayer (startPlayer);
            }
            else
               movePlayedCardsToPlayer (nextPlayer);
         }
#if CHECK > 0
         else
            Check (cPlayers > 1);
#endif
      }
      // All played cards are differnt: Winner is the one with highest card
      else {
         startPlayer = nextPlayer = pos2Player (posMax - *startPos);
         movePlayedCardsToPlayer (nextPlayer);
         TRACE5 ("Twopart::endRound (unsigned int) - Found winner: " << nextPlayer);

         bfPlayers = (1 << NUM_PLAYERS) - 1;   // Set all players (having cards)
         removePlayersWithoutCards ();
         bfOldPlayers = bfPlayers;

         if (!players[nextPlayer].hand.numberOfCards ())
            nextPlayer = findNextPlayer (nextPlayer);
         if (nextPlayer == -1)
            nextPlayer = ~startPlayer;
      }
      *startPos = played.numberOfCards ();
   }

   bfOldPlayers = bfPlayers;
   TRACE8 ("Twopart::endRound (unsigned int) - Continuing with player " << nextPlayer);
   return startPlayer = nextPlayer;
}

/*--------------------------------------------------------------------------*/
//Purpose   : Analyzes the played staple and retrieves the highest card(s)
//Parameters: startPos: Position from where to start analyzing
//            max: Highest single card
//            maxPos: Position of highest single card
//            maxEqua: Highest pair
//            maxEquaPos: Position of highest equal card
//            trumps: Number of trumps
//Remarks   : Values are not resetted!
/*--------------------------------------------------------------------------*/
void Twopart::analyzeLastPlayed (unsigned int startPos, unsigned int cards,
                                 int& max, int& maxPos, int& maxEqual,
                                 int& maxEqualPos, int& trumps) const {
   TRACE3 ("TwoPart::analyzeLastPlayed (...) - Analyzing cards [" << startPos
           << " to " << (cards + startPos) << ") of " << played.numberOfCards ());
   cards += startPos; Check3 (cards <= played.numberOfCards ());

   // Check if card is bigger then all previous
   for (; startPos < cards; ++startPos) {
      if ((int)(played.at (startPos).number ()) > max) {
         TRACE3 ("TwoPart::analyzeLastPlayed (...) - New highest card "
                 << played.at (startPos) << " at position " << startPos);
         max = (int)played.at (startPos).number ();
         maxPos = startPos;
      }
      Check3 (maxPos < played.numberOfCards ());
      Check3 (max == played.at (maxPos).number ());

      // Add trumps
      if (pTrump
          && (pTrump->color () == played.at (startPos).color ()))
         ++trumps;

      // Check if card has equal cards
      for (unsigned int j (startPos + 1); j < cards; ++j)
         if (played.at (startPos).number () == played.at (j).number ())
            if ((int)(played.at (startPos).number ()) > maxEqual) {
               TRACE3 ("TwoPart::analyzeLastPlayed (...) - Found equal "
                       << played.at (startPos).numberStr ()
                       << " at positions " << startPos << " and " << j);
               maxEqual = (int)played.at (startPos).number ();
               maxEqualPos = startPos;
               break;
            } // endif equal card found
   } // end-for all cards

   TRACE3 ("TwoPart::analyzeLastPlayed (...) - Trumps: " << trumps);
}

/*--------------------------------------------------------------------------*/
//Purpose   : Finds the next player having cards
//Parameters: player: Number of player to start with
//Returns   : int: Number of next player (or -1)
/*--------------------------------------------------------------------------*/
int Twopart::findNextPlayerWithCards (unsigned int player) const {
   unsigned int i (player);
   do {
      i = (i + 1) & 0x3;
      if (players[i].hand.numberOfCards ())
         return i;
   } while (i != player);

   return -1;
}

/*--------------------------------------------------------------------------*/
//Purpose   : Finds the next player which can continue according to the
//            bfPlayers bitfield
//Parameters: player: Number of player to start with
//Returns   : int: Number of next player (or -1)
/*--------------------------------------------------------------------------*/
int Twopart::findNextPlayer (unsigned int player) const {
   if (!bfPlayers)                                // No players left: Return -1
      return -1;

   // Find first player (starting with the passed one) being still in game
   do {
      player = (player + 1) & 0x3;
   } while (!(bfPlayers & (1 << player)));

   return player;
}

/*--------------------------------------------------------------------------*/
//Purpose   : Removes all players having no cards left from the player-bitfield
//Returns   : Number of players left
/*--------------------------------------------------------------------------*/
unsigned int Twopart::removePlayersWithoutCards () {
   unsigned int cPlayers (0);
   for (unsigned int i (0); i < NUM_PLAYERS; ++i)
      if (players[i].hand.numberOfCards ())
         ++cPlayers;
      else
         removePlayer (i);

   return cPlayers;
}

/*--------------------------------------------------------------------------*/
//Purpose   : Converts a position in the played staple into the number of the
//            player
//Parameters: pos: Position to convert
//            start: First player of round
//Returns   : unsigned int: Number of player
/*--------------------------------------------------------------------------*/
unsigned int Twopart::pos2Player (unsigned int pos) const {
   TRACE9 ("Twopart::pos2Player (unsigned int) - Pos to convert: " << pos
           << "; starting with player " << startPlayer);

   unsigned int start (startPlayer);
   while (pos) {
      ++start &= 0x3;
      if (bfPlayers & (1 << start))
         --pos;
   }
   
   TRACE9 ("Twopart::pos2Player (unsigned int) - Calculated player: " << start);
   return start;
}

/*--------------------------------------------------------------------------*/
//Purpose   : Method to move the cards of the actual round to the winner
//Parameters: receiver: Nr. of player getting all played cards
//            start: Startposition of cards to move
/*--------------------------------------------------------------------------*/
void Twopart::movePlayedCardsToPlayer (unsigned int receiver, unsigned int start) {
   TRACE8 ("Twopart::movePlayedCardsToPlayer () - " << played.numberOfCards ()
           << " cards for player " << receiver << " til position " << start);
   Check3 (receiver < NUM_PLAYERS);
   Check3 (start < played.numberOfCards ());

   movePile ((gameStatus () == PLAYING)
             ? players[receiver].won : players[receiver].hand,
             played, start);

   if (gameStatus () == PLAYING2)
      players[receiver].hand.sort (compByColorAccTrumps);
}

/*--------------------------------------------------------------------------*/
//Purpose   : Remove cards from everything which can hold them
/*--------------------------------------------------------------------------*/
void Twopart::clean () {
   staple.clear ();                                             // Clear staple
   for (int i (0); i < NUM_PLAYERS; ++i) {            // Clear cards of players
      players[i].hand.clear ();
      players[i].won.clear ();
   }
   played.clear ();

   disableLastPlayer ();
   staple.show ();
   if (pTrump) {
      delete pTrump;
      pTrump = NULL;
   }
}

/*--------------------------------------------------------------------------*/
//Purpose   : Deals the cards
/*--------------------------------------------------------------------------*/
void Twopart::dealCards () {
   TRACE9 ("Twopart::dealCards ()");

   // Show cards on table: For all players put 3 cards in hand
   for (unsigned int i (0); i < NUM_PLAYERS; ++i)
      for (unsigned int j (0); j < 3; ++j)
         players[i].hand.insertSorted (staple.removeTopCard ());

   for (unsigned int i (0); i < (NUM_PLAYERS - 1); ++i)
      startPos[i] = 0;
   offPos = 0;

   bfPlayers = bfOldPlayers = (1 << NUM_PLAYERS) - 1;
   enablePlayer (startPlayer = 0);

   displayTurn (0);
}

/*--------------------------------------------------------------------------*/
//Purpose   : Starts part two of the game
//Parameters: player: Player starting part II
//Returns   : int: Value indicating if timer should continue
/*--------------------------------------------------------------------------*/
int Twopart::startPartTwoTimerFnc (unsigned int player) {
   TRACE8 ("Twopart::startPartTwoTimerFnc ()");
   TRACE9 ("Twopart::startPartTwoTimerFnc (unsigned int) - Continuing with " << player);
   Check3 (!bfPlayers);
   setGameStatus (PLAYING2);

   // Prepare array for sorting according to trumps
   Check3 (pTrump);
   for (unsigned int i (0); i < 4; ++i)
      sortOrder[i] = (i - pTrump->color () + 3) & 0x3;
   Check3 (sortOrder[pTrump->color ()] == 3);

   startPlayer = (unsigned int)-1;

   unsigned int nrPlayers (0);
   // Check if there are players without cards
   for (unsigned int i (0); i < NUM_PLAYERS; ++i)
      if (!players[i].won.numberOfCards ()) {
         TRACE5 ("Twopart::startPartTwoTimerFnc (unsigned int) - Player "
                 << i << " has no cards");
         bfPlayers |= 1 << i;
         ++nrPlayers;
      }

   // Now move the cards from the played pile to the hand; if there are
   // players without cards give them the cards up to 5
   unsigned int victim (player);
   for (unsigned int i (0); i < NUM_PLAYERS; ++i) {
      for (unsigned int j (players[i].won.numberOfCards ()); j; --j) {
         CardWidget& card (players[i].won.removeTopCard ());
         TRACE9 ("Twopart::startPartTwoTimerFnc (unsigned int) - Moving cards "
                 << card << " for player " << i);
         if (bfPlayers && (card.number () <= CardWidget::FIVE)) {
            players[pos2Player (++victim)].hand.append (card);
            victim %= nrPlayers;
         }
         else
            players[i].hand.append (card);
      }
   }

   // Finally sort and show the cards
   for (unsigned int i (0); i < NUM_PLAYERS; ++i) {
      players[i].hand.sort (compByColorAccTrumps);
      players[i].hand.setStyle (i ? ICardPile::VERY_COMPRESSED : ICardPile::COMPRESSED);
   }
   
   bfPlayers = (1 << NUM_PLAYERS) - 1;
   pos2Play = (unsigned int)-1;

   setNextPlayer (player);
   makeNextMoves ();
   return 0;
}

/*--------------------------------------------------------------------------*/
//Purpose   : Compares the cards in the pile with regard of the color and
//            with special consideration of trumps
//Parameters: a: Card to compare
//            b: Card to compare
//Returns   : bool: True, if a < b
/*--------------------------------------------------------------------------*/
bool Twopart::compByColorAccTrumps (const CardWidget* a, const CardWidget* b) {
   Check3 (a); Check3 (b);

   TRACE9 ("Twopart::compByColorAccTrumps (const CardWidget*, const CardWidget*) - "
           << *a << " < " << *b << " = "
           << ((a->color () == b->color ()) ? a->number () < b->number ()
               : (sortOrder[a->color ()] <sortOrder[b->color ()])));
   return ((a->color () == b->color ())
           ? a->number () < b->number ()
           : (sortOrder[a->color ()] < sortOrder[b->color ()]));
}

/*--------------------------------------------------------------------------*/
//Purpose   : Starts part two of the game
//Parameters: player: Player to start part II
/*--------------------------------------------------------------------------*/
void Twopart::startPartTwo (unsigned int player) {
   TRACE9 ("Twopart::startPartTwo (unsigned int) - Continuing with " << player);

   Gtk::Main::timeout.connect (bind (slot (this, &Twopart::startPartTwoTimerFnc),
                                     player), 50);
   disableLastPlayer ();
}

/*--------------------------------------------------------------------------*/
//Purpose   : Shows or hides the cards of the computer player
//Parameters: open: Flag if cards should be shown or hidden
/*--------------------------------------------------------------------------*/
void Twopart::playOpen (bool open) {
   ICardPile::ShowOpt show (open ? ICardPile::SHOWFACE : ICardPile::SHOWBACK);

   for (int i (0); i < NUM_PLAYERS; ++i) {
      players[i].won.setShowOption (show);
      players[i].won.setStyle (open ? ICardPile::COMPRESSED : ICardPile::VERY_COMPRESSED);
      if (i)
         players[i].hand.setStyle ((gameStatus () == PLAYING2)
                                   ? (open ? ICardPile::COMPRESSED : ICardPile::VERY_COMPRESSED)
                                   : ICardPile::COMPRESSED);
      players[i].hand.setShowOption (i ? show : ICardPile::SHOWFACE);
   }
}

/*--------------------------------------------------------------------------*/
//Purpose   : Finds a bigger card, with respect to trumps
//Parameters: pile: Pile to analyze
//            nr: Number of card to beat
//Returns   : int: Pos to play (or -1, if no card is bigger)
/*--------------------------------------------------------------------------*/
int Twopart::findBigger (const ICardPile& pile, CardWidget::NUMBERS nr) const {
   // Find first bigger (or equal) card without checking for trumps
   int pos (pile.findFirstEqualOrBigger (nr));

   // Now check if there's a bigger trump
   if (pTrump && (pos != -1)) {
      unsigned int newPos (pos);
      while (++newPos < pile.numberOfCards ())
         if (pile.at (newPos).color () == pTrump->color ()) {
            pos = newPos;
            break;
         }
   }

   TRACE3 ("Twopart::findBigger (const ICardPile&, CardWidget::NUMBERS) const - Pos "
           << pos);
   return pos;
}
