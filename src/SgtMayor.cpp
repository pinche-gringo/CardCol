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

#include <YGP/Check.h>
#include <YGP/Trace.h>
#include <YGP/ConnMgr.h>
#include <YGP/ANumeric.h>
#include <YGP/Tokenize.h>

#include <gtkmm/menu.h>
#include <gtkmm/statusbar.h>
#include <gtkmm/messagedialog.h>

#include <Player.h>

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
     , pTrump (NULL), startPlayer (rand () % NUM_PLAYERS)
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
      players[i].won.set_size_request (width + 17 * 7, height + 5);
      players[i].won.setStyle (ICardPile::QUITE_COMPRESSED);
      players[i].won.setShowOption (ICardPile::SHOWBACK);
   }

   // Show played area
   played.setStyle (ICardPile::COMPRESSED);
   played.show ();
   attach (played, 2, 5, 4, 5, Gtk::SHRINK, Gtk::SHRINK, 5);
   played.set_size_request (width + 150, height);

   diffTicks[0] = 99;
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

      if ((diffTicks[0] > 5) || (diffTicks[1] > 5) || (diffTicks[2] > 5))
         memset (diffTicks, '\0', sizeof (diffTicks));

      playedCards.reset ();
      memset (playedColours, '\0', sizeof (playedColours));
      makeExchange ();
   }
}

//-----------------------------------------------------------------------------
/// Remove cards from everything which can hold them
//-----------------------------------------------------------------------------
void SgtMayor::clean () {
   TRACE9 ("SgtMayor::clean ()");
   for (unsigned int i (0); i < NUM_PLAYERS; ++i) {
      players[i].hand.clear ();

      for (ICardPile::iterator c (players[i].won.begin ());
           c != players[i].won.end (); ++c)
         (*c)->show ();
      players[i].won.clear ();
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

   for (unsigned int i (0); i < NUM_PLAYERS; ++i) {
      players[i].won.setShowOption (open ? ICardPile::SHOWFACE : ICardPile::SHOWBACK);
      players[i].won.setStyle (open ? ICardPile::COMPRESSED : ICardPile::QUITE_COMPRESSED);

      for (CardVPile::iterator c (players[i].won.begin ());
           c != players[i].won.end (); ++c)
         if (((c - players[i].won.begin ()) % 3) != 2)
            open ? (*c)->show () : (*c)->hide ();
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
   displayTurn (startPlayer);
   setNextPlayer (startPlayer);
   makeNextMoves ();
}

//-----------------------------------------------------------------------------
/// Callback after clicking on a card to exchange bad cards with good ones
/// \param iCard: Offset of card in hand
//-----------------------------------------------------------------------------
void SgtMayor::cardExchange (unsigned int iCard) {
   TRACE5 ("SgtMayor::cardExchange (unsigned int) - Position " << iCard);
   Check3 (diffTicks[0]);

   for (unsigned int i (1); i < NUM_PLAYERS; ++i) {
      if (diffTicks[i] < 0) {
         exchangeCards (0, iCard, i);

         if (!diffTicks[0]) {
            disableHuman ();
            startPlaying ();
         }
         return;
      }
   }
   Check3 (0);
}


//-----------------------------------------------------------------------------
/// Starts the playing phase of the game
//-----------------------------------------------------------------------------
void SgtMayor::startPlaying () {
   TRACE7 ("SgtMayor::startPlaying ()");
   Check3 (!diffTicks[0]);
   Check3 (!diffTicks[1]);
   Check3 (!diffTicks[2]);

   startPlayer = calcNextPlayer (startPlayer);

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
               TRACE9 ("SgtMayor::startPlaying () - Player " << i << " exchanges "
                       << *players[exchgPlayer].hand[exchgCard] << " with " << *pExchange);
               break;
            }
         }

      Glib::ustring needed (_("(needs %1 ticks)"));
      needed.replace (needed.find ("%1"), 2, 1, neededTicks[i]);
      players[(i + startPlayer) % NUM_PLAYERS].neededTicks.set_text (needed);
   }

   // Is a card to exchange?
   if (exchgPlayer != -1U) {
      Check3 (pExchange);
      CardWidget& exchg (players[exchgPlayer].hand.remove (exchgCard));
      unsigned int target (players[exchgPlayer].hand.insertColourSorted (*pExchange));
      pExchange = &exchg;
      pExchange->showBack ();

      Glib::ustring stat (_("Player %1 exchanged the two of spades; "));
      stat.replace (stat.find ("%1"), 2,
                    actPlayers[exchgPlayer]->getName ());

      // Mark the exchanged card, if the human received it
      if (!exchgPlayer) {
         players[0].hand[target]->mark ();
         Glib::signal_timeout ().connect
             (bind (slot (*this, &SgtMayor::unmark), players[0].hand[target]), 1000);
      }

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
   makeNextMoves ();
}

