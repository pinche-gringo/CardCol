//$Id$

//PROJECT     : Cardgames
//SUBSYSTEM   : Sgt. Mayor
//REFERENCES  :
//TODO        :
//BUGS        :
//REVISION    : $Revision$
//AUTHOR      : Markus Schwab
//CREATED     : 11.4.2004
//COPYRIGHT   : Copyright (C) 2004

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


#include <sstream>

#include <cardgames-cfg.h>

#define CHECK 9
#define TRACELEVEL 9
#include <YGP/Check.h>
#include <YGP/Trace.h>
#include <YGP/ConnMgr.h>
#include <YGP/Tokenize.h>

#include <gtkmm/menu.h>
#include <gtkmm/statusbar.h>
#include <gtkmm/messagedialog.h>

#include <Player.h>
#include <ScoreDlg.h>

#include "SgtMayor.h"


const unsigned int SgtMayor::COLS_PLAYER[NUM_PLAYERS] = { 1, 0, 5 };
const unsigned int SgtMayor::ROWS_PLAYER[NUM_PLAYERS] = { 1, 5, 5 };


//-----------------------------------------------------------------------------
/// Constructor
/// \param parent: Parent widget to display the game in
/// \param statusbar: Status bar widget to display information about the game
/// \param cardset: Cardset to use
/// \param player: Vector of player
/// \param posPlayer: Position of player for the server
/// \param mxSerialize: Mutex to serialize messages from the server
//-----------------------------------------------------------------------------
SgtMayor::SgtMayor (Gtk::Box& parent, Gtk::Statusbar& statusbar, CardSet& cardset,
                    const std::vector<Player*>& player, unsigned int posPlayer,
                    YGP::Mutex& mxSerialize)
   : Game (parent, statusbar, cardset, player, posPlayer, mxSerialize, 7, 8)
     , played (ICardPile::COMPRESSED, ICardPile::SHOWFACE)
     , startPlayer (0)
 {
   TRACE9 ("SgtMayor::SgtMayor (Box&, Statusbar&, CardSet&, const std::vector<Glib::ustring>&)");

   int width (cards.getCard (0).getImageWidth ());
   int height (cards.getCard (0).getImageHeight ());

   // Show and attach card-piles
   changeNames (player);
   for (unsigned int i (0); i < NUM_PLAYERS; ++i) {
      players[i].name.show ();
      attach (players[i].name, COLS_PLAYER[i], COLS_PLAYER[i] + (i ? 2 : 5),
              ROWS_PLAYER[i] + 1, ROWS_PLAYER[i] + 2, Gtk::EXPAND,
              Gtk::SHRINK, 1);

      players[i].won.show ();
      attach (players[i].won, COLS_PLAYER[i], COLS_PLAYER[i] + (i ? 2 : 5),
              ROWS_PLAYER[i] - 2, ROWS_PLAYER[i] - 1, Gtk::EXPAND,
              Gtk::SHRINK, 1);

      players[i].hand.show ();
      attach (players[i].hand, COLS_PLAYER[i], COLS_PLAYER[i] + (i ? 2 : 5),
              ROWS_PLAYER[i], ROWS_PLAYER[i] + 1, Gtk::EXPAND, Gtk::EXPAND, 5);
      TRACE9 ("SgtMayor::SgtMayor () - Attach at: " << COLS_PLAYER[i] << '/'
              << COLS_PLAYER[i] + (i ? 2 : 5) << " - " << ROWS_PLAYER[i] << '/'
              << ROWS_PLAYER[i] + 1);

      players[i].hand.setStyle (i ? ICardPile::QUITE_COMPRESSED : ICardPile::COMPRESSED);
      players[i].hand.setShowOption (i ? ICardPile::SHOWBACK : ICardPile::SHOWFACE);
      players[i].hand.set_size_request (width + 17 * (i ? 7 : 18), height + 5);
      players[i].won.set_size_request (width + 17 * 7, height + 20);
   }

   // Show played area
   played.setStyle (ICardPile::COMPRESSED);
   played.show ();
   attach (played, 2, 5, 3, 4, Gtk::SHRINK, Gtk::SHRINK, 5);
   played.set_size_request (width + 150, height);
}

//-----------------------------------------------------------------------------
/// Destructor
//-----------------------------------------------------------------------------
SgtMayor::~SgtMayor () {
   TRACE9 ("SgtMayor::~SgtMayor ()");
   clean ();
}


