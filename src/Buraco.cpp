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

#include <cstdlib>

#include <bitset>
#include <sstream>

#include <gtk/gtkdnd.h>

#include <gtkmm/scrolledwindow.h>
#include <gtkmm/messagedialog.h>

#include <YGP/ConnMgr.h>
#include <YGP/ANumeric.h>
#include <YGP/Tokenize.h>
#include <YGP/AttrParse.h>

#include <Human.h>
#include <ScoreDlg.h>
#include <ComputerPlayer.h>



#include "SigCExt.h"
std::vector<Gtk::TargetEntry> Buraco::dndType;


unsigned int Buraco::ENDPOINTS (2000);

/// \param parent: Parent widget to display the game in
/// \param statusbar: Status bar widget to display information about the game
/// \param cardset: Cardset to use
/// \param player: Vector of player
/// \param posPlayer: Position of player for the server
/// \param mxSerialize: Mutex to serialize messages from the server
//-----------------------------------------------------------------------------
Buraco::Buraco (Gtk::Box& parent, Gtk::Statusbar& statusbar,
                CardSet& cardset, const std::vector<Player*>& player,
                unsigned int posPlayer, YGP::Mutex& mxSerialize)
   : Game (parent, statusbar, cardset, player, posPlayer, mxSerialize, 3, 10)
     , startPlayer (-1U) , newPile (_("New pile"))
     , staple (ICardPile::TOTALLY_COMPRESSED, ICardPile::SHOWBACK)
     , dumped (ICardPile::TOTALLY_COMPRESSED, ICardPile::SHOWFACE)
     , acceptCards (-1U), target (-1U) , pScoreDlg (NULL) {
   TRACE9 ("Buraco::Buraco (Box&, Statusbar&, CardSet&, const "
           "std::vector<Glib::ustring>&)");

   for (unsigned int i (0); i < (NUM_PLAYERS >> 1); ++i) {
       scrlTable[i] = new Gtk::ScrolledWindow ();
       scrlTable[i]->add (boxTeam[i]);
       scrlTable[i]->set_policy (Gtk::POLICY_AUTOMATIC, Gtk::POLICY_AUTOMATIC);
       scrlTable[i]->show ();
   }

   int width (cards.getCard (0).getImageWidth ());
   int height (cards.getCard (0).getImageHeight ());

   TRACE9 ("Buraco::Buraco (Box&, Statusbar&, CardSet&, const "
           "std::vector<Glib::ustring>&) - Init common staples");
   staple.set_size_request (width, height);
   dumped.set_size_request (width, height);

   boxTeam[0].pack_end (newPile, Gtk::PACK_EXPAND_WIDGET, 5);
   boxTeam[0].set_size_request (-1, height + 5 * 15);
   boxTeam[1].set_size_request (-1, height + 5 * 15);

   for (unsigned int i (1); i < NUM_PLAYERS; ++i) {
      attach (hands[i], (i << 2) - 4, (i << 2) - 2, 4, 5,
              Gtk::EXPAND, Gtk::SHRINK, 5, 5);
      attach (names[i], (i << 2) - 4, (i << 2) - 2, 5, 6,
              Gtk::EXPAND, Gtk::SHRINK, 0);
      hands[i].show ();
      names[i].show ();
   }
   hands[0].setStyle (ICardPile::COMPRESSED);
   hands[0].setShowOption (ICardPile::SHOWFACE);
   hands[0].show ();
   names[0].show ();

   TRACE9 ("Buraco::Buraco (Box&, Statusbar&, CardSet&, const "
           "std::vector<Glib::ustring>&) - Attach widgets");
   attach (hands[0], 3, 10, 0, 1, Gtk::EXPAND, Gtk::SHRINK, 1, 5);
   attach (names[0], 3, 10, 1, 2, Gtk::EXPAND, Gtk::SHRINK, 1, 5);
   attach (staple, 0, 1, 0, 1, Gtk::SHRINK, Gtk::SHRINK, 5);
   attach (dumped, 1, 2, 0, 1, Gtk::SHRINK, Gtk::SHRINK, 1, 5);
   attach (*scrlTable[0], 0, 10, 2, 3, Gtk::EXPAND | Gtk::FILL,
           Gtk::EXPAND | Gtk::FILL, 0, 5);
   attach (*scrlTable[1], 0, 10, 3, 4, Gtk::EXPAND | Gtk::FILL,
           Gtk::EXPAND | Gtk::FILL, 0, 5);

   TRACE9 ("Buraco::Buraco (Box&, Statusbar&, CardSet&, const "
           "std::vector<Glib::ustring>&) - Show widgets");
   newPile.show ();
   staple.show ();
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

}

