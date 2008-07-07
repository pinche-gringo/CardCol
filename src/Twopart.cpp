//$Id$

//PROJECT     : Cardgames
//SUBSYSTEM   : Twopart
//REFERENCES  :
//TODO        :
//BUGS        :
//REVISION    : $Revision$
//AUTHOR      : Markus Schwab
//CREATED     : 20.7.2002
//COPYRIGHT   : Copyright (C) 2002 - 2008

// This file is part of CardCol.
//
// CardCol is free software: you can redistribute it and/or modify
// it under the terms of the GNU General Public License as published by
// the Free Software Foundation, either version 3 of the License, or
// (at your option) any later version.
//
// CardCol is distributed in the hope that it will be useful,
// but WITHOUT ANY WARRANTY; without even the implied warranty of
// MERCHANTABILITY or FITNESS FOR A PARTICULAR PURPOSE.  See the
// GNU General Public License for more details.
//
// You should have received a copy of the GNU General Public License
// along with CardCol.  If not, see <http://www.gnu.org/licenses/>.


#include <cardgames-cfg.h>

#include <ctime>
#include <cstdlib>

#include <sstream>

#include <glibmm/main.h>

#include <gtkmm/box.h>
#include <gtkmm/stock.h>
#include <gtkmm/statusbar.h>
#include <gtkmm/messagedialog.h>

#define CHECK 9
#define TRACELEVEL 9
#include <YGP/Check.h>
#include <YGP/Trace.h>
#include <YGP/ConnMgr.h>
#include <YGP/Tokenize.h>

#include <Player.h>
#include <CardSet.h>
#include <CardImgs.h>
#include <CardWidget.h>
#include <CardWindow.h>
#include <ComputerPlayer.h>

#include "Twopart.h"


const unsigned int Twopart::COLS_PLAYER[NUM_PLAYERS] = { 1, 13, 7, 1 };
const unsigned int Twopart::ROWS_PLAYER[NUM_PLAYERS] = { 10,  8, 4, 8 };

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
                  unsigned int posPlayer, YGP::Mutex& mxSerialize)
   : Game (parent, statusbar, cardset, player, posPlayer, mxSerialize, 12, 15)
     , bfPlayers ((1 << NUM_PLAYERS) - 1), offPos (0)
     , bfOldPlayers (bfPlayers), pTrump (NULL)
     , played (ICardPile::COMPRESSED, ICardPile::SHOWFACE)
     , staple (ICardPile::VERY_COMPRESSED, ICardPile::SHOWBACK) {
   staple.show ();
   attach (staple, 2, 3, 2, 3, Gtk::SHRINK, Gtk::SHRINK, 5, 5);

   // Show and attach card-piles
   changeNames (player);
   for (unsigned int i (0); i < NUM_PLAYERS; ++i) {
      players[i].name.show ();
      attach (players[i].name, COLS_PLAYER[i], COLS_PLAYER[i] + (i ? 3 : 15),
              ROWS_PLAYER[i] + (i ? 1 : 3),
              ROWS_PLAYER[i] + (i ? 2 : 4),
              Gtk::EXPAND, Gtk::EXPAND, 1);
      TRACE9 ("Twopart::Twopart () - Name at: " << COLS_PLAYER[i] << '/'
              << ROWS_PLAYER[i] + (i ? 1 : 3));

      players[i].won.show ();
      attach (players[i].won, COLS_PLAYER[i] + (i ? 1 : 3),
              COLS_PLAYER[i] + (i ? 3 : 15),
              ROWS_PLAYER[i] + (i ? -2 : 2),
              ROWS_PLAYER[i] + (i ? -1 : 3),
              Gtk::SHRINK, Gtk::SHRINK, 1);
      TRACE9 ("Twopart::Twopart () - Won pile at: "
              << COLS_PLAYER[i] + 1 << '/' << ROWS_PLAYER[i] + (i ? -2 : 2));

      players[i].hand.show ();
      attach (players[i].hand, COLS_PLAYER[i],
              COLS_PLAYER[i] + (i ? 3 : 15), ROWS_PLAYER[i],
              ROWS_PLAYER[i] + 1, Gtk::SHRINK, Gtk::SHRINK, 1);
      TRACE9 ("Twopart::Twopart () - Hand at: "
              << COLS_PLAYER[i] << '/' << ROWS_PLAYER[i]);

      players[i].won.setShowOption (ICardPile::SHOWBACK);
      players[i].hand.setShowOption (i ? ICardPile::SHOWBACK : ICardPile::SHOWFACE);
   }

   played.show ();
   attach (played, 3, 11, 6, 9, Gtk::SHRINK, Gtk::SHRINK, 0, 5);
   resizeCards ();
}

