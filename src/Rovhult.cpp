//PROJECT     : Cardgames
//SUBSYSTEM   : Rovhult
//REFERENCES  :
//TODO        :
//BUGS        :
//REVISION    : $Revision$
//AUTHOR      : Markus Schwab
//CREATED     : 28.3.2002
//COPYRIGHT   : Copyright (C) 2002 - 2018, 2024

// This file is part of CardCol.
//
// CardCol is free software: you can redistribute it and/or modify
// it under the terms of the GNU General Public License as published by
// the Free Software Foundation, either version 3 of the License, or
// (at your option) any later version.
//
// CardCol is distributed in the hope that it will be useful,
// but WITHOUT ANY WARRANTY; without even the implied warranty of
// MERCHANTABILITY or FITNESS FOR A PARTICULAR PURPOSE.  See the
// GNU General Public License for more details.
//
// You should have received a copy of the GNU General Public License
// along with CardCol.  If not, see <http://www.gnu.org/licenses/>.


#include <cardgames-cfg.h>

#include <sstream>

#include <glibmm/main.h>
#include <glibmm/value.h>

#include <gdkmm/drag.h>
#include <gdkmm/texture.h>
#include <gdkmm/contentprovider.h>

#include <gtkmm/statusbar.h>
#include <gtkmm/dragsource.h>
#include <gtkmm/droptarget.h>
#include <gtkmm/messagedialog.h>

#include <XGP/XDialog.h>

#include <YGP/Check.h>
#include <YGP/Trace.h>
#include <YGP/ConnMgr.h>

#include <CardValue.h>

#include <card/Widget.h>
#include <card/Window.h>
#include <card/ComputerPlayer.h>

#include "Rovhult.h"


const unsigned int Rovhult::COLS_PLAYER[NUM_PLAYERS] = { 7, 13, 7, 1 };
const unsigned int Rovhult::ROWS_PLAYER[NUM_PLAYERS] = { 13, 7, 4, 7 };


Card::Widget::NUMBERS Rovhult::cardNuke(Card::Widget::TEN);
Card::Widget::NUMBERS Rovhult::cardSkip(Card::Widget::EIGHT);
Card::Widget::NUMBERS Rovhult::cardReverse(Card::Widget::SEVEN);


//-----------------------------------------------------------------------------
/// Defaultconstructor; all widgets are created
/// \param parent Parent widget (box) to display the game in
/// \param statusbar Status bar widget to display information about the game
/// \param cardset Cardset to use
/// \param names Vector of players
/// \param posPlayer Position of player for the server
/// \param mxSerialize Mutex to serialize messages from the server
//-----------------------------------------------------------------------------
Rovhult::Rovhult(Gtk::Box& parent, Gtk::Statusbar& statusbar,
                 Card::Set& cardset, const std::vector<Card::Player*>& player,
                 unsigned int posPlayer, YGP::Mutex& mxSerialize)
   : Game(parent, statusbar, cardset, player, posPlayer, mxSerialize, 16, 20),
     played(Card::IPile::VERY_COMPRESSED, Card::IPile::SHOWFACE),
     staple(Card::IPile::VERY_COMPRESSED), aExchanged(0), cEndgame(0),
     aTableDND(), aHandDND(), aHandData(), aTableData() {
    TRACE9("Rovhult::Rovhult(Gtk::Box& Gtk::Statusbar&, CardSet&, const std::vector<Glib::ustring>&)");

   staple.show();
   attach(staple, 3, 2, 1, 5);
   Check3(cards.size());

   // Show and attach card-piles
   changeNames(player);
   for (unsigned int i(0); i < NUM_PLAYERS; ++i) {
      for (int j(0); j < 3; ++j) {
         players[i].reserve[j].setStyle(Card::IPile::QUITE_COMPRESSED);
         players[i].reserve[j].show();
         attach(players[i].reserve[j], COLS_PLAYER[i] + (j << 1), ROWS_PLAYER[i], 1, 2);
         TRACE9("Rovhult::Rovhult() - Set at: " << COLS_PLAYER[i]  + (j << 1) << '/' << ROWS_PLAYER[i]);
      }

      players[i].name.show();
      players[i].name.set_hexpand(); players[i].name.set_vexpand();
      players[i].name.set_margin(1);
      attach(players[i].name, COLS_PLAYER[i], ROWS_PLAYER[i] + ((i == 2) ? 2 : 5), 5, 1);

      players[i].hand.setStyle(i ? Card::IPile::QUITE_COMPRESSED : Card::IPile::NORMAL);
      players[i].hand.setShowOption(i ? Card::IPile::SHOWBACK : Card::IPile::SHOWFACE);
      players[i].hand.show();
      players[i].hand.set_hexpand(); players[i].hand.set_vexpand();
      attach(players[i].hand, COLS_PLAYER[i], ROWS_PLAYER[i] + ((i == 2) ? -3 : 3), 5, 2);
      TRACE9("Rovhult::Rovhult() - 2nd set at: " << COLS_PLAYER[i] << '/' << ROWS_PLAYER[i] + ((i == 2) ? -3 : 3));
   }

   resizeCards();
   played.show();
   staple.setShowOption(Card::IPile::SHOWBACK);

   played.set_margin(1);
   attach(played, 7, 5, 4, 9);
}

//-----------------------------------------------------------------------------
/// Destructor
//-----------------------------------------------------------------------------
Rovhult::~Rovhult() {
   TRACE8("Rovhult::~Rovhult()");
   clean();
}


//-----------------------------------------------------------------------------
/// Starts the game
//-----------------------------------------------------------------------------
void Rovhult::start() {
   Game::start();

   setGameStatus(EXCHANGE);
   TRACE1("Rovhult::start() - randomiseCards");
   if (randomiseCardsToPile(staple)) {
      TRACE1("Rovhult::start() - randomiseCards finished");
      dealCards();
      TRACE1("Rovhult::start() - cards dealt");

      aExchanged = 0;

      if (getConnectionMgr().getMode() != YGP::ConnectionMgr::CLIENT) {
         setNextPlayer(rand() & 0x3);
	 broadcastStartPlayer(currentPlayer());
      }

      cEndgame = 0;
   }
}

//-----------------------------------------------------------------------------
/// Callback after finishing card-exchange
/// \param iCard Offset of card in hand
//-----------------------------------------------------------------------------
void Rovhult::finishedExchange(unsigned int iCard) {
   unregisterDND();
   disableHuman();
   setGameStatus(EXCHANGED);

   sortReserve(0);
   players[0].hand.setStyle(Card::IPile::COMPRESSED);

   sendExchangedCards(0);

   exchangeAutoplayerCards();

   players[0].hand.sortByNumber();
   sortReserve(0);
   players[0].hand.setStyle(Card::IPile::COMPRESSED);

   status.pop();
   displayTurn(currentPlayer());

   if ((getConnectionMgr().getMode() == YGP::ConnectionMgr::NONE) || (aExchanged == 0xf)) {
      setGameStatus(PLAYING);
      makeNextMoves();
   }
   else
      status.push(_("Waiting for other player to exchange their cards ..."));
}

//-----------------------------------------------------------------------------
/// Values a card; this ranges from 3 to 9, J, K, A, 2, 10
/// \param card Card to value
/// \returns unsigned int Value representing the card
//-----------------------------------------------------------------------------
unsigned int Rovhult::getCardValue(const Card::Widget& card) {
   return ((card.number () == Card::Widget::TWO)
	   ? Card::Widget::ACE + 1
	   : (card.number () == cardNuke) ? Card::Widget::ACE + 2 : card.number());
}

//-----------------------------------------------------------------------------
/// Compares two cards according the rules of Rovhult
/// \param lhs, rhs Cards to compare
/// \returns int >0, if number of lhs is smaller; 0 if equal or >0 if
///     bigger
//-----------------------------------------------------------------------------
int Rovhult::compareCards(const Card::Widget& lhs, const Card::Widget& rhs) {
   return getCardValue(lhs) - getCardValue(rhs);
}

