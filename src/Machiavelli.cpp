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

#include "SigCExt.h"

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
     , target (-1U) {
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
   }
   hands[0].setStyle (ICardPile::COMPRESSED);
   hands[0].setShowOption (ICardPile::SHOWFACE);

   TRACE9 ("Machiavelli::Machiavelli (Box&, Statusbar&, CardSet&, const "
           "std::vector<Glib::ustring>&) - Attach widgets");
   attach (hands[0], 3, 12, 0, 1, Gtk::EXPAND, Gtk::SHRINK, 1, 5);
   attach (names[0], 3, 12, 1, 2, Gtk::EXPAND, Gtk::SHRINK, 1, 5);
   attach (staple, 0, 1, 0, 1, Gtk::SHRINK, Gtk::SHRINK, 5);
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
         (Gtk::TargetEntry ("icon/card/hand", GTK_TARGET_SAME_APP, HAND));
      dndTypeTable.push_back
         (Gtk::TargetEntry ("icon/card/table", GTK_TARGET_SAME_APP, TABLE));

      dndTypeBoth.push_back (dndTypeHand.front ());
      dndTypeBoth.push_back (dndTypeTable.front ());
   }

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
                     " them on the table - hit enter to end turn"));

      // Set random startplayer (if not already set)
      if (startPlayer == -1U)
         startPlayer = rand () & 0x3;
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

