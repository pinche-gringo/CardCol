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

   // Show and attach card-piles
   for (int i (0); i < NUM_PLAYERS; ++i) {
      players[i].won.setStyle (ICardPile::VERY_COMPRESSED);
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
   }

   played.show ();
   attach (played, 3, 11, 5, 8, 0, 0, 0, 5);

   unsigned int width (cards.getCard (0).getImageWidth ());
   unsigned int height (cards.getCard (0).getImageHeight ());

   for (unsigned int i (0); i < NUM_PLAYERS; ++i) {
      players[i].won.setShowOption (ICardPile::SHOWBACK);
      players[i].hand.setShowOption (i ? ICardPile::SHOWBACK : ICardPile::SHOWFACE);

      players[i].won.set_usize (width + 20, height + 5);
      players[i].hand.set_usize (width * 3, height + 5);
   }

   played.set_usize (width + 150, height);
   staple.set_usize (width, height);
}

/*--------------------------------------------------------------------------*/
//Purpose   : Destructor
/*--------------------------------------------------------------------------*/
Twopart::~Twopart () {
   TRACE9 ("Twopart::~Twopart ()");
   cleanTable ();
}

/*--------------------------------------------------------------------------*/
//Purpose   : Starts the game
/*--------------------------------------------------------------------------*/
void Twopart::start () {
   Game::start ();

   cleanTable ();
   randomizeCardsToPile (staple);
   dealCards ();
}

/*--------------------------------------------------------------------------*/
//Purpose   : Enables the cards of the actual player
/*--------------------------------------------------------------------------*/
int Twopart::enableActPlayer () {
   enablePlayer (actPlayer);
   return 0;
}

/*--------------------------------------------------------------------------*/
//Purpose   : Enables the cards of the passed player
//Parameters: player: Player to enable
//Remarks   : Depending of the status of the game (PLAYING2) also the top
//            card of the played pile is enabled
/*--------------------------------------------------------------------------*/
void Twopart::enablePlayer (unsigned int player) {
   Check3 (activeCards.empty ());
   Check3 (statGame >= PLAYING);

   TRACE2 ("Twopart::enablePlayer (unsigned int) - player "
           << player << " has " << players[player].hand.numberOfCards ()
           << " cards");

   for (int i (players[player].hand.numberOfCards ()); i;)
      activeCards.push_back
         (players[player].hand.at (--i).clicked.connect_after
           (bind (slot (this, (&Twopart::cardSelected)), player, i)));

   if ((statGame == PLAYING2)
       && (played.numberOfCards ()))
      pileTop = played.getTopCard ().clicked.connect_after
         (bind (slot (this, (&Twopart::playedSelected)), player));
}

/*--------------------------------------------------------------------------*/
//Purpose   : Disables the cards of the passed player
/*--------------------------------------------------------------------------*/
void Twopart::disableLastPlayer () {
   TRACE2 ("Twopart::disableLastPlayer () - " << activeCards.size () << " cards");

   Game::disableLastPlayer ();
   pileTop.disconnect ();
}

/*--------------------------------------------------------------------------*/
//Purpose   : Moving played cards (of last person) to the passed player
//Parameters: player: ID of player who should get the cards
//Requieres : Only for part 2 of the game
/*--------------------------------------------------------------------------*/
void Twopart::pickUpPlayedPile (unsigned int player) {
   TRACE3 ("Twopart::pickUpPlayedPile (unsigned int) - Player " << player
           << " picks up played pile");
   Check3 (statGame == PLAYING2);
   Check3 (bfPlayers);

   // Move played cards to player
   Check3 (offPos > 0); Check3 (offPos < NUM_PLAYERS);
   movePlayedCardsToPlayer (player, startPos[--offPos]);

   // Re-enable next two players (having cards); continue with first of them
   removePlayer (player);
   unsigned int next;
   unsigned int cAdded (0);
   for (unsigned int i (0); i < NUM_PLAYERS; ++i)
      if ((!(bfPlayers & (1 << (next = (actPlayer + i + 1) & 0x3))))
          && players[next].hand.numberOfCards ()) {
         TRACE5 ("Twopart::pickUpPlayedPile (unsigned int) - Re-adding player "
                 << next);
         addPlayer (next);
         if (++cAdded == 2)
            break;
      }
   actPlayer = findNextPlayer (player);

   TRACE7 ("Twopart::pickUpPlayedPile (unsigned int) - Continuing with player "
           << actPlayer);
}

