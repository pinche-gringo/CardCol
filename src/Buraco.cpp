//$Id$

//PROJECT     : Cardgames
//SUBSYSTEM   : Burazno
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
     , newPile (_("New pile")) {
   TRACE9 ("Burazno::Burazno (Box&, Statusbar&, CardSet&, const "
           "std::vector<std::string>&)");
       scrlTable[i] = new Gtk::ScrolledWindow ();
   int width, height;
   cards.getCard (0).getImageSize (width, height);
           "std::vector<Glib::ustring>&) - Init common staples");
   TRACE9 ("Burazno::Burazno (Box&, Statusbar&, CardSet&, const "
           "std::vector<std::string>&) - Init reserve cards");

   boxTeam[0].pack_end (newPile, Gtk::PACK_EXPAND_WIDGET, 5);
   boxTeam[0].set_size_request (-1, height + 5 * 15);
   staple.setShowOption (ICardPile::SHOWBACK);
   dumped.setShowOption (ICardPile::SHOWFACE);

   TRACE9 ("Burazno::Burazno (Box&, Statusbar&, CardSet&, const "
           "std::vector<std::string>&) - Init cards in hand");
   hands[0].setStyle (ICardPile::COMPRESSED);
   hands[0].setShowOption (ICardPile::SHOWFACE);

   boxTeam[0].pack_end (newPile, Gtk::PACK_EXPAND_WIDGET, 5);
           "std::vector<Glib::ustring>&) - Attach widgets");
   TRACE9 ("Burazno::Burazno (Box&, Statusbar&, CardSet&, const "
           "std::vector<std::string>&) - Attach widgets");
   attach (hands[0], 3, 10, 0, 1, Gtk::EXPAND, Gtk::SHRINK, 1, 5);
   attach (*scrlTable[0], 0, 10, 2, 3, Gtk::EXPAND | Gtk::FILL,
   attach (dumped, 1, 2, 0, 1, Gtk::SHRINK, Gtk::SHRINK, 1);
   attach (boxTeam[0], 0, 10, 1, 2);
   attach (boxTeam[1], 0, 10, 2, 3);
           "std::vector<Glib::ustring>&) - Show widgets");
   TRACE9 ("Burazno::Burazno (Box&, Statusbar&, CardSet&, const "
           "std::vector<std::string>&) - Show widgets");
   dumped.show ();
   hands[0].show ();
   boxTeam[0].show ();
   boxTeam[1].show ();

   if (dndType.empty ())
      dndType.push_back
   TRACE9 ("Burazno::Burazno (Box&, Statusbar&, CardSet&, const "
           "std::vector<std::string>&) - Create card packages");
   // Clone carddeck to play with 4 decks
   for (unsigned int i (0); i < 3; ++i)
      for (unsigned int j (0); j < cards.numberOfCards (); ++j)
         deck.push_back (new CardWidget (cards.getCard (j)));

         (Gtk::TargetEntry ("icon/card", Gtk::TARGET_SAME_APP, 0));

         (Gtk::TargetEntry ("icon/card", GTK_TARGET_SAME_APP, 0));