//-----------------------------------------------------------------------------
/// Destructor
//-----------------------------------------------------------------------------
Twopart::~Twopart () {
   TRACE9 ("Twopart::~Twopart ()");
   clean ();
}

//-----------------------------------------------------------------------------
/// Starts the game
//-----------------------------------------------------------------------------
void Twopart::start () {
   TRACE8 ("Twopart::start ()");
   Game::start ();
   if (randomizeCardsToPile (staple)) {
      // Show cards on the table: For all players put 3 cards in hand
      for (unsigned int i (0); i < NUM_PLAYERS; ++i)
         for (unsigned int j (0); j < 3; ++j)
            players[(i - posServer) % NUM_PLAYERS].hand.insertSorted (staple.removeTopCard ());

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

      if (getConnectionMgr ().getMode () != YGP::ConnectionMgr::CLIENT) {
         setNextPlayer (startPlayer = rand () % NUM_PLAYERS);
	 broadcastStartPlayer (startPlayer);
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

   TRACE2 ("Twopart::enableHuman () - Has " << players[0].hand.size () << " cards");
   for (int i (players[0].hand.size () - 1); i >= 0; --i)
      activeCards.push_back
         (players[0].hand[i]->signal_clicked ().connect
           (bind (mem_fun (*this, (&Twopart::cardSelected)), i)));

   if ((gameStatus () == PLAYING2) && (played.size ()))
      activeCards.push_back
         (played.getTopCard ().signal_clicked ().connect
          (mem_fun (*this, (&Twopart::playedSelected))));

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
           << " picks up played pile at " << offPos);
   Check3 (bfPlayers);

   // Move played cards to player
   Check3 (offPos > 0); Check3 (offPos < NUM_PLAYERS);
   CardPileWindow* anim;
   if (gameStatus () == PLAYING2)
      anim = &animateCards (players[player].hand, played, startPos[--offPos], played.size () - 1);
   else
      anim = &animateCards (players[player].won, played, 0, played.size () - 1);
   anim->sigAnimation.connect (bind (mem_fun (*this, &Twopart::endPickup), player));
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
      if ((!(bfPlayers & (1 << (next = (player + i + 1) % NUM_PLAYERS))))
          && players[next].hand.size ()) {
         TRACE5 ("Twopart::pickUpPlayedPile (unsigned int) - Re-adding player "
                 << next);
         addPlayer (next);
         --num;
      }
   }

   Glib::ustring stat ( _("%1 can't continue -> Picking up last cards; "));
   Check3 (actPlayers.size () > player);
   Check3 (actPlayers[player]);
   stat.replace (stat.find ("%1"), 2, actPlayers[player]->getName ());

   player = findNextPlayer (player);
   displayTurn (player, stat);

   return player;
}

//-----------------------------------------------------------------------------
/// Callback after clicking the top card of the played pile
/// \pre Call only in part 2 of the game
//-----------------------------------------------------------------------------
void Twopart::playedSelected () {
   TRACE3 ("Twopart::playedSelected (unsigned int) - Human picks up played pile");
   Check3 (gameStatus () == PLAYING2);
   Check3 (bfPlayers);

   if (getConnectionMgr ().getMode () != YGP::ConnectionMgr::NONE) {
      Check3 (startPos[offPos - 1] < played.size ());
      std::ostringstream msg;
      msg << "Play=" << played[startPos[offPos - 1]]->id () << ";Target=1";
      if (getConnectionMgr ().getMode () == YGP::ConnectionMgr::CLIENT)
         ignoreNextMsg = true;
      broadcastMessage (msg.str ());
   }
   setNextPlayer (pickUpPlayedPile (0));
   disableHuman ();
}

