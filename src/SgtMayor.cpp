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
const unsigned int SgtMayor::ROWS_PLAYER[NUM_PLAYERS] = { 1, 6, 6 };


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
   : Game (parent, statusbar, cardset, player, posPlayer, mxSerialize, 10, 8)
     , played (ICardPile::COMPRESSED, ICardPile::SHOWFACE), pExchange (NULL)
     , pTrump (NULL), startPlayer (rand () % 3)
 {
   TRACE9 ("SgtMayor::SgtMayor (Box&, Statusbar&, CardSet&, ...)");

   int width (cards.getCard (0).getImageWidth ());
   int height (cards.getCard (0).getImageHeight ());

   // Show and attach card-piles
   changeNames (player);
   for (unsigned int i (0); i < NUM_PLAYERS; ++i) {
      players[i].name.show ();
      attach (players[i].name, COLS_PLAYER[i], COLS_PLAYER[i] + (i ? 2 : 5),
              ROWS_PLAYER[i] + 1, ROWS_PLAYER[i] + 2, Gtk::EXPAND,
              Gtk::SHRINK, 1, 2);

      players[i].neededTicks.show ();
      attach (players[i].neededTicks, COLS_PLAYER[i],
              COLS_PLAYER[i] + (i ? 2 : 5), ROWS_PLAYER[i] + 2,
              ROWS_PLAYER[i] + 3, Gtk::EXPAND, Gtk::SHRINK, 1, 5);

      players[i].won.show ();
      attach (players[i].won, COLS_PLAYER[i], COLS_PLAYER[i] + (i ? 2 : 5),
              ROWS_PLAYER[i] - 1, ROWS_PLAYER[i], Gtk::EXPAND,
              Gtk::SHRINK, 1, 5);

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
   attach (played, 2, 5, 4, 5, Gtk::SHRINK, Gtk::SHRINK, 5);
   played.set_size_request (width + 150, height);
}

//-----------------------------------------------------------------------------
/// Destructor
//-----------------------------------------------------------------------------
SgtMayor::~SgtMayor () {
   TRACE9 ("SgtMayor::~SgtMayor ()");
   clean ();
   TRACE9 ("SgtMayor::~SgtMayor () - Done");
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

      player = playCard (player, pos1Play);
      pos1Play = pos2Play = -1U;
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
      Check3 (cards.size () == 52);
      for (unsigned int i (0); i < NUM_PLAYERS; ++i)
         for (unsigned int j (0); j < (cards.size () / NUM_PLAYERS); ++j)
            players[(i - posServer) & 0x3].hand.insertColourSorted (pile.removeTopCard ());

      // Add the card which can be exchanged by the two of spades
      Check3 (!pExchange);
      pExchange = &pile.removeTopCard ();

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

   pExchange = NULL;

   if (pTrump) {
      remove (*pTrump);
      delete pTrump;
      pTrump = NULL;
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
   Check2 (gameStatus () == PLAYING);
   Check2 (pos1Play == -1U);
   Check2 (pos2Play == -1U);

   CardWidget& selCard (*players[0].hand[iCard]);
   CardWidget::COLOURS playColour (selCard.colour ());

   if (played.size ()) {
      // The same colour must be played again (if available)
      CardWidget::COLOURS colour (played[0]->colour ());
      if ((playColour != colour) && players[0].hand.exists (colour)) {
         Gtk::MessageDialog dlg (_("Play first cards with an equal colour as "
                                   "the first played one!"), Gtk::MESSAGE_ERROR);
         dlg.set_title (PACKAGE " - SgtMayor");
         dlg.run ();
         return;
      }
   }

   if (getConnectionMgr ().getMode () != YGP::ConnectionMgr::NONE) {
      // Send played card to all clients (if any)
      std::ostringstream msg;
      msg << "Play=" << played[played.size () - 1]->id () << ";Target=0";
      if (getConnectionMgr ().getMode () == YGP::ConnectionMgr::CLIENT)
         ignoreNextMsg = true;
      broadcastMessage (msg.str ());
   }

   unsigned int player (playCard (0, iCard));
   if (player != -1U) {
      setNextPlayer (player);
      makeNextMoves ();
   }
   else
      disableHuman ();
}

//-----------------------------------------------------------------------------
/// Callback after clicking on a card to select the special colour
/// \param iCard: Offset of card in hand
//-----------------------------------------------------------------------------
void SgtMayor::cardColourSelect (unsigned int iCard) {
   TRACE5 ("SgtMayor::cardColourSelect (unsigned int) - Position " << iCard);
   Check1 (iCard < players[0].hand.size ());
   Check2 (gameStatus () == PLAYING);

   showTrump (players[0].hand[iCard]->colour ());
   disableHuman ();
   displayTurn (0);

   Glib::signal_idle ().connect
       (bind_return (slot (*this, &SgtMayor::enableHuman), false));
}

//-----------------------------------------------------------------------------
/// Starts the playing phase of the game
//-----------------------------------------------------------------------------
void SgtMayor::startPlaying () {
   TRACE7 ("SgtMayor::startPlaying ()");

   char neededTicks[NUM_PLAYERS] = { '9', '3', '5' };
   unsigned int exchgPlayer (-1U);
   unsigned int exchgCard (0);

   // Search for the player having the two of spades
   for (unsigned int i (0); i < NUM_PLAYERS; ++i) {
      if (exchgPlayer == -1U)
         for (exchgCard = 0; exchgCard < players[i].hand.size (); ++exchgCard) {
            if ((players[i].hand[exchgCard]->number () == CardWidget::TWO)
                && (players[i].hand[exchgCard]->colour () == CardWidget::SPADES)) {
               exchgPlayer = i;
               break;
            }
         }

      Glib::ustring needed ("(needs %1 ticks)");
      needed.replace (needed.find ("%1"), 2, 1,
                      neededTicks[(i + startPlayer) % NUM_PLAYERS]);
      players[i].neededTicks.set_text (needed);
   }

   // Is a card to exchange
   if (exchgPlayer != -1U) {
      Check3 (pExchange);
      players[exchgPlayer].hand.insertColourSorted (*pExchange);
      pExchange = &players[exchgPlayer].hand.remove (exchgCard);
      pExchange->showBack ();

      Glib::ustring stat (_("Player %1 exchanged the two of spades; "));
      stat.replace (stat.find ("%1"), 2,
                    actPlayers[exchgPlayer]->getName ());

      if (startPlayer)
         displayTurn (startPlayer, stat);
      else {
         stat += _("Select the special colour by clicking on a card");

         status.pop ();
         status.push (stat);

         for (int i (players[0].hand.size ()); i;)
            activeCards.push_back
               (players[0].hand[--i]->signal_clicked ().connect
                (bind (slot (*this, (&SgtMayor::cardColourSelect)), i)));
         return;
      }
   }
   else
      displayTurn (startPlayer, _("Nobodoy can exchange the two of clubs; "));


   // Find special colour
   ICardPile& pile (players[startPlayer].hand);
   int number[] = { 0, 0, 0, 0 };
   int points[] = { 0, 0, 0, 0 };

   for (unsigned int i (0); i < (pile.size () - 1); ++i) {
      ++number[pile[i]->colour ()];
      points[pile[i]->colour ()] += pile[i]->number () + 1;
   }

   unsigned int trumpColour (0);
   for (unsigned int i (1); i < 4; ++i) {
      if ((number[i] > number[i - 1])
          || ((number[i] == number[i - 1]) && (points[i] > points[i - 1])))
         trumpColour = i;
   }
   showTrump ((CardWidget::COLOURS)trumpColour);

   setNextPlayer (startPlayer);
   startPlayer = calcNextPlayer (startPlayer);
   makeNextMoves ();
}

//-----------------------------------------------------------------------------
/// Searches for the card to play
/// \param player: Player to inspect
//-----------------------------------------------------------------------------
unsigned int SgtMayor::findPos2Play (unsigned int player) {
   Check1 (player < NUM_PLAYERS);
   TRACE8 ("SgtMayor::findPos2Play (unsigned int) - Player " << player);

   
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

//----------------------------------------------------------------------------
/// Shows the special colour on the board (the ace with that colour)
/// \param colour: The special colour to display 
//----------------------------------------------------------------------------
void SgtMayor::showTrump (CardWidget::COLOURS colour) {
   TRACE9 ("SgtMayor::showTrump (CardWidget::COLOURS) - " << colour);

   for (unsigned int i (0); i < cards.size (); ++i) {
      if ((cards.getCards ()[i]->number () == CardWidget::ACE)
          && (cards.getCards ()[i]->colour () == colour)) {
         Check3 (!pTrump);
         pTrump = new CardWidget (*cards.getCards ()[i]);
         pTrump->show ();
         pTrump->showFace ();
         attach (*pTrump, 0, 1, 0, 1, Gtk::SHRINK, Gtk::SHRINK, 5, 1);
         return;
      }
   }
   Check3 (0);
}

//----------------------------------------------------------------------------
/// Plays the passed card
/// \param player: Player on turn
/// \param card: Card to play 
/// \returns unsigned int: Next player; or -1U, if end of game
//----------------------------------------------------------------------------
unsigned int SgtMayor::playCard (unsigned int player, unsigned int card) {
   TRACE9 ("SgtMayor::playCard (unsigned int, unsigned int) - Player " << player);

   movePile (played, players[player].hand, card, card);

   if (played.size () == NUM_PLAYERS) {
      CardVPile* wonPile (new CardVPile (ICardPile::VERY_COMPRESSED,
                                         ICardPile::SHOWBACK));
      wonPile->show ();

      while (played.size ())
         wonPile->insert (played.removeTopCard (), 0);

      players[player].won.pack_start (*wonPile);
      Check3 (played.empty ());
   }

   displayTurn (player = calcNextPlayer (player));
   return player;
}
