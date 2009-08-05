//$Id$

//PROJECT     : Cardgames
//SUBSYSTEM   : Buraco
//REFERENCES  :
//TODO        :
//BUGS        :
//REVISION    : $Revision$
//AUTHOR      : Markus Schwab
//CREATED     : 24.02.2003
//COPYRIGHT   : Copyright (C) 2003 - 2009

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

#include <cstdlib>

#include <bitset>
#include <sstream>

#include <gtk/gtkdnd.h>

#include <gtkmm/stock.h>
#include <gtkmm/statusbar.h>
#include <gtkmm/messagedialog.h>
#include <gtkmm/scrolledwindow.h>

#include <YGP/Check.h>
#include <YGP/Trace.h>
#include <YGP/ConnMgr.h>
#include <YGP/ANumeric.h>
#include <YGP/Tokenize.h>
#include <YGP/AttrParse.h>

#include <Human.h>
#include <ScoreDlg.h>
#include <CardImgs.h>
#include <CardWindow.h>
#include <ComputerPlayer.h>

#include "Buraco.h"


std::vector<Gtk::TargetEntry> Buraco::dndType;


unsigned int Buraco::ENDPOINTS (2000);
unsigned int Buraco::CARDS2DEAL (11);


//-----------------------------------------------------------------------------
/// Constructor
/// \param parent Parent widget to display the game in
/// \param statusbar Status bar widget to display information about the game
/// \param cardset Cardset to use
/// \param player Vector of player
/// \param posPlayer Position of player for the server
/// \param mxSerialize Mutex to serialize messages from the server
//-----------------------------------------------------------------------------
Buraco::Buraco (Gtk::Box& parent, Gtk::Statusbar& statusbar,
                CardSet& cardset, const std::vector<Player*>& player,
                unsigned int posPlayer, YGP::Mutex& mxSerialize)
   : Game (parent, statusbar, cardset, player, posPlayer, mxSerialize, 3, 10),
     nameTeams (), startPlayer (-1U), info (), newPile (_("New pile")),
     staple (ICardPile::TOTALLY_COMPRESSED, ICardPile::SHOWBACK),
     dumped (ICardPile::TOTALLY_COMPRESSED, ICardPile::SHOWFACE),
     dumpedTop (), stapleTop (), aDNDHand (), aDNDTable (), gStatus (),
     undo (), pScoreDlg (NULL), idMrg (), menuUndo (), menuSort (), menuSort2 (),
     menuShowScoreDlg () {
   TRACE9 ("Buraco::Buraco (Box&, Statusbar&, CardSet&, const "
           "std::vector<Glib::ustring>&)");

   for (unsigned int i (0); i < (NUM_PLAYERS >> 1); ++i) {
       scrlTable[i] = new Gtk::ScrolledWindow ();
       scrlTable[i]->add (boxTeam[i]);
       scrlTable[i]->set_policy (Gtk::POLICY_AUTOMATIC, Gtk::POLICY_AUTOMATIC);
       scrlTable[i]->show ();
   }

   TRACE9 ("Buraco::Buraco (Box&, Statusbar&, CardSet&, const "
           "std::vector<Glib::ustring>&) - Init common staples");

   boxTeam[0].pack_end (newPile, Gtk::PACK_EXPAND_WIDGET, 5);

   for (unsigned int i (1); i < NUM_PLAYERS; ++i) {
      attach (hands[i], (3 - i) << 2, ((3 - i) << 2) + 2, 0, 1,
              Gtk::EXPAND, Gtk::SHRINK, 5, 5);
      attach (names[i], (3 - i) << 2, ((3 - i) << 2) + 2, 1, 2,
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
   attach (hands[0], 3, 10, 4, 5, Gtk::EXPAND, Gtk::SHRINK, 1, 5);
   attach (names[0], 3, 10, 5, 6, Gtk::EXPAND, Gtk::SHRINK, 1, 5);
   attach (staple, 0, 1, 4, 6, Gtk::SHRINK, Gtk::SHRINK, 5);
   attach (dumped, 1, 2, 4, 6, Gtk::SHRINK, Gtk::SHRINK, 1, 5);
   attach (*scrlTable[1], 0, 10, 2, 3, Gtk::EXPAND | Gtk::FILL,
           Gtk::EXPAND | Gtk::FILL, 0, 5);
   attach (*scrlTable[0], 0, 10, 3, 4, Gtk::EXPAND | Gtk::FILL,
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

   Gtk::Frame* frameInfo (new Gtk::Frame);
   statusbar.pack_end (*manage (frameInfo), Gtk::PACK_SHRINK, 5);
   frameInfo->set_shadow_type (Gtk::SHADOW_IN);
   frameInfo->show ();
   info.show ();
   frameInfo->add (info);

   changeNames (player);
   resizeCards ();
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
/// \param team Team to inspect
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
/// \param player Actual player
//-----------------------------------------------------------------------------
void Buraco::makeMove (unsigned int player) {
   TRACE5 ("Buraco::makeMove (unsigned int) - Turn of player " << player);
   Check1 (player); Check1 (player < NUM_PLAYERS);

   if (cleanup ())                    // Cleanup; end if game has been finished
      return;

   // Turn back jokers
   if (undo.pickUp) {
      unsigned int oldPlayer (gStatus.startTurn ? ((player - 1) & 0x3) : player);
      CardHPile* pile (&hands[oldPlayer]);

      TRACE5 ("Buraco::makeMove (unsigned int) - Player with monos: " << oldPlayer);
      if (oldPlayer && (pile->size () > CARDS2DEAL)) {
	 Check3 (pile->size () > CARDS2DEAL);
	 showJoker (pile, pile->size () - CARDS2DEAL, false);
      }
      undo.pickUp = 0;
   }

   if (gStatus.startTurn && (gameStatus () == STOPPED))
      return;
   playCards ();
}

#include <sys/time.h>
#include <YGP/ATime.h>
#include <iomanip>
#define TIME(x) YGP::ATime ((time_t)x.tv_sec, false).toString ("%X") << ':' << std::setw (6) << std::setfill ('0') << x.tv_usec

//-----------------------------------------------------------------------------
/// Cleanup of piles after each turn
/// \returns bool True, if game has been ended
//-----------------------------------------------------------------------------
bool Buraco::cleanup () {
   unsigned int player (currentPlayer ());
   if (gStatus.startTurn)
      player = (player + NUM_PLAYERS - 1) & 0x3;
   TRACE5 ("Buraco::cleanup () - " << player);

   // First cleanup cerrado made in the last turn
   cleanCerrado (player);

   ICardPile& source (hands[player]);
   if (gStatus.pickUpPlayed) {
      Check3 (dumped.size ());
      gStatus.pickUpPlayed = 0;
      source.getCards (dumped);
      hands[player].sort (compByNumberWithJokers);
   }

   if (containsOnlyJoker (source)) {
      if (reserve[player & 1].size ())
	 addBuraco (player);
      else
	 if (source.empty ()) {
	    Check3 (points[player & 1] > 100);
	    points[player & 1] += 100;
	    endGame ();
	    return true;
	 }
   }
   return false;
}

//-----------------------------------------------------------------------------
/// Searches for cards to play and shows them in the hand of the
/// actual player. They are moved to the end and then animated to its target
//-----------------------------------------------------------------------------
void Buraco::playCards () {
   unsigned int player (currentPlayer ());
   TRACE2 ("Buraco::playCards () - " << player << " ("
           << gStatus.startGame << '/' << gStatus.startTurn << ')');
   Check1 (gameStatus () == PLAYING);
   Check1 (!hands[player].empty ());

   if (gStatus.startTurn
       && (((player & 1) ? gStatus.team2Buraco : gStatus.team1Buraco)
	   == (player >> 1)))
      ((player & 1) ? gStatus.team2Buraco : gStatus.team1Buraco) = 0x3;

   ICardPile& playerPile (hands[player]);
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
                 || (dumped.size () + playerPile.size ()) > 4))) {
         if (getConnectionMgr ().getMode () != YGP::ConnectionMgr::NONE) {
            // Send played card to all clients (if any)
            std::ostringstream msg;
            msg << "Play=" << dumpedCard.id () << ";Target=3";

            if (getConnectionMgr ().getMode () == YGP::ConnectionMgr::CLIENT)
               ignoreNextMsg = true;
            broadcastMessage (msg.str ());
         }

	 playerPile.insertSorted (dumped.removeTopCard (), compByNumberWithJokers);
         if (!gStatus.startGame) {
	    if (dumped.size ())
	       gStatus.pickUpPlayed = 1;

            std::map<unsigned int, unsigned int> aPos;
            std::vector<unsigned int> aOrder;
            unsigned int nrs (playerPile.getSeries (dumpedCard, aPos, aOrder,
                                                    &cardDistance));
            TRACE8 ("Buraco::playCards () - Sizes: " << nrs << "<->" << aPos.size ());
            Check3 ((nrs >= 3) || (aPos.size () >= 3));

            if (nrs > 7)
               nrs = 7;

	    unsigned int pos1Play, pos2Play;
	    unsigned int posTarget;
            if (nrs < aPos.size ()) {
	       nrs = aPos.size ();
	       pos1Play = playerPile.sortColourSerie (aPos, aOrder);
	       for (posTarget = pos1Play; posTarget < (pos1Play + nrs); ++posTarget)
		  if (playerPile[posTarget] == &dumpedCard)
		     break;
	       Check2 (posTarget < (pos1Play + nrs));
	       posTarget -= pos1Play;
	       TRACE1 ("posTarget: " << posTarget);
	    }
	    else {
	       posTarget = nrs - 1;
	       pos1Play = playerPile.find (dumpedCard, compByNumberWithJokers);
	    }
            pos2Play = pos1Play + nrs - 2;
            Check3 ((pos2Play - pos1Play) >= 1);

	    // Put taken card back for animation
	    dumped.setTopCard (playerPile.remove (dumpedCard));
	    flipCards2Play (playerPile, pos1Play, pos2Play);

            BuracoPile& newPile (makeNewPile (player & 1));
	    CardPileWindows& win (animateCards2 (newPile, playerPile, pos1Play, pos2Play));
	    win.sigAnimation.connect (mem_fun (*this, &Buraco::makeNextMoves));
	    win.addWindow (posTarget, dumped, dumped.size () - 1, dumped.size () - 1);
	    return;
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

         playerPile.insertSorted (staple.removeTopCard (), compByNumberWithJokers);
      }

      gStatus.startGame = 0;
      dumpedCard.show ();
   }

   unsigned int pos1Play, pos2Play;
   unsigned int target (executeMove (player, pos1Play, pos2Play));
   TRACE1 ("Buraco::playCards () - " << pos1Play << '/' << pos2Play << " -> " << std::hex << target << std::dec);

   flipCards2Play (playerPile, pos1Play, pos2Play);
   Check3 (pos1Play <= pos2Play); Check3 (pos2Play < hands[player].size ());
   if (target != -1U)
      animateCards (*tablePiles[player & 1][target >> 16], target & 0xff,
		    playerPile, pos1Play, pos2Play)
	 .sigAnimation.connect (mem_fun (*this, &Buraco::makeNextMoves));
   else {
      Check3 (pos1Play == pos2Play);
      gStatus.startTurn = 1;

      animateCard (dumped, playerPile, pos1Play)
	 .sigAnimation.connect (mem_fun (*this, &Buraco::makeNextMoves));

      ++player &= 0x3;
      displayTurn (player);
      setNextPlayer (player);
   }
}

//-----------------------------------------------------------------------------
/// Executes a move for the passed player; only one move is made at every
/// timer-iteration
/// \param player Actual player
/// \param pos1Play First card to play
/// \param pos2Play Last card to play
/// \returns int ID for target (32 Bit: Pile << 16 + Position)
//-----------------------------------------------------------------------------
int Buraco::executeMove (unsigned int player, unsigned int& pos1Play, unsigned int& pos2Play) {
   TRACE6 ("Buraco::executeMove (player) - " << player);

   ICardPile& playerPile (hands[player]);
   unsigned int target;

   // Check if any card can be added to an existing pile
   for (ICardPile::const_iterator p (playerPile.begin ());
	p != playerPile.end (); ++p) {
      TRACE8 ("Buraco::executeMove (unsigned int) - Adding card " << **p << '?');
      target = cardFitsOnPlayedPile (player, p - playerPile.begin ());
      if ((target != -1U) && canPlayCards (player, 1, ((int)target) >> 16)) {
	 pos1Play = pos2Play = p - playerPile.begin ();
	 return target;
      }
   }

   // Check for 3 cards belonging to a serie
   unsigned int i (0);
   for (; i < playerPile.size (); ++i) {
      TRACE8 ("Buraco::executeMove (unsigned int) - Analysing card " << *playerPile[i]);

      std::map<unsigned int, unsigned int> aPos;                   // diff, pos
      std::vector<unsigned int> aOrder;
      unsigned int nrs (playerPile.getSeries (*playerPile[i], aPos, aOrder,
                                              &cardDistance));

      // Play found cards (if any)
      //   - Play jokers if there are at least 5 and the team has still the
      //     reserve and the other team has no burraco and the reserve
      //   - Play the bigger of the found matching cards, if there are >= 3
      if (isJoker (*playerPile[i])
          ? ((((nrs > 4) && reserve[player & 1].size ())
              || (nrs > 5))
             && ((points[(player + 1) & 1] < 101)
                 || (nrs > 6)
		 || ((hands[(player + 2) % 3].size () > 5)
		     && (hands[(player + 1) % 3].size () > 3))))
          : ((nrs > aPos.size ()) ? (nrs > 2) : (aPos.size () > 2))) {
	 unsigned int firstPos (i);
         if (nrs < aPos.size ()) {
            firstPos = playerPile.sortColourSerie (aPos, aOrder);
            nrs = aPos.size ();
         }
         if (nrs > 7)
            nrs = 7;

	 bool canPlay (canPlayCards (player, nrs));
	 if (canPlay || (nrs > 5)) {
	    if (!canPlay)
	       nrs = 3;

	    if (isJoker (*playerPile[firstPos]))
	       ++unfinishedMonoPiles[player & 1];

	    // Create new pile with the found cards
	    makeNewPile (player & 1);
	    pos1Play = firstPos;
	    pos2Play = firstPos + nrs - 1;
	    return (tablePiles[player & 1].size () - 1) << 16;
	 }
	 else
	    hands[player].sort (compByNumberWithJokers);
      }
   }

   // Check if all cards in the hand can (and should) be played
   TRACE8 ("Buraco::executeMove (unsigned int) - Playing all?");
   if (!unfinishedMonoPiles[player & 1]
       && ((points[player & 1] > 100)
	   || (reserve[player & 1].size () && canGetRidOfCards (player)))) {
      ICardPile::const_iterator ci (playerPile.begin ());
      // If pile still has normal cards (no joker)
      while (!((ci == playerPile.end ()) || isJoker (**ci))) {
	 ICardPile::const_iterator next (playerPile.getFittingCard (**ci, ci + 1,
                                                                    &cardDistance));
	 if ((next != playerPile.end ())
             && isJoker (*playerPile[playerPile.size () - 1])) {
            TRACE1 ("Buraco::executeMove (unsigned int) - Have two with joker: "
                    << **ci << " and " << **next);
            int diff (cardDistance (**next, **ci));
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
   }

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
            return (bestPile << 16) + pos;
         }
      }
   }

   // No more cards to put down: Find a card to dump
   TRACE8 ("Buraco::executeMove (unsigned int) - Searching for a card to dump");
   for (i = 0; i < playerPile.size () - 1; ++i) {
      ICardPile::const_iterator p (playerPile.getFittingCard (*playerPile[i], playerPile.begin (),
                                                              &cardDistance));
      if (static_cast<unsigned int> (p - playerPile.begin ()) == i)
         p = playerPile.getFittingCard (*playerPile[i], ++p, &cardDistance);
      if (p == playerPile.end ())
         break;
   }

   while (i && isJoker (*playerPile[i]))              // Try to not dump jokers
      --i;

   Check3 (i < playerPile.size ());
   pos1Play = pos2Play = i;
   return -1U;
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
	 menuShowScoreDlg->set_sensitive (false);
         delete pScoreDlg;
         pScoreDlg = NULL;
      }
   }

   if (randomiseCardsToPile (staple)) {
      for (unsigned int i (0); i < NUM_PLAYERS; ++i)
	 hands[(i - posServer) & 0x3].getCards (staple, staple.size () - CARDS2DEAL - 1, staple.size () - 1);

      for (unsigned int i (0); i < (sizeof (reserve) / sizeof (reserve[0])); ++i)
	 for (unsigned int j (0); j < CARDS2DEAL; ++j)
	    reserve[(i - posServer) & 1].push_back (&staple.removeTopCard ());

      for (unsigned int i (0); i < NUM_PLAYERS; ++i)
         hands[i].sort (compByNumberWithJokers);
      for (unsigned int i (1); i < NUM_PLAYERS; ++i) {
          hands[i].setStyle (ICardPile::QUITE_COMPRESSED);
          hands[i].setShowOption (ICardPile::SHOWBACK);
      }

      dumped.setTopCard (staple.removeTopCard ());

      gStatus.startTurn = gStatus.startGame = 1;
      gStatus.team1Buraco = gStatus.team2Buraco = 0x3;
      gStatus.pickUpPlayed = 0;

      points[0] = points[1] = 0;
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
/// \param player Player to start the game
//----------------------------------------------------------------------------
void Buraco::setStartPlayer () {
   if (getConnectionMgr ().getMode () != YGP::ConnectionMgr::CLIENT) {
      setNextPlayer (startPlayer);
      broadcastStartPlayer (startPlayer);
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
/// \returns bool False
//-----------------------------------------------------------------------------
bool Buraco::enableHuman () {
   Check3 (stapleTop.empty ()); Check3 (dumpedTop.empty ());

   if (gameStatus () == PLAYING)
      cleanup ();

   if (staple.size ())
      stapleTop = staple.getTopCard ().signal_clicked ().connect
	 (mem_fun (*this, (&Buraco::stapleSelected)));
   if (dumped.size ())
      dumpedTop = dumped.getTopCard ().signal_clicked ().connect
	 (mem_fun (*this, (&Buraco::dumpedSelected)));

   return false;
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

   for (unsigned int i (0); i < tablePiles[0].size (); ++i) {
      Check3 (tablePiles[0][i]);
      for (unsigned int j (0); j < tablePiles[0][i]->size (); ++j)
         registerTableDND (*(*tablePiles[0][i])[j], (i << 8) + j);
   }

   menuSort->set_sensitive ();
   menuSort2->set_sensitive ();
}

//-----------------------------------------------------------------------------
/// Disables the cards the human player can select
/// This method must not assume that cards are activated
//-----------------------------------------------------------------------------
void Buraco::disableHuman () {
   TRACE2 ("Buraco::disableHuman () - DND: " << aDNDHand.size () << "; "
           << aDNDTable.size ());
   Game::disableHuman ();
   menuSort->set_sensitive (false);
   menuSort2->set_sensitive (false);

   newPile.drag_dest_unset ();

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
/// \param iCard Offset of card in hand
//-----------------------------------------------------------------------------
void Buraco::cardSelected (unsigned int iCard) {
   TRACE5 ("Buraco::cardSelected (unsigned int) - Position " << iCard);
   Check1 (iCard < hands[0].size ());
   Check1 (gameStatus () == PLAYING);
   gStatus.startGame = 0;

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
   animateCard (dumped, hands[0], iCard)
      .sigAnimation.connect (mem_fun (*this, &Buraco::makeNextMoves));
   menuUndo->set_sensitive (false);

   // If the player has no more cards left (except of jokers): Give him the
   // reserve
   if (containsOnlyJoker (hands[0])) {
      if (!reserve[0].empty ()) {
	 Game::disableHuman ();
         addBuraco (0);
      }
      else if (hands[0].empty ()) {
         points[0] += 100;
         endGame ();
         return;
      }
   }

   gStatus.startTurn = 1;
   setNextPlayer (1);
   displayTurn (1);
}

//-----------------------------------------------------------------------------
/// Callback after clicking on the staple
//-----------------------------------------------------------------------------
void Buraco::stapleSelected () {
   TRACE5 ("Buraco::stapleSelected ()");
   Check3 (staple.size ()); Check3 (stapleTop.connected ());

   dumpedTop.disconnect ();
   stapleTop.disconnect ();

   // Move top card to human and enable the cards in his hand, when idle
   // (means: *after* this signalhandler terminates)
   Glib::signal_idle ().connect
      (bind_return (mem_fun (*this, &Buraco::doStapleSelected), false));
}

//-----------------------------------------------------------------------------
/// Delayed callback after clicking on the staple
//-----------------------------------------------------------------------------
void Buraco::doStapleSelected () {
   TRACE5 ("Buraco::doStapleSelected ()");
   Check2 (staple.size ());

   if (gameStatus () != STOPPED) {
      if (getConnectionMgr ().getMode () != YGP::ConnectionMgr::NONE) {
	 // Send played card to all clients (if any)
	 std::ostringstream msg;
	 msg << "Play=" << staple.getTopCard ().id () << ";Target=2";

	 if (getConnectionMgr ().getMode () == YGP::ConnectionMgr::CLIENT)
	    ignoreNextMsg = true;
	 broadcastMessage (msg.str ());
      }

      animateCard (hands[0], staple, staple.size () - 1)
	 .sigAnimation.connect (mem_fun (*this, &Buraco::enableHumanHand));
   }
   else {
      dumped.append (staple.removeTopCard ());
      enableHuman ();
   }
}

//-----------------------------------------------------------------------------
/// Callback after clicking on the dumped staple
//-----------------------------------------------------------------------------
void Buraco::dumpedSelected () {
   TRACE5 ("Buraco::dumpedSelected ()");
   Check3 (dumped.size ());
   Check3 (stapleTop.connected ()); Check3 (dumpedTop.connected ());
   Check2 (!gStatus.pickUpPlayed);
   disableHuman ();

   // Move top card to human and enable the cards in his hand, when idle
   // (means: *after* this signalhandler terminates)
   Glib::signal_idle ().connect
      (bind_return (mem_fun (*this, &Buraco::doDelayedDumpedSelected), false));
}

//-----------------------------------------------------------------------------
/// Handles the user clicking the top card on the dumped staple
/// This is intened to be called after the callback has been de-registered to
/// to prevent side-effects caused by timing-issues
//-----------------------------------------------------------------------------
void Buraco::doDelayedDumpedSelected () {
   TRACE5 ("Buraco::doDelayedDumpedSelected ()");

   if (gameStatus () != STOPPED) {
      ICardPile* target (NULL);
      CardWidget& card (dumped.getTopCard ());
      if (!gStatus.startGame)
	 try {
	    if (isJoker (card))
	       throw _("You can't pick up monos!");

	    if (!pileHasFittingPair (hands[0], card))
	       throw _("You need a fitting pair to pick up the pile of dumped cards!");

	    // Don't allow picking up the pile, if that would force the game
	    // to end without having neither buraco nor reserve
	    if (((dumped.size () + hands[0].size ()) < 5)
		&& (points[0] < 200) && reserve[0].empty ())
	       throw _("Picking up the staple would leave you without cards\n"
		       "and you can't end the game now!");
	 }
	 catch (Glib::ustring& e) {
	    Gtk::MessageDialog dlg (e, Gtk::MESSAGE_ERROR);
	    dlg.set_title (_("Invalid move"));
	    dlg.run ();
	    enableHuman ();
	    return;
	 }

      if (getConnectionMgr ().getMode () != YGP::ConnectionMgr::NONE) {
	 // Send played card to all clients (if any)
	 std::ostringstream msg;
	 msg << "Play=" << card.id () << ";Target=3";

	 if (getConnectionMgr ().getMode () == YGP::ConnectionMgr::CLIENT)
	    ignoreNextMsg = true;
	 broadcastMessage (msg.str ());
      }

      // Special handling of player starting the game and can choose one of the
      // first two cards
      if (gStatus.startGame) {
	 Check3 (dumped.size () == 1);
	 target = &hands[0];
	 card.show ();
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

	 // Create new pile with picked up card
	 target = &makeNewPile (0);
	 if (dumped.size () > 1)
	    gStatus.pickUpPlayed = 1;
      }
      Check2 (target);
      CardWindow& win (animateCard (*target, dumped, dumped.size () - 1));
      win.sigAnimation.connect (mem_fun (*this, &Buraco::enableHumanHand));
   }
   else {
      staple.append (dumped.removeTopCard ());
      Glib::signal_idle ().connect (mem_fun (*this, &Buraco::enableHuman));
   }
}

//-----------------------------------------------------------------------------
/// Action after picking up the card from the dumped staple
//-----------------------------------------------------------------------------
void Buraco::doDumpedSelected () {
   TRACE5 ("Buraco::doDumpedSelected () - " << gStatus.startGame);
   Check3 (dumped.size ());
   Check3 (gameStatus () == PLAYING);
   unsigned int player (currentPlayer ());

   dumped.getTopCard ().show ();
   hands[player].getCards (dumped);
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

//-----------------------------------------------------------------------------
/// Prepares the card for drag´n´drop
/// \param iCard Number of card in hand
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

   card.drag_source_set_icon (card.getImage ());
   aDNDHand[&card].connReceive = card.signal_drag_data_received ().connect
      (bind (mem_fun (*this, &Buraco::cardDropped), iCard));
   aDNDHand[&card].connGet = card.signal_drag_data_get ().connect
      (bind (mem_fun (*this, &Buraco::getDropData), iCard));
}

//-----------------------------------------------------------------------------
/// Stops the drag´n´drop abilities of the passed card
/// \param card Card to unregister of dnd
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
/// \param pile Pile whose cards should be registered
/// \param start Number of first card to prepare for DND
/// \param end Number of last card to prepare for DND
/// \pre \c start < \c end; \c end <= Number of cards
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
/// \param card Card to register
/// \param nr Number of card in pile
//-----------------------------------------------------------------------------
void Buraco::registerTableDND (CardWidget& card, unsigned int nr) {
   TRACE9 ("Buraco::registerTableDND (CardWidget&, unsigned int) - " << card
           << " = " << std::hex << nr << " - " << &card << std::dec);

   // Card accepts drops from hand and drags from table
   card.drag_dest_set (dndType, Gtk::DEST_DEFAULT_ALL, Gdk::ACTION_MOVE);
   aDNDTable[&card] = card.signal_drag_data_received ().connect
      (bind (mem_fun (*this, &Buraco::cardDroppedOnTable), nr));
}

//-----------------------------------------------------------------------------
/// Stops the drag´n´drop abilities of the passed card
/// \param card Card to de-register
//-----------------------------------------------------------------------------
void Buraco::unregisterTableDND (CardWidget& card) {
   TRACE9 ("Buraco::unregisterTableDND (unsigned int) - Card: " << card
           << " - " << &card );
   Check1 (aDNDTable.size () > 1);

   std::map<CardWidget*, sigc::connection>::iterator i (aDNDTable.find (&card));
   Check1 (i != aDNDTable.end ());

   card.drag_dest_unset ();
   i->second.disconnect ();
   aDNDTable.erase (i);
}

//-----------------------------------------------------------------------------
/// Callback after dropping a card (within the hand)
/// \param pContext Context of the drag (contains things like source,
///    target, action, ...)
/// \param data Describes the thing which was dropped
/// \param info Describes the type of data (should be 0)
/// \param time Timestamp of the drag
/// \param card Number of card where something was dropped at
/// \pre \c pContext not NULL; Expects \c info to be 0
//-----------------------------------------------------------------------------
void Buraco::cardDropped (const Glib::RefPtr<Gdk::DragContext>& context,
                          gint, gint, const Gtk::SelectionData& data,
                          guint info, guint32 time, unsigned int card) {
   Check3 (!context->get_is_source ());
   Check3 (data.get_length () == sizeof (int));
   Check3 (data.get_format () == 8);
   Check3 (card < hands[0].size ());

   unsigned int* pValue (reinterpret_cast <unsigned int*>
                         (const_cast<guint8*> (data.get_data ())));
   Check3 (pValue);
   Check3 (*pValue < hands[0].size ());
   TRACE1 ("Buraco::cardDropped (...) - Inserting card " << *pValue
           << " at pos " << card);

   context->drag_finish (true, false, time);                     // End old DND

   CardWidget& cardMoved (hands[0].remove (*pValue));
   hands[0].insert (cardMoved, card);                     // Insert moved card

   // Adapt dnd-settigns
   if (*pValue < card) {
      unsigned int temp (card);
      card = *pValue;
      *pValue = temp;
   }

   // DND within hand directly after picking up the buraco disables undoing
   if (undo.pickUp)
      menuUndo->set_sensitive (false);

   Glib::signal_idle ().connect
       (bind (mem_fun (*this, &Buraco::doRegisterHand), card, *pValue));
}

//-----------------------------------------------------------------------------
/// Checks if the piles on the table are valid (have at least 3 cards)
/// \param except Pile which can be invalid
/// \returns bool True, if the piles are OK
//-----------------------------------------------------------------------------
bool Buraco::doRegisterHand (unsigned int first, unsigned int last) {
   TRACE9 ("Buraco::doRegisterHand (unsigned int, unsigned int) - [" << first << '-' << last);
   Check1 (last < hands[0].size ());
   Check1 (first <= last);

   registerHandDND (first, last);
   Check3 (aDNDHand.size () == hands[0].size ());
   return false;
}

//-----------------------------------------------------------------------------
/// Checks if the piles on the table are valid (have at least 3 cards)
/// \param except Pile which can be invalid
/// \returns bool True, if the piles are OK
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
/// \param pContext Context of the drag (contains things like source,
///     target, action, ...)
/// \param data Describes the thing which was dropped
/// \param info Describes the type of data (should be 0)
/// \param time Timestamp of the drag
/// \param iCard Combination of card and pile on which card was dropped
/// \pre \c pContext not NULL;
//-----------------------------------------------------------------------------
void Buraco::cardDroppedOnTable (const Glib::RefPtr<Gdk::DragContext>& context,
                                 gint, gint, const Gtk::SelectionData& data,
                                 guint, guint32 time, unsigned int iCard) {
   TRACE1 ("Buraco::cardDroppedOnTable (...) - Card dropped on " << std::hex
           << (int)iCard << std::dec);
   Check3 (!context->get_is_source ());
   Check3 (data.get_length () == sizeof (int));
   Check3 (data.get_format () == 8);

   unsigned int* pValue (reinterpret_cast <unsigned int*>
                         (const_cast<guint8*> (data.get_data ())));
   Check3 (pValue);
   TRACE1 ("Buraco::cardDroppedOnTable (...) - Inserting card " << *pValue
           << " in pile");
   Check3 (*pValue < hands[0].size ());

   try {
      // Check if all piles (except those to which card is dropped) are valid
      if (!humanPilesOK (iCard >> 8))
	 throw Glib::ustring (_("You need to fill up other piles first!"));

      CardWidget& moved (*hands[0][*pValue]);
      TRACE4 ("Buraco::cardDroppedOnTable (...) - Card dropped: " << moved);

      // Move dropped card to a (new) pile on the table
      unsigned int iPile;
      BuracoPile* pile (NULL);
      if (iCard == -1U) {     // If card was dropped on the new pile: Create pile
	 // Check validity of drop
	 if (!(isJoker (moved)
	       ? pileHasFittingPair (hands[0], &moved)
	       : pileHasFittingPair (hands[0], moved, !gStatus.pickUpPlayed)))
	    throw Glib::ustring (_("There are no cards to make a valid new pile!"));

	 // Only allow dropping on new pile while having < 5 cards, if the game
	 // can be ended, or there is still the reserve
	 if (!canPlayCards (0, 3))
	    throw Glib::ustring (_(*unfinishedMonoPiles
				   ? N_("You can't end the game (a pile of monos is not finished)!")
				   : ((hands[0].size () <= 5)
				      ? N_("You can't end the game (there's no \"cerrado\")!")
				      : N_("Not enough cards to make new pile!"))));

	 iPile = tablePiles[0].size ();
	 pile = &makeNewPile (0); Check3 (tablePiles[0].size ());
	 iCard = 0;
      }
      else {
	 // Can't use the new pile (with the picked up card) with a joker
	 if (isJoker (moved) && gStatus.pickUpPlayed)
	    throw Glib::ustring (N_("You may not start this new pile with a joker!"));

	 // Else check pile to use
	 Check1 ((iCard >> 8) < tablePiles[0].size ());
	 pile = tablePiles[0][iPile = (iCard >> 8)];

	 if ((iCard = cardFitsOnPile (iPile, moved)) == -1U)
	    throw Glib::ustring (_("This card does not fit on that pile!"));

	 // Only allow dropping of last card, if the game can be ended, or there
	 // is still the reserve
	 if (!canPlayCards (0, 1, iPile))
	    throw Glib::ustring (_(*unfinishedMonoPiles
				   ? N_("You can't end the game (a pile of monos is not finished)!")
				   : N_("You can't end the game (there's no \"cerrado\")!")));

	 if ((pile->size () == 1)
	     && isJoker (pile->getTopCard ())
	     && isJoker (moved))
	    ++unfinishedMonoPiles[0];
      }

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
      Check3 (iCard <= pile->size ());

      TRACE4 ("Buraco::cardDroppedOnTable (...) - Undo:  " << iPile << "; " << iCard
	      << "; " << *pValue << ": " << (gStatus.pickUpPlayed ? dumped.size () : 0)
	      << '/' << pile->getPosJoker ());
      undo.assign (iPile, iCard, *pValue);

      if (move != -1U) {
	 Check3 (move <= pile->size ());
	 Check3 (move != pile->getPosJoker ());
	 Check3 (pile->getPosJoker () != 7);
	 undo.monoPos = pile->getPosJoker ();
	 sendMoveCard (iPile, pile->getPosJoker (), move);
	 pile->move (move, pile->getPosJoker ());
      }

      // Send move
      if (getConnectionMgr ().getMode () != YGP::ConnectionMgr::NONE) {
	 std::ostringstream msg;
	 msg << "Play=" << moved.id () << ";Target="
	     << (iPile << 16) + iCard + 100;
	 if (getConnectionMgr ().getMode () == YGP::ConnectionMgr::CLIENT)
	    ignoreNextMsg = true;
	 broadcastMessage (msg.str ());
      }

      pile->insert (moved, iCard);
      registerTableDND (moved, (iPile << 8) + iCard);
      if (iCard < (pile->size () - 1))
	 registerTableDND (iPile, iCard + 1, pile->size () - 1);

      // Remove pile, if it contains 7 cards
      if (pile->size () == 7)
	 removeCerrado (0, *pile);

      // Accept again the jokers, if the pile has has now three cards (jokers are
      // disabled, if the human picked up the dumped pile.
      unsigned int size (hands[0].size ());
      if ((pile->size () == 3) && gStatus.pickUpPlayed) {
	 hands[0].getCards (dumped);
	 menuUndo->set_sensitive (false);
	 gStatus.pickUpPlayed = 0;

	 for (unsigned int i (size); i < hands[0].size (); ++i) {
	    enableCard (i);
	    registerHandDND (i);
	 }
      }
      else
	 menuUndo->set_sensitive ();

      if (*pValue < size)
	 registerHandDND (*pValue, size - 1);

      // If the player has no more cards left (except of joker): Give him the reserve
      if (containsOnlyJoker (hands[0]) && humanPilesOK ()) {
	 if (!reserve[0].empty ()) {
	    disableHuman ();
	    Glib::signal_idle ().connect
	       (bind_return (mem_fun (*this, &Buraco::addBuraco4HumanAndEnable), false));
	    return;
	 }
	 else
	    if (hands[0].empty ()) {
	       points[0] += 100;
	       endGame ();
	       return;
	    }
      }
      Check3 (aDNDHand.size () == hands[0].size ());
   }
   catch (Glib::ustring& error) {
      context->drag_finish (false, false, time);
      Gtk::MessageDialog dlg (error, Gtk::MESSAGE_ERROR);
      dlg.set_title (_("Invalid move"));
      dlg.run ();
      return;
   }
}

//-----------------------------------------------------------------------------
/// Adds the buraco to the human and re-enables his cards
//-----------------------------------------------------------------------------
void Buraco::addBuraco4HumanAndEnable () {
   addBuraco (0);
   enableHumanHand ();
}

//-----------------------------------------------------------------------------
/// Callback to query the data to drop
/// \param pContext Context of the drag (contains things like source,
///      target, action, ...)
/// \param data Describes the thing which was dropped
/// \param time Timestamp of the drag
/// \param cardPos Position of card (either in hand or pile on table)
/// \pre \c pContext not NULL; Expects \c info to be 0
//-----------------------------------------------------------------------------
void Buraco::getDropData (const Glib::RefPtr<Gdk::DragContext>& pContext,
                          Gtk::SelectionData& data, guint info, guint32 time,
                          unsigned int cardPos) {
   Check1 (!info);
   Check1 (pContext->get_is_source ());

   data.set (data.get_target (), 8, reinterpret_cast <guchar*> (&cardPos),
             sizeof (cardPos));
}

//-----------------------------------------------------------------------------
/// Prepares the passed region of cards for drag´n´drop
/// \param start Number of first card to prepare for DND
/// \param end Number of last card to prepare for DND
/// \pre \c start < \c end; \c end <= Nr. ofcards
//-----------------------------------------------------------------------------
void Buraco::registerHandDND (unsigned int start, unsigned int end) {
   TRACE5 ("Buraco::registerHandDND (unsigned int, unsigned int) - [" << start
           << '-' << end << "] of " << activeCards.size ());
   Check1 (start <= end);
   Check1 (end < hands[0].size ());
   Check1 (end < activeCards.size ());

   for (; start <= end; ++start) {
      TRACE9 ("Buraco::registerHandDND (unsigned int, unsigned int) - Handling card " << start);

      activeCards[start].disconnect ();
      activeCards[start] = hands[0][start]->signal_clicked ().connect
         (bind (mem_fun (*this, (&Buraco::cardSelected)), start));

      unregisterHandDND (*hands[0][start]);
      registerHandDND (start);
   }
   TRACE9 ("Buraco::registerHandDND (unsigned int, unsigned int) - End");
}

//-----------------------------------------------------------------------------
/// Checks, if the passed pile contains no cards except jokers or 2s. This is
/// also true for empty piles.
/// \param pile Pile to inspect
/// \returns bool True, if there are only jokers (or pile is empty)
//-----------------------------------------------------------------------------
bool Buraco::containsOnlyJoker (const std::vector<CardWidget*>& pile) {
   TRACE8 ("Buraco::containsOnlyJoker (const std::vector<CardWidget*>&)");

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
/// \param pile Pile to inspect
/// \returns bool True, if there are no jokers
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
/// \param player Player getting the reserve
//-----------------------------------------------------------------------------
void Buraco::addBuraco (unsigned int player) {
   TRACE3 ("Buraco::addBuraco (unsigned int) - " << player);
   Check1 (player < NUM_PLAYERS);

   undo.pickUp = 1;
   unsigned int cJokers (hands[player].size ());

   // Storing player picking up the buraco
   ((player & 1) ? gStatus.team2Buraco : gStatus.team1Buraco) = (player >> 1);

   // Add reserve
   sort (reserve[player & 1].begin (), reserve[player & 1].end (),
	 compByNumberWithJokers);

   for (std::vector<CardWidget*>::reverse_iterator i (reserve[player & 1].rbegin ());
	i != reserve[player & 1].rend (); ++i)
      hands[player].insert (**i, 0);
   reserve[player & 1].clear ();

   Check3 (actPlayers.size () > player);
   Check3 (actPlayers[player]);

   // If the computer-player had jokers left, show them
   if (player && cJokers)
      Glib::signal_idle ().connect
	 (bind (sigc::ptr_fun (&Buraco::showJoker), &hands[player], cJokers, true));

   status.pop ();
   Glib::ustring stat (_("%1 picked up the burraco"));
   stat.replace (stat.find ("%1"), 2, actPlayers[player]->getName ());
   status.push (stat);
   updateInfo ();
}

//-----------------------------------------------------------------------------
/// Shows or hides the joker, which are displayed when picking up the buraco
/// \param pile Pile holding the jokers shown
/// \param cJokers Numer of jokers shown
/// \param show Flag if the jokers should be shown or hidden
/// \returns bool false
//-----------------------------------------------------------------------------
bool Buraco::showJoker (ICardPile* pile, unsigned int cJokers, bool show) {
   TRACE9 ("Buraco::showJoker (ICardPile*, unsigned int, bool) - " << cJokers);
   Check1 (cJokers);
   Check1 ((cJokers + CARDS2DEAL) <= pile->size ());

   ICardPile::PileStyle opt (show ? ICardPile::COMPRESSED : ICardPile::VERY_COMPRESSED);
   for (ICardPile::iterator i (pile->begin () + CARDS2DEAL); i != pile->end (); ++i) {
      show ? (*i)->showFace () : (*i)->showBack ();
      pile->resize (**i, opt);
   }
   pile->resize (CARDS2DEAL - 1 + cJokers, ICardPile::NORMAL);

   if (show)
      Glib::signal_timeout ().connect
	 (bind (sigc::ptr_fun (&Buraco::showJoker), pile, cJokers, false),
	  ComputerPlayer::TIMEOUT - 50);
   return false;
}

//-----------------------------------------------------------------------------
/// Makes a new pile for the passed team.
/// \param team Which team to make the pile for
/// \returns BuracoPile& New created pile
//-----------------------------------------------------------------------------
BuracoPile& Buraco::makeNewPile (unsigned int team) {
   TRACE8 ("Buraco::makeNewPile (unsigned int) - New pile for team " << team + 1);
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
/// \param player Player to inspect
/// \param iCard Card to inspect
/// \returns unsigned int Value describing the pile (and the offset of the card)
///     to play to; -1 if none
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
         continue;
      }
      Check3 ((*p)->size () >= 3);
      Check3 ((*p)->size () < 7);

      // Play joker, if you can make a cerrado (7 in a row) - but only if the
      // one having picked up the reserve already played (the missing card
      // might be in there) and the oponent can't finish.
      int posPile ((*p)->size ());
      if (isJoker (card)
          ? (((((*p)->size () == 6) && ((*p)->getPosJoker () > 6))
	      && ((hands[player].size () - iCard) < 7)
              && (((reserve[player & 1].empty ()
                    && ((((player & 1) ? gStatus.team2Buraco : gStatus.team1Buraco
                         == 0x3))
			|| (hands[player].size () < 3)))
                   || (points[player & 1] > 100))
                  || reserve[!(player & 1)].empty ()
                  || (points[!(player & 1)] > 100)))
             || ((*p)->getPosFirst () > 6))
          : ((posPile = cardFitsOnPile (p - tablePiles[player & 1].begin (), card))
             != -1)) {
	 // Always play on a joker pile (don't bother checking for a second one)
	 if ((*p)->getPotentialPoints () >= 1000) {
            bestPile = p - tablePiles[player & 1].begin ();
	    break;
	 }
         if ((size < (*p)->size ())
             || ((size == (*p)->size ())
                 && (maxPoints < (*p)->getPotentialPoints ()))) {
            size = (*p)->size ();
            maxPoints = (*p)->getPotentialPoints ();
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
      if ((move != -1U) && canPlayCards (player, 1, bestPile)) {
         Check3 (move <= pile.size ());
         Check3 (move != pile.getPosJoker ());
	 Check3 (pile.getPosJoker () != 7);
         sendMoveCard (bestPile, pile.getPosJoker (), move);
         pile.move (move, pile.getPosJoker ());
      }
      return (bestPile << 16) + pos;
   }
   return -1U;
}

//-----------------------------------------------------------------------------
/// Checks if the passed card is a joker
/// \param card Card to inspect
/// \returns bool True if card is a joker
//-----------------------------------------------------------------------------
bool Buraco::isJoker (const CardWidget& card) {
   return ((card.number () == CardWidget::TWO)
           || (card.number () > CardWidget::ACE));
}

//-----------------------------------------------------------------------------
/// Removes a cerrado (a pile with 7 cards) from the table
/// \param player Player causing the remove of the pile
/// \param pile Pile holding the cerrado
//-----------------------------------------------------------------------------
void Buraco::removeCerrado (unsigned int player, BuracoPile& pile) {
   unsigned int team (player & 1);

   Check1 (player < NUM_PLAYERS);
   Check1 ((std::find (tablePiles[team].begin (), tablePiles[team].end (), &pile))
           != tablePiles[team].end ());
   TRACE8 ("Buraco::removeCerrado (unsigned int, BuracoPile&) - Pile "
           << (std::find (tablePiles[team].begin (), tablePiles[team].end (), &pile)
               - tablePiles[team].begin ()) << " of team " << team);
   Check2 (pile.size () == 7);

   pile.hide ();
   Check3 (static_cast<int> (pile.getPotentialPoints ()) == pile.getPoints ());
   points[team] += pile.getPoints ();

   if (pile.getPoints () > 999)
      --unfinishedMonoPiles[team];
   updateInfo ();
}

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

//-----------------------------------------------------------------------------
/// Checks if the passed card fits on the passed staple
/// \param iPile Pile to inspect
/// \param card Card to check
/// \returns Position where card can be played to, or -1 if card does not fit
//-----------------------------------------------------------------------------
int Buraco::cardFitsOnPile (unsigned int iPile, const CardWidget& card) const {
   Check1 (iPile < tablePiles[currentPlayer () & 1].size ());
   BuracoPile& pile (*tablePiles[currentPlayer () & 1][iPile]);
   Check2 (pile.size ()); Check2 (pile.size () < 7);

   // Card played on a joker: Valid is:
   //   - A joker; if there are at least 3 jokers (on table + in hand)
   //   - Any card, which has a pair (if there's only one joker on the table)
   if (pile.getPosFirst () > 6) {
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
   }

   unsigned int pos, move;
   if (pile.getPosition4Card (card, pos, move)) {
      Check3 (pos <= pile.size ());
      if ((pile.size () > 1) || isJoker (card))
          return pos;
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
/// \param pile Number of pile involved
/// \param from Card to move
/// \param to Position card to move to
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
/// \param open Flag if cards should be shown or hidden
//-----------------------------------------------------------------------------
void Buraco::playOpen (bool open) {
   for (unsigned int i (1); i < NUM_PLAYERS; ++i) {
      hands[i].setShowOption (open ? ICardPile::SHOWFACE : ICardPile::SHOWBACK);
      hands[i].setStyle (open ? ICardPile::COMPRESSED : ICardPile::QUITE_COMPRESSED);
   }
}

//-----------------------------------------------------------------------------
/// Creates the combined team names from the players
/// \param names Array to receive groups
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
      menuShowScoreDlg->set_sensitive ();
      pScoreDlg = ScoreDlg::create (nameTeams);
      pScoreDlg->get_window ()->set_transient_for (get_window ());
   }

   points[0] += reserve[0].empty () ? 100 : -100;
   points[1] += reserve[1].empty () ? 100 : -100;
   pScoreDlg->addPoints (points);

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

   for (unsigned int i (0); i < NUM_PLAYERS; ++i)
      for (std::vector<CardWidget*>::const_iterator c (hands[i].begin ());
           c != hands[i].end (); ++c)
         points[i & 1] -= getPoints (**c);

   pScoreDlg->addPoints (points);
   pScoreDlg->show ();

   Glib::ustring stat (_("Round ended"));
   unsigned int player;
   int maxPoints;
   pScoreDlg->getMaxPoints (maxPoints, player);
   TRACE7 ("Buraco::endGame () - Points: " << maxPoints);
   if (maxPoints >= (int)ENDPOINTS) {
      stat = _("Game ended; Team %1 won");
      stat.replace (stat.find ("%1"), 2, 1, (char)('1' + player));
   }

   // Move cards of partners to first player and show them
   for (unsigned int i (1); i < NUM_PLAYERS; ++i) {
      hands[i].setStyle (ICardPile::COMPRESSED);
      hands[i].setShowOption (ICardPile::SHOWFACE);
   }

   status.pop ();
   status.push (stat);

   setGameStatus (STOPPED);
   setNextPlayer (0);                  // enableHuman() checks for player == 0
   enableHuman ();
   setNextPlayer (-1);
}

//-----------------------------------------------------------------------------
/// Returns the value of the passed card
/// \param card Card to inspect
/// \returns unsigned int Value of the card
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
/// \param player Player whose cards should be inspected
/// \returns bool True: if all cards can be played
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

   TRACE8 ("Buraco::canGetRidOfCards (unsigned int) -  " << used.count ()
           << '/' << hands[player].size () << "; " << cJokers << " Joker for "
           << piles << " piles -> "
           << ((((used.count () + 1) >= hands[player].size ())
                && (piles <= cJokers)) ? 'Y' : 'N'));
   return (((used.count () + 1) >= hands[player].size ())
           && (piles <= cJokers));
}

//-----------------------------------------------------------------------------
/// Checks if the player can play the specified number of cards; a player can
/// only play all of his cards, if:
///   - The team has a cerrado
///   - The team still has the reserve
///   - The player can close a pile, with the cards to play
///   - After the turn there are no unfinished mono-piles
/// \param player Player to analyse
/// \param cards Number of cards player wants to play
/// \param pile Pile player is going to play its card to (or -1 for a new one)
/// \returns bool True, if card can be played
//-----------------------------------------------------------------------------
bool Buraco::canPlayCards (unsigned int player, unsigned int cards,
                           unsigned int pile) const {
   TRACE7 ("Buraco::canPlayCards (3x unsigned int) - Player "
           << player << " playing " << cards << " cards to " << pile);
   Check1 (player < NUM_PLAYERS);
   Check1 ((pile == -1U) || (tablePiles[player & 1].size () > pile));
   Check1 ((pile == -1U)
           || ((tablePiles[player & 1][pile]->size () + cards) <= 7));
   Check1 (hands[player].size () >= cards);
   Check1 (cards <= 7);

   unsigned int cCards (hands[player].size ());
   if (gStatus.pickUpPlayed)
     cCards += dumped.size ();

   if ((cCards <= (cards + 1)) && reserve[player & 1].empty ()) {
      bool canPlay ((points[player & 1] > 100)
		    || ((pile != -1U)
			&& (((tablePiles[player & 1][pile]->size () + cards) >= 7)
			    || (((tablePiles[player & 1][pile]->size () + cards) == 6)
				&& ((hands[player].size () - cards) == 1)
				&& canClosePile (player, pile))))
		    || (cards >= 7));
      TRACE1 ("Buraco::canPlayCards (3x unsigned int) - Can play: " << (canPlay ? "Yes" : "No"));
      return ((!unfinishedMonoPiles[player & 1])
	      || ((unfinishedMonoPiles[player & 1] == 1)
		  && (pile != -1U)
		  && (tablePiles[player & 1][pile]->getPoints () < 0))
	      ? canPlay : false);
   }
   return true;
}

//----------------------------------------------------------------------------
/// Checks if the player can with his two cards left close the passed pile
/// \param player Player to inspect
/// \param pile Pile to analyse
/// \return bool True, if the remaining cards of the player can make a
///        cerrado for this pile
/// \remarks - The player must have only two cards; the pile 5
//----------------------------------------------------------------------------
bool Buraco::canClosePile (unsigned int player, unsigned int pile) const {
   TRACE7 ("Buraco::canClosepile (2x unsigned int) - Player "
           << player << " closes pile " << pile);
   Check1 (player < NUM_PLAYERS);
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

          if (copy.getPosition4Card (*hands[player][!i], pos, move)) {
             isOK = true;
             break;
          }
	  else
	     delete &copy.remove (pos);
       }
   }

   // Free the copy
   for (BuracoPile::const_iterator i (copy.begin ()); i != copy.end (); ++i)
      delete *i;

   return isOK;
}

//-----------------------------------------------------------------------------
/// Checks if the passed pile contains a pair matching the passed card
/// \param pile Pile to inspect
/// \param card Card where to find a pair to
/// \param withJokers Flag, if jokers should be inspected
/// \returns bool True, if the pile contains a matching pair
//-----------------------------------------------------------------------------
bool Buraco::pileHasFittingPair (const ICardPile& pile, const CardWidget& card,
                                 bool withJokers) {
   TRACE3 ("Buraco::pileHasFittingPair (const ICardPile&, const CardWidget&,"
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
/// \param pile Pile to inspect
/// \param exclude Card to not inspect (can be NULL)
/// \returns bool True, if the pile contains a matching pair
//-----------------------------------------------------------------------------
bool Buraco::pileHasFittingPair (const ICardPile& pile, const CardWidget* exclude) {
   TRACE3 ("Buraco::pileHasFittingPair (const ICardPile&, const CardWidget*)");

   for (std::vector<CardWidget*>::const_iterator p (pile.begin ());
        p != pile.end (); ++p)
      if (*p != exclude)
         if ((pile.getFittingCard (**p, pile.begin (), &cardDistance) != p)
             || (pile.getFittingCard (**p, p + 1, &cardDistance) != pile.end ()))
            return true;
   return false;
}

//-----------------------------------------------------------------------------
/// Compares the cards in the pile with regard of the number and with special
/// consideration of joker cards
/// \param a Card to compare
/// \param b Card to compare
/// \returns bool True, if a < b
//-----------------------------------------------------------------------------
bool Buraco::compByNumberWithJokers (const CardWidget* a, const CardWidget* b) {
   // Card:                 2   3  4  5  6  7  8  9  10 J  Q   K  A   Joker
   static char values[] = { 12, 0, 1, 2, 3, 4, 5, 6, 7, 8, 9, 10, 11, 13 };
   Check3 (a->number () < static_cast<int> (sizeof (values) / sizeof (values[0])));
   Check3 (b->number () < static_cast<int> (sizeof (values) / sizeof (values[0])));

   return values[a->number ()] < values[b->number ()];
}

//-----------------------------------------------------------------------------
/// Compares the cards in the pile with regard of the colour and with special
/// consideration of joker cards
/// \param a Card to compare
/// \param b Card to compare
/// \returns bool True, if a < b
//-----------------------------------------------------------------------------
bool Buraco::compByColourWithJokers (const CardWidget* a, const CardWidget* b) {
   switch (a->number ()) {
   case CardWidget::TWO:
      return b->number () == CardWidget::UNREACHABLE;
      break;

   case CardWidget::UNREACHABLE:
      return false;
      break;

   default:
      return (isJoker (*b) ? true
	      : ((a->colour () == b->colour ())
		 ? a->number () < b->number () : a->colour () < b->colour ()));
   } // endswitch
}

//----------------------------------------------------------------------------
/// Returns the distance between two cards. The ace also counts as one (if the
/// other card is a 3 or a 4) and 2's are equal to jokers.
/// \param a Card to compare
/// \param b Card to compare
/// \returns int Distance of the two passed cards (a - b)
//----------------------------------------------------------------------------
int Buraco::cardDistance (const CardWidget& a, const CardWidget& b) {
   return cardDistance (a, b, true);
}

//----------------------------------------------------------------------------
/// Returns the distance between two cards. The ace also counts as one (if the
/// other card is a 3 or a 4) and 2's are equal to jokers.
/// \param a Card to compare
/// \param b Card to compare
/// \param aceIsOne Flag, if aces should (also) be treated as one
/// \returns int Distance of the two passed cards (a - b)
//----------------------------------------------------------------------------
int Buraco::cardDistance (const CardWidget& a, const CardWidget& b, bool aceIsOne) {
   TRACE9 ("Buraco::cardDistance (2x const CardWidget&, bool) - "
           << a << "<->" << b);
   // Special handling of jokers
   bool aJoker (isJoker (a));
   bool bJoker (isJoker (b));
   if (aJoker || bJoker)
      return aJoker && bJoker ? 0 : 99;

   if (a.colour () != b.colour ())
      return (a.number () == b.number ()) ? 0 : 99;

   if (aceIsOne) {                        // Special handling of the ace like 1
      TRACE9 ("Buraco::cardDistance (2x const CardWidget&, bool) - Ace");
      if ((a.number () == CardWidget::ACE)
          && (b.number () < CardWidget::EIGHT))
         return -static_cast<int> (b.number ());
      else if ((b.number () == CardWidget::ACE)
               && (a.number () < CardWidget::EIGHT))
         return static_cast<int> (a.number ());
   }

   TRACE9 ("Buraco::cardDistance (2x const CardWidget&, bool) - "
           "Distance: " << a.number () - b.number ());
   return a.number () - b.number ();
}

//----------------------------------------------------------------------------
/// Changes the names of the playing people
/// \param newPlayer Array holding the new player
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
/// \param player Number of player
/// \param pile ID of the pile to return
/// \returns ICardPile* Pointer to pile to use or NULL
//----------------------------------------------------------------------------
ICardPile* Buraco::getPileOfPlayer (unsigned int player, unsigned int pile) {
   if (((pile > 3) && (pile < 100)) || (player >= NUM_PLAYERS))
      return NULL;

   if ((pile >= 100) && (pile != -1U)) {
      if (pile != 0xffff0000) {
         pile -= 100;
         if ((pile >> 16) > tablePiles[player & 1].size ())
	    return NULL;

         if ((pile >> 16) == tablePiles[player & 1].size ()) {
            TRACE8 ("Buraco::getPileOfPlayer (unsigned int, unsigned int) - Creating pile");
            makeNewPile (player & 1);
         }
      }
      // TODO: What? target = pile;
      return &hands[player];
   }
   return &((pile == 2)
            ? static_cast<ICardPile&> (staple)
            : ((pile == 3)
               ? static_cast<ICardPile&> (dumped)
               : static_cast<ICardPile&> (hands[player])));
}

//----------------------------------------------------------------------------
/// Handles the messages the server might send for the Buraco cardgame
/// \param player ID of the player sending the message
/// \param message Message received from the server
/// \returns bool True, if message has been processed completey
/// \throw YGP::ParseError, YGP::CommError In case of an error an describing text
//----------------------------------------------------------------------------
bool Buraco::handleMessage (unsigned int player, const std::string& message) throw (YGP::ParseError, YGP::CommError) {
   TRACE1 ("Buraco::handleMessage (unsigned int player, const std::string&) - "
           << message << " (" << player << ')');

   YGP::Tokenize command (message);
   std::string cmd (command.getNextNode ('='));

   bool rc (true);
   if (cmd == "Undo") {
      // Inform clients about cards to play
      if (getConnectionMgr ().getMode () == YGP::ConnectionMgr::SERVER)
          broadcastMessage (cmd);

      undoLast (player);
   }
   else if (cmd == "Move") {
      YGP::AttributeParse ap;
      unsigned int card (-1U), dest (-1U), iPile (-1U);
      ATTRIBUTE (ap, unsigned int, card, "Move");
      ATTRIBUTE (ap, unsigned int, dest, "To");
      ATTRIBUTE (ap, unsigned int, iPile, "Pile");
      ap.assignValues (message);

      if (iPile >= tablePiles[currentPlayer () & 1].size ())
         throw YGP::ParseError (N_("Invalid pile!"));
      ICardPile& pile (*tablePiles[currentPlayer () & 1][iPile]);
      if ((card >= pile.size ()) || (dest >= pile.size ()))
         throw YGP::ParseError (N_("Invalid card!"));

      pile.move (dest, card);
   }
   else {
      rc = Game::handleMessage (player, message);
      if (cmd == "ActPlayer") {
         TRACE1 ("Buraco::handleMessage (unsigned int player, const std::string&) - "
                 "Next player: " << currentPlayer ());

         startPlayer = currentPlayer ();
         setStartPlayer ();
      }
   }
   return rc;
}

//----------------------------------------------------------------------------
/// Executes the remote move locally
/// \param pile Pile to move to/from
/// \param dest ID of target as send by the partner
/// \returns bool True, if the timer to execute the move should be set
/// \pre Expects \c pos1Play and \c pos2Play to be set to the positions to play
/// \remarks
///    - \c dest == 0: Computerplayer playing its card
///    - \c dest == 1: Play from hand to dumped
///    - \c dest == 2: Pick up from staple
///    - \c dest == 3: Pick up from dumped pile
///    - \c dest == 100 + pile/card: Played from hand to table pile \c pile
/// \throw YGP::ParseError In case of an invalid value
//----------------------------------------------------------------------------
bool Buraco::executeRemoteMove (ICardPile& pile, unsigned int dest) throw (YGP::ParseError) {
   TRACE8 ("Buraco::executeRemoteMove (ICardPile&, unsigned int) - " << dest);
   Check1 ((dest < 4) || (dest >= 100));
   Check2 (pos1Play != -1U);
   Check2 (pos2Play != -1U);

   switch (dest) {
   case 1:
      // TODO: What? target = 0xffff0000;
      gStatus.startGame = gStatus.startTurn = 0;
      break;

   case 2: {
      Check3 (staple.size ());
      CardWindow& win (animateCard (hands[currentPlayer ()], staple, staple.size () - 1));
      if (!currentPlayer ())
	 win.sigAnimation.connect (mem_fun (*this, &Buraco::enableHumanHand));
      return false;
   }
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
            throw YGP::ParseError (N_("Invalid target specification!"));

	 Check3 (pos1Play != -1U); Check3 (pos2Play != -1U);
	 if (pos1Play == pos2Play)
	    undo.assign (dest >> 16, dest & 0xffff, pos1Play);
      }
      break;
   }

   return Game::executeRemoteMove (pile, dest);
}

//----------------------------------------------------------------------------
/// Returns the actual target, where flipCard2Play should position the cards to
/// \returns unsigned int ID of the target
//----------------------------------------------------------------------------
unsigned int Buraco::getActTarget () const {
   return 0;
   // TODO: What? return (target == 0xffff0000) ? 0xffff0000 : (target + 100);
}

//-----------------------------------------------------------------------------
/// Adds buraco-specific menus
/// \param mgrUI UIManager to add to
//-----------------------------------------------------------------------------
void Buraco::addMenus (Glib::RefPtr<Gtk::UIManager> mgrUI) {
   TRACE1 ("Buraco::addMenus (Glib::RefPtr<Gtk::UIManager>)");
   Check1 (mgrUI);
   Glib::ustring ui ("<menubar name='Menu'>"
		     "  <placeholder name='GameMenu'>"
		     "    <menu action='MB'>"
		     "      <menuitem action='BuracoUndo'/>"
		     "      <separator/>"
		     "      <menuitem action='BuracoSort'/>"
		     "      <menuitem action='BuracoSortCol'/>"
		     "      <separator/>"
		     "      <menuitem action='showScoreDlg'/>"
		     "    </menu></placeholder></menubar>");

   Glib::RefPtr<Gtk::ActionGroup> grpAction (Gtk::ActionGroup::create ());
   grpAction->add (Gtk::Action::create ("MB", _("_Buraco")));
   grpAction->add (menuUndo = Gtk::Action::create ("BuracoUndo", Gtk::Stock::UNDO),
		   Gtk::AccelKey (_("<ctl>Z")),
		   mem_fun (*this, &Buraco::undoMove));
   grpAction->add (menuSort = Gtk::Action::create ("BuracoSort", Gtk::Stock::SORT_ASCENDING,
						   _("_Sort cards (by number)")),
		   Gtk::AccelKey ("S"),
		   mem_fun (*this, &Buraco::sortHand));
   grpAction->add (menuSort2 = Gtk::Action::create ("BuracoSortCol", Gtk::Stock::SORT_ASCENDING,
						    _("Sort cards (by _colour)")),
		   Gtk::AccelKey ("<shft>S"),
		   mem_fun (*this, &Buraco::sortHandByColour));
   grpAction->add (menuShowScoreDlg = Gtk::Action::create ("showScoreDlg", Gtk::Stock::EDIT,
							   _("Show score dialog")),
		   Gtk::AccelKey ("<shft><ctl>S"),
		   bind (ptr_fun (&ScoreDlg::display), &pScoreDlg));

   mgrUI->insert_action_group (grpAction);
   idMrg = mgrUI->add_ui_from_string (ui);

   menuUndo->set_sensitive (false);
   menuSort->set_sensitive (false);
   menuSort2->set_sensitive (false);
   menuShowScoreDlg->set_sensitive (false);
}

//-----------------------------------------------------------------------------
/// Removes the buraco-specific menus
/// \param mgrUI UIManager to remove from
//-----------------------------------------------------------------------------
void Buraco::removeMenus (Glib::RefPtr<Gtk::UIManager> mgrUI) {
   Check1 (mgrUI);
   mgrUI->remove_ui (idMrg);
}

//-----------------------------------------------------------------------------
/// Undoes the last move of the human player
//-----------------------------------------------------------------------------
void Buraco::undoMove () {
   if (getConnectionMgr ().getMode () != YGP::ConnectionMgr::NONE) {
      if (getConnectionMgr ().getMode () == YGP::ConnectionMgr::CLIENT)
	 ignoreNextMsg = true;
      broadcastMessage ("Undo");
   }

   undoLast (0);
}

//-----------------------------------------------------------------------------
/// Undoes the last move
/// \param player Player whose turn to undo
//-----------------------------------------------------------------------------
void Buraco::undoLast (unsigned int player) {
   TRACE4 ("Buraco::undoLast (unsigned int) - " << player << ": " << undo.destPile
	   << '-' << undo.destPos << "->" << undo.srcPos);
   if (!player)
      disableHuman ();

   if (undo.pickUp) {
      Check3 (reserve[0].empty ()); Check3 (hands[player].size () > 10);

      for (unsigned int i (0); i < 12; ++i)
	 reserve[player & 1].push_back (&hands[player].remove (0));
   }

   Check3 (undo.destPile < tablePiles[player & 1].size ());
   BuracoPile& src (*tablePiles[player & 1][undo.destPile]);
   Check3 (undo.destPos < src.size ());

   if (src.size () == 7) {
      src.show ();
      Check3 (static_cast<int> (src.getPotentialPoints ()) == src.getPoints ());
      points[player & 1] -= src.getPoints ();
      updateInfo ();

      if (src.getPosFirst () > 6)
	 ++unfinishedMonoPiles[player & 1];
   }

   hands[player].insert (src.remove (undo.destPos), undo.srcPos);

   if (src.size () == 0) {
      tablePiles[player & 1].erase (tablePiles[player & 1].begin () + undo.destPile);
      boxTeam[player & 1].remove (src);
      delete &src;
   }

   if (undo.monoPos != 7)
      src.move (undo.monoPos, src.getPosJoker ());

   menuUndo->set_sensitive (false);
   enableHumanHand ();
}

//-----------------------------------------------------------------------------
/// Sorts the cards in the hand by number
//-----------------------------------------------------------------------------
void Buraco::sortHand () {
   disableHuman ();
   hands[0].sort (compByNumberWithJokers);

   // Sorting directly after picking up the buraco disables undoing
   if (undo.pickUp)
      menuUndo->set_sensitive (false);
   enableHumanHand ();
}

//-----------------------------------------------------------------------------
/// Sorts the cards in the hand by colour
//-----------------------------------------------------------------------------
void Buraco::sortHandByColour () {
   disableHuman ();
   hands[0].sort (compByColourWithJokers);

   // Sorting directly after picking up the buraco disables undoing
   if (undo.pickUp)
      menuUndo->set_sensitive (false);
   enableHumanHand ();
}

//-----------------------------------------------------------------------------
/// Actions to take when the cards are resized
/// \pre The cardsize must be set in CardImages::WIDTH/HEIGHT
//-----------------------------------------------------------------------------
void Buraco::resizeCards () {
   staple.set_size_request (CardImages::WIDTH, CardImages::HEIGHT);
   dumped.set_size_request (CardImages::WIDTH, CardImages::HEIGHT);
   for (unsigned int i (0); i < NUM_PLAYERS; ++i)
      hands[i].set_size_request (-1, CardImages::HEIGHT);

   scrlTable[0]->set_size_request (-1, CardImages::HEIGHT + 5 * 15);
   scrlTable[1]->set_size_request (-1, CardImages::HEIGHT + 5 * 15);
}
