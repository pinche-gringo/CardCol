//$Id$

//PROJECT     : Cardgames
//SUBSYSTEM   : Machiavelli
//REFERENCES  :
//TODO        :
//BUGS        :
//REVISION    : $Revision$
//AUTHOR      : Markus Schwab
//CREATED     : 05.11.2003
//COPYRIGHT   : Copyright (C) 2003, 2004

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

#include <sstream>

#include <gtk/gtkdnd.h>

#include <gtkmm/statusbar.h>

#include <YGP/Check.h>
#include <YGP/Trace.h>
#include <YGP/ConnMgr.h>
#include <YGP/ANumeric.h>
#include <YGP/Tokenize.h>
#include <YGP/AttrParse.h>
#include <YGP/StatusObj.h>

#include <XGP/MessageDlg.h>

#include <Player.h>
#include <CardSet.h>
#include <CardWidget.h>
#include <ComputerPlayer.h>

#include "Machiavelli.h"


enum { HAND, TABLE };
static std::vector<Gtk::TargetEntry> dndTypeHand;
static std::vector<Gtk::TargetEntry> dndTypeTable;
static std::vector<Gtk::TargetEntry> dndTypeBoth;


//-----------------------------------------------------------------------------
/// Constructor
/// \param parent: Parent widget to display the game in
/// \param statusbar: Status bar widget to display information about the game
/// \param cardset: Cardset to use
/// \param player: Vector of player
/// \param posPlayer: Position of player for the server
/// \param mxSerialize: Mutex to serialize messages from the server
//-----------------------------------------------------------------------------
Machiavelli::Machiavelli (Gtk::Box& parent, Gtk::Statusbar& statusbar,
                CardSet& cardset, const std::vector<Player*>& player,
                unsigned int posPlayer, YGP::Mutex& mxSerialize)
   : Game (parent, statusbar, cardset, player, posPlayer, mxSerialize, 3, 10)
     , startPlayer (-1U), newPile (_("New pile"))
     , staple (ICardPile::TOTALLY_COMPRESSED, ICardPile::SHOWBACK)
     , nextTurn (_("_End turn"), true)
     , target (-1U), undoDlg (NULL) {
   TRACE9 ("Machiavelli::Machiavelli (Box&, Statusbar&, CardSet&, const "
           "std::vector<Glib::ustring>&)");

   int width (cards.getCard (0).getImageWidth ());
   int height (cards.getCard (0).getImageHeight ());

   TRACE9 ("Machiavelli::Machiavelli (Box&, Statusbar&, CardSet&, const "
           "std::vector<Glib::ustring>&) - Init common staples");
   staple.set_size_request (width, height);

   for (unsigned int i (1); i < NUM_PLAYERS; ++i) {
      attach (hands[i], (i << 2) - 4, (i << 2), 4, 5,
              Gtk::EXPAND, Gtk::SHRINK, 5, 5);
      attach (names[i], (i << 2) - 4, (i << 2), 5, 6,
              Gtk::EXPAND, Gtk::SHRINK, 0);
      hands[i].setStyle (ICardPile::QUITE_COMPRESSED);
      hands[i].setShowOption (ICardPile::SHOWBACK);
   }
   hands[0].setStyle (ICardPile::COMPRESSED);
   hands[0].setShowOption (ICardPile::SHOWFACE);

   TRACE9 ("Machiavelli::Machiavelli (Box&, Statusbar&, CardSet&, const "
           "std::vector<Glib::ustring>&) - Attach widgets");
   attach (hands[0], 3, 12, 0, 1, Gtk::EXPAND, Gtk::SHRINK, 1, 5);
   attach (names[0], 3, 12, 1, 2, Gtk::EXPAND, Gtk::SHRINK, 1, 5);
   attach (staple, 0, 1, 0, 1, Gtk::SHRINK, Gtk::SHRINK, 5);
   attach (nextTurn, 0, 1, 1, 2, Gtk::FILL, Gtk::SHRINK, 5);
   attach (newPile, 0, 12, 2, 3, Gtk::EXPAND | Gtk::FILL, Gtk::FILL, 0, 5);
   attach (piles, 0, 12, 3, 4, Gtk::EXPAND | Gtk::FILL,
           Gtk::EXPAND | Gtk::FILL, 0, 5);

   TRACE9 ("Machiavelli::Machiavelli (Box&, Statusbar&, CardSet&, const "
           "std::vector<Glib::ustring>&) - Show widgets");

   changeNames (player);

   if (dndTypeHand.empty ()) {
      Check3 (dndTypeTable.empty ());
      Check3 (dndTypeBoth.empty ());
      dndTypeHand.push_back
         (Gtk::TargetEntry ("icon/card/hand", Gtk::TARGET_SAME_APP, HAND));
      dndTypeTable.push_back
         (Gtk::TargetEntry ("icon/card/table", Gtk::TARGET_SAME_APP, TABLE));

      dndTypeBoth.push_back (dndTypeHand.front ());
      dndTypeBoth.push_back (dndTypeTable.front ());
   }

   nextTurn.set_flags (Gtk::CAN_DEFAULT);
   nextTurn.grab_default ();
   nextTurn.set_sensitive (false);
   nextTurn.signal_clicked ().connect (mem_fun (*this, (&Machiavelli::endTurn)));

   show_all_children ();
}

//-----------------------------------------------------------------------------
/// Destructor
//-----------------------------------------------------------------------------
Machiavelli::~Machiavelli () {
   TRACE9 ("Machiavelli::~Machiavelli ()");
   clean ();
}


//-----------------------------------------------------------------------------
/// Starts the game by dealing the cards
//-----------------------------------------------------------------------------
void Machiavelli::start () {
   TRACE9 ("Machiavelli::start ()");
   Game::start ();

   pos1Play = pos2Play = 0;
   target = -1U;

   if (randomizeCardsToPile (staple)) {
      for (unsigned int i (0); i < NUM_PLAYERS; ++i)
          for (unsigned int j (0); j < 7; ++j)
             hands[(i - posServer) & 0x3].append (staple.removeTopCard ());

      hands[0].sortByColour ();
      for (unsigned int i (1); i < NUM_PLAYERS; ++i) {
         hands[i].setStyle (ICardPile::QUITE_COMPRESSED);
         hands[i].setShowOption (ICardPile::SHOWBACK);
         hands[i].sortByNumber ();
      }

      status.pop ();
      status.push (_("You can sort the cards in your hand with drag and drop or put"
                     " them on the table - click the staple to end turn"));

      if (getConnectionMgr ().getMode () != YGP::ConnectionMgr::CLIENT) {
         // Set random startplayer (if not already set)
         if (startPlayer == -1U)
            startPlayer = rand () & 0x3;
         setStartPlayer ();
      }
   }
}

//-----------------------------------------------------------------------------
/// Remove cards from everything which can hold them
//-----------------------------------------------------------------------------
void Machiavelli::clean () {
   TRACE9 ("Machiavelli::clean ()");
   disableHuman ();
   for (unsigned int i (0); i < NUM_PLAYERS; ++i)
      hands[i].clear ();

   staple.clear ();

   for (std::vector<MachiPile*>::iterator i (tablePiles.begin ());
        i != tablePiles.end (); ++i) {
      (*i)->clear ();
      piles.remove (**i);
   }
   tablePiles.clear ();

   Game::clean ();
}

//-----------------------------------------------------------------------------
/// Shows or hides the cards of the computer player
/// \param open: Flag if cards should be shown or hidden
//-----------------------------------------------------------------------------
void Machiavelli::playOpen (bool open) {
    for (unsigned int i (1); i < NUM_PLAYERS; ++i) {
      hands[i].setShowOption (open ? ICardPile::SHOWFACE : ICardPile::SHOWBACK);
      hands[i].setStyle (open ? ICardPile::COMPRESSED : ICardPile::QUITE_COMPRESSED);
    }
}