//-----------------------------------------------------------------------------
/// Exchanges the cards of the computer-players
/// \param player Not really a void*, but actually the (next computer)player
//-----------------------------------------------------------------------------
void Rovhult::exchangeAutoplayerCards () {
   if (getConnectionMgr ().getMode () != YGP::ConnectionMgr::CLIENT)
      for (unsigned int i (getConnectionMgr ().getClients ().size () + 1);
           i < NUM_PLAYERS; ++i) {
         for (unsigned int j (0); j < 3; ++j) {
            unsigned int posPile (0);
            unsigned int posHand (0);

            // Search for smallest card in pile and biggest in hand
            for (unsigned int k (1); k < 3; ++k) {
               if (compareCards (players[i].reserve[k].getTopCard (),
                                 players[i].reserve[posPile].getTopCard ()) < 0)
                   posPile = k;

               if (compareCards (*players[i].hand[k], *players[i].hand[posHand]) > 0)
                   posHand = k;
            }

            // and exchange them, if hand is bigger than pile
            if (compareCards (players[i].reserve[posPile].getTopCard (),
                              *players[i].hand[posHand]) < 0) {
                Card::Widget& cardPile (players[i].reserve[posPile].removeTopCard ());
                Card::Widget& cardHand (players[i].hand.remove (posHand, true));

                TRACE3 ("Rovhult::exchangeAutoplayerCards () - exchanging card "
                        << cardHand << " in hand (" << posHand << ") with card on pile "
                        << posPile << " (" << cardPile << ')');
                // Swap cards
                players[i].reserve[posPile].setTopCard (cardHand);
                players[i].hand.insert (cardPile, posHand);
            }
         }
         Check3 (players[i].hand.size () == 3);
         Check3 (players[i].reserve[0].size () == 2);
         Check3 (players[i].reserve[1].size () == 2);
         Check3 (players[i].reserve[2].size () == 2);
         players[i].hand.sortByNumber ();
         sortReserve (i);

         sendExchangedCards (i);
      }
}

//----------------------------------------------------------------------------
/// Broadcasts the exchanged cards to the clients
/// \param player Player whose cards to send
//----------------------------------------------------------------------------
void Rovhult::sendExchangedCards (unsigned int player) {
   // Send starting positions to the clients
   if (getConnectionMgr ().getMode () != YGP::ConnectionMgr::NONE) {
      std::ostringstream msg;
      msg << "Exchange=" << players[player].hand[0]->id () << ' '
          << players[player].hand[1]->id () << ' '
          << players[player].hand[2]->id () << ' '
          << players[player].reserve[0].getTopCard ().id () << ' '
          << players[player].reserve[1].getTopCard ().id () << ' '
          << players[player].reserve[2].getTopCard ().id () << ";Player="
          << ((player + posServer) & 0x3);

      if (getConnectionMgr ().getMode () == YGP::ConnectionMgr::CLIENT)
         ignoreNextMsg = true;

      broadcastMessage (msg.str ());
      aExchanged |= (1 << player);
   }
}

//-----------------------------------------------------------------------------
/// Sorts the cards on the reserve piles
/// \param player Player whose cards should be sorted
//-----------------------------------------------------------------------------
void Rovhult::sortReserve (unsigned int player) {
   // Sort cards on piles
   for (int j (0); j < 2; ++j)
      for (int k (j); k >= 0; --k)
         if (compareCards (players[player].reserve[k + 1].getTopCard (),
                           players[player].reserve[k].getTopCard ()) < 0) {
            Card::Widget& low (players[player].reserve[k + 1].removeTopCard ());
            Card::Widget& high (players[player].reserve[k].removeTopCard ());

            TRACE3 ("Rovhult::exchangeAutoplayerCards () - exchanging card "
                    << low << " on pile " << (k + 1) << " with card " << high
                    << " on pile " << k);

            players[player].reserve[k + 1].setTopCard (high);
            players[player].reserve[k].setTopCard (low);
         }
}

//-----------------------------------------------------------------------------
/// Enables the cards of the human player
/// \param player Player to enable
//-----------------------------------------------------------------------------
bool Rovhult::enableHuman () {
   Check3 (activeCards.empty ());

   if (players[0].hand.size ()) {
      TRACE2 ("Rovhult::enableHuman () - Has " << players[0].hand.size ()
              << " card(s) in the hand");

      for (int i (players[0].hand.size () - 1); i >= 0; --i)
         activeCards.push_back
            (players[0].hand[i]->signal_clicked ().connect
             (bind (mem_fun (*this, &Rovhult::handSelected), i)));
   }
   else {
      TRACE2 ("Rovhult::enableHuman () - Enable reserve of human");

      for (int i (0); i < 3; ++i)
         if (players[0].reserve[i].size ()) {
            TRACE8 ("Rovhult::enableHuman () - Pile " << i << " has "
                    << players[0].reserve[i].size () << " card(s)");
            activeCards.push_back
               (players[0].reserve[i].getTopCard ().signal_clicked ().connect
                (bind (mem_fun (*this, &Rovhult::pileSelected), i)));
         }
   }

   if (played.size ()) {
      TRACE2 ("Rovhult::enablePlayer (unsigned int) - Enable last played card");
      activeCards.push_back (played.getTopCard ().signal_clicked ().connect
                             (mem_fun (*this, &Rovhult::takeCards)));
   }
   return Game::enableHuman ();
}

//-----------------------------------------------------------------------------
/// Callback after clicking on a card on the table
/// \param pile Offset of selected pile
//-----------------------------------------------------------------------------
void Rovhult::pileSelected (unsigned int pile) {
   TRACE1 ("Rovhult::pileSelected (unsinged int) - Pile " << pile);
   Check3 (pile < 3);

   Card::IPile& actPile (players[0].reserve[pile]);
   Card::Widget& card (actPile.getTopCard ());
   bool showsFace (card.showsFace ());

   // If played from bottom of pile (with invisible cards): Flip card first
   if (!showsFace) {
      for (unsigned int i (0); i < 3; ++i)
          if (players[0].reserve[i].size ()
              && players[0].reserve[i].getTopCard ().showsFace ()) {
             Gtk::MessageDialog dlg (_("You must first play the visible cards!"),
                                     false, Gtk::MessageType::ERROR);
             dlg.set_title (_("Invalid move"));
             XGP::runModal (dlg);
             return;
          }
      card.showFace ();
   }
   TRACE1 ("Rovhult::pileSelected (unsinged int) - Card " << card);

   if (!cardValid (card.number ())) {  // If selected card is not valid: Return
      if (!showsFace) {
         played.Card::IPile::append (actPile.removeTopCard ());

         // Inform the others about the move
         if (getConnectionMgr ().getMode () != YGP::ConnectionMgr::NONE) {
            // Send played card to all clients (if any)
            std::ostringstream msg;
            msg << "Play=" << card.id () << ";Target=" << (pile + 5);
            if (getConnectionMgr ().getMode () == YGP::ConnectionMgr::CLIENT)
                ignoreNextMsg = true;
            broadcastMessage (msg.str ());
         }
         movePlayedCardsToLoser (0);
      }
      return;
   }

   // Inform the others about the move
   if (getConnectionMgr ().getMode () != YGP::ConnectionMgr::NONE) {
      // Send played card to all clients (if any)
      std::ostringstream msg;
      msg << "Play=" << card.id () << ";Target=" << (pile + 1);
      if (getConnectionMgr ().getMode () == YGP::ConnectionMgr::CLIENT)
         ignoreNextMsg = true;
      broadcastMessage (msg.str ());
   }

   // If face of card was visible: Just go on (as the user knows what he has
   // selected); if not: Continue after GUI update.
   if (showsFace)
      doPileSelected (0, pile);
   else
      Glib::signal_timeout ().connect (bind (mem_fun (*this, &Rovhult::doPileSelected),
                                             0, pile), Card::ComputerPlayer::TIMEOUT);
}

