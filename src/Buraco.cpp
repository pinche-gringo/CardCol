//$Id$

//PROJECT     : Cardgames
//SUBSYSTEM   : Burazno
//REFERENCES  :
//TODO        : - Disconnect get_drop_data signal
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


#include <gtk/gtkdnd.h>

#include <gtkmm/statusbar.h>

#define CHECK 9
#define TRACELEVEL 9
#include <Check.h>
#include <Trace_.h>
#include <ScoreDlg.h>
#include "Burazno.h"
#include "SigCExt.h"
std::vector<Gtk::TargetEntry> Buraco::dndType;

std::vector<Gtk::TargetEntry> Burazno::dndType;
unsigned int Buraco::ENDPOINTS (2000);

/*--------------------------------------------------------------------------*/
//Purpose   : Constructor
//Parameters: parent: Parent widget to display the game in
//            statusbar: Status bar widget to display information about the game
//            cardset: Cardset to use
//            names: Vector of player-names
/*--------------------------------------------------------------------------*/
Burazno::Burazno (Gtk::Box& parent, Gtk::Statusbar& statusbar,
                  CardSet& cardset, const std::vector<std::string>& names)
   : Game (parent, statusbar, cardset, names, 3, 3), startPlayer (0)
     , staple (ICardPile::SHOWBACK), dumped (ICardPile::SHOWFACE)
     , newPile (_("New pile")), startTurn (true) {
   TRACE9 ("Burazno::Burazno (Box&, Statusbar&, CardSet&, const "
           "std::vector<std::string>&)");
       scrlTable[i] = new Gtk::ScrolledWindow ();

   TRACE9 ("Buraco::Buraco (Box&, Statusbar&, CardSet&, const "
           "std::vector<Glib::ustring>&) - Init common staples");
   TRACE9 ("Burazno::Burazno (Box&, Statusbar&, CardSet&, const "
           "std::vector<std::string>&) - Init common staples");

   boxTeam[0].pack_end (newPile, Gtk::PACK_EXPAND_WIDGET, 5);
   boxTeam[0].set_size_request (-1, height + 5 * 15);
   TRACE9 ("Burazno::Burazno (Box&, Statusbar&, CardSet&, const "
           "std::vector<std::string>&) - Init cards in hand");
   handHuman.setStyle (ICardPile::COMPRESSED);
   handHuman.setShowOption (ICardPile::SHOWFACE);

   boxTeam[0].pack_end (newPile, Gtk::PACK_EXPAND_WIDGET, 5);
           "std::vector<Glib::ustring>&) - Attach widgets");
   TRACE9 ("Burazno::Burazno (Box&, Statusbar&, CardSet&, const "
           "std::vector<std::string>&) - Attach widgets");
   attach (handHuman, 3, 10, 0, 1, Gtk::EXPAND, Gtk::SHRINK, 1);
   attach (*scrlTable[0], 0, 10, 2, 3, Gtk::EXPAND | Gtk::FILL,
           Gtk::EXPAND | Gtk::FILL, 0, 5);
   attach (boxTeam[0], 0, 10, 1, 2);
   attach (boxTeam[1], 0, 10, 2, 3);
           "std::vector<Glib::ustring>&) - Show widgets");
   TRACE9 ("Burazno::Burazno (Box&, Statusbar&, CardSet&, const "
           "std::vector<std::string>&) - Show widgets");
   dumped.show ();
   handHuman.show ();
   boxTeam[0].show ();
   boxTeam[1].show ();

   if (dndType.empty ())
      dndType.push_back
         (Gtk::TargetEntry ("icon/card", Gtk::TARGET_SAME_APP, 0));

         (Gtk::TargetEntry ("icon/card", GTK_TARGET_SAME_APP, 0));