//-----------------------------------------------------------------------------
/// Destructor
/*--------------------------------------------------------------------------*/
//Purpose   : Destructor
/*--------------------------------------------------------------------------*/
Burazno::~Burazno () {
   TRACE9 ("Burazno::~Burazno ()");

   for (std::vector<CardWidget*>::iterator i (deck.begin ()); i != deck.end (); ++i)
      delete *i;

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
   randomizeClonedCardsToPile (staple);
   randomizeCardsToPile (staple);
            reserve[(i - posServer) & 1].push_back (&staple.removeTopCard ());
   for (unsigned int i (0); i < NUM_PLAYERS; ++i) {
      for (unsigned int j (0); j < 13; ++j)
         hands[i].setTopCard (staple.removeTopCard ());
   if (startPlayer)
   hands[0].sortByNumber ();

   Check3 (hands[0].numberOfCards ());
   registerDND (0, hands[0].numberOfCards () - 1);

   status.pop ();
   status.push (_("You can sort the cards in your hand with drag and drop ("
                  "with button 2 or 3)"));

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
   for (unsigned int i (0); i < NUM_PLAYERS; ++i)
   staple.clear ();
   dumped.clear ();

   for (unsigned int i (0); i < (NUM_PLAYERS >> 1); ++i) {
   disableHuman ();

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
   Check3 (hands[0].numberOfCards ());
   registerDND (0, hands[0].numberOfCards () - 1);
           << " cards");
   TRACE2 ("Burazno::enableHuman () - Human has " << hands[0].numberOfCards ()
   newPile.drag_dest_set (dndType, Gtk::DEST_DEFAULT_ALL, Gdk::ACTION_MOVE);
   aDNDTable[NULL] = newPile.signal_drag_data_received ().connect
   for (int i (hands[0].numberOfCards ()); i;)
      activeCards.push_back
         (hands[0].at (--i).signal_clicked ().connect
           (bind (slot (*this, (&Burazno::cardSelected)), i)));

   if (staple.numberOfCards ())
      stapleTop = staple.signal_clicked ().connect
         (slot (*this, (&Burazno::stapleSelected)));
   if (dumped.numberOfCards ())
      dumpedTop = dumped.signal_clicked ().connect
         (slot (*this, (&Burazno::dumpedSelected)));

   newPile.drag_dest_set (dndType, Gtk::DEST_DEFAULT_ALL, Gdk::ACTION_COPY);
   newPile.signal_drag_data_received ().connect
      (slot (*this, &Burazno::cardDroppedOnTable));
      Check3 (tablePiles[0][i]);
   return Game::enableHuman ();
//-----------------------------------------------------------------------------
/// Disables the cards the human player can select
/*--------------------------------------------------------------------------*/
//Purpose   : Disables the cards the human player can select
/*--------------------------------------------------------------------------*/
void Burazno::disableHuman () {
   TRACE2 ("Buarzno::disableHuman ()");


   dumpedTop.disconnect ();
   stapleTop.disconnect ();
      for (unsigned int i (0); i < tablePiles[0].size (); ++i) {
   newPile.drag_dest_unset ();
//-----------------------------------------------------------------------------
/// Callback after clicking on a card in the hand
/*--------------------------------------------------------------------------*/
//Purpose   : Callback after clicking on a card in hand
//Parameters: iCard: Offset of card in hand
/*--------------------------------------------------------------------------*/
void Burazno::cardSelected (unsigned int iCard) {
   TRACE5 ("Burazno::cardSelected (unsigned int) - Position " << iCard);
   Check1 (iCard < hands[0].numberOfCards ());
   // Check if all piles are valid
   if (!humanPilesOK ()) {
   dumped.append (hands[0].remove (iCard));

   return;
//-----------------------------------------------------------------------------
/// Callback after clicking on the staple
/*--------------------------------------------------------------------------*/
//Purpose   : Callback after clicking on the staple
/*--------------------------------------------------------------------------*/
void Burazno::stapleSelected () {
   TRACE5 ("Burazno::stapleSelected ()");
   Check2 (dumped.size ());
   // (means: *after* this signalhandler termintes)
   hands[0].append (staple.removeTopCard ());
   makeNextMoves ();
   return;
//-----------------------------------------------------------------------------
/// Callback after clicking on the dumped staple
/*--------------------------------------------------------------------------*/
//Purpose   : Callback after clicking on the dumped staple
/*--------------------------------------------------------------------------*/
void Burazno::dumpedSelected () {
   TRACE5 ("Burazno::dumpedSelected ()");
   Check3 (stapleTop.connected ()); Check3 (dumpedTop.connected ());

   hands[0].append (dumped.removeTopCard ());
   makeNextMoves ();
   return;
//-----------------------------------------------------------------------------
/// Enables a card in the hand of the player
/*--------------------------------------------------------------------------*/
//Purpose   : Shuffles (Randomizes) the cloned cards onto the staple
/*--------------------------------------------------------------------------*/
void Burazno::randomizeClonedCardsToPile (ICardPile& pile) {
   unsigned int nr;
   for (int i (deck.size ()); i > 0;) {
      nr = rand () % i--;
      TRACE2 ("Burazno::randomizeClonedCardsToPile (ICardPile&) - " << i << " = " << nr);
      std::swap (deck[i], deck[nr]);
   }

   for (std::vector<CardWidget*>::iterator i (deck.begin ()); i != deck.end (); ++i)
      pile.setTopCard (**i);
}

/*--------------------------------------------------------------------------*/
//Purpose   : Prepares the card for drag´n´drop
//Parameters: pile: Number of card in hand
/*--------------------------------------------------------------------------*/
void Burazno::registerDND (unsigned int iCard) {
   TRACE9 ("Burazno::registerDND (unsigned int) - Card: " << iCard << " ("
           << hands[0].at (iCard) << ')');
   Check3 (aDNDHand.find (&card) == aDNDHand.end ());
   static Glib::RefPtr<Gdk::Bitmap> bitmap;
   CardWidget& card (hands[0].at (iCard));
   card.drag_dest_set (dndType, Gtk::DEST_DEFAULT_ALL, Gdk::ACTION_MOVE);
   card.drag_source_set
   card.drag_dest_set (dndType, Gtk::DEST_DEFAULT_ALL, Gdk::ACTION_COPY);
       Gdk::ACTION_MOVE);
      (dndType, Gdk::ModifierType (GDK_BUTTON2_MASK | GDK_BUTTON3_MASK));
   aDNDHand[&card].connReceive = card.signal_drag_data_received ().connect
   card.drag_source_set_icon (get_colormap (), card.getImage (), bitmap);
   card.signal_drag_data_received ().connect
      (bind (slot (*this, &Burazno::cardDropped), iCard));
   card.signal_drag_data_get ().connect
      (bind (slot (*this, &Burazno::getDropData), iCard));
//-----------------------------------------------------------------------------
/// Stops the drag´n´drop abilities of the passed card
/*--------------------------------------------------------------------------*/
//Purpose   : Stops the drag´n´drop abilities of the passed card
//Parameters: iCard: Offset of card to unregister of dnd
/*--------------------------------------------------------------------------*/
void Burazno::unregisterDND (unsigned int iCard) const {
   TRACE9 ("Burazno::unregisterDND (unsigned int) - Card: " << iCard << " ("
           << hands[0].at (iCard) << ')');
   Check1 (iCard < hands[0].numberOfCards ());
   card.drag_source_unset ();
   CardWidget& card (hands[0].at (iCard));
   i->second.connReceive.disconnect ();
   i->second.connGet.disconnect ();
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
   static bool second (true);
   second = !second;
   if (second)
      return;

   Check3 (pData);
   Check3 (data.get_format () == 8);
   Check3 (pData->length == sizeof (int));
   Check3 (pData->format == 8);

   Check3 (card < hands[0].numberOfCards ());
                         (const_cast<guint8*> (data.get_data ())));
   unsigned int* pValue (reinterpret_cast <unsigned int*> (pData->data));
   TRACE1 ("Buraco::cardDropped (...) - Inserting card " << *pValue
   Check3 (*pValue < hands[0].numberOfCards ());
   TRACE1 ("Burazno::cardDropped (...) - Inserting card " << *pValue
   if (acceptCards != -1U) {
      context->drag_finish (false, false, time);
   CardWidget& cardTarget (hands[0].at (card));
   CardWidget& cardMoved (hands[0].remove (*pValue));
   if (*pValue > card)
      ++card;

   // End old DND
   context->drag_finish (true, false, time);

   // Insert moved card
   hands[0].insert (cardMoved, card);
   if (*pValue < card) {
      unsigned int temp (card);
      card = *pValue;
      *pValue = temp;
   }

   Glib::signal_idle ().connect
   registerDND (card, *pValue);
//-----------------------------------------------------------------------------
/// Checks if the piles on the table are valid (have at least 3 cards)
/*--------------------------------------------------------------------------*/
//Purpose   : Callback after dropping a card on the table
//Parameters: pContext: Context of the drag (contains things like source,
//                      target, action, ...)
//            pData: Describes the thing which was dropped
//            info: Describes the type of pData (should be 0)
//            time: Timestamp of the drag
//Requieres : pContext, pData not NULL;
/*--------------------------------------------------------------------------*/
void Burazno::cardDroppedOnTable (const Glib::RefPtr<Gdk::DragContext>& context,
                                  gint, gint, GtkSelectionData* pData,
                                  guint, guint32 time) {
   Check3 (pData);
   Check3 (data.get_format () == 8);
   Check3 (pData->length == sizeof (int));
   Check3 (pData->format == 8);
                         (const_cast<guint8*> (data.get_data ())));
   unsigned int* pValue (reinterpret_cast <unsigned int*> (pData->data));
           << " in pile");
   Check3 (*pValue < hands[0].numberOfCards ());
   TRACE1 ("Burazno::cardDroppedOnTable (...) - Inserting card " << *pValue
           << " in new pile");
   if (!humanPilesOK (iCard >> 8)) {
   // End old DND
   context->drag_finish (true, false, time);

   // Move dropped card to a new pile on the table
   CardVPile* pile (Gtk::manage (new CardVPile ()));
   CardWidget moved (hands[0].remove (*pValue));
   moved.drag_dest_unset ();
   TRACE4 ("Burazno::cardDroppedOnTable (...) - Card dropped: " << moved);
       && isJoker (moved) || (*pValue >= acceptCards)) {
   pile->setShowOption (ICardPile::SHOWFACE);
   pile->setStyle (ICardPile::COMPRESSED);
   pile->setTopCard (moved);
   pile->set_size_request (50, 50);
	   << "; " << *pValue << ": " << acceptCards);
   boxTeam[0].pack_start (*pile, Gtk::PACK_SHRINK, 5);
   pile->show ();
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
   TRACE9 ("Burazno::getDropData (...) - Nummer " << cardPos << " for "
           << hands[0].at (cardPos));
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
void Burazno::registerDND (unsigned int start, unsigned int end) {
   Check1 (end < activeCards.size ());
   Check1 (end < hands[0].numberOfCards ());
      TRACE9 ("Buraco::registerHandDND (unsigned int, unsigned int) - Handling card " << start);

      unregisterDND (start);
      registerDND (start);
}
}

/*--------------------------------------------------------------------------*/
//Purpose   : Update the carddecks; handling of the images outside of the
//            carddeck
/*--------------------------------------------------------------------------*/
void Burazno::updateCards () {
   for (std::vector<CardWidget*>::iterator i (deck.begin ()); i != deck.end (); ++i)
      (*i)->update ();

   staple.update ();
   dumped.update ();