//-----------------------------------------------------------------------------
/// Executes the move from a pile: Moves the cards and enables next
/// \param player ID of player
/// \param pile Offset of selected pile
/// \returns bool false
//-----------------------------------------------------------------------------
bool Rovhult::doPileSelected (unsigned int player, unsigned int pile) {
   TRACE1 ("Rovhult::doPileSelected (unsigned int, unsinged int) - "
           << player << '/' << pile);
   Check1 (player < NUM_PLAYERS); Check1 (pile < 3);

   Card::IPile* actPile (&players[player].reserve[pile]);
   Check3 (actPile->size ());
   Card::Widget& card (actPile->getTopCard ());
   card.showFace ();
   Card::PileWindows& animPiles (animateCards2 (played, *actPile, actPile->size () - 1, actPile->size () - 1));
   animPiles.sigAnimation.connect (bind (mem_fun (*this, &Rovhult::executeMove), player));

   while (pile) {
      actPile = &players[player].reserve[--pile];
      if (actPile->size ()
          && actPile->topCardShowsFace ()
          && (actPile->getTopCard ().number () == card.number ())) {
         Card::Widget& sameCard (actPile->getTopCard ());
	 sameCard.showFace ();
	 animPiles.addWindow (*actPile, actPile->size () - 1, actPile->size () - 1);
      }
      else
         break;
   }

   return false;
}

//-----------------------------------------------------------------------------
/// Check if played card is valid (equal or bigger) The following cards have
/// special meaning:
///   - 2: Can be played always
///   - cardReverse (7): The next card must be equal or *smaller*
///   - cardSkip (8): Skips the next player
///   - cardNuke (10): Clears the staple; the
/// same player can continue with cards in hand
/// \param nr Card to check
/// \param silent Flag, if error should be displayed
/// \returns bool True, if card can be played
//-----------------------------------------------------------------------------
bool Rovhult::cardValid (Card::Widget::NUMBERS nr, bool silent) const {
   TRACE5 ("Rovhult::cardValid (Card::Widget::NUMBERS, bool) const - Checking "
           << nr << " in " << played.size () << " cards");

   if ((nr != Card::Widget::TWO) && (nr != cardNuke)) {
      if (played.size ()) {
         Glib::ustring error;

         Card::Widget& lastPlayed (played.getTopCard ());
         if (lastPlayed.number () == cardReverse) {
            if (nr > cardReverse) {
               error = _("After a %1, the played card must be equal or smaller!");
	       error.replace (error.find ("%1"), 2, CardValue::get ()[cardReverse]);
	    }
         }
         else
            if (nr < lastPlayed.number ())
               error = _("The played card must be equal or bigger!");

         if (error.size ()) {
            if (!silent) {
               Gtk::MessageDialog dlg (error, false, Gtk::MessageType::ERROR);
               dlg.set_title (_("Invalid move"));
               XGP::runModal (dlg);
            }
            return false;
         }
      }
   }
   return true;
}

//-----------------------------------------------------------------------------
/// Callback after clicking on a card in hand
/// \param pos Offset of card in hand
//-----------------------------------------------------------------------------
void Rovhult::handSelected (unsigned int pos) {
   TRACE3 ("Rovhult::handSelected (unsinged int) - Checking card at " << pos);
   Check3 (pos < players[0].hand.size ());

   Card::Widget& card (*players[0].hand[pos]);
   TRACE1 ("Rovhult::handSelected (unsinged int) - Card " << pos << " = " << card);

   if (!cardValid (card.number ()))
       return;

   // Inform the others about the move
   unsigned int start (players[0].hand.findFirstEqual (pos));
   if (getConnectionMgr ().getMode () != YGP::ConnectionMgr::NONE) {
      // Send played card to all clients (if any)
      std::ostringstream msg;
      msg << "Play=";
      for (unsigned int i (start); i < pos; ++i)
         msg << players[0].hand[i]->id () << ' ';
      msg << players[0].hand[pos]->id () << ";Target=0";

      if (getConnectionMgr ().getMode () == YGP::ConnectionMgr::CLIENT)
         ignoreNextMsg = true;
      broadcastMessage (msg.str ());
   }

   playCardsFromHand (0, start, pos);
}

//-----------------------------------------------------------------------------
/// Move card (and cards with equal number below) from player to played
/// staple. The cards are replaced, if the staple contains cards
/// \param player ID of player who played the last card
/// \param start Offset of first card in hand to play
/// \param end Offset of last card in hand to play
//-----------------------------------------------------------------------------
void Rovhult::playCardsFromHand (unsigned int player, unsigned int start,
                                                unsigned int end) {
   TRACE5 ("Rovhult::playCardsFromHand (unsigned int, unsigned int, unsigned int)"
           " - Player " << player << " from " << start << " to " << end);
   Check3 (end < players[player].hand.size ());
   Check3 (start <= end);
   animateCards (played, players[player].hand, start, end)
      .sigAnimation.connect (bind (mem_fun (*this, &Rovhult::executeMove), player));
}

//-----------------------------------------------------------------------------
/// Unmarks any played card and moves the played cards to the passed player
/// \param player ID of actual player
/// \param start First card to play
/// \param end Last card to play
/// \returns bool False, to stop the time
//-----------------------------------------------------------------------------
bool Rovhult::unmarkAndMoveToLoser (unsigned int player, unsigned int start, unsigned int end) {
   TRACE8 ("Rovhult::unmarkAndMoveToLoser (3x unsigned int) - Player " << player
	   << "; Cards: " << start << '/' << end);
   for (unsigned int i (start); i <= end; ++i) {
      Card::Widget& card (players[player].reserve[i].removeTopCard ());
      card.unmark ();
      players[player].hand.setTopCard (card);
   }
   movePlayedCardsToLoser (player);
   return false;
}

//-----------------------------------------------------------------------------
/// Unmarks any played card and executes the move
/// \param player ID of actual player
/// \param count Number of cards to unmark
/// \returns bool False, to stop the time
//-----------------------------------------------------------------------------
void Rovhult::unmarkAndExecuteMove (unsigned int player, unsigned int count) {
   TRACE8 ("Rovhult::unmarkAndExecuteMove (2x unsigned int) - Player " << player << "; Cards: " << count);
   Check1 (count <= played.size ());
   while (count)
      played[played.size () - count--]->unmark ();
   executeMove (player);
}

//-----------------------------------------------------------------------------
/// Executes the move -> Check consequences for next in round and calculate
/// next player
/// \param player ID of player who played the last card
//-----------------------------------------------------------------------------
void Rovhult::executeMove (unsigned int player) {
   TRACE3 ("Rovhult::executeMove (unsigned int) - Player " << player);
   Check3 (player < NUM_PLAYERS);

   // If staple contains cards and no 10 was played (except if hand is empty):
   // Fill up cards til player has 3 (or one, in case of a ten)
   Card::Widget::NUMBERS nr (played.getTopCard ().number ());
   TRACE7 ("Rovhult::executeMove (unsigned int) - Card " << nr);
   if ((nr != cardNuke) || (!players[player].hand.size ()))
      fillUpPile (players[player].hand, (nr != cardNuke) ? 3 : 1);

   Glib::ustring stat;

   // If last 4 cards have the same number or a ten was played: Don't increase
   // player (except of course, if actual player doesn't have any cards left)
   bool removePlayed ((nr == cardNuke) || played4Equal ());
   if (removePlayed) {
      played.clear ();
      stat = _("Pile cleared; ");
   }

   bool unfinished (static_cast<int> (player) != nextAvailablePlayer ((player - 1) & 0x3));
   if (!removePlayed || unfinished) {
      if (!player && unfinished && noMoreHumans ())
	 cEndgame = 1;

      player = nextAvailablePlayer (player);
      Check3 (actPlayers.size () > player);
      Check3 (actPlayers[player]);
      if (nextAvailablePlayer (player) == -1) {
         status.pop ();
         stat = _("%1 lost");
         stat.replace (stat.find ("%1"), 2, actPlayers[player]->getName ());
         status.push (stat);
         setGameStatus (STOPPED);
	 return;
      }

      if (nr == cardSkip) {
         stat = _("Skipping %1; ");
         stat.replace (stat.find ("%1"), 2, actPlayers[player]->getName ());
         player = nextAvailablePlayer (player);
      }
   }

   displayTurn (player, stat);
   setNextPlayer (player);
   makeNextMoves ();
}

