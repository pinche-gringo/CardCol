//$Id$

//PROJECT     : Cardgames
//SUBSYSTEM   : Machiavelli
//REFERENCES  :
//TODO        : Rewrite reorderTableToFit() to only iterate once over the piles
//BUGS        :
//REVISION    : $Revision$
//AUTHOR      : Markus Schwab
//CREATED     : 05.11.2003
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

#include <sstream>

#include <gtk/gtkdnd.h>

#include <gtkmm/stock.h>
#include <gtkmm/statusbar.h>

#include <YGP/Check.h>
#include <YGP/Trace.h>
#include <YGP/ConnMgr.h>
#include <YGP/ANumeric.h>
#include <YGP/AttrParse.h>
#include <YGP/StatusObj.h>

#include <XGP/MessageDlg.h>

#include <card/Set.h>
#include <card/Player.h>
#include <card/Images.h>
#include <card/Window.h>
#include <card/Widget.h>
#include <card/ComputerPlayer.h>

#include "Machiavelli.h"


// Some Windows-header seems to define ERROR
#ifdef ERROR
#  undef ERROR
#endif


enum { HAND, TABLE };
static std::vector<Gtk::TargetEntry> dndTypeHand;
static std::vector<Gtk::TargetEntry> dndTypeTable;
static std::vector<Gtk::TargetEntry> dndTypeBoth;