//-----------------------------------------------------------------------------
/// Callback after clicking on a card in hand
/// \param pos: Offset of card in hand
//-----------------------------------------------------------------------------
void Twopart::cardSelected (unsigned int pos) {
   TRACE5 ("Twopart::cardSelected (unsigned int) - Position " << pos);
   Check3 (pos < players[0].hand.size ());
   Check3 (gameStatus () >= PLAYING);

   // Perform validity-check in part 2: Card must have the same colour and be
   // bigger than the last played card or be a (bigger) trump
   unsigned int start (pos);
   if (gameStatus () == PLAYING2) {
      CardWidget& card (*players[0].hand[pos]);
      CardWidget::NUMBERS nr (card.number ());
      CardWidget::COLOURS colour (card.colour ());

      start = findStartOfSerie (0, pos);
      Check3 (pTrump);
      if (played.size ()) {
         CardWidget& top (played.getTopCard ());

         if ((colour == pTrump->colour ())
             ? ((top.colour () == pTrump->colour ())
                && (top.number () >= nr))
             : ((top.colour () != colour)
                || (top.number () >= nr))) {
            Gtk::MessageDialog dlg
	       (ngettext ("The played card must have the same colour and must be bigger (or be a trump)!",
			  "The played cards must have the same colour and must be bigger (or be trumps)!",
			  pos - start + 1),
		Gtk::MESSAGE_ERROR);
            dlg.set_title (PACKAGE " - Twopart");
            dlg.run ();
            return;
         }
      }
      animateCards (played, players[0].hand, start, pos)
	 .sigAnimation.connect (bind (mem_fun (*this, &Twopart::endTurn), 0));
   }
   else
      animateCard (played, players[0].hand, pos)
	 .sigAnimation.connect (bind (mem_fun (*this, &Twopart::endTurn), 0));

   // Inform the others about the move
   if (getConnectionMgr ().getMode () != YGP::ConnectionMgr::NONE) {
      // Send played card to all clients (if any)
      std::ostringstream msg;
      msg << "Play=";
      for (unsigned int i (start); i < pos; ++i)
         msg << players[0].hand[i]->id () << ' ';
      msg << players[0].hand[pos]->id () << ";Target=0";

      if (getConnectionMgr ().getMode () == YGP::ConnectionMgr::CLIENT)
         ignoreNextMsg = true;
      broadcastMessage (msg.str ());
   }
}