//-----------------------------------------------------------------------------
/// Callback after selection top card on played pile -> Moves all its card to
/// the passed player
/// \param player ID of player picking up the cards
//-----------------------------------------------------------------------------
void Rovhult::takeCards () {
   TRACE2 ("Rovhult::takeCards ()");

   if (getConnectionMgr ().getMode () != YGP::ConnectionMgr::NONE) {
      std::ostringstream msg;
      msg << "Play=" << played.getTopCard ().id () << ";Target=4";
      if (getConnectionMgr ().getMode () == YGP::ConnectionMgr::CLIENT)
         ignoreNextMsg = true;
      broadcastMessage (msg.str ());
   }

   movePlayedCardsToLoser (0);
}

//-----------------------------------------------------------------------------
/// Fills up the passed pile til it contains the specified number of cards
/// \param pile Pile to fill up
/// \param minCards Minimal number of cards pile should hold
//-----------------------------------------------------------------------------
void Rovhult::fillUpPile (Card::IPile& pile, unsigned int minCards) {
   TRACE3 ("Rovhult::fillUpPile (Card::IPile&, unsinged int) - "
           << pile.size () << " -> " << minCards);

   while ((pile.size () < minCards) && staple.size ())
      pile.insertSorted (staple.removeTopCard ());
}

//-----------------------------------------------------------------------------
/// Returns the number of equal cards from the played pile
/// \returns unsigned int Number of equal cards
//-----------------------------------------------------------------------------
unsigned int Rovhult::numberOfEqualTopCards () const {
   TRACE8 ("Rovhult::numberOfEqualTopCards () const");
   unsigned int nrCards (played.size ());

   if (nrCards)
      --nrCards;
   else
      return 0;

   unsigned int i (1);
   Card::Widget& card (played.getTopCard ());
   while (i <= nrCards) {
      TRACE9 ("Rovhult::numberOfEqualTopCards () const - Checking "
              << *played[nrCards - i] << " with " << card);

      if (played[nrCards - i]->number () != card.number ()) {
         TRACE8 ("Rovhult::numberOfEqualTopCards () const - found " << i);
         break;
      }
      ++i;
   }
   return i;
}

//-----------------------------------------------------------------------------
/// Clears the played staple if the last 4 cards are equal
/// \returns bool True, if 4 equal cards found
//-----------------------------------------------------------------------------
bool Rovhult::played4Equal () {
   TRACE8 ("Rovhult::played4Equal ()");

   int cards (numberOfEqualTopCards ()); Check3 (cards <= 4);
   if (cards < 4)
      return false;

   TRACE7 ("Rovhult::played4Equal () - found 4");
   return true;
}

//-----------------------------------------------------------------------------
/// Method to move the cards of the actual round to the winner
/// \param nrLoser Nr. of player getting all played cards
//-----------------------------------------------------------------------------
void Rovhult::movePlayedCardsToLoser (unsigned int nrLoser) {
   TRACE8 ("Rovhult::movePlayedCardsToLoser () - Player " << nrLoser << " gets "
           << played.size () << " cards");
   Check3 (nrLoser < NUM_PLAYERS);
   Check3 (played.size ());
   Check3 (actPlayers[nrLoser]);

   animateCards (players[nrLoser].hand, played, 0, played.size () - 1)
      .sigAnimation.connect (bind (mem_fun (*this, &Rovhult::cardsTaken), nrLoser));
   Glib::ustring stat (_("%1 can't continue -> Taking the whole pile. "));
   stat.replace (stat.find ("%1"), 2, actPlayers[nrLoser]->getName ());
   displayTurn (nrLoser = nextAvailablePlayer (nrLoser), stat);
   setNextPlayer (nrLoser);
}

//-----------------------------------------------------------------------------
/// Callback after cards of played pile have been animated toh the hand of
/// a player
/// \param player Player who took the cards
//-----------------------------------------------------------------------------
void Rovhult::cardsTaken (unsigned int player) {
   players[player].hand.sortByNumber ();
   makeNextMoves ();
}


//-----------------------------------------------------------------------------
/// Checks which player has still cards left
/// \param actPlayer ID of actual player
/// \returns int ID of player or -1 (if none can continue)
//-----------------------------------------------------------------------------
int Rovhult::nextAvailablePlayer (unsigned int actPlayer) const {
   // We assume (without checking), that the acutal player still has cards
   for (unsigned int i (1); i < NUM_PLAYERS; ++i) {
      actPlayer = (actPlayer + 1) & 0x3;
      if ((players[actPlayer].hand.size ())
          || players[actPlayer].reserve[0].size ()
          || players[actPlayer].reserve[1].size ()
          || players[actPlayer].reserve[2].size ()) {
         TRACE8 ("Rovhult::nextAvailablePlayer (unsigned int) const - Player: " << actPlayer);
         return actPlayer;
      }
   }
   return -1;
}

//-----------------------------------------------------------------------------
/// Remove cards from everything which can hold them and unregister any
/// signals (DND)
//-----------------------------------------------------------------------------
void Rovhult::clean () {
   TRACE8 ("Rovhult::clean () - Status: " << gameStatus ());

   if (gameStatus () == EXCHANGE)
      unregisterDND ();

   staple.clear ();                                             // Clear staple
   for (unsigned int i (0); i < NUM_PLAYERS; ++i) {   // Clear cards of players
      for (unsigned int j (0); j < 3; ++j)
         players[i].reserve[j].clear ();

      players[i].hand.clear ();
   }
   played.clear ();
   players[0].hand.setStyle (Card::IPile::NORMAL);
   Game::clean ();
}

//-----------------------------------------------------------------------------
/// Prepares the card for drag'n'drop (starting from the table, ending on the
/// hand or ending on the table, starting from the hand)
/// \param card Card to prepare for drag'n'drop
/// \param pile Number of pile on reserve holding card
//-----------------------------------------------------------------------------
void Rovhult::registerTableDND (Card::Widget& card, unsigned int pile) {
   TRACE8 ("Rovhult::registerTableDND (Card::Widget&, unsigned int) - "
           << card << " for pile " << pile);
   Check1 (pile < 3);
   Check3 (gameStatus () == EXCHANGE);

   // Card accepts drops from hand (offering the hand-position as payload)
   // and drags from table (offering its own pile-position as payload)
   Glib::RefPtr<Gtk::DropTarget> dst (Gtk::DropTarget::create (G_TYPE_UINT, Gdk::DragAction::MOVE));
   dst->signal_drop ().connect
      ([this, pile] (const Glib::ValueBase& value, double, double) -> bool {
          Glib::Value<unsigned int> v; v.init (value.gobj ());
          return cardDroppedOnTable (v.get (), pile);
       }, false);
   card.add_controller (dst);
   aTableDND[&card] = dst;

   Glib::RefPtr<Gtk::DragSource> src (Gtk::DragSource::create ());
   src->set_actions (Gdk::DragAction::MOVE);
   src->signal_prepare ().connect
      ([pile] (double, double) -> Glib::RefPtr<Gdk::ContentProvider> {
          Glib::Value<unsigned int> v; v.init (Glib::Value<unsigned int>::value_type ());
          v.set (pile);
          return Gdk::ContentProvider::create (v);
       }, false);
   src->signal_drag_begin ().connect
      ([&card, src] (const Glib::RefPtr<Gdk::Drag>&) {
          src->set_icon (Gdk::Texture::create_for_pixbuf (card.getImage ()), 0, 0);
       }, false);
   card.add_controller (src);
   aTableData[&card] = src;
}