//-----------------------------------------------------------------------------
/// Makes the move for the next player.
/// \param player: Actual player
/// \returns \c int: Next player or -1 if end of game
/// \remarks This method expects the target pile to play in the target-member
///     and the positions to play in pos1Play and pos2Play
//-----------------------------------------------------------------------------
int Machiavelli::makeMove (unsigned int player) {
   TRACE5 ("Machiavelli::makeMove (unsigned int) - Turn of player " << player
           << "; Target: " << std::hex << (int)target << std::dec);
   Check1 (player); Check1 (player < NUM_PLAYERS);
   Check1 (gameStatus () == PLAYING);

   if (target == -1U) {
      target = showCardsToPlay (player);
      TRACE8 ("Machiavelli::makeMove (unsigned int) - Going to play cards to "
              << std::hex << (int)target << std::dec);

      // No more cards found: Continue with next player
      if (target == -1U) {
         if (getConnectionMgr ().getMode () != YGP::ConnectionMgr::NONE) {
            // Send played card to all clients (if any)
            if (getConnectionMgr ().getMode () == YGP::ConnectionMgr::CLIENT)
               ++ignoreNextMsg;
            broadcastMessage ("EndTurn");
         }

         unsigned int nextPlayer (findNextPlayer (player));
         if (nextPlayer == findNextPlayer (nextPlayer)) {
            endGame (nextPlayer);
            return -1;
         }
         displayTurn (player = nextPlayer);
         dealCard (player);
      }
      else {
         Check3 (pos1Play <= pos2Play);
         Check3 (pos2Play <= hands[player].size ());
         if (pos2Play < hands[player].size ())
            flipCards2Play (hands[player], pos1Play, pos2Play);
      }
   }
   else {
      TRACE4 ("Machiavelli::makeMove (unsigned int) - Moving cards to pile " << target);
      unsigned int pos (target & 0xffff);
      target >>= 16;
      TRACE8 ("Machiavelli::makeMove (unsigned int) - Pile " << target
              << "; Size: " << tablePiles.size ());
      Check3 (target < tablePiles.size ());

      // Check if cards have to be moved from the table
      while (posPiles.size ()) {
         unsigned int posPile (posPiles.back ());
         unsigned int pile ((posPile >> 8) & 0xff);
         unsigned int nr (posPile >> 16);
         unsigned int posSrc (posPile & 0xff);
         TRACE9 ("Machiavelli::makeMove (unsigned int) - Add from " << pile
                 << " cards " << posSrc << '-' << (posSrc + nr - 1));

         Check3 (pile < tablePiles.size ());
         Check3 ((posSrc + nr) <= tablePiles[pile]->size ());

         while (nr--) {
            CardWidget& card (tablePiles[pile]->remove (posSrc));
            card.unmark ();
            tablePiles[target]->insert (card, pos++);
         }
         if (tablePiles[pile]->empty ())
            removePile (pile);

         posPiles.pop_back ();
      }

      if (pos2Play < hands[player].size ()) {
         pos = tablePiles[target]->getPosition4Card (*hands[player][pos1Play]);

         // getPosition4Card can't handle a coloured pair with a gap
         if (pos == -1U) {
            Check3 (((tablePiles[target]->size () == 2)
                     && (MachiPile::cardDistance (*(*tablePiles[target])[1],
                                                 *(*tablePiles[target])[0]) == 2))
                    || (tablePiles[target]->size () == 1));
            pos = (tablePiles[target]->size () == 1) ? 0 : 1;
         }
         Check3 (pos != -1U);
      }

      Check1 (pos1Play <= pos2Play);

      if (pos2Play < hands[player].size ()) {
#if CHECK > 2
         for (unsigned int t (pos1Play); t < pos2Play; ++t) {
            int diff (MachiPile::cardDistance (*hands[player][t + 1], *hands[player][t],
                                               (t == pos1Play) ? MachiPile::BOTH : MachiPile::ACE));
            TRACE1 ("Buraco::makeMove (unsigned int) - Card " << *hands[player][t]);
            Check3 ((diff == 0) || (diff == 1));
         }
#endif

         // Move the played cards to the pile to play
         Check3 (hands[player].size () > pos2Play);
         for (; (int)pos1Play <= (int)pos2Play; --pos2Play)
            tablePiles[target]->insert (hands[player].remove (pos1Play), pos++);
      }
      target = -1U;

#if CHECK > 2
      if (typeid (*actPlayers[player]) == typeid (ComputerPlayer)) {
         YGP::StatusObject obj;
         checkPiles (obj);
         if (obj.getType () != YGP::StatusObject::UNDEFINED) {
            TRACE ("Machiavelli::makeMove (unsigned int) - Invalid piles!\n"
                   << obj.getMessage ());
            Check (!"Valid piles");
         }
      }
#endif
   }

   return player;
}

//-----------------------------------------------------------------------------
/// Enables the cards the human can pick up.
/// \returns \c 0
//-----------------------------------------------------------------------------
bool Machiavelli::enableHuman () {
   TRACE4 ("Machiavelli::enableHuman ()");
   Check3 (staple.size ());
   Check3 (activeCards.empty ());

   if (staple.size ())
       activeCards.push_back (staple.getTopCard ().signal_clicked ().connect
                              (mem_fun (*this, (&Machiavelli::endTurn))));
   nextTurn.set_sensitive (true);

   for (unsigned int i (0); i < hands[0].size (); ++i)
      registerHandDND (i);
   Check3 (aDNDHand.size () == hands[0].size ());

   newPile.drag_dest_set (dndTypeBoth, Gtk::DEST_DEFAULT_ALL, Gdk::ACTION_MOVE);
   aDNDTable[NULL].connReceive = newPile.signal_drag_data_received ().connect
       (bind (mem_fun (*this, &Machiavelli::cardDroppedOnTable), -1U));

   for (unsigned int i (0); i < tablePiles.size (); ++i) {
      MachiPile& pile (*tablePiles[i]);
      unsigned int value (i << 8);
      for (unsigned int j (0); j < pile.size (); ++j)
         registerTableDND (*pile[j], value++);
   }

   return Game::enableHuman ();
}

//-----------------------------------------------------------------------------
/// Disables the cards the human player can select
//-----------------------------------------------------------------------------
void Machiavelli::disableHuman () {
   TRACE2 ("Machiavelli::disableHuman () - DND: " << aDNDHand.size () << "; "
           << aDNDTable.size ());
   Game::disableHuman ();

   newPile.drag_dest_unset ();

   if (aDNDHand.size ())
      for (unsigned int i (0); i < hands[0].size (); ++i)
         unregisterHandDND (*hands[0][i]);
   Check3 (aDNDHand.empty ());

   unregisterTableDND ();
   nextTurn.set_sensitive (false);
}

//----------------------------------------------------------------------------
/// Changes the names of the playing people
/// \param newPlayer: Array holding the new player
//----------------------------------------------------------------------------
void Machiavelli::changeNames (const std::vector<Player*>& newPlayer) {
   Game::changeNames (newPlayer);

   for (unsigned int i (0); i < NUM_PLAYERS; ++i) {
      TRACE1 ("Machiavelli::changeNames () " << i << ": " << newPlayer[i]->getName ());
      names[i].set_text (newPlayer[i]->getName ());
   }
}

//----------------------------------------------------------------------------
/// Sets the startplayer; including showing it in the status bar
/// \param player: Player to start the game
//----------------------------------------------------------------------------
void Machiavelli::setStartPlayer () {
   if (getConnectionMgr ().getMode () != YGP::ConnectionMgr::CLIENT) {
      setNextPlayer (startPlayer);

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
   }

   dealCard (startPlayer);
   displayTurn (startPlayer++);
   startPlayer &= 0x3;
   makeNextMoves ();
}