/*--------------------------------------------------------------------------*/
//Purpose   : Callback after clicking the top card of the played pile
//Parameters: player: ID of player
//Requieres : Only for part 2 of the game
/*--------------------------------------------------------------------------*/
void Twopart::playedSelected (unsigned int player) {
   TRACE3 ("Twopart::playedSelected (unsigned int) - Player " << player
           << " picks up played pile");
   Check3 (statGame == PLAYING2);
   Check3 (bfPlayers);

   pickUpPlayedPile (player);
   makeNextMoves ();
   disableLastPlayer ();
}

/*--------------------------------------------------------------------------*/
//Purpose   : Callback after clicking on a card in hand
//Parameters: player: ID of player
//            iCard: Offset of card in hand
//Returns   : bool: Status of moving; true: Card could be moved; false else
/*--------------------------------------------------------------------------*/
bool Twopart::moveSelectedCardToPlayed (unsigned int player, unsigned int pos) {
   Check3 (player <= NUM_PLAYERS);
   Check3 (pos <= players[player].hand.numberOfCards ());
   TRACE5 ("Twopart::moveSelectedCardToPlayed (unsigned int, unsigned int) - Player: "
           << player << " at position " << pos);

   Check3 (statGame >= PLAYING);

   if (statGame == PLAYING) {
      CardWidget& card (players[player].hand.remove (pos));
      TRACE9 ("Twopart::moveSelectedCardToPlayed (unsigned int, unsinged int) - Player "
              << player << "; Card at " << pos << " = " << card);
      played.append (card);

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
      CardWidget* card (&players[player].hand.at (pos));
      unsigned int nr (card->number ());
      CardWidget::COLORS  color (card->color ());
      
      // Perform validity-check in part 2: Card must have the same color and be
      // bigger than the last played card or be a (bigger) trump
      Check3 (pTrump);
      if (played.numberOfCards ()
          && ((color == pTrump->color ())
              ? ((played.getTopCard ().color () == pTrump->color ())
                 && (played.getTopCard ().number () >= nr))
              : ((played.getTopCard ().color () != color)
                 || (played.getTopCard ().number () >= nr)))) {
         XMessageBox::Show (_("Played card(s) must have the same color and must be "
                              "bigger (or be a trump)!"), PACKAGE " - Twopart",
                            XMessageBox::ERROR);
         return false;
      }

      unsigned int posIns (played.numberOfCards ());
      if (offPos < (NUM_PLAYERS - 1))
         startPos[offPos++] = posIns;
#if TRACELEVEL > 8
      for (unsigned int i (0); i < (NUM_PLAYERS - 1); ++i)
         TRACE ("Twopart::moveSelectedCardToPlayed (unsigned int, unsinged int) - "
                << i << ". Position: " << startPos[i]);
#endif
      played.append (players[player].hand.remove (pos));
      TRACE3 ("Twopart::moveSelectedCardToPlayed (unsigned int, unsinged int) - Player "
              << player << "; Card at " << pos << " = " << *card);

      while (pos
             && ((card = &players[player].hand.at (--pos)),
                 (card->number ()) == (nr - 1))
             && (card->color () == color)) {
         TRACE9 ("Twopart::moveSelectedCardToPlayed (unsigned int, unsinged int) - Player "
                 << player << "; Card at " << pos << " = " << *card);

         nr = card->number ();
         played.insert (players[player].hand.remove (pos), posIns);
      }
   }
   return true;
}

/*--------------------------------------------------------------------------*/
//Purpose   : Callback after clicking on a card in hand
//Parameters: player: ID of player
//            iCard: Offset of card in hand
/*--------------------------------------------------------------------------*/
void Twopart::cardSelected (unsigned int player, unsigned int pos) {
   TRACE5 ("Twopart::cardSelected (unsigned int, unsigned int) - Player: "
           << player << " at position " << pos);
   Check3 (player <= NUM_PLAYERS);
   Check3 (pos <= players[player].hand.numberOfCards ());
   Check3 (statGame >= PLAYING);

   executeMove (player, pos);
   makeNextMoves ();
   disableLastPlayer ();
}