//-----------------------------------------------------------------------------
/// Prepares the card for drag'n'drop (starting from the hand ending on table
/// or ending on hand, starting from table)
/// \param card Card to prepare for drag'n'drop
/// \param pile Number of pile on reserve holding card
//-----------------------------------------------------------------------------
void Rovhult::registerHandDND (Card::Widget& card,  unsigned int iCard) {
   TRACE8 ("Rovhult::registerHandDND (Card::Widget&, unsigned int) - " << card
           << "; pos " << iCard);
   Check3 (gameStatus () == EXCHANGE);

   // Card accepts drops from table (offering the table-pile as payload) and
   // drags from hand (offering its own hand-position as payload)
   Glib::RefPtr<Gtk::DropTarget> dst (Gtk::DropTarget::create (G_TYPE_UINT, Gdk::DragAction::MOVE));
   dst->signal_drop ().connect
      ([this, iCard] (const Glib::ValueBase& value, double, double) -> bool {
          Glib::Value<unsigned int> v; v.init (value.gobj ());
          return cardDroppedOnHand (v.get (), iCard);
       }, false);
   card.add_controller (dst);
   aHandDND[&card] = dst;

   Glib::RefPtr<Gtk::DragSource> src (Gtk::DragSource::create ());
   src->set_actions (Gdk::DragAction::MOVE);
   src->signal_prepare ().connect
      ([iCard] (double, double) -> Glib::RefPtr<Gdk::ContentProvider> {
          Glib::Value<unsigned int> v; v.init (Glib::Value<unsigned int>::value_type ());
          v.set (iCard);
          return Gdk::ContentProvider::create (v);
       }, false);
   src->signal_drag_begin ().connect
      ([&card, src] (const Glib::RefPtr<Gdk::Drag>&) {
          src->set_icon (Gdk::Texture::create_for_pixbuf (card.getImage ()), 0, 0);
       }, false);
   card.add_controller (src);
   aHandData[&card] = src;

   TRACE9 ("Rovhult::registerHandDND (Card::Widget&, unsigned int) - Activate: "
           << activeCards.size () << '/' << activeCards.capacity ());
   Check3 (activeCards.size () >= iCard);
   sigc::connection conn (card.signal_clicked ().connect
                          (bind (mem_fun (*this, &Rovhult::finishedExchange), iCard)));
   if (activeCards.size () > iCard)
      activeCards[iCard] = conn;
   else
      activeCards.push_back (conn);
   TRACE9 ("Rovhult::registerHandDND (Card::Widget&, unsigned int) - End");
}

//-----------------------------------------------------------------------------
/// Stops the drag'n'drop abilities of the passed card
/// \param card Card to unregister of dnd
//-----------------------------------------------------------------------------
void Rovhult::unregisterDND (Card::Widget& card) const {
   std::map <const Card::Widget*, Glib::RefPtr<Gtk::DropTarget> >::const_iterator iDst;
   if ((iDst = aHandDND.find (&card)) != aHandDND.end ())
      card.remove_controller (iDst->second);
   else if ((iDst = aTableDND.find (&card)) != aTableDND.end ())
      card.remove_controller (iDst->second);

   std::map <const Card::Widget*, Glib::RefPtr<Gtk::DragSource> >::const_iterator iSrc;
   if ((iSrc = aHandData.find (&card)) != aHandData.end ())
      card.remove_controller (iSrc->second);
   else if ((iSrc = aTableData.find (&card)) != aTableData.end ())
      card.remove_controller (iSrc->second);
}

//-----------------------------------------------------------------------------
/// Stops the drag'n'drop abilities of the cards of player 0
/// \param card Card to unregister of dnd
//-----------------------------------------------------------------------------
void Rovhult::unregisterDND () {
   TRACE8 ("Rovhult::unregisterDND () - Status: " << gameStatus ());
   Check3 (gameStatus () == EXCHANGE);
   Check3 (aHandDND.size () == players[0].hand.size ());
   Check3 (aTableDND.size () == players[0].hand.size ());

   for (unsigned int i (0); i < players[0].hand.size (); ++i) {
      Card::Widget& card (*players[0].hand[i]);
      unregisterDND (card);
      Check3 (players[0].reserve[i].size () == 2);
      activeCards[i].disconnect ();
      disconnectCardInHand (card);
   }

   for (unsigned int i (0);
        i < (sizeof (players[0].reserve) / sizeof (players[0].reserve[0]));
        ++i) {
      unregisterDND (players[0].reserve[i].getTopCard ());
      disconnectCardOnTable (players[0].reserve[i].getTopCard ());
   }

   Check3 (aHandDND.empty ()); Check3 (aHandData.empty ());
   Check3 (aTableDND.empty ()); Check3 (aTableData.empty ());
   activeCards.clear ();
}

//-----------------------------------------------------------------------------
/// Disconnects the card (in the hand) from every connection hold
//-----------------------------------------------------------------------------
void Rovhult::disconnectCardInHand (const Card::Widget& card) {
   TRACE3 ("Rovhult::disconnectCardInHand (const Card::Widget&) - " << card);
   Check1 (aHandDND.find (&card) != aHandDND.end ());
   Check1 (aHandData.find (&card) != aHandData.end ());

   aHandDND.erase (&card);
   aHandData.erase (&card);
}

//-----------------------------------------------------------------------------
/// Disconnects the card (on the table) from every connection hold
//-----------------------------------------------------------------------------
void Rovhult::disconnectCardOnTable (const Card::Widget& card) {
   TRACE3 ("Rovhult::disconnectCardOnTable (const Card::Widget&) - " << card);
   Check1 (aTableDND.find (&card) != aTableDND.end ());
   Check1 (aTableData.find (&card) != aTableData.end ());

   aTableDND.erase (&card);
   aTableData.erase (&card);
}

//-----------------------------------------------------------------------------
/// Deals the cards
//-----------------------------------------------------------------------------
void Rovhult::dealCards () {
   TRACE9 ("Rovhult::dealCards ()");
   Check3 (staple.size () > 36);

   // Show cards on table: For all players put 6 cards on table (only the
   // (upper visible) and 3 (visible ones) in hand
   for (unsigned int i (0); i < NUM_PLAYERS; ++i)
      for (unsigned int j (0); j < 3; ++j) {
         for (unsigned int k (0); k < 2; ++k) {           // Set cards on table
            Card::Widget& card (staple.removeTopCard ());
            players[(i - posServer) & 0x3].reserve[j].setTopCard (card, k);
         } // end-for two cards pro pile (in reserve)

         // Put card into hand
         players[(i - posServer) & 0x3].hand.insertSorted (staple.removeTopCard ());
      }

   // Enable drag-and-drop for cards in the hand (of human player)
   for (unsigned int i (0); i < players[i].hand.size (); ++i) {
      registerHandDND (*players[0].hand[i], i);
      registerTableDND (players[0].reserve[i].getTopCard (), i);
   }

   Check3 (staple.size ());

   status.pop ();
   status.push (_("Exchange the cards in your hand with the one on the "
                  "table (with drag and drop) - click on one card to start playing"));
}

//-----------------------------------------------------------------------------
/// Callback after dropping a (hand-)card onto (cards on) table
/// \param handCard Offset of the dragged card in the hand
/// \param pile Number of pile the card was dropped onto
/// \returns bool True, drop accepted
//-----------------------------------------------------------------------------
bool Rovhult::cardDroppedOnTable (unsigned int handCard, unsigned int pile) {
   Check3 (pile < 3);

   TRACE1 ("Rovhult::cardDroppedOnTable (...) - Data = "
           << handCard << " <-> " << pile);

   Glib::signal_idle ().connect
      (bind (mem_fun (*this, &Rovhult::doSwapCards), pile, handCard));
   return true;
}