//----------------------------------------------------------------------------
/// Handles the messages the server might send for the twopart cardgame
/// \param player: ID of the player sending the message
/// \param message: Message received from the server
/// \returns bool: True, if message has been processed completey
//----------------------------------------------------------------------------
bool Machiavelli::handleMessage (unsigned int player, const char* message) {
   TRACE1 ("Machiavelli::handleMessage (unsigned int player, const char*) - "
           << message << " (" << player << ')');
    
   YGP::Tokenize command (message);
   std::string cmd (command.getNextNode ('='));

   bool rc (true);
   if (cmd == "Move") {
      YGP::AttributeParse ap;
      unsigned int card (-1U), dest (-1U), iPile (-1U);
      ATTRIBUTE (ap, unsigned int, card, "Move");
      ATTRIBUTE (ap, unsigned int, dest, "To");
      ATTRIBUTE (ap, unsigned int, iPile, "Pile");
      ap.assignValues (message);

      if (iPile >= tablePiles.size ())
         throw std::string ("Invalid pile!");
      ICardPile& pile (*tablePiles[iPile]);
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

   if (target == -1U) {
      target = showCardsToPlay (player);
      TRACE8 ("Machiavelli::makeMove (unsigned int) - Going to play cards to "
              << std::hex << (int)target << std::dec);

      // No more cards found: Continue with next player
      if (target == -1U) {
         player = findNextPlayer (player);
         displayTurn (player);

         dealCard (player);
      }
      else {
         Check3 (pos1Play <= pos2Play);
         Check3 (pos2Play < hands[player].size ());
         flipCards2Play (hands[player], pos1Play, pos2Play);
      }
   }
   else {
      Check1 (pos1Play <= pos2Play);

      unsigned int pos (target & 0xffff);
      target >>= 16;

      TRACE4 ("Machiavelli::makeMove (unsigned int) - Moving cards to pile " << target);
      Check3 (target < tablePiles.size ());

#if CHECK > 2
      for (unsigned int t (pos1Play); t < pos2Play; ++t) {
         int diff (MachiPile::cardDistance (*hands[player][t + 1], *hands[player][t]));
         TRACE1 ("Buraco::makeMove (unsigned int) - Card " << *hands[player][t]);
         Check3 ((diff == 0) || (diff == 1));
      }
#endif

      // Move the played cards to the pile to play
      Check3 (hands[player].size () > pos2Play);
      for (; (int)pos1Play <= (int)pos2Play; --pos2Play)
         tablePiles[target]->insert (hands[player].remove (pos1Play), pos++);
      target = -1U;

      // No more cards found: Continue with next player
      if (hands[player].empty ()) {
         unsigned int next (findNextPlayer (player));
         if (next == player) {
            status.pop ();
            Glib::ustring stat (_("%1 lost"));
            stat.replace (stat.find ("%1"), 2, actPlayers[player]->getName ());
            status.push (stat);
            setGameStatus (STOPPED);
            return -1;
         }
         displayTurn (player = next);
         dealCard (player);
      }
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
                              (slot (*this, (&Machiavelli::stapleSelected))));

   Check3 (hands[0].size ());
   for (unsigned int i (0); i < hands[0].size (); ++i)
      registerHandDND (i);
   Check3 (aDNDHand.size () == hands[0].size ());

   newPile.drag_dest_set (dndTypeBoth, Gtk::DEST_DEFAULT_ALL, Gdk::ACTION_MOVE);
   aDNDTable[NULL].connReceive = newPile.signal_drag_data_received ().connect
       (bind (slot (*this, &Machiavelli::cardDroppedOnTable), -1U));

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
/// Returns the passed pile of the player
/// \param player: Number of player
/// \param pile: ID of the pile to return
//----------------------------------------------------------------------------
ICardPile& Machiavelli::getPileOfPlayer (unsigned int player, unsigned int pile) {
   Check1 (player < NUM_PLAYERS);
   Check1 ((pile < 4) || (pile >= 100));

   return *tablePiles[pile];
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
/// Callback after clicking on the staple
//-----------------------------------------------------------------------------
void Machiavelli::stapleSelected () {
   TRACE5 ("Machiavelli::stapleSelected ()");
   Check1 (gameStatus () == PLAYING);
   Check3 (staple.size ()); Check3 (activeCards.size ());

   if (!checkPiles ())
      return;

   if (getConnectionMgr ().getMode () != YGP::ConnectionMgr::NONE) {
      // Send played card to all clients (if any)
      std::ostringstream msg;
      msg << "Play=" << staple.getTopCard ().id () << ";Target=1";

      if (getConnectionMgr ().getMode () == YGP::ConnectionMgr::CLIENT)
         ignoreNextMsg = true;
      broadcastMessage (msg.str ());
   }

   disableHuman ();

   unsigned int nextPlayer (findNextPlayer (currentPlayer ()));
   setNextPlayer (nextPlayer);
   displayTurn (nextPlayer);
   dealCard (nextPlayer);
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
   card.drag_dest_set (dndTypeHand, Gtk::DEST_DEFAULT_ALL, Gdk::ACTION_MOVE);
   card.drag_source_set
      (dndTypeHand, Gdk::ModifierType (GDK_BUTTON2_MASK | GDK_BUTTON3_MASK),
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
           << " = " << std::hex << nr << " - " << &card << std::dec);

   // Card accepts drops from hand and drags from table
   card.drag_dest_set (dndTypeBoth, Gtk::DEST_DEFAULT_ALL, Gdk::ACTION_MOVE);
   card.drag_source_set
       (dndTypeTable, Gdk::ModifierType (GDK_BUTTON2_MASK | GDK_BUTTON3_MASK),
        Gdk::ACTION_MOVE);
   card.drag_source_set_icon (card.getImage ());

   aDNDTable[&card].connReceive = card.signal_drag_data_received ().connect
      (bind (slot (*this, &Machiavelli::cardDroppedOnTable), nr));
   aDNDTable[&card].connGet = card.signal_drag_data_get ().connect
      (bind (slot (*this, &Machiavelli::getDropData), nr));
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
/// \param pData: Describes the thing which was dropped
/// \param time: Timestamp of the drag
/// \param card: Number of card where something was dropped at
/// \pre \c pContext, \c pData not NULL; Expects \c info to be 0
//-----------------------------------------------------------------------------
void Machiavelli::cardDropped (const Glib::RefPtr<Gdk::DragContext>& context,
                               gint, gint, GtkSelectionData* pData, guint,
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
                               GtkSelectionData* pData, guint, guint32 time,
                               unsigned int cardPos) {
   Check1 (pData);
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
                                      guint info, guint32 time, unsigned int iCard) {
   TRACE1 ("Machiavelli::cardDroppedOnTable (...) - Card dropped on " << std::hex
           << (int)iCard << std::dec << "; " << info);
   Check1 (pData);
   Check1 (!context->get_is_source ());
   Check1 (pData->length == sizeof (int));
   Check1 (pData->format == 8);
   Check1 ((info == HAND) || (info == TABLE));

   unsigned int* pValue (reinterpret_cast <unsigned int*> (pData->data));
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
   unsigned int nr ((info == HAND) ? 1 : (src.size () - off));
   TRACE4 ("Machiavelli::cardDroppedOnTable (...) - Card dropped: " << *moved);

   if (iCard == -1U) {    // If card was dropped on the new label: Create pile
      iPile = tablePiles.size ();
      pile = &makeNewPile ();
      iCard = 0;
   }
   else {
      // Else check pile to use
      iPile = iCard >> 8;
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
   }
   Check3 (pile);

   // End old drag
   context->drag_finish (true, false, time);

   while (nr--) {
      TRACE9 ("Machiavelli::cardDroppedOnTable (...) - Insert to: " << iPile
              << "; Pos: " << iCard);
      Check3 (iCard != -1U);

      // Send move
      if (getConnectionMgr ().getMode () != YGP::ConnectionMgr::NONE) {
         std::ostringstream msg;
         msg << "Play=" << src[off]->id () << ";Target="
             << (iPile << 16) + iCard + 100;
         if (getConnectionMgr ().getMode () == YGP::ConnectionMgr::CLIENT)
             ignoreNextMsg = true;
         broadcastMessage (msg.str ());
      }

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
      iCard = pile->getPosition4Card (*moved);
      if (iCard == -1U)
         break;
   }

   // Pile moved completely?
   if ((info == TABLE) && src.empty ()) {
      std::vector<MachiPile*>::iterator i (tablePiles.begin () + nrpile);
      Check3 (*i == &src);
      tablePiles.erase (i);
      delete &src;

      if (nrpile > iPile)
         nrpile = iPile;

      // Re-register the following piles
      while (nrpile < tablePiles.size ()) {
         registerTableDND (nrpile, 0, tablePiles[nrpile]->size () - 1);
         ++nrpile;
      }
   }

   // Re-register the cards in the hand of the human for DND
   if ((info ==HAND) && *pValue < hands[0].size ())
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

//-----------------------------------------------------------------------------
/// Makes a new pile.
/// \returns MachiPile&: New created pile
//-----------------------------------------------------------------------------
MachiPile& Machiavelli::makeNewPile () {
   TRACE9 ("Machi::makeNewPile ()");

   MachiPile* pile (new MachiPile ());
   pile->show ();
   piles.add (*pile);
   tablePiles.push_back (pile);
   return *pile;
}

//-----------------------------------------------------------------------------
/// Searches for cards to play and shows them in the hand of the actual player
/// \param player: Player to inspect
/// \returns \c ID of the target (32 Bit: Pile << 16 + Position) or -1U
//-----------------------------------------------------------------------------
unsigned int Machiavelli::showCardsToPlay (unsigned int player) {
   TRACE2 ("Machiavelli::showCardsToPlay (unsigned int) - Player " << player);

   ICardPile& playerPile (hands[player]);
   unsigned int dest (-1U);

   // Check if any card can be added to an existing pile
   if (tablePiles.size ())
      for (ICardPile::const_iterator p (playerPile.begin ());
           p != playerPile.end (); ++p) {
         TRACE8 ("Machiavelli::showCardsToPlay (player) - Adding card " << **p << '?');

         for (std::vector<MachiPile*>::const_iterator m (tablePiles.begin ());
              m != tablePiles.end (); ++m) {
            dest = (*m)->getPosition4Card (**p);
            if (dest != -1U) {
                pos1Play = pos2Play = p - playerPile.begin ();
                return ((m - tablePiles.begin ()) << 16) + dest;
            }
         }
      }

   // Check for 3 cards belonging to a serie
   unsigned int i (0);
   for (; i < playerPile.size (); ++i) {
      TRACE8 ("Machiavelli::showCardsToPlay (player) - Analyzing card " << *playerPile[i]);

      std::map<unsigned int, unsigned int> aPos;                   // diff, pos
      std::vector<unsigned int> aOrder;
      unsigned int nrs (playerPile.getSeries (*playerPile[i], aPos, aOrder,
                                              &MachiPile::cardDistance, false));

      // Play the bigger of the found matching cards, if there are >= 3
      if ((nrs > aPos.size ()) ? (nrs > 2) : (aPos.size () > 2)) {
         if (nrs < aPos.size ()) {
            i = playerPile.sortColourSerie (aPos, aOrder);
            nrs = aPos.size ();
         }

         makeNewPile ();
         pos1Play = i;
         pos2Play = i + nrs - 1;
         return (tablePiles.size () - 1) << 16;
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
      player ? hands[player].insertSorted (card) : hands[0].append (card);
   }
}

//----------------------------------------------------------------------------
/// Checks, if all the piles on the table are valid
/// \return bool: True, if piles are valid 
//----------------------------------------------------------------------------
bool Machiavelli::checkPiles () const {
   YGP::StatusObject obj;

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

   // Show error, if any
   if (obj.getType () != YGP::StatusObject::UNDEFINED) {
      obj.abstract (_("The piles are not valid!"));
      XGP::MessageDlg dlg (obj);
      dlg.set_title (_("Invalid move"));
      dlg.run ();
      return false;
   }
   return true;
}
