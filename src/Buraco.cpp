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

#define CHECK 9
#define TRACELEVEL 9
#include <Check.h>
#include <Trace_.h>
#include <ANumeric.h>
#include <ScoreDlg.h>

#include "SigCExt.h"
std::vector<Gtk::TargetEntry> Buraco::dndType;


unsigned int Buraco::ENDPOINTS (2000);

/*--------------------------------------------------------------------------*/
//Purpose   : Constructor
//Parameters: parent: Parent widget to display the game in
//            statusbar: Status bar widget to display information about the game
//            cardset: Cardset to use
//            names: Vector of player-names
/*--------------------------------------------------------------------------*/
                unsigned int posPlayer, YGP::Mutex& mxSerialize)
                  CardSet& cardset, const std::vector<std::string>& names)
   : Game (parent, statusbar, cardset, names, 3, 10), startPlayer (0)
     , acceptCards (-1U), target (-1U) , pScoreDlg (NULL) {
   TRACE9 ("Buraco::Buraco (Box&, Statusbar&, CardSet&, const "
     , newPile (_("New pile")), startTurn (START_GAME | START_TURN)
     , target (-1U), pos1 (0), pos2 (0) {

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
/*--------------------------------------------------------------------------*/
//Purpose   : Destructor
/*--------------------------------------------------------------------------*/
   clean ();
   delete pScoreDlg;

//-----------------------------------------------------------------------------
/// Removes a cerrado from the table
/*--------------------------------------------------------------------------*/
//Purpose   : Removes a cerrado from the table
//Parameters: team: Team to inspect
//Remarks   : As every move can only make one cerrado; only the first is
//            removed.
/*--------------------------------------------------------------------------*/

   for (std::vector<BuracoPile*>::iterator p (tablePiles[player & 1].begin ());
        p != tablePiles[player & 1].end (); ++p) {
   for (std::vector<CardVPile*>::iterator p (tablePiles[player & 1].begin ());
      if (((*p)->size () == 7) && (*p)->is_visible ()) {
         removeCerrado (player, **p);
      if ((*p)->size () == 7) {
      }
   }
}

//-----------------------------------------------------------------------------
/// Makes the move for the next player.
/*--------------------------------------------------------------------------*/
//Purpose   : Makes the move for the next player.
//Parameters: player: Actual player
//Returns   : int: Next player or -1 if end of game
//Remarks   : This method expects the target pile to play in the target-member
//            and the positions to play in pos1 and pos2
/*--------------------------------------------------------------------------*/
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
         startTurn = START_TURN;
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
      // Move played cards to pile to play
      Check3 (source.size () > pos2);
      for (; (int)pos1 <= (int)pos2; --pos2)
         dest->insert (source.remove (pos1), pos);
      if (gStatus.pickUpPlayed) {
         Check3 (dumped.size ());
            addBuraco (oldPlayer);
         else
            addReserve (oldPlayer, false);
               cleanCerrado (oldPlayer);
               Check3 (points[oldPlayer & 1] > 100);
               Check3 (cerrados[oldPlayer & 1]);
               cleanCerrado (oldPlayer);
            }
      }
   }
   return player;
}

//-----------------------------------------------------------------------------
/// Searches for cards to play and shows them in the hand of the actual player
/*--------------------------------------------------------------------------*/
//Purpose   : Searches for cards to play and shows them in the hand of the
//            actual player
//Parameters: player: Player to inspect
//Returns   : ID for target (32 Bit: Pile << 16 + Position)
/*--------------------------------------------------------------------------*/
           << gStatus.startGame << '/' << gStatus.startTurn << ')');
   TRACE2 ("Buraco::showCardsToPlay (unsigned int) - " << player);
   if (startTurn) {
      ICardPile& playerPile (hands[player]);
          ? (isJoker (dumpedCard)
      Check3 (dumped.size ());
      // Check if there are equal cards as the last dumped one
      int start (playerPile.find (dumped.getTopCard ().number ()));
      int end ((start == -1) ? -1 : playerPile.findLastEqual (start));
      Check3 ((start != -1) ? (start <= end) : (start == end));
      Check3 ((end == -1) || (end < playerPile.size ()));

      if ((!isJoker (dumped.getTopCard ())) && ((end - start) >= 1)
          && (cerrados[player & 1] || !reserve[player & 1].empty ()
              || (dumped.size () + playerPile.size () > 3))) {
         CardVPile& pile (makeNewPile (player & 1));         // Create new pile
         pile.setTopCard (dumped.removeTopCard ());      // with picked up card
         movePile (pile, playerPile, start, end);
            std::map<unsigned int, unsigned int> aPos;
         if (dumped.size ())
            movePile (playerPile, dumped);
         playerPile.sortByNumber ();
         if (getConnectionMgr ().getMode () != YGP::ConnectionMgr::NONE) {
      else
         playerPile.insertSorted
             (((startTurn & START_GAME) && isJoker (dumped.getTopCard ()))
              ? dumped.removeTopCard () : staple.removeTopCard ());

      startTurn = 0;
   if (target == -1U)
      target = executeMove (player);
   return executeMove (player);
//-----------------------------------------------------------------------------
/// Executes a move for the passed player; only one move is made at every
/*--------------------------------------------------------------------------*/
//Purpose   : Executes a move for the passed player; only one move is made at
//            every timer-iteration
//Parameters: player: Actual player
//Returns   : ID for target (32 Bit: Pile << 16 + Position)
/*--------------------------------------------------------------------------*/

   TRACE6 ("Buraco::executeMove (player) - Checking for 3 in a row");

   // Check for 3 cards having the same number
   // Check if any card can be added to an existing pile
   unsigned int count (1);
   unsigned int i (0);
   for (; i < playerPile.size (); ++i) {
      TRACE8 ("Buraco::executeMove (player) - Analyzing " << playerPile[i]);
      Check3 (playerPile[i]);

      // Check if the actual card can be added to an existing pile
      if (tablePiles[player & 1].size ()) {
         TRACE8 ("Buraco::executeMove (player) - Adding single " << playerPile[i]);
         unsigned int target (cardFitsOnPlayedPile (player, i));
         if ((target != -1U) && canDumpCards (player, 1))

   // Check for 3 cards belonging to a serie

      // Check if there are 3 (or more) of a kind
      if (i && (playerPile[i]->number () == playerPile[i - 1]->number ())) {
         TRACE8 ("Buraco::executeMove (player) - Adding new pile "
                 << playerPile[i]);
         if (++count == 7)
            break;
      }
      else {
         if ((count >= 3) && !isJoker (*playerPile[i - 1]))
            break;
         count = 1;

   // Check if all cards in the hand can (and should) be played
   if ((count >= 3)  && !isJoker (*playerPile[i - 1]))
      if (canDumpCards (player, count) || (count-- > 3)) {
         CardVPile& pile (makeNewPile (player & 1)); // Create new pile with
         flipCards2Play (playerPile, pos1 = i - count, pos2 = i - 1);
         return (tablePiles[player & 1].size () - 1) << 16;
      }
   TRACE8 ("Buraco::executeMove (unsigned int) - Playing all?");
   if (!unfinishedMonoPiles[player & 1]
   TRACE8 ("Buraco::executeMove (player) - Playing all?");
   if ((!reserve[player & 1].empty () || cerrados[player & 1]) 
       && canGetRidOfCards (player)) {
      // Find first non-joker
      std::vector<CardWidget*>::const_iterator ci (playerPile.begin ());
      for (; ci != playerPile.end (); ++ci) {
         if (!isJoker (**ci))
            break;
      }
      // If non joker found
      if (ci != playerPile.end ()) {
         if (((ci + 1) != playerPile.end ())
             && cardFitsNext (ci)) {
            unsigned int pos (ci - playerPile.begin ());
            playerPile.move (playerPile.size () - 1, pos);
            playerPile.move (playerPile.size () - 1, pos);
            if (!isJoker (*playerPile[playerPile.size () - 3])) {
               Check3 (isJoker (*playerPile[0]));
               playerPile.move (playerPile.size () - 1, 0);
            }
            makeNewPile (player & 1);
            // Create new pile with the found pair and a joker
            CardVPile& pile (makeNewPile (player & 1));
            flipCards2Play (playerPile, pos1 = playerPile.size () - 3,
                            pos2 = playerPile.size () - 1);
         ++ci;
      }

      else
         // Get rid off jokers, if team has already a "cerrado" (7 equal)
         if (cerrados[player & 1]) {
            for (std::vector<CardVPile*>::const_iterator p (tablePiles[player & 1].begin ());
                 p != tablePiles[player & 1].end (); ++p)
               if (containsNoJoker (**p)) {
                  flipCards2Play (playerPile, pos1 = 0, pos2 = 0);
                  return (p - tablePiles[player & 1].begin ()) << 16;
               }
         }
   if (containsOnlyJoker (playerPile))
      if (!reserve[player & 1].empty ()) {
         addBuraco (player);
         return executeMove (player);
         addReserve (player);

   // No more cards to put down: Find a card to dump
   TRACE8 ("Buraco::executeMove (unsigned int) - Searching for a card to dump");
   for (i = 0; i < playerPile.size () - 1; ++i) {
      ICardPile::const_iterator p (playerPile.getFittingCard (*playerPile[i], playerPile.begin (),
   i = 0;
   for (; i < playerPile.size () - 1; ++i) {
      if (isJoker (*playerPile[i]))
          continue;

      if (cardFitsNext (playerPile.begin () + i))
         ++i;
      else

   }
      --i;
   if (i == playerPile.size ())
      --i;
   while (isJoker (*playerPile[i]) && i)              // Try to not dump jokers
      --i;
   pos1Play = pos2Play = i;
   return 0xffff0000;
   flipCards2Play (playerPile, pos1 = i, pos2 = i);
   return 0xffff << 16;
//-----------------------------------------------------------------------------
/// Starts the game by dealing the cards
/*--------------------------------------------------------------------------*/
//Purpose   : Starts the game by dealing the cards
/*--------------------------------------------------------------------------*/
   Game::start ();

   if (pScoreDlg) {
      unsigned int player;
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
       hands[i].sortByNumber ();

   dumped.setTopCard (staple.removeTopCard ());
   TRACE9 ("Buraco::start () - NUMBER OF CARDS: " << staple.size ());

   status.pop ();
   status.push (_("You can sort the cards in your hand with drag and drop or put"
                  " them on the table - click card to dump to end turn"));

   startTurn = START_TURN | START_GAME;
   cerrados[0] = cerrados[1] = 0;
   updateInfo ();
   setNextPlayer (startPlayer);
   displayTurn (startPlayer++);
   startPlayer &= 0x3;
   hands[0].show ();
   hands[startPlayer ? startPlayer : 1].show ();
   makeNextMoves ();
//-----------------------------------------------------------------------------
/// Remove cards from everything which can hold them
/*--------------------------------------------------------------------------*/
//Purpose   : Remove cards from everything which can hold them
/*--------------------------------------------------------------------------*/
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
           p != tablePiles[i].end (); ++p)
      tablePiles[i].clear ();

   for (unsigned int i (0); i < (sizeof (reserve) / sizeof (reserve[0])); ++i)
      reserve[i].clear ();

   Game::clean ();


//-----------------------------------------------------------------------------
/// Enables the cards the human can pick up.
/*--------------------------------------------------------------------------*/
//Purpose   : Enables the cards the human can pick up.
//Returns   : 0
/*--------------------------------------------------------------------------*/
   Check3 (!stapleTop.connected ()); Check3 (!dumpedTop.connected ());
   Check3 (staple.size ());
   Check3 (dumped.size ());
   dumpedTop = dumped.getTopCard ().signal_clicked ().connect
      (slot (*this, (&Buraco::stapleSelected)));

      (slot (*this, (&Buraco::dumpedSelected)));
}

//-----------------------------------------------------------------------------
/// Enables the cards in the hand of the human player
/*--------------------------------------------------------------------------*/
//Purpose   : Enables the cards in the hand of the human player
/*--------------------------------------------------------------------------*/
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
/*--------------------------------------------------------------------------*/
//Purpose   : Disables the cards the human player can select
/*--------------------------------------------------------------------------*/
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
//-----------------------------------------------------------------------------
/// Callback after clicking on a card in the hand
/*--------------------------------------------------------------------------*/
//Purpose   : Callback after clicking on a card in hand
//Parameters: iCard: Offset of card in hand
/*--------------------------------------------------------------------------*/
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
   // If the player has no more cards left (except of joker): Give him the reserve
         addBuraco (0);
      else if (hands[0].empty ()) {
         addReserve (0);
         endGame ();
      }

   gStatus.startTurn = 1;
   gStatus.startGame = 0;
   hands[2].hide ();

   makeNextMoves ();
}

//-----------------------------------------------------------------------------
/// Callback after clicking on the staple
/*--------------------------------------------------------------------------*/
//Purpose   : Callback after clicking on the staple
/*--------------------------------------------------------------------------*/
   Check1 (gameStatus () == PLAYING);
   Check3 (staple.size ());
   Check2 (dumped.size ());
   Check3 (stapleTop.connected ()); Check3 (dumpedTop.connected ());

   // Move top card to human and enable the cards in his hand, when idle
   // (means: *after* this signalhandler termintes)
   Glib::signal_idle ().connect
      (bind_return (mem_fun (*this, &Buraco::doStapleSelected), false));
}
       (bind_return (slot (*this, &Buraco::doStapleSelected), false));
//-----------------------------------------------------------------------------
/// Delayed callback after clicking on the staple
/*--------------------------------------------------------------------------*/
//Purpose   : Delayed callback after clicking on the staple
/*--------------------------------------------------------------------------*/
   Check1 (gameStatus () == PLAYING);
   Check2 (staple.size ());
   Check2 (dumped.size ());
   Check3 (staple.size ());
   unsigned int player (currentPlayer ());
   hands[0].append (staple.removeTopCard ());
   enableHumanHand ();
//-----------------------------------------------------------------------------
/// Callback after clicking on the dumped staple
/*--------------------------------------------------------------------------*/
//Purpose   : Callback after clicking on the dumped staple
/*--------------------------------------------------------------------------*/
   Check1 (gameStatus () == PLAYING);
   Check3 (dumped.size ());
   Check3 (stapleTop.connected ()); Check3 (dumpedTop.connected ());

      try {
   if (!((startTurn & START_GAME)
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
/*--------------------------------------------------------------------------*/
//Purpose   : Delayed callback after clicking on the dumped staple
/*--------------------------------------------------------------------------*/
   Check1 (gameStatus () == PLAYING);
   TRACE5 ("Buraco::doDumpedSelected ()");
   unsigned int player (currentPlayer ());


   // Special handling of human starts the game and can choose one of the
   // first two cards
   if (startTurn & START_GAME) {
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
/*--------------------------------------------------------------------------*/
//Purpose   : Enables a card in the hand of the player
/*--------------------------------------------------------------------------*/
   Check1 (pos < hands[0].size ());

   activeCards.push_back
      (hands[0][pos]->signal_clicked ().connect
       (bind (mem_fun (*this, (&Buraco::cardSelected)), pos)));
}
       (bind (slot (*this, (&Buraco::cardSelected)), pos)));
//-----------------------------------------------------------------------------
/// Prepares the card for drag´n´drop
/*--------------------------------------------------------------------------*/
//Purpose   : Prepares the card for drag´n´drop
//Parameters: iCard: Number of card in hand
/*--------------------------------------------------------------------------*/
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
   aDNDHand[&card] = card.signal_drag_data_received ().connect
      (bind (slot (*this, &Buraco::cardDropped), iCard));
   card.signal_drag_data_get ().connect
      (bind (slot (*this, &Buraco::getDropData), iCard));
//-----------------------------------------------------------------------------
/// Stops the drag´n´drop abilities of the passed card
/*--------------------------------------------------------------------------*/
//Purpose   : Stops the drag´n´drop abilities of the passed card
//Parameters: card: Card to unregister of dnd
/*--------------------------------------------------------------------------*/
           << " -> Address: " << &card);
   Check1 (aDNDHand.size ());
           << " -> Address: " << std::hex << &card << std::dec);
   std::map<CardWidget*, CONNECTIONS>::iterator i (aDNDHand.find (&card));
   Check1 (i != aDNDHand.end ());
   std::map<CardWidget*, SigC::Connection>::iterator i (aDNDHand.find (&card));
   card.drag_dest_unset ();
   card.drag_source_unset ();
   i->second.connReceive.disconnect ();
   i->second.connGet.disconnect ();
   i->second.disconnect ();

//-----------------------------------------------------------------------------
/// Prepares the passed region of cards for drag´n´drop
/*--------------------------------------------------------------------------*/
//Purpose   : Prepares the passed region of cards for drag´n´drop
//Parameters: pile: Pile whose cards should be registered
//            start: Number of first card to prepare for DND
//            end: Number of last card to prepare for DND
//Requieres : start < end; end <= cards
/*--------------------------------------------------------------------------*/
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
/*--------------------------------------------------------------------------*/
//Purpose   : Prepares the card for drag´n´drop
//Parameters: card: Card to register
//            nr: Number of card in pile
/*--------------------------------------------------------------------------*/
           << " = " << std::hex << nr << " - " << &card << std::dec);

   // Card accepts drops from hand and drags from table
   card.drag_dest_set (dndType, Gtk::DEST_DEFAULT_ALL, Gdk::ACTION_MOVE);
   aDNDTable[&card] = card.signal_drag_data_received ().connect
      (bind (mem_fun (*this, &Buraco::cardDroppedOnTable), nr));
}
      (bind (slot (*this, &Buraco::cardDroppedOnTable), nr));
//-----------------------------------------------------------------------------
/// Stops the drag´n´drop abilities of the passed card
/*--------------------------------------------------------------------------*/
//Purpose   : Stops the drag´n´drop abilities of the passed card
//Parameters: card: Card to de-register
/*--------------------------------------------------------------------------*/
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
/*--------------------------------------------------------------------------*/
//Purpose   : Callback after dropping a card
//Parameters: pContext: Context of the drag (contains things like source,
//                      target, action, ...)
//            pData: Describes the thing which was dropped
//            info: Describes the type of pData (should be 0)
//            time: Timestamp of the drag
//            card: Number of card where something was dropped at
//Requieres : pContext, pData not NULL; Expects info to be 0
/*--------------------------------------------------------------------------*/
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
/*--------------------------------------------------------------------------*/
//Purpose   : Checks if the piles on the table are valid (have at least 3 cards)
//Parameters: except: Pile which can be invalid
//Returns   : True, if the piles are OK
/*--------------------------------------------------------------------------*/
        p != tablePiles[0].end (); ++p) {
   for (std::vector<CardVPile*>::const_iterator p (tablePiles[0].begin ());
      if ((p - tablePiles[0].begin ()) == static_cast<int> (except))
      Check3 (*p); Check3 ((*p)->size () < 7);
      if ((p - tablePiles[0].begin ()) == except)
      if ((*p)->size () < 3)
         return false;
   }
   return true;
}

//-----------------------------------------------------------------------------
/// Callback after dropping a card on the table
/*--------------------------------------------------------------------------*/
//Purpose   : Callback after dropping a card on the table
//Parameters: pContext: Context of the drag (contains things like source,
//                      target, action, ...)
//            pData: Describes the thing which was dropped
//            info: Describes the type of pData (should be 0)
//            time: Timestamp of the drag
//            iCard: Combination of card and pile on which card was dropped
//Requieres : pContext, pData not NULL;
/*--------------------------------------------------------------------------*/
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
   // Only allow dropping of last card, if the game can be ended, or there
   // is still the reserve
   if (!canDumpCards (0, 1)) {
      context->drag_finish (false, false, time);
      Gtk::MessageDialog dlg (_("You can't end the game (there's no \"cerrado\")!"),
                              Gtk::MESSAGE_ERROR);
      dlg.set_title (_("Invalid move"));
      dlg.run ();
      return;
   }

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
      if (isJoker (moved)) {                // Joker?: Check if there is a pair
         if (!pileHasFittingPair (hands[0])) {
            context->drag_finish (false, false, time);
            Gtk::MessageDialog dlg (_("Can't drop a joker without having a pair!"),
                                    Gtk::MESSAGE_ERROR);
            dlg.set_title (_("Invalid move"));
            dlg.run ();
            return;
         }
      }
      else {
         // Check validity of ordinary dropped card
         unsigned int equalNr (1);
         unsigned int equalColours (0);
         bool joker (false);
         for (unsigned int i (0); i < hands[0].size (); ++i) {
            Check3 (hands[0][i]);
            if (isJoker (*hands[0][i]) && !joker) {
               joker = true;
               equalNr++;
               continue;
            }
            if (moved.number () == hands[0][i]->number ())
               ++equalNr;
            else if (moved.colour () == hands[0][i]->colour ()) {
               int diff (moved.number () - hands[0][i]->number ());
               if ((unsigned int)(diff + 2) > 5)
                  continue;
               equalColours |= (diff < 0) ? -diff << 2 : diff;
            }
         }
         Check3 (equalColours < 0x10);

         // Card can be dropped, if there are 3 equal numbers
         TRACE9 ("Buraco::cardDroppedOnTable (...) - Equal: " << equalNr
                 << "; Colours: " << std::hex << equalColours << std::dec);
         if ((equalNr < 4)        // Compare with 4 as card itself also matched
             && (equalColours != 0x3) && (equalColours != 0x6)
             && (equalColours != 0xc) && !(equalColours && joker)) {
            context->drag_finish (false, false, time);
            Gtk::MessageDialog dlg (_("There are no cards to make three of a kind!"),
                                    Gtk::MESSAGE_ERROR);
            dlg.set_title (_("Invalid move"));
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
         addReserve (0);
      else
         if (hands[0].empty ()) {
            points[0] += 100;
            endGame ();
         }

   // Re-register the cards in the hand of the human for DND
   if (*pValue < hands[0].size ())
  // Re-register the cards in the hand for DND
   Check3 (aDNDHand.size () == hands[0].size ());
}

//-----------------------------------------------------------------------------
/// Callback to query the data to drop
/*--------------------------------------------------------------------------*/
//Purpose   : Callback to query the data to drop
//Parameters: pContext: Context of the drag (contains things like source,
//                      target, action, ...)
//            pData: Describes the thing which was dropped
//            time: Timestamp of the drag
//            cardPos: Position of card (either in hand or pile on table)
//Requieres : pContext, pData not NULL; Expects info to be 0
/*--------------------------------------------------------------------------*/
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
/*--------------------------------------------------------------------------*/
//Purpose   : Prepares the passed region of cards for drag´n´drop
//Parameters: start: Number of first card to prepare for DND
//            end: Number of last card to prepare for DND
//Requieres : start < end; end <= cards
/*--------------------------------------------------------------------------*/
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
/*--------------------------------------------------------------------------*/
//Purpose   : Checks, if the passed pile contains no cards except jokers or 2s.
//            This is also true for empty piles.
//Parameters: pile: Pile to inspect
//Returns   : True, if there are only jokers (or pile is empty)
/*--------------------------------------------------------------------------*/

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
/*--------------------------------------------------------------------------*/
//Purpose   : Checks, if the passed pile does not containcards neither jokers
//            nor 2s.
//Parameters: pile: Pile to inspect
//Returns   : True, if there are no jokers
/*--------------------------------------------------------------------------*/
        i != pile.end (); ++i) {
      Check3 (*i);
      if (isJoker (**i))
         return false;
   }
   return true;
}

//-----------------------------------------------------------------------------
/// Adds the buraco to the passed player.
/*--------------------------------------------------------------------------*/
//Purpose   : Checks, if the passed pile contains no cards except jokers or 2s.
//            This is also true for empty piles.
//Parameters: player: Player getting the reserve
//            showt: Flag, if info-message should be displayed
/*--------------------------------------------------------------------------*/
void Buraco::addReserve (unsigned int player, bool show) {
   undo.pickUp = 1;
   undo.cJokers = hands[player].size ();
      Game::disableHuman ();
      for (unsigned int i (0); i < hands[0].size (); ++i)
         unregisterHandDND (*hands[0][i]);
   }

   // Add reserve
   sort (reserve[player & 1].begin (), reserve[player & 1].end (),
	 compByNumberWithJokers);
   hands[player].setTopCards (reserve[player & 1]);
   hands[player].sortByNumber ();
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


//-----------------------------------------------------------------------------
/// Hides the joker, which are displayed when picking up the buraco
/*--------------------------------------------------------------------------*/
//Purpose   : Checks, if the passed pile contains no cards except jokers or 2s.
//            This is also true for empty piles.
//Parameters: pile: Pile add reserve to
//            team: Which reserve to use
/*--------------------------------------------------------------------------*/
CardVPile& Buraco::makeNewPile (unsigned int team) {
   TRACE9 ("Buraco::makeNewPile (unsigned int) - New pile for team " << team);

   Check1 ((sizeof (tablePiles) / sizeof (tablePiles[0]))
            == (sizeof (boxTeam) / sizeof (boxTeam[0])));
   tablePiles[team].push_back (pile);
   CardVPile* pile (new CardVPile (ICardPile::COMPRESSED, ICardPile::SHOWFACE));

   pile->show ();
   return *pile;
}

//-----------------------------------------------------------------------------
/// Checks if the passed card can be put on one of the existing piles
/*--------------------------------------------------------------------------*/
//Purpose   : Checks if the passed card can be put on one of the existing piles
//Parameters: player: Player to inspect
//            iCard: Card to inspect
//Returns   : Value describing the pile (and the offset of the card) to play to; -1 if none
//Remarks   : This method actually moves the card
/*--------------------------------------------------------------------------*/
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

      // one having picked up the reserve already played (the missing card
      // Play joker, if you can make a cerrado (7 in a row) - but only if
      // you haven't already picked up the reserve (the missing card might be
      // in there and the oponent can't finish
      if (isJoker (card)) {
         if (((*p)->size () == 6) && containsNoJoker (**p)
             && (reserve[player & 1].empty ()
                 || ((reserve[(player + 1) & 1].empty ())
                     && cerrados[(player + 1) & 1]))) {
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
/*--------------------------------------------------------------------------*/
//Purpose   : Checks if the passed card is a joker
//Parameters: card: Card to inspect
//Returns   : True if card is a joker
/*--------------------------------------------------------------------------*/
           || (card.number () > CardWidget::ACE));
   TRACE9 ("Buraco::isJoker (const CardWidget&) const - " << card << " = "
           << card.number ());
   return (card.number () == CardWidget::TWO) || (card.number () > CardWidget::ACE);
//-----------------------------------------------------------------------------
/// Removes a cerrado (a pile with 7 cards) from the table
/*--------------------------------------------------------------------------*/
//Purpose   : Removes a cerrado (a pile with 7 cards) from the table
//Parameters: player: Player causing the remove of the pile
//            pile: Pile holding the cerrado
/*--------------------------------------------------------------------------*/
void Buraco::removeCerrado (unsigned int player, CardVPile& pile) {
           != tablePiles[team].end ());
   unsigned int team (player & 1);
   std::vector<CardVPile*>::iterator i
      (std::find (tablePiles[team].begin (), tablePiles[team].end (), &pile));
   Check1 (i != tablePiles[team].end ());
   Check3 (static_cast<int> (pile.getPotentialPoints ()) == pile.getPoints ());
   TRACE9 ("Buraco::removeCerrado (unsigned int, CardVPile&) - Pile "
           << (i - tablePiles[team].begin ()) << " of team " << team);
   boxTeam[team].remove (pile);
   i = tablePiles[team].erase (i);

   // De- and re-register DND for removed and following piles
   if (!player) {
      while (i != tablePiles[0].end ()) {
         Check3 (*i); Check3 ((*i)->size () >= 3);
         registerTableDND (static_cast<unsigned int> (i - tablePiles[0].begin ()),
                           0, (*i)->size () - 1);
         ++i;
      }

      for (ICardPile::iterator c (pile.begin ()); c != pile.end (); ++c)
         unregisterTableDND (**c);
   }
   delete &pile;
   cerrados[team]++;

//-----------------------------------------------------------------------------
/// Actualizes the info-part of the statusbar
/*--------------------------------------------------------------------------*/
//Purpose   : Actualizes the info-part of the statusbar
/*--------------------------------------------------------------------------*/
   strInfo.replace (strInfo.find ("%1"), 2, YGP::ANumeric::toString (points[0]));
   std::string strInfo (_("Cerrados [Buracos]: %1 [%2] / %3 [%4]"));
   strInfo.replace (strInfo.find ("%1"), 2, ANumeric::toString (cerrados[0]));
   strInfo.replace (strInfo.find ("%2"), 2, 1, (reserve[0].empty () ? 'N' : 'Y'));
   strInfo.replace (strInfo.find ("%3"), 2, ANumeric::toString (cerrados[1]));
   strInfo.replace (strInfo.find ("%4"), 2, 1, (reserve[1].empty () ? 'N' : 'Y'));
}
   info.pop ();
   info.push (strInfo);
//-----------------------------------------------------------------------------
/// Checks if the passed card fits on the passed staple
/*--------------------------------------------------------------------------*/
//Purpose   : Checks if the passed card fits on the passed staple
//Parameters: pile: Pile to inspect
//            card: Card to check
//Returns   : Position where card can be played to, or -1 if card does not fit
//Requires  : Coloured piles must be sorted strict ascending
/*--------------------------------------------------------------------------*/
int Buraco::cardFitsOnPile (ICardPile& pile, const CardWidget& card) const {
   Check1 (pile.size ());
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

   // Check if pile contains only jokers; only accept another joker or
   // - if the pile has only 1 card - a card which is part of a pair
   if (first == -1U)
      return (isJoker (card)
              ? 0
              : (posJoker
                 ? - 1
                 : (pileHasFittingPair (hands[currentPlayer ()], card, false)
                    ? 0 : -1)));

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
             && (card.number () == (pile[first]->number () + posJoker))) {
            pile.move (0, posJoker);
            return posJoker;
         }

         // This code assums that the coloured pile is sorted from lower card
         // to higher cards (strict ascending)
         Check3 (pile[first]->number () <= pile[last]->number ());
         // Possible difference the card can have: 1 or two if joker at one end
         unsigned int maxDiff ((posJoker == -1U) ? 1
                               : ((posJoker < first) || (posJoker > last)) ? 2 : 1);
         unsigned int diff (pile[first]->number () - card.number ());
         TRACE9 ("Buraco::cardFitsOnPile (CardVPile&, CardWidget&) - Diff (start): "
                 << diff << "; max: " << maxDiff);
         Check3 (diff);
         if (diff && (diff <= maxDiff)) {
            if ((diff == 2) && posJoker > first) {
               Check3 (!first);
               pile.move (0, posJoker);
               ++first;
            }
            return first - diff + 1;
         }

         // Test if card fits at other end
         diff = card.number () - pile[last]->number ();
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
/*--------------------------------------------------------------------------*/
//Purpose   : Shows or hides the cards of the computer player
//Parameters: open: Flag if cards should be shown or hidden
/*--------------------------------------------------------------------------*/
      hands[i].setShowOption (open ? ICardPile::SHOWFACE : ICardPile::SHOWBACK);
   for (unsigned int i (1); i < NUM_PLAYERS; ++i)
   }
//-----------------------------------------------------------------------------
/// Creates the combined team names from the players
/*--------------------------------------------------------------------------*/
//Purpose   : Performs the steps to end the game
/*--------------------------------------------------------------------------*/

   status.pop ();
   status.push (_("Game ended"));

   if (!pScoreDlg) {
      pScoreDlg = ScoreDlg::create (nameTeams);
   setGameStatus (STOPPED);   

//-----------------------------------------------------------------------------
/*--------------------------------------------------------------------------*/
//Purpose   : Checks if the player can get rid of all cards in his hand
//            except of the jokers
//Parameters: player: Player whose cards should be inspected
//Returns   : True: if all cards can be played
//Remarks   : This method does not check for triplets anymore!
/*--------------------------------------------------------------------------*/
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
/*--------------------------------------------------------------------------*/
//Purpose   : Checks if the player can dump the specified number of cards; a
//            player can only dump all of his cards, if:
//              - The team has a cerrado
//              - The team still has the reserve
//Parameters: player: Player to analyze
//Returns   : True, if card can be played
/*--------------------------------------------------------------------------*/
bool Buraco::canDumpCards (unsigned int player, unsigned int cards) const {
   unsigned int cPile (hands[player].size ());
   return (cPile >= (cards + 2) || cerrados[player & 1]
           || !reserve[player & 1].empty ());
//----------------------------------------------------------------------------

/*--------------------------------------------------------------------------*/
//Purpose   : Checks if the passed pile contains a pair matching the passed
//            card
//Parameters: pile: Pile to inspect
//            card: Card where to find a pair to
//            pileHoldsCard: Flag, if the pile contains the card (to skip)
//Returns   : True, if the pile contains a matching pair
/*--------------------------------------------------------------------------*/
bool Buraco::pileHasFittingPair (const ICardPile& pile, 
                                 const CardWidget& card, bool pileHoldsCard) {
   TRACE3 ("Buraco::pileHasFittingPair (const ICardPile&, const CardWidget*, bool) "
           " - " << card);
      ICardPile::const_iterator i (pile.getFittingCard (card, pile.begin (),
   bool bJoker (isJoker (card));
   CardWidget::NUMBERS nr (card.number ());
   CardWidget::COLOURS colour (card.colour ());
   unsigned int nrs (0);
   unsigned int bCols (0);

   for (std::vector<CardWidget*>::const_iterator p (pile.begin ());
        p != pile.end (); ++p) {
      if (pileHoldsCard && (*p == &card))    // Skip card if its the passed one
         continue;

      if (bJoker) {
         if (isJoker (**p) && (++nrs == 2))
            return true;
      }
      else {
         if ((*p)->number () == nr) {
            if (++nrs == 2)
               return true;
         }
         else
            if ((*p)->colour () == colour) {
               int diff ((*p)->number () - nr);
               diff = (diff < 0) ? (diff + 2) : (diff + 1);
               TRACE9 ("Buraco::pileHasFittingPair (const ICardPile&, const "
                       "CardWidget*) - " << **p << " differs " << diff);
               if ((diff < 4) && !(bCols & (1 << diff))) {
                   if (bCols & (diff ? (0x5 << (diff - 1)) : 0x1))
                      return true;
                   bCols |= (1 << diff);
               }
            }
      }
   }
   TRACE9 ("Buraco::pileHasFittingPair (const ICardPile&, const "
           "CardWidget*, bool) - " << card << " matches " << nrs << '/'
           << std::hex << bCols << std::dec);
   return false;
//-----------------------------------------------------------------------------
/// Checks if the passed pile contains a pair matching the passed card
/*--------------------------------------------------------------------------*/
//Purpose   : Checks if the passed pile contains a pair matching the passed
//            card
//Parameters: pile: Pile to inspect
//            card: Card where to find a pair to
//Returns   : True, if the pile contains a matching pair
/*--------------------------------------------------------------------------*/
bool Buraco::pileHasFittingPair (const ICardPile& pile) {
   TRACE3 ("Buraco::pileHasFittingPair (const ICardPile&)");
        p != pile.end (); ++p)
      if (*p != exclude)
           p != pile.end (); ++p)
       if (pileHasFittingPair (pile, **p, false))
          return true;

}

/*--------------------------------------------------------------------------*/
//Purpose   : Checks if the card builds a pair with the following one
//Parameters: i: Iterator to card
//Returns   : True, if card fits to next
//Requires  : Pile must be sorted
/*--------------------------------------------------------------------------*/
bool Buraco::cardFitsNext (ICardPile::const_iterator i) {
   TRACE3 ("Buraco::cardFitsNext (ICardPile::const_iterator) - " << **i);
   return (((*i)->number () == (*(i + 1))->number ())
           || (((*i)->colour () == (*(i + 1))->colour ())
               && (*i)->number () == ((*(i + 1))->number () + 1)));