/*--------------------------------------------------------------------------*/
//Purpose   : Executes a move out of a hand
//Parameters: player: ID of player
//            iCard: Offset of card in hand
/*--------------------------------------------------------------------------*/
void Twopart::executeMove (unsigned int player, unsigned int pos) {
   TRACE5 ("Twopart::executeMove (unsigned int, unsigned int) - Player: "
           << player << " at position " << pos);
   Check3 (player <= NUM_PLAYERS);
   Check3 (pos <= players[player].hand.numberOfCards ());

   if (!moveSelectedCardToPlayed (player, pos))
      return;

   // Check if every player still in game or has already played; end round if so
   // or calculate next player if not
   TRACE7 ("Twopart::executeMove (unsigned int, unsigned int) - Players: "
           << hex << bfPlayers << dec);
   removePlayer (actPlayer = player);
   int newPlayer (actPlayer);
   if (bfPlayers)
      newPlayer = findNextPlayer (player);
   else {
      // Show trump if not already visible
      if (pTrump && !pTrump->is_visible ()) {
         pTrump->showFace ();
         pTrump->show ();
         attach (*pTrump, 2, 3, 2, 3, 0, 0, 5, 5);
      }

      newPlayer = endRound ();
   }

   // Check if the actual part is terminated
   if ((statGame == PLAYING)
       ? (newPlayer < 0)
       : (newPlayer == findNextPlayerWithCards (newPlayer))) {
      actPlayer = (statGame == PLAYING) ? ~newPlayer : newPlayer;

      std::string str ((statGame == PLAYING)
                       ? _("First part ended; Part 2 starts player %1")
                       : _("Player %1 lost"));
      str.replace (str.find ("%1"), 2, (char)(actPlayer + '0'));
      status.pop (1);
      status.push (1, str);

      if (statGame == PLAYING)
         startPartTwo (actPlayer);
      else
         statGame = STOPPED;
   }
   else {
      actPlayer = newPlayer;

      status.pop (1);
      std::string stat ( _("Turn of player %1"));
      stat.replace (stat.find ("%1"), 2, (char)(actPlayer + '0'));
      status.push (1, stat);
   }
}

/*--------------------------------------------------------------------------*/
//Purpose   : Makes the move for the next player.
//Returns   : int: Flag for timer, if it should continue (0: no; else: yes)
/*--------------------------------------------------------------------------*/
int Twopart::makeNextMove () {
   TRACE5 ("Twopart::makeNextMove () - Turn of player " << actPlayer);
   Check3 (actPlayer); Check3 (actPlayer < NUM_PLAYERS);
   if (statGame == TOSTOP) {
      TRACE8 ("Twopart::makeNextMove () - End game ");
      statGame = STOPPED;
      // TODO: Let restarting handle the parent/game
      if (restart)
         start ();
      return 0;
   }

   Check3 (statGame >= PLAYING);
   if (pos2Play == -1) {
      pos2Play = findPos2Play (actPlayer);
      if (pos2Play != -1) {
         unsigned int pos (pos2Play);
         // Flip card(s) to play
         if (statGame == PLAYING)
            players[actPlayer].hand.at (pos).showFace ();
         else
            do {
               players[actPlayer].hand.at (pos).showFace ();
               if (pos != (players[actPlayer].hand.numberOfCards () - 1))
                   players[actPlayer].hand.resize (pos, ICardPile::COMPRESSED);
            } while (pos
                     && ((players[actPlayer].hand.at (pos2Play).number ()
                          - players[actPlayer].hand.at (--pos).number ()
                          == (pos2Play - pos)))
                     && (players[actPlayer].hand.at (pos).color ()
                         == players[actPlayer].hand.at (pos2Play).color ()));
         return 1;
      }
      else {
         unsigned int oldPlayer (actPlayer);
         pickUpPlayedPile (actPlayer);

         status.pop (1);
         std::string stat ( _("Player %1 can't continue -> Picking up last cards;"
                              " Turn of player %2"));
         stat.replace (stat.find ("%1"), 2, (char)(oldPlayer + '0'));
         stat.replace (stat.find ("%2"), 2, (char)(actPlayer + '0'));
         status.push (1, stat);
      }
   }
   else
      executeMove (actPlayer, pos2Play);

   if (statGame >= PLAYING) {
      // If turn of human player: Stop computer playing
      if (!actPlayer) {
         TRACE5 ("Twopart::makeNextMove () - Enable human");
         enablePlayer (0);
      }

      pos2Play = -1;
      return actPlayer;
   }
   else
      return 0;
}

