//$Id$

//PROJECT     : Cardgames
//SUBSYSTEM   : Buraco
//REFERENCES  :
//TODO        :
//BUGS        :
//REVISION    : $Revision$
//AUTHOR      : Markus Schwab
//CREATED     : 24.02.2003
//COPYRIGHT   : Anticopyright (A) 2003

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

#include <bitset>

#include <gtk/gtkdnd.h>

#include <gtkmm/messagedialog.h>

#include <Check.h>
#include <Trace_.h>
#include <ANumeric.h>
#include <ScoreDlg.h>



#include "SigCExt.h"
std::vector<Gtk::TargetEntry> Buraco::dndType;


unsigned int Buraco::ENDPOINTS (2000);

/// \param parent: Parent widget to display the game in
/// \param statusbar: Status bar widget to display information about the game
/// \param cardset: Cardset to use
/// \param player: Vector of player
/// \param posPlayer: Position of player for the server
/// \param names: Vector of player-names
                CardSet& cardset, const std::vector<Player*>& player,
                unsigned int posPlayer, YGP::Mutex& mxSerialize)
                  CardSet& cardset, const std::vector<std::string>& names)
   : Game (parent, statusbar, cardset, names, 3, 10), startPlayer (0)
     , acceptCards (-1U), target (-1U) , pScoreDlg (NULL) {
   TRACE9 ("Buraco::Buraco (Box&, Statusbar&, CardSet&, const "
     , newPile (_("New pile")), target (-1U), pos1 (0), pos2 (0)
     , pScoreDlg (NULL) {

           "std::vector<std::string>&)");
       scrlTable[i] = new Gtk::ScrolledWindow ();

   TRACE9 ("Buraco::Buraco (Box&, Statusbar&, CardSet&, const "
           "std::vector<Glib::ustring>&) - Init common staples");
   staple.set_size_request (width, height);
           "std::vector<std::string>&) - Init common staples");

   boxTeam[0].pack_end (newPile, Gtk::PACK_EXPAND_WIDGET, 5);
   boxTeam[0].set_size_request (-1, height + 5 * 15);
   boxTeam[1].set_size_request (-1, height + 5 * 15);
   boxTeam[0].set_size_request (width, height + 5 * 15);
   boxTeam[1].set_size_request (width, height + 5 * 15);
      attach (hands[i], (i << 2) - 4, (i << 2) - 2, 4, 5,
              Gtk::EXPAND, Gtk::SHRINK, 5, 5);
      attach (hands[i], 0, 10, 3, 4, Gtk::EXPAND, Gtk::SHRINK, 0);
      hands[i].setStyle (ICardPile::COMPRESSED);
      hands[i].setShowOption (ICardPile::SHOWBACK);
   hands[0].setShowOption (ICardPile::SHOWFACE);
   hands[0].show ();
   names[0].show ();
           "std::vector<Glib::ustring>&) - Attach widgets");
   attach (hands[0], 3, 10, 0, 1, Gtk::EXPAND, Gtk::SHRINK, 1, 5);
           "std::vector<std::string>&) - Attach widgets");
   attach (hands[0], 3, 10, 0, 1, Gtk::EXPAND, Gtk::SHRINK, 1);
   attach (*scrlTable[0], 0, 10, 2, 3, Gtk::EXPAND | Gtk::FILL,
           Gtk::EXPAND | Gtk::FILL, 0, 5);
   attach (boxTeam[0], 0, 10, 1, 2);
   attach (boxTeam[1], 0, 10, 2, 3);
           "std::vector<Glib::ustring>&) - Show widgets");
   newPile.show ();
           "std::vector<std::string>&) - Show widgets");
   dumped.show ();
   boxTeam[0].show ();
   boxTeam[1].show ();

   if (dndType.empty ())
      dndType.push_back
         (Gtk::TargetEntry ("icon/card", Gtk::TARGET_SAME_APP, 0));

         (Gtk::TargetEntry ("icon/card", GTK_TARGET_SAME_APP, 0));
   frameInfo.set_shadow_type (Gtk::SHADOW_IN);
   info.set_size_request (300, -1);
   statusbar.set_has_resize_grip (false);
   statusbar.pack_end (info, Gtk::PACK_SHRINK, 5);

//-----------------------------------------------------------------------------
/// Destructor
//-----------------------------------------------------------------------------
Buraco::~Buraco () {
   TRACE9 ("Buraco::~Buraco ()");
   clean ();
   delete pScoreDlg;
   // Free team names

//-----------------------------------------------------------------------------
/// Removes a cerrado from the table
/// \param team: Team to inspect
/// \remarks As every move can only make one cerrado; only the first is
///     removed.
//-----------------------------------------------------------------------------
void Buraco::cleanCerrado (unsigned int player) {
   Check1 (player < NUM_PLAYERS);

   for (std::vector<BuracoPile*>::iterator p (tablePiles[player & 1].begin ());
        p != tablePiles[player & 1].end (); ++p) {
   for (std::vector<CardVPile*>::iterator p (tablePiles[player & 1].begin ());
      if (((*p)->size () == 7) && (*p)->is_visible ()) {
         removeCerrado (player, **p);
         return;
      }
   }
}

//-----------------------------------------------------------------------------
/// Makes the move for the next player.
/// \param player: Actual player
/// \returns \c int: Next player or -1 if end of game
/// \remarks This method expects the target pile to play in the target-member
///     and the positions to play in pos1Play and pos2Play
//-----------------------------------------------------------------------------
///     and the positions to play in pos1 and pos2
   TRACE5 ("Buraco::makeMove (unsigned int) - Turn of player " << player
           << "; Target: " << std::hex << (int)target << std::dec);
   Check1 (player); Check1 (player < NUM_PLAYERS);
   Check1 (gameStatus () == PLAYING);
   Check1 (!hands[player].empty ());

   // First cleanup cerrado made in the last turn
   cleanCerrado (player);

   if (target == -1U) {
      target = showCardsToPlay (player);
      TRACE8 ("Buraco::makeMove (unsigned int) - Going to play cards to "
              << std::hex << (int)target << std::dec);
      Check3 (target != -1U);
      Check1 (pos1Play <= pos2Play);
   }
      Check1 (pos1Play <= pos2Play);

      Check1 (pos1 <= pos2);
      // to dumped staple; else target specifies offset of pile and card on
      // Calculate pile to play cards to: If target = 0xffff0000, append cards to
      // dumped staple; else target specifies offset of pile and card on table
      ICardPile* dest;
      CardHPile& source (hands[player]);
      unsigned int oldPlayer (player);

      target >>= 16;
      if (target == 0xffff) {
         TRACE4 ("Buraco::makeMove (unsigned int) - Dumping card");
         Check3 (pos1Play == pos2Play);
         pos = dumped.size ();
         Check3 (pos1 == pos2);

         gStatus.startTurn = 1;
         ++player &= 0x3;
         displayTurn (player);
      }
      else {

         // Make next player continue
         source.hide ();
         hands[player ? player : 1].show ();
         TRACE4 ("Buraco::makeMove (unsigned int) - Moving cards to pile " << target);
         Check3 (target < tablePiles[player & 1].size ());
         dest = tablePiles[player & 1][target];

#if CHECK > 2
      // Move played cards to the pile to play
      Check3 (source.size () > pos2Play);
      for (; (int)pos1Play <= (int)pos2Play; --pos2Play)
      Check3 (source.size () > pos2);
      for (; (int)pos1 <= (int)pos2; --pos2)
         dest->insert (source.remove (pos1), pos++);
      if (gStatus.pickUpPlayed) {
         Check3 (dumped.size ());
            addBuraco (oldPlayer);
         else
            addBuraco (oldPlayer, false);
               cleanCerrado (oldPlayer);
               Check3 (points[oldPlayer & 1] > 100);
               endGame ();
               cleanCerrado (oldPlayer);
               return -1;
            }
      }
   }
   return player;
}