//-----------------------------------------------------------------------------
/// Searches for the card to play
/// \param player: Player to inspect
//-----------------------------------------------------------------------------
unsigned int SgtMayor::findPos2Play (unsigned int player) {
   Check1 (player < NUM_PLAYERS);
   TRACE8 ("SgtMayor::findPos2Play (unsigned int) - Player " << player);

   unsigned int pos (0);
   switch (played.size ()) {
   case 0:
      // Try to find a highest card
      while (pos < players[player].hand.size ()) {
         pos = players[player].hand.findLastEqualColour (pos);
         if (isHighest (*players[player].hand[pos]))
             break;
         ++pos;
      }

      if (pos >= players[player].hand.size ())
         pos = players[player].hand.findLowestCard (pTrump->colour ());
      break;

   case 1:
      pos = players[player].hand.find1EqualOrBiggerByColour (*played[0]);
      if ((pos == -1U)
          || (players[player].hand[pos]->colour () != played[0]->colour ()))
         pos = (players[player].hand.exists (played[0]->colour ())
                ? players[player].hand.find (played[0]->colour ())
                : tryToGetTickWithTrump (players[player].hand));
      else {
         unsigned int highest (players[player].hand.findLastEqualColour (pos));
         pos = (isHighest (*players[player].hand[highest])
                ? highest : players[player].hand.findFirstEqualColour (pos));
      }
      break;

   case 2:
      // The tick is taken by the second player with a trump. Either play a
      // small card or use a bigger trump.
      if ((played[0]->colour () != pTrump->colour ())
          && (played[1]->colour () == pTrump->colour ())) {
         pos = players[player].hand.find (played[0]->colour ());
         if (pos == -1U) {
            pos = players[player].hand.find1EqualOrBiggerByColour (*played[1]);
            if (pos == -1U)
               pos = players[player].hand.findLowestCard (pTrump->colour ());
         }
      }
      else {
         pos = players[player].hand.find1EqualOrBiggerByColour (*played[0]);
         if ((pos == -1U)
             || (players[player].hand[pos]->colour () != played[0]->colour ()))
            pos = (players[player].hand.exists (played[0]->colour ())
                   ? players[player].hand.find (played[0]->colour ())
                   : tryToGetTickWithTrump (players[player].hand));
      }
      break;

   default:
      Check3 (0);
   }
   Check3 (pos < players[player].hand.size ());
   return pos;
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
/// Plays the passed card; find winner and give him the cards at the end of a
/// turn.
/// \param player: Player on turn
/// \param card: Card to play 
/// \returns unsigned int: Next player; or -1U, if end of game
//----------------------------------------------------------------------------
unsigned int SgtMayor::playCard (unsigned int player, unsigned int card) {
   TRACE3 ("SgtMayor::playCard (unsigned int, unsigned int) - Player " << player);

   movePile (played, players[player].hand, card, card);
   playedCards.set (players[player].hand[card]->id ());
   ++playedColours[players[player].hand[card]->colour ()];

   if (played.size () == NUM_PLAYERS) {
      // Find the winner
      CardVPile::const_iterator i (played.begin ());
      CardWidget::NUMBERS nr ((*i)->number ());
      CardWidget::COLOURS col ((*i)->colour ());
      unsigned int bestPlayer (0);

      Check3 (pTrump);
      while (++i != played.end ()) {
         TRACE8 ("SgtMayor::playCard (unsigned int, unsigned int) - Comparing "
                 << (**(i - 1)) << " - " << **i);

         if ((col != pTrump->colour ())
             && ((*i)->colour () == pTrump->colour ())) {
            TRACE9 ("SgtMayor::playCard (unsigned int, unsigned int) - Found trump ");
            col = pTrump->colour ();
            nr = (*i)->number ();
            bestPlayer = i - played.begin ();
            continue;
         }

         if (((*i)->number () > nr) && ((*i)->colour () == col)) {
            TRACE9 ("SgtMayor::playCard (unsigned int, unsigned int) - New best card " << **i);
            nr = (*i)->number ();
            bestPlayer = i - played.begin ();
         }
      }
      TRACE9 ("SgtMayor::playCard (unsigned int, unsigned int) - Calc. winner from "
              << player << " and " << bestPlayer);
      player = (player + 1 + bestPlayer) % NUM_PLAYERS;
      TRACE9 ("SgtMayor::playCard (unsigned int, unsigned int) - Winner " << player);

      // Move the cards to his won pile (but show only one of them)
      while (played.size () > 1) {
         CardWidget& card (played.remove (0));
         card.hide ();
         players[player].won.append (card);
      }
      players[player].won.append (played.removeTopCard ());
      Check3 (played.empty ());
   }
   else
      player = calcNextPlayer (player);

   if (players[player].hand.size ()) {
      displayTurn (player);
      return player;
   }

   unsigned int neededTicks[NUM_PLAYERS] = { 9, 3, 5 };
   player = startPlayer;
   for (unsigned int i (0); i < NUM_PLAYERS; ++i) {
      int madeTicks (players[player].won.size () / 3);
      diffTicks[player] = madeTicks - neededTicks[i];
      TRACE9 ("SgtMayor::playCard (unsigned int, unsigned int) - Player "
              << player << " made " << madeTicks << " = " << diffTicks[player]);

      player = calcNextPlayer (player);
   }

   Glib::ustring stat (_("Game ended; %1 has %2 ticks, %3 %4 and %5 %6 ticks"));
   stat.replace (stat.find ("%1"), 2, actPlayers[0]->getName ());
   stat.replace (stat.find ("%3"), 2, actPlayers[1]->getName ());
   stat.replace (stat.find ("%5"), 2, actPlayers[2]->getName ());

   stat.replace (stat.find ("%2"), 2, YGP::ANumeric::toString (diffTicks[0]));
   stat.replace (stat.find ("%4"), 2, YGP::ANumeric::toString (diffTicks[1]));
   stat.replace (stat.find ("%6"), 2, YGP::ANumeric::toString (diffTicks[2]));

   status.pop ();
   status.push (stat);
   setGameStatus (STOPPED);
   return -1U;
}

//----------------------------------------------------------------------------
/// Checks if the passed card is the highest card of its colours, which has
/// not been played.
/// \param card: Card to inspect 
/// \return bool: True, if card is the highest unplayed one 
//----------------------------------------------------------------------------
bool SgtMayor::isHighest (const CardWidget& card) const {
   TRACE8 ("SgtMayor::isHighest (const CardWidget&) - " << card << " = " << card.id ());

   int id (card.id () - 4);
   while (id >= 0) {
      if (!playedCards[id])
         return false;
      id -= 4;
   }
   return true;
}

//----------------------------------------------------------------------------
/// Tries to get the tick with a trump card; returns a bad card, if there's no
/// trump.
/// \param pile: Pile to play from 
/// \return unsigned int: Position of card to play 
//----------------------------------------------------------------------------
unsigned int SgtMayor::tryToGetTickWithTrump (const ICardPile& pile) const {
   TRACE8 ("SgtMayor::tryToGetTickWithTrump (const ICardPile&)  - Size" << pile.size ());
   Check3 (pile.size ());

   unsigned int pos (pile.find (pTrump->colour ()));
   if (pos == -1U)
      pos = pile.findLowestCard (pTrump->colour ());
   return pos;
}

//----------------------------------------------------------------------------
/// Exchanges cards between players having too much/too less ticks in the last
/// round.
//----------------------------------------------------------------------------
void SgtMayor::makeExchange () {
   TRACE5 ("SgtMayor::makeExchange () - Exchanging cards");

   for (unsigned int i (1); i < NUM_PLAYERS; ++i)
      while (diffTicks[i] > 0)
         for (unsigned int j (1); j < NUM_PLAYERS; ++j)
            if (diffTicks[(j + i) % 3] < 0)
               exchangeCards (i, (j + i) % 3);

   if (diffTicks[0] > 0) {
      Glib::ustring msg (_("You can give away %1 bad cards (which are "
                           "exchanged with good ones). Click on each of them!"));
      msg.replace (msg.find ("%1"), 2, YGP::ANumeric::toString (diffTicks[0]));
      status.pop ();
      status.push (msg);

      for (int i (players[0].hand.size ()); i;)
         activeCards.push_back
            (players[0].hand[--i]->signal_clicked ().connect
             (bind (slot (*this, (&SgtMayor::cardExchange)), i)));
   }
   else
      startPlaying ();
}

//----------------------------------------------------------------------------
/// Exchanges a good card from playerGood with a bad card from player bad
/// \param playerBad: Player giving away a bad card 
/// \param playerGood: Player giving away a good card 
//----------------------------------------------------------------------------
void SgtMayor::exchangeCards (unsigned int playerBad, unsigned int playerGood) {
   TRACE7 ("SgtMayor::exchangeCards (unsigned int, unsigned int) - Players "
           << playerBad << " and " << playerGood);
   Check1 (playerBad < NUM_PLAYERS);
   Check1 (playerGood < NUM_PLAYERS);

   unsigned int posBad (players[playerBad].hand.findLowestCard ());
   Check3 (posBad < players[playerBad].hand.size ());
   exchangeCards (playerBad, posBad, playerGood);
}


//----------------------------------------------------------------------------
/// Exchanges a good card from playerGood with a bad card from player bad
/// \param playerBad: Player giving away a bad card 
/// \param playerGood: Player giving away a good card 
//----------------------------------------------------------------------------
void SgtMayor::exchangeCards (unsigned int playerBad, unsigned int posBad,
                              unsigned int playerGood) {
   TRACE7 ("SgtMayor::exchangeCards (3x unsigned int int) - Players "
           << playerBad << " and " << playerGood);
   Check1 (playerBad < NUM_PLAYERS);
   Check1 (playerGood < NUM_PLAYERS);
   Check1 (posBad < players[playerBad].hand.size ());

   unsigned int posGood (players[playerGood].hand.findLastEqualOrBiggerColour
                         (players[playerBad].hand[posBad]->colour ()));
   if (posGood == -1U)
      posGood = players[playerGood].hand.findLowestCard ();
   TRACE9 ("SgtMayor::exchangeCards (3x unsigned int) - Player "
           << playerBad << " exchanges card " << posBad << " with "
           << playerGood << "'s " << posGood);
   Check3 (posGood < players[playerGood].hand.size ());

   CardWidget& bad (players[playerBad].hand.remove (posBad));
   CardWidget& good (players[playerGood].hand.remove (posGood));
   posBad = players[playerBad].hand.insertColourSorted (good);
   posGood = players[playerGood].hand.insertColourSorted (bad);
   TRACE9 ("SgtMayor::exchangeCards (3x unsigned int ) - Player "
           << playerBad << " and " << playerGood << " exchange " << bad
           << " and " << good);

   --diffTicks[playerBad];
   ++diffTicks[playerGood];

   if (!playerBad) {
      playerGood = 0;
      posGood = posBad;
   }
   if (!playerGood) {
      players[0].hand[posGood]->mark ();
      Glib::signal_timeout ().connect
         (bind (slot (*this, &SgtMayor::unmark), players[0].hand[posGood]), 1000);
   }
}

//----------------------------------------------------------------------------
/// Unmarks the passed card
/// \param card: Card to unmark 
/// \return bool: Always false to end the timer 
//----------------------------------------------------------------------------
bool SgtMayor::unmark (const CardWidget* card) {
   Check1 (card);
   TRACE9 ("SgtMayor::unmark (unsigned int) - Unmarking " << *card);
   Check3 (players[0].hand.findByColour (*card) < players[0].hand.size ());

   players[0].hand[players[0].hand.findByColour (*card)]->unmark ();
   return false;
}