/*--------------------------------------------------------------------------*/
//Purpose   : Searches for the card(s) to play by analyzing the previously
//            played cards
//            to him if so. Else enable the players which can continue
//Parameters: player: ID of player to analyze
//Returns   : int: Position to play; or -1 if player can't continue
/*--------------------------------------------------------------------------*/
int Twopart::findPos2Play (unsigned int player) const {
   Check3 (actPlayer); Check3 (actPlayer < NUM_PLAYERS);
   Check3 (statGame >= PLAYING);
   TRACE5 ("Twopart::findPos2Play (unsigned int) - Player " << player);

   if (statGame == PLAYING) {
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
      analyzeLastPlayed (*startPos, played.numberOfCards () - *startPos,
                         maxNr, posMax, maxEqualNr, posMaxEqual);

      int pos (-1);
      // Try to get the cards if there are loads of high cards (a third or more)
      // or if the average card played is at least a 8
      if (((played.numberOfCards () / 3) < cHigh)
          || (points >= CardWidget::EIGHT)) {
         // Search for card whose number you own
         for (unsigned int i (*startPos);
              i < players[player].hand.numberOfCards (); ++i)
            if ((played.exists (players[player].hand.at (i).number ()))
                && ((posMaxEqual == -1) 
                    || (played.at (i).number () >= maxEqualNr))) {
               TRACE2 ("Twopart::findPos2Play (unsigned int) - Having equal card at "
                       << i);
               return i;
            }
         
         // Player has no equal card: Play high card if higher (and he is the
         // last player) or 
         if (((!(bfPlayers & ~(1 << player)))
              && (posMaxEqual == -1)
              && ((pos = players[player].hand.findFirstEqualOrBigger
                   (static_cast<CardWidget::NUMBERS> (maxNr))) != -1))
             // or the staple is being fighted for and player has high cards
             || (*startPos
                 && ((pos = players[player].hand.numberOfCards () - 1),
                     ((players[player].hand.at (pos).number ()
                       == CardWidget::ACE))
                     || ((playersInBitfield (bfOldPlayers) < *startPos)
                         && (players[player].hand.at (pos).number () >= maxNr)
                         && (posMaxEqual == -1))))) {
            TRACE2 ("Twopart::findPos2Play (unsigned int) - Playing highest card at "
                    << pos);
            return pos;
         }
      }

      // We don't want the pile; so try not to get it:
      TRACE5 ("Twopart::findPos2Play (unsigned int) - Avoiding pile");
      pos = ((players[player].hand.numberOfCards () > 1)
             && ((played.exists (players[player].hand.at (0).number (), *startPos)
                  && (players[player].hand.at (1).number () < CardWidget::SIX)
                  && !played.exists (players[player].hand.at (1).number (), *startPos))
                 || ((players[player].hand.at (0).number () == maxEqualNr)
                     && !played.exists (players[player].hand.at (1).number (),
                                        *startPos)
                     && (players[player].hand.at (1).number () < CardWidget::SIX))));

      // Final check: If you have to pick up the pile and you're the last,
      // use at least a high card
      if (!pos
          && !(bfPlayers & ~(1 << player))
          && (players[player].hand.numberOfCards () > 1)
          && (players[player].hand.at (pos).number () > maxNr)
          && (posMaxEqual == -1))
         pos = 1;
      return pos;
   }
   else {
      // Find first fitting card
      int pos (played.numberOfCards ()
               ? players[player].hand.find1EqualOrBigger (played.getTopCard (),
                                                          compByColorAccTrumps)
               : findSmallestCard (player));
      TRACE5 ("Twopart::findPos2Play (unsigned int) - First try: " << pos);

      Check3 (pTrump);
      if ((pos == -1)
          || (played.numberOfCards ()
              && (played.getTopCard ().color ()
                  != players[player].hand.at (pos).color ()))) {
         TRACE5 ("Twopart::findPos2Play (unsigned int) - No card found; trying trump");
         Check3 (pTrump);
         if (played.getTopCard ().color () != pTrump->color ()) {
            for (pos = players[player].hand.numberOfCards (); pos; --pos)
               if (players[player].hand.at (pos - 1).color () != pTrump->color ())
                  break;

            if (!pos)
               pos = findEndOfSerie (player, 0);
            else
               return (pos == players[player].hand.numberOfCards ()) ? -1 : pos;
         }
         else
            return -1;
      }
      else
         // Card was found; now search for last card to play (only if not trump
         // or only trump left)
         if (!pos
             || (players[player].hand.at (pos).color () != pTrump->color ()))
            pos = findEndOfSerie (player, pos);
      TRACE5 ("Twopart::findPos2Play (unsigned int) - Playing card at pos " << pos);
      return pos;
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
         pos = endPos;
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
//Purpose   : Checks if there is a winner for the round and moves played cards
//            to him if so. Else enable the players which can continue
//Returns   : int: Next player; or -1 if there is no next player
/*--------------------------------------------------------------------------*/
int Twopart:: endRound () {
   TRACE8 ("Twopart::endRound ()");
   Check3 (!bfPlayers);

   unsigned int nextPlayer (NUM_PLAYERS);

   if (statGame == PLAYING2) {
      played.clear ();
      bfPlayers = (1 << NUM_PLAYERS) - 1;
      removePlayersWithoutCards ();
      offPos = 0;
      nextPlayer = (!players[actPlayer].hand.numberOfCards ())
         ? findNextPlayer (actPlayer) : actPlayer;
   }
   else {
      bfPlayers = bfOldPlayers;
      unsigned int cPlayers (playersInBitfield (bfPlayers));

      TRACE8 ("Twopart::endRound () - Round has " << cPlayers << " players; Start = "
              << *startPos << " of " << played.numberOfCards () << " cards");
      Check3 ((*startPos + cPlayers) <= played.numberOfCards ());

      int maxNr (-1);
      int maxEqualNr (-1);
      int posMax (-1);
      int posMaxEqual (-1);
      analyzeLastPlayed (*startPos, cPlayers, maxNr, posMax, maxEqualNr, posMaxEqual);

      TRACE4 ("Twopart::endRound () - Player starting round: " << startPlayer
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
               
               TRACE5 ("Twopart::endRound () - Found equal cards; Player "
                       << pos2Player (i - *startPos)
                       << (cPlayers ? " still in round" : " is winner"));
               ++cPlayers;
            }
            else
               bfPlayersOut |= (1 << pos2Player (i - *startPos));
         } // endfor check for equal cards
         bfPlayers &= ~bfPlayersOut;
         TRACE5 ("Twopart::endRound () - Found equal cards; " << cPlayers
                 << " player(s) still in round (" << hex << bfPlayers << dec << ')');

         // Find player to continue
         if (!players[nextPlayer].hand.numberOfCards ())
            nextPlayer = findNextPlayer (nextPlayer);
         TRACE6 ("Twopart::endRound () - Try to continue with player " << nextPlayer);
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
         TRACE5 ("Twopart::endRound () - Found winner: " << nextPlayer);

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
   TRACE8 ("Twopart::endRound () - Continuing with player " << nextPlayer);
   return startPlayer = nextPlayer;
}

/*--------------------------------------------------------------------------*/
//Purpose   : Analyzes the played staple and retrieves the highest card(s)
//Parameters: startPos: Position from where to start analyzing
//            max: Highest single card
//            maxPos: Position of highest single card
//            maxEqua: Highest pair
//            maxEquaPos: Position of highest equal card
/*--------------------------------------------------------------------------*/
void Twopart::analyzeLastPlayed (unsigned int startPos, unsigned int cards,
                                 int& max, int& maxPos, int& maxEqual,
                                 int& maxEqualPos) const {
   cards += startPos;
   // Check if card is bigger then all previous
   for (; startPos < cards; ++startPos) {
      if ((int)(played.at (startPos).number ()) > max) {
         TRACE3 ("TwoPart::analyzePlayed () - New highest card " << played.at (startPos)
                 << " at position " << startPos);
         max = (int)played.at (startPos).number ();
         maxPos = startPos;
         }
         Check3 (maxPos < played.numberOfCards ());
         Check3 (max == played.at (maxPos).number ());

         // Check if card has equal cards
         for (unsigned int j (startPos + 1); j < cards; ++j)
            if (played.at (startPos).number () == played.at (j).number ())
               if ((int)(played.at (startPos).number ()) > maxEqual) {
                  TRACE3 ("TwoPart::analyzePlayed () - Found equal "
                          << played.at (startPos).numberStr ()
                          << " at positions " << startPos << " and " << j);
                  maxEqual = (int)played.at (startPos).number ();
                  maxEqualPos = startPos;
                  break;
               } // endif equal card found
   } // end-for all players still in game
}

/*--------------------------------------------------------------------------*/
//Purpose   : Finds the next player having cards
//Parameters: player: Number of player to start with
//Returns   : int: Number of next player (or -1)
/*--------------------------------------------------------------------------*/
int Twopart::findNextPlayerWithCards (unsigned int player) {
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
int Twopart::findNextPlayer (unsigned int player) {
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

   while (played.numberOfCards () > start)
      if (statGame == PLAYING)
         players[receiver].won.append (played.remove (0));
      else
         players[receiver].hand.append (played.remove (start));

   if (statGame == PLAYING2)
      players[receiver].hand.sort (compByColorAccTrumps);
}

/*--------------------------------------------------------------------------*/
//Purpose   : Remove cards from everything which can hold them
/*--------------------------------------------------------------------------*/
void Twopart::cleanTable () {
   staple.clear ();                                             // Clear staple
   for (int i (0); i < NUM_PLAYERS; ++i) {            // Clear cards of players
      players[i].hand.clear ();
      players[i].hand.setStyle (i ? ICardPile::COMPRESSED : ICardPile::NORMAL);
      players[i].won.clear ();
   }
   played.clear ();

   disableLastPlayer ();
   staple.show ();
   if (pTrump) {
      delete pTrump;
      pTrump = NULL;
   }

   pos2Play = -1;
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
   enablePlayer (actPlayer = startPlayer = 0);
}

/*--------------------------------------------------------------------------*/
//Purpose   : Starts part two of the game
//Returns   : int: Value indicating if timer should continue
/*--------------------------------------------------------------------------*/
int Twopart::startPartTwoTimerFnc () {
   TRACE8 ("Twopart::startPartTwoTimerFnc ()");
   Check3 (!bfPlayers);
   statGame = PLAYING2;

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
         TRACE5 ("Twopart::startPartTwoTimerFnc () - Player " << i << " has no cards");
         bfPlayers |= 1 << i;
         ++nrPlayers;
      }

   // Now move the cards from the played pile to the hand; if there are
   // players without cards give them the cards up to 5
   unsigned int victim (actPlayer);
   for (unsigned int i (0); i < NUM_PLAYERS; ++i) {
      for (unsigned int j (players[i].won.numberOfCards ()); j; --j) {
         CardWidget& card (players[i].won.removeTopCard ());
         TRACE9 ("Twopart::startPartTwoTimerFnc () - Moving cards " << card
                 << " for player " << i);
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
   if (!(startPlayer = actPlayer))
      enablePlayer (0);

   pos2Play = -1;
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
   TRACE9 ("Twopart::startPartTwo () - *** Start timer ***");
   // Delay starting of part two, in case of human player; as re-enabling a
   // signal (button-callback) inside the signal handler wreaks quite a bit
   // of havoc
   if (actPlayer = player)
      startPartTwoTimerFnc ();
   else
      Gtk::Main::timeout.connect (slot (this, &Twopart::startPartTwoTimerFnc), 100);
}

/*--------------------------------------------------------------------------*/
//Purpose   : Makes the next move(s)
/*--------------------------------------------------------------------------*/
void Twopart::makeNextMoves () {
   TRACE9 ("Twopart::makeComputerMoves () - *** Start timer ***");
   Check3 ((statGame == PLAYING) || (statGame == PLAYING2));
   Gtk::Main::timeout.connect (slot (this, actPlayer
                                     ? &Twopart::makeNextMove
                                     : &Twopart::enableActPlayer),
                               actPlayer ? 700 : 50);
}
