//$Id$

//PROJECT     : Cardgames
//SUBSYSTEM   : Machiavelli
//REFERENCES  :
//TODO        :
//BUGS        :
//REVISION    : $Revision$
//AUTHOR      : Markus Schwab
//CREATED     : 05.11.2003
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

#include <sstream>

#include <gtk/gtkdnd.h>

#include <gtkmm/statusbar.h>

#define CHECK 9
#define TRACELEVEL 9
#include <Check.h>
#include <Trace_.h>
#include <ConnMgr.h>
#include <Tokenize.h>
#include <AttrParse.h>

#include <Player.h>
#include <CardSet.h>
#include <CardWidget.h>

#include "SigCExt.h"

#include "Machiavelli.h"


static std::vector<Gtk::TargetEntry> dndType;


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
                unsigned int posPlayer, Mutex& mxSerialize)
   : Game (parent, statusbar, cardset, player, posPlayer, mxSerialize, 3, 10)
     , startPlayer (-1U), newPile (_("New pile"))
     , staple (ICardPile::TOTALLY_COMPRESSED, ICardPile::SHOWBACK)
     , target (-1U) {
   TRACE9 ("Machiavelli::Machiavelli (Box&, Statusbar&, CardSet&, const "
           "std::vector<Glib::ustring>&)");

   scrlTable.set_policy (Gtk::POLICY_AUTOMATIC, Gtk::POLICY_AUTOMATIC);
   scrlTable.add (table);

   int width (cards.getCard (0).getImageWidth ());
   int height (cards.getCard (0).getImageHeight ());

   TRACE9 ("Machiavelli::Machiavelli (Box&, Statusbar&, CardSet&, const "
           "std::vector<Glib::ustring>&) - Init common staples");
   staple.set_size_request (width, height);

   table.pack_start (newPile, Gtk::PACK_EXPAND_WIDGET, 5);
   for (unsigned int i (0); i < (sizeof (piles) / sizeof (piles[0])); ++i) {
      piles[i].set_size_request (-1, height + 5);
      piles[i].show ();
      table.pack_start (piles[i], Gtk::PACK_EXPAND_WIDGET, 5);
   }

   for (unsigned int i (1); i < NUM_PLAYERS; ++i) {
      attach (hands[i], (i << 2) - 4, (i << 2) - 2, 3, 4,
              Gtk::EXPAND, Gtk::SHRINK, 5, 5);
      attach (names[i], (i << 2) - 4, (i << 2) - 2, 4, 5,
              Gtk::EXPAND, Gtk::SHRINK, 0);
   }
   hands[0].setStyle (ICardPile::COMPRESSED);
   hands[0].setShowOption (ICardPile::SHOWFACE);

   TRACE9 ("Machiavelli::Machiavelli (Box&, Statusbar&, CardSet&, const "
           "std::vector<Glib::ustring>&) - Attach widgets");
   attach (hands[0], 3, 10, 0, 1, Gtk::EXPAND, Gtk::SHRINK, 1, 5);
   attach (names[0], 3, 10, 1, 2, Gtk::EXPAND, Gtk::SHRINK, 1, 5);
   attach (staple, 0, 1, 0, 1, Gtk::SHRINK, Gtk::SHRINK, 5);
   attach (scrlTable, 0, 10, 2, 3, Gtk::EXPAND | Gtk::FILL,
           Gtk::EXPAND | Gtk::FILL, 0, 5);

   TRACE9 ("Machiavelli::Machiavelli (Box&, Statusbar&, CardSet&, const "
           "std::vector<Glib::ustring>&) - Show widgets");

   changeNames (player);

   if (dndType.empty ())
      dndType.push_back
         (Gtk::TargetEntry ("icon/card", GTK_TARGET_SAME_APP, 0));

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
      for (unsigned int i (0); i < NUM_PLAYERS; ++i) {
          for (unsigned int j (0); j < 7; ++j)
             hands[(i - posServer) & 0x3].setTopCard (staple.removeTopCard ());

          hands[i].sortByColour ();
      }

      for (unsigned int i (1); i < NUM_PLAYERS; ++i) {
         hands[i].setStyle (ICardPile::QUITE_COMPRESSED);
         hands[i].setShowOption (ICardPile::SHOWBACK);
      }

      status.pop ();
      status.push (_("You can sort the cards in your hand with drag and drop or put"
                     " them on the table - hit enter to end turn"));

      // Set random startplayer (if not already set)
      if (startPlayer == -1U)
         startPlayer = 0; // TODO: Set to: rand () & 0x3;
      setStartPlayer ();
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

   for (unsigned int i (0); i < (sizeof (piles) / sizeof (piles[0])); ++i) {
      Gtk::Box::BoxList& children (piles[i].children ());
      children.erase (children.begin (), children.end ());
   }

   Game::clean ();
}