//-----------------------------------------------------------------------------
/// Searches for cards to play and shows them in the hand of the actual player
/// \param player: Player to inspect
/// \returns \c ID of the target (32 Bit: Pile << 16 + Position)
//-----------------------------------------------------------------------------
/// \returns \c ID for target (32 Bit: Pile << 16 + Position)
   TRACE2 ("Buraco::showCardsToPlay (unsigned int) - " << player << " ("
           << gStatus.startGame << '/' << gStatus.startTurn << ')');
   TRACE2 ("Buraco::showCardsToPlay (unsigned int) - " << player);
       == (player >> 1))
      ((player & 1) ? gStatus.team2Buraco : gStatus.team1Buraco) = 0x3;

      (player & 1) ? gStatus.team2Buraco : gStatus.team1Buraco = 0x3;
   if (gStatus.startTurn) {
      gStatus.startTurn = 0;
      ICardPile& playerPile (hands[player]);
          ? (isJoker (dumpedCard)
      Check3 (dumped.size ());
      // Check if there are equal cards as the last dumped one
      int start (playerPile.find (dumped.getTopCard ().number ()));
      int end ((start == -1) ? -1 : playerPile.findLastEqual (start));
      Check3 ((start != -1) ? (start <= end) : (start == end));
      Check3 ((end == -1) || (end < playerPile.size ()));

      if ((!isJoker (dumped.getTopCard ())) && ((end - start) >= 1)
          && ((points[player & 1] > 100) || !reserve[player & 1].empty ()
              || (dumped.size () + playerPile.size () > 3))) {
         CardVPile& pile (makeNewPile (player & 1));         // Create new pile
         pile.setTopCard (dumped.removeTopCard ());      // with picked up card
         movePile (pile, playerPile, start, end);
            std::map<unsigned int, unsigned int> aPos;
         if (dumped.size ())
            movePile (playerPile, dumped);
         playerPile.sort (compByNumberWithJokers);
         if (getConnectionMgr ().getMode () != YGP::ConnectionMgr::NONE) {
      else
         playerPile.insertSorted
             (((gStatus.startGame) && isJoker (dumped.getTopCard ()))
              ? dumped.removeTopCard () : staple.removeTopCard (),
              compByNumberWithJokers);

      gStatus.startTurn = gStatus.startGame = 0;
   if (target == -1U)
      target = executeMove (player);
   return executeMove (player);
//-----------------------------------------------------------------------------
/// Executes a move for the passed player; only one move is made at every
/// timer-iteration
/// \param player: Actual player
/// \returns \c ID for target (32 Bit: Pile << 16 + Position)
//-----------------------------------------------------------------------------
int Buraco::executeMove (unsigned int player) {
   TRACE6 ("Buraco::executeMove (player) - " << player);

   TRACE6 ("Buraco::executeMove (player) - Checking for 3 in a row");

   // Check for 3 cards having the same number
   // Check if any card can be added to an existing pile
   if (tablePiles[player & 1].size ())
   unsigned int i (0);
   for (; i < playerPile.size (); ++i) {
      TRACE8 ("Buraco::executeMove (player) - Analyzing Card " << *playerPile[i]);

      // Check if the actual card can be added to an existing pile
      if (tablePiles[player & 1].size ()) {
         TRACE8 ("Buraco::executeMove (unsigned int) - Adding single "
                 << *playerPile[i] << '?');
         unsigned int target (cardFitsOnPlayedPile (player, i));
         if ((target != -1U) && canDumpCards (player, 1))

   // Check for 3 cards belonging to a serie

      unsigned int nrs (1);
      unsigned int bCols (0x4);
      std::map<int, unsigned int> aPos;
      for (ICardPile::const_iterator p (playerPile.begin ());
           ((p = getFittingCard (playerPile, *playerPile[i], p)) 
             != playerPile.end ()); ++p) {
         if (*p == playerPile[i])
            aPos[2] = i;
         else {
            int diff (cardDistance (**p, *playerPile[i]));
            TRACE9 ("Buraco::executeMove (unsigned int) - " << **p
                    << " differs " << diff);
            if (diff) {
               diff = (diff < 0) ? (diff + 2) : (diff + 2);
               Check3 (diff <= 4);
               if (!(bCols & (1 << diff))) {
                  Check3 (aPos.find (diff) == aPos.end ());
                  aPos[diff] = p - playerPile.begin ();
                  bCols |= (1 << diff);
                  if (aPos.size () == 7)
                     break;
               }
            }
            else
                if (++nrs == 7)
                    break;
         }
      }

      // Check if the series of colors is a valid one
      TRACE9 ("Buraco::executeMove (unsigned int) - Serie: " << std::hex
              << bCols << std::dec);
      Check3 (aPos.find (2) != aPos.end ());
      Check3 (bCols & 0x4);
      if (((bCols & 0x3) != 0x3) && ((bCols & 0xa) != 0xa)
          && ((bCols &0x18) != 0x18))
         aPos.clear ();
      //   - Play jokers if there are at least 5 and the team has still the
      //     reserve and the other team has no burraco and the reserve
      //   - Play jokers if there are at least 5
          ? ((((nrs > 4) && (reserve[player & 1].size ()))
              || (nrs > 5))
          ? (nrs > 4)
         if (nrs < aPos.size ()) {
          if (nrs < aPos.size ()) {
             // Although more cards have been found, only play 3 of them as
             // this makes the logic easier; the others are added in the next
             // turns.
             nrs = ((bCols & 0x3) == 0x3) ? 2 : ((bCols & 0xa) == 0xa) ? 3 : 4;
             for (unsigned int c (0); c < 3; ++c) {
                Check3 (aPos.find (nrs - c) != aPos.end ());
                playerPile.move (playerPile.size () - 1 - c, aPos[nrs - c]);
             }
             i = playerPile.size () - (nrs = 3);
          }
          aPos.clear ();
         if (!unfinishedMonoPiles[player & 1]
         if (canDumpCards (player, nrs) || (nrs-- > 3)) {
            pos1Play = firstPos;
            CardVPile& pile (makeNewPile (player & 1));
            flipCards2Play (playerPile, pos1 = i, pos2 = i + nrs - 1);
      }
   }

   // Check if all cards in the hand can (and should) be played
   TRACE8 ("Buraco::executeMove (unsigned int) - Playing all?");
   if (!unfinishedMonoPiles[player & 1]
   TRACE8 ("Buraco::executeMove (player) - Playing all?");
   if ((!reserve[player & 1].empty () && canGetRidOfCards (player))
       || (points[player & 1] > 100)) {
      while (!((ci == playerPile.end ()) || isJoker (**ci))) {
         ICardPile::const_iterator next (playerPile.getFittingCard (**ci, ci + 1,
      if (!isJoker (**ci)) {
         ICardPile::const_iterator next (getFittingCard (playerPile, **ci, ci + 1));
            TRACE1 ("Buraco::executeMove (unsigned int) - Have two with joker: "
                    << **ci << " and " << **next);
            TRACE1 ("Buraco::executeMove (player) - Have two with joker: "
            Check3 (diff ? (*next)->colour () == (*ci)->colour () : true);
            if (diff < 0) {
               Check3 (diff >= -2);
               playerPile.move (playerPile.size () - 2,
                                next - playerPile.begin ());
               playerPile.move (playerPile.size () - 1 + diff,
                                ci - playerPile.begin ());
               playerPile.move (playerPile.size () - 1, ci - playerPile.begin ());
               Check3 (diff <= 2);
               playerPile.move (playerPile.size () - 1, next - playerPile.begin ());
               playerPile.move (playerPile.size () - 1 - diff,
                                ci - playerPile.begin ());
            }

            // Create a new pile with the found pair and a joker
            makeNewPile (player & 1);
            pos1Play = playerPile.size () - 3;
            CardVPile& pile (makeNewPile (player & 1));
            flipCards2Play (playerPile, pos1 = playerPile.size () - 3,
                            pos2 = playerPile.size () - 1);
         ++ci;
      }

      ++ci;
   // Play all jokers if team has a cerrado, or leave one, if the player has
   // >= 2 normal cards left.
   if (playerPile.size () && (points[player & 1] > 100)) {
      if ((isJoker (*playerPile[playerPile.size () - 1]))
             && ((playerPile.size () <= 2)
                 || ((!isJoker (*playerPile[1]))
                     || isJoker (*playerPile[playerPile.size () - 2])))) {
         unsigned int bestPile (-1U);
                     || isJoker (*playerPile[playerPile.size () - 2]))))
         for (std::vector<CardVPile*>::const_iterator p (tablePiles[player & 1].begin ());
              p != tablePiles[player & 1].end (); ++p)
            if (((*p)->size () < 7) && containsNoJoker (**p)) {
               pos1 = pos2 = playerPile.size () - 1;
               flipCards2Play (playerPile, pos1, pos2);
               return (p - tablePiles[player & 1].begin ()) << 16;

   if (containsOnlyJoker (playerPile))
      if (!reserve[player & 1].empty ()) {
         addBuraco (player);
         return executeMove (player);
      }

   // No more cards to put down: Find a card to dump
   TRACE8 ("Buraco::executeMove (unsigned int) - Searching for a card to dump");
   for (i = 0; i < playerPile.size () - 1; ++i) {
      ICardPile::const_iterator p (playerPile.getFittingCard (*playerPile[i], playerPile.begin (),
   for (i = 0; i < playerPile.size () - 1; ++i)
      if (getFittingCard (playerPile, *playerPile[i], playerPile.begin () + i + 1)
          == playerPile.end ())

      --i;

   Check3 (i < playerPile.size ());
   pos1Play = pos2Play = i;
   return 0xffff0000;
   flipCards2Play (playerPile, pos1 = i, pos2 = i);
   return 0xffff << 16;
//-----------------------------------------------------------------------------
/// Starts the game by dealing the cards
//-----------------------------------------------------------------------------
void Buraco::start () {
   TRACE9 ("Buraco::start ()");
   Game::start ();

   if (pScoreDlg) {
      unsigned int player;
      int points;
      pScoreDlg->getMaxPoints (points, player);
      if (points >= (int)ENDPOINTS) {
         delete pScoreDlg;
      if (points >= 2000) {
      }
   }

   pos1Play = pos2Play = 0;
   target = -1U;
   randomizeCardsToPile (staple);
            reserve[(i - posServer) & 1].push_back (&staple.removeTopCard ());
   for (unsigned int j (0); j < 11; ++j) {
      for (unsigned int i (1); i < NUM_PLAYERS; ++i) {
         hands[i].setTopCard (staple.removeTopCard ());
      gStatus.team1Buraco = gStatus.team2Buraco = 0x3;
      for (unsigned int i (0); i < (sizeof (reserve) / sizeof (reserve[0])); ++i)
         reserve[i].push_back (&staple.removeTopCard ());
   if (startPlayer)
   for (unsigned int i (0); i < NUM_PLAYERS; ++i)
       hands[i].sort (compByNumberWithJokers);

   dumped.setTopCard (staple.removeTopCard ());
   hands[0].show ();

   status.pop ();
   status.push (_("You can sort the cards in your hand with drag and drop or put"
                  " them on the table - click card to dump to end turn"));

   gStatus.startTurn = gStatus.startGame = 1;
   gStatus.team1Buraco = gStatus.team2Buraco = 0x3;

   points[0] = points[1] = 0;
   updateInfo ();
   hands[startPlayer ? startPlayer : 1].show ();
   setNextPlayer (startPlayer);
   displayTurn (startPlayer++);
   startPlayer &= 0x3;
   makeNextMoves ();
//-----------------------------------------------------------------------------
/// Remove cards from everything which can hold them
//-----------------------------------------------------------------------------
void Buraco::clean () {
   TRACE9 ("Buraco::clean ()");
   disableHuman ();
   for (unsigned int i (0); i < NUM_PLAYERS; ++i)
      hands[i].clear ();
   for (unsigned int i (0); i < NUM_PLAYERS; ++i) {
   staple.clear ();
      hands[i].hide ();
   }
   dumped.clear ();

   for (unsigned int i (0); i < (NUM_PLAYERS >> 1); ++i) {
      for (std::vector<BuracoPile*>::iterator p (tablePiles[i].begin ());
           p != tablePiles[i].end (); ++p) {
      for (std::vector<CardVPile*>::iterator p (tablePiles[i].begin ());
         delete *p;
      }
      tablePiles[i].clear ();
   }

   for (unsigned int i (0); i < (sizeof (reserve) / sizeof (reserve[0])); ++i)
      reserve[i].clear ();

   Game::clean ();


//-----------------------------------------------------------------------------
/// Enables the cards the human can pick up.
/// \returns \c 0
//-----------------------------------------------------------------------------
bool Buraco::enableHuman () {
   Check3 (staple.size ()); Check3 (dumped.size ());
   Check3 (!stapleTop.connected ()); Check3 (!dumpedTop.connected ());

   stapleTop = staple.getTopCard ().signal_clicked ().connect
      (mem_fun (*this, (&Buraco::stapleSelected)));
   dumpedTop = dumped.getTopCard ().signal_clicked ().connect
      (slot (*this, (&Buraco::stapleSelected)));

      (slot (*this, (&Buraco::dumpedSelected)));
}

//-----------------------------------------------------------------------------
/// Enables the cards in the hand of the human player
//-----------------------------------------------------------------------------
void Buraco::enableHumanHand () {
   Check1 (activeCards.empty ());
   Check1 (gameStatus () == PLAYING);

   Check3 (hands[0].size ());
   for (unsigned int i (0); i < hands[0].size (); ++i) {
      registerHandDND (i);
   for (unsigned int i (0); i < hands[0].size (); ++i)
   }
   TRACE2 ("Buraco::enableHuman () - Human has " << hands[0].size ()
           << " cards");

   newPile.drag_dest_set (dndType, Gtk::DEST_DEFAULT_ALL, Gdk::ACTION_MOVE);
   aDNDTable[NULL] = newPile.signal_drag_data_received ().connect
   for (unsigned int i (0); i < hands[0].size (); ++i)
      enableCard (i);

      (bind (mem_fun (*this, &Buraco::cardDroppedOnTable), -1U));

      (bind (slot (*this, &Buraco::cardDroppedOnTable), -1U));
      Check3 (tablePiles[0][i]);
      for (unsigned int j (0); j < tablePiles[0][i]->size (); ++j)
         registerTableDND (*(*tablePiles[0][i])[j], (i << 8) + j);
   }

   menuSort->set_sensitive ();
//-----------------------------------------------------------------------------
/// Disables the cards the human player can select
//-----------------------------------------------------------------------------
void Buraco::disableHuman () {
   TRACE2 ("Buraco::disableHuman () - DND: " << aDNDHand.size () << "; "
           << aDNDTable.size ());
   TRACE2 ("Buarzno::disableHuman () - DND: " << aDNDHand.size () << "; "
   menuSort->set_sensitive (false);


   if (aDNDHand.size ())
      for (unsigned int i (0); i < hands[0].size (); ++i)
         unregisterHandDND (*hands[0][i]);
   Check3 (aDNDHand.empty ());

   if (aDNDTable.size ()) {
      for (unsigned int i (0); i < tablePiles[0].size (); ++i) {
         Check3 (tablePiles[0][i]);
         for (unsigned int j (0); j < tablePiles[0][i]->size (); ++j)
            unregisterTableDND (*(*tablePiles[0][i])[j]);
      }
      aDNDTable[NULL].disconnect ();
      aDNDTable.erase (NULL);
   }
   Check3 (aDNDTable.empty ());

   if (dumpedTop.connected ())
      dumpedTop.disconnect ();
   if (stapleTop.connected ())
      stapleTop.disconnect ();
}

//-----------------------------------------------------------------------------
/// Callback after clicking on a card in the hand
/// \param iCard: Offset of card in hand
//-----------------------------------------------------------------------------
void Buraco::cardSelected (unsigned int iCard) {
   TRACE5 ("Buraco::cardSelected (unsigned int) - Position " << iCard);
   Check1 (iCard < hands[0].size ());
   Check1 (gameStatus () == PLAYING);

   // Check if all piles are valid
   if (!humanPilesOK ()) {
      Gtk::MessageDialog dlg (_("Every pile on the table must have at least 3 cards!"),
                              Gtk::MESSAGE_ERROR);
      dlg.set_title (_("Invalid move"));
      dlg.run ();
      return;
   }

   if (getConnectionMgr ().getMode () != YGP::ConnectionMgr::NONE) {
      // Send played card to all clients (if any)
   menuUndo->set_sensitive (false);

   // reserve
   if (containsOnlyJoker (hands[0]))
      if (!reserve[0].empty ())
         addBuraco (0);
      else if (hands[0].empty ()) {
         points[0] += 100;
         endGame ();
         return;
      }

   gStatus.startTurn = 1;
   gStatus.startGame = 0;
   makeNextMoves ();
}

//-----------------------------------------------------------------------------
/// Callback after clicking on the staple
//-----------------------------------------------------------------------------
void Buraco::stapleSelected () {
   TRACE5 ("Buraco::stapleSelected ()");
   Check1 (gameStatus () == PLAYING);
   Check3 (staple.size ());
   Check2 (dumped.size ());
   Check3 (stapleTop.connected ()); Check3 (dumpedTop.connected ());
   if (getConnectionMgr ().getMode () != YGP::ConnectionMgr::NONE) {
      // Send played card to all clients (if any)

   // Move top card to human and enable the cards in his hand, when idle
   // (means: *after* this signalhandler termintes)
   Glib::signal_idle ().connect
      (bind_return (mem_fun (*this, &Buraco::doStapleSelected), false));
}
       (bind_return (slot (*this, &Buraco::doStapleSelected), false));
//-----------------------------------------------------------------------------
/// Delayed callback after clicking on the staple
//-----------------------------------------------------------------------------
void Buraco::doStapleSelected () {
   TRACE5 ("Buraco::doStapleSelected ()");
   Check1 (gameStatus () == PLAYING);
   Check2 (staple.size ());
   Check2 (dumped.size ());
   Check3 (staple.size ());
   unsigned int player (currentPlayer ());
   hands[0].append (staple.removeTopCard ());
   enableHumanHand ();
//-----------------------------------------------------------------------------
/// Callback after clicking on the dumped staple
//-----------------------------------------------------------------------------
void Buraco::dumpedSelected () {
   TRACE5 ("Buraco::dumpedSelected ()");
   Check1 (gameStatus () == PLAYING);
   Check3 (dumped.size ());
   Check3 (stapleTop.connected ()); Check3 (dumpedTop.connected ());

   if (!gStatus.startGame)
      try {
   if (!((gStatus.startGame)
         || pileHasFittingPair (hands[0], dumped.getTopCard ()))) {
      Gtk::MessageDialog dlg (_("You need a fitting pair to pick up the"
                                " pile of dumped cards!"),
                              Gtk::MESSAGE_ERROR);
      dlg.set_title (_("Invalid move"));
      dlg.run ();
      return;
   }
      // Send played card to all clients (if any)

   // Special handling of player starting the game and can choose one of the
   // first two cards
   // Move top card to human and enable the cards in his hand, when idle
   // (means: *after* this signalhandler termintes)
}
       (bind_return (slot (*this, &Buraco::doDumpedSelected), false));
//-----------------------------------------------------------------------------
/// Action after picking up the card from the dumped staple
//-----------------------------------------------------------------------------
/// Delayed callback after clicking on the dumped staple
   TRACE5 ("Buraco::doDumpedSelected () - " << gStatus.startGame);
   Check1 (gameStatus () == PLAYING);
   TRACE5 ("Buraco::doDumpedSelected ()");
   unsigned int player (currentPlayer ());


   // Special handling of human starts the game and can choose one of the
   // first two cards
   if (gStatus.startGame) {
      Check3 (dumped.size () == 1);
      hands[0].append (dumped.removeTopCard ());
   }
   else {
      Check3 (pileHasFittingPair (hands[0], dumped.getTopCard ()));
      CardVPile& pile (makeNewPile (0));                     // Create new pile
      pile.setTopCard (dumped.removeTopCard ());         // with picked up card

      while (dumped.size ())
          hands[0].append (dumped.removeTopCard ());
   }

   enableHumanHand ();
//-----------------------------------------------------------------------------
/// Enables a card in the hand of the player
//-----------------------------------------------------------------------------
void Buraco::enableCard (unsigned int pos) {
   TRACE9 ("Buraco::enableCard (unsigned int) - Enabling card " << pos);
   Check1 (pos < hands[0].size ());

   activeCards.push_back
      (hands[0][pos]->signal_clicked ().connect
       (bind (mem_fun (*this, (&Buraco::cardSelected)), pos)));
}
       (bind (slot (*this, (&Buraco::cardSelected)), pos)));
//-----------------------------------------------------------------------------
/// Prepares the card for drag´n´drop
/// \param iCard: Number of card in hand
//-----------------------------------------------------------------------------
void Buraco::registerHandDND (unsigned int iCard) {
   Check1 (iCard < hands[0].size ());
   TRACE9 ("Buraco::registerHandDND (unsigned int) - Card: " << iCard << " ("
           << *hands[0][iCard] << " = " << hands[0][iCard] << ')');

   CardWidget& card (*hands[0][iCard]);
   Check3 (aDNDHand.find (&card) == aDNDHand.end ());

   // Card accepts drops from hand and drags from table
   card.drag_dest_set (dndType, Gtk::DEST_DEFAULT_ALL, Gdk::ACTION_MOVE);
   card.drag_source_set
      (dndType, Gdk::ModifierType (GDK_BUTTON1_MASK | GDK_BUTTON2_MASK | GDK_BUTTON3_MASK),
       Gdk::ACTION_MOVE);
      (dndType, Gdk::ModifierType (GDK_BUTTON2_MASK | GDK_BUTTON3_MASK),
   card.drag_source_set_icon (card.getImage ());
   aDNDHand[&card].connReceive = card.signal_drag_data_received ().connect
      (bind (mem_fun (*this, &Buraco::cardDropped), iCard));
   aDNDHand[&card].connGet = card.signal_drag_data_get ().connect
      (bind (slot (*this, &Buraco::cardDropped), iCard));
}
      (bind (slot (*this, &Buraco::getDropData), iCard));
//-----------------------------------------------------------------------------
/// Stops the drag´n´drop abilities of the passed card
/// \param card: Card to unregister of dnd
//-----------------------------------------------------------------------------
void Buraco::unregisterHandDND (CardWidget& card) {
   TRACE9 ("Buraco::unregisterHandDND (CardWidget&) - Card: " << card
           << " -> Address: " << &card);
   Check1 (aDNDHand.size ());

   std::map<CardWidget*, CONNECTIONS>::iterator i (aDNDHand.find (&card));
   Check1 (i != aDNDHand.end ());

   card.drag_dest_unset ();
   card.drag_source_unset ();
   i->second.connReceive.disconnect ();
   i->second.connGet.disconnect ();
   aDNDHand.erase (i);
}

//-----------------------------------------------------------------------------
/// Prepares the passed region of cards for drag´n´drop
/// \param pile: Pile whose cards should be registered
/// \param start: Number of first card to prepare for DND
/// \param end: Number of last card to prepare for DND
/// \pre: \c start < \c end; \c end <= Number of cards
//-----------------------------------------------------------------------------
void Buraco::registerTableDND (unsigned int pile, unsigned int start, unsigned int end) {
   TRACE9 ("Buraco::registerTableDND (unsigned int, unsigned int, unsigned int)"
           << " - " << pile << '[' << start << '-' << end << ']');
   Check1 (pile < tablePiles[0].size ());
   Check1 (start <= end);
   Check1 (end < tablePiles[0][pile]->size ());

   ICardPile& tmp (*tablePiles[0][pile]);
   pile <<= 8;
      unregisterTableDND (card);
      CardWidget& card (*(*tablePiles[0][pile])[start]);
   }
      registerTableDND (card, (pile << 8) + start);

//-----------------------------------------------------------------------------
/// Prepares the card for drag´n´drop
/// \param card: Card to register
/// \param nr: Number of card in pile
//-----------------------------------------------------------------------------
void Buraco::registerTableDND (CardWidget& card, unsigned int nr) {
   TRACE9 ("Buraco::registerTableDND (CardWidget&, unsigned int) - " << card
           << " = " << std::hex << nr << " - " << &card << std::dec);

   // Card accepts drops from hand and drags from table
   card.drag_dest_set (dndType, Gtk::DEST_DEFAULT_ALL, Gdk::ACTION_MOVE);
   aDNDTable[&card] = card.signal_drag_data_received ().connect
      (bind (mem_fun (*this, &Buraco::cardDroppedOnTable), nr));
}
      (bind (slot (*this, &Buraco::cardDroppedOnTable), nr));
//-----------------------------------------------------------------------------
/// Stops the drag´n´drop abilities of the passed card
/// \param card: Card to de-register
//-----------------------------------------------------------------------------
void Buraco::unregisterTableDND (CardWidget& card) {
   TRACE9 ("Buraco::unregisterTableDND (unsigned int) - Card: " << card
           << " - " << &card );
   Check1 (aDNDTable.size () > 1);

   std::map<CardWidget*, SigC::Connection>::iterator i (aDNDTable.find (&card));
   Check1 (i != aDNDTable.end ());

   card.drag_dest_unset ();
   i->second.disconnect ();
   aDNDTable.erase (i);
}

//-----------------------------------------------------------------------------
/// Callback after dropping a card (within the hand)
/// \param pContext: Context of the drag (contains things like source,
/// Callback after dropping a card
/// \param data: Describes the thing which was dropped
/// \param info: Describes the type of data (should be 0)
/// \param pData: Describes the thing which was dropped
/// \param info: Describes the type of pData (should be 0)
/// \pre \c pContext not NULL; Expects \c info to be 0
//-----------------------------------------------------------------------------
/// \pre \c pContext, \c pData not NULL; Expects \c info to be 0
                          gint, gint, const Gtk::SelectionData& data,
                          guint info, guint32 time, unsigned int card) {
                           gint, gint, GtkSelectionData* pData, guint info,
                           guint32 time, unsigned int card) {
   Check3 (pData);
   Check3 (data.get_format () == 8);
   Check3 (pData->length == sizeof (int));
   Check3 (pData->format == 8);
   unsigned int* pValue (reinterpret_cast <unsigned int*>
                         (const_cast<guint8*> (data.get_data ())));
   unsigned int* pValue (reinterpret_cast <unsigned int*> (pData->data));
   TRACE1 ("Buraco::cardDropped (...) - Inserting card " << *pValue
           << " at pos " << card);

   if (acceptCards != -1U) {
      context->drag_finish (false, false, time);
   CardWidget& cardMoved (hands[0].remove (*pValue));
   hands[0].insert (cardMoved, card);                     // Insert moved card
   CardWidget& cardTarget (*hands[0][card]);

   if (*pValue > card)
      ++card;

   // Adapt dnd-settigns
   if (*pValue < card) {
      unsigned int temp (card);
      card = *pValue;
      *pValue = temp;
   }

   Glib::signal_idle ().connect
   registerHandDND (card, *pValue);
}
//-----------------------------------------------------------------------------
/// Checks if the piles on the table are valid (have at least 3 cards)
/// \param except: Pile which can be invalid
/// \returns \c True, if the piles are OK
//-----------------------------------------------------------------------------
bool Buraco::humanPilesOK (unsigned int except) const {
   for (std::vector<BuracoPile*>::const_iterator p (tablePiles[0].begin ());
        p != tablePiles[0].end (); ++p) {
   for (std::vector<CardVPile*>::const_iterator p (tablePiles[0].begin ());
      if ((p - tablePiles[0].begin ()) == static_cast<int> (except))
         continue;
      if ((p - tablePiles[0].begin ()) == except)
      if ((*p)->size () < 3)
         return false;
   }
   return true;
}

//-----------------------------------------------------------------------------
/// Callback after dropping a card on the table
/// \param pContext: Context of the drag (contains things like source,
/// \param target, action, ...)
/// \param data: Describes the thing which was dropped
/// \param info: Describes the type of data (should be 0)
/// \param pData: Describes the thing which was dropped
/// \param info: Describes the type of pData (should be 0)
/// \pre \c pContext not NULL;
//-----------------------------------------------------------------------------
/// \pre \c pContext, \c pData not NULL;
                                 gint, gint, const Gtk::SelectionData& data,
                                 guint, guint32 time, unsigned int iCard) {
                                  gint, gint, GtkSelectionData* pData,
                                  guint, guint32 time, unsigned int iCard) {
   Check3 (!context->get_is_source ());
   Check3 (data.get_length () == sizeof (int));
   Check3 (pData);
   Check3 (data.get_format () == 8);
   Check3 (pData->length == sizeof (int));
   Check3 (pData->format == 8);
                         (const_cast<guint8*> (data.get_data ())));
   unsigned int* pValue (reinterpret_cast <unsigned int*> (pData->data));
           << " in pile");
   Check3 (*pValue < hands[0].size ());
   Check3 (*pValue < hands[0].size ());

   if (!humanPilesOK (iCard >> 8)) {
      context->drag_finish (false, false, time);
      Gtk::MessageDialog dlg (_("You need to fill up other piles first!"),
                              Gtk::MESSAGE_ERROR);
      dlg.set_title (_("Invalid move"));
      dlg.run ();
      return;
   }

   CardWidget& moved (*hands[0][*pValue]);
   TRACE4 ("Buraco::cardDroppedOnTable (...) - Card dropped: " << moved);
   // Move dropped card to a (new) pile on the table
   unsigned int iPile;
   CardVPile* pile (NULL);

   if ((acceptCards != -1U)
       && isJoker (moved) || (*pValue >= acceptCards)) {
   if (iCard == -1U) {    // If card was dropped on the new label: Create pile
      // Only allow dropping on new pile while having < 5 cards, if the game
      // can be ended, or there is still the reserve
      if (!canDumpCards (0, 3)) {
                                 Gtk::MESSAGE_ERROR);
         Gtk::MessageDialog dlg ((_((hands[0].size () <= 5)
                                    ? N_("You can't end the game (there's no \"cerrado\")!")
                                    : N_("Not enough cards to make new pile!"))).c_str (),
         dlg.run ();
         return;
      }

      // Only allow dropping on new pile while having < 5 cards, if the game
      // can be ended, or there is still the reserve
      // Check validity of drop
      if (!(isJoker (moved)
            ? pileHasFittingPair (hands[0])
            : pileHasFittingPair (hands[0], moved, true, true))) {
                                    ? N_("You can't end the game (there's no \"cerrado\")!")
         Gtk::MessageDialog dlg (_("There are no cards to make three of a kind!"),
         dlg.run ();
         return;
      }

      iPile = tablePiles[0].size ();
      pile = &makeNewPile (0); Check3 (tablePiles[0].size ());
      iCard = 0;
   }
   else {
      // Else check pile to use
      Check1 ((iCard >> 8) < tablePiles[0].size ());
      pile = tablePiles[0][iPile = (iCard >> 8)];
      Check1 ((iCard >> 8) <= tablePiles[0].size ());
      pile = tablePiles[0][iPile = (iCard++ >> 8)];
      if ((iCard = cardFitsOnPile (*pile, moved)) == -1) {
                                 Gtk::MESSAGE_ERROR);
         Gtk::MessageDialog dlg (_("This card does not fit on that pile!"),
         dlg.run ();
         return;
      }

      if ((iCard = cardFitsOnPile (iPile, moved)) == -1U) {
         context->drag_finish (false, false, time);
      // Only allow dropping of last card, if the game can be ended, or there
      // is still the reserve
      if (!canDumpCards (0, 1) && (pile->size () < 6)) {
                                 Gtk::MESSAGE_ERROR);
         Gtk::MessageDialog dlg (_("You can't end the game (there's no \"cerrado\")!"),
         dlg.run ();
         return;
      }

      if ((pile->size () == 1)
   // End old drag
   context->drag_finish (true, false, time);
   activeCards[*pValue].disconnect ();
   activeCards.erase (activeCards.begin () + *pValue);

   // Unregister old card
   hands[0].remove (*pValue);
   unregisterHandDND (moved);

   // Insert card into pile and register it for DND
   unsigned int move (-1U);
   pile->getPosition4Card (moved, iCard, move);
   if (iCard < (pile->size () - 1))
      registerTableDND (iPile, iCard + 1, pile->size () - 1);

   TRACE9 ("Buraco::cardDroppedOnTable (...) - Undo:  " << iPile << "; " << iCard
	   << "; " << *pValue << ": " << acceptCards);
      removeCerrado (0, *pile);

   // Accept again the jokers, if the pile has has now three cards (jokers are
   // disabled, if the human picked up the dumped pile.
      if (!reserve[0].empty ()) {
         addBuraco (0);
         return;
      }
      else
         if (hands[0].empty ()) {
            points[0] += 100;
            endGame ();
            return;
         }

   // Re-register the cards in the hand of the human for DND
   if (*pValue < hands[0].size ())
  // Re-register the cards in the hand for DND
   Check3 (aDNDHand.size () == hands[0].size ());
}

//-----------------------------------------------------------------------------
/// Callback to query the data to drop
/// \param pContext: Context of the drag (contains things like source,
/// \param target, action, ...)
/// \param data: Describes the thing which was dropped
/// \param time: Timestamp of the drag
/// \param pData: Describes the thing which was dropped
/// \pre \c pContext not NULL; Expects \c info to be 0
//-----------------------------------------------------------------------------
/// \pre \c pContext, \c pData not NULL; Expects \c info to be 0
                          Gtk::SelectionData& data, guint info, guint32 time,
                          unsigned int cardPos) {
                           GtkSelectionData* pData, guint info, guint32 time,
                           unsigned int cardPos) {
   Check1 (pData); Check1 (!info);
   data.set (data.get_target (), 8, reinterpret_cast <guchar*> (&cardPos),
             sizeof (cardPos));
   gtk_selection_data_set (pData, pData->target, 8, reinterpret_cast <guchar*> (&cardPos),
                           sizeof (cardPos));
//-----------------------------------------------------------------------------
/// Prepares the passed region of cards for drag´n´drop
/// \param start: Number of first card to prepare for DND
/// \param end: Number of last card to prepare for DND
/// \pre \c start < \c end; \c end <= Nr. ofcards
//-----------------------------------------------------------------------------
void Buraco::registerHandDND (unsigned int start, unsigned int end) {
   TRACE5 ("Buraco::registerHandDND (unsigned int, unsigned int) - [" << start
           << '-' << end << ']');
   Check1 (start <= end);
   Check1 (end < hands[0].size ());
   Check1 (end < activeCards.size ());

   for (; start <= end; ++start) {
      TRACE9 ("Buraco::registerHandDND (unsigned int, unsigned int) - Handling card " << start);

         (bind (mem_fun (*this, (&Buraco::cardSelected)), start));

         (bind (slot (*this, (&Buraco::cardSelected)), start));
      registerHandDND (start);
   }
   TRACE9 ("Buraco::registerHandDND (unsigned int, unsigned int) - End ");
}
//-----------------------------------------------------------------------------
/// Checks, if the passed pile contains no cards except jokers or 2s. This is
/// also true for empty piles.
/// \param pile: Pile to inspect
/// \returns \c True, if there are only jokers (or pile is empty)
//-----------------------------------------------------------------------------
bool Buraco::containsOnlyJoker (const std::vector<CardWidget*>& pile) {
   TRACE8 ("Buraco::containsOnlyJoker (const std::vector<CardWidget*>&");

   for (std::vector<CardWidget*>::const_iterator i (pile.begin ());
        i != pile.end (); ++i) {
      Check3 (*i);
      if (!isJoker (**i))
         return false;
   }
   return true;
}

//-----------------------------------------------------------------------------
/// Checks, if the passed pile does not contain neither jokers nor 2s.
/// \param pile: Pile to inspect
/// \returns \c True, if there are no jokers
//-----------------------------------------------------------------------------
bool Buraco::containsNoJoker (const std::vector<CardWidget*>& pile) {
   for (std::vector<CardWidget*>::const_iterator i (pile.begin ());
        i != pile.end (); ++i) {
      Check3 (*i);
      if (isJoker (**i))
         return false;
   }
   return true;
}

//-----------------------------------------------------------------------------
/// Adds the buraco to the passed player.
/// \param player: Player getting the reserve
/// Adds the buraco to the passed player This is also true for empty piles.
void Buraco::addBuraco (unsigned int player) {
/// \param showt: Flag, if info-message should be displayed
   TRACE3 ("Buraco::addBuraco (unsigned int) - " << player);
void Buraco::addBuraco (unsigned int player, bool show) {
   undo.pickUp = 1;
   undo.cJokers = hands[player].size ();
   (player & 1) ? gStatus.team2Buraco : gStatus.team1Buraco = (player >> 1);
      Game::disableHuman ();
      for (unsigned int i (0); i < hands[0].size (); ++i)
         unregisterHandDND (*hands[0][i]);
   }

   // Add reserve
   sort (reserve[player & 1].begin (), reserve[player & 1].end (),
	 compByNumberWithJokers);
   hands[player].setTopCards (reserve[player & 1]);
   hands[player].sort (compByNumberWithJokers);
   if (!player)
      for (unsigned int i (0); i < hands[player].size (); ++i) {
         enableCard (i);
         registerHandDND (i);
      }

   Check3 (actPlayers.size () > player);
   Check3 (actPlayers[player]);
   if (show) {
      status.pop ();
      std::string stat (_("%1 picked up the pile with the dumped cards"));
      stat.replace (stat.find ("%1"), 2, names[player]);
      status.push (stat);
   status.pop ();

   points[player & 1] += 100;

//-----------------------------------------------------------------------------
/// Hides the joker, which are displayed when picking up the buraco
/// \param pile: Pile holding the jokers shown
/// \returns BuracoPile&: New created pile
//-----------------------------------------------------------------------------
   TRACE9 ("Buraco::makeNewPile (unsigned int) - New pile for team " << team + 1);
CardVPile& Buraco::makeNewPile (unsigned int team) {
   Check1 (team < (sizeof (tablePiles) / sizeof (tablePiles[0])));

   Check1 ((sizeof (tablePiles) / sizeof (tablePiles[0]))
            == (sizeof (boxTeam) / sizeof (boxTeam[0])));
   tablePiles[team].push_back (pile);
   CardVPile* pile (new CardVPile (ICardPile::COMPRESSED, ICardPile::SHOWFACE));

   pile->show ();
   return *pile;
}

//-----------------------------------------------------------------------------
/// Checks if the passed card can be put on one of the existing piles
/// \param player: Player to inspect
/// \param iCard: Card to inspect
/// \returns Value describing the pile (and the offset of the card) to play
///     to; -1 if none
/// \returns \c Value describing the pile (and the offset of the card) to play
//-----------------------------------------------------------------------------
/// \remarks This method actually moves the card
   TRACE8 ("Buraco::cardFitsOnPlayedPile (unsigned int, unsigned int) - "
           "Card " << iCard << " of player " << player);
   TRACE9 ("Buraco::cardFitsOnPlayedPile (unsigned int, unsigned int) - "
   Check1 (player < NUM_PLAYERS);
   Check1 (iCard < hands[player].size ());
   CardWidget& card (*hands[player][iCard]);
   TRACE3 ("Buraco::cardFitsOnPlayedPile (unsigned int, unsigned int) - Card " << card);

   unsigned int bestPile (-1U);
   unsigned int maxPoints (0);
   for (std::vector<CardVPile*>::iterator p (tablePiles[player & 1].begin ());
              "Checking pile " << (int)(p - tablePiles[player & 1].begin ()));
      Check3 (*p);
      if ((*p)->size () == 7) {                         // Skip finished piles
         Check3 (!(*p)->is_visible ());
      if ((*p)->size () == 7) {     // Skip hidden piles (as they are finished)
      }
      Check3 ((*p)->size () >= 3);
      Check3 ((*p)->size () < 7);

      // Play joker, if you can make a cerrado (7 in a row) - but only if the
      // one having picked up the reserve already played (the missing card
      // might be in there) and the oponent can't finish. And of course not,
      // if you have 7 monos in your hand!
      // might be in there) and the oponent can't finish
      if (isJoker (card)) {
         if ((((*p)->size () == 6) && containsNoJoker (**p)
              && ((reserve[player & 1].empty ()
                   && (((player & 1) ? gStatus.team2Buraco : gStatus.team1Buraco
                        == 0x3)))
                  || ((reserve[(player + 1) & 1].empty ())
                      && (points[(player + 1) & 1] > 100))))
             || (((*p)->size () > 2) && containsOnlyJoker (**p))) {
            pos1 = pos2 = iCard;
            flipCards2Play (hands[player], pos1, pos2);
            return ((p - tablePiles[player & 1].begin ()) << 16) + (*p)->size ();
   }

      else {
         int pos (cardFitsOnPile (**p, card));
         if (pos != -1) {
            pos1 = pos2 = iCard;
            flipCards2Play (hands[player], pos1, pos2);
            return ((p - tablePiles[player & 1].begin ()) << 16) + pos;
         }
      pos1Play = pos2Play = iCard;
}

//-----------------------------------------------------------------------------
/// Checks if the passed card is a joker
/// \param card: Card to inspect
/// \returns \c True if card is a joker
//-----------------------------------------------------------------------------
bool Buraco::isJoker (const CardWidget& card) {
   return ((card.number () == CardWidget::TWO)
           || (card.number () > CardWidget::ACE));
   TRACE9 ("Buraco::isJoker (const CardWidget&) const - " << card << " = "
           << card.number ());
}

//-----------------------------------------------------------------------------
/// Removes a cerrado (a pile with 7 cards) from the table
/// \param player: Player causing the remove of the pile
/// Returns the value of the passed card
/// \param card: Card to inspect
/// \returns \c Value of the card
//-----------------------------------------------------------------------------
unsigned int Buraco::getPoints (const CardWidget& card) {
   // Card:                 2   3  4  5  6  7  8   9   10  J   Q   K   A   Joker
   static char values[] = { 25, 5, 5, 5, 5, 5, 10, 10, 10, 10, 10, 10, 20, 50 };
   Check3 (card.number () < (sizeof (values) / sizeof (values[0])));
   return values[card.number ()];
}

//-----------------------------------------------------------------------------
/// \param pile: Pile holding the cerrado
//-----------------------------------------------------------------------------
void Buraco::removeCerrado (unsigned int player, BuracoPile& pile) {
   unsigned int team (player & 1);
void Buraco::removeCerrado (unsigned int player, CardVPile& pile) {
           != tablePiles[team].end ());
   unsigned int team (player & 1);
   std::vector<CardVPile*>::iterator i
      (std::find (tablePiles[team].begin (), tablePiles[team].end (), &pile));
   Check1 (i != tablePiles[team].end ());
   Check3 (static_cast<int> (pile.getPotentialPoints ()) == pile.getPoints ());
   TRACE9 ("Buraco::removeCerrado (unsigned int, CardVPile&) - Pile "
           << (i - tablePiles[team].begin ()) << " of team " << team);
   points[team] += pile.getPoints ();

   CardWidget& card (isJoker (*pile[0]) ? *pile[1] : *pile[0]);
   if (isJoker (card)) {       // Cerrado of jokers found: Check if it is clean
      ICardPile::const_iterator p (pile.begin ());
      for (; p != pile.end (); ++p)
          if ((*p)->number () == CardWidget::UNREACHABLE)
             break;

      points[team] += (p == pile.end ()) ? 2000 : 1000;
   }
   else
      points[team] += ((card.number () == CardWidget::ACE)
                       ? containsNoJoker (pile) ? 500 : 300
                       : containsNoJoker (pile) ? 400 : 200);

//-----------------------------------------------------------------------------
/// Actualizes the info-part of the statusbar
//-----------------------------------------------------------------------------
void Buraco::updateInfo () {
   Glib::ustring strInfo (_("Points [Buraco]: %1 [%2] / %3 [%4]"));
   strInfo.replace (strInfo.find ("%1"), 2, YGP::ANumeric::toString (points[0]));
   std::string strInfo (_("Points [Buraco]: %1 [%2] / %3 [%4]"));
   strInfo.replace (strInfo.find ("%1"), 2, ANumeric::toString (points[0]));
   strInfo.replace (strInfo.find ("%2"), 2, 1, (reserve[0].empty () ? 'N' : 'Y'));
   strInfo.replace (strInfo.find ("%3"), 2, ANumeric::toString (points[1]));
   strInfo.replace (strInfo.find ("%4"), 2, 1, (reserve[1].empty () ? 'N' : 'Y'));
}
   info.pop ();
   info.push (strInfo);
//-----------------------------------------------------------------------------
/// Checks if the passed card fits on the passed staple
/// \param iPile: Pile to inspect
/// \param card: Card to check
/// \param pile: Pile to inspect
//-----------------------------------------------------------------------------
/// \returns \c Position where card can be played to, or -1 if card does not
///     fit
/// \pre Coloured piles must be sorted strict ascending
   Check1 (iPile < tablePiles[currentPlayer () & 1].size ());
int Buraco::cardFitsOnPile (ICardPile& pile, const CardWidget& card) const {
   Check1 (pile.size ()); Check1 (pile.size () < 7);
   //   - A joker; if there are at least 3 jokers (on table + in hand)
   // Check for jokers in pile; first and last points to first and last card
   // which are not jokers (if any)
   unsigned int posJoker (-1U);
   unsigned int first (-1U);
   unsigned int last (-1U);
   for (CardVPile::const_iterator i (pile.begin ()); i != pile.end (); ++i)
      if (isJoker (**i)) {
         Check3 ((posJoker == -1U) || (first == -1U));
         posJoker = (unsigned int)(i - pile.begin ());
      }
      else
         ((first == -1U) ? first : last) = (unsigned int)(i - pile.begin ());

   //   - Any card, which has a pair (if there's only one joker on the table)
   if (pile.getPosFirst () > 6)
      if (pile.getPosJoker ())
   if (first == -1U)
      if (posJoker)
         ICardPile::const_iterator pCard
             (hands[currentPlayer ()].getFittingCard (card, &cardDistance));
         ICardPile::const_iterator pCard (getFittingCard (hands[currentPlayer ()], card));
                                                             &cardDistance);
             pCard = getFittingCard (hands[currentPlayer ()], card, ++pCard);

   unsigned int pos, move;
   if (pile.getPosition4Card (card, pos, move)) {
   if (last == -1U)
      last = first;

   // A joker played on a pair: Find position to place the joker
   if ((posJoker == -1U) && isJoker (card)) {
      if (pile[first]->number () == pile[last]->number ())
         return 1;

      unsigned int nr (pile[first]->number ());
      for (; first < last; ++first)
         if (nr++ != first)
            break;
      return first;
   }

   // Else check if the pile is a numberd or a coloured one
   Check3 (first <= last); Check3 (last < pile.size ());
   TRACE5 ("Buraco::cardFitsOnPile (CardVPile&, CardWidget&) - "
           "Cards: " << *pile[first] << " and " << *pile[last]);
   if (pile[first]->number () == card.number ()) {
      TRACE9 ("Buraco::cardFitsOnPile (CardVPile&, CardWidget&) - Equal cards");
      if (pile[first]->number () == pile[last]->number ())
         return pile.size ();
   }
   else
      if ((pile[first]->colour () == card.colour ())
          && ((first == last)
              || (pile[first]->number () != pile[last]->number ()))
          && (pile[last]->colour () == card.colour ())) {
         // First check, if a joker can be replaced
         if ((posJoker != -1U)
             && (cardDistance (card, *pile[first]) == posJoker)) {
            pile.move (0, posJoker);
            return posJoker + 1;
         }

         // This code assums that the coloured pile is sorted from lower card
         // to higher cards (strict ascending)
         Check3 ((cardDistance (*pile[first], *pile[last]) <= 0)
                 || (pile[first]->number () == CardWidget::ACE));
         // Possible difference the card can have: 1 or two if joker at one end
         unsigned int maxDiff ((posJoker == -1U) ? 1
                               : (((posJoker < first) || (posJoker > last))
                                  ? 2 : 1));
         unsigned int diff (pile[first]->number () == CardWidget::ACE ? -1U
                            : cardDistance (*pile[first], card,
                                            pile[first]->number () <= CardWidget::FOUR));
         TRACE9 ("Buraco::cardFitsOnPile (CardVPile&, CardWidget&) - Diff (start): "
                 << diff << "; max: " << maxDiff);
         Check3 (diff);

         if (diff && (diff <= maxDiff)) {
            if ((diff == 2) && (posJoker > first)) {
               Check3 (!first);
               pile.move (0, posJoker);
               ++first;
            }
            return first - diff + 1;
         }

         // Test if card fits at other end
         diff = cardDistance (card, *pile[last],
                              first == last);
         TRACE9 ("Buraco::cardFitsOnPile (CardVPile&, CardWidget&) - Diff (end): "
                 << diff << "; max: " << maxDiff);
         if (diff && (diff <= maxDiff)) {
            if ((diff == 2) && (posJoker < last)) {
               Check3 (first > posJoker);
               pile.move (last, posJoker);
               --last;
            }
            TRACE9 ("Buraco::cardFitsOnPile (CardVPile&, CardWidget&) - Diff (ret): "
                    << last + diff);
            return last + diff;
         }
      }

//----------------------------------------------------------------------------
/// Shows or hides the cards of the computer player
/// \param open: Flag if cards should be shown or hidden
//-----------------------------------------------------------------------------
void Buraco::playOpen (bool open) {
   for (unsigned int i (1); i < NUM_PLAYERS; ++i) {
      hands[i].setShowOption (open ? ICardPile::SHOWFACE : ICardPile::SHOWBACK);
   for (unsigned int i (1); i < NUM_PLAYERS; ++i)
   }
//-----------------------------------------------------------------------------
/// Creates the combined team names from the players
/// \param names: Array to receive groups
void Buraco::endGame () {
   TRACE8 ("Buraco::endGame ()");

   if (!currentPlayer ())
      disableHuman ();

   if (!pScoreDlg) {
      pScoreDlg = ScoreDlg::create (nameTeams);
      pScoreDlg->get_window ()->set_transient_for (get_window ());
      std::vector <std::string> _names;
      for (unsigned int i (0); i < (NUM_PLAYERS >> 1); ++i) {
         std::string name (_("Team %1\n%2/%3"));
         name.replace (name.find ("%1"), 2, 1, char ('1' + i));
         name.replace (name.find ("%2"), 2, names[i << 1]);
         name.replace (name.find ("%3"), 2, names[(i << 1) + 1]);
         _names.push_back (name);
      }
      pScoreDlg = ScoreDlg::perform (_names);
   points[0] += reserve[0].empty () ? 100 : -100;
   points[1] += reserve[1].empty () ? 100 : -100;
   // Sum up all cards on the table
   for (unsigned int i (0); i < (NUM_PLAYERS >> 1); ++i) {
      int sum (0);
      int monoPile (0);

      for (std::vector<BuracoPile*>::const_iterator p (tablePiles[i].begin ());
           p != tablePiles[i].end (); ++p) {
      for (std::vector<CardVPile*>::const_iterator p (tablePiles[i].begin ());
         Check3 ((*p)->size () > 2);
         (*p)->show ();
            monoPile += 1000;
         if (((*p)->size () < 7) && (containsOnlyJoker (**p)))
            monoPile += 1000;
         for (std::vector<CardWidget*>::const_iterator c ((*p)->begin ());
              c != (*p)->end (); ++c)
            sum += getPoints (**c);
      TRACE5 ("Buraco::endGame () - Points of team " << i << " on table: "
              << sum << '/' << monoPile);
      points[i] = ((points[i] < (reserve[i].size () ? 100 : 300)) ? -sum : sum) - monoPile;
              << sum);
      if (points[i] < 200)
         points[i] = -sum - monoPile;
      else
         points[i] = sum - monoPile;
   for (unsigned int i (0); i < NUM_PLAYERS; ++i)
      for (std::vector<CardWidget*>::const_iterator c (hands[i].begin ());
           c != hands[i].end (); ++c)
         points[i & 1] -= getPoints (**c);

   pScoreDlg->addPoints (points);
   

   Glib::ustring stat (_("Round ended"));
   unsigned int player;
   std::string stat (_("Round ended"));
   pScoreDlg->getMaxPoints (maxPoints, player);
   TRACE9 ("Buraco::endGame () - Points: " << maxPoints);
   if (maxPoints >= (int)ENDPOINTS) {
      stat = _("Game ended; Team %1 won");
   if (maxPoints >= 2000) {
   }

   // Move cards of partners to first player and show them
   status.push (stat);

   setGameStatus (STOPPED);
}
   setGameStatus (STOPPED);   

//-----------------------------------------------------------------------------
/// jokers
/// \param player: Player whose cards should be inspected
/// \returns \c True: if all cards can be played
/// \remarks This method does not check for triplets anymore!
//-----------------------------------------------------------------------------
bool Buraco::canGetRidOfCards (unsigned int player) const {
   TRACE5 ("Buraco::canGetRidOfCards (unsigned int) - Checking player " << player);
bool Buraco::canGetRidOfCards (unsigned int player) {
   const CardHPile& pile (hands[player]);
   std::bitset<200> used; Check3 (hands[player].size () < used.size ());
   CardHPile& pile (hands[player]);
   unsigned int piles (0);
   for (ICardPile::const_iterator i (pile.begin ()); i != pile.end (); ++i) {
      if (used[i - pile.begin ()])
   CardVPile::const_iterator i (pile.begin ());
   if (isJoker (**i)) {
      cJokers = 1;
      used.set (0);
   }
   for (++i; i != pile.end (); ++i) {
         ++cJokers;
         continue;
      }

      // If there are equal cards (and the first card is not already marked as
      // used: Mark both card as used
      ICardPile::const_iterator o (pile.getFittingCard (**i, i + 1, &cardDistance));
      if ((o != pile.end ()) && !used[o - pile.begin ()]) {
      if (!used[i - pile.begin ()]
          && (((*i)->number () == (*(i - 1))->number ())
              || (((*i)->number () == ((*(i - 1))->number () + 1))
                  && ((*i)->colour () == (*(i - 1))->colour ())))) {
         used.set (o - pile.begin ());
      }
         used.set (i - pile.begin () - 1);

   TRACE9 ("Buraco::canGetRidOfCards (unsigned int) -  " << used.count ()
           << '/' << hands[player].size () << "; " << cJokers << " Joker for "
           << piles << " piles -> "
           << '/' << hands[player].size () << "; Joker: " << cJokers);
   return (((used.count () + 1) >= hands[player].size ())

//-----------------------------------------------------------------------------
/// Checks if the player can dump the specified number of cards; a player can
/// only dump all of his cards, if:
///   - The team has a cerrado
/// only dump all of his cards, if: - The team has a cerrado - The team still
/// has the reserve
/// \param pile: Pile player is going to play its card to (or -1 for a new one)
bool Buraco::canDumpCards (unsigned int player, unsigned int cards,
                           unsigned int pile) const {
bool Buraco::canDumpCards (unsigned int player, unsigned int cards) const {
   unsigned int cPile (hands[player].size ());
   return (cPile >= (cards + 2) || (points[player & 1] > 100)
           || !reserve[player & 1].empty ());
//----------------------------------------------------------------------------

//-----------------------------------------------------------------------------
/// Returns a card fitting to the passed on
/// \param pile: Pile to inspect
/// \param card: Card where to find a fitting one to
/// \param start: Position where to start the search
/// \returns \c Position of matching card or pile.end ()
/// \pre start must be a valid iterator in pile
//-----------------------------------------------------------------------------
ICardPile::const_iterator Buraco::getFittingCard (const ICardPile& pile,
                                                  const CardWidget& card,
                                                  ICardPile::const_iterator start) {
   TRACE3 ("Buraco::getFittingCard (const ICardPile&, const CardWidget*,"
           " const_iterator) - " << card);

   bool bJoker (isJoker (card));
   CardWidget::NUMBERS nr (card.number ());
   CardWidget::COLOURS colour (card.colour ());
   while (start != pile.end ()) {
      if (isJoker (**start)) {
         if (bJoker)
            break;
      }
      else if ((*start)->number () == nr)
         break;
      else
         if (((*start)->colour () == colour)
             && ((static_cast<unsigned int> (cardDistance (card, **start)) + 2) < 5))
            break;
      ++start;
   }

#if TRACELEVEL > 1
   if (start == pile.end ()) {
      TRACE ("Buraco::getFittingCard (const ICardPile&, const CardWidget*,"
             " const_iterator) - End");
   }
   else {
      TRACE ("Buraco::getFittingCard (const ICardPile&, const CardWidget*,"
             " const_iterator) - " << **start);
   }
#endif
   return start;
}

/// \param pile: Pile to inspect
/// \param card: Card where to find a pair to
/// \param withJokers: Flag, if jokers should be inspected
/// \returns \c True, if the pile contains a matching pair
/// \param pileHoldsCard: Flag, if the pile contains the card (to skip)
//-----------------------------------------------------------------------------
bool Buraco::pileHasFittingPair (const ICardPile& pile, const CardWidget& card,
                                 bool withJokers) {
   TRACE3 ("Buraco::pileHasFittingPair (const ICardPile&, const CardWidget*,"
                                 bool pileHoldsCard, bool withJokers) {

           " 2x bool) - " << card);
      ICardPile::const_iterator i (pile.getFittingCard (card, pile.begin (),
   unsigned int nrs (0);
   unsigned int bCols (0);

   for (ICardPile::const_iterator p (pile.begin ());
        (p = getFittingCard (pile, card, p)) != pile.end (); ++p) {
      if (pileHoldsCard && (*p == &card))    // Skip card if its the passed one
         continue;

      int diff (cardDistance (**p, card));
      if (diff) {
         diff = (diff < 0) ? (diff + 2) : (diff + 1);
         Check3 (diff < 5);
         TRACE9 ("Buraco::pileHasFittingPair (const ICardPile&, const "
                 "CardWidget*, 2x bool) - " << **p << " differs " << diff);
         if ((((unsigned int)diff) < 4) && !(bCols & (1 << diff))) {
             if (bCols & ((diff > 1) ? (0x1b << (diff - 2)) : 0x3))
                 return true;
             bCols |= (1 << diff);
         }
      }
      else
         if (++nrs == 2)
            return true;
   }
   TRACE9 ("Buraco::pileHasFittingPair (const ICardPile&, const "
           "CardWidget*, 2x bool) - " << card << " matches " << nrs << '/'
           << std::hex << bCols << std::dec);
   return (withJokers && !containsNoJoker (pile)) ? (nrs || bCols) : false;
//-----------------------------------------------------------------------------
/// Checks if the passed pile contains a pair matching the passed card
/// \param pile: Pile to inspect
/// \param exclude: Card to not inspect (can be NULL)
/// \returns \c True, if the pile contains a matching pair
/// \param card: Card where to find a pair to
bool Buraco::pileHasFittingPair (const ICardPile& pile, const CardWidget* exclude) {
   TRACE3 ("Buraco::pileHasFittingPair (const ICardPile&, const CardWidget*)");
bool Buraco::pileHasFittingPair (const ICardPile& pile) {
   TRACE3 ("Buraco::pileHasFittingPair (const ICardPile&)");
        p != pile.end (); ++p)
   unsigned int jokers (0);
      if (*p != exclude)
         if ((pile.getFittingCard (**p, pile.begin (), &cardDistance) != p)
       if (isJoker (**p)) {
           if (++jokers == 3)
              return true;
       }
       else
          if (pileHasFittingPair (pile, **p, false))
             return true;

//-----------------------------------------------------------------------------
/// Compares the cards in the pile with regard of the colour and with special
/// consideration of joker cards
/// \param a: Card to compare
/// \param b: Card to compare
/// \returns \c bool: True, if a < b
//-----------------------------------------------------------------------------
bool Buraco::compByNumberWithJokers (const CardWidget* a, const CardWidget* b) {
   // Card:                 2   3  4  5  6  7  8  9  10 J  Q   K  A   Joker
   static char values[] = { 12, 0, 1, 2, 3, 4, 5, 6, 7, 8, 9, 10, 11, 13 };
   Check3 (a->number () < static_cast<int> (sizeof (values) / sizeof (values[0])));
   Check3 (b->number () < static_cast<int> (sizeof (values) / sizeof (values[0])));
   Check3 (a->number () < (sizeof (values) / sizeof (values[0])));
   Check3 (b->number () < (sizeof (values) / sizeof (values[0])));
}

//----------------------------------------------------------------------------
/// Returns the distance between two cards. The ace also counts as one (if the
/// Returns the distance between two cards. The ace also counts as one (if the
/// other card is a 3 or a 4) and 2's are equal to jokers.
/// \param a: Card to compare
/// \param b: Card to compare
/// \param aceIsOne: Flag, if aces should (also) be treated as one
/// \returns \c int: Distance of the two passed cards (a - b)
//----------------------------------------------------------------------------
/// \returns \c int: Distance of the two passed cards
   TRACE9 ("Buraco::cardDistance (2x const CardWidget&, bool) - "
           << a << "<->" << b);
   TRACE1 ("Buraco::cardDistance (const CardWidget&, const CardWidget&) - "
   bool aJoker (isJoker (a));
   bool bJoker (isJoker (b));
   if (aJoker || bJoker)
      return aJoker && bJoker ? 0 : 99;

   if (a.colour () != b.colour ())
      return (a.number () == b.number ()) ? 0 : 99;
      if ((a.number () == CardWidget::ACE)
       TRACE9 ("Buraco::cardDistance (const CardWidget&, const CardWidget&) - "
               "Checking for Ace");
         return -static_cast<int> (b.number ());
          && (b.number () <= CardWidget::FOUR))
               && (a.number () < CardWidget::EIGHT))
         return static_cast<int> (a.number ());
               && (a.number () <= CardWidget::FOUR))

   TRACE4 ("Buraco::cardDistance (2x const CardWidget&, bool) - "
           "Distance: " << a.number () - b.number ());
   TRACE1 ("Buraco::cardDistance (const CardWidget&, const CardWidget&) - "
}