//-----------------------------------------------------------------------------
/// Makes the move for the next player.
/// \param player: Actual player
/// \returns \c int: Next player or -1 if end of game
//-----------------------------------------------------------------------------
int SgtMayor::makeMove (unsigned int player) {
   TRACE5 ("SgtMayor::makeMove () - Turn of player " << player);
   Check1 (gameStatus () == PLAYING);
   Check3 (pos2Play == pos1Play);

   if (pos2Play == -1U) {
      pos2Play = pos1Play = findPos2Play (player);
      TRACE8 ("SgtMayor::makeMove (unsigned int) - Going to play card at pos " << pos2Play);
      flipCards2Play (players[player].hand, pos1Play, pos2Play);
   }
   else {
      TRACE9 ("SgtMayor::makeMove (unsigned int) - Playing card at pos " << pos2Play);

      movePile (played, players[player].hand, pos1Play, pos2Play);
      pos1Play = pos2Play = -1U;
      player = calcNextPlayer (player);
   }
   return player;
}

//-----------------------------------------------------------------------------
/// Starts the game by dealing the cards
//-----------------------------------------------------------------------------
void SgtMayor::start () {
   TRACE9 ("SgtMayor::start ()");
   Game::start ();

   Check2 (!played.size ());
   ICardPile pile;
   if (randomizeCardsToPile (pile)) {
      for (unsigned int i (0); i < NUM_PLAYERS; ++i)
         for (unsigned int j (0); j < (cards.size () / NUM_PLAYERS); ++j)
            players[(i - posServer) & 0x3].hand.insertColourSorted (pile.removeTopCard ());

      startPlaying ();
   }
}

//-----------------------------------------------------------------------------
/// Remove cards from everything which can hold them
//-----------------------------------------------------------------------------
void SgtMayor::clean () {
   TRACE9 ("SgtMayor::clean ()");
   for (unsigned int i (0); i < NUM_PLAYERS; ++i) {
      players[i].hand.clear ();
   }

   played.clear ();
   disableHuman ();
   Game::clean ();
}

//-----------------------------------------------------------------------------
/// Shows or hides the cards of the computer player
/// \param open: Flag if cards should be shown or hidden
//-----------------------------------------------------------------------------
void SgtMayor::playOpen (bool open) {
   for (unsigned int i (1); i < NUM_PLAYERS; ++i) {
      players[i].hand.setShowOption (open ? ICardPile::SHOWFACE : ICardPile::SHOWBACK);
      players[i].hand.setStyle (open ? ICardPile::COMPRESSED : ICardPile::QUITE_COMPRESSED);
   }
}

//-----------------------------------------------------------------------------
/// Enables the cards of the human player
/// \returns \c Flag, if time should be continued
/// \remarks Depending of the status of the game (PLAYING2) also the top card
///     of the played pile is enabled
//-----------------------------------------------------------------------------
bool SgtMayor::enableHuman () {
   Check3 (activeCards.empty ());
   TRACE2 ("SgtMayor::enableHuman () - Human has " << players[0].hand.size () << " cards");

   for (int i (players[0].hand.size ()); i;)
      activeCards.push_back
         (players[0].hand[--i]->signal_clicked ().connect
           (bind (slot (*this, (&SgtMayor::cardSelected)), i)));

   return Game::enableHuman ();
}

//-----------------------------------------------------------------------------
/// Callback after clicking on a card in hand
/// \param iCard: Offset of card in hand
//-----------------------------------------------------------------------------
void SgtMayor::cardSelected (unsigned int iCard) {
   TRACE5 ("SgtMayor::cardSelected (unsigned int) - Position " << iCard);
   Check1 (iCard < players[0].hand.size ());
   Check3 (gameStatus () == PLAYING);
   Check3 (pos1Play == -1U);
   Check3 (pos2Play == -1U);

   if (moveSelectedCardToPlayed (0, iCard)) {
      if (getConnectionMgr ().getMode () != YGP::ConnectionMgr::NONE) {
         // Send played card to all clients (if any)
         std::ostringstream msg;
         msg << "Play=" << played[played.size () - 1]->id () << ";Target=0";
         if (getConnectionMgr ().getMode () == YGP::ConnectionMgr::CLIENT)
            ignoreNextMsg = true;
         broadcastMessage (msg.str ());
      }

      setNextPlayer (calcNextPlayer (currentPlayer ()));
      makeNextMoves ();
   }
}