//-----------------------------------------------------------------------------
/// Shows or hides the cards of the computer player
/// \param open: Flag if cards should be shown or hidden
//-----------------------------------------------------------------------------
void Machiavelli::playOpen (bool open) {
   for (unsigned int i (1); i < NUM_PLAYERS; ++i)
      hands[i].setShowOption (open ? ICardPile::SHOWFACE : ICardPile::SHOWBACK);
}

//----------------------------------------------------------------------------
/// Handles the messages the server might send for the twopart cardgame
/// \param player: ID of the player sending the message
/// \param message: Message received from the server
/// \returns bool: True, if message has been processed completey
//----------------------------------------------------------------------------
bool Machiavelli::handleMessage (unsigned int player, const char* message) {
   TRACE1 ("Machiavelli::handleMessage (unsigned int player, const char*) - "
           << message << " (" << player << ')');
    
   Tokenize command (message);
   std::string cmd (command.getNextNode ('='));

   bool rc (true);
   if (cmd == "Move") {
      AttributeParse ap;
      unsigned int card (-1U), dest (-1U), iPile (-1U);
      ATTRIBUTE (ap, unsigned int, card, "Move");
      ATTRIBUTE (ap, unsigned int, dest, "To");
      ATTRIBUTE (ap, unsigned int, iPile, "Pile");
      ap.assignValues (message);

      unsigned int iRow (iPile >> 16);
      unsigned int iColumn (iPile & 0xffff);
      if ((iRow >= (sizeof (piles) / sizeof (piles[0])))
          || (iColumn >= piles[iRow].children ().size ()))
         throw std::string ("Invalid pile!");

      Gtk::Box_Helpers::Child& child (piles[iRow].children ()[iColumn]);
      ICardPile& pile (*dynamic_cast<ICardPile*> (child.get_widget ()));
      if ((card >= pile.size ()) || (dest >= pile.size ()))
         throw std::string ("Invalid card!");

      pile.move (dest, card);
   }
   else {
      rc = Game::handleMessage (player, message);
      if (cmd == "ActPlayer") {
         TRACE1 ("Machiavelli::handleMessage (unsigned int player, const char*) - "
                 "Next player: " << currentPlayer ());

         startPlayer = currentPlayer ();
         setStartPlayer ();
      }
   }
   return rc;
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
   Check1 (!hands[player].empty ());

   unsigned int nextPlayer (findNextPlayer (player));
   hands[nextPlayer].append (staple.removeTopCard ());
   return nextPlayer;
}

//-----------------------------------------------------------------------------
/// Enables the cards the human can pick up.
/// \returns \c 0
//-----------------------------------------------------------------------------
bool Machiavelli::enableHuman () {
   TRACE4 ("Machiavelli::enableHuman ()");
   Check3 (staple.size ());
   Check3 (activeCards.empty ());

   activeCards.push_back (staple.getTopCard ().signal_clicked ().connect
                          (slot (*this, (&Machiavelli::stapleSelected))));

   Check3 (hands[0].size ());
   for (unsigned int i (0); i < hands[0].size (); ++i)
      registerHandDND (i);
   Check3 (aDNDHand.size () == hands[0].size ());

   newPile.drag_dest_set (dndType, Gtk::DEST_DEFAULT_ALL, Gdk::ACTION_MOVE);
   aDNDTable[NULL] = newPile.signal_drag_data_received ().connect
      (bind (slot (*this, &Machiavelli::cardDroppedOnTable), -1U));

   for (unsigned int i (0); i < (sizeof (piles) / sizeof (piles[0])); ++i)
      for (unsigned int j (0); j < piles[i].children ().size (); j++) {
         unsigned int valPile ((i << 12) + (j << 8));

         Gtk::Box_Helpers::Child& child (piles[i].children ()[j]);
         ICardPile& pile (*dynamic_cast<ICardPile*> (child.get_widget ()));

         for (unsigned int k (0); k < pile.size (); ++k)
            registerTableDND (*pile[k], (valPile) + k);
   }

   return Game::enableHuman ();
}