//-----------------------------------------------------------------------------
/// Swaps a card in the hand with one (top-card) on the table
/// \param pile Offset of pile whose top-card should be swapped
/// \param card Offset of card in the hand which should be swapped
/// \returns bool Always false
//-----------------------------------------------------------------------------
bool Rovhult::doSwapCards (unsigned int pile, unsigned int card) {
   Card::Widget& cardTable (players[0].reserve[pile].removeTopCard ());
   Card::Widget& cardHand (players[0].hand.remove (card));

   TRACE1 ("Rovhult::doSwapCards (unsigned int, unsigned int) - Exchanging cards "
           << cardHand.id () << "<->" << cardTable.id ());

   activeCards[card].disconnect ();
   unregisterDND (cardHand);
   unregisterDND (cardTable);
   disconnectCardInHand (cardHand);
   disconnectCardOnTable (cardTable);

   // Swap cards
   players[0].reserve[pile].setTopCard (cardHand);
   players[0].hand.insert (cardTable, card);

   // Adapt dnd-settigns
   registerHandDND (cardTable, card);
   registerTableDND (cardHand, pile);
   return false;
}

//-----------------------------------------------------------------------------
/// Callback after dropping a (table-)card onto the hand
/// \param tablePile Offset of the pile the dragged card came from
/// \param card Offset the card was dropped onto in the hand
/// \returns bool True, drop accepted
//-----------------------------------------------------------------------------
bool Rovhult::cardDroppedOnHand (unsigned int tablePile, unsigned int card) {
   Check3 (card < players[0].hand.size ());

   TRACE1 ("Rovhult::cardDroppedOnHand (...) - Data = "
           << tablePile << " <-> " << card);

   Glib::signal_idle ().connect
       (bind (mem_fun (*this, &Rovhult::doSwapCards), tablePile, card));
   return true;
}

//-----------------------------------------------------------------------------
/// Shows visually the card the non-human is about to play from a pile
/// \param player Player in question
/// \param pile Pile in question
/// \param invalid Flag, if the card is invalid (can't actually be played)
//-----------------------------------------------------------------------------
void Rovhult::showCardOfPile (unsigned int player, unsigned int pile, bool invalid) const {
   TRACE9 ("Rovhult::showCardOfPile (2x unsigned int) - " << player << "->" << pile);
   const Card::VPile& actPile (players[player].reserve[pile]);
   Check3 (actPile.size ());

   Card::Widget& card (actPile.getTopCard ());
   ((actPile.size () > 1) || invalid) ? card.mark () : card.showFace ();
}

//-----------------------------------------------------------------------------
/// Shows the cards the user is about to play
/// \param player Player in turn
/// \param start First card to play
/// \param end Last card to play
//-----------------------------------------------------------------------------
void Rovhult::showCards2Play (unsigned int player, unsigned int start, unsigned int end) {
   TRACE2 ("Rovhult::showCards2Play (3x unsigned int) - Player " << player << ": " << start << '/' << end);
   Check1 (player < NUM_PLAYERS);
   Check1 (start <= end);

   if (players[player].hand.size ()) {
      Check1 (end < players[player].hand.size ());
      flipCards2Play (players[player].hand, start, end);
      animateCards (played, players[player].hand, start, end)
	 .sigAnimation.connect (bind (mem_fun (*this, &Rovhult::executeMove), player));
   }
   else {
      Check1 (end < (sizeof (players[player].reserve) / sizeof (players[player].reserve[0])));
      Card::IPile& pile (players[player].reserve[start]);
      unsigned int target (end + 1);

      Card::PileWindows* animPiles (NULL);
      if (cardValid (pile.getTopCard ().number (), true)) {
	 animPiles = &animateCards2 (played, pile, pile.size () - 1, pile.size () - 1);
	 animPiles->sigAnimation.connect (bind (mem_fun (*this, &Rovhult::unmarkAndExecuteMove), player, end - start + 1));
      }
      else {
         target += 4;
	 Glib::signal_timeout ().connect (bind (mem_fun (*this, &Rovhult::unmarkAndMoveToLoser),
						player, start, end), Card::ComputerPlayer::TIMEOUT);
      }
      showCardOfPile (player, start, target > 3);

      while (++start <= end) {
	 showCardOfPile (player, start, target > 3);
	 if (animPiles) {
	    Check2 (players[player].reserve[start].size () == 2);
	    animPiles->addWindow (players[player].reserve[start], 1, 1);
	 }
      }

      // Inform the others about the move
      if (getConnectionMgr ().getMode () == YGP::ConnectionMgr::SERVER) {
         // Send played card to all clients (if any)
         std::ostringstream msg;
         msg << "Play=" << pile.getTopCard ().id () << ";Target=" << target;
         broadcastMessage (msg.str ());
      }
   }
}

//-----------------------------------------------------------------------------
/// Finds an executes the turn of a (computer controled) player
//-----------------------------------------------------------------------------
void Rovhult::makeMove (unsigned int player) {
   TRACE2 ("Rovhult::makeMove (unsigned int) - Player " << player);

   unsigned int pos1Play, pos2Play;
   if (cEndgame)
      ++cEndgame;
   if ((cEndgame > 30) && !(cEndgame & 0x7) && players[player].hand.size ())
      pos1Play = pos2Play = selectRandomCard (player);
   else
      findCard2Play (player, pos1Play, pos2Play);
   TRACE8 ("Rovhult::makeMove (unsigned int) - Player " << player << "; Card: " << pos2Play);

   if (pos2Play != -1U) {
      Check3 (pos1Play <= pos2Play);
      showCards2Play (player, pos1Play, pos2Play);
   }
   else {
      if (getConnectionMgr ().getMode () == YGP::ConnectionMgr::SERVER) {
	 std::ostringstream msg;
	 msg << "Play=" << played[played.size () - 1]->id () << ";Target=4";
	 broadcastMessage (msg.str ());
      }
      movePlayedCardsToLoser (player);
   }
}

//-----------------------------------------------------------------------------
/// Checks if there are only special cards up to the passed position
/// \param pile Pile to inspect
/// \param start Lower position of cards to inspect
/// \param end Upper position of cards to inspect
/// \returns bool True, if there are only special cards
//-----------------------------------------------------------------------------
bool Rovhult::existOnlySpecialCards (const Card::IPile& pile, unsigned int start,
                                     unsigned int end) const {
   Check3 (start <= end);
   Check3 (end < pile.size ());

   do {
      if (!isSpecialCard (pile[start]->number ()))
         return false;
   } while (++start <= end);

   TRACE8 ("Rovhult::existOnlySpecialCards (const Card::IPile&, unsigned int) - Yes");
   return true;
}

