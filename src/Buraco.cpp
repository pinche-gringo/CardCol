//$Id$

//PROJECT     : Cardgames
//SUBSYSTEM   : Burazno
//REFERENCES  :
//TODO        : - Shuffle staple; - Disconnect get_drop_data signal
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
     , newPile (_("New pile")) {
   TRACE9 ("Burazno::Burazno (Box&, Statusbar&, CardSet&, const "
           "std::vector<std::string>&)");
       scrlTable[i] = new Gtk::ScrolledWindow ();

   TRACE9 ("Buraco::Buraco (Box&, Statusbar&, CardSet&, const "
           "std::vector<Glib::ustring>&) - Init common staples");
   TRACE9 ("Burazno::Burazno (Box&, Statusbar&, CardSet&, const "
           "std::vector<std::string>&) - Init reserve cards");

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
   TRACE5 ("Burazno::makeMove () - Turn of player " << player);

   Check3 (player);
   cleanCerrado (player);
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
         hands[i].setTopCard (staple.removeTopCard ());
      handHuman.setTopCard (staple.removeTopCard ());
      gStatus.team1Buraco = gStatus.team2Buraco = 0x3;
      for (unsigned int i (0); i < (sizeof (reserve) / sizeof (reserve[0])); ++i)
         reserve[i].setTopCard (staple.removeTopCard ());
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
   handHuman.clear ();
   for (unsigned int i (0); i < NUM_PLAYERS - 1; ++i)
   staple.clear ();
   dumped.clear ();

   for (unsigned int i (0); i < (NUM_PLAYERS >> 1); ++i) {
   disableHuman ();


   for (std::vector<CardVPile*>::iterator i (aPiles.begin ());
        i != aPiles.end (); ++i)
      delete *i;
   aPiles.clear ();
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
   Check3 (handHuman.numberOfCards ());
   for (unsigned int i (0); i < handHuman.numberOfCards (); ++i)
   }
   Check3 (aDNDHand.size () == handHuman.numberOfCards ());
           << " cards");
   TRACE2 ("Burazno::enableHuman () - Human has " << handHuman.numberOfCards ()
   newPile.drag_dest_set (dndType, Gtk::DEST_DEFAULT_ALL, Gdk::ACTION_MOVE);
   aDNDTable[NULL] = newPile.signal_drag_data_received ().connect
   for (unsigned int i (0); i < handHuman.numberOfCards (); ++i)
      enableCard (i);

   if (staple.numberOfCards ())
      stapleTop = staple.signal_clicked ().connect
         (slot (*this, (&Burazno::stapleSelected)));
   if (dumped.numberOfCards ())
      dumpedTop = dumped.signal_clicked ().connect
         (slot (*this, (&Burazno::dumpedSelected)));

      (bind (mem_fun (*this, &Buraco::cardDroppedOnTable), -1U));

      (bind (slot (*this, &Burazno::cardDroppedOnTable), -1U));
      Check3 (tablePiles[0][i]);
   for (unsigned int i (0); i < aPiles.size (); ++i) {
      Check3 (aPiles[i]);
      for (unsigned int j (0); j < aPiles[i]->numberOfCards (); ++j)
         registerTableDND (aPiles[i]->at (j), (i << 8) + j);
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


   dumpedTop.disconnect ();
   stapleTop.disconnect ();
   if (aDNDHand.size ())
      for (unsigned int i (0); i < hands[0].size (); ++i)
         unregisterHandDND (*hands[0][i]);
      for (unsigned int i (0); i < handHuman.numberOfCards (); ++i)
         unregisterHandDND (handHuman.at (i));
   if (aDNDTable.size ()) {
      for (unsigned int i (0); i < tablePiles[0].size (); ++i) {
   for (unsigned int i (0); i < aPiles.size (); ++i) {
      Check3 (aPiles[i]);
      for (unsigned int j (0); j < aPiles[i]->numberOfCards (); ++j)
         unregisterTableDND (aPiles[i]->at (j));

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
   Check1 (iCard < handHuman.numberOfCards ());
   // Check if all piles are valid
   if (!humanPilesOK ()) {
   unregisterHandDND (handHuman.at (iCard));
   dumped.append (handHuman.remove (iCard));

//-----------------------------------------------------------------------------
/// Callback after clicking on the staple
/*--------------------------------------------------------------------------*/
//Purpose   : Callback after clicking on the staple
/*--------------------------------------------------------------------------*/
void Burazno::stapleSelected () {
   TRACE5 ("Burazno::stapleSelected ()");
   Check2 (dumped.size ());
   Check3 (staple.numberOfCards ());
   // (means: *after* this signalhandler termintes)
   handHuman.append (staple.removeTopCard ());
   enableCard (handHuman.numberOfCards () - 1);
   registerHandDND (handHuman.numberOfCards () - 1);
   dumpedTop.disconnect ();
   stapleTop.disconnect ();
//-----------------------------------------------------------------------------
/// Callback after clicking on the dumped staple
/*--------------------------------------------------------------------------*/
//Purpose   : Callback after clicking on the dumped staple
/*--------------------------------------------------------------------------*/
void Burazno::dumpedSelected () {
   TRACE5 ("Burazno::dumpedSelected ()");
   Check3 (stapleTop.connected ()); Check3 (dumpedTop.connected ());
   Check3 (dumped.numberOfCards ());

   while (dumped.numberOfCards ()) {
      handHuman.append (dumped.removeTopCard ());
      enableCard (handHuman.numberOfCards () - 1);
      registerHandDND (handHuman.numberOfCards () - 1);
   }

   dumpedTop.disconnect ();
   stapleTop.disconnect ();
//-----------------------------------------------------------------------------
/// Enables a card in the hand of the player
/*--------------------------------------------------------------------------*/
//Purpose   : Enables a card in the hand of the player
/*--------------------------------------------------------------------------*/
void Burazno::enableCard (unsigned int pos) {
   TRACE9 ("Burazno::enableCard (unsigned int) - Enabling card " << pos);
   Check1 (pos < handHuman.numberOfCards ());
      (hands[0][pos]->signal_clicked ().connect
       (bind (mem_fun (*this, (&Buraco::cardSelected)), pos)));
      (handHuman.at (pos).signal_clicked ().connect
       (bind (slot (*this, (&Burazno::cardSelected)), pos)));
//-----------------------------------------------------------------------------
/// Prepares the card for drag´n´drop
/*--------------------------------------------------------------------------*/
//Purpose   : Prepares the card for drag´n´drop
//Parameters: iCard: Number of card in hand
/*--------------------------------------------------------------------------*/
void Burazno::registerHandDND (unsigned int iCard) {
   TRACE9 ("Burazno::registerHandDND (unsigned int) - Card: " << iCard << " ("
           << handHuman.at (iCard) << " = " << &handHuman.at (iCard) << ')');
   Check3 (aDNDHand.find (&card) == aDNDHand.end ());
   CardWidget& card (handHuman.at (iCard));
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
   Check1 (pile < aPiles.size ());

   Check1 (end < aPiles[pile]->numberOfCards ());
   pile <<= 8;
      unregisterTableDND (card);
      CardWidget& card (aPiles[pile]->at (start));
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
   TRACE ("Searching for connection");

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
   Check3 (card < handHuman.numberOfCards ());
                         (const_cast<guint8*> (data.get_data ())));
   unsigned int* pValue (reinterpret_cast <unsigned int*> (pData->data));
   TRACE1 ("Buraco::cardDropped (...) - Inserting card " << *pValue
   Check3 (*pValue < handHuman.numberOfCards ());
   TRACE1 ("Burazno::cardDropped (...) - Inserting card " << *pValue
   if (acceptCards != -1U) {
      context->drag_finish (false, false, time);
   CardWidget& cardMoved (hands[0].remove (*pValue));
   hands[0].insert (cardMoved, card);                     // Insert moved card
   CardWidget& cardTarget (handHuman.at (card));
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
   Check3 (aDNDHand.size () == handHuman.numberOfCards ());
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
   Check3 (*pValue < handHuman.numberOfCards ());
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
      int width (cards.getCard (0).getImageWidth ());
      int height (cards.getCard (0).getImageHeight ());
      iPile = aPiles.size ();
      aPiles.push_back (pile = (new CardVPile (ICardPile::COMPRESSED,
                                               ICardPile::SHOWFACE)));
      pile->set_size_request (width, height + 6 * 15);
      boxTeam[0].pack_start (*pile, Gtk::PACK_SHRINK, 5);
      pile->show ();
   else {
      // Else check pile to use
      Check1 ((iCard >> 8) < tablePiles[0].size ());
      // Else find pile to use
      Check1 ((iCard >> 8) < aPiles.size ());
      pile = aPiles[iPile = (iCard++ >> 8)];
      iCard &= 0xff;
      Check1 (iCard <= pile->numberOfCards ());
   // End old drag
   pile->getPosition4Card (moved, iCard, move);
   if (iCard < (pile->size () - 1))
      registerTableDND (iPile, iCard + 1, pile->size () - 1);
   if (iCard < (pile->numberOfCards () - 1))
      registerTableDND (iPile, iCard + 1, pile->numberOfCards () - 1);
	   << "; " << *pValue << ": " << acceptCards);
   // Re-register the cards in the hand for DND
   if (*pValue < handHuman.numberOfCards ())
      registerHandDND (*pValue, handHuman.numberOfCards () - 1);
   Check3 (aDNDHand.size () == handHuman.numberOfCards ());
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
   Check1 (end < handHuman.numberOfCards ());
   for (; start <= end; ++start) {
      TRACE9 ("Buraco::registerHandDND (unsigned int, unsigned int) - Handling card " << start);

         (bind (mem_fun (*this, (&Buraco::cardSelected)), start));
      activeCards[start] = handHuman.at (start).signal_clicked ().connect
         (bind (slot (*this, (&Burazno::cardSelected)), start));
      registerHandDND (start);
      unregisterHandDND (handHuman.at (start));
   TRACE9 ("Buraco::registerHandDND (unsigned int, unsigned int) - End ");
}
