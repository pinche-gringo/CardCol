//$Id$

//PROJECT     : Cardgames
//SUBSYSTEM   : Twopart
//REFERENCES  :
//TODO        :
//BUGS        :
//REVISION    : $Revision$
//AUTHOR      : Markus Schwab
//CREATED     : 20.7.2002
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

#include <ctime>
#include <cstdlib>

#include <sstream>

#include <glibmm/main.h>

#include <gtkmm/box.h>
#include <gtkmm/statusbar.h>
#include <gtkmm/messagedialog.h>

#include <Check.h>
#include <Trace_.h>
#include <ConnMgr.h>
#include <Tokenize.h>

#include <Player.h>
#include <CardSet.h>
#include <CardWidget.h>

#include "Twopart.h"


const unsigned int Twopart::COLS_PLAYER[NUM_PLAYERS] = { 7, 13, 7, 1 };
const unsigned int Twopart::ROWS_PLAYER[NUM_PLAYERS] = { 4,  8, 10, 8 };

char Twopart::sortOrder[4];


//-----------------------------------------------------------------------------
/// Defaultconstructor; all widget are created
/// \param parent: Parent widget to display the game in
/// \param statusbar: Status bar widget to display information about the game
/// \param cardset: Cardset to use
/// \param players: Vector of player
/// \param posPlayer: Position of player for the server
/// \param mxSerialize: Mutex to serialize messages from the server
//-----------------------------------------------------------------------------
Twopart::Twopart (Gtk::Box& parent, Gtk::Statusbar& statusbar, 
                  CardSet& cardset, const std::vector<Player*>& player,
                  unsigned int posPlayer, Mutex& mxSerialize)
   : Game (parent, statusbar, cardset, player, posPlayer, mxSerialize, 12, 15)
     , played (ICardPile::COMPRESSED, ICardPile::SHOWFACE)
     , staple (ICardPile::VERY_COMPRESSED, ICardPile::SHOWBACK)
     , bfPlayers ((1 << NUM_PLAYERS) - 1), pTrump (NULL), offPos (0)
     , bfOldPlayers (bfPlayers) {
   staple.show ();
   attach (staple, 2, 3, 2, 3, Gtk::SHRINK, Gtk::SHRINK, 5, 5);

   int width (cards.getCard (0).getImageWidth ());
   int height (cards.getCard (0).getImageHeight ());

   // Show and attach card-piles
   changeNames (player);
   for (int i (0); i < NUM_PLAYERS; ++i) {
      players[i].name.show ();
      attach (players[i].name, COLS_PLAYER[i], COLS_PLAYER[i] + 3,
              ROWS_PLAYER[i] + ((i == 2) ? 3 : 1),
              ROWS_PLAYER[i] + ((i == 2) ? 4 : 2),
              Gtk::EXPAND, Gtk::EXPAND, 1);
      TRACE9 ("Twopart::Twopart () - Name at: " << COLS_PLAYER[i] << '/'
              << ROWS_PLAYER[i] + ((i == 2) ? 3 : 1));

      players[i].won.show ();
      attach (players[i].won, COLS_PLAYER[i] + 1,
              COLS_PLAYER[i] + 2,
              ROWS_PLAYER[i] + ((i == 2) ? 2 : -2),
              ROWS_PLAYER[i] + ((i == 2) ? 3 : -1),
              Gtk::SHRINK, Gtk::SHRINK, 1);
      TRACE9 ("Twopart::Twopart () - Won pile at: "
              << COLS_PLAYER[i] + 1 << '/' << ROWS_PLAYER[i] + ((i == 2) ? 2 : -2));

      players[i].hand.show ();
      attach (players[i].hand, COLS_PLAYER[i],
              COLS_PLAYER[i] + 3, ROWS_PLAYER[i],
              ROWS_PLAYER[i] + 1, Gtk::SHRINK, Gtk::SHRINK, 1);
      TRACE9 ("Twopart::Twopart () - Hand at: "
              << COLS_PLAYER[i] << '/' << ROWS_PLAYER[i]);

      players[i].won.setShowOption (ICardPile::SHOWBACK);
      players[i].hand.setShowOption (i ? ICardPile::SHOWBACK : ICardPile::SHOWFACE);

      players[i].won.set_size_request (width + 20, height + 5);
      players[i].hand.set_size_request (width * 3, height + 5);
   }

   played.show ();
   attach (played, 3, 11, 6, 9, Gtk::SHRINK, Gtk::SHRINK, 0, 5);

   played.set_size_request (width + 150, height);
   staple.set_size_request (width, height);

   srand (time (NULL));
}

//-----------------------------------------------------------------------------
/// Destructor
//-----------------------------------------------------------------------------
Twopart::~Twopart () {
   TRACE9 ("Twopart::~Twopart ()");
}