//-----------------------------------------------------------------------------
/// Finds the next card to play (for a computer controlled player)
/// \param player Player to inspect
/// \param start Position of (first) card to play
/// \param end Position of (last) card to play
//-----------------------------------------------------------------------------
void Rovhult::findCard2Play (unsigned int player, unsigned int& start,
                             unsigned int& end) const {
   TRACE2 ("Rovhult::findCard2Play (unsigned int) - Player " << player);
   Check3 (player < NUM_PLAYERS);

   // Search for minimal card to play; this is either a card equal or
   // bigger or - if no previous card is played or the last card
   // played was a cardReverse (7) - the smallest available
   Card::Widget::NUMBERS cardMin (Card::Widget::THREE);
   if (played.size ()
       && (played.getTopCard ().number () != cardReverse)
       && (played.getTopCard ().number () != Card::Widget::TWO))
      cardMin = played.getTopCard ().number ();
   TRACE5 ("Rovhult::findCard2Play (unsigned int) - Card to beat " << cardMin);

   // Check if to play from hand or to play from reserve
   unsigned int nrCards (players[player].hand.size ());
   if (nrCards) {
      // Special handling if cards of next player are know: Try to give him
      // the whole pile
      Card::Widget::NUMBERS nextMin, nextMax;
      int hpPos (-1);
      start =
         (played.size ()
          && getPileLimits (nextAvailablePlayer (player), nextMin, nextMax)
          && (((nextMin > cardReverse)
               && ((hpPos = players[player].hand.find (cardReverse)) =! -1))
              || (((hpPos = (players[player].hand.findFirstEqualOrBigger
                             (Card::Widget::NUMBERS (nextMax + 1)))) != -1)
                  && ((hpPos = skip (cardReverse, players[player].hand, hpPos)) != -1)
                  && ((hpPos = skip (cardNuke, players[player].hand, hpPos)) != -1))
              && cardValid (players[player].hand[hpPos]->number (), true)))
         ? hpPos : players[player].hand.findFirstEqualOrBigger (cardMin);
      TRACE6 ("Rovhult::findCard2Play (unsigned int) - First matching card"
              " at pos " << start);

      // Check if no matching normal card is found or found card is
      // bigger than the played cardReverse (7). If so, use special
      // card instead.
      if ((start == -1U)
          || (played.size ()
              && ((played.getTopCard ().number () == cardReverse)
                  && players[player].hand[start]->number () > cardReverse))) {
         TRACE7 ("Rovhult::findCard2Play (unsigned int) - Ordinary cards don't"
                 " match -> Searching for special card");

         if (players[player].hand[0]->number () == Card::Widget::TWO)
            start = 0;
         else {
            start = players[player].hand.findFirstEqualOrBigger (cardNuke);
            if ((start == -1U)
		|| (players[player].hand[start]->number () != cardNuke)) {
	       end = start = -1U;
	       TRACE7 ("Rovhult::findCard2Play (unsigned int) - Can't continue!");
	       return;
	    }
	    TRACE7 ("Rovhult::findCard2Play (unsigned int) - Using special card "
		    << *players[player].hand[start] << " at pos " << start);
         }
      }
      else {
         Check3 ((cardMin == cardReverse)
                 ? (players[player].hand[start]->number () <= cardReverse)
                 : (players[player].hand[start]->number () >= cardMin));

         // If player would continue with a card coming directly
         // before cardReverse (6), but has also a cardReverse (7),
         // play that card instead
         if (players[player].hand[start]->number () == (cardReverse - 1)) {
            if ((end = players[player].hand.find (cardReverse, start)) != -1U) {
               TRACE8 ("Rovhult::findCard2Play (unsigned int) - Exchanging "
                       << *players[player].hand[start] << " with "
                       << *players[player].hand[end]);
               start = end;
            }
         }
         else
            // The search of Card::Widget does (and shall) not know
            // about the special meaning of tens, so skip them by
            // yourself, but use a TWO (if available) in case a TEN
            // is/are the last card(s)
            if ((end = skip (cardNuke, players[player].hand, start)) == -1U) {
               if (players[player].hand[0]->number () == Card::Widget::TWO)
                  start = 0;
            }
            else
               start = end;

         TRACE5 ("Rovhult::findCard2Play (unsigned int) - Playing "
                 << *players[player].hand[start] << " at pos " << start);
      }

      // Now find the last of equal cards; get rid of all of them if:
      // - they would complete 4
      // - there are are only special cards left
      // - it's not a special card which is
      //     * not the highest card
      //     * it's the first card
      //     * it's a not that high card (up to 9)
      unsigned int last (players[player].hand.findLastEqual (start));
      TRACE8 ("Rovhult::findCard2Play (unsigned int) - Last equal at pos "
              << last << " (of " << players[player].hand.size ()
              << " cards)");
      Check3 (last < players[player].hand.size ());
      end = (((numberOfEqualTopCards () + last - start) == 3)
             || ((start
                  ? existOnlySpecialCards (players[player].hand, 0, start - 1) : 1)
                 && ((last < (players[player].hand.size () - 1))
                     ? existOnlySpecialCards (players[player].hand, last + 1,
                                              players[player].hand.size () - 1)
                     : 1))
             || ((!isSpecialCard (players[player].hand[start]->number ()))
                 && ((last != (players[player].hand.size () - 1))
                     || !start
                     || (players[player].hand[start]->number () < cardNuke))))
         ? last : start;

      return;
   }
   else {
      TRACE5 ("Rovhult::findCard2Play (unsigned int) - Analysing reserve");

      // Bitfield for lower cards: Bit 0: Cards visible; Bit 1: Normal cards
      int bfLowerCardsInfo (0);

      // Play first visible cards
      for (start = 0; start < 3; ++start) {
         const Card::IPile* pPile (&players[player].reserve[start]);
         if (pPile->size ()) {
            Card::Widget& card (pPile->getTopCard ());
            if (card.showsFace ()) {
               bfLowerCardsInfo |= 0x1;

               // If card can be played: Search for last equal card
               if (cardValid (card.number (), true)) {
                  end = start;

                  // Don't play all cards, if it is a special card and "normal"
                  // cards remain
                  if (!(isSpecialCard (card.number ())
                        && (bfLowerCardsInfo & 0x2)))
                     while ((end < 2)
                            && (pPile = &players[player].reserve[end + 1])
                            && pPile->size ()
                            && pPile->topCardShowsFace ()
                            && (pPile->getTopCard ().number ()
                                == card.number ()))
                        ++end;

                  if (!isSpecialCard (card.number ()))
                     bfLowerCardsInfo |= 0x2;

                  TRACE7 ("Rovhult::findCard2Play (unsigned int) - Playing visible card "
			  << card << " at pos " << end);
                  return;
               }
            }
         }
      }

      if (bfLowerCardsInfo)
	 start = end = -1U;                             // No valid card found
      else {
	 // No card visible: Play the first
         start = 0;
         while (!players[player].reserve[start].size ()) {
            ++start;
            Check3 (start < 3);
         }
         end = start;
         TRACE7 ("Rovhult::findCard2Play (unsigned int) - Playing invisible card "
                 << players[player].reserve[end].getTopCard () << " at pos " << end);
      }
   }
}

//-----------------------------------------------------------------------------
/// Retrieves the minimal and maximal card of the player
/// \param player Player whose card to analyse
/// \param min Returns the minimal card
/// \param max Returns the maximal card
/// \returns bool true, if cardinfo is available
//-----------------------------------------------------------------------------
bool Rovhult::getPileLimits (unsigned int player, Card::Widget::NUMBERS& min,
                             Card::Widget::NUMBERS& max) const {
   if (players[player].hand.size ())
      return false;

   TRACE3 ("Rovhult::getPileInfo (unsigned int, unsigned int&, unsigned int&)"
           " - Analysing cards of player " << player);

   bool cardFound (false);
   for (unsigned int i (0); i < 3; ++i) {
      const Card::IPile* pPile (&players[player].reserve[i]);
      if (pPile->size ()) {
         Card::Widget& card (pPile->getTopCard ());
         if (card.showsFace ()) {
            if (isSpecialCard (card.number ()))
                break;

            if (cardFound)
               max = card.number ();
            else {
               min = max = card.number ();
               cardFound = true;
            }
         }
      }
   }

   TRACE5 ("Rovhult::getPileInfo (unsigned int, unsigned int&, unsigned int&)"
           " - Limits found: " << int (cardFound ? min : -1) << '/'
           << int (cardFound ? max : -1));
   return cardFound;
}


//-----------------------------------------------------------------------------
/// Shows or hides the cards of the computer player
/// \param open Flag if cards should be shown or hidden
//-----------------------------------------------------------------------------
void Rovhult::playOpen (bool open) {
   Card::IPile::ShowOpt show (open ? Card::IPile::SHOWFACE : Card::IPile::SHOWBACK);

   for (unsigned int i (1); i < NUM_PLAYERS; ++i) {
      players[i].hand.setShowOption (show);
      players[i].hand.setStyle ((show == Card::IPile::SHOWFACE)
                                ? Card::IPile::COMPRESSED
                                : Card::IPile::QUITE_COMPRESSED);
   }
}

//-----------------------------------------------------------------------------
/// End the current game as soon as possible
//-----------------------------------------------------------------------------
void Rovhult::end (bool restart) {
   if (gameStatus () == EXCHANGE)
      unregisterDND ();

   Game::end (restart);
}