//-----------------------------------------------------------------------------
/// Callback to end a turn
//-----------------------------------------------------------------------------
void Machiavelli::endTurn () {
   TRACE5 ("Machiavelli::endTurn ()");
   Check1 (gameStatus () == PLAYING);
   Check3 (staple.size ()); Check3 (activeCards.size ());

   // Show error, if any
   YGP::StatusObject obj;
   checkPiles (obj);
   if (obj.getType () != YGP::StatusObject::UNDEFINED) {
      obj.generalize (_("Can't end turn: The piles are not valid!"));
      undoDlg = XGP::MessageDlg::create (obj);
      undoDlg->set_title (PACKAGE);
      undoDlg->get_window ()->set_transient_for (this->get_window ());

      // Add undo-buttons
      Gtk::Button* undoAll (manage (new Gtk::Button (_("_Undo all"), true)));
      Gtk::Button* undoLast (manage (new Gtk::Button (_("Undo _last"), true)));

      undoAll->show ();
      undoLast->show ();
      undoDlg->get_action_area ()->pack_end (*undoAll, Gtk::PACK_SHRINK, 5);
      undoDlg->get_action_area ()->pack_end (*undoLast, Gtk::PACK_SHRINK, 5);

      undoAll->signal_clicked ().connect
          (bind (mem_fun (*this, &Machiavelli::undoMove), -1U));
      undoLast->signal_clicked ().connect
          (bind (mem_fun (*this, &Machiavelli::undoMove), 1));
      return;
   }

   while (undo.size ())
      undo.pop ();

   if (getConnectionMgr ().getMode () != YGP::ConnectionMgr::NONE) {
      // Send played card to all clients (if any)
      if (getConnectionMgr ().getMode () == YGP::ConnectionMgr::CLIENT)
         ++ignoreNextMsg;
      broadcastMessage ("EndTurn");
   }

   disableHuman ();

   unsigned int nextPlayer (findNextPlayer (currentPlayer ()));
   setNextPlayer (nextPlayer);
   if (nextPlayer == findNextPlayer (nextPlayer))
      endGame (nextPlayer);
   else {
      displayTurn (nextPlayer);
      dealCard (nextPlayer);
      makeNextMoves ();
   }
}

//-----------------------------------------------------------------------------
/// Prepares the passed region of cards for drag´n´drop
/// \param start: Number of first card to prepare for DND
/// \param end: Number of last card to prepare for DND
/// \pre \c start < \c end; \c end <= Nr. ofcards
//-----------------------------------------------------------------------------
void Machiavelli::registerHandDND (unsigned int start, unsigned int end) {
   TRACE5 ("Machiavelli::registerHandDND (unsigned int, unsigned int) - [" << start
           << '-' << end << ']');
   Check1 (start <= end);
   Check1 (end < hands[0].size ());

   for (; start <= end; ++start) {
      unregisterHandDND (*hands[0][start]);
      registerHandDND (start);
   }
}

//-----------------------------------------------------------------------------
/// Prepares the card for drag´n´drop
/// \param iCard: Number of card in hand
//-----------------------------------------------------------------------------
void Machiavelli::registerHandDND (unsigned int iCard) {
   Check1 (iCard < hands[0].size ());
   TRACE9 ("Machiavelli::registerHandDND (unsigned int) - Card: " << iCard << " ("
           << *hands[0][iCard] << ')');

   CardWidget& card (*hands[0][iCard]);
   Check3 (aDNDHand.find (&card) == aDNDHand.end ());

   // Card accepts drops from hand and drags from table
   card.drag_dest_set (dndTypeHand, Gtk::DEST_DEFAULT_ALL, Gdk::ACTION_MOVE);
   card.drag_source_set
      (dndTypeHand, Gdk::ModifierType (GDK_BUTTON2_MASK | GDK_BUTTON3_MASK),
       Gdk::ACTION_MOVE);

   card.drag_source_set_icon (card.getImage ());
   aDNDHand[&card].connReceive = card.signal_drag_data_received ().connect
      (bind (mem_fun (*this, &Machiavelli::cardDropped), iCard));
   aDNDHand[&card].connGet = card.signal_drag_data_get ().connect
      (bind (mem_fun (*this, &Machiavelli::getDropData), iCard));
}