//-----------------------------------------------------------------------------
/// Starts the game
//-----------------------------------------------------------------------------
void Twopart::start () {
   Game::start ();
   if (randomizeCardsToPile (staple)) {
      // Show cards on the table: For all players put 3 cards in hand
      for (unsigned int i (0); i < NUM_PLAYERS; ++i)
         for (unsigned int j (0); j < 3; ++j)
            players[(i - posServer) & 0x3].hand.insertSorted (staple.removeTopCard ());

      for (unsigned int i (0); i < (NUM_PLAYERS - 1); ++i)
         startPos[i] = 0;
      offPos = 0;

      bfPlayers = bfOldPlayers = (1 << NUM_PLAYERS) - 1;

      for (unsigned int i (1); i < NUM_PLAYERS; ++i) {
          players[i].hand.setStyle (ICardPile::COMPRESSED);
          players[i].won.setStyle (ICardPile::VERY_COMPRESSED);
      }
      players[0].hand.setStyle (ICardPile::NORMAL);
      players[0].won.setStyle (ICardPile::QUITE_COMPRESSED);

      pos1Play = pos2Play = -1U;

      if (getConnectionMgr ().getMode () != ConnectionMgr::CLIENT) {
         setNextPlayer (startPlayer = rand () & 0x3);

         // Send startplayer to the clients
         if (getConnectionMgr ().getMode () == ConnectionMgr::SERVER) {
            const std::vector<Socket*>& clients (getConnectionMgr ().getClients ());
            unsigned int player ((currentPlayer () - 1) & 0x3);
            for (std::vector<Socket*>::const_iterator i (clients.begin ());
                 i != clients.end (); ++i) {
               std::ostringstream msg;
               msg << "ActPlayer=" << player;
               writeMessage (**i, msg.str ());
               player = (player + 1) & 0x3;
            }
         }
         displayTurn (currentPlayer ());
         makeNextMoves ();
      }
   }
}

//-----------------------------------------------------------------------------
/// Enables the cards of the passed player
/// \returns bool: Flag, if timer should be continued; False
/// \remarks Depending of the status of the game (PLAYING2) also the top card
///     of the played pile is enabled
//-----------------------------------------------------------------------------
bool Twopart::enableHuman () {
   Check3 (activeCards.empty ());
   Check3 (gameStatus () >= PLAYING);

   TRACE2 ("Twopart::enableHuman () - Has " << players[0].hand.size ()
           << " cards");

   for (int i (players[0].hand.size ()); i;)
      activeCards.push_back
         (players[0].hand[--i]->signal_clicked ().connect
           (bind (slot (*this, (&Twopart::cardSelected)), i)));

   if ((gameStatus () == PLAYING2)
       && (played.size ()))
      activeCards.push_back
         (played.getTopCard ().signal_clicked ().connect
          (slot (*this, (&Twopart::playedSelected))));

   return Game::enableHuman ();
}

//-----------------------------------------------------------------------------
/// Moving played cards (of last person) to the passed player
/// \param player: ID of player who should get the cards
/// \returns unsigned int: Next player
/// \pre Call only in part 2 of the game
//-----------------------------------------------------------------------------
unsigned int Twopart::pickUpPlayedPile (unsigned int player) {
   TRACE3 ("Twopart::pickUpPlayedPile (unsigned int) - Player " << player
           << " picks up played pile");
   Check3 (gameStatus () == PLAYING2);
   Check3 (bfPlayers);

   Glib::ustring stat ( _("%1 can't continue -> Picking up last cards; "));
   Check3 (actPlayers.size () > player);
   Check3 (actPlayers[player]);
   stat.replace (stat.find ("%1"), 2, actPlayers[player]->getName ());
   displayTurn (player, stat);

   // Move played cards to player
   Check3 (offPos > 0); Check3 (offPos < NUM_PLAYERS);
   movePlayedCardsToPlayer (player, startPos[--offPos]);
   removePlayer (player);

   // Calculate players to re-enable: They are the number of players still
   // in game (with cards) minus the players still in round; but maximal 2
   unsigned int num (0);
   for (unsigned int i (0); i < NUM_PLAYERS; ++i)
      if (players[i].hand.size ())
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
          && players[next].hand.size ()) {
         TRACE5 ("Twopart::pickUpPlayedPile (unsigned int) - Re-adding player "
                 << next);
         addPlayer (next);
         --num;
      }
   }
   return findNextPlayer (player);
}

//-----------------------------------------------------------------------------
/// Callback after clicking the top card of the played pile
/// \pre Call only in part 2 of the game
//-----------------------------------------------------------------------------
void Twopart::playedSelected () {
   TRACE3 ("Twopart::playedSelected (unsigned int) - Human picks up played pile");
   Check3 (gameStatus () == PLAYING2);
   Check3 (bfPlayers);

   if (getConnectionMgr ().getMode () == ConnectionMgr::CLIENT) {
      Check3 (startPos[--offPos] < played.size ());
      std::ostringstream msg;
      msg << "Play=" << played[startPos[offPos - 1]]->id () << ";Target=1";
      ignoreNextMsg = true;
      broadcastMessage (msg.str ());
   }
   setNextPlayer (pickUpPlayedPile (0));
   makeNextMoves ();
   disableHuman ();
}