//-----------------------------------------------------------------------------
/// Constructor
/// \param parent Parent widget to display the game in
/// \param statusbar Status bar widget to display information about the game
/// \param cardset Cardset to use
/// \param player Vector of player
/// \param posPlayer Position of player for the server
/// \param mxSerialize Mutex to serialize messages from the server
//-----------------------------------------------------------------------------
Machiavelli::Machiavelli (Gtk::Box& parent, Gtk::Statusbar& statusbar,
			  Card::Set& cardset, const std::vector<Card::Player*>& player,
			  unsigned int posPlayer, YGP::Mutex& mxSerialize)
   : Game (parent, statusbar, cardset, player, posPlayer, mxSerialize, 3, 10),
     piles (), tablePiles (), startPlayer (-1U), newPile (_("New pile")),
     staple (Card::IPile::TOTALLY_COMPRESSED, Card::IPile::SHOWBACK),
     nextTurn (_("_End turn"), true), aDNDHand (), aDNDTable (), target (-1U),
     undo (), missing (), undoDlg (NULL), undo1 (), undoAll (), nxtTurn (), idMrg () {
   TRACE9 ("Machiavelli::Machiavelli (Box&, Statusbar&, CardSet&, const "
           "std::vector<Glib::ustring>&)");

   TRACE9 ("Machiavelli::Machiavelli (Box&, Statusbar&, CardSet&, const "
           "std::vector<Glib::ustring>&) - Init common staples");
   for (unsigned int i (1); i < NUM_PLAYERS; ++i) {
      attach (hands[i], ((NUM_PLAYERS - i) << 2) - 4, ((NUM_PLAYERS - i) << 2),
	      0, 1, Gtk::EXPAND, Gtk::SHRINK, 5, 5);
      attach (names[i], ((NUM_PLAYERS - i) << 2) - 4, ((NUM_PLAYERS - i) << 2),
	      1, 2, Gtk::EXPAND, Gtk::SHRINK, 0);
      hands[i].setStyle (Card::IPile::QUITE_COMPRESSED);
      hands[i].setShowOption (Card::IPile::SHOWBACK);
   }
   hands[0].setStyle (Card::IPile::COMPRESSED);
   hands[0].setShowOption (Card::IPile::SHOWFACE);

   TRACE9 ("Machiavelli::Machiavelli (Box&, Statusbar&, CardSet&, const "
           "std::vector<Glib::ustring>&) - Attach widgets");
   attach (hands[0], 3, 12, 4, 5, Gtk::EXPAND, Gtk::SHRINK, 1, 5);
   attach (names[0], 3, 12, 5, 6, Gtk::EXPAND, Gtk::SHRINK, 1, 5);
   attach (staple,   0, 1,  4, 5, Gtk::SHRINK, Gtk::SHRINK, 5);
   attach (nextTurn, 0, 1,  5, 6, Gtk::FILL, Gtk::SHRINK, 5);
   attach (newPile,  0, 12, 3, 4, Gtk::EXPAND | Gtk::FILL, Gtk::FILL, 0, 5);
   attach (piles,    0, 12, 2, 3, Gtk::EXPAND | Gtk::FILL,
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

   resizeCards ();
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
   TRACE6 ("Machiavelli::start ()");
   Game::start ();

   pos1Play = pos2Play = 0;
   target = -1U;

   if (randomiseCardsToPile (staple)) {
      for (unsigned int i (0); i < NUM_PLAYERS; ++i)
	 hands[(i - posServer) & 0x3].getCards (staple, staple.size () - 7, staple.size () - 1);

      hands[0].sortByColour ();
      for (unsigned int i (1); i < NUM_PLAYERS; ++i) {
         hands[i].setStyle (Card::IPile::QUITE_COMPRESSED);
         hands[i].setShowOption (Card::IPile::SHOWBACK);
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
   TRACE6 ("Machiavelli::clean ()");
   Game::clean ();

   for (unsigned int i (0); i < NUM_PLAYERS; ++i)
      hands[i].clear ();

   staple.clear ();

   for (std::vector<MachiPile*>::iterator i (tablePiles.begin ());
        i != tablePiles.end (); ++i) {
      (*i)->clear ();
      piles.remove (**i);
   }
   tablePiles.clear ();
}

//-----------------------------------------------------------------------------
/// Shows or hides the cards of the computer player
/// \param open Flag if cards should be shown or hidden
//-----------------------------------------------------------------------------
void Machiavelli::playOpen (bool open) {
    for (unsigned int i (1); i < NUM_PLAYERS; ++i) {
      hands[i].setShowOption (open ? Card::IPile::SHOWFACE : Card::IPile::SHOWBACK);
      hands[i].setStyle (open ? Card::IPile::COMPRESSED : Card::IPile::QUITE_COMPRESSED);
    }
}

//-----------------------------------------------------------------------------
/// Makes the move for the next player.
/// \param player Actual player
/// \remarks This method expects the target pile to play in the target-member
///     and the positions to play in pos1Play and pos2Play
//-----------------------------------------------------------------------------
void Machiavelli::makeMove (unsigned int player) {
   TRACE5 ("Machiavelli::makeMove (unsigned int) - Turn of player " << player
           << "; Target: " << std::hex << (int)target << std::dec);
   Check1 (player); Check1 (player < NUM_PLAYERS);
   Check1 (gameStatus () == PLAYING);

   // No more cards found: Continue with next player
   if (showCardsToPlay (player)) {
      if (getConnectionMgr ().getMode () != YGP::ConnectionMgr::NONE) {
	 // Send end of turn to all clients (if any)
	 if (getConnectionMgr ().getMode () == YGP::ConnectionMgr::CLIENT)
	    ++ignoreNextMsg;
	 broadcastMessage ("EndTurn");
      }

      unsigned int nextPlayer (findNextPlayer (player));
      if (nextPlayer == findNextPlayer (nextPlayer)) {
	 endGame (nextPlayer);
	 return;
      }
      displayTurn (player = nextPlayer);
      dealCard (player);
      setNextPlayer (player);
   }
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

   nxtTurn->set_sensitive ();
   nextTurn.set_sensitive ();

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
   TRACE9 ("Machiavelli::disableHuman () - Remaining cards: " << aDNDHand.size ());
   Check3 (aDNDHand.empty ());

   unregisterTableDND ();
   nextTurn.set_sensitive (false);
   nxtTurn->set_sensitive (false);

   if (undo.empty ()) {
      undo1->set_sensitive (false);
      undoAll->set_sensitive (false);
   }
}

//----------------------------------------------------------------------------
/// Changes the names of the playing people
/// \param newPlayer Array holding the new player
//----------------------------------------------------------------------------
void Machiavelli::changeNames (const std::vector<Card::Player*>& newPlayer) {
   Game::changeNames (newPlayer);

   for (unsigned int i (0); i < NUM_PLAYERS; ++i) {
      TRACE1 ("Machiavelli::changeNames () " << i << ": " << newPlayer[i]->getName ());
      names[i].set_text (newPlayer[i]->getName ());
   }
}

//----------------------------------------------------------------------------
/// Sets the startplayer; including showing it in the status bar
/// \param player Player to start the game
//----------------------------------------------------------------------------
void Machiavelli::setStartPlayer () {
   if (getConnectionMgr ().getMode () != YGP::ConnectionMgr::CLIENT) {
      setNextPlayer (startPlayer);
      broadcastStartPlayer (startPlayer);
   }

   dealCard (startPlayer);
   displayTurn (startPlayer++);
   startPlayer &= 0x3;
}

//-----------------------------------------------------------------------------
/// Callback to end a turn. Checks if the piles are OK
//-----------------------------------------------------------------------------
void Machiavelli::endTurn () {
   TRACE5 ("Machiavelli::endTurn ()");
   Check1 (gameStatus () == PLAYING);
   Check3 (staple.size ()); Check3 (activeCards.size ());

   // Show error, if any
   YGP::StatusObject obj;
   checkPiles (obj, true);
   if (obj.getType () != YGP::StatusObject::UNDEFINED) {
      obj.generalize (_("Can't end turn: The piles are not valid!"));
      undoDlg = new XGP::MessageDlg (obj);
      undoDlg->set_title (PACKAGE);
      undoDlg->get_window ()->set_transient_for (this->get_window ());
      undoDlg->signal_response ().connect (mem_fun (*this, &Machiavelli::removeUndoDlg));

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
   }
   else
      doEndTurn ();
}

//-----------------------------------------------------------------------------
/// Ends a turn without checking if the piles are OK
//-----------------------------------------------------------------------------
void Machiavelli::doEndTurn () {
   TRACE5 ("Machiavelli::doEndTurn ()");
   Check1 (gameStatus () == PLAYING);
   Check3 (staple.size ()); Check3 (activeCards.size ());
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
   }
}

//-----------------------------------------------------------------------------
/// Prepares the passed region of cards for drag'n'drop
/// \param start Number of first card to prepare for DND
/// \param end Number of last card to prepare for DND
/// \pre \c start < \c end; \c end <= Nr. ofcards
//-----------------------------------------------------------------------------
void Machiavelli::registerHandDND (unsigned int start, unsigned int end) {
   TRACE9 ("Machiavelli::registerHandDND (unsigned int, unsigned int) - [" << start
           << '-' << end << ']');
   Check1 (start <= end);
   Check1 (end < hands[0].size ());

   for (; start <= end; ++start) {
      unregisterHandDND (*hands[0][start]);
      registerHandDND (start);
   }
}

//-----------------------------------------------------------------------------
/// Prepares the card for drag'n'drop
/// \param iCard Number of card in hand
//-----------------------------------------------------------------------------
void Machiavelli::registerHandDND (unsigned int iCard) {
   Check1 (iCard < hands[0].size ());
   TRACE9 ("Machiavelli::registerHandDND (unsigned int) - Card: " << iCard << " ("
           << *hands[0][iCard] << ')');

   Card::Widget& card (*hands[0][iCard]);
   Check3 (aDNDHand.find (&card) == aDNDHand.end ());

   // Card accepts drops from hand and drags from table
   card.drag_dest_set (dndTypeHand, Gtk::DEST_DEFAULT_ALL, Gdk::ACTION_MOVE);
   card.drag_source_set
      (dndTypeHand, Gdk::ModifierType (GDK_BUTTON1_MASK | GDK_BUTTON2_MASK | GDK_BUTTON3_MASK),
       Gdk::ACTION_MOVE);

   card.drag_source_set_icon (card.getImage ());
   aDNDHand[&card].connReceive = card.signal_drag_data_received ().connect
      (bind (mem_fun (*this, &Machiavelli::cardDropped), iCard));
   aDNDHand[&card].connGet = card.signal_drag_data_get ().connect
      (bind (mem_fun (*this, &Machiavelli::getDropData), iCard));
}

//-----------------------------------------------------------------------------
/// Stops the drag'n'drop abilities of the passed card
/// \param card Card to unregister of dnd
//-----------------------------------------------------------------------------
void Machiavelli::unregisterHandDND (Card::Widget& card) {
   TRACE9 ("Machiavelli::unregisterHandDND (Card::Widget&) - Card: " << card);
   Check1 (aDNDHand.size ());

   std::map<Card::Widget*, CONNECTIONS>::iterator i (aDNDHand.find (&card));
   Check1 (i != aDNDHand.end ());

   card.drag_dest_unset ();
   card.drag_source_unset ();
   i->second.connReceive.disconnect ();
   i->second.connGet.disconnect ();
   aDNDHand.erase (i);
}

//-----------------------------------------------------------------------------
/// Prepares the passed region of cards for drag'n'drop
/// \param pile Pile whose cards should be registered. This value is calcualated
///     like (row << 4) + column
/// \param start Number of first card to prepare for DND
/// \param end Number of last card to prepare for DND
/// \pre \c start < \c end; \c end <= Number of cards
//-----------------------------------------------------------------------------
void Machiavelli::registerTableDND (unsigned int pile, unsigned int start, unsigned int end) {
   TRACE9 ("Machiavelli::registerTableDND (unsigned int, unsigned int, unsigned int)"
           << " - " << pile << '[' << start << '-' << end << ']');
   Check1 (pile < tablePiles.size ());
   Check1 (start <= end);
   Check1 (end < tablePiles[pile]->size ());

   Card::IPile& tmp (*tablePiles[pile]);

   pile <<= 8;
   for (; start <= end; ++start) {
      Card::Widget& card (*tmp[start]);
      unregisterTableDND (card);
      registerTableDND (card, pile + start);
   }
}

//-----------------------------------------------------------------------------
/// Prepares the card for drag'n'drop
/// \param card Card to register
/// \param nr Number of card in pile
//-----------------------------------------------------------------------------
void Machiavelli::registerTableDND (Card::Widget& card, unsigned int nr) {
   TRACE9 ("Machiavelli::registerTableDND (Card::Widget&, unsigned int) - " << card
           << " = " << std::hex << nr << std::dec);

   // Card accepts drops from hand and drags from table
   card.drag_dest_set (dndTypeBoth, Gtk::DEST_DEFAULT_ALL, Gdk::ACTION_MOVE);
   card.drag_source_set
       (dndTypeTable, Gdk::ModifierType (GDK_BUTTON1_MASK | GDK_BUTTON2_MASK | GDK_BUTTON3_MASK),
        Gdk::ACTION_MOVE);
   card.drag_source_set_icon (card.getImage ());

   aDNDTable[&card].connReceive = card.signal_drag_data_received ().connect
      (bind (mem_fun (*this, &Machiavelli::cardDroppedOnTable), nr));
   aDNDTable[&card].connGet = card.signal_drag_data_get ().connect
      (bind (mem_fun (*this, &Machiavelli::getDropData), nr));
}

//-----------------------------------------------------------------------------
/// Stops the drag'n'drop abilities of the passed card
/// \param card Card to de-register
//-----------------------------------------------------------------------------
void Machiavelli::unregisterTableDND (Card::Widget& card) {
   TRACE9 ("Machiavelli::unregisterTableDND (unsigned int) - Card: " << card
           << " - " << &card );
   Check1 (aDNDTable.size () > 1);

   std::map<Card::Widget*, CONNECTIONS>::iterator i (aDNDTable.find (&card));
   Check1 (i != aDNDTable.end ());

   card.drag_dest_unset ();
   i->second.connGet.disconnect ();
   i->second.connReceive.disconnect ();
   aDNDTable.erase (i);
}

//-----------------------------------------------------------------------------
/// Stops the drag'n'drop abilities of all cards on the table
//-----------------------------------------------------------------------------
void Machiavelli::unregisterTableDND () {
   for (std::map<Card::Widget*, CONNECTIONS>::iterator i (aDNDTable.begin ());
        i != aDNDTable.end (); ++i) {
      i->second.connGet.disconnect ();
      i->second.connReceive.disconnect ();
   }

   aDNDTable.clear ();
}

//-----------------------------------------------------------------------------
/// Callback after dropping a card (within the hand)
/// \param pContext Context of the drag (contains things like source,
///     target, action, ...)
/// \param data Describes the thing which was dropped
/// \param time Timestamp of the drag
/// \param card Number of card where something was dropped at
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

   Card::Widget& cardMoved (hands[0].remove (*pValue));
   hands[0].insert (cardMoved, card);                     // Insert moved card

   // Adapt dnd-settigns
   if (*pValue < card) {
      unsigned int temp (card);
      card = *pValue;
      *pValue = temp;
   }

   Glib::signal_idle ().connect
       (bind (mem_fun (*this, &Machiavelli::doRegisterHand), card, *pValue));
}

//-----------------------------------------------------------------------------
/// Checks if the piles on the table are valid (have at least 3 cards)
/// \param except Pile which can be invalid
/// \returns bool True, if the piles are OK
//-----------------------------------------------------------------------------
bool Machiavelli::doRegisterHand (unsigned int first, unsigned int last) {
   TRACE9 ("Buraco::doRegisterHand (unsigned int, unsigned int) - [" << first << '-' << last);
   Check1 (last < hands[0].size ());
   Check1 (first <= last);

   registerHandDND (first, last);
   Check3 (aDNDHand.size () == hands[0].size ());
   return false;
}

//-----------------------------------------------------------------------------
/// Callback to query the data to drop
/// \param pContext Context of the drag (contains things like source,
///   target, action, ...)
/// \param data Describes the thing which was dropped
/// \param time Timestamp of the drag
/// \param cardPos Position of card (either in hand or pile on table)
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
/// \param pContext Context of the drag (contains things like source,
///   target, action, ...)
/// \param data Describes the thing which was dropped
/// \param info Describes the type of data (should be HAND or TABLE)
/// \param time Timestamp of the drag
/// \param iCard Combination of card and pile on which card was dropped
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
   Card::IPile& src ((info == HAND) ? hands[0] : *tablePiles[nrpile]);
   Card::Widget* moved (src[off]);
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

      // Check if only cards from an edge are moved to a numbered pile
      if ((tablePiles[nrpile]->getType () == MachiPile::COLOUR)
	  && ((off != (tablePiles[nrpile]->size () - 1)) && off)
	  && (info == TABLE)
	  && (moved->number () == (*pile)[0]->number ())) {
	 context->drag_finish (true, false, time);
	 Gtk::MessageDialog dlg (_("Can't move this card - try splitting the origin first!"),
				 Gtk::MESSAGE_ERROR);
	 dlg.set_title (_("Invalid move"));
	 dlg.run ();
	 return;
      }

      if (info == TABLE) {
	 TRACE8 ("Machiavelli::cardDroppedOnTable (...) - Position: " << iCard);
         // Move only one card from/to a numbered pile
         if ((pile->getType () == MachiPile::NUMBER)
             || ((pile->getType () == MachiPile::UNDEFINED)
                 ? ((pile->size () == 1)
                    && ((*pile)[0]->number () == moved->number ()))
                 : !iCard)
             || (tablePiles[nrpile]->getType () == MachiPile::NUMBER)
	     || ((tablePiles[nrpile]->getType () != MachiPile::NUMBER)
		 && (iCard && (moved->number () == Card::Widget::ACE))))
            nr = 1;
	 else
	    // Move left part of pile, if inserted to the left of the target
	    if (!iCard) {
	       nr = off + 1;
	       moved = src[off = 0];
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

   TRACE8 ("Machiavelli::cardDroppedOnTable (...) - Moving " << nr << " cards from " << off);
   while (nr--) {
      TRACE8 ("Machiavelli::cardDroppedOnTable (...) - Insert to: " << iPile
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

   // Check if human got rid of all cards
   if (hands[0].empty ()) {
      YGP::StatusObject obj;
      checkPiles (obj, true);
      if (obj.getType () == YGP::StatusObject::UNDEFINED) {
	 doEndTurn ();
	 return;
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
	((MachiPile&)src).markValidity ();
      }
   }

   // Re-register the cards in the hand of the human for DND
   if ((info ==HAND) && *pValue < hands[0].size ())
      registerHandDND (*pValue, hands[0].size () - 1);
   Check3 (aDNDHand.size () == hands[0].size ());

   pile->markValidity ();

   undo.push (val);
   undo1->set_sensitive (true);
   undoAll->set_sensitive (true);
}

//----------------------------------------------------------------------------
/// Finds the next player still having cards
/// \param player Player to find next player to
/// \return unsigned int Next player having cards
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
/// \returns MachiPile& New created pile
//-----------------------------------------------------------------------------
MachiPile& Machiavelli::makeNewPile () {
   TRACE8 ("Machiavelli::makeNewPile ()");

   MachiPile* pile (new MachiPile ());
   pile->show ();
   piles.add (*pile);
   tablePiles.push_back (pile);
   TRACE8 ("Machiavelli::makeNewPile () - Pile " << tablePiles.size ());
   return *pile;
}

//-----------------------------------------------------------------------------
/// Makes a new pile in a certain position
/// \param pos Position of pile on the table
/// \returns MachiPile& New created pile
//-----------------------------------------------------------------------------
MachiPile& Machiavelli::makeNewPile (unsigned int pos) {
   TRACE8 ("Machiavelli::makeNewPile ()");

   MachiPile* pile (new MachiPile ());
   pile->show ();
   Check3 (pos <= tablePiles.size ());
   piles.insert (*pile, pos);
   tablePiles.insert (tablePiles.begin () + pos, pile);
   return *pile;
}

//-----------------------------------------------------------------------------
/// Searches the passed pile, if it has a serie of 3 or more and animates them
/// to a new created pile.
/// \param playerPile Pile to inspect
/// \returns bool true, if there's a serie
/// \requires The pile must have at least 3 cards
//-----------------------------------------------------------------------------
bool Machiavelli::playSerie (Card::IPile& playerPile) {
   Check3 (playerPile.size () >= 3);

   // Check for 3 cards belonging to a serie
   for (unsigned int i (0); i < (playerPile.size () - 2); ++i) {
      TRACE8 ("Machiavelli::playSerie (Card::IPile&) - Analyzing card "
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
	 Check3 (nrs >= 3);

	 unsigned int pos1 (i), pos2 (i + nrs - 1);
	 flipCards2Play (playerPile, pos1, pos2);
	 Card::PileWindow& win (animateCards (makeNewPile (), playerPile, pos1, pos2));
	 win.sigAnimation.connect (mem_fun (*this, &Machiavelli::endComputerMove));
         return true;
      }
   }
   return false;
}

//-----------------------------------------------------------------------------
/// Searches for cards to play and shows them in the hand of the actual player.
/// Afterwards they are animated to the target pile.
/// \param card Card which might be added to a pile on the table
/// \param offset Offset of the card in the hand of the player
/// \returns bool True, if the card fits on the pile
/// \remarks Creates a new pile if needed
//-----------------------------------------------------------------------------
bool Machiavelli::cardFitsOnPile (const Card::Widget& card, unsigned int offset) {
   TRACE8 ("Machiavelli::cardFitsOnPile (const Card::Widget&, unsigned int) - Adding card " << card << '?');

   unsigned int dest (-1U);
   for (std::vector<MachiPile*>::const_iterator m (tablePiles.begin ());
        m != tablePiles.end (); ++m) {
      Check2 ((*m)->getType () != MachiPile::UNDEFINED);

      // Check if the card can be added to an existing pile
      dest = (*m)->getPosition4Card (card);
      if (dest != -1U) {
	 flipCards2Play (hands[currentPlayer ()], offset, offset);
	 Card::Window& win (animateCard (**m, dest, hands[currentPlayer ()], offset));
	 win.sigAnimation.connect (mem_fun (*this, &Machiavelli::endComputerMove));
	 return true;
      }

      // Or can the card be added by splitting the pile?
      int diff (MachiPile::cardDistance (card, *(**m)[0]));
      if (((*m)->getType () == MachiPile::COLOUR)
          ? ((diff < 0) || (card.colour () != (**m)[0]->colour ()))
          : diff)
         continue;

      int pos ((*m)->size () - static_cast<unsigned int> (diff));
      TRACE7 ("Machiavelli::cardFitsOnPile (const Card::Widget&, unsigned int) - Splitting "
              << (m - tablePiles.begin ()) << " at " << pos << " (" << diff << ") of " << (*m)->size ());

      // A new pile can be made directly (enough cards on both sides)
      if ((pos > 2) && (diff > 1)) {
         pos = ++diff;
         TRACE8 ("Machiavelli::cardFitsOnPile (const Card::Widget&, unsigned int) - Marked pile: "
                 << std::hex << (m - tablePiles.begin ()) << std::dec);

	 Card::IPile& source (**m);
	 Check3 ((unsigned int)diff < source.size ());
         do
            source[diff]->mark ();
	 while (static_cast<unsigned int> (++diff) < source.size ());
	 MachiPile& newPile (makeNewPile ());
	 flipCards2Play (hands[currentPlayer ()], offset, offset);
	 Card::PileWindows& win (animateCards2 (newPile, hands[currentPlayer ()], offset, offset));
	 win.addWindow (1, source, pos, source.size () - 1);
	 win.sigAnimation.connect (bind (mem_fun (*this, &Machiavelli::unmarkAndEnd),
					 &newPile));
	 return true;
      }
   }
   return false;
}

//-----------------------------------------------------------------------------
/// Searches for cards to play and shows them in the hand of the actual player.
/// Then they are animated to the target pile.
/// \param player Player to inspect
/// \returns bool True, if no card can be played
//-----------------------------------------------------------------------------
bool Machiavelli::showCardsToPlay (unsigned int player) {
   TRACE2 ("Machiavelli::showCardsToPlay (unsigned int) - Player " << player);

   Card::IPile& playerPile (hands[player]);
   if ((playerPile.size () > 2) && playSerie (playerPile))
      return false;

   if (tablePiles.size ()) {
      // Check if any cards fits somewhere/somehow on an existing pile
      for (Card::IPile::const_iterator p (playerPile.begin ());
           p != playerPile.end (); ++p) {
         if (cardFitsOnPile (**p, p - playerPile.begin ()))
	    return false;
      }

      // Try to re-order the piles to enable playing of other cards
      bool rc (reorderTableToFit (playerPile));
#if 0
      if (posPiles.size ()
	  && (getConnectionMgr ().getMode () != YGP::ConnectionMgr::NONE)) {
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
#endif
      missing.clear ();
      return !rc;
   }
   return true;
}

//-----------------------------------------------------------------------------
/// Tries to play a card by rearranging the cards on the table. First it checks
/// if two cards in the hand can be extended by a card on the table.
/// If that fails it calls other methods to perform other checks.
/// \param playerPile Pile to inspect
/// \returns bool True, if table can be re-ordered
//-----------------------------------------------------------------------------
bool Machiavelli::reorderTableToFit (Card::IPile& playerPile) {
   for (Card::IPile::const_iterator p (playerPile.begin ());
        p != playerPile.end (); ++p) {
      Card::IPile::const_iterator h (p);
      std::vector<Card::Widget*> work;
      unsigned int pos1Play, pos2Play;

      // First try to make piles with two cards from the hand
      while ((h = playerPile.getFittingCard (**p, h + 1, &MachiPile::cardDistance))
             != playerPile.end ()) {
         int diff (MachiPile::cardDistance (**h, **p));
	 TRACE5 ("Machiavelli::reorderTableToFit (Card::IPile&) - " << **h << "<->" << **p << ": " << diff);
         if ((diff == 0) && ((*p)->id () == (*h)->id ()))
            continue;

         work.push_back (*p);
         work.push_back (*h);
         TRACE8 ("Machiavelli::reorderTableToFit (Card::IPile&) - Pair: " << **p << " - " << **h);

         // Try to add from the table
         for (std::vector<MachiPile*>::const_iterator t (tablePiles.begin ());
              t != tablePiles.end (); ++t) {
	    Check3 ((*t)->size () > 2);
            Check2 ((*t)->getType () != MachiPile::UNDEFINED);

            MachiPile::const_iterator c;
            unsigned int nr;
            if ((*t)->hasMatching3rd (work, c, nr)) {
	       Check3 (c != (*t)->end ());
	       if ((*t)->size () < 4) {
		  // Don't memorise card as missing, if the hand holds two equal
		  // numbers and the pile is also a numbered one
		  if (diff || ((*t)->getType () == MachiPile::COLOUR))
		     addBorderCards2Missing (t - tablePiles.begin (), 1 << (c == (*t)->begin ()));
		  continue;
	       }

	       // **h is the 1st card from the hand
	       // **p is the 2nd card from the hand
	       // **c is the card from the second pile (*t)
	       TRACE6 ("Machiavelli::reorderTableToFit (Card::IPile&) - Hand "
		       << (p - playerPile.begin ()) << "; " << h - playerPile.begin ());
	       // Sort the card in the hands in the right order;
	       // pos1Play points to the first, pos2Play to the second
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
	       flipCards2Play (playerPile, pos1Play, pos2Play);

	       MachiPile& src (**t);
	       unsigned int posSrc1 (c - (*t)->begin ());
	       unsigned int posSrc2 ((c - (*t)->begin ()) + nr - 1);
               TRACE3 ("Machiavelli::reorderTableToFit (Card::IPile&) - Pile "
                       << (t - tablePiles.begin ()) << "; Cards " << posSrc1 << '-' << posSrc2);

               for (unsigned int pos (0); pos < nr; ++pos)
                  (*(c + pos))->mark ();

	       MachiPile& newPile (makeNewPile ());
	       Card::PileWindows& win (animateCards2 (newPile, playerPile,
						    pos1Play, pos2Play));
	       win.addWindow ((MachiPile::cardDistance (**c, *playerPile[pos2Play]) == 1)
			      ? 2 : 0, src, posSrc1, posSrc2);
	       win.sigAnimation.connect (bind (mem_fun (*this, &Machiavelli::unmarkAndEnd),
					       &newPile));
               return true;
            } // endif pile has matching card
         } // end-for all table piles
         work.clear ();
      } // end-while card has a fitting one
   }

   return reorderTableToFit2 (playerPile);
}

//-----------------------------------------------------------------------------
/// Tries to play a card by moving one card from one pile on the table to
/// another one, so that a card from the hand also fits.
/// \param playerPile Pile to inspect
/// \returns bool True, if cards to play have been found
//-----------------------------------------------------------------------------
bool Machiavelli::reorderTableToFit2 (Card::IPile& playerPile) {
   unsigned int pos2Play;
   for (Card::IPile::const_iterator p (playerPile.begin ());
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
         if ((diff == -2) || ((diff - (*t)->size ()) == 1)) {
            TRACE8 ("Machiavelli::reorderTableToFit2 (Card::IPile&) - With move: "
                    << **p << "; Diff: " << diff);

            for (std::vector<MachiPile*>::const_iterator o (tablePiles.begin ());
                 o != tablePiles.end (); ++o) {
               Check2 ((*o)->getType () != MachiPile::UNDEFINED);
               if ((o == t)
                   || (((*o)->getType () == MachiPile::COLOUR)
                       && ((**o)[0]->colour () != (*p)->colour ())))
                  continue;

	       int pos ((*o)->getPosOfColour ((*p)->colour ()));
	       if (pos <= 0)
		  pos = 0;
               int diffTable (MachiPile::cardDistance (**p, *((**o)[pos]),
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
		  if ((*o)->size () < 4)
		     addBorderCards2Missing (o - tablePiles.begin (),
					     1 << (c == (*o)->begin ()));
		  else {
		     pos2Play = p - playerPile.begin ();
		     flipCards2Play (playerPile, pos2Play, pos2Play);
		     TRACE8 ("Machiavelli::reorderTableToFit2 (Card::IPile&) - Hand: "
			     << **p << " (" << pos1Play << ')');

		     MachiPile& src (**o);
		     unsigned int posSrc (c - (*o)->begin ());
		     unsigned int posDest ((diff < 0) ? 0 : (*t)->size ());
		     TRACE8 ("Machiavelli::reorderTableToFit2 (Card::IPile&) - Pile: "
			     << (o - tablePiles.begin ()) << "; Card " << **c << " (" << posSrc << ')');
		     (*c)->mark ();

		     Card::PileWindows& win (animateCards2 (**t, posDest, playerPile,
							  pos2Play, pos2Play));
		     win.addWindow (posDest + (diff < diffTable), src, posSrc, posSrc);
		     win.sigAnimation.connect (bind (mem_fun (*this, &Machiavelli::unmarkAndEnd),
						     &src));
		     return true;
		  }
               }
            }
         }
      }
   }

   return reorderTableToFit3 (playerPile);
}

//-----------------------------------------------------------------------------
/// Tries to play a card by rearranging two piles on the table
/// \param playerPile Pile to inspect
/// \returns bool True, if table can be re-ordered
//-----------------------------------------------------------------------------
bool Machiavelli::reorderTableToFit3 (Card::IPile& playerPile) {
   unsigned int pos2Play;

   for (Card::IPile::const_iterator p (playerPile.begin ());
        p != playerPile.end (); ++p) {
      std::vector<Card::Widget*> work;

      // Try to find two cards from the table (from different piles)
      for (std::vector<MachiPile*>::const_iterator t (tablePiles.begin ());
           t != tablePiles.end (); ++t) {
         Check2 ((*t)->getType () != MachiPile::UNDEFINED);
         if ((*t)->size () < 4)
            continue;

         for (Card::IPile::const_iterator i ((*t)->begin ());
              (i = (*t)->getFittingCard (**p, i, &MachiPile::cardDistance))
                 != (*t)->end (); ++i) {
            if (((*p)->number () == (*i)->number ())
                && ((*p)->colour () == (*i)->colour ()))
               continue;
	    TRACE7 ("Machiavelli::reorderTableToFit3 (Card::IPile&) - Hand: " << **p
		    << " with " << **i << " on pile " << (t - tablePiles.begin ()));

            int diff (i - (*t)->begin ());
            if (diff && (diff != (int)((*t)->size () - 1))
                && ((diff < 3) || (diff > (int)((*t)->size () - 4))))
               continue;

	    // **p card in hand
	    // *t 1st pile on table
	    // **i 1st matching card
            diff = MachiPile::cardDistance (**p, **i);
            TRACE6 ("Machiavelli::reorderTableToFit3 (Card::IPile&) - Matching " << **i
                    << " differs " << diff);
            work.push_back (*p);
            work.push_back (*i);

	    // Now try to find a 3rd matching card somewhere on the table
            for (std::vector<MachiPile*>::const_iterator o (t + 1);
                 o != tablePiles.end (); ++o) {
               MachiPile::const_iterator c;
               unsigned int nr;
               if ((*o)->hasMatching3rd (work, c, nr)) {
                  Check3 (c != (*o)->end ());
		  TRACE6 ("Machiavelli::reorderTableToFit3 (Card::IPile&) - 3rd: " << **c);

		  // *o 2nd pile on table
		  // **c 2nd matching card
		  if ((*o)->size () < 4) {
		     addBorderCards2Missing (o - tablePiles.begin (), 1 << (c == (*o)->begin ()));
		     if (missing.size ()
			 && (missing.back ().nr == (*i)->number ())
			 && (missing.back ().colour == (*i)->colour ()))
			missing.pop_back ();
		     continue;
		  }

                  // Does the first pile need to be split up?
                  if ((i != (*t)->begin ()) && (i != ((*t)->end () - 1))) {
                     TRACE7 ("Machiavelli::reorderTableToFit3 (Card::IPile&) - Splitting at "
                             << (i - (*t)->begin ()));
                     work.clear ();
                     o = t;
                     c = i + 1;
                     nr = (*o)->end () - c;
                  }

                  if (work.size ()) {
		     int diff2 (MachiPile::cardDistance (**p, **c));
                     TRACE8 ("Machiavelli::reorderTableToFit3 (Card::IPile&) - Hand " << pos2Play
			     << " differs from 2nd table: " << diff2);

                     pos2Play = p - playerPile.begin ();
		     Check3 (o >= tablePiles.begin ());
		     Check3 (c >= (*o)->begin ());
                     Check3 (t >= tablePiles.begin ());
                     Check3 (i >= (*t)->begin ());

		     MachiPile& src1 (**t);
		     MachiPile& src2 (**o);
		     unsigned int posSrc1 (i - (*t)->begin ());
		     unsigned int posSrc2 (c - (*o)->begin ());
                     TRACE8 ("Machiavelli::reorderTableToFit3 (Card::IPile&) - Piles "
                             << (t - tablePiles.begin ()) << " (" << posSrc1 << ") and "
                             << (o - tablePiles.begin ()) << " (" << posSrc2 << ')');
                     (*i)->mark ();
		     (*c)->mark ();

		     flipCards2Play (playerPile, pos2Play, pos2Play);
		     MachiPile& newPile (makeNewPile ());
		     Card::PileWindows& win (animateCards2 (newPile, playerPile, pos2Play, pos2Play));
		     win.addWindow (diff < 0, src1, posSrc1, posSrc1);
		     win.addWindow ((diff2 > diff) ? (diff2 == -1) : ((diff2 < 0) ? 2 : 1),
				    src2, posSrc2, posSrc2);
		     win.sigAnimation.connect (bind (mem_fun (*this, &Machiavelli::unmarkAndEnd),
						     &newPile));
                  }
                  else {
		     Check3 (o >= tablePiles.begin ());
		     Check3 (c >= (*o)->begin ());

		     MachiPile& src (**o);
		     unsigned int posSrc (c - (*o)->begin ());
		     MachiPile& newPile (makeNewPile ());
		     Card::PileWindow& win (animateCards (newPile, src, posSrc, posSrc + nr - 1));
		     win.sigAnimation.connect (bind (mem_fun (*this, &Machiavelli::unmarkAndEnd),
						     &newPile));

		     TRACE8 ("Machiavelli::reorderTableToFit3 (Card::IPile&) - Pile "
			     << (o - tablePiles.begin ()) << "; Card "
			     << (c - (*o)->begin ()) << '-' << (c - (*o)->begin () + nr - 1));
		     Check3 (((unsigned int)(c - (*o)->begin ()) + nr) <= (*o)->size ());
		     while (nr--)
			(*c++)->mark ();
		  }
                  return true;
               } // endif pile has matching card
            } // endfor all following piles

            work.clear ();
         } // end-for all matching cards in the pile
      } // end-for all table piles
   } // end-for all cards

   return reorderTableToFit4 ();
}

//-----------------------------------------------------------------------------
/// Tries to play a card indirectly by filling up a pile missing one card,
/// so that in the next turn the card can be played
/// \returns bool True, if the table can be re-ordered
//-----------------------------------------------------------------------------
bool Machiavelli::reorderTableToFit4 () {
   TRACE8 ("Machiavelli::reorderTableToFit4 ()");

   // Try to find any of the missing cards
   for (std::vector<missingCards>::iterator i (missing.begin ()); i != missing.end (); ++i) {
      TRACE3 ("Machiavelli::reorderTableToFit4 () - " << Card::Widget::strColour (i->colour) << Card::Widget::strNumber (i->nr) << " for pile " << i->pile);

      for (std::vector<MachiPile*>::iterator t (tablePiles.begin ());
           t != tablePiles.end (); ++t) {
	 if (i->pile == (unsigned int)(t - tablePiles.begin ()))
	    continue;

	 Check3 ((*t)->getType () != MachiPile::UNDEFINED);
	 if ((*t)->getType () == MachiPile::COLOUR) {
	    if ((**t)[0]->colour () == i->colour) {
	       TRACE6 ("Machiavelli::reorderTableToFit4 () - Inspecting coloured pile " << t - tablePiles.begin ());

	       unsigned int pos ((*t)->findFirstEqualOrBigger (i->nr));
	       Check3 ((pos == -1U) || (pos < (*t)->size ()));
	       if (((*t)->size () > 3) && (pos != -1U) && ((**t)[pos]->number () == i->nr)) {
		  TRACE8 ("Machiavelli::reorderTableToFit4 () - Card found: " << pos);

		  if (!pos || (pos == ((*t)->size () - 1))) {
		     (**t)[pos]->mark ();

		     Card::Window& win (animateCard (*tablePiles[i->pile],
						   tablePiles[i->pile]->getPosition4Card (*(**t)[pos]),
						   **t, pos));
		     win.sigAnimation.connect (bind (mem_fun (*this, &Machiavelli::unmarkAndEnd),
						     tablePiles[i->pile]));
		     return true;
		  }
		  else
		     if ((pos > 3) && (pos < ((*t)->size () - 2))) {
			MachiPile& src (**t);
			unsigned int nr ((*t)->size () - pos - 1);
			MachiPile& newPile (makeNewPile ());
			Card::PileWindow& win (animateCards (newPile, src, pos, pos + nr));
			win.sigAnimation.connect (bind (mem_fun (*this, &Machiavelli::unmarkAndEnd),
							&newPile));
			while (nr)
			   src[pos + --nr]->mark ();
			return true;
		     }
	       }
	    } // endif pile has the right colour
	 } // endif coloured pile
	 else
	    if (((*t)->size () == 4) && ((**t)[0]->number () == i->nr)) {
	       TRACE6 ("Machiavelli::reorderTableToFit4 () - Inspecting numbered pile " << t - tablePiles.begin ());

	       for (Card::IPile::const_iterator p ((*t)->begin ()); p != (*t)->end (); ++p)
		  if ((*p)->colour () == i->colour) {
		     unsigned int pos (p - (*t)->begin ());
		     Card::Window& win (animateCard (*tablePiles[i->pile],
						   tablePiles[i->pile]->getPosition4Card (*(**t)[pos]),
						   **t, pos));
		     win.sigAnimation.connect (bind (mem_fun (*this, &Machiavelli::unmarkAndEnd),
						     tablePiles[i->pile]));
		     (**t)[pos]->mark ();
		     return true;
		  }
	    }
      }
   }
   return false;
}

//----------------------------------------------------------------------------
/// Deals a card to the passed player
/// \param player Player to give a card to
//----------------------------------------------------------------------------
void Machiavelli::dealCard (unsigned int player) {
   TRACE5 ("Machiavelli::dealCard (unsigned int) - " << player);
   if (staple.size () == 1) {
      Gtk::MessageDialog dlg (_("Taking last card! Solve the game (somehow) ..."),
                              Gtk::MESSAGE_ERROR);
      dlg.set_title (_("Game over"));
      dlg.run ();
   }

   if (staple.size ()) {
      unsigned int pos (player ? hands[player].findByNr (staple.getTopCard ()) : hands[0].size ());
      if (pos == -1U)
	 pos = hands[player].size ();
      animateCard (hands[player], pos, staple, staple.size () - 1)
	 .sigAnimation.connect (mem_fun (this, &Machiavelli::endComputerMove));
   }
}

//----------------------------------------------------------------------------
/// Checks, if all the piles on the table are valid
/// \param obj Object collecting all errors
/// \param mark Flag if invalid piles should be marked
//----------------------------------------------------------------------------
void Machiavelli::checkPiles (YGP::StatusObject& obj, bool mark) const {
   for (std::vector<MachiPile*>::const_iterator i (tablePiles.begin ());
        i != tablePiles.end (); ++i) {
       try {
           (*i)->checkIntegrity ();
	   (*i)->unmark ();
       }
       catch (MachiPile::PileError& error) {
	  (*i)->mark ();
	  TRACE8 ("Machiavelli::checkPiles () const - Pile " << (i - tablePiles.begin ()) << ": " << error.what ());
	  Glib::ustring msg (_("Pile %1: %2\n"));
	  msg.replace (msg.find ("%1"), 2, YGP::ANumeric::toString (i - tablePiles.begin () + 1));
	  msg.replace (msg.find ("%2"), 2, error.what ());
          obj.setMessage (YGP::StatusObject::ERROR, msg);
       }
   }
}

//----------------------------------------------------------------------------
/// Undoes the passed number of moves (starting from the last)
/// \param number Number of moves to undo
//----------------------------------------------------------------------------
void Machiavelli::undoMove (unsigned int number) {
   TRACE3 ("Machiavelli::undoMove (unsigned int) - Undo " << number);
   Check2 (number);
   Check2 (undo.size ());

   if (number > undo.size ())
      number = undo.size ();
   TRACE8 ("Machiavelli::undoMove (unsigned int) - Undo (avail): " << number);

   disableHuman ();

   while (number--) {
      undoValue move (undo.top ());
      undo.pop ();

      if (move.create)
         makeNewPile (move.srcPile);

      TRACE8 ("Machiavelli::undoMove (unsigned int) - Undo " << move.number
              << "; " << move.destPile << '/' << move.destPos << "-> "
              << move.srcPile << '/' << move.srcPos);
      Card::IPile& dest ((move.srcPile == 0xff)
                       ? hands[currentPlayer ()] : *tablePiles[move.srcPile]);
      Check3 (move.srcPos <= dest.size ());

      Check3 (move.destPile < tablePiles.size ());
      MachiPile& src (*tablePiles[move.destPile]);
      Check3 (move.destPos < src.size ());
      Check3 (move.number);
      Check3 ((unsigned int)(move.number + move.destPos) <= src.size ());

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

   if (undo.empty ()) {
      if (undoDlg) {
	 delete undoDlg;
	 undoDlg = NULL;
      }

      undo1->set_sensitive (false);
      undoAll->set_sensitive (false);
   }
   else
      if (undoDlg) {
	 YGP::StatusObject obj;
	 checkPiles (obj, true);
	 if (obj.getType () != YGP::StatusObject::UNDEFINED)
	    obj.generalize (_("Can't end turn: The piles are not valid!"));
	 else
	    obj.setMessage (YGP::StatusObject::INFO, _("Could end turn: The piles are OK!"));

	 undoDlg->update (obj);
      }
}

//----------------------------------------------------------------------------
/// Removes the passed pile from the table and internally
/// \param pile Offset of pile to remove
//----------------------------------------------------------------------------
void Machiavelli::removePile (unsigned int pile) {
   TRACE8 ("Machiavelli::removePile (unsigned int) - " << pile);
   Check1 (pile < tablePiles.size ());

   MachiPile& tmp (*tablePiles[pile]);
   Check3 (tmp.empty ());

   tablePiles.erase (tablePiles.begin () + pile);
   piles.remove (tmp);
   delete &tmp;
}

//----------------------------------------------------------------------------
/// Ends the game
/// \param looser Number of player having lost the game
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
/// \param player Number of player
/// \param pile ID of the pile to return
/// \returns Card::IPile* Pile corresponding to the passed number or NULL
//----------------------------------------------------------------------------
Card::IPile* Machiavelli::getPileOfPlayer (unsigned int player, unsigned int pile) {
   if ((player >= NUM_PLAYERS) || ((pile >> 16) > tablePiles.size ()))
      return NULL;

   target = pile;
   pile >>= 16;
   if (pile == tablePiles.size ())
      makeNewPile ();

   return &hands[player];
}

//----------------------------------------------------------------------------
/// Handles the messages the server might send for the Machiavelli cardgame
/// \param player ID of the player sending the message
/// \param message Message received from the server
/// \returns bool True, if message has been processed completey
/// \throw YGP::ParseError, YGP::CommError In case of an error an describing text
//----------------------------------------------------------------------------
bool Machiavelli::handleMessage (unsigned int player, const std::string& message) throw (YGP::ParseError, YGP::CommError) {
   TRACE1 ("Machiavelli::handleMessage (unsigned int player, const std::string&) - "
           << message << " (" << player << ')');

   bool rc (true);
#if 0
   YGP::Tokenize command (message);
   std::string cmd (command.getNextNode ('='));

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
            TRACE1 ("Machiavelli::handleMessage (unsigned int, const std::string&)"
		    " - Invalid piles!\n" << obj.getMessage ());
            Check (!"Valid piles");
         }
#endif

      }
   }
   else if (cmd == "Reorder") {
      YGP::Tokenize tokCards (command.getNextNode (';'));
      unsigned long posPile (0);
      while (tokCards.getNextNode (' ').size ()) {
         if (stringToNumber (posPile, tokCards.getActNode ().c_str ())) {
            std::string error ("Not a card number: `%1'");
            error.replace (error.find ("%1"), 2, tokCards.getActNode ());
            throw YGP::ParseError (error);
         }

         unsigned int pile ((posPile >> 8) & 0xff);
         unsigned int nr (posPile >> 16);
         unsigned int posSrc (posPile & 0xff);
         TRACE8 ("Machiavelli::handleMessage (unsigned int, const std::string&)"
                 " - Add from " << pile << " cards " << posSrc << '-' << (posSrc + nr - 1));

         if (pile >= tablePiles.size ())
            throw YGP::ParseError (N_("Invalid source pile!"));
         Card::IPile& srcPile (*tablePiles[pile]);
         if ((posSrc + nr) > srcPile.size ())
            throw YGP::ParseError (N_("Invalid cards!"));

         // TODO: posPiles.push_back (posPile);

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
         throw YGP::ParseError (N_("Invalid destination pile!"));
      }

      if (targetPile == tablePiles.size ())
         makeNewPile ();

      // Inform clients about cards to play
      if (getConnectionMgr ().getMode () == YGP::ConnectionMgr::SERVER)
          broadcastMessage (message);

      if (now)
         Glib::signal_timeout ().connect
            (bind (mem_fun (*this, &Machiavelli::endRemoteMove),
                   currentPlayer ()), Card::ComputerPlayer::TIMEOUT);
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
         throw YGP::ParseError (N_("Invalid destination pile!"));
      if (create) {
         Check3 (dest != 255);
         makeNewPile (dest);
      }
      Card::IPile& pile ((dest == 255) ? hands[player] : *tablePiles[dest]);
      Card::IPile* srcPile (NULL);
      try {
         if (destPos > pile.size ())
            throw YGP::ParseError (N_("Invalid position in destination pile!"));

         if (src >= tablePiles.size ())
            throw YGP::ParseError (N_("Invalid source pile!"));
          srcPile = tablePiles[src];
         if ((card2 < card1) || (card2 >= srcPile->size ()))
            throw YGP::ParseError (N_("Invalid cards!"));
      }
      catch (...) {
         if (create)
            removePile (dest);
         throw;
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
#endif
   return rc;
}

//----------------------------------------------------------------------------
/// Returns the actual target, where flipCard2Play should position the cards to
/// \returns unsigned int ID of the target
//----------------------------------------------------------------------------
unsigned int Machiavelli::getActTarget () const {
   Check3 ((target >> 16) < tablePiles.size ());
   return target;
}

//-----------------------------------------------------------------------------
/// Adds machiavelli-specific menus
/// \param mgrUI UIManager to add to
//-----------------------------------------------------------------------------
void Machiavelli::addMenus (Glib::RefPtr<Gtk::UIManager> mgrUI) {
   Check1 (mgrUI);
   Glib::ustring ui ("<menubar name='Menu'>"
		     "  <placeholder name='GameMenu'>"
		     "    <menu action='MB'>"
		     "      <menuitem action='MachiUndo'/>"
		     "      <menuitem action='MachiUndoAll'/>"
		     "      <separator/>"
		     "      <menuitem action='MachiSort'/>"
		     "      <menuitem action='MachiSortCol'/>"
		     "      <separator/>"
		     "      <menuitem action='MachiEndTurn'/>"
		     "    </menu></placeholder></menubar>");

   Glib::RefPtr<Gtk::ActionGroup> grpAction (Gtk::ActionGroup::create ());
   grpAction->add (Gtk::Action::create ("MB", _("_Machiavelli")));
   grpAction->add (undo1 = Gtk::Action::create ("MachiUndo", Gtk::Stock::UNDO),
		   Gtk::AccelKey ("<ctl>Z"),
		   bind (mem_fun (*this, &Machiavelli::undoMove), 1));
   grpAction->add (undoAll = Gtk::Action::create ("MachiUndoAll", _("Undo _all")),
		   Gtk::AccelKey ("<ctl><alt>Z"),
		   bind (mem_fun (*this, &Machiavelli::undoMove), -1U));
   grpAction->add (Gtk::Action::create ("MachiSort", Gtk::Stock::SORT_ASCENDING,
					_("_Sort cards (by number)")),
		   Gtk::AccelKey ("<shft>S"),
		   mem_fun (*this, &Machiavelli::sortHand));
   grpAction->add (Gtk::Action::create ("MachiSortCol", Gtk::Stock::SORT_ASCENDING,
					_("Sort cards (by _colour)")),
		   Gtk::AccelKey ("S"),
		   mem_fun (*this, &Machiavelli::sortHandByColour));
   grpAction->add (nxtTurn = Gtk::Action::create ("MachiEndTurn", _("_End turn")),
		   mem_fun (*this, (&Machiavelli::endTurn)));


   mgrUI->insert_action_group (grpAction);
   idMrg = mgrUI->add_ui_from_string (ui);

   undo1->set_sensitive (false);
   undoAll->set_sensitive (false);
}

//-----------------------------------------------------------------------------
/// Removes the machiavelli-specific menus
/// \param mgrUI UIManager to remove from
//-----------------------------------------------------------------------------
void Machiavelli::removeMenus (Glib::RefPtr<Gtk::UIManager> mgrUI) {
   Check1 (mgrUI);
   mgrUI->remove_ui (idMrg);
}

//-----------------------------------------------------------------------------
/// Sorts the cards in the hand by number
//-----------------------------------------------------------------------------
void Machiavelli::sortHand () {
   bool enabled (activeCards.size ());
   if (enabled)
      disableHuman ();
   hands[0].sort (Card::IPile::compCardsByNr);
   if (enabled)
      enableHuman ();
}

//-----------------------------------------------------------------------------
/// Sorts the cards in the hand by colour
//-----------------------------------------------------------------------------
void Machiavelli::sortHandByColour () {
   bool enabled (activeCards.size ());
   if (enabled)
      disableHuman ();
   hands[0].sort (Card::IPile::compCards);
   if (enabled)
      enableHuman ();
}

//-----------------------------------------------------------------------------
/// Adds "bordering" cards to the missing cards vector.
///
/// "Bordering cards" are defined as either the cards which would fit
/// on the edge for numbered piles or the missing colour in a coloured
/// pile.
/// \param iPile Offset of pile to get the "bordering" cards from
/// \param which Defines for numbered pile at which end of the pile a card
///              should  be added (0x1: left; 0x2: right; can be or-ed together)
//-----------------------------------------------------------------------------
void Machiavelli::addBorderCards2Missing (unsigned int iPile, unsigned int which) {
   TRACE8 ("Machiavelli::addBorderCards2Missing (MachiPile&) - Pile: " << iPile << "; Which: " << which);
   Check1 (iPile < tablePiles.size ());

   MachiPile& pile (*tablePiles[iPile]);
   Check3 (pile.size () > 2);
   Check3 (pile.getType () != MachiPile::UNDEFINED);

   missingCards card (iPile);
   if (pile.getType () == MachiPile::COLOUR) {
      card.colour = pile[0]->colour ();
      if ((which & 0x1) && (pile[0]->number () != Card::Widget::ACE)) {
	 card.nr = Card::Widget::NUMBERS (pile[0]->number () - 1);
	 missing.push_back (card);
      }
      if ((which & 0x2) && (pile[pile.size () - 1]->number () != Card::Widget::ACE)) {
	 card.nr = Card::Widget::NUMBERS (pile[pile.size () - 1]->number () + 1);
	 missing.push_back (card);
      }
   }
   else {
      unsigned int fUsed (0);
      for (Card::IPile::const_iterator p (pile.begin ()); p != pile.end (); ++p) {
	 Check2 ((*p)->number () == (*pile.begin ())->number ());
	 fUsed |= 1 << (*p)->colour ();
      }

      card.nr = pile[0]->number ();
      for (unsigned int i (0); i < 4; ++i)
	 if (!(fUsed & (1 << i))) {
	    card.colour = Card::Widget::COLOURS (i);
	    missing.push_back (card);
	    break;
	 }
   }
}

//-----------------------------------------------------------------------------
/// Callback after removing the undo-dialog. Sets the undoDlg variable to NULL
//-----------------------------------------------------------------------------
void Machiavelli::removeUndoDlg (int) {
   TRACE1 ("Machiavelli::removeUndoDlg (int)");
   delete undoDlg;
   undoDlg = NULL;
}

//-----------------------------------------------------------------------------
/// Actions to take when the cards are resized
/// \pre The cardsize must be set in CardImages::WIDTH/HEIGHT
//-----------------------------------------------------------------------------
void Machiavelli::resizeCards () {
   staple.set_size_request (Card::Images::WIDTH, Card::Images::HEIGHT);
   for (unsigned int i (0); i < NUM_PLAYERS; ++i)
      hands[i].set_size_request (-1, Card::Images::HEIGHT);

}

//-----------------------------------------------------------------------------
/// Callback after animating marked cards; all cards in the passed pile are
/// unmarked
/// \param pile Pile to unmark
//-----------------------------------------------------------------------------
void Machiavelli::unmarkAndEnd (MachiPile* pile) {
   Check1 (pile);
   pile->unmark ();
   endComputerMove ();
}

//-----------------------------------------------------------------------------
/// Finishes the turn and starts the next one
//-----------------------------------------------------------------------------
void Machiavelli::endComputerMove () {
   TRACE9 ("Machiavelli::endComputerMove ()");
#if CHECK > 0
   YGP::StatusObject obj;
   checkPiles (obj);
   Check (obj.getType () == YGP::StatusObject::UNDEFINED);
#endif

   makeNextMoves ();
}