//-----------------------------------------------------------------------------
/// Disables the cards the human player can select
//-----------------------------------------------------------------------------
void Machiavelli::disableHuman () {
   TRACE2 ("Buarzno::disableHuman () - DND: " << aDNDHand.size () << "; "
           << aDNDTable.size ());
   Game::disableHuman ();

   newPile.drag_dest_unset ();

   if (aDNDHand.size ())
      for (unsigned int i (0); i < hands[0].size (); ++i)
         unregisterHandDND (*hands[0][i]);
   Check3 (aDNDHand.empty ());

   unregisterTableDND ();
}

//----------------------------------------------------------------------------
/// Changes the names of the playing people
/// \param newPlayer: Array holding the new player
/// \param pile: ID of the pile to return
//----------------------------------------------------------------------------
ICardPile& Machiavelli::getPileOfPlayer (unsigned int player, unsigned int pile) {
   Check1 (player < NUM_PLAYERS);

   unsigned int iRow (pile >> 16);
   unsigned int iColumn (pile & 0xffff);
   Check1 (iRow < (sizeof (piles) / sizeof (piles[0])));
   Check1 (iColumn < piles[iRow].children ().size ());

   Gtk::Box_Helpers::Child& child (piles[iRow].children ()[iColumn]);
   return (*dynamic_cast<ICardPile*> (child.get_widget ()));
}

//----------------------------------------------------------------------------
/// Returns the actual target, where flipCard2Play should position the cards to
/// \returns unsigned int: ID of the target
//----------------------------------------------------------------------------
unsigned int Machiavelli::getActTarget () const {
   return target;
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
   if (getConnectionMgr ().getMode () != ConnectionMgr::CLIENT) {
      setNextPlayer (startPlayer);

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
   }

   displayTurn (startPlayer++);
   startPlayer &= 0x3;
   makeNextMoves ();
}

//-----------------------------------------------------------------------------
/// Callback after clicking on the staple
//-----------------------------------------------------------------------------
void Machiavelli::stapleSelected () {
   TRACE5 ("Machiavelli::stapleSelected ()");
   Check1 (gameStatus () == PLAYING);
   Check3 (staple.size ()); Check3 (activeCards.size ());

   if (getConnectionMgr ().getMode () != ConnectionMgr::NONE) {
      // Send played card to all clients (if any)
      std::ostringstream msg;
      msg << "Play=" << staple.getTopCard ().id () << ";Target=1";

      if (getConnectionMgr ().getMode () == ConnectionMgr::CLIENT)
         ignoreNextMsg = true;
      broadcastMessage (msg.str ());
   }

   disableHuman ();

   unsigned int nextPlayer (findNextPlayer (currentPlayer ()));
   setNextPlayer (nextPlayer);
   hands[nextPlayer].append (staple.removeTopCard ());

   makeNextMoves ();
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
           << *hands[0][iCard] << " = " << hands[0][iCard] << ')');

   CardWidget& card (*hands[0][iCard]);
   Check3 (aDNDHand.find (&card) == aDNDHand.end ());

   // Card accepts drops from hand and drags from table
   card.drag_dest_set (dndType, Gtk::DEST_DEFAULT_ALL, Gdk::ACTION_MOVE);
   card.drag_source_set
      (dndType, Gdk::ModifierType (GDK_BUTTON2_MASK | GDK_BUTTON3_MASK),
       Gdk::ACTION_MOVE);

   card.drag_source_set_icon (card.getImage ());
   aDNDHand[&card].connReceive = card.signal_drag_data_received ().connect
      (bind (slot (*this, &Machiavelli::cardDropped), iCard));
   aDNDHand[&card].connGet = card.signal_drag_data_get ().connect
      (bind (slot (*this, &Machiavelli::getDropData), iCard));
}

