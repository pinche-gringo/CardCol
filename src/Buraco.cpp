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
   : Game (parent, statusbar, cardset, names, 3, 3) {
   TRACE9 ("::Burazno::Burazno (Box&, Statusbar&, CardSet&, const "
           "std::vector<std::string>&)");
       scrlTable[i] = new Gtk::ScrolledWindow ();
   unsigned int width (cards.getCard (0).getImageWidth ());
   unsigned int height (cards.getCard (0).getImageHeight ());
           "std::vector<Glib::ustring>&) - Init common staples");

   boxTeam[0].pack_end (newPile, Gtk::PACK_EXPAND_WIDGET, 5);
   boxTeam[0].set_size_request (-1, height + 5 * 15);
   staple.setShowOption (ICardPile::SHOWBACK);
   dumped.setShowOption (ICardPile::SHOWFACE);

   hands[0].setStyle (ICardPile::COMPRESSED);
   hands[0].setShowOption (ICardPile::SHOWFACE);

   attach (hands[0], 3, 10, 0, 1, Gtk::EXPAND, Gtk::SHRINK, 1);
           "std::vector<Glib::ustring>&) - Attach widgets");
   attach (staple, 0, 1, 0, 1, Gtk::SHRINK, Gtk::SHRINK, 1);
   attach (dumped, 1, 2, 0, 1, Gtk::SHRINK, Gtk::SHRINK, 1);
           "std::vector<Glib::ustring>&) - Show widgets");
   hands[0].show ();
   boxTeam[0].show ();
   boxTeam[1].show ();
      dndType.push_back
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

   for (unsigned int i (0); i < deck.size (); ++i)
      delete deck[i];

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
   randomizeClonedCardsToPile (staple);
            reserve[(i - posServer) & 1].push_back (&staple.removeTopCard ());
   for (unsigned int i (0); i < NUM_PLAYERS; ++i) {
      for (unsigned int j (0); j < 13; ++j)
         hands[i].setTopCard (staple.removeTopCard ());
   if (startPlayer)

   Check3 (hands[0].numberOfCards ());
   registerDND (0, hands[0].numberOfCards () - 1);
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
   TRACE2 ("Burazno::enableHuman () - Human has " << hands[0].numberOfCards ()
   newPile.drag_dest_set (dndType, Gtk::DEST_DEFAULT_ALL, Gdk::ACTION_MOVE);
   aDNDTable[NULL] = newPile.signal_drag_data_received ().connect
   for (int i (hands[0].numberOfCards ()); i;)
      activeCards.push_back
         (hands[0].at (--i).signal_clicked ().connect
           (bind (slot (*this, (&Burazno::cardSelected)), i)));

   return Game::enableHuman ();
//-----------------------------------------------------------------------------
/// Disables the cards the human player can select
/*--------------------------------------------------------------------------*/
//Purpose   : Callback after clicking on a card in hand
//Parameters: iCard: Offset of card in hand
/*--------------------------------------------------------------------------*/
void Burazno::cardSelected (unsigned int iCard) {
   TRACE5 ("Burazno::cardSelected (unsigned int) - Position " << iCard);
   Check1 (iCard < hands[0].numberOfCards ());
   // Check if all piles are valid
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

   for (unsigned int i (0); i < deck.size (); ++i)
      pile.setTopCard (*deck[i]);
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
      (dndType,
       Gdk::ModifierType (GDK_BUTTON1_MASK | GDK_BUTTON2_MASK | GDK_BUTTON3_MASK));
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