//-----------------------------------------------------------------------------
/// Stops the drag´n´drop abilities of the passed card
/// \param card: Card to unregister of dnd
//-----------------------------------------------------------------------------
void Machiavelli::unregisterHandDND (CardWidget& card) {
   TRACE9 ("Machiavelli::unregisterHandDND (CardWidget&) - Card: " << card);
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
/// \param pile: Pile whose cards should be registered. This value is calcualated
///     like (row << 4) + column
/// \param start: Number of first card to prepare for DND
/// \param end: Number of last card to prepare for DND
/// \pre: \c start < \c end; \c end <= Number of cards
//-----------------------------------------------------------------------------
void Machiavelli::registerTableDND (unsigned int pile, unsigned int start, unsigned int end) {
   TRACE9 ("Machiavelli::registerTableDND (unsigned int, unsigned int, unsigned int)"
           << " - " << pile << '[' << start << '-' << end << ']');
   Check1 (pile < tablePiles.size ());
   Check1 (start <= end);
   Check1 (end < tablePiles[pile]->size ());

   ICardPile& tmp (*tablePiles[pile]);

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
void Machiavelli::registerTableDND (CardWidget& card, unsigned int nr) {
   TRACE9 ("Machiavelli::registerTableDND (CardWidget&, unsigned int) - " << card
           << " = " << std::hex << nr << std::dec);

   // Card accepts drops from hand and drags from table
   card.drag_dest_set (dndTypeBoth, Gtk::DEST_DEFAULT_ALL, Gdk::ACTION_MOVE);
   card.drag_source_set
       (dndTypeTable, Gdk::ModifierType (GDK_BUTTON2_MASK | GDK_BUTTON3_MASK),
        Gdk::ACTION_MOVE);
   card.drag_source_set_icon (card.getImage ());

   aDNDTable[&card].connReceive = card.signal_drag_data_received ().connect
      (bind (mem_fun (*this, &Machiavelli::cardDroppedOnTable), nr));
   aDNDTable[&card].connGet = card.signal_drag_data_get ().connect
      (bind (mem_fun (*this, &Machiavelli::getDropData), nr));
}

//-----------------------------------------------------------------------------
/// Stops the drag´n´drop abilities of the passed card
/// \param card: Card to de-register
//-----------------------------------------------------------------------------
void Machiavelli::unregisterTableDND (CardWidget& card) {
   TRACE9 ("Machiavelli::unregisterTableDND (unsigned int) - Card: " << card
           << " - " << &card );
   Check1 (aDNDTable.size () > 1);

   std::map<CardWidget*, CONNECTIONS>::iterator i (aDNDTable.find (&card));
   Check1 (i != aDNDTable.end ());

   card.drag_dest_unset ();
   i->second.connGet.disconnect ();
   i->second.connReceive.disconnect ();
   aDNDTable.erase (i);
}

//-----------------------------------------------------------------------------
/// Stops the drag´n´drop abilities of all cards on the table
//-----------------------------------------------------------------------------
void Machiavelli::unregisterTableDND () {
   for (std::map<CardWidget*, CONNECTIONS>::iterator i (aDNDTable.begin ());
        i != aDNDTable.end (); ++i) {
      i->second.connGet.disconnect ();
      i->second.connReceive.disconnect ();
   }

   aDNDTable.clear ();
}

//-----------------------------------------------------------------------------
/// Callback after dropping a card (within the hand)
/// \param pContext: Context of the drag (contains things like source,
/// \param target, action, ...)
/// \param data: Describes the thing which was dropped
/// \param time: Timestamp of the drag
/// \param card: Number of card where something was dropped at
/// \pre \c pContext not NULL; Expects \c info to be 0
//-----------------------------------------------------------------------------
void Machiavelli::cardDropped (const Glib::RefPtr<Gdk::DragContext>& context,
                               gint, gint, const Gtk::SelectionData& data, guint,
                               guint32 time, unsigned int card) {
   Check3 (!context->get_is_source ());
   Check3 (data.get_length () == sizeof (int));
   Check3 (data.get_format () == 8);
   Check3 (card < hands[0].size ());

   unsigned int* pValue (reinterpret_cast <unsigned int*>
                         (const_cast<guint8*> (data.get_data ())));
   Check3 (pValue);
   Check3 (*pValue < hands[0].size ());
   TRACE1 ("Machiavelli::cardDropped (...) - Inserting card " << *pValue
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
   registerHandDND (card, *pValue);
   Check3 (aDNDHand.size () == hands[0].size ());
}

//-----------------------------------------------------------------------------
/// Callback to query the data to drop
/// \param pContext: Context of the drag (contains things like source,
/// \param target, action, ...)
/// \param data: Describes the thing which was dropped
/// \param time: Timestamp of the drag
/// \param cardPos: Position of card (either in hand or pile on table)
/// \pre \c pContext not NULL; Expects \c info to be 0
//-----------------------------------------------------------------------------
void Machiavelli::getDropData (const Glib::RefPtr<Gdk::DragContext>& pContext,
                               Gtk::SelectionData& data, guint, guint32 time,
                               unsigned int cardPos) {
   Check1 (pContext->get_is_source ());

   data.set (data.get_target (), 8, reinterpret_cast <guchar*> (&cardPos),
             sizeof (cardPos));
}

//-----------------------------------------------------------------------------
/// Callback after dropping a card on the table
/// \param pContext: Context of the drag (contains things like source,
/// \param target, action, ...)
/// \param data: Describes the thing which was dropped
/// \param info: Describes the type of data (should be 0)
/// \param time: Timestamp of the drag
/// \param iCard: Combination of card and pile on which card was dropped
/// \pre \c pContext not NULL;
//-----------------------------------------------------------------------------
void Machiavelli::cardDroppedOnTable (const Glib::RefPtr<Gdk::DragContext>& context,
                                      gint, gint, const Gtk::SelectionData& data,
                                      guint info, guint32 time, unsigned int iCard) {
   TRACE1 ("Machiavelli::cardDroppedOnTable (...) - Card dropped on " << std::hex
           << (int)iCard << std::dec << "; " << info);
   Check1 (!context->get_is_source ());
   Check1 (data.get_length () == sizeof (int));
   Check1 (data.get_format () == 8);
   Check1 ((info == HAND) || (info == TABLE));

   unsigned int* pValue (reinterpret_cast <unsigned int*>
                         (const_cast<guint8*> (data.get_data ())));
   TRACE1 ("Machiavelli::cardDroppedOnTable (...) - Inserting card " << std::hex
           << *pValue << std::hex << " in pile");
   Check2 (pValue);

   unsigned int nrpile (*pValue >> 8);
   unsigned int off (*pValue & 0xff);
   Check2 ((info == HAND)
           ? (off < hands[0].size ())
           : (nrpile < tablePiles.size () && (off < tablePiles[nrpile]->size ())));

   // Move dropped card to a (new) pile on the table
   unsigned int iPile (0);
   MachiPile* pile (NULL);
   ICardPile& src ((info == HAND) ? hands[0] : *tablePiles[nrpile]);
   CardWidget* moved (src[off]);
   unsigned int nr (((info == HAND)
                     || (tablePiles[nrpile]->getType () == MachiPile::NUMBER))
                    ? 1 : (src.size () - off));
   TRACE4 ("Machiavelli::cardDroppedOnTable (...) - Card dropped: " << *moved);

   if (iCard == -1U) {    // If card was dropped on the new label: Create pile
      iPile = tablePiles.size ();
      pile = &makeNewPile ();
      iCard = 0;
   }
   else {
      // Else check pile to use
      iPile = iCard >> 8;

      // Ignore dnd from a pile to itself
      if ((info == TABLE) && (iPile == nrpile)) {
         context->drag_finish (true, false, time);
         return;
      }

      Check1 (iPile < tablePiles.size ());
      pile = tablePiles[iPile];

      iCard = pile->getPosition4Card (*moved);
      if (iCard == -1U) {
         context->drag_finish (true, false, time);
         Gtk::MessageDialog dlg (_("This card does not fit on that pile!"),
                                 Gtk::MESSAGE_ERROR);
         dlg.set_title (_("Invalid move"));
         dlg.run ();
         return;
      }

      if (info == TABLE) {
         // Move only one card from/to a numbered pile
         if ((pile->getType () == MachiPile::NUMBER)
             || ((pile->getType () == MachiPile::UNDEFINED)
                 ? ((pile->size () == 1)
                    && ((*pile)[0]->number () == moved->number ()))
                 : !iCard)
             || (tablePiles[nrpile]->getType () == MachiPile::NUMBER))
            nr = 1;

         // Check if only cards from an edge are moved to the beginning of
         // a coloured pile or a numbered pile
         if ((tablePiles[nrpile]->getType () == MachiPile::COLOUR)
             && (((off + 1) != src.size ()) && off)
             && ((iCard != pile->size ())
                 || (pile->getType () == MachiPile::NUMBER))) {
            context->drag_finish (true, false, time);
            Gtk::MessageDialog dlg (_("Card is not on the edge of the origen - try splitting the origin first!"),
                                    Gtk::MESSAGE_ERROR);
            dlg.set_title (_("Invalid move"));
            dlg.run ();
            return;
         }
      }
   }
   Check3 (pile);

   // End old drag
   context->drag_finish (true, false, time);

   // Store undo-info 4 Bytes: Target-pile, target-card, source-pile,
   // source-card; if played from hand, set source-pile to 0xff
   undoValue val (iPile, iCard, (info == HAND) ? 0xff : nrpile, off, nr);

   // Send move
   if (getConnectionMgr ().getMode () != YGP::ConnectionMgr::NONE) {
      std::ostringstream msg;
      if (info == HAND)
         msg << "Play=" << src[off]->id ();
      else
         msg << "Reorder=" << ((nr << 16) + (nrpile << 8) + off);

      msg << ";Target=" << (iPile << 16) + iCard;
      if (info ==TABLE)
         msg << ";Now=1";

      if (getConnectionMgr ().getMode () == YGP::ConnectionMgr::CLIENT)
          ++ignoreNextMsg;
      broadcastMessage (msg.str ());
   }

   while (nr--) {
      TRACE9 ("Machiavelli::cardDroppedOnTable (...) - Insert to: " << iPile
              << "; Pos: " << iCard);
      Check3 (iCard != -1U);

      // Unregister old card
      src.remove (off);
      (info == HAND) ? unregisterHandDND (*moved) : unregisterTableDND (*moved);

      // Insert card into pile and register it for DND
      Check3 (iCard <= pile->size ());
      pile->insert (*moved, iCard);
      registerTableDND (*moved, (iPile << 8) + iCard);
      if (iCard < (pile->size () - 1))
         registerTableDND (iPile, iCard + 1, pile->size () - 1);

      moved = src[off];
      iCard++;
   }

   if (hands[0].empty ()) {
      YGP::StatusObject obj;
      checkPiles (obj);
      if (obj.getType () == YGP::StatusObject::UNDEFINED) {
         unsigned int nextPlayer (findNextPlayer (0));
         if (nextPlayer == findNextPlayer (nextPlayer)) {
            endGame (nextPlayer);
            disableHuman ();
            return;
         }
      }
   }

   if (info == TABLE) {
      if (src.empty ()) {                             // Pile moved completely?
         std::vector<MachiPile*>::iterator i (tablePiles.begin () + nrpile);
         Check3 (*i == &src);
         tablePiles.erase (i);
         delete &src;
         val.create = true;

         if (nrpile > iPile)
             nrpile = iPile;

         // Re-register the following piles
         while (nrpile < tablePiles.size ()) {
             registerTableDND (nrpile, 0, tablePiles[nrpile]->size () - 1);
             ++nrpile;
         }
      }
      else {
        for (MachiPile::iterator i (tablePiles[nrpile]->begin ());
             i != tablePiles[nrpile]->end (); ++i) {
           unregisterTableDND (**i);
           registerTableDND (**i, (nrpile << 8) + i - tablePiles[nrpile]->begin ());
        }
      }
   }

   // Re-register the cards in the hand of the human for DND
   if ((info ==HAND) && *pValue < hands[0].size ())
      registerHandDND (*pValue, hands[0].size () - 1);
   Check3 (aDNDHand.size () == hands[0].size ());

   undo.push (val);
}

//----------------------------------------------------------------------------
/// Finds the next player still having cards
/// \param player: Player to find next player to
/// \return unsigned int: Next player having cards
/// \remarks We assume (without really checking), that there's a next player.
//----------------------------------------------------------------------------
unsigned int Machiavelli::findNextPlayer (unsigned int player) const {
   for (unsigned int i (0); i < NUM_PLAYERS; ++i) {
      player = (player + 1) & 0x3;
      if (hands[player].size ()) {
         TRACE8 ("Machiavelli::findNextPlayer (unsigned int) const - Player: " << player);
         break;
      }
   }
   Check3 (player < NUM_PLAYERS);
   return player;
}

//-----------------------------------------------------------------------------
/// Makes a new pile.
/// \returns MachiPile&: New created pile
//-----------------------------------------------------------------------------
MachiPile& Machiavelli::makeNewPile () {
   TRACE9 ("Machiavelli::makeNewPile ()");

   MachiPile* pile (new MachiPile ());
   pile->show ();
   piles.add (*pile);
   tablePiles.push_back (pile);
   TRACE9 ("Machiavelli::makeNewPile () - Pile " << tablePiles.size ());
   return *pile;
}

//-----------------------------------------------------------------------------
/// Makes a new pile in a certain position
/// \param pos: Position of pile on the table
/// \returns MachiPile&: New created pile
//-----------------------------------------------------------------------------
MachiPile& Machiavelli::makeNewPile (unsigned int pos) {
   TRACE9 ("Machiavelli::makeNewPile ()");

   MachiPile* pile (new MachiPile ());
   pile->show ();
   Check3 (pos <= tablePiles.size ());
   piles.insert (*pile, pos);
   tablePiles.insert (tablePiles.begin () + pos, pile);
   return *pile;
}

//-----------------------------------------------------------------------------
/// Searches the passed pile, if it has a serie of 3 or more. This method also
/// \param playerPile: Pile to inspect
/// \returns \c bool:
/// \requires The pile must have at least 3 cards
/// \remarks: Sets pos1Play and pos2Play, if a serie is found
//-----------------------------------------------------------------------------
bool Machiavelli::hasSerie (ICardPile& playerPile) {
   Check3 (playerPile.size () >= 3);

   // Check for 3 cards belonging to a serie
   for (unsigned int i (0); i < (playerPile.size () - 2); ++i) {
      TRACE8 ("Machiavelli::hasSerie (ICardPile&) - Analyzing card "
              << *playerPile[i]);

      std::map<unsigned int, unsigned int> aPos;                   // diff, pos
      std::vector<unsigned int> aOrder;
      unsigned int nrs (playerPile.getSeries (*playerPile[i], aPos, aOrder,
                                              &MachiPile::cardDistance, false));

      // Play the bigger of the found matching cards, if there are >= 3
      if ((nrs > aPos.size ()) ? (nrs > 2) : (aPos.size () > 2)) {
         if (nrs <= aPos.size ()) {
            i = playerPile.sortColourSerie (aPos, aOrder);
            nrs = aPos.size ();
         }

         pos1Play = i;
         pos2Play = i + nrs - 1;
         return true;
      }
   }
   return false;
}

//-----------------------------------------------------------------------------
/// Searches for cards to play and shows them in the hand of the actual player
/// \param player: Player to inspect
/// \returns \c ID of the target (32 Bit: Pile << 16 + Position) or -1U;
/// \remarks: - Sets pos1Play and pos2Play approbiatly
///    - Creates a new pile if needed
//-----------------------------------------------------------------------------
unsigned int Machiavelli::cardFitsToPile (const CardWidget& card, unsigned int offset) {
   TRACE8 ("Machiavelli::cardFitsToPile (const CardWidget&, unsigned int) - Adding card "
           << card << '?');

   unsigned int dest (-1U);
   for (std::vector<MachiPile*>::const_iterator m (tablePiles.begin ());
        m != tablePiles.end (); ++m) {
      Check2 ((*m)->getType () != MachiPile::UNDEFINED);

      // Check if the card can be added to an existing pile
      dest = (*m)->getPosition4Card (card);
      if (dest != -1U) {
         pos1Play = pos2Play = offset;
         return ((m - tablePiles.begin ()) << 16) + dest;
      }

      // Or can the card be added by splitting the pile?
      int diff (MachiPile::cardDistance (card, *(**m)[0]));
      if (((*m)->getType () == MachiPile::COLOUR)
          ? ((diff < 0) || (card.colour () != (**m)[0]->colour ()))
          : diff)
         continue;

      int pos ((*m)->size () - static_cast<unsigned int> (diff));
      TRACE8 ("Machiavelli::cardFitsToPile (const CardWidget&unsigned int) - Splitting "
              << (m - tablePiles.begin ()) << " at " << diff << " (" << pos << ")?");

      // A new pile can be made directly (enough cards on both sides)
      if ((pos > 2) && (diff > 1)) {
         // Move cards to remove to hand (to be shown); The card from the
         // hand will be added in the next move
         ++diff;
         pos1Play = pos2Play = offset;
         posPiles.push_back ((((*m)->size () - diff) << 16)
                             + ((m - tablePiles.begin ()) << 8) + diff);
         TRACE9 ("Machiavelli::cardFitsToPile (const CardWidget&unsigned int) - Marked pile: "
                 << std::hex << pos2Play << std::dec);

         do {
            Check3 ((unsigned int)diff < (*m)->size ());
            (**m)[diff]->mark ();
         } while (static_cast<unsigned int> (++diff) < (*m)->size ());
         makeNewPile ();
         return (tablePiles.size () - 1) << 16;
      }
   }
   return -1U;
}

//-----------------------------------------------------------------------------
/// Searches for cards to play and shows them in the hand of the actual player
/// \param player: Player to inspect
/// \returns \c ID of the target (32 Bit: Pile << 16 + Position) or -1U
/// \remarks: Sets pos1Play and pos2Play approbiatly
//-----------------------------------------------------------------------------
unsigned int Machiavelli::showCardsToPlay (unsigned int player) {
   TRACE2 ("Machiavelli::showCardsToPlay (unsigned int) - Player " << player);

   ICardPile& playerPile (hands[player]);
   if ((playerPile.size () > 2) && hasSerie (playerPile)) {
      Check3 ((pos2Play - pos1Play) >= 2);
      makeNewPile ();
      return (tablePiles.size () - 1) << 16;
   }

   unsigned int dest (-1U);
   if (tablePiles.size ()) {
      // Check if any cards fits somewhere/somehow on an existing pile
      for (ICardPile::const_iterator p (playerPile.begin ());
           p != playerPile.end (); ++p) {
         dest = cardFitsToPile (**p, p - playerPile.begin ());
         if (dest != -1U)
            break;
      }

      if (dest == -1U)
         dest = reorderTableToFit (playerPile);
      if ((dest != -1U) && posPiles.size ()) {
         if (getConnectionMgr ().getMode () != YGP::ConnectionMgr::NONE) {
            std::ostringstream msg;
            msg << "Reorder=";

            for (std::deque<unsigned int>::const_iterator i (posPiles.begin ());
                 i != posPiles.end (); ++i)
               msg << *i << ' ';
            msg << ";Target=" << dest;
            if (pos2Play == hands[player].size ())
               msg << ";Now=1";
                  
            if (getConnectionMgr ().getMode () == YGP::ConnectionMgr::CLIENT)
               ++ignoreNextMsg;

            broadcastMessage (msg.str ());
         }
      }
   }
   return dest;
}

//-----------------------------------------------------------------------------
/// Tries to play a card by rearranging the cards on the table
/// \param playerPile: Pile to inspect
/// \returns \c ID of the target (32 Bit: Pile << 16 + Position) or -1U
/// \remarks: Sets pos1Play and pos2Play approbiatly
//-----------------------------------------------------------------------------
unsigned int Machiavelli::reorderTableToFit (ICardPile& playerPile) {
   for (ICardPile::const_iterator p (playerPile.begin ());
        p != playerPile.end (); ++p) {
      ICardPile::const_iterator h (p);
      ICardPile work;

      // First try to make piles with two cards from the hand
      while ((h = playerPile.getFittingCard (**p, h + 1, &MachiPile::cardDistance))
             != playerPile.end ()) {
         int diff (MachiPile::cardDistance (**h, **p));
         if ((diff == 0) && ((*p)->id () == (*h)->id ()))
            continue;

         work.append (**p);
         work.append (**h);
         TRACE9 ("Machiavelli::reorderTableToFit (ICardPile&) - Pair: "
                 << **p << " - " << **h);

         // Try to add from the table
         for (std::vector<MachiPile*>::const_iterator t (tablePiles.begin ());
              t != tablePiles.end (); ++t) {
            if ((*t)->size () == 3)
               continue;
            Check2 ((*t)->getType () != MachiPile::UNDEFINED);

            MachiPile::const_iterator c;
            unsigned int nr;
            if ((*t)->hasMatching3rd (work, c, nr)) {
               Check3 (c != (*t)->end ());

               if (work.size ()) {
                  TRACE9 ("Machiavelli::reorderTableToFit (ICardPile&) - Hand "
                          << (p - playerPile.begin ()) << "; " << h - playerPile.begin ());
                  if (diff < 0) {
                     pos1Play = h - playerPile.begin ();
                     playerPile.move (pos1Play - 1,
                                      p - playerPile.begin ());
                  }
                  else {
                     pos1Play = p - playerPile.begin ();
                     playerPile.move (pos1Play + 1,
                                      h - playerPile.begin ());
                  }
                  pos2Play = pos1Play + 1;
               }
               else
                  pos1Play = pos2Play = playerPile.size ();

               TRACE9 ("Machiavelli::reorderTableToFit (ICardPile&) - Pile "
                       << (t - tablePiles.begin ()) << "; Cards "
                       << (c - (*t)->begin ()) << '-' << (c - (*t)->begin () + nr - 1));
               posPiles.push_back (((t - tablePiles.begin ()) << 8)
                                   + (nr << 16) + c - (*t)->begin ());
               while (nr--)
                  (*c++)->mark ();
               makeNewPile ();

               return (tablePiles.size () - 1) << 16;
            } // endif pile has matching card
         } // end-for all table piles
         work.clear ();
      } // end-while card has a fitting one
   }

   return reorderTableToFit2 (playerPile);
}

//-----------------------------------------------------------------------------
/// Tries to play a card by moving one card from on pile on the table to
/// another one, so that a card from the hand also fits.
/// \param playerPile: Pile to inspect
/// \returns \c ID of the target (32 Bit: Pile << 16 + Position) or -1U
/// \remarks: Sets pos1Play and pos2Play approbiatly
//-----------------------------------------------------------------------------
unsigned int Machiavelli::reorderTableToFit2 (ICardPile& playerPile) {
   for (ICardPile::const_iterator p (playerPile.begin ());
        p != playerPile.end (); ++p) {
      // Try to find a pile, where the card from the hand differs by 2 from
      // any end.
      for (std::vector<MachiPile*>::const_iterator t (tablePiles.begin ());
           t != tablePiles.end (); ++t) {
         Check2 ((*t)->getType () != MachiPile::UNDEFINED);
         if (((*t)->getType () == MachiPile::NUMBER)
             || ((**t)[0]->colour () != (*p)->colour ()))
            continue;

         int diff (MachiPile::cardDistance (**p, *((**t)[0]), MachiPile::ONE));
         if ((diff == -2) || (diff - (*t)->size () == 1)) {
            TRACE8 ("Machiavelli::reorderTableToFit2 (ICardPile&) - With move: "
                    << **p << "; Diff: " << diff);

            for (std::vector<MachiPile*>::const_iterator o (tablePiles.begin ());
                 o != tablePiles.end (); ++o) {
               Check2 ((*o)->getType () != MachiPile::UNDEFINED);
               if (((o == t) || ((*o)->size () < 4))
                   || (((*o)->getType () == MachiPile::COLOUR)
                       && ((**o)[0]->colour () != (*p)->colour ())))
                  continue;

               int diffTable (MachiPile::cardDistance (**p, *((**o)[0]),
                                                       MachiPile::ONE));
               MachiPile::const_iterator c ((*o)->end ());
               if ((*o)->getType () == MachiPile::NUMBER) {
                  if (diffTable == ((diff == -2) ? -1 : 1)) {
                     for (c = (*o)->begin (); c != (*o)->end (); ++c)
                        if ((*c)->colour () == (*p)->colour ())
                           break;
                     Check3 (c != (*o)->end ());
                  }
               }
               else {
                  if (diff == -2) {
                     if ((diffTable + 2) == (int)(*o)->size ())
                        c = (*o)->end () - 1;
                  }
                  else
                     if ((diffTable == 1))
                        c = (*o)->begin ();
               }

               // Move card, if one has been found
               if (c != (*o)->end ()) {
                  pos1Play = pos2Play = p - playerPile.begin ();
                  TRACE9 ("Machiavelli::reorderTableToFit2 (ICardPile&) - Hand: "
                          << **p << " (" << pos1Play << ')');
                  TRACE9 ("Machiavelli::reorderTableToFit2 (ICardPile&) - Pile: "
                          << (o - tablePiles.begin ()) << "; Card " << **c
                          << " (" << (c - (*o)->begin ()) << ')');

                  posPiles.push_back (((o - tablePiles.begin ()) << 8)
                                      + (1 << 16) + c - (*o)->begin ());
                  (*c)->mark ();
                  return (((t - tablePiles.begin ()) << 16)
                          + ((diff == -2) ? 0 : (*t)->size ()));

               }
            }
         }
      }
   }

   return reorderTableToFit3 (playerPile);
}

//-----------------------------------------------------------------------------
/// Tries to play a card by rearranging two piles on the table
/// \param playerPile: Pile to inspect
/// \returns \c ID of the target (32 Bit: Pile << 16 + Position) or -1U
/// \remarks: Sets pos1Play and pos2Play approbiatly
//-----------------------------------------------------------------------------
unsigned int Machiavelli::reorderTableToFit3 (ICardPile& playerPile) {
   for (ICardPile::const_iterator p (playerPile.begin ());
        p != playerPile.end (); ++p) {
      ICardPile::const_iterator h (p);
      ICardPile work;

      // Try to find two cards from the table (from different piles)
      for (std::vector<MachiPile*>::const_iterator t (tablePiles.begin ());
           t != tablePiles.end (); ++t) {
         Check2 ((*t)->getType () != MachiPile::UNDEFINED);
         if ((*t)->size () == 3)
            continue;
         TRACE8 ("Machiavelli::reorderTableToFit3 (ICardPile&) - Single: " << **p);

         for (ICardPile::const_iterator i ((*t)->begin ());
              (i = (*t)->getFittingCard (**p, i, &MachiPile::cardDistance))
                 != (*t)->end (); ++i) {
            if (((*p)->number () == (*i)->number ())
                &&((*p)->colour () == (*i)->colour ()))
               continue;

            int diff (i - (*t)->begin ());
            if (diff && (diff != (int)((*t)->size () - 1))
                && ((diff < 2) || (diff > (int)((*t)->size () - 4))))
               continue;

            diff = MachiPile::cardDistance (**p, **i);
            TRACE8 ("Machiavelli::reorderTableToFit3 (ICardPile&) - Matching: " << **i
                    << "; Diff: " << diff);
            work.append (**p);
            work.append (**i);

            for (std::vector<MachiPile*>::const_iterator o (t + 1);
                 o != tablePiles.end (); ++o) {
               if ((*o)->size () < 4)
                  continue;

               MachiPile::const_iterator c;
               unsigned int nr;
               if ((*o)->hasMatching3rd (work, c, nr)) {
                  Check3 (c != (*o)->end ());

                  // Does the first pile need to be split up?
                  if ((i != (*t)->begin ()) && (i != ((*t)->end () - 1))) {
                     TRACE9 ("Machiavelli::reorderTableToFit3 (ICardPile&) - Splitting at "
                             << (i - (*t)->begin ()));
                     work.clear ();
                     o = t;
                     c = i + 1;
                     nr = (*o)->end () - c;
                  }

                  if (work.size ()) {
                     TRACE9 ("Machiavelli::reorderTableToFit3 (ICardPile&) - Hand "
                             << (p - playerPile.begin ()));
                     pos2Play = pos1Play = p - playerPile.begin ();

                     if ((*i)->number () < (*c)->number ()) {
                        TRACE9 ("Machiavelli::reorderTableToFit3 (ICardPile&) - Swapping "
                                << **i << " with " << **c);
                        std::swap (i, c);
                        std::swap (t, o);
                     }

                     Check3 (t >= tablePiles.begin ());
                     Check3 (i >= (*t)->begin ());
                     TRACE9 ("Machiavelli::reorderTableToFit3 (ICardPile&) - Pile "
                             << (t - tablePiles.begin ()) << "; Card "
                             << (i - (*t)->begin ()) << '-' << (i - (*t)->begin () + nr - 1));
                     posPiles.push_back (((t - tablePiles.begin ()) << 8)
                                         + (nr << 16) + i - (*t)->begin ());

                     (*i)->mark ();
                  }
                  else
                     pos1Play = pos2Play = playerPile.size ();

                  Check3 (o >= tablePiles.begin ());
                  Check3 (c >= (*o)->begin ());
                  TRACE9 ("Machiavelli::reorderTableToFit3 (ICardPile&) - Pile "
                          << (o - tablePiles.begin ()) << "; Card "
                          << (c - (*o)->begin ()) << '-' << (c - (*o)->begin () + nr - 1));
                  posPiles.push_back (((o - tablePiles.begin ()) << 8)
                                      + (nr << 16) + c - (*o)->begin ());
                  while (nr--)
                     (*c++)->mark ();
                  makeNewPile ();
                  return (tablePiles.size () - 1) << 16;
               } // endif pile has matching card
            } // endfor all following piles

            work.clear ();
         } // end-for all matching cards in the pile
      } // end-for all table piles
   } // end-for all cards

   return (playerPile.size () < 4) ? reorderTableToFit4 (playerPile) : -1U;
}

//-----------------------------------------------------------------------------
/// Tries to play a card by recursively reordering the whole table
/// \param playerPile: Pile to inspect
/// \returns \c ID of the target (32 Bit: Pile << 16 + Position) or -1U
/// \remarks: Sets pos1Play and pos2Play approbiatly
//-----------------------------------------------------------------------------
unsigned int Machiavelli::reorderTableToFit4 (ICardPile& playerPile) {
   // Inspect every card
   for (ICardPile::const_iterator c (playerPile.begin ());
        c != playerPile.end (); ++c) {
      
      for (std::vector<MachiPile*>::const_iterator t (tablePiles.begin ());
           t != tablePiles.end (); ++t) {
         Check2 ((*t)->getType () != MachiPile::UNDEFINED);
         
         // TODO
      }

   }
   
   return -1U;
}

//----------------------------------------------------------------------------
/// Deals a card to the passed player
/// \param player: Player to give a card to 
//----------------------------------------------------------------------------
void Machiavelli::dealCard (unsigned int player) {
   if (staple.size () == 1) {
      Gtk::MessageDialog dlg (_("Taking last card! Solve the game (somehow) ..."),
                              Gtk::MESSAGE_ERROR);
      dlg.set_title (_("Game over"));
      dlg.run ();
   }

   if (staple.size ()) {
      CardWidget& card (staple.removeTopCard ());
      if (player)
         hands[player].insertSorted (card);
      else
         hands[0].append (card);
   }
}

//----------------------------------------------------------------------------
/// Checks, if all the piles on the table are valid
//----------------------------------------------------------------------------
void Machiavelli::checkPiles (YGP::StatusObject& obj) const {
   for (std::vector<MachiPile*>::const_iterator i (tablePiles.begin ());
        i != tablePiles.end (); ++i) {
       try {
           (*i)->checkIntegrity ();
       }
       catch (Glib::ustring& error) {
           TRACE9 ("Machiavelli::checkPiles () const - " << (i - tablePiles.begin ())
                   << ": " << error);
           Glib::ustring msg (_("Pile %1: %2\n"));
           msg.replace (msg.find ("%1"), 2,
                        YGP::ANumeric::toString (i - tablePiles.begin () + 1));
           msg.replace (msg.find ("%2"), 2, error);
           obj.setMessage (YGP::StatusObject::ERROR, msg);
       }
   }
}

//----------------------------------------------------------------------------
/// Undoes the passed number of moves (starting from the last)
/// \param number: Number of moves to undo 
//----------------------------------------------------------------------------
void Machiavelli::undoMove (unsigned int number) {
   TRACE3 ("Machiavelli::undoMove (unsigned int) - Undo " << number);
   Check2 (number);
   Check2 (undo.size ());

   if (number > undo.size ())
      number = undo.size ();

   disableHuman ();

   while (number--) {
      undoValue move (undo.top ());
      undo.pop ();

      if (move.create)
         makeNewPile (move.srcPile);

      TRACE9 ("Machiavelli::undoMove (unsigned int) - Undo " << move.number
              << "; " << move.destPile << '/' << move.destPos << "-> "
              << move.srcPile << '/' << move.srcPos);
      ICardPile& dest ((move.srcPile == 0xff)
                       ? hands[currentPlayer ()] : *tablePiles[move.srcPile]);
      Check3 (move.srcPos <= dest.size ());

      Check3 (move.destPile < tablePiles.size ());
      MachiPile& src (*tablePiles[move.destPile]);
      Check3 (move.destPos < src.size ());
      Check3 (move.number);
      Check3 ((move.number + move.destPos) <= src.size ());

      // Inform clients about cards to play
      if (getConnectionMgr ().getMode () != YGP::ConnectionMgr::NONE) {
         if (getConnectionMgr ().getMode () == YGP::ConnectionMgr::CLIENT)
            ++ignoreNextMsg;

         std::ostringstream msg;
         msg << "Move=" << move.destPile << ";From=" << move.destPos << ";To="
             << (move.destPos + move.number - 1) << ";Target=" << move.srcPile
             << ";At=" << move.srcPos;
         if (move.create)
            msg << ";Create=1";
         broadcastMessage (msg.str ());
      }

      do {
         dest.insert (src.remove (move.destPos), move.srcPos++);
      } while (--move.number);

      if (src.empty ())
         removePile (move.destPile);
   }

   enableHuman ();

   Check3 (undoDlg);
   if (undo.empty ()) {
      delete undoDlg;
      undoDlg = NULL;
   }
   else {
      YGP::StatusObject obj;
      checkPiles (obj);
      if (obj.getType () != YGP::StatusObject::UNDEFINED)
         obj.generalize (_("Can't end turn: The piles are not valid!"));
      else
         obj.setMessage (YGP::StatusObject::INFO, _("Could end turn: The piles are OK!"));
      undoDlg->update (obj);
   }
}

//----------------------------------------------------------------------------
/// Removes the passed pile from the table and internally
/// \param pile: Offset of pile to remove 
//----------------------------------------------------------------------------
void Machiavelli::removePile (unsigned int pile) {
   TRACE9 ("Machiavelli::removePile (unsigned int) - " << pile);
   Check1 (pile < tablePiles.size ());

   MachiPile& tmp (*tablePiles[pile]);
   Check3 (tmp.empty ());

   tablePiles.erase (tablePiles.begin () + pile);
   piles.remove (tmp);
   delete &tmp;
}

//----------------------------------------------------------------------------
/// Ends the game
/// \param looser: Number of player having lost the game
//----------------------------------------------------------------------------
void Machiavelli::endGame (unsigned int looser) {
   Check1 (looser < NUM_PLAYERS);

   status.pop ();
   Glib::ustring stat (_("%1 lost"));
   stat.replace (stat.find ("%1"), 2, actPlayers[looser]->getName ());
   status.push (stat);
   setGameStatus (STOPPED);
}

//----------------------------------------------------------------------------
/// Converts the pile-number to the actual pile
/// \param player: Number of player
/// \param pile: ID of the pile to return
/// \returns ICardPile*: Pile corresponding to the passed number or NULL
//----------------------------------------------------------------------------
ICardPile* Machiavelli::getPileOfPlayer (unsigned int player, unsigned int pile) {
   if ((player >= NUM_PLAYERS) || ((pile >> 16) > tablePiles.size ()))
      return NULL;

   target = pile;
   pile >>= 16;
   if (pile == tablePiles.size ())
      makeNewPile ();

   return &hands[player];
}

//----------------------------------------------------------------------------
/// Handles the messages the server might send for the twopart cardgame
/// \param player: ID of the player sending the message
/// \param message: Message received from the server
/// \returns bool: True, if message has been processed completey
/// \throw std::string: In case of an error an describing text
//----------------------------------------------------------------------------
bool Machiavelli::handleMessage (unsigned int player, const std::string& message) throw (std::string) {
   TRACE1 ("Machiavelli::handleMessage (unsigned int player, const std::string&) - "
           << message << " (" << player << ')');
    
   YGP::Tokenize command (message);
   std::string cmd (command.getNextNode ('='));

   bool rc (true);
   if (cmd == "EndTurn") {
      if (getConnectionMgr ().getMode () == YGP::ConnectionMgr::SERVER)
         broadcastMessage (message);

      unsigned int nextPlayer (findNextPlayer (currentPlayer ()));
      if (nextPlayer == findNextPlayer (nextPlayer))
         endGame (nextPlayer);
      else {
         displayTurn (player = nextPlayer);
         dealCard (nextPlayer);
         setNextPlayer (nextPlayer);
         makeNextMoves ();

#if CHECK > 2
         YGP::StatusObject obj;
         checkPiles (obj);
         if (obj.getType () != YGP::StatusObject::UNDEFINED) {
            TRACE ("Machiavelli::handleMessage (unsigned int, const std::string&)"
                   " - Invalid piles!\n" << obj.getMessage ());
            Check (!"Valid piles");
         }
#endif

      }
   }
   else if (cmd == "Reorder") {
      Check2 (posPiles.empty ());

      YGP::Tokenize tokCards (command.getNextNode (';'));
      unsigned long posPile (0);
      while (tokCards.getNextNode (' ').size ()) {
         if (stringToNumber (posPile, tokCards.getActNode ().c_str ())) {
            std::string error ("Not a card number: `%1'");
            error.replace (error.find ("%1"), 2, tokCards.getActNode ());
            throw error;
         }

         unsigned int pile ((posPile >> 8) & 0xff);
         unsigned int nr (posPile >> 16);
         unsigned int posSrc (posPile & 0xff);
         TRACE9 ("Machiavelli::handleMessage (unsigned int, const std::string&)"
                 " - Add from " << pile << " cards " << posSrc << '-' << (posSrc + nr - 1));

         if (pile >= tablePiles.size ())
            throw std::string ("Invalid source pile");
         ICardPile& srcPile (*tablePiles[pile]);
         if ((posSrc + nr) > srcPile.size ())
            throw std::string ("Invalid cards");

         posPiles.push_back (posPile);

         while (nr--)
            srcPile[posSrc++]->mark ();
      }

      YGP::AttributeParse ap;
      unsigned int now (0);
      ATTRIBUTE (ap, unsigned int, target, "Target");
      ATTRIBUTE (ap, unsigned int, now, "Now");
      ap.assignValues (command.getNextNode ('\0'));

      pos1Play = pos2Play = hands[currentPlayer ()].size ();
      unsigned int targetPile (target >> 16);
      if (targetPile > tablePiles.size ()) {
         target = -1U;
         throw std::string ("Invalid target pile");
      }

      if (targetPile == tablePiles.size ())
         makeNewPile ();

      // Inform clients about cards to play
      if (getConnectionMgr ().getMode () == YGP::ConnectionMgr::SERVER)
          broadcastMessage (message);

      if (now)
         Glib::signal_timeout ().connect
            (bind (mem_fun (*this, &Machiavelli::endRemoteMove),
                   currentPlayer ()), ComputerPlayer::TIMEOUT);
      return !now;
   }
   else if (cmd == "Move") {
      YGP::AttributeParse ap;
      unsigned int card1 (-1U), card2 (-1U), dest (-1U), src (-1U),
         destPos (0), create (0);
      ATTRIBUTE (ap, unsigned int, src, "Move");
      ATTRIBUTE (ap, unsigned int, card1, "From");
      ATTRIBUTE (ap, unsigned int, card2, "To");
      ATTRIBUTE (ap, unsigned int, dest, "Target");
      ATTRIBUTE (ap, unsigned int, destPos, "At");
      ATTRIBUTE (ap, unsigned int, create, "Create");
      ap.assignValues (message);

      if ((dest >= tablePiles.size ()) && (dest != 255))
         throw std::string ("Invalid destination pile!");
      if (create) {
         Check3 (dest != 255);
         makeNewPile (dest);
      }
      ICardPile& pile ((dest == 255) ? hands[player] : *tablePiles[dest]);
      ICardPile* srcPile (NULL);
      try {
         if (destPos > pile.size ())
            throw std::string ("Invalid position in destination pile!");

         if (src >= tablePiles.size ())
            throw std::string ("Invalid source pile!");
          srcPile = tablePiles[src];
         if ((card2 < card1) || (card2 >= srcPile->size ()))
            throw std::string ("Invalid cards!");
      }
      catch (std::string& error) {
         if (create)
            removePile (dest);
         throw error;
      }

      if (getConnectionMgr ().getMode () == YGP::ConnectionMgr::SERVER)
         broadcastMessage (message);

      Check3 (srcPile);
      do {
         pile.insert (srcPile->remove (card1), destPos++);
      } while (card1 < card2--);

      if (srcPile->empty ())
         removePile (src);
   }
   else {
      rc = Game::handleMessage (player, message);
      if (cmd == "ActPlayer") {
         TRACE1 ("Machiavelli::handleMessage (unsigned int player, const std::string&) - "
                 "Next player: " << currentPlayer ());

         startPlayer = currentPlayer ();
         setStartPlayer ();
      }
   }
   return rc;
}

//----------------------------------------------------------------------------
/// Returns the actual target, where flipCard2Play should position the cards to
/// \returns unsigned int: ID of the target
//----------------------------------------------------------------------------
unsigned int Machiavelli::getActTarget () const {
   Check3 ((target >> 16) < tablePiles.size ());
   return target;
}
