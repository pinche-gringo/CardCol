//$Id$

//PROJECT     : Cardgames
//SUBSYSTEM   : Rovhult
//REFERENCES  :
//TODO        :
//BUGS        :
//REVISION    : $Revision$
//AUTHOR      : Markus Schwab
//CREATED     : 28.3.2002
//COPYRIGHT   : Anticopyright (A) 2002, 2003

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

#include <glibmm/main.h>

#include <gtkmm/statusbar.h>
#include <gtkmm/accelgroup.h>
#include <gtkmm/messagedialog.h>

#include <YGP/Check.h>
#include <YGP/Trace.h>
#include <YGP/ConnMgr.h>
#include <YGP/Tokenize.h>

#include <Player.h>
#include <CardWidget.h>
#include <ComputerPlayer.h>

#include "SigCExt.h"
#include "Rovhult.h"


std::vector<Gtk::TargetEntry> Rovhult::dndTypeHand;
std::vector<Gtk::TargetEntry> Rovhult::dndTypeTable;


const unsigned int Rovhult::COLS_PLAYER[NUM_PLAYERS] = { 7, 13, 7, 1 };
const unsigned int Rovhult::ROWS_PLAYER[NUM_PLAYERS] = { 4, 7, 13, 7 };


//-----------------------------------------------------------------------------
/// Defaultconstructor; all widgets are created
/// \param parent: Parent widget (box) to display the game in
/// \param statusbar: Status bar widget to display information about the game
/// \param cardset: Cardset to use
/// \param names: Vector of players
/// \param posPlayer: Position of player for the server
/// \param mxSerialize: Mutex to serialize messages from the server
//-----------------------------------------------------------------------------
Rovhult::Rovhult (Gtk::Box& parent, Gtk::Statusbar& statusbar,
                  CardSet& cardset, const std::vector<Player*>& player,
                  unsigned int posPlayer, YGP::Mutex& mxSerialize)
   : Game (parent, statusbar, cardset, player, posPlayer, mxSerialize, 16, 20)
     , played (ICardPile::VERY_COMPRESSED, ICardPile::SHOWFACE)
     , staple (ICardPile::VERY_COMPRESSED)
     , aExchanged (0) {
    TRACE9 ("Rovhult::Rovhult (Gtk::Box& Gtk::Statusbar&, CardSet&,"
            " const std::vector<Glib::ustring>&)");

   staple.show ();
   attach (staple, 3, 4, 2, 7);

   Check3 (cards.size ());
   int width (cards.getCard (0).getImageWidth ());
   int height (cards.getCard (0).getImageHeight ());

   // Show and attach card-piles
   changeNames (player);
   for (unsigned int i (0); i < NUM_PLAYERS; ++i) {
      for (int j (0); j < 3; ++j) {
         players[i].reserve[j].setStyle (ICardPile::QUITE_COMPRESSED);
         players[i].reserve[j].show ();
         attach (players[i].reserve[j], COLS_PLAYER[i] + (j << 1),
                 COLS_PLAYER[i] + 1 + (j << 1), ROWS_PLAYER[i],
                 ROWS_PLAYER[i] + 2, Gtk::SHRINK, Gtk::SHRINK, 0);

         TRACE9 ("Rovhult::Rovhult () - Set at: "
                 << COLS_PLAYER[i]  + (j << 1) << '/' << ROWS_PLAYER[i]);

         players[i].reserve[j].set_size_request (width, height + 7);
      }

      players[i].name.show ();
      attach (players[i].name, COLS_PLAYER[i], COLS_PLAYER[i] + 5,
              ROWS_PLAYER[i] + (i ? 5 : 2),
              ROWS_PLAYER[i] + (i ? 6 : 3),
              Gtk::EXPAND, Gtk::EXPAND, 1);

      players[i].hand.set_size_request (width * 3, height);

      players[i].hand.setStyle (i ? ICardPile::QUITE_COMPRESSED : ICardPile::NORMAL);
      players[i].hand.setShowOption (i ? ICardPile::SHOWBACK : ICardPile::SHOWFACE);
      players[i].hand.show ();
      attach (players[i].hand, COLS_PLAYER[i],
              COLS_PLAYER[i] + 5,
              ROWS_PLAYER[i] + (i ? 3 : -3),
              ROWS_PLAYER[i] + (i ? 3 : -3) + 2,
              Gtk::FILL | Gtk::EXPAND, Gtk::FILL | Gtk::EXPAND, 0);
      TRACE9 ("Rovhult::Rovhult () - 2nd set at: "
              << COLS_PLAYER[i] + (i << 1) << '/'
              << ROWS_PLAYER[i] + (i ? 3 : -3));
   }

   played.show ();
   played.set_size_request (width, height);
   staple.set_size_request (width, height + 50);
   staple.setShowOption (ICardPile::SHOWBACK);

   attach (played, 7, 11, 5, 14, Gtk::SHRINK, Gtk::SHRINK, 1);

   if (dndTypeHand.empty ()) {
      Check3 (dndTypeTable.empty ());
      dndTypeHand.push_back
         (Gtk::TargetEntry ("icon/card/hand", GTK_TARGET_SAME_APP, HAND));
      dndTypeTable.push_back
         (Gtk::TargetEntry ("icon/card/table", GTK_TARGET_SAME_APP, TABLE));
   }
}

//-----------------------------------------------------------------------------
/// Destructor
//-----------------------------------------------------------------------------
Rovhult::~Rovhult () {
   TRACE8 ("Rovhult::~Rovhult ()");
   clean ();
}


