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

#include <cardgames-cfg.h>

#include <gtk/gtkdnd.h>

#include <gtkmm/messagedialog.h>

#define CHECK 9
#define TRACELEVEL 9
#include <Check.h>
#include <Trace_.h>
#include <ANumeric.h>
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
   : Game (parent, statusbar, cardset, names, 3, 10), startPlayer (0)
     , staple (ICardPile::SHOWBACK), dumped (ICardPile::SHOWFACE)
     , handHuman (ICardPile::COMPRESSED, ICardPile::SHOWFACE)
     , newPile (_("New pile")), startTurn (true), target (-1U)
     , pos1 (0), pos2 (0) {
   TRACE9 ("Burazno::Burazno (Box&, Statusbar&, CardSet&, const "
           "std::vector<std::string>&)");
       scrlTable[i] = new Gtk::ScrolledWindow ();

   TRACE9 ("Buraco::Buraco (Box&, Statusbar&, CardSet&, const "
           "std::vector<Glib::ustring>&) - Init common staples");
   TRACE9 ("Burazno::Burazno (Box&, Statusbar&, CardSet&, const "
           "std::vector<std::string>&) - Init common staples");

   boxTeam[0].pack_end (newPile, Gtk::PACK_EXPAND_WIDGET, 5);
   boxTeam[0].set_size_request (-1, height + 5 * 15);
   boxTeam[1].set_size_request (-1, height + 5 * 15);

   for (unsigned int i (0); i < NUM_PLAYERS - 1; ++i) {
      attach (hands[i], 0, 10, 3, 4, Gtk::EXPAND, Gtk::SHRINK, 0);
      hands[i].setStyle (ICardPile::COMPRESSED);
      hands[i].setShowOption (ICardPile::SHOWBACK);
   hands[0].setShowOption (ICardPile::SHOWFACE);
   hands[0].show ();
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
   frameInfo.set_shadow_type (Gtk::SHADOW_IN);
   info.set_size_request (200, -1);
   statusbar.set_has_resize_grip (false);
   statusbar.pack_end (info, Gtk::PACK_SHRINK, 5);

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

   Check1 (player); Check1 (player < NUM_PLAYERS);
   Check1 (hands[player - 1].size ());
   cleanCerrado (player);
   // First cleanup buraznos made in the last turn
   for (std::vector<CardVPile*>::iterator p (tablePiles[player & 1].begin ());
        p != tablePiles[player & 1].end (); ++p) {
      Check3 (*p); Check3 ((*p)->size () <= 7);
      if ((*p)->size () == 7) {
         removeBurazno (player, **p);
         break;
      }
   }
      target = showCardsToPlay (player);
      TRACE8 ("Buraco::makeMove (unsigned int) - Going to play cards to "
              << std::hex << (int)target << std::dec);
   }
      Check1 (pos1Play <= pos2Play);

      Check1 (pos1 <= pos2);
      // to dumped staple; else target specifies offset of pile and card on
      // Calculate pile to play cards to: If target = 0xffff0000, append cards to
      // dumped staple; else target specifies offset of pile and card on table
      Check3 (hands[player - 1].getTopCard ().showsFace ());
      CardHPile& source (hands[player]);
      unsigned int oldPlayer (player);
      CardHPile& source (hands[player - 1]);
      unsigned int oldPlayer (player);
      if (target == 0xffff) {
         TRACE4 ("Buraco::makeMove (unsigned int) - Dumping card");
         Check3 (pos1Play == pos2Play);
         TRACE4 ("Burazno::makeMove (unsigned int) - Dumping card");
         Check3 (pos1 == pos2);

         gStatus.startTurn = 1;
         ++player &= 0x3;
         // Make next player continue
         source.hide ();
         hands[player == 3 ? 0 : player].show ();

         startTurn = true;
      }
      else {
         TRACE4 ("Buraco::makeMove (unsigned int) - Moving cards to pile " << target);
         Check3 (target < tablePiles[player & 1].size ());
         TRACE4 ("Burazno::makeMove (unsigned int) - Moving cards to pile " << target);

#if CHECK > 2
      // Move played cards to the pile to play
      Check3 (source.size () > pos2Play);
      // Move played cards to pile to play
      for (; pos1 <= pos2; --pos2)
         dest->insert (source.remove (pos1), pos);
      
            addBuraco (oldPlayer);
         if (reserve[oldPlayer & 1].size ())
            addReserve (oldPlayer);
         else {
            Check3 (buraznos[oldPlayer & 1]);
            endGame ();
            return -1;
         }
   return player;
      target = -1U;
}

//-----------------------------------------------------------------------------
/// Searches for cards to play and shows them in the hand of the actual player
/*--------------------------------------------------------------------------*/
//Purpose   : Searches for cards to play and shows them in the hand of the
//            actual player
//Parameters: player: Player to inspect
//Returns   : ID for target (32 Bit: Pile << 16 + Position)
/*--------------------------------------------------------------------------*/
unsigned int Burazno::showCardsToPlay (unsigned int player) {
   TRACE2 ("unsigned int Burazno::showCardsToPlay (unsigned int) - " << player);
   if (startTurn) {
      startTurn = false;
      ICardPile& playerPile (hands[player - 1]);
          ? (isJoker (dumpedCard)
      Check3 (dumped.size ());
      // Check if there are equal cards as the last dumped one
      int start (playerPile.find (dumped.getTopCard ().number ()));
      int end ((start == -1) ? -1 : playerPile.findLastEqual (start));
      Check3 ((start != -1) ? (start <= end) : (start == end));
      Check3 ((end == -1) || (end < playerPile.size ()));

      if ((!isJoker (dumped.getTopCard ())) && ((end - start) >= 1)
          && (buraznos[player & 1] || !reserve[player & 1].empty ()
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
         playerPile.insertSorted (staple.removeTopCard ());
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
int Burazno::executeMove (unsigned int player) {
   TRACE8 ("Burazno::executeMove (player) - Checking for 3 in a row");

   // Check for 3 cards having the same number
   ICardPile& playerPile (hands[player - 1]);
   unsigned int count (1);
   for (unsigned int i (1); i < playerPile.size () - 1; ++i) {
      Check3 (playerPile[i]);
      if (isJoker (*playerPile[i]))
          continue;

      // Check if the actual card can be added to an existing pile
      if (tablePiles[player & 1].size ()) {
         unsigned int target (cardFitsOnPlayedPile (player, i));
         if (target != -1U)

   // Check for 3 cards belonging to a serie

      // Check if there are 3 (or more) of a kind
      if (playerPile[i]->number () == playerPile[i - 1]->number ())
         ++count;
      else {
         if (count >= 3) {
            CardVPile& pile (makeNewPile (player & 1));    // Create new pile with
            pos1 = i - count;
            pos2 = i - 1;
            flipCards2Play (playerPile, pos1, pos2);
            return (tablePiles[player & 1].size () - 1) << 16;
         }
         count = 1;

   // Check if all cards in the hand can (and should) be played
   TRACE8 ("Buraco::executeMove (unsigned int) - Playing all?");
         addBuraco (player);
      if (reserve[player & 1].size ()) {
         addReserve (player);

   // No more cards to put down: Find a card to dump
   TRACE8 ("Buraco::executeMove (unsigned int) - Searching for a card to dump");
   for (i = 0; i < playerPile.size () - 1; ++i) {
   TRACE8 ("Burazno::executeMove (unsigned int) - Searching for a card to dump");
   count = 1;
   unsigned int i (1);
   for (; i < playerPile.size () - 1; ++i) {
      if (isJoker (*playerPile[i]))
          continue;

      Check3 (i);
      if (playerPile[i]->number () != playerPile[i - 1]->number ())
         if (count == 1)
            break;
         else
            count = 1;
      else
         ++count;
      break;
   }
   Check3 (i <= playerPile.size ());
   if (i >= playerPile.size ()) {         // No single card found: Dump highest
      while (i--)
         if (!isJoker (*playerPile[i]))
            break;
   }
   pos1Play = pos2Play = i;
   return 0xffff0000;
   flipCards2Play (playerPile, pos1 = i, pos2 = i);
   return 0xffff << 16;
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

   buraznos[0] = buraznos[1] = 0;
   updateInfo ();
   hands[0].show ();
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
void Burazno::cardSelected (unsigned int iCard) {
   TRACE5 ("Burazno::cardSelected (unsigned int) - Position " << iCard);
   Check1 (iCard < handHuman.size ());
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
   unregisterHandDND (*handHuman[iCard]);
   dumped.append (handHuman.remove (iCard));
   // reserve
   // If the player has no more cards left (except of joker): Give him the reserve
   if (containsOnlyJoker (handHuman))
      if (reserve[0].size ())
         addReserve (0);
      else if (handHuman.empty ()) {
      }

   gStatus.startTurn = 1;
   gStatus.startGame = 0;
   hands[2].hide ();
   hands[0].show ();

   makeNextMoves ();
}

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
//Purpose   : Checks if the piles on the table are valid (have at least 3 cards)
//Parameters: except: Pile which can be invalid
//Returns   : True, if the piles are OK
/*--------------------------------------------------------------------------*/
bool Burazno::humanPilesOK (unsigned int except) const {
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
   // Only allow dropping of last card, if the game can be ended, or there
   // is still the reserve
   if ((handHuman.size () < 2) && !buraznos[0] && reserve[0].empty ()) {
      context->drag_finish (false, false, time);
      Gtk::MessageDialog dlg (_("Can't drop last  cards!!"),
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
   CardWidget& moved (*handHuman[*pValue]);
   TRACE4 ("Burazno::cardDroppedOnTable (...) - Card dropped: " << moved);
       && isJoker (moved) || (*pValue >= acceptCards)) {
   if (iCard == -1U) {    // If card was dropped on the new label: Create pile
      // Check validity of drop
      unsigned int equalNr (1);
      unsigned int equalColors (0);
      bool joker (false);
      for (std::vector<CardWidget*>::const_iterator i (handHuman.begin ());
           i != handHuman.end (); ++i) {
         Check3 (*i);
         if (isJoker (**i) && !joker) {
            joker = true;
            equalNr++;
            continue;
         }
         if (moved.number () == (*i)->number ())
            ++equalNr;
         else if (moved.color () == (*i)->color ()) {
            int diff (moved.number () - (*i)->number ());
            if ((unsigned int)(diff + 2) > 5)
               continue;
            equalColors |= (diff < 0) ? -diff << 2 : diff;
         }
      }
      Check (equalColors < 0x10);

      if ((equalNr < 3)      // Card can be dropped, if there are 3 equal numbers
          && (equalColors != 0x3) && (equalColors != 0x6) && (equalColors != 0xc)
          && !(equalColors && joker)) {
         context->drag_finish (false, false, time);
         Gtk::MessageDialog dlg (_("There are no cards to make three of a kind!"),
                                 Gtk::MESSAGE_ERROR);
         dlg.set_title (_("Invalid move"));
         dlg.run ();
         return;
      iPile = tablePiles[0].size ();
      pile = &makeNewPile (0); Check3 (tablePiles[0].size ());
      iCard = 0;
   }
   else {
      // Else check pile to use
      Check1 ((iCard >> 8) < tablePiles[0].size ());
      // Else find pile to use
      Check1 ((iCard >> 8) <= tablePiles[0].size ());
      pile = tablePiles[0][iPile = (iCard++ >> 8)];
      if (cardFitsOnPile (*pile, moved) == -1) {
                                 Gtk::MESSAGE_ERROR);
         Gtk::MessageDialog dlg (_("This card does not fit on the dropped pile!"),
         dlg.run ();
         return;
      }

      if ((iCard = cardFitsOnPile (iPile, moved)) == -1U) {
      iCard &= 0xff;
   // End old drag
   context->drag_finish (true, false, time);
   activeCards[*pValue].disconnect ();
   activeCards.erase (activeCards.begin () + *pValue);

   // Unregister old card
   hands[0].remove (*pValue);
   unregisterHandDND (moved);
   handHuman.remove (*pValue);
   // Insert card into pile and register it for DND
   unsigned int move (-1U);
   pile->getPosition4Card (moved, iCard, move);
   if (iCard < (pile->size () - 1))
      registerTableDND (iPile, iCard + 1, pile->size () - 1);

   TRACE9 ("Buraco::cardDroppedOnTable (...) - Undo:  " << iPile << "; " << iCard
	   << "; " << *pValue << ": " << acceptCards);
      removeCerrado (0, *pile);

      removeBurazno (0, *pile);
   // disabled, if the human picked up the dumped pile.
      if (!reserve[0].empty ()) {
   if (containsOnlyJoker (handHuman))
      if (reserve[0].size ())
         addReserve (0);
      else {
         endGame ();
         return;
      }
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
//Parameters: player: Player getting the reserve
/*--------------------------------------------------------------------------*/
void Burazno::addReserve (unsigned int player) {
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
   target.setTopCards (reserve[player & 1]);
   if (!player)
   target.sortByNumber ();
      for (unsigned int i (0); i < hands[player].size (); ++i) {
         enableCard (i);
      for (unsigned int i (0); i < target.size (); ++i) {
      }

   Check3 (actPlayers.size () > player);

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
   pile->set_size_request (width, height + 5 * 15);

//-----------------------------------------------------------------------------
/// Checks if the passed card can be put on one of the existing piles
/*--------------------------------------------------------------------------*/
//Purpose   : Checks if the passed card can be put on one of the existing piles
//Parameters: player: Player to inspect
//            iCard: Card to inspect
//Returns   : Value describing the pile (and the offset of the card) to play to; -1 if none
//Remarks   : This method actually moves the card
/*--------------------------------------------------------------------------*/
unsigned int Burazno::cardFitsOnPlayedPile (unsigned int player, unsigned int iCard) {
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
         pos1 = pos2 = iCard;
         flipCards2Play (hands[player - 1], pos1, pos2);
         return ((p - tablePiles[player & 1].begin ()) << 16) + (*p)->size ();

      else {
         int pos (cardFitsOnPile (**p, card));
         if (pos != -1) {
            pos1 = pos2 = iCard;
            flipCards2Play (hands[player - 1], pos1, pos2);
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
bool Burazno::isJoker (CardWidget& card) const {
   TRACE9 ("Burazno::isJoker (CardWidget&) const - " << card << " = "
           << card.number ());
   return (card.number () == CardWidget::TWO) || (card.number () > CardWidget::ACE);
//-----------------------------------------------------------------------------
/// Removes a cerrado (a pile with 7 cards) from the table
/*--------------------------------------------------------------------------*/
//Purpose   : Removes a Burazno (a pile with 7 cards) from the table
//Parameters: player: Player causing the remove of the pile
//            pile: Pile holding the Burazno
/*--------------------------------------------------------------------------*/
void Burazno::removeBurazno (unsigned int player, CardVPile& pile) {
           != tablePiles[team].end ());
   unsigned int team (player & 1);
   std::vector<CardVPile*>::iterator i
      (std::find (tablePiles[team].begin (), tablePiles[team].end (), &pile));
   Check1 (i != tablePiles[team].end ());
   Check3 (static_cast<int> (pile.getPotentialPoints ()) == pile.getPoints ());
   TRACE9 ("Burazno::removeBurazno (unsigned int, CardVPile&) - Pile "
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
   buraznos[team]++;

//-----------------------------------------------------------------------------
/// Actualizes the info-part of the statusbar
/*--------------------------------------------------------------------------*/
//Purpose   : Actualizes the info-part of the statusbar
/*--------------------------------------------------------------------------*/
void Burazno::updateInfo () {
   std::string strInfo (_("Buraznos: %1 [%2] / %3 [%4]"));
   strInfo.replace (strInfo.find ("%1"), 2, ANumeric::toString (buraznos[0]));
   strInfo.replace (strInfo.find ("%2"), 2, 1, (reserve[0].size () ? 'Y' : 'N'));
   strInfo.replace (strInfo.find ("%3"), 2, ANumeric::toString (buraznos[1]));
   strInfo.replace (strInfo.find ("%4"), 2, 1, (reserve[1].size () ? 'Y' : 'N'));
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
/*--------------------------------------------------------------------------*/
int Burazno::cardFitsOnPile (CardVPile& pile, CardWidget& card) const {
   Check1 (pile.size ());
   if ((pile.size () == 1) && isJoker (*pile[0]))
      return true;
   //   - A joker; if there are at least 3 jokers (on table + in hand)
   if (isJoker (card))
      return containsNoJoker (pile) ? 1 : -1;

   CardVPile::const_iterator i (pile.begin ());
   if (isJoker (**i)) ++i;
   CardWidget* first (*i++); Check3 (first); Check3 (!isJoker (*first));
   CardWidget* second (((i != pile.end ()) && isJoker (**i)) ? *++i : *i);
   if (i == pile.end ())
      second = first;
   Check3 (!isJoker (*second));

   TRACE5 ("Burazno::cardFitsOnPile (CardVPile&, CardWidget&) - "
           "Cards: " << *first << " and " << *second);
   // Then check if the pile is a numberd or a coloured one
   if (first->number () == card.number ()) {
      if (first->number () == second->number ())
         return pile.size ();
   }
   else
      if ((first->color () == card.color ())
          && (second->color () == card.color ())) {
         // TODO: Check for joker in pile
         if (card.number () == (first->number () - 1))
            return 0;
         second = pile.back ();
         if (isJoker (*second) && (pile.size () > 1))
            second = pile[pile.size () - 2];
         if (card.number () == (second->number () + 1))
            return pile.size ();
      }

//----------------------------------------------------------------------------
/// Shows or hides the cards of the computer player
/*--------------------------------------------------------------------------*/
//Purpose   : Shows or hides the cards of the computer player
//Parameters: open: Flag if cards should be shown or hidden
/*--------------------------------------------------------------------------*/
void Burazno::playOpen (bool open) {
   for (unsigned int i (0); i < (NUM_PLAYERS - 1); ++i)
   }
//-----------------------------------------------------------------------------
/// Creates the combined team names from the players
/*--------------------------------------------------------------------------*/
//Purpose   : Performs the steps to end the game
/*--------------------------------------------------------------------------*/
void Burazno::endGame () {
   status.pop ();
   status.push (_("Game ended"));

   if (!pScoreDlg) {
      pScoreDlg = ScoreDlg::create (nameTeams);
   setGameStatus (STOPPED);   