//-----------------------------------------------------------------------------
/// Finish of a turn (in part 2), called after the card has been animated
/// \param player: ID of player
/// \param start: Offset of first card to play
/// \param end: Offset of last card to play
/// \returns int: Next player or -1 at end
//-----------------------------------------------------------------------------
void Twopart::endTurn (unsigned int player) {
   TRACE5 ("Twopart::endTurn (unsigned int) - Player: " << player);
   Check3 (player < NUM_PLAYERS);

   if (staple.size ()) {
      CardWidget& card (staple.removeShownTopCard ());
      players[player].hand.insertSorted (card);

      if (!staple.size ()) {
	 Check3 (!pTrump);
	 pTrump = new CardWidget (card); Check3 (pTrump);
	 staple.hide ();
      }
   }

   bool isAnimated (false);
   // Check if every player is still in game or has already played; end round
   // if so or calculate next player if not
   TRACE7 ("Twopart::endTurn (unsigned int) - Players: " << std::hex << bfPlayers << std::dec);
   removePlayer (player);
   unsigned int newPlayer (player);
   if (bfPlayers)
      newPlayer = (unsigned int)findNextPlayer (player);
   else {
      // Show trump if not already visible
      if (pTrump && !pTrump->is_visible ()) {
         pTrump->showFace ();
         pTrump->show ();
         attach (*pTrump, 2, 3, 2, 3, Gtk::SHRINK, Gtk::SHRINK, 5, 5);
      }
      newPlayer = player;
      isAnimated = endRound (newPlayer);
   }

   // Check if the current part is terminated
   if ((gameStatus () == PLAYING)
       ? ((int)newPlayer < 0)
       : (newPlayer == (unsigned int)findNextPlayerWithCards (newPlayer))) {
      Glib::ustring str;
      if (gameStatus () == PLAYING) {
	 player = ~newPlayer;
         startPartTwo (player);
	 str = _("First part ended; Part 2 starts %1");
      }
      else {
	 player = newPlayer;
         setGameStatus (STOPPED);
	 str = _("%1 lost");
      }
      Check3 (actPlayers.size () > player);
      Check3 (actPlayers[player]);
      str.replace (str.find ("%1"), 2, actPlayers[player]->getName ());
      status.pop ();
      status.push (str);
   }
   else {
      if (gameStatus () == PLAYING2)
	 ++offPos;

      displayTurn (newPlayer);
      setNextPlayer (newPlayer);
      if (!isAnimated)
	 makeNextMoves ();
   }
   return;
}