//-----------------------------------------------------------------------------
/// Starts the game
//-----------------------------------------------------------------------------
void Rovhult::start () {
   Game::start ();

   setGameStatus (EXCHANGE);
   if (randomizeCardsToPile (staple)) {
      dealCards ();

      pos1Play = pos2Play = -1U;
      aExchanged = 0;

      if (getConnectionMgr ().getMode () != YGP::ConnectionMgr::CLIENT) {
         setNextPlayer (rand () & 0x3);

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
   }
}

//-----------------------------------------------------------------------------
/// Callback after finishing card-exchange
/// \param iCard: Offset of card in hand
//-----------------------------------------------------------------------------
void Rovhult::finishedExchange (unsigned int iCard) {
   unregisterDND ();
   disableHuman ();
   setGameStatus (EXCHANGED);

   sortReserve (0);
   players[0].hand.setStyle (ICardPile::COMPRESSED);

   sendExchangedCards (0);

   exchangeAutoplayerCards ();

   players[0].hand.sortByNumber ();
   sortReserve (0);
   players[0].hand.setStyle (ICardPile::COMPRESSED);

   status.pop ();
   displayTurn (currentPlayer ());

   if ((getConnectionMgr ().getMode () == YGP::ConnectionMgr::NONE)
       || (aExchanged == 0xf)) {
      setGameStatus (PLAYING);
      makeNextMoves ();
   }
   else
      status.push (_("Waiting for other player to exchange their cards ..."));
}

//-----------------------------------------------------------------------------
/// Compares two cards according the rules of Rovhult
/// \param lhs, rhs: Cards to compare
/// \returns \c int: >0, if number of lhs is smaller; 0 if equal or >0 if
///     bigger
//-----------------------------------------------------------------------------
int Rovhult::compareCards (const CardWidget& lhs, const CardWidget& rhs) {
   unsigned int lhsValue ((lhs.number () == CardWidget::TWO) ? CardWidget::ACE + 1
                          : (lhs.number () == CardWidget::TEN) ? CardWidget::ACE + 2 :
                          lhs.number ());
   unsigned int rhsValue ((rhs.number () == CardWidget::TWO) ? CardWidget::ACE + 1
                          : (rhs.number () == CardWidget::TEN) ? CardWidget::ACE + 2 :
                          rhs.number ());
   TRACE9 ("Rovhult::compareCards (const CardWidget&, const CardWidget&) - "
           << lhsValue << " - " << rhsValue << " = " << (int)(lhsValue - rhsValue));

   return lhsValue - rhsValue;
}

//-----------------------------------------------------------------------------
/// Exchanges the cards of the computer-players
/// \param player: Not really a void*, but actually the (next computer)player
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
                CardWidget& cardPile (players[i].reserve[posPile].removeTopCard ());
                CardWidget& cardHand (players[i].hand.remove (posHand, true));

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
/// \param player: Player whose cards to send 
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
/// \param player: Player whose cards should be sorted
//-----------------------------------------------------------------------------
void Rovhult::sortReserve (unsigned int player) {
   // Sort cards on piles
   for (int j (0); j < 2; ++j)
      for (int k (j); k >= 0; --k)
         if (compareCards (players[player].reserve[k + 1].getTopCard (),
                           players[player].reserve[k].getTopCard ()) < 0) {
            CardWidget& low (players[player].reserve[k + 1].removeTopCard ());
            CardWidget& high (players[player].reserve[k].removeTopCard ());

            TRACE3 ("Rovhult::exchangeAutoplayerCards () - exchanging card "
                    << low << " on pile " << (k + 1) << " with card " << high
                    << " on pile " << k);

            players[player].reserve[k + 1].setTopCard (high);
            players[player].reserve[k].setTopCard (low);
         }
}

//-----------------------------------------------------------------------------
/// Enables the cards of the human player
/// \param player: Player to enable
//-----------------------------------------------------------------------------
bool Rovhult::enableHuman () {
   Check3 (activeCards.empty ());

   if (players[0].hand.size ()) {
      TRACE2 ("Rovhult::enableHuman () - Has " << players[0].hand.size ()
              << " card(s) in the hand");

      for (int i (players[0].hand.size ()); i;)
         activeCards.push_back
            (players[0].hand[--i]->signal_clicked ().connect
             (bind (slot (*this, &Rovhult::handSelected), i)));
   }
   else {
      TRACE2 ("Rovhult::enableHuman () - Enable reserve of human");

      for (int i (0); i < 3; ++i)
         if (players[0].reserve[i].size ()) {
            TRACE8 ("Rovhult::enableHuman () - Pile " << i << " has "
                    << players[0].reserve[i].size () << " card(s)");
            activeCards.push_back
               (players[0].reserve[i].getTopCard ().signal_clicked ().connect
                (bind (slot (*this, &Rovhult::pileSelected), i)));
         }
   }

   if (played.size ()) {
      TRACE2 ("Rovhult::enablePlayer (unsigned int) - Enable last played card");
      activeCards.push_back (played.getTopCard ().signal_clicked ().connect
                             (slot (*this, &Rovhult::takeCards)));
   }
   return Game::enableHuman ();
}

//-----------------------------------------------------------------------------
/// Callback after clicking on a card on the table
/// \param pile: Offset of selected pile
//-----------------------------------------------------------------------------
void Rovhult::pileSelected (unsigned int pile) {
   TRACE1 ("Rovhult::pileSelected (unsinged int) - Pile " << pile);
   Check3 (pile < 3);

   ICardPile& actPile (players[0].reserve[pile]);
   CardWidget& card (actPile.getTopCard ());
   bool showsFace (card.showsFace ());

   // If played from bottom of pile (with invisible cards): Flip card first
   if (!showsFace) {
      for (unsigned int i (0); i < 3; ++i)
          if (players[0].reserve[i].size ()
              && players[0].reserve[i].getTopCard ().showsFace ()) {
             Gtk::MessageDialog dlg (_("You must first play the visible cards!"),
                                     Gtk::MESSAGE_ERROR);
             dlg.set_title (_("Invalid move"));
             dlg.run ();
             return;
          }
      card.showFace ();
   }

   TRACE1 ("Rovhult::pileSelected (unsinged int) - Card " << card);

   if (!cardValid (card.number ())) {  // If selected card is not valid: Return
      if (!showsFace) {
         played.append (actPile.removeTopCard ());

         // Inform the others about the move
         if (getConnectionMgr ().getMode () != YGP::ConnectionMgr::NONE) {
            // Send played card to all clients (if any)
            std::ostringstream msg;
            msg << "Play=" << card.id () << ";Target=" << (pile + 5);
            if (getConnectionMgr ().getMode () == YGP::ConnectionMgr::CLIENT)
                ignoreNextMsg = true;
            broadcastMessage (msg.str ());
         }
         setNextPlayer (movePlayedCardsToLooser (0));
         makeNextMoves ();
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
      playFromPile (pile);
   else
      Glib::signal_timeout ().connect (bind (slot (*this, &Rovhult::playFromPile),
                                             pile), ComputerPlayer::TIMEOUT);
}

//-----------------------------------------------------------------------------
/// Performs playing from a pile
/// \param pile: Offset of selected pile
/// \returns \c bool: Always false to stop the timer (if called from one,
///     that's it)
//-----------------------------------------------------------------------------
bool Rovhult::playFromPile (unsigned int pile) {
   Check1 (pile < 3);
   TRACE1 ("Rovhult::playFromPile (unsinged int) - Card at pos " << pile);

   setNextPlayer (doPileSelected (0, pile));
   makeNextMoves ();
   return false;
}

//-----------------------------------------------------------------------------
/// Executes the move from a pile: Moves the cards and enables next
/// \param player: ID of player
/// \param pile: Offset of selected pile
/// \returns \c int: player to continue
//-----------------------------------------------------------------------------
int Rovhult::doPileSelected (unsigned int player, unsigned int pile) {
   TRACE1 ("Rovhult::doPileSelected (unsigned int, unsinged int) - " 
           << player << '/' << pile);
   Check1 (player < NUM_PLAYERS); Check1 (pile < 3);

   ICardPile* actPile (&players[player].reserve[pile]);
   Check3 (actPile->size ());
   CardWidget& card (actPile->removeShownTopCard ());

   // Move card (and visible cards with equal number on other piles) from
   // player to played staple
   if (card.number () == CardWidget::TEN)
      played.clear ();
   else
      played.append (card);

   while (pile) {
      actPile = &players[player].reserve[--pile];
      if (actPile->size ()
          && actPile->topCardShowsFace ()
          && (actPile->getTopCard ().number () == card.number ())) {
         CardWidget& sameCard (actPile->removeShownTopCard ());
         if (sameCard.number () != CardWidget::TEN)
            played.append (sameCard);
      }
      else
         break;
   }

   return executeMove (player, card.number ());
}


//-----------------------------------------------------------------------------
/// Check if played card is valid (equal or bigger) The following cards have
/// special meaning: - 2: Can be played always - 7: The next card must be
/// equal or *smaller* - 8: Skips the next player -10: Clears the staple; the
/// same player can continue with cards in hand
/// \param nr: Card to check
/// \param silent: Flag, if error should be displayed
/// \returns \c bool: True, if card can be played
//-----------------------------------------------------------------------------
bool Rovhult::cardValid (CardWidget::NUMBERS nr, bool silent) const {
   TRACE5 ("Rovhult::cardValid (CardWidget::NUMBERS, bool) const - Checking "
           << nr << " in " << played.size () << " cards");

   switch (nr) {
   case CardWidget::TEN:
   case CardWidget::TWO:
      break;

   default:
      if (played.size ()) {
         Glib::ustring error;

         CardWidget& lastPlayed (played.getTopCard ());
         if (lastPlayed.number () == CardWidget::SEVEN) {
            if (nr > CardWidget::SEVEN)
               error = _("After a 7, the played card must be equal or smaller!");
         }
         else
            if (nr < lastPlayed.number ())
               error = _("Played card must be equal or bigger!");

         if (error.size ()) {
            if (!silent) {
               Gtk::MessageDialog dlg (error, Gtk::MESSAGE_ERROR);
               dlg.set_title (_("Invalid move"));
               dlg.run ();
            }
            return false;
         }
      }
   } // end-switch

   return true;
}

//-----------------------------------------------------------------------------
/// Callback after clicking on a card in hand
/// \param pos: Offset of card in hand
//-----------------------------------------------------------------------------
void Rovhult::handSelected (unsigned int pos) {
   TRACE3 ("Rovhult::handSelected (unsinged int) - Checking card at " << pos);
   Check3 (pos < players[0].hand.size ());

   CardWidget& card (*players[0].hand[pos]);
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

   setNextPlayer (executeMove (0, card.number ()));
   makeNextMoves ();
}

//-----------------------------------------------------------------------------
/// Move card (and cards with equal number below) from player to played
/// staple. The cards are replaced, if the staple contains cards
/// \param player: ID of player who played the last card
/// \param start: Offset of first card in hand to play
/// \param end: Offset of last card in hand to play
/// \returns \c CardWidget::NUMBERS: Number of played card
//-----------------------------------------------------------------------------
CardWidget::NUMBERS Rovhult::playCardsFromHand (unsigned int player, unsigned int start,
                                                unsigned int end) {
   TRACE5 ("Rovhult::playCardsFromHand (unsigned int, unsigned int, unsigned int)"
           " - Player " << player << " from " << start << " to " << end);
   Check3 (end < players[player].hand.size ());
   Check3 (start <= end);
   CardWidget::NUMBERS nr (players[player].hand[start]->number ());

   do {
      CardWidget& movedCard (players[player].hand.remove (start));
      if (movedCard.number () == CardWidget::TEN)
         played.clear ();
      else
         played.append (movedCard);
   } while (start < end--);

   // If staple contains cards and no 10 was played (except if hand is empty):
   // Fill up cards til player has 3 (or one, in case of a ten)
   if ((nr != CardWidget::TEN) || (!players[player].hand.size ()))
      fillUpPile (players[player].hand, (nr != CardWidget::TEN) ? 3 : 1);

   return nr;
}

//-----------------------------------------------------------------------------
/// Executes the move -> Check consequences for next in round and calculate
/// next player
/// \param player: ID of player who played the last card
/// \param nr: Played card
/// \returns \c int: The next player
//-----------------------------------------------------------------------------
int Rovhult::executeMove (unsigned int player, CardWidget::NUMBERS nr) {
   TRACE3 ("Rovhult::executeMove (unsigned int, CardWidget::NUMBERS) - Player "
           << player << "; Card " << nr);
   Check3 (player < NUM_PLAYERS);

   Glib::ustring stat;

   // If last 4 cards have the same number or ten was played: Don't increase
   // player (except of course, if actual player don't have anymore cards)
   if (!((nr == CardWidget::TEN) || clearPlayedIf4Equal ())
       || (static_cast<int> (player) != nextAvailablePlayer ((player - 1) & 0x3))) {
      player = nextAvailablePlayer (player);

      Check3 (actPlayers.size () > player);
      Check3 (actPlayers[player]);
      if (nextAvailablePlayer (player) == -1) {
         status.pop ();
         stat = _("%1 lost");
         stat.replace (stat.find ("%1"), 2, actPlayers[player]->getName ());
         status.push (stat);
         setGameStatus (STOPPED);
         return -1;
      }

      if (nr == CardWidget::EIGHT) {
         stat = _("Skipping %1; ");
         stat.replace (stat.find ("%1"), 2, actPlayers[player]->getName ());
         player = nextAvailablePlayer (player);
      }
   }
   else
      stat = _("Pile cleared; ");

   displayTurn (player, stat);
   return player;
}

//-----------------------------------------------------------------------------
/// Callback after selection top card on played pile -> Moves all its card to
/// the passed player
/// \param player: ID of player picking up the cards
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

   setNextPlayer (movePlayedCardsToLooser (0));
   makeNextMoves ();
}

//-----------------------------------------------------------------------------
/// Checks if the passed pile has a card which can be played
/// \param player: ID of player to analyze
/// \param card: Last played card
//-----------------------------------------------------------------------------
bool Rovhult::playerCanContinue (unsigned int player, CardWidget::NUMBERS card) const {
   TRACE3 ("Rovhult::playerCanContinue (unsigned int, CardWidget::NUMBERS) const - "
           << player << "; Card: " << card);
   Check3 (player < NUM_PLAYERS);

   if (card == CardWidget::UNREACHABLE)
      return false;
 
   if (players[player].hand.size ())
      return playerHandCanContinue (players[player].hand, card);

   // Check pile: Analyze only visible cards; if there are none, return true
   bool hasNoVisibleCards (true);
   for (int i (0); i < 3; ++i)
      if (players[player].reserve[i].size () > 1) {
         TRACE5 ("Rovhult::playerCanContinue (unsigned int, CardWidget::NUMBERS) const"
                 " - Checking pile " << i << "; "
                 << players[player].reserve[i].size () << " cards");
         hasNoVisibleCards = false;

         CardWidget::NUMBERS nr (players[player].reserve[i].getTopCard ().number ());
         switch (nr) {
         case CardWidget::TWO:
         case CardWidget::TEN:
            return true;

         default:
            TRACE7 ("Rovhult::playerCanContinue (unsigned int, CardWidget::NUMBERS) const"
                    " - Value of card: " << nr);
            if ((card == CardWidget::SEVEN)
                ? (nr <= CardWidget::SEVEN) : (nr >= card))
               return true;
         } // end-switch card
      } // endif pile contains cards

   return hasNoVisibleCards ? (card != CardWidget::UNREACHABLE) : false;
}

//-----------------------------------------------------------------------------
/// Checks if the passed pile has a card which can be played
/// \param player: ID of player to analyze
/// \param card: Last played card
//-----------------------------------------------------------------------------
bool Rovhult::playerHandCanContinue (const ICardPile& pile, CardWidget::NUMBERS card) const {
   TRACE3 ("Rovhult::playerHandCanContinue (const ICardPile&, CardWidget::NUMBERS) const"
           << " - Card " << card << " in " << pile.size () << " cards");
   Check3 (pile.size ());

   // Check if first/last is smaller/bigger then passed one
   if (card == CardWidget::SEVEN) {
      if (pile[0]->number () <= card)
         return true;
   }
   else {
      if (pile[pile.size () - 1]->number () >= card)
         return true;
   }

   TRACE3 ("Rovhult::playerHandCanContinue (const ICardPile&, CardWidget::NUMBERS"
           " - Special check");
   // Simple check failed -> Check for special card (2 or 10)
   if (pile[0]->number () == CardWidget::TWO)
      return true;

   return pile.exists (CardWidget::TEN);
}

//-----------------------------------------------------------------------------
/// Fills up the passed pile til it contains the specified number of cards
/// \param pile: Pile to fill up
/// \param minCards: Minimal number of cards pile should hold
//-----------------------------------------------------------------------------
void Rovhult::fillUpPile (ICardPile& pile, unsigned int minCards) {
   TRACE3 ("Rovhult::fillUpPile (ICardPile&, unsinged int) - "
           << pile.size () << " -> " << minCards);

   while ((pile.size () < minCards) && staple.size ())
      pile.insertSorted (staple.removeTopCard ());
}

//-----------------------------------------------------------------------------
/// Returns the number of equal cards from the played pile
/// \returns \c unsigned int: Number of equal cards
//-----------------------------------------------------------------------------
unsigned int Rovhult::numberOfEqualTopCards () const {
   TRACE8 ("Rovhult::numberOfEqualTopCards () const");
   unsigned int nrCards (played.size ());

   if (nrCards)
      --nrCards;
   else
      return 0;

   unsigned int i (1);
   CardWidget& card (played.getTopCard ());
   while (i <= nrCards) {
      TRACE9 ("Rovhult::numberOfEqualTopCards () const - Checking "
              << played[nrCards - i] << " with " << card);
      
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
/// \returns \c bool: True, if 4 equal cards found
//-----------------------------------------------------------------------------
bool Rovhult::clearPlayedIf4Equal () {
   TRACE8 ("Rovhult::clearPlayedIf4Equal ()");

   int cards (numberOfEqualTopCards ()); Check3 (cards <= 4);
   if (cards < 4)
      return false;

   TRACE7 ("Rovhult::clearPlayedIf4Equal () - found 4");
   played.clear ();
   return true;
}

//-----------------------------------------------------------------------------
/// Method to move the cards of the actual round to the winner
/// \param nrLooser: Nr. of player getting all played cards
/// \returns \c unsigned int: Next player
//-----------------------------------------------------------------------------
unsigned int Rovhult::movePlayedCardsToLooser (unsigned int nrLooser) {
   TRACE8 ("Rovhult::movePlayedCardsToLooser () - Player " << nrLooser << " gets "
           << played.size () << " cards");
   Check3 (nrLooser < NUM_PLAYERS);
   Check3 (played.size ());
   Check3 (actPlayers[nrLooser]);

   movePile (players[nrLooser].hand, played);
   players[nrLooser].hand.sortByNumber ();
   Glib::ustring stat (_("%1 can't continue -> Taking the whole pile. "));
   stat.replace (stat.find ("%1"), 2, actPlayers[nrLooser]->getName ());
   displayTurn (nrLooser = nextAvailablePlayer (nrLooser), stat);
   return nrLooser;
}

//-----------------------------------------------------------------------------
/// Checks which player has still cards left
/// \param actPlayer: ID of actual player
/// \returns \c int: ID of player or -1 (if none can continue)
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

   players[0].hand.setStyle (ICardPile::NORMAL);
   disableHuman ();

   Game::clean ();
}

//-----------------------------------------------------------------------------
/// Prepares the card for drag´n´drop (starting from the table, ending on the
/// hand or ending on the table, starting from the hand)
/// \param card: Card to prepare for drag´n´drop
/// \param pile: Number of pile on reserve holding card
//-----------------------------------------------------------------------------
void Rovhult::registerTableDND (CardWidget& card, unsigned int pile) {
   TRACE8 ("Rovhult::registerTableDND (CardWidget&, unsigned int) - "
           << card << " for pile " << pile);
   Check1 (pile < 3);
   Check3 (gameStatus () == EXCHANGE);

   // Card accepts drops from hand and drags from table
   card.drag_dest_set (dndTypeHand, Gtk::DEST_DEFAULT_ALL, Gdk::ACTION_MOVE);
   card.drag_source_set
      (dndTypeTable, Gdk::ModifierType (GDK_BUTTON2_MASK | GDK_BUTTON3_MASK),
                                        Gdk::ACTION_MOVE);

   card.drag_source_set_icon (card.getImage ());
   aTableDND[&card] = card.signal_drag_data_received ().connect
       (bind (slot (*this, &Rovhult::cardDroppedOnTable), pile));
   aTableData[&card] = card.signal_drag_data_get ().connect
       (bind (slot (*this, &Rovhult::getDropData), pile));
}

//-----------------------------------------------------------------------------
/// Prepares the card for drag´n´drop (starting from the hand ending on table
/// or ending on hand, starting from table)
/// \param card: Card to prepare for drag´n´drop
/// \param pile: Number of pile on reserve holding card
//-----------------------------------------------------------------------------
void Rovhult::registerHandDND (CardWidget& card,  unsigned int iCard) {
   TRACE8 ("Rovhult::registerHandDND (CardWidget&, unsigned int) - " << card
           << "; pos " << iCard);
   Check3 (gameStatus () == EXCHANGE);

   // Card accepts drops from table and drags from hand
   card.drag_dest_set (dndTypeTable, Gtk::DEST_DEFAULT_ALL, Gdk::ACTION_MOVE);
   card.drag_source_set
      (dndTypeHand,
       Gdk::ModifierType (GDK_BUTTON2_MASK | GDK_BUTTON3_MASK), Gdk::ACTION_MOVE);

   card.drag_source_set_icon (card.getImage ());
   aHandDND[&card] = card.signal_drag_data_received ().connect
       (bind (slot (*this, &Rovhult::cardDroppedOnHand), iCard));
   aHandData[&card] = card.signal_drag_data_get ().connect
       (bind (slot (*this, &Rovhult::getDropData), iCard));

   TRACE9 ("Rovhult::registerHandDND (CardWidget&, unsigned int) - Activate: "
           << activeCards.size () << '/' << activeCards.capacity ());
   Check3 (activeCards.size () >= iCard);
   SigC::Connection conn (card.signal_clicked ().connect
                          (bind (slot (*this, &Rovhult::finishedExchange), iCard)));
   if (activeCards.size () > iCard)
      activeCards[iCard] = conn;
   else
      activeCards.push_back (conn);
   TRACE9 ("Rovhult::registerHandDND (CardWidget&, unsigned int) - End");
}

//-----------------------------------------------------------------------------
/// Stops the drag´n´drop abilities of the passed card
/// \param card: Card to unregister of dnd
//-----------------------------------------------------------------------------
void Rovhult::unregisterDND (CardWidget& card) const {
   card.drag_dest_unset ();
   card.drag_source_unset ();
}

//-----------------------------------------------------------------------------
/// Stops the drag´n´drop abilities of the cards of player 0
/// \param card: Card to unregister of dnd
//-----------------------------------------------------------------------------
void Rovhult::unregisterDND () {
   TRACE8 ("Rovhult::unregisterDND () - Status: " << gameStatus ());
   Check3 (gameStatus () == EXCHANGE);
   Check3 (aHandDND.size () == players[0].hand.size ());
   Check3 (aTableDND.size () == players[0].hand.size ());

   for (unsigned int i (0); i < players[0].hand.size (); ++i) {
      CardWidget& card (*players[0].hand[i]);
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
void Rovhult::disconnectCardInHand (const CardWidget& card) {
   TRACE3 ("Rovhult::disconnectCardInHand (const CardWidget&) - " << card);
   Check1 (aHandDND.find (&card) != aHandDND.end ());
   Check1 (aHandData.find (&card) != aHandData.end ());

   aHandDND[&card].disconnect ();
   aHandData[&card].disconnect ();
   aHandDND.erase (&card);
   aHandData.erase (&card);
}

//-----------------------------------------------------------------------------
/// Disconnects the card (on the table) from every connection hold
//-----------------------------------------------------------------------------
void Rovhult::disconnectCardOnTable (const CardWidget& card) {
   TRACE3 ("Rovhult::disconnectCardOnTable (const CardWidget&) - " << card);
   Check1 (aTableDND.find (&card) != aTableDND.end ());
   Check1 (aTableData.find (&card) != aTableData.end ());

   aTableDND[&card].disconnect ();
   aTableData[&card].disconnect ();
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
            CardWidget& card (staple.removeTopCard ());
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
/// Callback after dropping a card onto (cards on) table
/// \param pContext: Context of the drag (contains things like source,
/// \param target, action, ...)
/// \param pData: Describes the thing which was dropped
/// \param info: Describes the type of pData (should be 0)
/// \param time: Timestamp of the drag
/// \param pile: Number of pile
/// \param Requieres : pContext, pData not NULL; Expects info to be 0
//-----------------------------------------------------------------------------
void Rovhult::cardDroppedOnTable (const Glib::RefPtr<Gdk::DragContext>& context,
                                  gint, gint, GtkSelectionData* pData, guint info,
                                  guint32 time, unsigned int pile) {
   Check3 (pData);
   Check3 (!context->get_is_source ());
   Check3 (pData->length == sizeof (int));
   Check3 (pData->format == 8);

   Check3 (pile < 3);
   
   unsigned int* pValue (reinterpret_cast <unsigned int*> (pData->data));
   Check3 (pValue);

   TRACE1 ("Rovhult::cardDroppedOnTable (...) - Data = "
           << *pValue << " <-> " << pile);

   // Check if player matches
   CardWidget& cardTable (players[0].reserve[pile].removeTopCard ());
   CardWidget& cardHand (players[0].hand.remove (*pValue));

   TRACE1 ("Rovhult::cardDroppedOnTable (...) - Exchanging cards "
           << cardHand.id () << "<->" << cardTable.id ());

   // End old dnd
   context->drag_finish (true, false, time);

   activeCards[*pValue].disconnect ();
   disconnectCardInHand (cardHand);
   disconnectCardOnTable (cardTable);
   unregisterDND (cardHand);
   unregisterDND (cardTable);

   players[0].reserve[pile].setTopCard (cardHand);
   players[0].hand.insert (cardTable, *pValue);

   registerHandDND (cardTable, *pValue);
   registerTableDND (cardHand, pile);
}

//-----------------------------------------------------------------------------
/// Callback after dropping a card onto onto hand
/// \param context: Context of the drag (contains things like source,
/// \param target, action, ...)
/// \param pData: Describes the thing which was dropped
/// \param info: Describes the type of pData (should be 0)
/// \param time: Timestamp of the drag
/// \param card: Number of card
/// \param Requieres : pData not NULL; Expects info to be 0
//-----------------------------------------------------------------------------
void Rovhult::cardDroppedOnHand (const Glib::RefPtr<Gdk::DragContext>& context,
                                 gint, gint, GtkSelectionData* pData, guint info,
                                 guint32 time, unsigned int card) {
   Check3 (pData);
   Check3 (!context->get_is_source ());
   Check3 (pData->length == sizeof (int));
   Check3 (pData->format == 8);

   Check3 (card < players[0].hand.size ());

   unsigned int* pValue (reinterpret_cast <unsigned int*> (pData->data));
   Check3 (pValue);

   TRACE1 ("Rovhult::cardDroppedOnHand (...) - Data = "
           << *pValue << " <-> " << card);

   CardWidget& cardTable (players[0].reserve[*pValue].removeTopCard ());
   CardWidget& cardHand (players[0].hand.remove (card));

   TRACE1 ("Rovhult::cardDroppedOnHand (...) - Exchanging cards "
           << cardHand.id () << "<->" << cardTable.id ());

   // End old DND
   context->drag_finish (true, false, time);

   activeCards[card].disconnect ();
   disconnectCardInHand (cardHand);
   disconnectCardOnTable (cardTable);
   unregisterDND (cardHand);
   unregisterDND (cardTable);

   // Swap cards
   players[0].reserve[*pValue].setTopCard (cardHand);
   players[0].hand.insert (cardTable, card);

   // Adapt dnd-settigns
   registerHandDND (cardTable, card);
   registerTableDND (cardHand, *pValue);
}

//-----------------------------------------------------------------------------
/// Callback to query the data to drop
/// \param context: Context of the drag (contains things like source,
/// \param target, action, ...)
/// \param pData: Describes the thing which was dropped
/// \param time: Timestamp of the drag
/// \param cardPos: Position of card (either in hand or pile on table)
/// \param Requieres : pContext, pData not NULL; Expects info to be 0
//-----------------------------------------------------------------------------
void Rovhult::getDropData (const Glib::RefPtr<Gdk::DragContext>& context,
                           GtkSelectionData* pData, guint info, guint32 time,
                           unsigned int cardPos) {
   Check3 (pData); Check3 (info < 2);
   Check3 (context->get_is_source ());

   gtk_selection_data_set (pData, pData->target, 8, reinterpret_cast <guchar*> (&cardPos),
                           sizeof (cardPos));
}

//-----------------------------------------------------------------------------
/// Shows the cards the user is about to play
/// \param player: Player in turn
//-----------------------------------------------------------------------------
void Rovhult::showCards2Play (unsigned int player) {
   TRACE2 ("Rovhult::showCards2Play (unsigned int, unsigned int) - "
           "For player " << player);
   Check3 (player < NUM_PLAYERS);

   findCard2Play (player, pos1Play, pos2Play);
   if (players[player].hand.size ())
      flipCards2Play (players[player].hand, pos1Play, pos2Play);
   else {
      ICardPile& pile (players[player].reserve[pos2Play]);

      if ((pos1Play == pos2Play)
          && (cardValid (pile.getTopCard ().number (), true))) {
         pile.getTopCard ().showFace ();

         // Inform the others about the move
         if (getConnectionMgr ().getMode () == YGP::ConnectionMgr::SERVER) {
            // Send played card to all clients (if any)
            std::ostringstream msg;
            msg << "Play="
                << players[player].reserve[pos2Play].getTopCard ().id ()
                << ";Target=" << (pos2Play + 5);
            broadcastMessage (msg.str ());
         }
      }
      else {
         // Inform the others about the move
         if (getConnectionMgr ().getMode () == YGP::ConnectionMgr::SERVER) {
            // Send played card to all clients (if any)
            std::ostringstream msg;
            msg << "Play="
                << players[player].reserve[pos2Play].getTopCard ().id ()
                << ";Target=" << (pos2Play + 5);
            broadcastMessage (msg.str ());
         }
      }
   }
}

//-----------------------------------------------------------------------------
/// Finds an executes the turn of a (computer controled) player
/// \returns \c int: The next player
//-----------------------------------------------------------------------------
int Rovhult::makeMove (unsigned int player) {
   TRACE2 ("Rovhult::makeMove (unsigned int) - Player " << player);

   if (pos2Play == -1U) {
      if (playerCanContinue (player,
                             (played.size ()
                              ? played.getTopCard ().number () : CardWidget::TWO)))
         showCards2Play (player);
      else {
         if (getConnectionMgr ().getMode () == YGP::ConnectionMgr::SERVER) {
            std::ostringstream msg;
            msg << "Play=" << played[played.size () - 1]->id () << ";Target=4";
            broadcastMessage (msg.str ());
         }

         player = movePlayedCardsToLooser (player);
      }
   }
   else {
      TRACE2 ("Rovhult::makeMove (unsigned int) - play cards " << pos1Play
              << " to " << pos2Play);
      Check3 (pos1Play >= 0);
      Check3 (pos1Play <= pos2Play);

      if (players[player].hand.size ()) {
         Check3 (pos2Play < players[player].hand.size ());

         if (players[player].hand[pos1Play]->number () == CardWidget::TEN) {
            Check3 (players[player].hand[pos2Play]->number () == CardWidget::TEN);
            played.clear ();
         }

         player = executeMove (player, playCardsFromHand (player, pos1Play, pos2Play));
      }
      else {
         Check3 (pos1Play <= pos2Play); Check3 (pos2Play < 3);

         // If cards are visible
         if (players[player].reserve[pos1Play].size () > 1)
            player = doPileSelected (player, pos2Play);         // Execute move
         else {
            Check3 (pos1Play == pos2Play);

            // If card is valid: Play it
            CardWidget& card (players[player].reserve[pos2Play].getTopCard ());
            if (cardValid (card.number (), true))
               player = doPileSelected (player, pos2Play);
            else {
               // Card is not valid: Take up pile
               players[player].reserve[pos2Play].removeTopCard ();
               played.append (card);
               movePlayedCardsToLooser (player);
               player = executeMove (player, CardWidget::UNREACHABLE);
            }
         }
      }
      pos2Play = -1U;
   }
   return player;
}

//-----------------------------------------------------------------------------
/// Checks if there are only special cards up to the passed position
/// \param pile: Pile to inspect
/// \param start: Lower position of cards to inspect
/// \param end: Upper position of cards to inspect
/// \returns \c bool: True, if there are only special cards
//-----------------------------------------------------------------------------
bool Rovhult::existOnlySpecialCards (const ICardPile& pile, unsigned int start,
                                     unsigned int end) const {
   Check3 (start <= end);
   Check3 (end < pile.size ());

   do {
      if (!isSpecialCard (pile[start]->number ()))
         return false;
   } while (++start <= end);

   TRACE8 ("Rovhult::existOnlySpecialCards (const ICardPile&, unsigned int) - Yes");
   return true;
}

//-----------------------------------------------------------------------------
/// Finds the next card to play (for a computer controlled player)
/// \param player: Player to inspect
/// \param start: Position of (first) card to play
/// \param end: Position of (last) card to play
//-----------------------------------------------------------------------------
void Rovhult::findCard2Play (unsigned int player, unsigned int& start,
                             unsigned int& end) const {
   TRACE2 ("Rovhult::findCard2Play (unsigned int) - Player " << player);
   Check3 (player < NUM_PLAYERS);

   // Search for minimal card to play; this is either a card equal or bigger
   // or - if no previous card is played or the last card played was a 7 -
   // the smallest available
   CardWidget::NUMBERS cardMin (CardWidget::THREE);
   if (played.size ()
       && (played.getTopCard ().number () != CardWidget::SEVEN)
       && (played.getTopCard ().number () != CardWidget::TWO))
      cardMin = played.getTopCard ().number ();

   TRACE5 ("Rovhult::findCard2Play (unsigned int) - Card to beat " << cardMin);

   // Check if to play from hand or to play from reserve
   unsigned int nrCards (players[player].hand.size ());
   if (nrCards) {
      // Special handling if cards of next player are know: Try to give him
      // the whole pile
      CardWidget::NUMBERS nextMin, nextMax;
      int hpPos (-1);
      start =
         (played.size ()
          && getPileLimits (nextAvailablePlayer (player), nextMin, nextMax)
          && (((nextMin > CardWidget::SEVEN)
               && ((hpPos = players[player].hand.find (CardWidget::SEVEN)) =! -1))
              || (((hpPos = (players[player].hand.findFirstEqualOrBigger
                             (CardWidget::NUMBERS (nextMax + 1))))
                   != -1)
                  && ((hpPos = skip (CardWidget::SEVEN, players[player].hand, hpPos))
                      != -1)
                  && ((hpPos = skip (CardWidget::TEN, players[player].hand, hpPos))
                      != -1))
              && cardValid (players[player].hand[hpPos]->number (), true)))
         ? hpPos : players[player].hand.findFirstEqualOrBigger (cardMin);
   
      TRACE6 ("Rovhult::findCard2Play (unsigned int) - First matching card"
              " at pos " << start);
   
      // Check if no matching normal card is found or found card is bigger than
      // the played 7. If so, use special card instead
      // We know one card must match as "playerCanContinue" reported this player
      // as valid
      if ((start == -1U)
          || (played.size ()
              && ((played.getTopCard ().number () == CardWidget::SEVEN)
                  && players[player].hand[start]->number () > CardWidget::SEVEN))) {
         TRACE7 ("Rovhult::findCard2Play (unsigned int) - Ordinary cards don't"
                 " match -> Searching for special card");

         if (players[player].hand[0]->number () == CardWidget::TWO)
            start = 0;
         else {
            start = players[player].hand.findFirstEqualOrBigger (CardWidget::TEN);
            Check3 (start != -1);
            Check3 (players[player].hand[start]->number () == CardWidget::TEN);
         }
         TRACE7 ("Rovhult::findCard2Play (unsigned int) - Using special card "
                 << players[player].hand[start] << " at pos " << start);
      }
      else {
         Check3 ((cardMin == CardWidget::SEVEN)
                 ? (players[player].hand[start]->number () <= CardWidget::SEVEN)
                 : (players[player].hand[start]->number () >= cardMin));

         // If player would continue with a 6, but has also a 7, play that
         // card instead
         if (players[player].hand[start]->number () == CardWidget::SIX) {
            if ((end = players[player].hand.find (CardWidget::SEVEN, start)) != -1U) {
               TRACE8 ("Rovhult::findCard2Play (unsigned int) - Exchanging "
                       << *players[player].hand[start] << " with "
                       << *players[player].hand[end]);
               start = end;
            }
         }
         else
            // The search of CardWidget does not know (and shall not know anything)
            // about the special meaning of tens, so skip them by yourself,
            // but use a TWO (if available) in case a TEN is/are the last card(s)
            if ((end = skip (CardWidget::TEN, players[player].hand, start)) == -1U) {
               if (players[player].hand[0]->number () == CardWidget::TWO)
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
                     || (players[player].hand[start]->number () < CardWidget::TEN))))
         ? last : start;

      return;
   }
   else {
      TRACE5 ("Rovhult::findCard2Play (unsigned int) - Analyzing reserve");

      // Bitfield for lower cards: Bit 0: Cards visible; Bit 1: Normal cards
      int bfLowerCardsInfo (0);

      // Play first visible cards
      for (start = 0; start < 3; ++start) {
         const ICardPile* pPile (&players[player].reserve[start]);
         if (pPile->size ()) {
            CardWidget& card (pPile->getTopCard ());
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

                  TRACE7 ("Rovhult::findCard2Play (unsigned int) - Playing "
                          "visible card " << card << " at pos " << end);
                  return;
               }
            }
         }
      }

      // No card visible: Play the first
      if (!bfLowerCardsInfo) {
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
/// \param player: Player whose card to analyze
/// \param min: Returns the minimal card
/// \param max: Returns the maximal card
/// \returns \c bool: true, if cardinfo is available
//-----------------------------------------------------------------------------
bool Rovhult::getPileLimits (unsigned int player, CardWidget::NUMBERS& min,
                             CardWidget::NUMBERS& max) const {
   if (players[player].hand.size ())
      return false;

   TRACE3 ("Rovhult::getPileInfo (unsigned int, unsigned int&, unsigned int&)"
           " - Analyzing cards of player " << player);

   bool cardFound (false);
   for (unsigned int i (0); i < 3; ++i) {
      const ICardPile* pPile (&players[player].reserve[i]);
      if (pPile->size ()) {
         CardWidget& card (pPile->getTopCard ());
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
/// \param open: Flag if cards should be shown or hidden
//-----------------------------------------------------------------------------
void Rovhult::playOpen (bool open) {
   ICardPile::ShowOpt show (open ? ICardPile::SHOWFACE : ICardPile::SHOWBACK);

   for (unsigned int i (1); i < NUM_PLAYERS; ++i) {
      players[i].hand.setShowOption (show);
      players[i].hand.setStyle ((show == ICardPile::SHOWFACE)
                                ? ICardPile::COMPRESSED
                                : ICardPile::QUITE_COMPRESSED);
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
/// \param newPlayer: Array holding the new player
//-----------------------------------------------------------------------------
void Rovhult::changeNames (const std::vector<Player*>& newPlayer) {
   Game::changeNames (newPlayer);

   for (unsigned int i (0); i < NUM_PLAYERS; ++i)
      players[i].name.set_text (actPlayers[i]->getName ());
}

//----------------------------------------------------------------------------
/// Changes the names of the playing people
/// \param newPlayer: Array holding the new player
/// \param pile: ID of the pile to return
//----------------------------------------------------------------------------
ICardPile& Rovhult::getPileOfPlayer (unsigned int player, unsigned int pile) {
   Check1 (player < NUM_PLAYERS);
   Check1 (pile < 8);
   TRACE8 ("Rovhult::getPileOfPlayer (unsigned int, unsigned int) - Player "
           << player << "; Pile " << pile);

   if (pile >= 5) {
      ICardPile& pile (players[player].reserve[pile - 5]);
      Check3 (pile.size ());
      if (cardValid (pile.getTopCard ().number (), true))
         return pile;
      else {
         played.append (pile.removeTopCard ());
         return played;
      }
   }

   return (pile
           ? ((pile == 4)
              ? static_cast<ICardPile&> (played)
              : static_cast<ICardPile&> (players[player].reserve[pile - 1]))
           : static_cast<ICardPile&> (players[player].hand));
}

//----------------------------------------------------------------------------
/// Handles the messages the server might send for the Rovhult cardgame
/// \param player: ID of player sending the message
/// \param message: Message received from the server
/// \returns bool: True, if message has completey processed
/// \throw std::string: In case of an error an describing text
//----------------------------------------------------------------------------
bool Rovhult::handleMessage (unsigned int player, const std::string& message) throw (std::string) {
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
            Check3 (player ? (lPlayer == player) : true);
            if (!player)
               lPlayer = static_cast<unsigned long> (lPlayer);

            register unsigned int save (lPlayer);
            lPlayer = (lPlayer - posServer) & 0x3;
            
            // Don't exchange already exchanged cards
            if (save != posServer) {
               // Remove the cards in the hand and the top of the table piles
               ICardPile pile;
               for (unsigned int i (0); i < 3; ++i) {
                  pile.push_back (&players[lPlayer].hand.removeTopCard ());
                  pile.push_back (&players[lPlayer].reserve[i].removeTopCard ());
               }

               command = cards;
               unsigned long card (0);
               unsigned int target (0);

               // Target piles
               ICardPile* piles[] =
                   { &players[lPlayer].hand, &players[lPlayer].hand,
                     &players[lPlayer].hand, &players[lPlayer].reserve[0],
                     &players[lPlayer].reserve[1], &players[lPlayer].reserve[2] };

               while (command.getNextNode (' ').size ()) {
                  if (stringToNumber (card, command.getActNode ().c_str ()))
                     break;

                  TRACE8 ("Rovhult::handleMessage (unsigned int, const std::string&) - "
                          << lPlayer << ": " << card);
                  card = pile.find (static_cast<unsigned int> (card));
                  Check3 (card < pile.size ());
                  if (card != -1U) {
                     CardWidget& movedCard (*pile[card]);
                     if (target > 2)
                        movedCard.showFace ();
                     piles[target++]->setTopCard (movedCard);
                     pile.erase (pile.begin () + card);
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
   return Game::handleMessage (player, message);
}

//----------------------------------------------------------------------------
/// Executes the remote move locally
/// \param pile: Pile to move to/from
/// \param target: ID of target as send by the partner
//----------------------------------------------------------------------------
bool Rovhult::executeRemoteMove (ICardPile& pile, unsigned int target) {
   if (target) {
      Check3 (gameStatus () == PLAYING);
      TRACE7 ("Rovhult::executeRemoteMove (ICardPile&, unsigned int) - Target " << target);

      if (target >= 4) {
         if (&pile == &played) {
            setNextPlayer (movePlayedCardsToLooser (currentPlayer ()));
            pos1Play = pos2Play = -1U;
            return false;
         }
         target -= 4;
      }

      Check3 (!players[currentPlayer ()].hand.size ());
      setNextPlayer (doPileSelected (currentPlayer (), target - 1));
      pos1Play = pos2Play = -1U;
      return false;
   }
   else
      return Game::executeRemoteMove (pile, target);
}