//-----------------------------------------------------------------------------
/// Destructor
//-----------------------------------------------------------------------------
Buraco::~Buraco () {
   TRACE9 ("Buraco::~Buraco ()");
   clean ();
   delete pScoreDlg;

   // Free team names
   for (std::vector<Player*>::iterator i (nameTeams.begin ());
        i != nameTeams.end (); ++i)
      delete *i;

   for (unsigned int i (0); i < (NUM_PLAYERS >> 1); ++i)
      delete scrlTable[i];
}


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
      Check3 (*p); Check3 ((*p)->size () <= 7);
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
int Buraco::makeMove (unsigned int player) {
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
   else {
      Check1 (pos1Play <= pos2Play);

      // Calculate pile to play cards to: If target = 0xffff0000, append cards
      // to dumped staple; else target specifies offset of pile and card on
      // table
      Check3 (hands[player].getTopCard ().showsFace ());
      unsigned int pos (target & 0xffff);
      ICardPile* dest;
      CardHPile& source (hands[player]);
      unsigned int oldPlayer (player);

      target >>= 16;
      if (target == 0xffff) {
         TRACE4 ("Buraco::makeMove (unsigned int) - Dumping card");
         Check3 (pos1Play == pos2Play);
         pos = dumped.size ();
         dest = &dumped;

         gStatus.startTurn = 1;
         ++player &= 0x3;
         displayTurn (player);
      }
      else {
         TRACE4 ("Buraco::makeMove (unsigned int) - Moving cards to pile " << target);
         Check3 (target < tablePiles[player & 1].size ());
         dest = tablePiles[player & 1][target];

#if CHECK > 2
         for (unsigned int pos (pos1Play); pos < pos2Play; ++pos) {
            int diff (cardDistance (*source[pos + 1], *source[pos]));
            TRACE1 ("Buraco::makeMove (unsigned int) - Card " << *source[pos]);
            Check3 (isJoker (*source[pos]) || isJoker (*source[pos + 1])
                    ? true : (diff == 0) || (diff == 1));
         }
#endif
      }

      // Move played cards to the pile to play
      Check3 (source.size () > pos2Play);
      for (; (int)pos1Play <= (int)pos2Play; --pos2Play)
         dest->insert (source.remove (pos1Play), pos++);
      target = -1U;

      if (gStatus.pickUpPlayed) {
         Check3 (dumped.size ());
            addBuraco (oldPlayer);
         else
            addBuraco (oldPlayer, false);
               cleanCerrado (oldPlayer);
               Check3 (points[oldPlayer & 1] > 100);
               points[oldPlayer & 1] += 100;
               endGame ();
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
unsigned int Buraco::showCardsToPlay (unsigned int player) {
   TRACE2 ("Buraco::showCardsToPlay (unsigned int) - " << player << " ("
           << gStatus.startGame << '/' << gStatus.startTurn << ')');

   if (((player & 1) ? gStatus.team2Buraco : gStatus.team1Buraco)
       == (player >> 1))
      ((player & 1) ? gStatus.team2Buraco : gStatus.team1Buraco) = 0x3;

      (player & 1) ? gStatus.team2Buraco : gStatus.team1Buraco = 0x3;
   if (gStatus.startTurn) {
      Check3 (dumped.size ());
      gStatus.startTurn = 0;

      CardWidget& dumpedCard (dumped.getTopCard ());

      if (gStatus.startGame
          ? (isJoker (dumpedCard)
             || playerPile.getFittingCard (dumpedCard, &cardDistance) != playerPile.end ())
          : ((!isJoker (dumpedCard))
             && pileHasFittingPair (playerPile, dumpedCard)
             && ((points[player & 1] > 100)
                 || reserve[player & 1].size ()
                 || (dumped.size () + playerPile.size () > 4)))) {
         if (getConnectionMgr ().getMode () != YGP::ConnectionMgr::NONE) {
                 || (dumped.size () + playerPile.size () > 3)))) {
            std::ostringstream msg;
            msg << "Play=" << dumpedCard.id () << ";Target=3";

            if (getConnectionMgr ().getMode () == YGP::ConnectionMgr::CLIENT)
               ignoreNextMsg = true;
            broadcastMessage (msg.str ());
         }

         dumpedCard.show ();
         playerPile.insertSorted (dumped.removeTopCard (), compByNumberWithJokers);
         movePile (playerPile, dumped);
         playerPile.sort (compByNumberWithJokers);
         if (dumped.size ())
            std::map<unsigned int, unsigned int> aPos;
            std::vector<unsigned int> aOrder;
            unsigned int nrs (playerPile.getSeries (dumpedCard, aPos, aOrder,
                                                    &cardDistance));
            TRACE8 ("Buraco::showCardsToPlay (unsigned int) - Sizes: "
                    << nrs << "<->" << aPos.size ());
            Check3 ((nrs >= 3) || (aPos.size () >= 3));

            if (nrs > 7)
               nrs = 7;
            pos1Play = ((nrs < aPos.size ())
                        ? (nrs = aPos.size (),
                           playerPile.sortColourSerie (aPos, aOrder))
                        : playerPile.find (dumpedCard, compByNumberWithJokers));
            pos2Play = pos1Play + nrs - 1;
                        : playerPile.findByNr (dumpedCard));

            makeNewPile (player & 1);
            target = (tablePiles[player & 1].size () - 1) << 16;
         }
      }
      else {
         if (getConnectionMgr ().getMode () != YGP::ConnectionMgr::NONE) {
            // Send played card to all clients (if any)
            std::ostringstream msg;
            msg << "Play=" << staple.getTopCard ().id () << ";Target=2";

            if (getConnectionMgr ().getMode () == YGP::ConnectionMgr::CLIENT)
               ignoreNextMsg = true;
            broadcastMessage (msg.str ());
         }

         dumpedCard.show ();
         playerPile.insertSorted (staple.removeTopCard (), compByNumberWithJokers);
      }

      gStatus.startGame = 0;
   }

   if (target == -1U)
      target = executeMove (player);
   flipCards2Play (playerPile, pos1Play, pos2Play);
   return target;
}