//-----------------------------------------------------------------------------
/// Callback after clicking on a card in hand
/// \param player: ID of player
/// \param start: Offset of first card to play
/// \param end: Offset of last card to play
/// \returns bool: Status of moving; true: Card could be moved; false else
//-----------------------------------------------------------------------------
bool Twopart::moveSelectedCardToPlayed (unsigned int player,
                                        unsigned int start, unsigned int end) {
   TRACE5 ("Twopart::moveSelectedCardToPlayed (unsigned int, unsigned int) - Player: "
           << player << " at position " << start << " to " << end);
   Check3 (player < NUM_PLAYERS);
   Check3 (end < players[player].hand.size ());
   Check3 (start <= end);
   Check3 (gameStatus () >= PLAYING);

   if (gameStatus () == PLAYING) {
      Check3 (start == end);
      TRACE9 ("Twopart::moveSelectedCardToPlayed (unsigned int, unsinged int)"
              " - Player " << player << "; Card at " << end << " = "
              << *players[player].hand[end]);
      movePile (played, players[player].hand, start, end);

      if (staple.size ()) {
         CardWidget& card (staple.removeShownTopCard ());
         players[player].hand.insertSorted (card);

         if (!staple.size ()) {
            Check3 (!pTrump);
            pTrump = new CardWidget (card); Check3 (pTrump);
            staple.hide ();
         }
      }
   }
   else {
      if (offPos < (NUM_PLAYERS - 1))
         startPos[offPos++] = played.size ();
#if TRACELEVEL > 8
      for (unsigned int i (0); i < (NUM_PLAYERS - 1); ++i)
         TRACE ("Twopart::moveSelectedCardToPlayed (unsigned int, unsinged int) - "
                << i << ". Position: " << startPos[i]);
#endif
      movePile (played, players[player].hand, start, end);
   }
   return true;
}

//-----------------------------------------------------------------------------
/// Callback after clicking on a card in hand
/// \param pos: Offset of card in hand
//-----------------------------------------------------------------------------
void Twopart::cardSelected (unsigned int pos) {
   TRACE5 ("Twopart::cardSelected (unsigned int) - Position " << pos);
   Check3 (pos < players[0].hand.size ());
   Check3 (gameStatus () >= PLAYING);

   CardWidget& card (*players[0].hand[pos]);
   unsigned int nr (card.number ());
   CardWidget::COLOURS colour (card.colour ());
      
   // Perform validity-check in part 2: Card must have the same colour and be
   // bigger than the last played card or be a (bigger) trump
   if (gameStatus () == PLAYING2) {
      Check3 (pTrump);
      if (played.size ()) {
         CardWidget& top (played.getTopCard ());

         if ((colour == pTrump->colour ())
             ? ((top.colour () == pTrump->colour ())
                && (top.number () >= nr))
             : ((top.colour () != colour)
                || (top.number () >= nr))) {
            Gtk::MessageDialog dlg (_("Played card(s) must have the same colour"
                                      " and must be bigger (or be a trump)!"),
                                    Gtk::MESSAGE_ERROR);
            dlg.set_title (PACKAGE " - Twopart");
            dlg.run ();
            return;
         }
      }
   }

   unsigned int start ((gameStatus () == PLAYING2) ? findStartOfSerie (0, pos) : pos);
   // Inform the others about the move
   if (getConnectionMgr ().getMode () != ConnectionMgr::NONE) {
      // Send played card to all clients (if any)
      std::ostringstream msg;
      msg << "Play=";
      for (unsigned int i (start); i < pos; ++i)
         msg << players[0].hand[i]->id () << ' ';
      msg << players[0].hand[pos]->id () << ";Target=0";

      if (getConnectionMgr ().getMode () == ConnectionMgr::CLIENT)
         ignoreNextMsg = pos - start + 1;
      broadcastMessage (msg.str ());
   }

   setNextPlayer (executeMove (0, start, pos));
   makeNextMoves ();
}