//-----------------------------------------------------------------------------
/// Starts the playing phase of the game
//-----------------------------------------------------------------------------
void SgtMayor::startPlaying () {
   TRACE7 ("SgtMayor::startPlaying ()");

   setGameStatus (PLAYING);

   // Search for the player having the two of spades
   for (unsigned int i (1); i < NUM_PLAYERS; ++i)
      if ((players[i].hand[0]->number () == CardWidget::TWO)
          && (players[i].hand[0]->colour () == CardWidget::SPADES)) {
         TRACE7 ("SgtMayor::startPlaying () - Player " << i << " can exchange");

         Glib::ustring stat (_("Player %1 can exchange the two of spades"));
         stat.replace (stat.find ("%1"), 2,
                       actPlayers[i]->getName ());
         status.pop ();
         status.push (stat);
         break;
      }

   setNextPlayer (startPlayer);
   displayTurn (startPlayer);
   makeNextMoves ();
}

//-----------------------------------------------------------------------------
/// Moves the selected card to the played pile
/// \param player: ID of player
/// \param card: Offset of card to play
/// \returns \c Status of moving; true: Card could be moved; false else
//-----------------------------------------------------------------------------
bool SgtMayor::moveSelectedCardToPlayed (unsigned int player, unsigned int card) {
   TRACE5 ("SgtMayor::moveSelectedCardToPlayed (unsigned int, unsigned int) - Player "
           << player << "; Pos.  " << card);
   Check1 (player < NUM_PLAYERS);
   Check1 (card < players[player].hand.size ());
   Check2 (gameStatus () == PLAYING);

   CardWidget& selCard (*players[player].hand[card]);
   CardWidget::COLOURS playColour (selCard.colour ());

   if (played.size ()) {
      // The same colour must be played again (if available)
      CardWidget::COLOURS colour (played[0]->colour ());
      if ((playColour != colour) && players[player].hand.exists (colour)) {
         Gtk::MessageDialog dlg (_("Play first cards with an equal colour as "
                                   "the first played one!"), Gtk::MESSAGE_ERROR);
         dlg.set_title (PACKAGE " - SgtMayor");
         dlg.run ();
         return false;
      }
   }

   movePile (played, players[player].hand, card, card);
   return true;
}

//-----------------------------------------------------------------------------
/// Calculate the number of cards for each colour
/// \param pile: Pile to inspect
/// \param result: Array of number of cards for earch colour
//-----------------------------------------------------------------------------
unsigned int SgtMayor::findPos2Play (unsigned int player) {
   Check1 (player < NUM_PLAYERS);
   TRACE8 ("SgtMayor::findPos2Play (unsigned int)");

   return 0;
}

//-----------------------------------------------------------------------------
/// Changes the names of the playing people
/// \param newPlayer: Array holding the new player
//-----------------------------------------------------------------------------
void SgtMayor::changeNames (const std::vector<Player*>& newPlayer) {
   Game::changeNames (newPlayer);

   std::vector<Player*> player;
   for (unsigned int i (0); i < NUM_PLAYERS; ++i) {
      player.push_back (actPlayers[(i + posServer) & 0x3]);
      players[i].name.set_text (actPlayers[i]->getName ());
   }
}

//----------------------------------------------------------------------------
/// Changes the names of the playing people
/// \param newPlayer: Array holding the new player
/// \param pile: ID of the pile to return
/// \returns ICardPile*: Pointer to pile to use or NULL
//----------------------------------------------------------------------------
ICardPile* SgtMayor::getPileOfPlayer (unsigned int player, unsigned int pile) {
   return ((player >= NUM_PLAYERS) || pile) ? NULL : &players[player].hand;
}

//----------------------------------------------------------------------------
/// Handles the messages the server might send for the hearts cardgame
/// \param player: ID of player sending the message
/// \param message: Message received from the server
/// \returns bool: True, if message has completey processed
/// \throw std::string: In case of an error an describing text
//----------------------------------------------------------------------------
bool SgtMayor::handleMessage (unsigned int player, const std::string& message) throw (std::string) {
   return Game::handleMessage (player, message);
}