//-----------------------------------------------------------------------------
/// Executes a move for the passed player; only one move is made at every
/// timer-iteration
/// \param player: Actual player
/// \returns \c ID for target (32 Bit: Pile << 16 + Position)
//-----------------------------------------------------------------------------
int Buraco::executeMove (unsigned int player) {
   TRACE6 ("Buraco::executeMove (player) - " << player);

   ICardPile& playerPile (hands[player]);

   // Check if any card can be added to an existing pile
   if (tablePiles[player & 1].size ())
      for (ICardPile::const_iterator p (playerPile.begin ());
           p != playerPile.end (); ++p) {
         TRACE8 ("Buraco::executeMove (unsigned int) - Adding card " << **p << '?');
         unsigned int target (cardFitsOnPlayedPile (player, p - playerPile.begin ()));
         TRACE8 ("Buraco::executeMove (player) - Adding card " << **p << '?');
            return target;
         if ((target != -1U) && canDumpCards (player, 1, target >> 16))

   // Check for 3 cards belonging to a serie
   unsigned int i (0);
   for (; i < playerPile.size (); ++i) {
      TRACE8 ("Buraco::executeMove (unsigned int) - Analyzing card " << *playerPile[i]);

      TRACE8 ("Buraco::executeMove (player) - Analyzing card " << *playerPile[i]);
      std::vector<unsigned int> aOrder;
      unsigned int nrs (playerPile.getSeries (*playerPile[i], aPos, aOrder,
                                              &cardDistance));

      // Play found cards (if any)
      //   - Play jokers if there are at least 5 and the team has still the
      //     reserve and the other team has no burraco and the reserve
      //   - Play the bigger of the found matching cards, if there are >= 3
      if (isJoker (*playerPile[i])
          ? ((((nrs > 4) && (reserve[player & 1].size ()))
              || (nrs > 5))
             && ((points[(player + 1) & 1] < 101)
                 || (nrs > 6)
             && ((points[(player + 1) & 1] < 100)
                 || (nrs > 6)))
         if (nrs < aPos.size ()) {
            nrs = aPos.size ();
            i = playerPile.sortColourSerie (aPos, aOrder);
         if (nrs > 7)
            nrs = 7;

	 bool canDump (canDumpCards (player, nrs));
         if (!unfinishedMonoPiles[player & 1]
         if (canDumpCards (player, nrs) || (nrs-- > 3)) {
            pos1Play = firstPos;
            pos2Play = firstPos + nrs - 1;
            pos1Play = i;
            pos2Play = i + nrs - 1;
      }
   }

   // Check if all cards in the hand can (and should) be played
   TRACE8 ("Buraco::executeMove (unsigned int) - Playing all?");
   if (!unfinishedMonoPiles[player & 1]
   TRACE8 ("Buraco::executeMove (player) - Playing all?");
   if ((reserve[player & 1].size () && canGetRidOfCards (player))
       || (points[player & 1] > 100)) {
      while (!((ci == playerPile.end ()) || isJoker (**ci))) {
         ICardPile::const_iterator next (playerPile.getFittingCard (**ci, ci + 1,
      if (!isJoker (**ci)) {
         if ((next != playerPile.end ())
             && isJoker (*playerPile[playerPile.size () - 1])) {
            TRACE1 ("Buraco::executeMove (unsigned int) - Have two with joker: "
                    << **ci << " and " << **next);
            TRACE1 ("Buraco::executeMove (player) - Have two with joker: "
            Check3 (diff ? (*next)->colour () == (*ci)->colour () : true);
            if (diff < 0) {
               Check3 (diff >= -2);
               playerPile.move (playerPile.size () - 2,
                                next - playerPile.begin ());
               playerPile.move (playerPile.size () + ((diff == -2) ? -1 : -2),
                                ci - playerPile.begin ());
            }
            else {
               Check3 (diff <= 2);
               playerPile.move (playerPile.size () - 1, next - playerPile.begin ());
               playerPile.move (playerPile.size () - 1 - diff,
                                ci - playerPile.begin ());
            }

            // Create a new pile with the found pair and a joker
            makeNewPile (player & 1);
            pos1Play = playerPile.size () - 3;
            pos2Play = playerPile.size () - 1;
            return (tablePiles[player & 1].size () - 1) << 16;
         }
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
         unsigned int size (0);
         for (std::vector<BuracoPile*>::const_iterator p (tablePiles[player & 1].begin ());
              p != tablePiles[player & 1].end (); ++p) {
            if ((((*p)->size () < 7) && ((*p)->getPosJoker () > 6))
                && (((*p)->size () > size)
                    || (((*p)->size () == size)
                        && ((*p)->getPotentialPoints ()
                            > tablePiles[player & 1][bestPile]->getPotentialPoints ()))
                    || ((*p)->getPotentialPoints ()) >= 1000)) {
               bestPile = p - tablePiles[player & 1].begin ();
               size = (*p)->size ();
            }
         }

         if (bestPile != -1U) {
            pos1Play = pos2Play = playerPile.size () - 1;
            unsigned int pos, move;

            Check3 (bestPile < tablePiles[player & 1].size ());
            tablePiles[player & 1][bestPile]->getPosition4Card
                (*playerPile[playerPile.size () - 1], pos, move);
            tablePiles[player & 1][bestPile]->getPosition4Card 
         }
      }
   }

   if (containsOnlyJoker (playerPile))
      if (!reserve[player & 1].empty ()) {
         addBuraco (player);
         return executeMove (player);
      }

   // No more cards to put down: Find a card to dump
   TRACE8 ("Buraco::executeMove (unsigned int) - Searching for a card to dump");
   for (i = 0; i < playerPile.size () - 1; ++i) {
      ICardPile::const_iterator p (playerPile.getFittingCard (*playerPile[i], playerPile.begin (),
                                                              &cardDistance));
      if (static_cast<unsigned int> (p - playerPile.begin ()) == i)
         p = playerPile.getFittingCard (*playerPile[i], ++p, &cardDistance);
      if ((p - playerPile.begin ()) == i)
         break;
   }

   while (i && isJoker (*playerPile[i]))              // Try to not dump jokers
      --i;

   Check3 (i < playerPile.size ());
   pos1Play = pos2Play = i;
   return 0xffff0000;
}

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

   if (randomizeCardsToPile (staple)) {
      for (unsigned int j (0); j < 11; ++j) {
         for (unsigned int i (0); i < NUM_PLAYERS; ++i)
            hands[(i - posServer) & 0x3].setTopCard (staple.removeTopCard ());

         for (unsigned int i (0); i < (sizeof (reserve) / sizeof (reserve[0])); ++i)
            reserve[(i - posServer) & 1].push_back (&staple.removeTopCard ());
      }
      for (unsigned int i (0); i < NUM_PLAYERS; ++i)
         hands[i].sort (compByNumberWithJokers);
      for (unsigned int i (1); i < NUM_PLAYERS; ++i) {
          hands[i].setStyle (ICardPile::QUITE_COMPRESSED);
          hands[i].setShowOption (ICardPile::SHOWBACK);
      }

      dumped.setTopCard (staple.removeTopCard ());

      gStatus.startTurn = gStatus.startGame = 1;
      gStatus.team1Buraco = gStatus.team2Buraco = 0x3;
      status.pop ();
      status.push (_("You can sort the cards in your hand with drag and drop or put"
                     " them on the table - click card to dump to end turn"));

      gStatus.pickUpPlayed = 0;

      unfinishedMonoPiles[0] = unfinishedMonoPiles[1] = 0;
      updateInfo ();
      // Set random startplayer (if not already set)
      if (startPlayer == -1U)
         startPlayer = rand () & 0x3;
      setStartPlayer ();

      if (getConnectionMgr ().getMode () == YGP::ConnectionMgr::CLIENT)
         dumped.getTopCard ().hide ();
   }
}

//----------------------------------------------------------------------------
/// Sets the startplayer; including showing it in the status bar
/// \param player: Player to start the game
//----------------------------------------------------------------------------
void Buraco::setStartPlayer () {
   if (getConnectionMgr ().getMode () != YGP::ConnectionMgr::CLIENT) {
      setNextPlayer (startPlayer);
      broadcastStartPlayer (startPlayer);
   }

      // Send startplayer to the clients
      if (getConnectionMgr ().getMode () == YGP::ConnectionMgr::SERVER) {
         const std::vector<YGP::Socket*>& clients (getConnectionMgr ().getClients ());
         unsigned int player ((currentPlayer () - 1) & 0x3);
         for (std::vector<YGP::Socket*>::const_iterator i (clients.begin ());
              i != clients.end (); ++i) {
            std::ostringstream msg;
            msg << "ActPlayer=" << player;
            writeMessage (**i, msg.str ());
            player = (player + 1) & 0x3;
         }
      }
   if (startPlayer)
      dumped.getTopCard ().hide ();
   else
      dumped.getTopCard ().show ();
   displayTurn (startPlayer++);
   startPlayer &= 0x3;
   makeNextMoves ();
}

//-----------------------------------------------------------------------------
/// Remove cards from everything which can hold them
//-----------------------------------------------------------------------------
void Buraco::clean () {
   TRACE9 ("Buraco::clean ()");
   disableHuman ();
   for (unsigned int i (0); i < NUM_PLAYERS; ++i)
      hands[i].clear ();

   staple.clear ();
   dumped.clear ();

   for (unsigned int i (0); i < (NUM_PLAYERS >> 1); ++i) {
      for (std::vector<BuracoPile*>::iterator p (tablePiles[i].begin ());
           p != tablePiles[i].end (); ++p) {
         boxTeam[i].remove (**p);
         delete *p;
      }
      tablePiles[i].clear ();
   }

   for (unsigned int i (0); i < (sizeof (reserve) / sizeof (reserve[0])); ++i)
      reserve[i].clear ();

   Game::clean ();
}

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
      enableCard (i);
   }
   Check3 (aDNDHand.size () == hands[0].size ());

   TRACE2 ("Buraco::enableHuman () - Human has " << hands[0].size ()
           << " cards");

   newPile.drag_dest_set (dndType, Gtk::DEST_DEFAULT_ALL, Gdk::ACTION_MOVE);
   aDNDTable[NULL] = newPile.signal_drag_data_received ().connect
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
   Game::disableHuman ();
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
      std::ostringstream msg;
      msg << "Play=" << hands[0][iCard]->id () << ";Target=1";

      if (getConnectionMgr ().getMode () == YGP::ConnectionMgr::CLIENT)
         ignoreNextMsg = true;
      broadcastMessage (msg.str ());
   }

   unregisterHandDND (*hands[0][iCard]);
   dumped.append (hands[0].remove (iCard));
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
   setNextPlayer (1);
   displayTurn (1);
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
      std::ostringstream msg;
      msg << "Play=" << staple.getTopCard ().id () << ";Target=2";

      if (getConnectionMgr ().getMode () == YGP::ConnectionMgr::CLIENT)
         ignoreNextMsg = true;
      broadcastMessage (msg.str ());
   }

   dumpedTop.disconnect ();
   stapleTop.disconnect ();

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

   dumped.getTopCard ().show ();
   unsigned int player (currentPlayer ());
   hands[player].append (staple.removeTopCard ());
   if (!player)
      enableHumanHand ();
}

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
      std::ostringstream msg;
      msg << "Play=" << dumped.getTopCard ().id () << ";Target=3";

      if (getConnectionMgr ().getMode () == YGP::ConnectionMgr::CLIENT)
         ignoreNextMsg = true;
      broadcastMessage (msg.str ());
   }

   dumpedTop.disconnect ();
   stapleTop.disconnect ();

   // Special handling of player starting the game and can choose one of the
   // first two cards
   CardWidget& card (dumped.removeTopCard ());
   card.show ();
   if (gStatus.startGame) {
      Check3 (dumped.size () == 0);
      hands[0].append (card);
   }
   else {
      Check3 (pileHasFittingPair (hands[0], card));

      if (getConnectionMgr ().getMode () != YGP::ConnectionMgr::NONE) {
         // Send played card to all clients (if any)
         std::ostringstream msg;
         msg << "Play=" << card.id () << ";Target="
             << (tablePiles[0].size () << 16) + 100;

         if (getConnectionMgr ().getMode () == YGP::ConnectionMgr::CLIENT)
            ++ignoreNextMsg;
         broadcastMessage (msg.str ());
      }

      BuracoPile& pile (makeNewPile (0 & 1));                // Create new pile
      pile.setTopCard (card);                            // with picked up card

      acceptCards = hands[0].size ();;
      if (dumped.size ())
         movePile (hands[0], dumped);
   }

   // Enable the cards in humans hand, when idle (means: *after* this
   // signalhandler terminates)
   Glib::signal_idle ().connect
       (bind_return (mem_fun (*this, &Buraco::enableHumanHand), false));
}
       (bind_return (slot (*this, &Buraco::enableHumanHand), false));
//-----------------------------------------------------------------------------
/// Action after picking up the card from the dumped staple
//-----------------------------------------------------------------------------
void Buraco::doDumpedSelected () {
   TRACE5 ("Buraco::doDumpedSelected () - " << gStatus.startGame);
   Check1 (gameStatus () == PLAYING);
   Check3 (dumped.size ());
   unsigned int player (currentPlayer ());

   dumped.getTopCard ().show ();
   movePile (hands[player], dumped);
}

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
   for (; start <= end; ++start) {
      CardWidget& card (*tmp[start]);
      unregisterTableDND (card);
      registerTableDND (card, pile + start);
   }
}

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
/// \param target, action, ...)
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
      Gtk::MessageDialog dlg (_("Can't move cards before completing the pile!"),
                              Gtk::MESSAGE_ERROR);
      dlg.set_title (_("Invalid move"));
      dlg.run ();
      return;
   }

   context->drag_finish (true, false, time);                     // End old DND

   CardWidget& cardMoved (hands[0].remove (*pValue));
   hands[0].insert (cardMoved, card);                     // Insert moved card

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
      Check3 (*p); Check3 (((*p)->size () < 7) || !(*p)->is_visible ());
      if ((p - tablePiles[0].begin ()) == static_cast<int> (except))
         continue;

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
           << (int)iCard << std::dec);
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
   BuracoPile* pile (NULL);

   if ((acceptCards != -1U)
       && isJoker (moved) || (*pValue >= acceptCards)) {
      context->drag_finish (false, false, time);
      Gtk::MessageDialog dlg (_("You must play your cards (without \"monos\"), when you picked up the pile!"),
                              Gtk::MESSAGE_ERROR);
      dlg.set_title (_("Invalid move"));
      dlg.run ();
      return;
   }

   // Move dropped card to a (new) pile on the table
   unsigned int iPile;
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
            : pileHasFittingPair (hands[0], moved, acceptCards == -1U))) {
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

      // Only allow dropping of last card, if the game can be ended, or there
      // is still the reserve
      if (!canDumpCards (0, 1, iCard >> 8)) {
         context->drag_finish (false, false, time);
         Gtk::MessageDialog dlg (_("You can't end the game (there's no \"cerrado\")!"),
                                 Gtk::MESSAGE_ERROR);
         dlg.set_title (_("Invalid move"));
         dlg.run ();
         return;
      }

      if ((iCard = cardFitsOnPile (iPile, moved)) == -1U) {
         context->drag_finish (false, false, time);
         Gtk::MessageDialog dlg (_("This card does not fit on that pile!"),
                                 Gtk::MESSAGE_ERROR);
         dlg.set_title (_("Invalid move"));
         dlg.run ();
         return;
      }

      if ((pile->size () == 1)
   // End old drag
   context->drag_finish (true, false, time);
   // Send move
   if (getConnectionMgr ().getMode () != YGP::ConnectionMgr::NONE) {
      std::ostringstream msg;
      msg << "Play=" << hands[0][*pValue]->id () << ";Target="
          << (iPile << 16) + iCard + 100;
      if (getConnectionMgr ().getMode () == YGP::ConnectionMgr::CLIENT)
         ignoreNextMsg = true;
      broadcastMessage (msg.str ());
   }

   activeCards[*pValue].disconnect ();
   activeCards.erase (activeCards.begin () + *pValue);

   // Unregister old card
   hands[0].remove (*pValue);
   unregisterHandDND (moved);

   // Insert card into pile and register it for DND
   unsigned int move (-1U);
   pile->getPosition4Card (moved, iCard, move);
   Check3 (iCard <= pile->size ());
   if (move != -1U) {
      Check3 (move <= pile->size ());
      Check3 (move != pile->getPosJoker ());
      Check3 (pile->getPosJoker () != 7);
      sendMoveCard (iPile, pile->getPosJoker (), move);
   }

   // Send move
   if (getConnectionMgr ().getMode () != YGP::ConnectionMgr::NONE) {
   if (iCard < (pile->size () - 1))
      registerTableDND (iPile, iCard + 1, pile->size () - 1);

   TRACE9 ("Buraco::cardDroppedOnTable (...) - Undo:  " << iPile << "; " << iCard
	   << "; " << *pValue << ": " << acceptCards);
      removeCerrado (0, *pile);

   // Accept again the jokers, if the pile has has now three cards (jokers are
   // disabled, if the human picked up the dumped pile.
   if (pile->size () == 3)
      acceptCards = -1U;
   else
      if (acceptCards != -1U)
         --acceptCards;

   // If the player has no more cards left (except of joker): Give him the reserve
   if (containsOnlyJoker (hands[0]) && humanPilesOK ())
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
      registerHandDND (*pValue, hands[0].size () - 1);
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
      Check3 (actPlayers.size () > player);
      Check3 (actPlayers[player]);

      status.pop ();
      Glib::ustring stat (_("%1 picked up the burraco"));
      stat.replace (stat.find ("%1"), 2, actPlayers[player]->getName ());
      status.push (stat);
   status.pop ();

   points[player & 1] += 100;

//-----------------------------------------------------------------------------
/// Hides the joker, which are displayed when picking up the buraco
/// \param pile: Pile holding the jokers shown
/// \returns BuracoPile&: New created pile
//-----------------------------------------------------------------------------
BuracoPile& Buraco::makeNewPile (unsigned int team) {
   TRACE9 ("Buraco::makeNewPile (unsigned int) - New pile for team " << team + 1);
   Check1 (team < (sizeof (boxTeam) / sizeof (boxTeam[0])));
   Check1 (team < (sizeof (tablePiles) / sizeof (tablePiles[0])));

   BuracoPile* pile (new BuracoPile ());
   tablePiles[team].push_back (pile);
   boxTeam[team].pack_start (*pile, Gtk::PACK_SHRINK, 5);

   pile->show ();
   return *pile;
}

//-----------------------------------------------------------------------------
/// Checks if the passed card can be put on one of the existing piles
/// \param player: Player to inspect
/// \param iCard: Card to inspect
/// \returns Value describing the pile (and the offset of the card) to play
///     to; -1 if none
/// \remarks This method moves the card
//-----------------------------------------------------------------------------
unsigned int Buraco::cardFitsOnPlayedPile (unsigned int player, unsigned int iCard) {
   TRACE8 ("Buraco::cardFitsOnPlayedPile (unsigned int, unsigned int) - "
           "Card " << iCard << " of player " << player);
   Check1 (player);
   Check1 (player < NUM_PLAYERS);
   Check1 (iCard < hands[player].size ());
   CardWidget& card (*hands[player][iCard]);
   TRACE3 ("Buraco::cardFitsOnPlayedPile (unsigned int, unsigned int) - Card " << card);

   unsigned int bestPile (-1U);
   unsigned int maxPoints (0);
   unsigned int size (0);
   for (std::vector<BuracoPile*>::iterator p (tablePiles[player & 1].begin ());
        p != tablePiles[player & 1].end (); ++p) {
      TRACE5 ("Buraco::cardFitsOnPlayedPile (unsigned int, unsigned int) - "
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
          ? (((((*p)->size () == 6) && ((*p)->getPosJoker () > 6))
	      && ((hands[player].size () - iCard) < 7)
              && (((reserve[player & 1].empty ()
                         == 0x3)))
                   || (points[player & 1] > 100))
                  || reserve[(player + 1) & 1].empty ()
                  || (points[(player + 1) & 1] > 100)))
             || (((*p)->size () > 2) && ((*p)->getPosFirst () > 6)))
          : ((posPile = cardFitsOnPile (p - tablePiles[player & 1].begin (), card))
             != -1)) {
	 // Always play on a joker pile (don't bother checking for a second one)
	 if ((*p)->getPotentialPoints () >= 1000) {
                 && (maxPoints < (*p)->getPotentialPoints ()))) {
            size = (*p)->size ();
                 && (maxPoints < (*p)->getPotentialPoints ()))
             || ((*p)->getPotentialPoints ()) >= 1000) {
            bestPile = p - tablePiles[player & 1].begin ();
         }
      }
   }

   if (bestPile != -1U) {
      unsigned int pos (0), move (-1U);
      BuracoPile& pile (*tablePiles[player & 1][bestPile]);
      Check3 (pile.getPosition4Card (card, pos, move));

      pile.getPosition4Card (card, pos, move);
      Check3 (pos <= pile.size ());
      if ((move != -1U) && canDumpCards (player, 1, ((int)target) >> 16)) {
         Check3 (move <= pile.size ());
      if (move != -1U) {
	 Check3 (pile.getPosJoker () != 7);
         sendMoveCard (bestPile, pile.getPosJoker (), move);
      }

      pos1Play = pos2Play = iCard;
      return (bestPile << 16) + pos;
   }
   return -1U;
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
/// \param pile: Pile holding the cerrado
//-----------------------------------------------------------------------------
void Buraco::removeCerrado (unsigned int player, BuracoPile& pile) {
   unsigned int team (player & 1);

   Check1 (player < NUM_PLAYERS);
   Check1 ((std::find (tablePiles[team].begin (), tablePiles[team].end (), &pile))
           != tablePiles[team].end ());
   TRACE9 ("Buraco::removeCerrado (unsigned int, BuracoPile&) - Pile "
           << (std::find (tablePiles[team].begin (), tablePiles[team].end (), &pile)
               - tablePiles[team].begin ()) << " of team " << team);
   Check2 (pile.size () == 7);

   pile.hide ();
   Check3 (static_cast<int> (pile.getPotentialPoints ()) == pile.getPoints ());
   points[team] += pile.getPoints ();

   points[team] += pile.getPotentialPoints ();

//-----------------------------------------------------------------------------
/// Actualizes the info-part of the statusbar
//-----------------------------------------------------------------------------
void Buraco::updateInfo () {
   Glib::ustring strInfo (_("Points [Buraco]: %1 [%2] / %3 [%4]"));
   strInfo.replace (strInfo.find ("%1"), 2, YGP::ANumeric::toString (points[0]));
   strInfo.replace (strInfo.find ("%2"), 2, (reserve[0].empty () ? _("N") : _("Y")));
   strInfo.replace (strInfo.find ("%3"), 2, YGP::ANumeric::toString (points[1]));
   strInfo.replace (strInfo.find ("%4"), 2, (reserve[1].empty () ? _("N") : _("Y")));

   info.set_text (strInfo);
}
   info.pop ();
   info.push (strInfo);
//-----------------------------------------------------------------------------
/// Checks if the passed card fits on the passed staple
/// \param iPile: Pile to inspect
/// \param card: Card to check
/// \returns Position where card can be played to, or -1 if card does not fit
//-----------------------------------------------------------------------------
int Buraco::cardFitsOnPile (unsigned int iPile, const CardWidget& card) const {
   Check1 (iPile < tablePiles[currentPlayer () & 1].size ());
   BuracoPile& pile (*tablePiles[currentPlayer () & 1][iPile]);
   Check2 (pile.size ()); Check2 (pile.size () < 7);

   // Card played on a joker: Valid is:
   //   - A joker; if there are at least 3 jokers (on table + in hand)
   //   - Any card, which has a pair (if there's only one joker on the table)
   if (pile.getPosFirst () > 6)
      if (pile.getPosJoker ())
         return isJoker (card) ? 0 : -1;
      else {
         ICardPile::const_iterator pCard
             (hands[currentPlayer ()].getFittingCard (card, &cardDistance));
         if (*pCard == &card)
             pCard = hands[currentPlayer ()].getFittingCard (card, ++pCard,
                                                             &cardDistance);
         return pCard == hands[currentPlayer ()].end () ? -1 : 0;
   }

   unsigned int pos, move;
   if (pile.getPosition4Card (card, pos, move)) {
      Check3 (pos <= pile.size ());
      if ((pile.size () > 1) || isJoker (card))
      else {
         const CardHPile& hand (hands[currentPlayer ()]);
         if (!containsNoJoker (hand))
            return pos;

         CardWidget& pileCard (*pile[pile.getPosFirst ()]);
         int dist (cardDistance (pileCard, card));
         Check3 ((dist > -2) && (dist < 2));
         int cmp (0);

         ICardPile::const_iterator pCard (hand.begin ());
         do {
            pCard = hand.getFittingCard (pileCard, pCard, &cardDistance);
            if (*pCard == &card)
               pCard = hand.getFittingCard (pileCard, ++pCard, &cardDistance);
            if (pCard == hand.end ())
               return -1;

            TRACE8 ("Buraco::cardFitsOnPile (unsigned int, const CardWidget&) const -  "
                    "Dist: " << dist << "<->" << cardDistance (**pCard, card));

            cmp = dist - cardDistance (**pCard, card);
            ++pCard;
         }
         while ((cmp != -dist) & (cmp != (dist << 1)));
         return pos;
      }
   }
   return -1;
}

//----------------------------------------------------------------------------
/// Sends a move-message to the connected machines
/// \param pile: Number of pile involved
/// \param from: Card to move
/// \param to: Position card to move to
//----------------------------------------------------------------------------
void Buraco::sendMoveCard (unsigned int pile, unsigned int from, unsigned int to) const {
   TRACE8 ("Buraco::sendMoveCard (3x unsigned int) - Pile " << pile << ' '
           << from << "->" << to);
   if (getConnectionMgr ().getMode () != YGP::ConnectionMgr::NONE) {
      // Send played card to all clients (if any)
      std::ostringstream msg;
      msg << "Move=" << from << ";To=" << to << ";Pile=" << pile;

      if (getConnectionMgr ().getMode () == YGP::ConnectionMgr::CLIENT)
         const_cast<Buraco*> (this)->ignoreNextMsg = true;
      broadcastMessage (msg.str ());
   }
}


//-----------------------------------------------------------------------------
/// Shows or hides the cards of the computer player
/// \param open: Flag if cards should be shown or hidden
//-----------------------------------------------------------------------------
void Buraco::playOpen (bool open) {
   for (unsigned int i (1); i < NUM_PLAYERS; ++i) {
      hands[i].setShowOption (open ? ICardPile::SHOWFACE : ICardPile::SHOWBACK);
      hands[i].setStyle (open ? ICardPile::COMPRESSED : ICardPile::QUITE_COMPRESSED);
   }
}

//-----------------------------------------------------------------------------
/// Creates the combined team names from the players
/// \param names: Array to receive groups
//-----------------------------------------------------------------------------
void Buraco::makeTeamNames (std::vector<Player*>& names) const {
   Check1 (actPlayers.size () >= NUM_PLAYERS);

   // First delete old names
   for (unsigned int i (0); i < names.size (); ++i)
      delete names[i];
   names.clear ();

   // ... then create it new with pair 0/2; 1/3
   for (unsigned int i (0); i < (NUM_PLAYERS >> 1); ++i) {
      Glib::ustring name (_("Team %1\n%2/%3"));
      name.replace (name.find ("%1"), 2, 1, char ('1' + i));
      name.replace (name.find ("%2"), 2, actPlayers[i]->getName ());
      name.replace (name.find ("%3"), 2, actPlayers[i + 2]->getName ());

      TRACE8 ("Buraco::makeTeamNames (std::vector<Player*>) - Add: " << name);
      names.push_back (new Human (name));
   }
}

//-----------------------------------------------------------------------------
/// Performs the steps to end the game
//-----------------------------------------------------------------------------
void Buraco::endGame () {
   TRACE8 ("Buraco::endGame ()");

   if (!currentPlayer ())
      disableHuman ();

   if (!pScoreDlg) {
      pScoreDlg = ScoreDlg::create (nameTeams);
      pScoreDlg->get_window ()->set_transient_for (get_window ());
   }
   points[0] += reserve[0].empty () ? 100 : -100;
   points[1] += reserve[1].empty () ? 100 : -100;
   // Sum up all cards on the table
   for (unsigned int i (0); i < (NUM_PLAYERS >> 1); ++i) {
      int sum (0);
      int monoPile (0);

      for (std::vector<BuracoPile*>::const_iterator p (tablePiles[i].begin ());
           p != tablePiles[i].end (); ++p) {
         Check3 (*p);
         Check3 ((*p)->size () > 2);

         (*p)->show ();
         // Substract 1000 points for every started cerrado of monos
         if ((*p)->getPoints () < 0)
            monoPile += 1000;
         sum += (*p)->getCardPoints ();
      }

      TRACE5 ("Buraco::endGame () - Points of team " << i << " on table: "
              << sum << '/' << monoPile);
      points[i] = ((points[i] < (reserve[i].size () ? 100 : 300)) ? -sum : sum) - monoPile;
   }
      points[i] = ((points[i] < 200) ? -sum : sum) - monoPile;
   for (unsigned int i (0); i < NUM_PLAYERS; ++i)
      for (std::vector<CardWidget*>::const_iterator c (hands[i].begin ());
           c != hands[i].end (); ++c)
         points[i & 1] -= getPoints (**c);

   pScoreDlg->addPoints (points);
   

   Glib::ustring stat (_("Round ended"));
   unsigned int player;
   int maxPoints;
   pScoreDlg->getMaxPoints (maxPoints, player);
   TRACE9 ("Buraco::endGame () - Points: " << maxPoints);
   if (maxPoints >= (int)ENDPOINTS) {
      stat = _("Game ended; Team %1 won");
   if (maxPoints >= 2000) {
   }

   // Move cards of partners to first player and show them
   for (unsigned int i (1); i < NUM_PLAYERS; ++i) {
      hands[i].setStyle (ICardPile::COMPRESSED);
      hands[i].setShowOption (ICardPile::SHOWFACE);
   }

   status.pop ();
   status.push (stat);

   setGameStatus (STOPPED);
}
   setGameStatus (STOPPED);   

//-----------------------------------------------------------------------------
/// Returns the value of the passed card
/// \param card: Card to inspect
/// \returns \c Value of the card
//-----------------------------------------------------------------------------
unsigned int Buraco::getPoints (const CardWidget& card) {
   // Card:                 2   3  4  5  6  7  8   9   10  J   Q   K   A   Joker
   static char values[] = { 25, 5, 5, 5, 5, 5, 10, 10, 10, 10, 10, 10, 20, 50 };
   Check3 (card.number () < static_cast<int> (sizeof (values) / sizeof (values[0])));
   return values[card.number ()];
}

//-----------------------------------------------------------------------------
/// Checks if the player can get rid of all cards in his hand except of the
/// jokers
/// \param player: Player whose cards should be inspected
/// \returns \c True: if all cards can be played
/// \remarks This method does not check for triplets anymore!
//-----------------------------------------------------------------------------
bool Buraco::canGetRidOfCards (unsigned int player) const {
   TRACE5 ("Buraco::canGetRidOfCards (unsigned int) - Checking player " << player);
   std::bitset<160> used; Check3 (hands[player].size () < used.size ());
   const CardHPile& pile (hands[player]);

   unsigned int cJokers (0);
   unsigned int piles (0);
   for (ICardPile::const_iterator i (pile.begin ()); i != pile.end (); ++i) {
      if (used[i - pile.begin ()])
         continue;

      if (isJoker (**i)) {
         used.set (i - pile.begin ());
         ++cJokers;
         continue;
      }

      // If there are equal cards (and the first card is not already marked as
      // used: Mark both card as used
      ICardPile::const_iterator o (pile.getFittingCard (**i, i + 1, &cardDistance));
      if ((o != pile.end ()) && !used[o - pile.begin ()]) {
         ++piles;
         used.set (i - pile.begin ());
         used.set (o - pile.begin ());
      }
   }

   TRACE9 ("Buraco::canGetRidOfCards (unsigned int) -  " << used.count ()
           << '/' << hands[player].size () << "; " << cJokers << " Joker for "
           << piles << " piles -> "
           << '/' << hands[player].size () << "; Joker: " << cJokers);
   return (((used.count () + 2) >= hands[player].size ())

//-----------------------------------------------------------------------------
/// Checks if the player can dump the specified number of cards; a player can
/// only dump all of his cards, if:
///   - The team has a cerrado
/// only dump all of his cards, if: - The team has a cerrado - The team still
/// has the reserve
/// \param pile: Pile player is going to play its card to (or -1 for a new one)
/// \returns \c True, if card can be played
//-----------------------------------------------------------------------------
bool Buraco::canDumpCards (unsigned int player, unsigned int cards,
                           unsigned int pile) const {
   TRACE7 ("Buraco::canDumpCards (3x unsigned int) - Player "
           << player << " playing " << cards << " cards to " << pile);
   Check1 ((pile == -1U) || (tablePiles[player & 1].size () > pile));
   Check1 ((pile == -1U)
           || ((tablePiles[player & 1][pile]->size () + cards) <= 7));
   Check1 (hands[player].size () >= cards);
   Check1 (cards <= 7);

   bool enoughCards ((hands[player].size () > (cards + 1))
		     || reserve[player & 1].size ());
   return ((hands[player].size () > (cards + 1))
           || (points[player & 1] > 100)
           || reserve[player & 1].size ()
           || ((pile != -1U)
               && (((tablePiles[player & 1][pile]->size () + cards) >= 7)
                   || (((tablePiles[player & 1][pile]->size () + cards) == 6)
                       && ((hands[player].size () - cards) == 1)
                       && canClosePile (player, pile))))
           || (cards >= 7));
//----------------------------------------------------------------------------
/// Checks if the player can with his two cards left close the passed pile
/// \param player: Player to inspect
/// \param pile: Pile to analyze
/// \return bool: True, if the remaining cards of the player can make a
///        cerrado for this pile
/// \remarks: - The player must have only two cards; the pile 5
///        cerrado for this pile 
/// \remarks: - The player must have only two cards; the pile 5 
//---------------------------------------------------------------------------- 
           << player << " closes pile " << pile);
   Check1 (tablePiles[player & 1].size () > pile);

   BuracoPile& orig (*tablePiles[player & 1][pile]);
   Check1 (orig.size () == 5);
   Check1 (hands[player].size () == 2);

   bool isOK (false);
   // Make a copy of the original pile
   BuracoPile copy;
   for (BuracoPile::const_iterator i (orig.begin ()); i != orig.end (); ++i)
       copy.append (*new CardWidget (**i));

   unsigned int pos, move;
   for (unsigned int i (0); i < 2; ++i) {
       if (copy.getPosition4Card (*hands[player][i], pos, move)) {
          Check3 (pos <= copy.size ());
          if (move != -1U) {
             Check3 (move <= copy.size ());
             Check3 (move != copy.getPosJoker ());
             copy.move (move, copy.getPosJoker ());
          }
	  copy.insert (*new CardWidget (*hands[player][i]), pos);

             isOK = true;
             break;
          }
	  else
	     delete &copy.remove (pos);

   // Free the copy
   for (BuracoPile::const_iterator i (copy.begin ()); i != copy.end (); ++i)
      delete *i;

   return isOK;
}

//-----------------------------------------------------------------------------
/// Checks if the passed pile contains a pair matching the passed card
/// \param pile: Pile to inspect
/// \param card: Card where to find a pair to
/// \param withJokers: Flag, if jokers should be inspected
/// \returns \c True, if the pile contains a matching pair
//-----------------------------------------------------------------------------
bool Buraco::pileHasFittingPair (const ICardPile& pile, const CardWidget& card,
                                 bool withJokers) {
   TRACE3 ("Buraco::pileHasFittingPair (const ICardPile&, const CardWidget*,"
           " bool) - " << card);

   if (withJokers) {
      ICardPile::const_iterator i (pile.getFittingCard (card, pile.begin (),
                                                        &cardDistance));
      if (*i == &card)
         i = pile.getFittingCard (card, ++i, &cardDistance);
      if ((i != pile.end ()) && !containsNoJoker (pile))
         return true;
   }

   return pile.hasFittingPair (card, &cardDistance);
}

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
      if (*p != exclude)
         if ((pile.getFittingCard (**p, pile.begin (), &cardDistance) != p)
      if ((pile.getFittingCard (**p, pile.begin (), &cardDistance) != p)
          || (pile.getFittingCard (**p, p + 1, &cardDistance) != pile.end ()))
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

   return values[a->number ()] < values[b->number ()];
}

//----------------------------------------------------------------------------
/// Returns the distance between two cards. The ace also counts as one (if the
/// other card is a 3 or a 4) and 2's are equal to jokers.
/// \param a: Card to compare
/// \param b: Card to compare
/// \returns \c int: Distance of the two passed cards (a - b)
//----------------------------------------------------------------------------
int Buraco::cardDistance (const CardWidget& a, const CardWidget& b) {
   return cardDistance (a, b, true);
}

//----------------------------------------------------------------------------
/// Returns the distance between two cards. The ace also counts as one (if the
/// other card is a 3 or a 4) and 2's are equal to jokers.
/// \param a: Card to compare
/// \param b: Card to compare
/// \param aceIsOne: Flag, if aces should (also) be treated as one
/// \returns \c int: Distance of the two passed cards (a - b)
//----------------------------------------------------------------------------
int Buraco::cardDistance (const CardWidget& a, const CardWidget& b, bool aceIsOne) {
   TRACE9 ("Buraco::cardDistance (2x const CardWidget&, bool) - "
           << a << "<->" << b);
   TRACE9 ("Buraco::cardDistance (const CardWidget&, const CardWidget&, bool) - "
   bool aJoker (isJoker (a));
   bool bJoker (isJoker (b));
   if (aJoker || bJoker)
      return aJoker && bJoker ? 0 : 99;

   if (a.colour () != b.colour ())
      return (a.number () == b.number ()) ? 0 : 99;
      if ((a.number () == CardWidget::ACE)
      TRACE9 ("Buraco::cardDistance (const CardWidget&, const CardWidget&, bool) - "
              "Checking for Ace");
         return -static_cast<int> (b.number ());
          && (b.number () <= CardWidget::FOUR))
               && (a.number () < CardWidget::EIGHT))
         return static_cast<int> (a.number ());
               && (a.number () <= CardWidget::FOUR))

   TRACE4 ("Buraco::cardDistance (2x const CardWidget&, bool) - "
           "Distance: " << a.number () - b.number ());
   TRACE9 ("Buraco::cardDistance (const CardWidget&, const CardWidget&, bool) - "
}

//----------------------------------------------------------------------------
/// Changes the names of the playing people
/// \param newPlayer: Array holding the new player
//----------------------------------------------------------------------------
void Buraco::changeNames (const std::vector<Player*>& newPlayer) {
   Game::changeNames (newPlayer);
   makeTeamNames (nameTeams);

   for (unsigned int i (0); i < NUM_PLAYERS; ++i) {
      TRACE1 ("Buraco::changeNames () " << i << ": " << newPlayer[i]->getName ());
      names[i].set_text (newPlayer[i]->getName ());
   }

   if (pScoreDlg)
      pScoreDlg->update (nameTeams);
}

//----------------------------------------------------------------------------
/// Returns the passed pile of the player
/// \param player: Number of player
/// \param pile: ID of the pile to return
/// \returns ICardPile*: Pointer to pile to use or NULL
//----------------------------------------------------------------------------
   if (((pile > 3) && (pile < 100)) || (player >= NUM_PLAYERS))
ICardPile& Buraco::getPileOfPlayer (unsigned int player, unsigned int pile) {
   Check1 (player < NUM_PLAYERS);
   Check1 ((pile < 4) || (pile >= 100));
         pile -= 100;
       if (pile != 0xffff0000) {
          pile -= 100;
          if ((pile >> 16) == tablePiles[player & 1].size ()) {
             TRACE8 ("Buraco::getPileOfPlayer (unsigned int, unsigned int) - Creating pile");
             makeNewPile (player & 1);
          }
       }
       target = pile;
       return hands[player];
            ? static_cast<ICardPile&> (staple)
   return ((pile == 2)
           ? static_cast<ICardPile&> (staple)
           : ((pile == 3)
              ? static_cast<ICardPile&> (dumped)
              : static_cast<ICardPile&> (hands[player])));
//----------------------------------------------------------------------------
/// Handles the messages the server might send for the Buraco cardgame
/// \param player: ID of the player sending the message
/// Handles the messages the server might send for the twopart cardgame
/// \returns bool: True, if message has been processed completey
/// \throw std::string: In case of an error an describing text
//----------------------------------------------------------------------------
   TRACE1 ("Buraco::handleMessage (unsigned int player, const std::string&) - "
bool Buraco::handleMessage (unsigned int player, const char* message) {
   TRACE1 ("Buraco::handleMessage (unsigned int player, const char*) - "
   YGP::Tokenize command (message);
    

   bool rc (true);
   if (cmd == "Undo") {
      // Inform clients about cards to play
   if (cmd == "Move") {
      ATTRIBUTE (ap, unsigned int, card, "Move");
      ATTRIBUTE (ap, unsigned int, dest, "To");
      ATTRIBUTE (ap, unsigned int, iPile, "Pile");
      ap.assignValues (message);

      if (iPile >= tablePiles[currentPlayer () & 1].size ())
         throw std::string ("Invalid pile!");
      ICardPile& pile (*tablePiles[currentPlayer () & 1][iPile]);
      if ((card >= pile.size ()) || (dest >= pile.size ()))
         throw std::string ("Invalid card!");

      pile.move (dest, card);
   }
   else {
      rc = Game::handleMessage (player, message);
      if (cmd == "ActPlayer") {
         TRACE1 ("Buraco::handleMessage (unsigned int player, const std::string&) - "
                 "Next player: " << currentPlayer ());
         TRACE1 ("Buraco::handleMessage (unsigned int player, const char*) - "
         startPlayer = currentPlayer ();
         setStartPlayer ();
      }
   }
   return rc;
}

//----------------------------------------------------------------------------
/// Executes the remote move locally
/// \param pile: Pile to move to/from
/// \param dest: ID of target as send by the partner
/// \returns bool: True, if the timer to execute the move should be set
/// \param target: ID of target as send by the partner
/// \remarks
///    - \c dest == 0: Computerplayer playing its card
///    - \c dest == 1: Play from hand to dumped
///    - \c dest == 2: Pick up from staple
///    - \c dest == 3: Pick up from dumped pile
///    - \c dest == 100 + pile/card: Played from hand to table pile \c pile
//----------------------------------------------------------------------------
bool Buraco::executeRemoteMove (ICardPile& pile, unsigned int dest) {
   TRACE8 ("Buraco::executeRemoteMove (ICardPile&, unsigned int) - " << dest
           << "; Target: " << target);
   Check1 ((dest < 4) || (dest >= 100));
   Check2 (pos1Play != -1U);
   Check2 (pos2Play != -1U);

   switch (dest) {
   case 1:
      target = 0xffff0000;
      gStatus.startGame = gStatus.startTurn = 0;
      break;

   case 2:
      Check3 (staple.size ());
      pos1Play = pos2Play = -1U;
      doStapleSelected ();
      return false;

   case 3:
      Check3 (dumped.size ());
      pos1Play = pos2Play = -1U;
      doDumpedSelected ();
      return false;

   default:
      if (dest != 0xffff0000) {
         dest -= 100;
         if (((dest >> 16) >= tablePiles[currentPlayer () & 1].size ())
             || (tablePiles[currentPlayer () & 1][dest >> 16]->size ()
                 < (dest & 0xffff)))
            throw std::string ("Invalid target specification!");

	 Check3 (pos1Play != -1U); Check3 (pos2Play != -1U);
   }

   return Game::executeRemoteMove (pile, dest);
}

//----------------------------------------------------------------------------
/// Returns the actual target, where flipCard2Play should position the cards to
/// \returns unsigned int: ID of the target
//----------------------------------------------------------------------------
unsigned int Buraco::getActTarget () const {
   return (target == 0xffff0000) ? 0xffff0000 : (target + 100);
}