//-----------------------------------------------------------------------------
/// Executes a move out of a hand
/// \param player: ID of player
/// \param start: Offset of first card to play
/// \param end: Offset of last card to play
/// \returns int: Next player or -1 at end
//-----------------------------------------------------------------------------
int Twopart::executeMove (unsigned int player, unsigned int start, unsigned int end) {
   TRACE5 ("Twopart::executeMove (unsigned int, unsigned int) - Player: "
           << player << " at position " << start << " to " << end);
   Check3 (player < NUM_PLAYERS);
   Check3 (start <= end);
   Check3 (end < players[player].hand.size ());

   if (!moveSelectedCardToPlayed (player, start, end))
      return player;

   // Check if every player is still in game or has already played; end round
   // if so or calculate next player if not
   TRACE7 ("Twopart::executeMove (unsigned int, unsigned int) - Players: "
           << std::hex << bfPlayers << std::dec);
   removePlayer (player);
   int newPlayer (player);
   if (bfPlayers)
      newPlayer = findNextPlayer (player);
   else {
      // Show trump if not already visible
      if (pTrump && !pTrump->is_visible ()) {
         pTrump->showFace ();
         pTrump->show ();
         attach (*pTrump, 2, 3, 2, 3, Gtk::SHRINK, Gtk::SHRINK, 5, 5);
      }

      newPlayer = endRound (player);
   }

   // Check if the actual part is terminated
   if ((gameStatus () == PLAYING)
       ? (newPlayer < 0)
       : (newPlayer == findNextPlayerWithCards (newPlayer))) {
      player = (gameStatus () == PLAYING) ? ~newPlayer : newPlayer;

      Glib::ustring str ((gameStatus () == PLAYING)
                         ? _("First part ended; Part 2 starts %1")
                         : _("%1 lost"));
      Check3 (actPlayers.size () > player);
      Check3 (actPlayers[player]);
      str.replace (str.find ("%1"), 2, actPlayers[player]->getName ());
      status.pop ();
      status.push (str);

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

//-----------------------------------------------------------------------------
/// Makes the move for the next player.
/// \param player: Actual player
/// \returns int: Next player or -1 if end of game
//-----------------------------------------------------------------------------
int Twopart::makeMove (unsigned int player) {
   TRACE5 ("Twopart::makeMove () - Turn of player " << player);
   Check3 (gameStatus () >= PLAYING);

   if (pos2Play == -1U) {
      if (findPos2Play (player, pos1Play, pos2Play) != -1) {
         // Flip card(s) to play
         flipCards2Play (players[player].hand, pos1Play, pos2Play);
         return player;
      }
      else
         player = pickUpPlayedPile (player);
   }
   else {
      player = executeMove (player, pos1Play, pos2Play);
      pos1Play = pos2Play = -1U;
   }

   return player;
}

//-----------------------------------------------------------------------------
/// Searches for the card(s) to play by analyzing the previously played cards
/// to him if so. Else enable the players which can continue
/// \param player: ID of player to analyze
/// \param start: Offset of first card to play
/// \param end: Offset of last card to play
/// \returns int: Position to play; or -1 if player can't continue
//-----------------------------------------------------------------------------
int Twopart::findPos2Play (unsigned int player, unsigned int& start,
                           unsigned int& end) const {
   Check3 (player); Check3 (player < NUM_PLAYERS);
   Check3 (gameStatus () >= PLAYING);
   TRACE5 ("Twopart::findPos2Play (unsigned int) - Player " << player);

   if (gameStatus () == PLAYING) {
      unsigned int points (0);
      unsigned int cHigh (0);

      // Analyze played staple
      for (unsigned int i (0); i < played.size (); ++i) {
         CardWidget::NUMBERS nr (played[i]->number ());
         points += nr;
         if (nr >= CardWidget::TEN)
            ++cHigh;
      }

      TRACE2 ("Twopart::findPos2Play (unsigned int) - Points: " << points
              << "; Avg: " << (played.size ()
                               ? (points / played.size ()) : 0)
              << "; High: " << cHigh);
      if (played.size ())
         points /= played.size ();

      int maxNr (-1);
      int maxEqualNr (-1);
      int posMax (-1);
      int posMaxEqual (-1);
      int trumps (0);
      analyzeLastPlayed (*startPos, played.size () - *startPos,
                         maxNr, posMax, maxEqualNr, posMaxEqual, trumps);

      start = -1U;
      // Try to get the cards if there are loads of high cards (a third or more)
      // or if the average card played is at least a 8 or there are trumps inside
      if (((played.size () / 3) < cHigh)
          || (points >= CardWidget::SEVEN)
          || trumps) {
         end = -1U;
         // Search for card whose number you own
         for (start = 0;
              start < players[player].hand.size (); ++start)
            if ((played.exists (players[player].hand[start]->number (), *startPos))
                && ((posMaxEqual == -1) 
                    || (played[start]->number () >= maxEqualNr))) {
               TRACE2 ("Twopart::findPos2Play (unsigned int) - Having equal card at "
                       << start);

               // Use card if it's a trump
               if (pTrump
                   && (pTrump->colour () == players[player].hand[start]->colour ())) {
                  return end = start;
               }
               if (end == -1U)
                  end = start;
            }
         // Reset start to first found card (or 0)
         Check3 (start == players[player].hand.size ());
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
                 && ((end = players[player].hand.size () - 1),
                     ((players[player].hand[end]->number ()
                       == CardWidget::ACE))
                     || ((playersInBitfield (bfOldPlayers) < *startPos)
                         && (players[player].hand[end]->number () >= maxNr)
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
      if (!players[player].won.size ()
          && (staple.size () < 13)
          && (posMaxEqual == -1)
          && (((!(bfPlayers & ~(1 << player)))
               || (!(bfPlayers & ~((1 << player)
                                   | (1 << findNextPlayer ((player + 1) & 0x3))))
                   && (points < CardWidget::SIX)))
              && (maxNr < players[player].hand[players[player].hand.size ()-1]->number ())))
         return end = start = 2;

      // We don't want the pile; so try not to get it. To do so, play
      // the second (biggest) card, if the first card exists in the pile and
      // the second not (though the second card must also small).
      // An exception is also for the last player
      TRACE5 ("Twopart::findPos2Play (unsigned int) - Avoiding pile");
      start = ((players[player].hand.size () > 1)
               && played.exists (players[player].hand[0]->number (), *startPos)
               && !played.exists (players[player].hand[1]->number (), *startPos)
               && (((!(bfPlayers & ~(1 << player)))
                    && (players[player].hand[1]->number () < maxNr))
                   || (players[player].hand[1]->number () <= CardWidget::SEVEN)));
      TRACE5 ("Twopart::findPos2Play (unsigned int) - Avoiding returns " << start);

      // Final check: If you have to pick up the pile and you're the last,
      // use at least a high card (unless there are doubles)
      if (!start
          && (posMaxEqual == -1)
          && !(bfPlayers & ~(1 << player))
          && (players[player].hand.size () > 1)
          && (players[player].hand[start]->number () > maxNr))
         start = 1;

      TRACE5 ("Twopart::findPos2Play (unsigned int) - Playing card at " << start);
      return end = start;
   }
   else {
      // Find first fitting card
      start = (played.size ()
               ? players[player].hand.find1EqualOrBigger (played.getTopCard (),
                                                          compByColourAccTrumps)
               : findSmallestCard (player));
      TRACE5 ("Twopart::findPos2Play (unsigned int) - First try (II): " << start);

      Check3 (pTrump);
      if ((start == -1U)
          || (played.size ()
              && (played.getTopCard ().colour ()
                  != players[player].hand[start]->colour ()))) {
         TRACE5 ("Twopart::findPos2Play (unsigned int) - No card found; trying trump");
         Check3 (pTrump);
         if (played.getTopCard ().colour () != pTrump->colour ()) {
            for (start = players[player].hand.size (); start; --start)
               if (players[player].hand[start - 1]->colour () != pTrump->colour ())
                  break;

            if (!start)
               end = findEndOfSerie (player, 0);
            else {
               end = players[player].hand.size () - 1;
               // Take up pile if no trump was found or if only a "small amount"
               // of trumps are left (like less than 4 or less than the half)
               // and you are not the last player
               return (((start == players[player].hand.size ())
                        || ((bfPlayers & ~(1 << player))
                            && (((end - start) < 4)
                                || (start < (end - start)))))
                       ? (end = -1U) : (end = start));
            }
         }
         else
            return end = -1U;
      }
      else
         // Card was found; now search for last card to play (only if not trump
         // or only trump left)
         end = start;
         if (!start
             || (players[player].hand[start]->colour () != pTrump->colour ()))
            end = findEndOfSerie (player, start);
      TRACE5 ("Twopart::findPos2Play (unsigned int) - Playing card at pos " << start);
      return start;
   }
}

//-----------------------------------------------------------------------------
/// Searches for the smallest card in the hand of the passed player
/// \param player: Player to inspect
/// \returns int: Position of smallest card
//-----------------------------------------------------------------------------
unsigned int Twopart::findSmallestCard (unsigned int player) const {
   TRACE5 ("Twopart::findSmallestCard (unsigned int) - Inspecting player " << player);
   Check3 (player < NUM_PLAYERS);
   Check3 (pTrump);

   unsigned int nrMin (CardWidget::UNREACHABLE);
   unsigned int cSerie (0);
   unsigned int pos (0);
   for (unsigned int i (0); i < players[player].hand.size (); ++i) {
      CardWidget& card (*players[player].hand[i]);

      // Stop searching if a trump was found
      if ((card.colour () == pTrump->colour ()) && i)
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

//-----------------------------------------------------------------------------
/// Searches for the last position of the card which are in a serie (same
/// colour; number increasing by 1)
/// \param player: Player to inspect
/// \param start: Position to start
/// \returns unsigned int: Position of last card in serie
//-----------------------------------------------------------------------------
unsigned int Twopart::findEndOfSerie (unsigned int player, unsigned int start) const {
   Check3 (start < players[player].hand.size ());

   CardWidget* card (players[player].hand[start]);
   CardWidget::NUMBERS nr (card->number ());
   CardWidget::COLOURS colour (card->colour ());

   while ((++start < players[player].hand.size ())
          && ((card = players[player].hand[start]),
              (card->number ()) == (nr + 1))
          && (card->colour () == colour)) {
      TRACE9 ("Twopart::findEndOfSerie (unsigned int, unsigned int) - Next valid card "
              << *card << " at " << start);
      nr = card->number ();
   }

   return start - 1;
}

//-----------------------------------------------------------------------------
/// Searches for the first position of the card which are in a serie (same
/// colour; number decreasing by 1)
/// \param player: Player to inspect
/// \param start: Position to start
/// \returns unsigned int: Position of first card in serie
//-----------------------------------------------------------------------------
unsigned int Twopart::findStartOfSerie (unsigned int player, unsigned int start) const {
   Check3 (start < players[player].hand.size ());

   CardWidget* card (players[player].hand[start]);
   CardWidget::NUMBERS nr (card->number ());
   CardWidget::COLOURS colour (card->colour ());

   while ((--start < players[player].hand.size ())
          && ((card = players[player].hand[start]),
              (card->number ()) == (nr - 1))
          && (card->colour () == colour)) {
      TRACE9 ("Twopart::findStartOfSerie (unsigned int, unsigned int) - Next valid card "
              << *card << " at " << start);
      nr = card->number ();
   }

   return start + 1;
}

//-----------------------------------------------------------------------------
/// Checks if there is a winner for the round and moves played cards to him if
/// so. Else enable the players which can continue
/// \param player: Actual player (ending the round)
/// \returns int: Next player; or -1 if there is no next player
//-----------------------------------------------------------------------------
int Twopart:: endRound (unsigned int player) {
   TRACE8 ("Twopart::endRound (unsigned int)");
   Check3 (!bfPlayers);

   unsigned int nextPlayer (NUM_PLAYERS);

   if (gameStatus () == PLAYING2) {
      played.clear ();
      bfPlayers = (1 << NUM_PLAYERS) - 1;
      removePlayersWithoutCards ();
      offPos = 0;
      nextPlayer = (!players[player].hand.size ())
         ? findNextPlayer (player) : player;
   }
   else {
      bfPlayers = bfOldPlayers;
      unsigned int cPlayers (playersInBitfield (bfPlayers));

      TRACE8 ("Twopart::endRound (unsigned int) - Round has " << cPlayers
              << " players; Start = " << *startPos << " of " << played.size ()
              << " cards");
      Check3 ((*startPos + cPlayers) <= played.size ());

      int maxNr (-1);
      int maxEqualNr (-1);
      int posMax (-1);
      int posMaxEqual (-1);
      int trumps (-1);
      analyzeLastPlayed (*startPos, cPlayers, maxNr, posMax, maxEqualNr,
                         posMaxEqual, trumps);

      TRACE4 ("Twopart::endRound (unsigned int) - Player starting round: "
              << startPlayer << "; players: " << cPlayers);
      Check3 ((*startPos + cPlayers) == played.size ());

      // Equal cards found
      if (posMaxEqual >= 0) {
         unsigned int bfPlayersOut (0);
         cPlayers = 0;

         // Add players having equal cards and having still cards left
         nextPlayer = startPlayer;
         for (unsigned int i (*startPos); i < played.size (); ++i) {
            if ((played[i]->number () == maxEqualNr)
                && players[pos2Player (i - *startPos)].hand.size ()) {
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
         TRACE5 ("Twopart::endRound (unsigned int) - Found equal cards; "
                 << cPlayers << " player(s) still in round (" << std::hex
                 << bfPlayers << std::dec << ')');

         // Find player to continue
         if (!players[nextPlayer].hand.size ())
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

         if (!players[nextPlayer].hand.size ())
            nextPlayer = findNextPlayer (nextPlayer);
         if (nextPlayer == -1)
            nextPlayer = ~startPlayer;
      }
      *startPos = played.size ();
   }

   bfOldPlayers = bfPlayers;
   TRACE8 ("Twopart::endRound (unsigned int) - Continuing with player " << nextPlayer);
   return startPlayer = nextPlayer;
}

//-----------------------------------------------------------------------------
/// Analyzes the played staple and retrieves the highest card(s)
/// \param startPos: Position from where to start analyzing
/// \param max: Highest single card
/// \param maxPos: Position of highest single card
/// \param maxEqua: Highest pair
/// \param maxEquaPos: Position of highest equal card
/// \param trumps: Number of trumps
/// \remarks Values are not resetted!
//-----------------------------------------------------------------------------
void Twopart::analyzeLastPlayed (unsigned int startPos, unsigned int cards,
                                 int& max, int& maxPos, int& maxEqual,
                                 int& maxEqualPos, int& trumps) const {
   TRACE3 ("TwoPart::analyzeLastPlayed (...) - Analyzing cards [" << startPos
           << " to " << (cards + startPos) << ") of " << played.size ());
   cards += startPos; Check3 (cards <= played.size ());

   // Check if card is bigger then all previous
   for (; startPos < cards; ++startPos) {
      if ((int)(played[startPos]->number ()) > max) {
         TRACE3 ("TwoPart::analyzeLastPlayed (...) - New highest card "
                 << *played[startPos] << " at position " << startPos);
         max = (int)played[startPos]->number ();
         maxPos = startPos;
      }
      Check3 (maxPos < played.size ());
      Check3 (max == played[maxPos]->number ());

      // Add trumps
      if (pTrump
          && (pTrump->colour () == played[startPos]->colour ()))
         ++trumps;

      // Check if card has equal cards
      for (unsigned int j (startPos + 1); j < cards; ++j)
         if (played[startPos]->number () == played[j]->number ())
            if ((int)(played[startPos]->number ()) > maxEqual) {
               TRACE3 ("TwoPart::analyzeLastPlayed (...) - Found equal "
                       << played[startPos]->numberStr ()
                       << " at positions " << startPos << " and " << j);
               maxEqual = (int)played[startPos]->number ();
               maxEqualPos = startPos;
               break;
            } // endif equal card found
   } // end-for all cards

   TRACE3 ("TwoPart::analyzeLastPlayed (...) - Trumps: " << trumps);
}

//-----------------------------------------------------------------------------
/// Finds the next player having cards
/// \param player: Number of player to start with
/// \returns int: Number of next player (or -1)
//-----------------------------------------------------------------------------
int Twopart::findNextPlayerWithCards (unsigned int player) const {
   unsigned int i (player);
   do {
      i = (i + 1) & 0x3;
      if (players[i].hand.size ())
         return i;
   } while (i != player);

   return -1;
}

//-----------------------------------------------------------------------------
/// Finds the next player which can continue according to the bfPlayers
/// bitfield
/// \param player: Number of player to start with
/// \returns int: Number of next player (or -1)
//-----------------------------------------------------------------------------
int Twopart::findNextPlayer (unsigned int player) const {
   if (!bfPlayers)                                // No players left: Return -1
      return -1;

   // Find first player (starting with the passed one) being still in game
   do {
      player = (player + 1) & 0x3;
   } while (!(bfPlayers & (1 << player)));

   return player;
}

//-----------------------------------------------------------------------------
/// Removes all players having no cards left from the player-bitfield
/// \returns Number of players left
//-----------------------------------------------------------------------------
unsigned int Twopart::removePlayersWithoutCards () {
   unsigned int cPlayers (0);
   for (unsigned int i (0); i < NUM_PLAYERS; ++i)
      if (players[i].hand.size ())
         ++cPlayers;
      else
         removePlayer (i);

   return cPlayers;
}

//-----------------------------------------------------------------------------
/// Converts a position in the played staple into the number of the player
/// \param pos: Position to convert
/// \param start: First player of round
/// \returns unsigned int: Number of player
//-----------------------------------------------------------------------------
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

//-----------------------------------------------------------------------------
/// Method to move the cards of the actual round to the winner
/// \param receiver: Nr. of player getting all played cards
/// \param start: Startposition of cards to move
//-----------------------------------------------------------------------------
void Twopart::movePlayedCardsToPlayer (unsigned int receiver, unsigned int start) {
   TRACE8 ("Twopart::movePlayedCardsToPlayer () - " << played.size ()
           << " cards for player " << receiver << " til position " << start);
   Check3 (receiver < NUM_PLAYERS);
   Check3 (start < played.size ());

   if ((gameStatus () == PLAYING2)
       && (getConnectionMgr ().getMode () == ConnectionMgr::SERVER)) {
      std::ostringstream msg;
      msg << "Play=" << played[start]->id () << ";Target=1";
      broadcastMessage (msg.str ());
   }
   movePile (((gameStatus () == PLAYING)
              ? players[receiver].won : players[receiver].hand),
             played, start);

   if (!receiver && (gameStatus () == PLAYING))
      enableWonCards (players[0].won);

   if (gameStatus () == PLAYING2)
      players[receiver].hand.sort (compByColourAccTrumps);
}

//-----------------------------------------------------------------------------
/// Remove cards from everything which can hold them
//-----------------------------------------------------------------------------
void Twopart::clean () {
   staple.clear ();                                             // Clear staple
   for (int i (0); i < NUM_PLAYERS; ++i) {            // Clear cards of players
      players[i].hand.clear ();
      players[i].won.clear ();
   }
   played.clear ();

   disableHuman ();
   staple.show ();
   if (pTrump) {
      delete pTrump;
      pTrump = NULL;
   }
   Game::clean ();
}

//-----------------------------------------------------------------------------
/// Starts part two of the game
/// \param player: Player starting part II
/// \returns int: Value indicating if timer should continue
//-----------------------------------------------------------------------------
bool Twopart::startPartTwoTimerFnc (unsigned int player) {
   TRACE8 ("Twopart::startPartTwoTimerFnc ()");
   TRACE9 ("Twopart::startPartTwoTimerFnc (unsigned int) - Continuing with " << player);
   Check3 (!bfPlayers);
   setGameStatus (PLAYING2);

   disableWonCards ();

   // Prepare array for sorting according to trumps
   Check3 (pTrump);
   for (unsigned int i (0); i < 4; ++i)
      sortOrder[i] = (i - pTrump->colour () + 3) & 0x3;
   Check3 (sortOrder[pTrump->colour ()] == 3);

   startPlayer = -1U;

   unsigned int nrPlayers (0);
   // Check if there are players without cards
   for (unsigned int i (0); i < NUM_PLAYERS; ++i)
      if (!players[i].won.size ()) {
         TRACE5 ("Twopart::startPartTwoTimerFnc (unsigned int) - Player "
                 << i << " has no cards");
         bfPlayers |= 1 << i;
         ++nrPlayers;
      }

   // Now move the cards from the played pile to the hand; if there are
   // players without cards give them the cards up to 5
   unsigned int victim (player);
   for (unsigned int i (0); i < NUM_PLAYERS; ++i) {
      for (unsigned int j (players[i].won.size ()); j; --j) {
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
      players[i].hand.sort (compByColourAccTrumps);
      players[i].hand.setStyle (i ? ICardPile::VERY_COMPRESSED : ICardPile::COMPRESSED);
   }
   
   bfPlayers = (1 << NUM_PLAYERS) - 1;
   pos1Play = pos2Play = -1U;

   setNextPlayer (player);
   makeNextMoves ();
   return false;
}

//-----------------------------------------------------------------------------
/// Compares the cards in the pile with regard of the colour and with special
/// consideration of trumps
/// \param a: Card to compare
/// \param b: Card to compare
/// \returns bool: True, if a < b
//-----------------------------------------------------------------------------
bool Twopart::compByColourAccTrumps (const CardWidget* a, const CardWidget* b) {
   Check3 (a); Check3 (b);

   TRACE9 ("Twopart::compByColourAccTrumps (const CardWidget*, const CardWidget*) - "
           << *a << " < " << *b << " = "
           << ((a->colour () == b->colour ()) ? a->number () < b->number ()
               : (sortOrder[a->colour ()] <sortOrder[b->colour ()])));
   return ((a->colour () == b->colour ())
           ? a->number () < b->number ()
           : (sortOrder[a->colour ()] < sortOrder[b->colour ()]));
}

//-----------------------------------------------------------------------------
/// Starts part two of the game
/// \param player: Player to start part II
//-----------------------------------------------------------------------------
void Twopart::startPartTwo (unsigned int player) {
   TRACE9 ("Twopart::startPartTwo (unsigned int) - Continuing with " << player);

   Glib::signal_timeout ().connect
      (bind (slot (*this, &Twopart::startPartTwoTimerFnc),
             player), 50);
   disableHuman ();
}

//-----------------------------------------------------------------------------
/// Shows or hides the cards of the computer player
/// \param open: Flag if cards should be shown or hidden
//-----------------------------------------------------------------------------
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

//-----------------------------------------------------------------------------
/// Finds a bigger card, with respect to trumps
/// \param pile: Pile to analyze
/// \param nr: Number of card to beat
/// \returns int: Pos to play (or -1, if no card is bigger)
//-----------------------------------------------------------------------------
int Twopart::findBigger (const ICardPile& pile, CardWidget::NUMBERS nr) const {
   // Find first bigger (or equal) card without checking for trumps
   int pos (pile.findFirstEqualOrBigger (nr));

   // Now check if there's a bigger trump
   if (pTrump && (pos != -1)) {
      unsigned int newPos (pos);
      while (++newPos < pile.size ())
         if (pile[newPos]->colour () == pTrump->colour ()) {
            pos = newPos;
            break;
         }
   }

   TRACE3 ("Twopart::findBigger (const ICardPile&, CardWidget::NUMBERS) const - Pos "
           << pos);
   return pos;
}

//-----------------------------------------------------------------------------
/// Changes the names of the playing people
/// \param newPlayer: Array holding the new player
//-----------------------------------------------------------------------------
void Twopart::changeNames (const std::vector<Player*>& newPlayer) {
   Game::changeNames (newPlayer);

   for (int i (0); i < NUM_PLAYERS; ++i)
      players[i].name.set_text (actPlayers[i]->getName ());
}

//----------------------------------------------------------------------------
/// Changes the names of the playing people
/// \param newPlayer: Array holding the new player
/// \param pile: ID of the pile to return
//----------------------------------------------------------------------------
ICardPile& Twopart::getPileOfPlayer (unsigned int player, unsigned int pile) {
   Check1 (player < NUM_PLAYERS);
   Check1 (!pile);
   return pile ? played : players[player].hand;
}

//----------------------------------------------------------------------------
/// Handles the messages the server might send for the twopart cardgame
/// \param player: ID of player sending the message
/// \param message: Message received from the server
//----------------------------------------------------------------------------
void Twopart::handleMessage (unsigned int player, const char* message) {
   TRACE1 ("Twopart::handleMessage (unsigned int player, const char*) - "
           << message << " (" << player << ')');
    
   Tokenize command (message);
   std::string cmd (command.getNextNode ('='));

   Game::handleMessage (player, message);
   if (cmd == "ActPlayer") {
      TRACE1 ("Twopart::handleMessage (unsigned int player, const char*) - "
              "Next player: " << currentPlayer ());
      startPlayer = currentPlayer ();
      makeNextMoves ();
   }
}

//----------------------------------------------------------------------------
/// Executes the remote move locally
/// \param pile: Pile to move to/from
/// \param card: Card which to use from pile
//----------------------------------------------------------------------------
bool Twopart::executeRemoteMove (ICardPile& pile, unsigned int card) {
   if (&pile == &played) {
      TRACE7 ("Twopart::executeRemoteMove (ICardPile&, unsigned int) - Card "
              << card << "; ("
              << (getConnectionMgr ().getMode () == ConnectionMgr::SERVER) ? "S)" : "C)");
      Check3 (gameStatus () == PLAYING2);
      pos1Play = pos2Play = -1U;
      setNextPlayer (pickUpPlayedPile (currentPlayer ()));
      return false;
   }
   else
      Game::executeRemoteMove (pile, card);
      if (pos2Play != pos1Play)
         pile.resize (pos2Play - 1, ICardPile::COMPRESSED);
      return true;
}