//-----------------------------------------------------------------------------
/// Makes the move for the next player.
/// \param player: Actual player
//-----------------------------------------------------------------------------
void Twopart::makeMove (unsigned int player) {
   TRACE5 ("Twopart::makeMove () - Turn of player " << player);
   Check3 (gameStatus () >= PLAYING);

   unsigned int pos1Play, pos2Play;
   if (findPos2Play (player, pos1Play, pos2Play) != -1) {
      // Flip card(s) to play
      flipCards2Play (players[player].hand, pos1Play, pos2Play);
      animateCards (played, players[player].hand, pos1Play, pos2Play)
	 .sigAnimation.connect (bind (mem_fun (*this, &Twopart::endTurn), player));
   }
   else {
      if ((gameStatus () == PLAYING2)
	  && (getConnectionMgr ().getMode () == YGP::ConnectionMgr::SERVER)) {
	 std::ostringstream msg;
	 Check3 (startPos[offPos - 1] < played.size ());
	 msg << "Play=" << played[startPos[offPos - 1]]->id () << ";Target=1";
	 broadcastMessage (msg.str ());
      }
      setNextPlayer (pickUpPlayedPile (player));
   }
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
         if ((((start != end)
	       || (cHigh > 1) || (points >= CardWidget::TEN))
	      && (posMaxEqual == -1)
	      && ((!(bfPlayers & ~(1 << player)))
		  && trumps
		  && ((end = (findBigger
			      (players[player].hand,
			       static_cast<CardWidget::NUMBERS> (maxNr))))
		      != -1U)))
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
                                   | (1 << findNextPlayer ((player + 1) % NUM_PLAYERS))))
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

   CardWidget::NUMBERS nrMin (CardWidget::UNREACHABLE);
   unsigned int cSerie (0);
   unsigned int pos (0);
   for (unsigned int i (0); i < players[player].hand.size (); ++i) {
      CardWidget& card (*players[player].hand[i]);

      // Stop searching if a trump was found
      if ((card.colour () == pTrump->colour ()) && i)
         break;

      if (nrMin >= card.number ()) {
         TRACE8 ("Twopart::findSmallestCard (unsigned int) - New smallest card at "
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
      TRACE8 ("Twopart::findEndOfSerie (unsigned int, unsigned int) - Next valid card "
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
      TRACE8 ("Twopart::findStartOfSerie (unsigned int, unsigned int) - Next valid card "
              << *card << " at " << start);
      nr = card->number ();
   }

   return start + 1;
}

//-----------------------------------------------------------------------------
/// Checks if there is a winner for the round and moves played cards to him if
/// so. Else enable the players which can continue
/// \param player: Actual player (ending the round)
/// \returns bool: True, if an animation has been started by this method
//-----------------------------------------------------------------------------
bool Twopart::endRound (unsigned int& player) {
   TRACE8 ("Twopart::endRound (unsigned int&)");
   Check3 (!bfPlayers);
   bool rc (false);

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

      TRACE8 ("Twopart::endRound (unsigned int&) - Round has " << cPlayers
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

      TRACE4 ("Twopart::endRound (unsigned int&) - Player starting round: "
              << startPlayer << "; players: " << cPlayers);
      Check3 ((*startPos + cPlayers) == played.size ());

      // Equal cards found
      if (posMaxEqual >= 0) {
         unsigned int bfPlayersOut (0);
         cPlayers = 0;

         // Add players having equal cards and having still cards left
         nextPlayer = pos2Player (posMaxEqual - *startPos);
         for (unsigned int i (*startPos); i < played.size (); ++i) {
            if ((played[i]->number () == maxEqualNr)
                && players[pos2Player (i - *startPos)].hand.size ()) {
               TRACE5 ("Twopart::endRound (unsigned int&) - Found equal cards; Player "
                       << pos2Player (i - *startPos)
                       << (cPlayers ? " still in round" : " is winner"));
               ++cPlayers;
            }
            else
               bfPlayersOut |= (1 << pos2Player (i - *startPos));
         } // endfor check for equal cards
         bfPlayers &= ~bfPlayersOut;
         TRACE5 ("Twopart::endRound (unsigned int&) - Found equal cards; "
                 << cPlayers << " player(s) still in round (" << std::hex
                 << bfPlayers << std::dec << ')');

         TRACE6 ("Twopart::endRound (unsigned int&) - Try to continue with player " << nextPlayer);
         if (players[nextPlayer].hand.empty () && cPlayers)
            nextPlayer = findNextPlayer (nextPlayer);
         TRACE8 ("Twopart::endRound (unsigned int&) - Remaining (" << cPlayers
                 << ") " << std::hex << bfPlayers << std::dec << "; Next: " << nextPlayer);

	 if (cPlayers < 2) {                   // Less than two players found:
            bfPlayers = (1 << NUM_PLAYERS) - 1;
            cPlayers = removePlayersWithoutCards ();

	    setNextPlayer (NUM_PLAYERS - 1);
	    animateCards (players[nextPlayer].won, played, 0, played.size () - 1)
	       .sigAnimation.connect (bind (mem_fun (*this, &Twopart::endPickup), nextPlayer));
	    rc = true;
            if (!cPlayers)
               nextPlayer = ~nextPlayer;
            else
               if (players[nextPlayer].hand.empty ())
                  nextPlayer = findNextPlayer (nextPlayer);
         }
      }
      // All played cards are differnt: Winner is the one with highest card
      else {
	 setNextPlayer (NUM_PLAYERS - 1);
	 startPlayer = nextPlayer = pos2Player (posMax - *startPos);
	 animateCards (players[nextPlayer].won, played, 0, played.size () - 1)
	    .sigAnimation.connect (bind (mem_fun (*this, &Twopart::endPickup), nextPlayer));
	 rc = true;
         TRACE5 ("Twopart::endRound (unsigned int&) - Found winner: " << nextPlayer);

         bfPlayers = (1 << NUM_PLAYERS) - 1;   // Set all players (having cards)
         removePlayersWithoutCards ();
         bfOldPlayers = bfPlayers;

         if (players[nextPlayer].hand.empty ())
            nextPlayer = findNextPlayer (nextPlayer);
         if (nextPlayer == -1U)
            nextPlayer = ~startPlayer;
      }
      *startPos = played.size ();
   }

   bfOldPlayers = bfPlayers;
   player = startPlayer = nextPlayer;
   TRACE8 ("Twopart::endRound (unsigned int&) - Continuing with player " << nextPlayer);
   return rc;
}

//-----------------------------------------------------------------------------
/// Analyzes the played staple and retrieves the highest card(s)
/// \param startPos: Position from where to start analyzing
/// \param max: Highest single card
/// \param maxPos: Position of highest single card
/// \param maxEqual: Highest pair
/// \param maxEqualPos: Position of highest equal card
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
      Check3 (maxPos < (int)played.size ());
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
      i = (i + 1) % NUM_PLAYERS;
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
      player = (player + 1) % NUM_PLAYERS;
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
/// Callback after animating the played cards to a player
/// \param receiver: Nr. of player getting all played cards
//-----------------------------------------------------------------------------
void Twopart::endPickup (unsigned int receiver) {
   TRACE8 ("Twopart::endPickup () - " << receiver);
   Check3 (receiver < NUM_PLAYERS);
   if (!receiver && (gameStatus () == PLAYING))
      enableWonCards (players[0].won);

   if (gameStatus () == PLAYING2)
      players[receiver].hand.sort (compByColourAccTrumps);
   else
      *startPos = 0;
   makeNextMoves ();
}

//-----------------------------------------------------------------------------
/// Remove cards from everything which can hold them
//-----------------------------------------------------------------------------
void Twopart::clean () {
   staple.clear ();                                             // Clear staple
   for (unsigned int i (0); i < NUM_PLAYERS; ++i) {   // Clear cards of players
      players[i].hand.clear ();
      players[i].won.clear ();
   }
   played.clear ();

   disableHuman ();
   staple.show ();
   if (pTrump) {
      remove (*pTrump);
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
   TRACE9 ("Twopart::startPartTwoTimerFnc (unsigned int) - Continuing with " << player);
   Check3 (!bfPlayers);
   setGameStatus (PLAYING2);

   disableWonCards ();

   // Prepare array for sorting according to trumps
   Check3 (pTrump);
   for (unsigned int i (0); i < 4; ++i)
      sortOrder[i] = (i - pTrump->colour () + 3) % NUM_PLAYERS;
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
         TRACE8 ("Twopart::startPartTwoTimerFnc (unsigned int) - Moving cards "
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
   setNextPlayer (player);
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
   TRACE8 ("Twopart::startPartTwo (unsigned int) - Continuing with " << player);

   Glib::signal_timeout ().connect
      (bind (mem_fun (*this, &Twopart::startPartTwoTimerFnc),
             player), 50);
   disableHuman ();
}

//-----------------------------------------------------------------------------
/// Shows or hides the cards of the computer player
/// \param open: Flag if cards should be shown or hidden
//-----------------------------------------------------------------------------
void Twopart::playOpen (bool open) {
   ICardPile::ShowOpt show (open ? ICardPile::SHOWFACE : ICardPile::SHOWBACK);

   for (unsigned int i (0); i < NUM_PLAYERS; ++i) {
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

   for (unsigned int i (0); i < NUM_PLAYERS; ++i)
      players[i].name.set_text (actPlayers[i]->getName ());
}

//----------------------------------------------------------------------------
/// Converts a pile-number to the actual pile
/// \param newPlayer: Array holding the new player
/// \param pile: ID of the pile to return
/// \returns ICardPile*: Pile corresponding to the passed number or NULL
//----------------------------------------------------------------------------
ICardPile* Twopart::getPileOfPlayer (unsigned int player, unsigned int pile) {
   TRACE8 ("Twopart::getPileOfPlayer (unsigned int, unsigned int) - Player "
           << player << "; Pile " << pile);
   if ((player >= NUM_PLAYERS) || (pile > 1))
      return NULL;

   return &(pile ? played : players[player].hand);
}

//----------------------------------------------------------------------------
/// Handles the messages the server might send for the Twopart cardgame
/// \param player: ID of player sending the message
/// \param message: Message received from the server
/// \returns bool: True, if message has been completey processed
/// \throw YGP::ParseError, YGP::CommError: In case of an error an describing text
//----------------------------------------------------------------------------
bool Twopart::handleMessage (unsigned int player, const std::string& message) throw (YGP::ParseError, YGP::CommError) {
   TRACE1 ("Twopart::handleMessage (unsigned int player, const std::string&) - "
           << message << " (" << player << ')');

   YGP::Tokenize command (message);
   std::string cmd (command.getNextNode ('='));

   bool rc (Game::handleMessage (player, message));
   if (cmd == "ActPlayer") {
      TRACE1 ("Twopart::handleMessage (unsigned int player, const std::string&) - "
              "Next player: " << currentPlayer ());
      startPlayer = currentPlayer ();
      makeNextMoves ();
      return true;
   }
   return rc;
}

//----------------------------------------------------------------------------
/// Executes the remote move locally
/// \param pile: Pile to move to/from
/// \param target: ID of target as send by the partner
/// \pre Expects \c pos1Play and \c pos2Play to be set to the positions to play
/// \throw YGP::ParseError: In case of an error
//----------------------------------------------------------------------------
bool Twopart::executeRemoteMove (ICardPile& pile, unsigned int target) throw (YGP::ParseError) {
   Check2 (pos1Play != -1U);
   Check2 (pos2Play != -1U);
   if (target) {
      TRACE7 ("Twopart::executeRemoteMove (ICardPile&, unsigned int) - Target " << target);
      Check3 (gameStatus () == PLAYING2);
      pos1Play = pos2Play = -1U;
      setNextPlayer (pickUpPlayedPile (currentPlayer ()));
      return false;
   }
   return Game::executeRemoteMove (pile, target);
}

//-----------------------------------------------------------------------------
/// Adds game-specific menus
/// \param mgrUI: UIManager to add to
//-----------------------------------------------------------------------------
void Twopart::addMenus (Glib::RefPtr<Gtk::UIManager> mgrUI) {
   Check1 (mgrUI);
   Glib::ustring ui ("<menubar name='Menu'>"
		     "  <placeholder name='GameMenu'>"
		     "    <menu action='MB'>"
		     "      <menuitem action='TwopartSort'/>"
		     "      <menuitem action='TwopartSortCol'/>"
		     "    </menu></placeholder></menubar>");

   Glib::RefPtr<Gtk::ActionGroup> grpAction (Gtk::ActionGroup::create ());
   grpAction->add (Gtk::Action::create ("MB", _("_Twopart")));
   grpAction->add (Gtk::Action::create ("TwopartSort", Gtk::Stock::SORT_ASCENDING,
					_("_Sort won cards (by number)")),
		   Gtk::AccelKey ("<shft>S"),
		   mem_fun (*this, &Twopart::sortWonByNumber));
   grpAction->add (Gtk::Action::create ("TwopartSortCol", Gtk::Stock::SORT_ASCENDING,
					_("Sort won cards (by _colour)")),
		   Gtk::AccelKey ("S"),
		   mem_fun (*this, &Twopart::sortWonByColour));

   mgrUI->insert_action_group (grpAction);
   idMrg = mgrUI->add_ui_from_string (ui);
}

//-----------------------------------------------------------------------------
/// Removes the game-specific menus
/// \param mgrUI: UIManager to remove from
//-----------------------------------------------------------------------------
void Twopart::removeMenus (Glib::RefPtr<Gtk::UIManager> mgrUI) {
   Check1 (mgrUI);
   mgrUI->remove_ui (idMrg);
}

//-----------------------------------------------------------------------------
/// Actions to take when the cards are resized
/// \pre The cardsize must be set in CardImages::WIDTH/HEIGHT
//-----------------------------------------------------------------------------
void Twopart::resizeCards () {
   played.set_size_request (CardImages::WIDTH + 150, CardImages::HEIGHT);
   staple.set_size_request (CardImages::WIDTH, CardImages::HEIGHT);
   for (unsigned int i (0); i < NUM_PLAYERS; ++i) {
      players[i].won.set_size_request (CardImages::WIDTH + 20, CardImages::HEIGHT + 5);
      players[i].hand.set_size_request (i ? (CardImages::WIDTH + (2 * 7)) : (CardImages::WIDTH * 3), CardImages::HEIGHT + 5);
   }
}