//-----------------------------------------------------------------------------
/// Stops the drag´n´drop abilities of the passed card
/// \param card: Card to unregister of dnd
//-----------------------------------------------------------------------------
void Machiavelli::unregisterHandDND (CardWidget& card) {
   TRACE9 ("Machiavelli::unregisterHandDND (CardWidget&) - Card: " << card
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
/// \param pile: Pile whose cards should be registered. This value is calcualated
///     like (row << 4) + column
/// \param start: Number of first card to prepare for DND
/// \param end: Number of last card to prepare for DND
/// \pre: \c start < \c end; \c end <= Number of cards
//-----------------------------------------------------------------------------
void Machiavelli::registerTableDND (unsigned int pile, unsigned int start, unsigned int end) {
   TRACE9 ("Machiavelli::registerTableDND (unsigned int, unsigned int, unsigned int)"
           << " - " << pile << '[' << start << '-' << end << ']');

   Check1 (start <= end);
   unsigned int iRow (pile >> 4);
   unsigned int iColumn (pile & 0xff);
   Check1 (iRow < (sizeof (piles) / sizeof (piles[0])));
   Check1 (iColumn < piles[iRow].children ().size ());

   Gtk::Box_Helpers::Child& child (piles[iRow].children ()[iColumn]);
   ICardPile& tmp (*dynamic_cast<ICardPile*> (child.get_widget ()));
   Check1 (end < tmp.size ());

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
           << " = " << std::hex << nr << " - " << &card << std::dec);

   // Card accepts drops from hand and drags from table
   card.drag_dest_set (dndType, Gtk::DEST_DEFAULT_ALL, Gdk::ACTION_MOVE);
   aDNDTable[&card] = card.signal_drag_data_received ().connect
      (bind (slot (*this, &Machiavelli::cardDroppedOnTable), nr));
}

//-----------------------------------------------------------------------------
/// Stops the drag´n´drop abilities of the passed card
/// \param card: Card to de-register
//-----------------------------------------------------------------------------
void Machiavelli::unregisterTableDND (CardWidget& card) {
   TRACE9 ("Machiavelli::unregisterTableDND (unsigned int) - Card: " << card
           << " - " << &card );
   Check1 (aDNDTable.size () > 1);

   std::map<CardWidget*, SigC::Connection>::iterator i (aDNDTable.find (&card));
   Check1 (i != aDNDTable.end ());

   card.drag_dest_unset ();
   i->second.disconnect ();
   aDNDTable.erase (i);
}

//-----------------------------------------------------------------------------
/// Stops the drag´n´drop abilities of all cards on the table
//-----------------------------------------------------------------------------
void Machiavelli::unregisterTableDND () {
   for (std::map<CardWidget*, SigC::Connection>::iterator i (aDNDTable.begin ());
        i != aDNDTable.end (); ++i)
      i->second.disconnect ();

   aDNDTable.clear ();
}

//-----------------------------------------------------------------------------
/// Callback after dropping a card (within the hand)
/// \param pContext: Context of the drag (contains things like source,
/// \param target, action, ...)
/// \param pData: Describes the thing which was dropped
/// \param info: Describes the type of pData (should be 0)
/// \param time: Timestamp of the drag
/// \param card: Number of card where something was dropped at
/// \pre \c pContext, \c pData not NULL; Expects \c info to be 0
//-----------------------------------------------------------------------------
void Machiavelli::cardDropped (const Glib::RefPtr<Gdk::DragContext>& context,
                               gint, gint, GtkSelectionData* pData, guint info,
                               guint32 time, unsigned int card) {
   Check3 (pData);
   Check3 (!context->get_is_source ());
   Check3 (pData->length == sizeof (int));
   Check3 (pData->format == 8);
   Check3 (card < hands[0].size ());

   unsigned int* pValue (reinterpret_cast <unsigned int*> (pData->data));
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
/// \param pData: Describes the thing which was dropped
/// \param time: Timestamp of the drag
/// \param cardPos: Position of card (either in hand or pile on table)
/// \pre \c pContext, \c pData not NULL; Expects \c info to be 0
//-----------------------------------------------------------------------------
void Machiavelli::getDropData (const Glib::RefPtr<Gdk::DragContext>& pContext,
                               GtkSelectionData* pData, guint info, guint32 time,
                               unsigned int cardPos) {
   Check1 (pData); Check1 (!info);
   Check1 (pContext->get_is_source ());

   gtk_selection_data_set (pData, pData->target, 8, reinterpret_cast <guchar*> (&cardPos),
                           sizeof (cardPos));
}

//-----------------------------------------------------------------------------
/// Callback after dropping a card on the table
/// \param pContext: Context of the drag (contains things like source,
/// \param target, action, ...)
/// \param pData: Describes the thing which was dropped
/// \param info: Describes the type of pData (should be 0)
/// \param time: Timestamp of the drag
/// \param iCard: Combination of card and pile on which card was dropped
/// \pre \c pContext, \c pData not NULL;
//-----------------------------------------------------------------------------
void Machiavelli::cardDroppedOnTable (const Glib::RefPtr<Gdk::DragContext>& context,
                                      gint, gint, GtkSelectionData* pData,
                                      guint, guint32 time, unsigned int iCard) {
   TRACE1 ("Machiavelli::cardDroppedOnTable (...) - Card dropped on " << std::hex
           << (int)iCard << std::dec);
   Check3 (pData);
   Check3 (!context->get_is_source ());
   Check3 (pData->length == sizeof (int));
   Check3 (pData->format == 8);

   unsigned int* pValue (reinterpret_cast <unsigned int*> (pData->data));
   Check3 (pValue);
   Check3 (*pValue < hands[0].size ());
   TRACE1 ("Machiavelli::cardDroppedOnTable (...) - Inserting card " << *pValue
           << " in pile");

   // Move dropped card to a (new) pile on the table
   unsigned int iPile (piles[0].children ().size ());
   MachiPile* pile (NULL);
   CardWidget& moved (*hands[0][*pValue]);
   TRACE4 ("Machiavelli::cardDroppedOnTable (...) - Card dropped: " << moved);

   if (iCard == -1U) {    // If card was dropped on the new label: Create pile
      unsigned int iRow (0);

      // Find row where to create new pile
      for (unsigned int i (1); i < (sizeof (piles) / sizeof (piles[0])); ++i) {
         iPile += piles[i].children ().size ();

         GtkRequisition req1, req2;
         piles[i].size_request (&req1);
         TRACE9 ("Machiavelli::cardDroppedOnTable (...) - Width: " << req1.width);

         piles[iRow].size_request (&req2);
         if (req1.width < req2.width)
            iRow = i;
      }

      pile = new MachiPile ();
      piles[iRow].pack_start (*pile, Gtk::PACK_SHRINK, 5);

      pile->show ();
      iCard = 0;
      iPile += (iRow << 4);
   }
   else {
      // Else check pile to use
      Check1 ((iCard >> 12) < (sizeof (piles) / sizeof (piles[0])));
      iPile = (iCard >> 8) & 0xff;
      Check1 (iPile < piles[iCard >> 12].children ().size ());

      Gtk::Box_Helpers::Child& child (piles[iCard >> 12].children ()[iPile]);
      pile = dynamic_cast<MachiPile*> (child.get_widget ());
   }
   Check3 (pile);

   // Send move
   if (getConnectionMgr ().getMode () != ConnectionMgr::NONE) {
      std::ostringstream msg;
      msg << "Play=" << hands[0][*pValue]->id () << ";Target="
          << (iPile << 16) + iCard + 100;
      if (getConnectionMgr ().getMode () == ConnectionMgr::CLIENT)
         ignoreNextMsg = true;
      broadcastMessage (msg.str ());
   }

   // End old drag
   context->drag_finish (true, false, time);

   // Unregister old card
   hands[0].remove (*pValue);
   unregisterHandDND (moved);

   // Insert card into pile and register it for DND
   iCard = pile->getPosition4Card (moved);
   Check3 (iCard <= pile->size ());
   pile->insert (moved, iCard);
   registerTableDND (moved, (iPile << 8) + iCard);
   if (iCard < (pile->size () - 1))
      registerTableDND (iPile, iCard + 1, pile->size () - 1);

   // Player has won, if he does not have any cards left
   if (hands[0].empty ()) {
      return;
   }

   // Re-register the cards in the hand of the human for DND
   if (*pValue < hands[0].size ())
      registerHandDND (*pValue, hands[0].size () - 1);
   Check3 (aDNDHand.size () == hands[0].size ());
}

//----------------------------------------------------------------------------
/// Finds the next player still having cards
/// \param player: Player to find next player to
/// \return unsigned int: Next player having cards
/// \remarks We assume (without really checking), that there's a next player.
//----------------------------------------------------------------------------
unsigned int Machiavelli::findNextPlayer (unsigned int player) const {
   for (unsigned int i (1); i < NUM_PLAYERS; ++i) {
      player = (player + 1) & 0x3;
      if (hands[player].size ()) {
         TRACE8 ("Machiavelli::findNextPlayer (unsigned int) const - Player: " << player);
         break;
      }
   }
   Check3 (player < NUM_PLAYERS);
   return player;
}