//-----------------------------------------------------------------------------
/// Changes the names of the playing people
/// \param newPlayer Array holding the new player
//-----------------------------------------------------------------------------
void Rovhult::changeNames (const std::vector<Card::Player*>& newPlayer) {
   Game::changeNames (newPlayer);

   for (unsigned int i (0); i < NUM_PLAYERS; ++i)
      players[i].name.set_text (actPlayers[i]->getName ());
}

//----------------------------------------------------------------------------
/// Converts a pile-number to the actual pile
/// \param player Actual player
/// \param pile ID of the pile to return
///    - 0: Play from hand
///    - 1 - 3: Play from pile 0 - 2
///    - 4: Pick up played pile
///    - 5 - 7: Tried to play from pile 0 - 2, but failed
/// \returns Card::IPile* Pile corresponding to the passed number or NULL
//----------------------------------------------------------------------------
Card::IPile* Rovhult::getPileOfPlayer (unsigned int player, unsigned int pile) {
   if ((player >= NUM_PLAYERS) || (pile > 7))
      return NULL;

   TRACE8 ("Rovhult::getPileOfPlayer (unsigned int, unsigned int) - Player "
           << player << "; Pile " << pile);

   if (pile > 4) {
      Card::IPile& playerPile (players[player].reserve[pile - 5]);
      Check3 (playerPile.size ());
      if (cardValid (playerPile.getTopCard ().number (), true))
         return &playerPile;
      else {
         played.Card::IPile::append (playerPile.removeTopCard ());
         return &played;
      }
   }

   return &(pile
            ? ((pile == 4)
               ? static_cast<Card::IPile&> (played)
               : static_cast<Card::IPile&> (players[player].reserve[pile - 1]))
            : static_cast<Card::IPile&> (players[player].hand));
}

//----------------------------------------------------------------------------
/// Handles the messages the server might send for the Rovhult cardgame
/// \param player ID of player sending the message
/// \param message Message received from the server
/// \returns bool True, if message has been completey processed
//----------------------------------------------------------------------------
bool Rovhult::handleMessage (unsigned int player, const std::string& message) {
#if 0
   if (gameStatus () >= EXCHANGE) {
      TRACE1 ("Rovhult::handleMessage (unsigned int player, const std::string&) - "
              << message << " (" << player << ')');

      YGP::Tokenize command (message);
      std::string cmd (command.getNextNode ('='));

      if (cmd == "Exchange") {
         std::string cards (command.getNextNode (';'));
         cmd = command.getNextNode ('=');
         unsigned long lPlayer (player);
         if ((cmd == "Player")
             && !stringToNumber (lPlayer, command.getNextNode (';').c_str ())
             && (lPlayer < NUM_PLAYERS)) {
            register unsigned int save (lPlayer);
            lPlayer = (lPlayer - posServer) & 0x3;

            // Don't exchange already exchanged cards
            if (save != posServer) {
               // Remove the cards in the hand and the top of the table piles
	       std::vector<Card::Widget*> pile;
               for (unsigned int i (0); i < 3; ++i) {
                  pile.push_back (&players[lPlayer].hand.removeTopCard ());
                  pile.push_back (&players[lPlayer].reserve[i].removeTopCard ());
               }

               command = cards;
               unsigned long card (0);
               unsigned int target (0);

               // Target piles
               Card::IPile* piles[] =
                   { &players[lPlayer].hand, &players[lPlayer].hand,
                     &players[lPlayer].hand, &players[lPlayer].reserve[0],
                     &players[lPlayer].reserve[1], &players[lPlayer].reserve[2] };

               while (command.getNextNode (' ').size ()) {
                  if (stringToNumber (card, command.getActNode ().c_str ()))
                     break;

                  TRACE8 ("Rovhult::handleMessage (unsigned int, const std::string&) - "
                          << lPlayer << ": " << card);
		  for (std::vector<Card::Widget*>::iterator i (pile.begin ()); i != pile.end (); ++i)
		     if ((*i)->id () == card) {
			Card::Widget& movedCard (**i);
			if (target > 2)
			   movedCard.showFace ();
			piles[target++]->setTopCard (movedCard);
			pile.erase (i);
		     }
               }
               if (pile.empty ()) {
                  aExchanged |= (1 << lPlayer);

                  TRACE2 ("Rovhult::handleMessage (unsigned int player, const std::string&) - "
                          "Exchanged: " << std::hex << aExchanged << std::dec);
                     YGP::ConnectionMgr& cmgr (getConnectionMgr ());
                     // Inform other clients
                     if (cmgr.getMode () == YGP::ConnectionMgr::SERVER)
                        broadcastMessage (message);

                     if (aExchanged == 0xf) {
                        status.pop ();
                        setGameStatus (PLAYING);
                        makeNextMoves ();
                     }
               }
            }
            else
               ignoreNextMsg = false;
            return true;
         }
      }
   }
#endif
   return Game::handleMessage (player, message);
}

//----------------------------------------------------------------------------
/// Executes the remote move locally
/// \param pile Pile to move to/from
/// \param target ID of target as send by the partner
/// \returns bool True, if the timer to execute the move should be set
//----------------------------------------------------------------------------
bool Rovhult::executeRemoteMove (Card::IPile& pile, unsigned int target) {
   if (target) {
      Check3 (gameStatus () == PLAYING);
      TRACE7 ("Rovhult::executeRemoteMove (Card::IPile&, unsigned int) - Target " << target);

      unsigned int player (currentPlayer ());
      unsigned int pos1Play, pos2Play;
      if (target > 3) {
         if (&pile == &played) {
            movePlayedCardsToLoser (player);
            return false;
         }
         pos2Play = target - 5;
      }
      else
         pos2Play = target - 1;

      Card::IPile* pile (&players[player].reserve[pos1Play = pos2Play]);
      Card::Widget::NUMBERS nr (pile->getTopCard ().number ());
      pile->getTopCard ().mark ();
      if (pile->size () > 1) {
         while (pos1Play) {
            pile = &players[player].reserve[pos1Play - 1];
            if ((pile->size () > 1)
                && (pile->getTopCard ().number () == nr)) {
               --pos1Play;
               pile->getTopCard ().mark ();
            }
            else
               break;
         }
      }

      if ((nextAvailablePlayer ((player - 1) & 0x3) != (int)player) && noMoreHumans ())
	 cEndgame = 1;
      return true;
   }
   else
      return Game::executeRemoteMove (pile, target);
}

//-----------------------------------------------------------------------------
/// Actions to take when the cards are resized
/// \pre The cardsize must be set in CardImages::WIDTH/HEIGHT
//-----------------------------------------------------------------------------
void Rovhult::resizeCards() {
   for (unsigned int i(0); i < NUM_PLAYERS; ++i) {
      for (int j(0); j < 3; ++j)
         players[i].reserve[j].set_size_request(Card::Images::WIDTH, Card::Images::HEIGHT + 7);
      players[i].hand.set_size_request(Card::Images::WIDTH * 3, Card::Images::HEIGHT);
   }

   played.set_size_request(Card::Images::WIDTH, Card::Images::HEIGHT);
   staple.set_size_request(Card::Images::WIDTH, Card::Images::HEIGHT + 50);
}

//-----------------------------------------------------------------------------
/// Checks if there are only computer-player with cards left
/// \returns bool True, if only computer-players are left
//-----------------------------------------------------------------------------
bool Rovhult::noMoreHumans() const {
   int first(nextAvailablePlayer(0));
   int i(first);
   do {
      if (typeid(*actPlayers[i]) != typeid(Card::ComputerPlayer))
	 return false;
      i = nextAvailablePlayer(i);
   } while (first < i);
   return true;
}

//-----------------------------------------------------------------------------
/// Selects a random card for the passed player
/// \param player Player to analyse
/// \returns unsigned int Card to play
//-----------------------------------------------------------------------------
unsigned int Rovhult::selectRandomCard(unsigned int player) {
   Check3(players[player].hand.size());
   unsigned int pos2Play(rand() % players[player].hand.size());
   return cardValid(players[player].hand[pos2Play]->number(), true) ? pos2Play : -1U;
}