//-----------------------------------------------------------------------------
/// Destructor
/*--------------------------------------------------------------------------*/
//Purpose   : Destructor
/*--------------------------------------------------------------------------*/
Burazno::~Burazno () {
   TRACE9 ("Burazno::~Burazno ()");

//-----------------------------------------------------------------------------
/// Removes a cerrado from the table
/*--------------------------------------------------------------------------*/
//Purpose   : Makes the move for the next player.
//Parameters: player: Actual player
//Returns   : int: Next player or -1 if end of game
/*--------------------------------------------------------------------------*/
int Burazno::makeMove (unsigned int player) {
   TRACE5 ("Burazno::makeMove (unsigned int) - Turn of player " << player);

   Check3 (player);
   cleanCerrado (player);
   // First cleanup buraznos made in the last turn
   for (std::vector<CardVPile*>::iterator p (tablePiles[player & 1].begin ());
        p != tablePiles[player & 1].end (); ++p) {
      Check3 (*p); Check3 ((*p)->size () <= 7);
      if ((*p)->size () == 7)
         removeBurazno (player & 1, **p);
   }
      target = showCardsToPlay (player);
   if (startTurn) {
      ICardPile& playerPile (hands[player - 1]);
          ? (isJoker (dumpedCard)
      if (dumped.size ()) {
         // Check if there are equal cards as the last dumped one
         int start (playerPile.find (dumped.getTopCard ().number ()));
         int end ((start == -1) ? -1 : playerPile.findLastEqual (start));
         Check3 ((start != -1) ? (start <= end) : (start == end));
         Check3 ((end == -1) || (end < playerPile.size ()));

         if ((!isJoker (dumped.getTopCard ())) && ((end - start) >= 1)) {
            CardVPile& pile (makeNewPile (player & 1));      // Create new pile
            pile.setTopCard (dumped.removeTopCard ());   // with picked up card
            movePile (pile, playerPile, start, end);

            if (dumped.size ())
               movePile (playerPile, dumped);
            playerPile.sortByNumber ();
         }
         else
            playerPile.insertSorted (staple.removeTopCard ());
         if (getConnectionMgr ().getMode () != YGP::ConnectionMgr::NONE) {
   if (target == -1U)
   startTurn = false;
      target = executeMove (player);
   return executeMove (player);
//-----------------------------------------------------------------------------
/// Executes a move for the passed player; only one move is made at every
/*--------------------------------------------------------------------------*/
//Purpose   : Executes a move for the passed player; only one move is made at
//            every timer-iteration
//Parameters: player: Actual player
//Returns   : int: Next player or -1 if end of game
/*--------------------------------------------------------------------------*/
int Burazno::executeMove (unsigned int player) {
   TRACE8 ("Burazno::executeMove (player) - Checking for 3 in a row");

   // Check for 3 cards having the same number
   ICardPile& playerPile (hands[player - 1]);
   unsigned int count (1);
   for (unsigned int i (1); i < playerPile.size () - 1; ++i) {
      Check3 (playerPile[i]);

      // Check if the actual card can be added to an existing pile
      if (tablePiles[player & 1].size () && cardFitsOnPlayedPile (player, i))
         return player;

      // Check if there are 3 (or more) of a kind
      if (playerPile[i]->number () == playerPile[i - 1]->number ())
         ++count;
      else {
         if (count >= 3) {
            CardVPile& pile (makeNewPile (player & 1));    // Create new pile with
            movePile (pile, playerPile, i - count, i - 1);
            return player;
         }
         count = 1;

   // Check if all cards in the hand can (and should) be played
   TRACE8 ("Buraco::executeMove (unsigned int) - Playing all?");
   for (i = 0; i < playerPile.size () - 1; ++i) {
   TRACE8 ("Burazno::executeMove (unsigned int) - Searching for a card to dump");
   for (unsigned int i (0); i < playerPile.size () - 1; ++i) {
      dumped.setTopCard (playerPile.remove (0)); // TODO
      break;
   }
   pos1Play = pos2Play = i;
   startTurn = true;
   return (player + 1) & 0x3;
//-----------------------------------------------------------------------------
/// Starts the game by dealing the cards
/*--------------------------------------------------------------------------*/
//Purpose   : Starts the game by dealing the cards
/*--------------------------------------------------------------------------*/
void Burazno::start () {
   TRACE9 ("Burazno::start ()");
   if (pScoreDlg) {
      unsigned int player;
   randomizeCardsToPile (staple);
            reserve[(i - posServer) & 1].push_back (&staple.removeTopCard ());
   for (unsigned int j (0); j < 13; ++j) {
      for (unsigned int i (0); i < NUM_PLAYERS - 1; ++i)
         hands[i].insertSorted (staple.removeTopCard ());
      handHuman.setTopCard (staple.removeTopCard ());
      gStatus.team1Buraco = gStatus.team2Buraco = 0x3;
      for (unsigned int i (0); i < (sizeof (reserve) / sizeof (reserve[0])); ++i)
         reserve[i].push_back (&staple.removeTopCard ());
   if (startPlayer)
   handHuman.sortByNumber ();

   dumped.append (staple.removeTopCard ());

   status.pop ();
   status.push (_("You can sort the cards in your hand with drag and drop or put"
                  " them on the table - click card to dump to end turn"));

   setNextPlayer (startPlayer);
   displayTurn (startPlayer++);
   startPlayer &= 0x3;
   makeNextMoves ();
//-----------------------------------------------------------------------------
/// Remove cards from everything which can hold them
/*--------------------------------------------------------------------------*/
//Purpose   : Remove cards from everything which can hold them
/*--------------------------------------------------------------------------*/
void Burazno::clean () {
   TRACE9 ("Burazno::clean ()");
      hands[i].clear ();
   handHuman.clear ();
   for (unsigned int i (0); i < NUM_PLAYERS - 1; ++i)
   staple.clear ();
   dumped.clear ();

   for (unsigned int i (0); i < (NUM_PLAYERS >> 1); ++i) {
      for (std::vector<BuracoPile*>::iterator p (tablePiles[i].begin ());
           p != tablePiles[i].end (); ++p) {
      for (std::vector<CardVPile*>::iterator p (tablePiles[i].begin ());
           p != tablePiles[i].end (); ++p)
      tablePiles[i].clear ();

   for (unsigned int i (0); i < (sizeof (reserve) / sizeof (reserve[0])); ++i)
      reserve[i].clear ();

//-----------------------------------------------------------------------------
/// Enables the cards the human can pick up.
/*--------------------------------------------------------------------------*/
//Purpose   : Enables the cards of the human player
//Returns   : 0
//Remarks   : Depending of the status of the game (PLAYING2) also the top
//            card of the played pile is enabled
/*--------------------------------------------------------------------------*/
bool Burazno::enableHuman () {

   Check3 (hands[0].size ());
   for (unsigned int i (0); i < hands[0].size (); ++i) {
   Check3 (handHuman.size ());
   for (unsigned int i (0); i < handHuman.size (); ++i)
   }
   Check3 (aDNDHand.size () == handHuman.size ());
           << " cards");
   TRACE2 ("Burazno::enableHuman () - Human has " << handHuman.size ()
   newPile.drag_dest_set (dndType, Gtk::DEST_DEFAULT_ALL, Gdk::ACTION_MOVE);
   aDNDTable[NULL] = newPile.signal_drag_data_received ().connect
   for (unsigned int i (0); i < handHuman.size (); ++i)
      enableCard (i);

   Check3 (staple.size ());
   Check3 (dumped.size ());
   stapleTop = staple.signal_clicked ().connect
      (slot (*this, (&Burazno::stapleSelected)));
   dumpedTop = dumped.signal_clicked ().connect
      (slot (*this, (&Burazno::dumpedSelected)));

      (bind (mem_fun (*this, &Buraco::cardDroppedOnTable), -1U));

      (bind (slot (*this, &Burazno::cardDroppedOnTable), -1U));
      Check3 (tablePiles[0][i]);
      for (unsigned int j (0); j < tablePiles[0][i]->size (); ++j)
         registerTableDND (*(*tablePiles[0][i])[j], (i << 8) + j);
   }

   menuSort->set_sensitive ();

   return Game::enableHuman ();
//-----------------------------------------------------------------------------
/// Disables the cards the human player can select
/*--------------------------------------------------------------------------*/
//Purpose   : Disables the cards the human player can select
/*--------------------------------------------------------------------------*/
void Burazno::disableHuman () {
   TRACE2 ("Buarzno::disableHuman () - DND: " << aDNDHand.size () << "; "
   menuSort->set_sensitive (false);


   if (aDNDHand.size ())
      for (unsigned int i (0); i < hands[0].size (); ++i)
         unregisterHandDND (*hands[0][i]);
      for (unsigned int i (0); i < handHuman.size (); ++i)
         unregisterHandDND (*handHuman[i]);
   if (aDNDTable.size ()) {
      for (unsigned int i (0); i < tablePiles[0].size (); ++i) {
   for (unsigned int i (0); i < tablePiles[0].size (); ++i) {
      Check3 (tablePiles[0][i]);
      for (unsigned int j (0); j < tablePiles[0][i]->size (); ++j)
         unregisterTableDND (*(*tablePiles[0][i])[j]);

   aDNDTable[NULL].disconnect ();
   aDNDTable.erase (NULL);
   if (dumpedTop.connected ())
//-----------------------------------------------------------------------------
/// Callback after clicking on a card in the hand
/*--------------------------------------------------------------------------*/
//Purpose   : Callback after clicking on a card in hand
//Parameters: iCard: Offset of card in hand
/*--------------------------------------------------------------------------*/
void Burazno::cardSelected (unsigned int iCard) {
   TRACE5 ("Burazno::cardSelected (unsigned int) - Position " << iCard);
   Check1 (iCard < handHuman.size ());
   // Check if all piles are valid
   if (!humanPilesOK ()) {
   unregisterHandDND (*handHuman[iCard]);
   dumped.append (handHuman.remove (iCard));
   // reserve
   // If the player has no more cards left (except of joker): Give him the reserve
   if (reserve[0].size () && containsOnlyJoker (handHuman))
      addReserve (handHuman, 0);
   gStatus.startGame = 0;
   makeNextMoves ();

//-----------------------------------------------------------------------------
/// Callback after clicking on the staple
/*--------------------------------------------------------------------------*/
//Purpose   : Callback after clicking on the staple
/*--------------------------------------------------------------------------*/
void Burazno::stapleSelected () {
   TRACE5 ("Burazno::stapleSelected ()");
   Check2 (dumped.size ());
   Check3 (stapleTop.connected ()); Check3 (dumpedTop.connected ());

   // Move top card to human and enable the cards in his hand, when idle
   // (means: *after* this signalhandler termintes)
   handHuman.append (staple.removeTopCard ());
   enableCard (handHuman.size () - 1);
   registerHandDND (handHuman.size () - 1);
//-----------------------------------------------------------------------------
/// Callback after clicking on the dumped staple
/*--------------------------------------------------------------------------*/
//Purpose   : Callback after clicking on the dumped staple
/*--------------------------------------------------------------------------*/
void Burazno::dumpedSelected () {
   TRACE5 ("Burazno::dumpedSelected ()");
   Check3 (stapleTop.connected ()); Check3 (dumpedTop.connected ());

   dumpedTop.disconnect ();
   stapleTop.disconnect ();

   while (dumped.size ()) {
      handHuman.append (dumped.removeTopCard ());
      enableCard (handHuman.size () - 1);
      registerHandDND (handHuman.size () - 1);
   }
//-----------------------------------------------------------------------------
/// Enables a card in the hand of the player
/*--------------------------------------------------------------------------*/
//Purpose   : Enables a card in the hand of the player
/*--------------------------------------------------------------------------*/
void Burazno::enableCard (unsigned int pos) {
   TRACE9 ("Burazno::enableCard (unsigned int) - Enabling card " << pos);
   Check1 (pos < handHuman.size ());
      (hands[0][pos]->signal_clicked ().connect
       (bind (mem_fun (*this, (&Buraco::cardSelected)), pos)));
      (handHuman[pos]->signal_clicked ().connect
       (bind (slot (*this, (&Burazno::cardSelected)), pos)));
//-----------------------------------------------------------------------------
/// Prepares the card for drag´n´drop
/*--------------------------------------------------------------------------*/
//Purpose   : Prepares the card for drag´n´drop
//Parameters: iCard: Number of card in hand
/*--------------------------------------------------------------------------*/
void Burazno::registerHandDND (unsigned int iCard) {
   Check1 (iCard < handHuman.size ());
   TRACE9 ("Burazno::registerHandDND (unsigned int) - Card: " << iCard << " ("
           << *handHuman[iCard] << " = " << handHuman[iCard] << ')');
   Check3 (aDNDHand.find (&card) == aDNDHand.end ());
   CardWidget& card (*handHuman[iCard]);
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
      (bind (slot (*this, &Burazno::cardDropped), iCard));
   card.signal_drag_data_get ().connect
      (bind (slot (*this, &Burazno::getDropData), iCard));
//-----------------------------------------------------------------------------
/// Stops the drag´n´drop abilities of the passed card
/*--------------------------------------------------------------------------*/
//Purpose   : Stops the drag´n´drop abilities of the passed card
//Parameters: card: Card to unregister of dnd
/*--------------------------------------------------------------------------*/
void Burazno::unregisterHandDND (CardWidget& card) {
   TRACE9 ("Burazno::unregisterHandDND (CardWidget&) - Card: " << card
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
void Burazno::registerTableDND (unsigned int pile, unsigned int start, unsigned int end) {
   TRACE9 ("Burazno::registerTableDND (unsigned int, unsigned int, unsigned int)"
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
void Burazno::registerTableDND (CardWidget& card, unsigned int nr) {
   TRACE9 ("Burazno::registerTableDND (CardWidget&, unsigned int) - " << card
   // Card accepts drops from hand and drags from table
   card.drag_dest_set (dndType, Gtk::DEST_DEFAULT_ALL, Gdk::ACTION_MOVE);
   aDNDTable[&card] = card.signal_drag_data_received ().connect
      (bind (mem_fun (*this, &Buraco::cardDroppedOnTable), nr));
}
      (bind (slot (*this, &Burazno::cardDroppedOnTable), nr));
//-----------------------------------------------------------------------------
/// Stops the drag´n´drop abilities of the passed card
/*--------------------------------------------------------------------------*/
//Purpose   : Stops the drag´n´drop abilities of the passed card
//Parameters: card: Card to de-register
/*--------------------------------------------------------------------------*/
void Burazno::unregisterTableDND (CardWidget& card) {
   TRACE9 ("Burazno::unregisterTableDND (unsigned int) - Card: " << card

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
void Burazno::cardDropped (const Glib::RefPtr<Gdk::DragContext>& context,
                           gint, gint, GtkSelectionData* pData, guint info,
                           guint32 time, unsigned int card) {
   Check3 (pData);
   Check3 (data.get_format () == 8);
   Check3 (pData->length == sizeof (int));
   Check3 (pData->format == 8);
   Check3 (card < handHuman.size ());
                         (const_cast<guint8*> (data.get_data ())));
   unsigned int* pValue (reinterpret_cast <unsigned int*> (pData->data));
   TRACE1 ("Buraco::cardDropped (...) - Inserting card " << *pValue
   Check3 (*pValue < handHuman.size ());
   TRACE1 ("Burazno::cardDropped (...) - Inserting card " << *pValue
   if (acceptCards != -1U) {
      context->drag_finish (false, false, time);
   CardWidget& cardMoved (hands[0].remove (*pValue));
   hands[0].insert (cardMoved, card);                     // Insert moved card
   CardWidget& cardTarget (*handHuman[card]);
   CardWidget& cardMoved (handHuman.remove (*pValue));
   if (*pValue > card)
      ++card;

   handHuman.insert (cardMoved, card);                     // Insert moved card
   if (*pValue < card) {
      unsigned int temp (card);
      card = *pValue;
      *pValue = temp;
   }

   Glib::signal_idle ().connect
   registerHandDND (card, *pValue);
   Check3 (aDNDHand.size () == handHuman.size ());
//-----------------------------------------------------------------------------
/// Checks if the piles on the table are valid (have at least 3 cards)
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
void Burazno::cardDroppedOnTable (const Glib::RefPtr<Gdk::DragContext>& context,
                                  gint, gint, GtkSelectionData* pData,
                                  guint, guint32 time, unsigned int iCard) {
   TRACE1 ("Burazno::cardDroppedOnTable (...) - Card dropped on " << std::hex
   Check3 (data.get_length () == sizeof (int));
   Check3 (pData);
   Check3 (data.get_format () == 8);
   Check3 (pData->length == sizeof (int));
   Check3 (pData->format == 8);
                         (const_cast<guint8*> (data.get_data ())));
   unsigned int* pValue (reinterpret_cast <unsigned int*> (pData->data));
           << " in pile");
   Check3 (*pValue < handHuman.size ());
   TRACE1 ("Burazno::cardDroppedOnTable (...) - Inserting card " << *pValue

   if (!humanPilesOK (iCard >> 8)) {
   // End old DND
   context->drag_finish (true, false, time);
   activeCards[*pValue].disconnect ();
   activeCards.erase (activeCards.begin () + *pValue);

   // Move dropped card to a (new) pile on the table
   unsigned int iPile;
   CardVPile* pile (NULL);
   CardWidget& moved (handHuman.remove (*pValue));
   unregisterHandDND (moved);
   TRACE4 ("Burazno::cardDroppedOnTable (...) - Card dropped: " << moved);
       && isJoker (moved) || (*pValue >= acceptCards)) {
   if (iCard == -1U) {    // If card was dropped on the new label: Create pile
      iCard = 0;
   }
   else {
      // Else check pile to use
      Check1 ((iCard >> 8) < tablePiles[0].size ());
      // Else find pile to use
      Check1 ((iCard >> 8) <= tablePiles[0].size ());
      pile = tablePiles[0][iPile = (iCard++ >> 8)];
      iCard &= 0xff;
   // End old drag
   pile->getPosition4Card (moved, iCard, move);
   if (iCard < (pile->size () - 1))
      registerTableDND (iPile, iCard + 1, pile->size () - 1);

   TRACE9 ("Buraco::cardDroppedOnTable (...) - Undo:  " << iPile << "; " << iCard
	   << "; " << *pValue << ": " << acceptCards);
      removeCerrado (0, *pile);

      removeBurazno (0, *pile);
   // disabled, if the human picked up the dumped pile.
      if (!reserve[0].empty ()) {
   if (reserve[0].size () && containsOnlyJoker (handHuman))
      addReserve (handHuman, 0);
   if (*pValue < hands[0].size ())
  // Re-register the cards in the hand for DND
   if (*pValue < handHuman.size ())
      registerHandDND (*pValue, handHuman.size () - 1);
   Check3 (aDNDHand.size () == handHuman.size ());
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
void Burazno::getDropData (const Glib::RefPtr<Gdk::DragContext>& pContext,
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
void Burazno::registerHandDND (unsigned int start, unsigned int end) {
   TRACE5 ("Burazno::registerHandDND (unsigned int, unsigned int) - [" << start
   Check1 (end < hands[0].size ());
   Check1 (end < activeCards.size ());
   Check1 (end < handHuman.size ());
   for (; start <= end; ++start) {
      TRACE9 ("Buraco::registerHandDND (unsigned int, unsigned int) - Handling card " << start);

         (bind (mem_fun (*this, (&Buraco::cardSelected)), start));
      activeCards[start] = handHuman[start]->signal_clicked ().connect
         (bind (slot (*this, (&Burazno::cardSelected)), start));
      registerHandDND (start);
      unregisterHandDND (*handHuman[start]);
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
bool Burazno::containsOnlyJoker (std::vector<CardWidget*>& pile) const {
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
bool Burazno::containsNoJoker (std::vector<CardWidget*>& pile) const {
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
//Parameters: pile: Pile add reserve to
//            player: Player getting the reserve
/*--------------------------------------------------------------------------*/
void Burazno::addReserve (ICardPile& pile, unsigned int player) {
   undo.pickUp = 1;
   undo.cJokers = hands[player].size ();
   ICardPile& target (player ? hands[player - 1] : handHuman);

      Game::disableHuman ();
      for (unsigned int i (0); i < hands[0].size (); ++i)
         unregisterHandDND (*hands[0][i]);
      for (unsigned int i (0); i < handHuman.size (); ++i)
         unregisterHandDND (*handHuman[i]);
   // Add reserve
   sort (reserve[player & 1].begin (), reserve[player & 1].end (),
	 compByNumberWithJokers);
   pile.setTopCards (reserve[player & 1]);
   if (!player)
   pile.sortByNumber ();
      for (unsigned int i (0); i < hands[player].size (); ++i) {
         enableCard (i);
      for (unsigned int i (0); i < pile.size (); ++i)
         registerHandDND (0, pile.size () - 1);
//-----------------------------------------------------------------------------
/// Hides the joker, which are displayed when picking up the buraco
/*--------------------------------------------------------------------------*/
//Purpose   : Checks, if the passed pile contains no cards except jokers or 2s.
//            This is also true for empty piles.
//Parameters: pile: Pile add reserve to
//            team: Which reserve to use
/*--------------------------------------------------------------------------*/
CardVPile& Burazno::makeNewPile (unsigned int team) {
   TRACE9 ("Burazno::makeNewPile (unsigned int) - New pile for team " << team);

   Check1 ((sizeof (tablePiles) / sizeof (tablePiles[0]))
            == (sizeof (boxTeam) / sizeof (boxTeam[0])));
   tablePiles[team].push_back (pile);
   static int width (staple.getTopCard ().getImageWidth ());
   static int height (staple.getTopCard ().getImageHeight ());

   CardVPile* pile (new CardVPile (ICardPile::COMPRESSED, ICardPile::SHOWFACE));

   pile->show ();
   return *pile;
}
   pile->set_size_request (width, height + 6 * 15);

//-----------------------------------------------------------------------------
/// Checks if the passed card can be put on one of the existing piles
/*--------------------------------------------------------------------------*/
//Purpose   : Checks if the passed card can be put on one of the existing piles
//Parameters: player: Player to inspect
//            iCard: Card to inspect
//Returns   : True, if card could be played
//Remarks   : This method actually moves the card
/*--------------------------------------------------------------------------*/
bool Burazno::cardFitsOnPlayedPile (unsigned int player, unsigned int iCard) {
   TRACE9 ("Burazno::cardFitsOnPlayedPile (unsigned int, unsigned int) - "
   Check1 (player < NUM_PLAYERS);
   Check1 (iCard < hands[player].size ());
   CardWidget& card (*hands[player][iCard]);
   Check1 (iCard < hands[player - 1].size ());
   CardWidget& card (*hands[player - 1][iCard]);
   TRACE3 ("Burazno::cardFitsOnPlayedPile (unsigned int, unsigned int) - Card " << card);
   unsigned int maxPoints (0);
   for (std::vector<CardVPile*>::iterator p (tablePiles[player & 1].begin ());
              "Checking pile " << (int)(p - tablePiles[player & 1].begin ()));
      TRACE5 ("Burazno::cardFitsOnPlayedPile (unsigned int, unsigned int) - "
      if ((*p)->size () == 7) {                         // Skip finished piles
         Check3 (!(*p)->is_visible ());

      // one having picked up the reserve already played (the missing card
      // Play joker, if you can make a burazno (7 in a row)
      if (isJoker (card)
          && ((*p)->size () == 6) && containsNoJoker (**p)) {
         playCardOnPile (**p, player, iCard, (*p)->size ());
         return true;

      else {
         unsigned int type (0);
         CardVPile::const_iterator i ((*p)->begin ());
         if (isJoker (**i)) ++i;
         CardWidget* first (*i++); Check3 (first);
         CardWidget* second (isJoker (**i) ? *(i + 1) : *i);
         Check3 (second); Check3 (i != (*p)->end ());
         TRACE5 ("Burazno::cardFitsOnPlayedPile (unsigned int, unsigned int) - "
                 "Cards: " << *first << " and " << *second);

         // Then check if the pile is a numberd or a coloured one
         if (first->number () == second->number ()) {
            if (first->number () == card.number ()) {
               playCardOnPile (**p, player, iCard, (*p)->size ());
               return true;
            }
         }
         else
            if (first->color () == card.color ()) {
               Check3 (second->color () == card.color ());
               // TODO: Check for joker in pile
               if (card.number () == first->number () - 1) {
                  playCardOnPile (**p, player, iCard);
                  return true;
               }
               if (card.number () == (*p)->back ()->number () + 1) {
                  playCardOnPile (**p, player, iCard, (*p)->size ());
                  return true;
               }
            }
      pos1Play = pos2Play = iCard;
}
   return false;
//-----------------------------------------------------------------------------
/// Checks if the passed card is a joker
/*--------------------------------------------------------------------------*/
//Purpose   : Checks if the passed card is a joker
//Parameters: card: Card to inspect
//Returns   : True if card is a joker
/*--------------------------------------------------------------------------*/
bool Burazno::isJoker (CardWidget& card) const {
   return (card.number () == CardWidget::TWO) || (card.number () > CardWidget::ACE);
//-----------------------------------------------------------------------------
/// Removes a cerrado (a pile with 7 cards) from the table
/*--------------------------------------------------------------------------*/
//Purpose   : Playes the passed card of player on the passed pile
//Parameters: pile: Pile to add card to
//            player: Number of player
//            card: Card of player to play
//            pos: Position of pile where to insert the card
/*--------------------------------------------------------------------------*/
void Burazno::playCardOnPile (CardVPile& pile, unsigned int player,
                              unsigned int card, unsigned int pos) {
   Check1 (player);
   Check1 (player < NUM_PLAYERS);
   Check1 (card < hands[player - 1].size ());
   Check1 (pos <= pile.size ());

   pile.insert (hands[player - 1].remove (card), pos);
}

/*--------------------------------------------------------------------------*/
//Purpose   : Removes a Burazno (a pile with 7 cards) from the table
//Parameters: team: Player causing the remove of the pile
//            pile: Pile holding the Burazno
/*--------------------------------------------------------------------------*/
void Burazno::removeBurazno (unsigned int team, CardVPile& pile) {
   Check1 (team < (NUM_PLAYERS >> 1));
   TRACE9 ("Burazno::removeBurazno (unsigned int, CardVPile&) - Pile "
           << (std::find (tablePiles[team].begin (),
                          tablePiles[team].end (), &pile)
               - tablePiles[team].begin ()) << " of team " << (team));
   Check1 (std::find (tablePiles[team].begin (), tablePiles[team].end (), &pile)
           != tablePiles[team].end ());

   boxTeam[team].remove (pile);
   tablePiles[team].erase (std::find (tablePiles[team].begin (),
                                      tablePiles[team].end (), &pile));
   delete &pile;
