//$Id$

//PROJECT     : Cardgames
//SUBSYSTEM   : Rovhult
//REFERENCES  :
//TODO        : - MsgBox displaying looser?
//BUGS        :
//REVISION    : $Revision$
//AUTHOR      : Markus Schwab
//CREATED     : 28.3.2002
//COPYRIGHT   : Anticopyright (A) 2002

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

#include <gtk--/main.h>
#include <gtk--/statusbar.h>
#include <gtk--/accelgroup.h>

#include <time.h>
#include <stdlib.h>
#include <locale.h>
#include <unistd.h>

#include <glib.h>

#include <Check.h>
#include <Trace_.h>

#include <XAbout.h>
#include <XMessageBox.h>
#include <Cardset-config.h>

#include <CardWidget.h>
#include "Rovhult.h"


GtkTargetEntry Rovhult::dndTypeHand  = { "icon/card/hand", GTK_TARGET_SAME_APP, 0 };
GtkTargetEntry Rovhult::dndTypeTable = { "icon/card/table", GTK_TARGET_SAME_APP, 1 };


const unsigned int Rovhult::COLS_PLAYER[NUM_PLAYERS] = { 7, 13, 7, 1 };
const unsigned int Rovhult::ROWS_PLAYER[NUM_PLAYERS] = { 4, 7, 13, 7 };

using namespace Gtk;


/*--------------------------------------------------------------------------*/
//Purpose   : Defaultconstructor; all widget are created
/*--------------------------------------------------------------------------*/
Rovhult::Rovhult (Gtk::Box& parent, Gtk::Statusbar& statusbar, CardSet& cardset)
   : Game (parent, statusbar, cardset, 16, 20)
     , actPlayer (0), staple (ICardPile::VERY_COMPRESSED)
     , played (ICardPile::VERY_COMPRESSED) {
   staple.show ();
   attach (staple, 3, 4, 2, 5, 0, 0);

   // Show and attach card-piles
   for (int i (0); i < NUM_PLAYERS; ++i) {
      for (int j (0); j < 3; ++j) {
         players[i].reserve[j].setStyle (ICardPile::QUITE_COMPRESSED);
         players[i].reserve[j].show ();
         attach (players[i].reserve[j], COLS_PLAYER[i] + (j << 1),
                 COLS_PLAYER[i] + 1 + (j << 1), ROWS_PLAYER[i],
                 ROWS_PLAYER[i] + 2, 0, 0, 0);

         TRACE9 ("Rovhult::Rovhult () - Set at: "
                 << COLS_PLAYER[i]  + (j << 1) << '/' << ROWS_PLAYER[i]);
      }

      players[i].hand.setStyle (i ? ICardPile::QUITE_COMPRESSED : ICardPile::NORMAL);
      players[i].hand.setShowOption (i ? ICardPile::SHOWBACK : ICardPile::SHOWFACE);
      players[i].hand.show ();
      attach (players[i].hand, COLS_PLAYER[i],
              COLS_PLAYER[i] + 5,
              ROWS_PLAYER[i] + (i ? 3 : -3),
              ROWS_PLAYER[i] + (i ? 3 : -3) + 2
              , GTK_FILL|GTK_EXPAND, GTK_FILL|GTK_EXPAND, 0);
      TRACE9 ("Rovhult::Rovhult () - 2nd set at: "
              << COLS_PLAYER[i] + (i << 1) << '/'
              << ROWS_PLAYER[i] + (i ? 3 : -3));
   }

   attach (played, 7, 11, 5, 14, 0, 0, 1);
}

/*--------------------------------------------------------------------------*/
//Purpose   : Destructor
/*--------------------------------------------------------------------------*/
Rovhult::~Rovhult () {
   TRACE9 ("Rovhult::~Rovhult ()");
}


/*--------------------------------------------------------------------------*/
//Purpose   : Starts the game
/*--------------------------------------------------------------------------*/
void Rovhult::start () {
   Game::start ();

   cleanTable ();
   randomizeCardsToPile (staple);
   dealCards ();
   statGame = PREPLAYING;
}

/*--------------------------------------------------------------------------*/
//Purpose   : Callback after finishing card-exchange
/*--------------------------------------------------------------------------*/
void Rovhult::finishedExchange () {
   played.show ();

   status.pop (1);
   status.push (1, "");

   exchangeAutoplayerCards ();

   // Remove drag´n´drop abilities and compress cards
   for (int i (0); i < NUM_PLAYERS; ++i)
      players[i].hand.sortByNumber ();

   unregisterDND ();

   players[0].hand.setStyle (ICardPile::COMPRESSED);
   enablePlayer (0);
   staple.getTopCard ().remove_accelerator (*(get_toplevel ()->get_accel_group ()),
                                            ' ', 0);
   pileTop.disconnect ();

   statGame = PLAYING;
}

/*--------------------------------------------------------------------------*/
//Purpose   : Compares two cards according the rules of Rovhult
//Parameters: lhs, rhs: Cards to compare
//Returns   : int: >0, if number of lhs is smaller; 0 if equal or >0 if bigger
/*--------------------------------------------------------------------------*/
int Rovhult::compareCards (const CardWidget& lhs, const CardWidget& rhs) {
   unsigned int lhsValue ((lhs.number () == CardWidget::TWO) ? CardWidget::ACE + 1
                          : (lhs.number () == CardWidget::TEN) ? CardWidget::ACE + 2 :
                          lhs.number ());
   unsigned int rhsValue ((rhs.number () == CardWidget::TWO) ? CardWidget::ACE + 1
                          : (rhs.number () == CardWidget::TEN) ? CardWidget::ACE + 2 :
                          rhs.number ());
   TRACE3 ("Rovhult::compareCards (const CardWidget&, const CardWidget&) - "
           << lhsValue << " - " << rhsValue << " = " << (int)(lhsValue - rhsValue));

   return lhsValue - rhsValue;
}

/*--------------------------------------------------------------------------*/
//Purpose   : Exchanges the cards of the computer-players
//Parameters: player: Not really a void*, but actually the (next computer)player
/*--------------------------------------------------------------------------*/
void Rovhult::exchangeAutoplayerCards () {
   for (unsigned int i (1); i < NUM_PLAYERS; ++i) {
      for (unsigned int j (0); j < 3; ++j) {
         unsigned int posPile (0);
         unsigned int posHand (0);

         // Search for smallest card in pile and biggest in hand
         for (unsigned int k (1); k < 3; ++k) {
            if (compareCards (players[i].reserve[k].getTopCard (),
                              players[i].reserve[posPile].getTopCard ()) < 0)
               posPile = k;

            if (compareCards (players[i].hand.at (k), players[i].hand.at (posHand)) > 0)
               posHand = k;
         }

         // and exchange them, if hand is bigger than pile
         if (compareCards (players[i].reserve[posPile].getTopCard (),
                           players[i].hand.at (posHand)) < 0) {
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

      // Sort cards on piles
      for (int j (0); j < 2; ++j) {
         for (int k (j); k >= 0; --k) {
            if (compareCards (players[i].reserve[k + 1].getTopCard (),
                              players[i].reserve[k].getTopCard ()) < 0) {
               CardWidget& low (players[i].reserve[k + 1].removeTopCard ());
               CardWidget& high (players[i].reserve[k].removeTopCard ());

               TRACE3 ("Rovhult::exchangeAutoplayerCards () - exchanging card "
                       << low << " on pile " << (k + 1) << " with card " << high
                       << " on pile " << k);

               players[i].reserve[k + 1].setTopCard (high);
               players[i].reserve[k].setTopCard (low);
            }
         }
      }
   }
}

/*--------------------------------------------------------------------------*/
//Purpose   : Makes a move for a computer controlled player. If the next
//            player is human, enable its cards
/*--------------------------------------------------------------------------*/
int Rovhult::makeComputerMove () {
#ifdef CHECKLEVEL
   static bool inTurn (false);
   if (inTurn) {
      Check (!"in Turn");
      return true;
   }
   inTurn = true;
#endif

   if (statGame == TOSTOP) {
      TRACE8 ("Rovhult::makeComputerMove () - End game ");
      statGame = STOPPED;
      if (restart) // TODO: Let parent change
         start ();
      return 0;
   }

   TRACE2 ("Rovhult::makeComputerMove () - Start with player "
           << actPlayer);

   actPlayer = makeTurn (actPlayer);

   TRACE2 ("Rovhult::makeComputerMove () - Next player: "
           << actPlayer);

   if (!actPlayer) {
      statGame = PLAYING;
      enablePlayer (0);
   }

   // Continue with computer-moves (means: let timer enabled), if computer
   // controlled players are on turn
#ifdef CHECKLEVEL
   inTurn = false;
#endif
   return actPlayer > 0;
}

/*--------------------------------------------------------------------------*/
//Purpose   : Enables the cards of the passed player
//Parameters: player: Player to enable
/*--------------------------------------------------------------------------*/
void Rovhult::enablePlayer (unsigned int player) {
   disableLastPlayer (); Check3 (activeCards.empty ());

   if (players[player].hand.numberOfCards ()) {
      TRACE2 ("Rovhult::enablePlayer (unsigned int) - Hand of player "
           << player << " has " << players[player].hand.numberOfCards () << " card(s)");

      for (int i (players[player].hand.numberOfCards ()); i;)
         activeCards.push_back
            (players[player].hand.at (--i).clicked.connect_after
             (bind (slot (this, &Rovhult::handSelected), player, i)));
   }
   else {
      TRACE2 ("Rovhult::enablePlayer (unsigned int) - Enable reserve of player "
              << player);

      for (int i (0); i < 3; ++i)
         if (players[player].reserve[i].numberOfCards ()) {
            TRACE8 ("Rovhult::enablePlayer (unsigned int) - Pile " << i << " has "
                    << players[player].reserve[i].numberOfCards () << " card(s)");
            activeCards.push_back
               (players[player].reserve[i].getTopCard ().clicked.connect_after
                (bind (slot (this, &Rovhult::pileSelected), player, i)));
         }
   }

   if (played.numberOfCards ()) {
      TRACE2 ("Rovhult::enablePlayer (unsigned int) - Enable last played card for "
              "player " << player);
      activeCards.push_back (played.getTopCard ().clicked.connect_after
                             (bind (slot (this, &Rovhult::takeCards),
                                    player)));
   }
}

/*--------------------------------------------------------------------------*/
//Purpose   : Callback after clicking on a card on table
//Parameters: player: ID of player
//            pile: Offset of selected pile
/*--------------------------------------------------------------------------*/
void Rovhult::pileSelected (unsigned int player, unsigned int pile) {
   Check3 (player < NUM_PLAYERS); Check3 (pile < 3);
   TRACE1 ("Rovhult::pileSelected (unsigned int, unsinged int) - Position "
           << pile << " of player " << player);

   ICardPile& actPile (players[player].reserve[pile]);
   CardWidget& card (actPile.getTopCard ());
   bool showsFace (card.showsFace ());

   // If played from bottom of pile (with invisible cards): Flip card first
   if (!showsFace)
      card.showFace ();

   TRACE1 ("Rovhult::pileSelected (unsigned int, unsinged int) - Card " << card);

   if (!cardValid (card.number ()))  { // If selected card is not valid: Return
      actPile.removeTopCard ();
      card.set_relief (GTK_RELIEF_NONE);
      played.append (card);
      if (actPlayer = executeMove (player, CardWidget::UNREACHABLE) > 0) {
         makeComputerMoves ();
      }
      return;
   }

   // If face of card was visible: Just go on (as the user knows what he has
   // selected); if not: Wait a while to let the GUI update and continue then.
   if (showsFace)
      playFromPile (player, pile);
   else
      Gtk::Main::timeout.connect (bind (slot (this, &Rovhult::playFromPile),
                                        player, pile), 1000);

   if (actPlayer > 0)
      disableLastPlayer ();
}

/*--------------------------------------------------------------------------*/
//Purpose   : Performs playing from a pile
//Parameters: player: ID of player
//            pile: Offset of selected pile
//Returns   : int: Always 0 to stop the timer (if called from one, that's it)
/*--------------------------------------------------------------------------*/
int Rovhult::playFromPile (unsigned int player, unsigned int pile) {
   Check3 (player < NUM_PLAYERS); Check3 (pile < 3);
   TRACE1 ("Rovhult::playFromPile (unsigned int, unsinged int) - Card at pos "
           << pile << " for player " << player);

   actPlayer = doPileSelected (player, pile);

   if (actPlayer > 0)
      // Start a timer to perform the computer-moves
      makeComputerMoves ();
   else
      enablePlayer (0);

   return 0;
}

/*--------------------------------------------------------------------------*/
//Purpose   : Executes the move from a pile: Moves the cards and enables next
//Parameters: player: ID of player
//            pile: Offset of selected pile
//Returns   : int: player to continue
/*--------------------------------------------------------------------------*/
int Rovhult::doPileSelected (unsigned int player, unsigned int pile) {
   TRACE1 ("Rovhult::doPileSelected (unsigned int, unsinged int) - " 
           << player << '/' << pile);

   ICardPile* actPile (&players[player].reserve[pile]);

   Check3 (player < NUM_PLAYERS); Check3 (pile < 3);
   Check3 (actPile->numberOfCards ());

   // Move card (and visible cards with equal number below) from player to
   // played staple
   CardWidget& card (actPile->removeShownTopCard ());
   if (card.number () == CardWidget::TEN)
      played.clear ();
   else {
      card.set_relief (GTK_RELIEF_NONE);
      played.append (card);
   }

   while (pile) {
      actPile = &players[player].reserve[--pile];
      if (actPile->numberOfCards ()
          && actPile->topCardShowsFace ()
          && (actPile->getTopCard ().number () == card.number ())) {
         CardWidget& sameCard (actPile->removeShownTopCard ());
         if (sameCard.number () != CardWidget::TEN) {
            sameCard.set_relief (GTK_RELIEF_NONE);
            played.append (sameCard);
         }
      }
   }

   return executeMove (player, card.number ());
}


/*--------------------------------------------------------------------------*/
//Purpose   : Check if played card is valid (equal or bigger)
//            The following cards have special meaning:
//              - 2: Can be played always
//              - 7: The next card must be equal or *smaller*
//              - 8: Skips the next player
//              -10: Clears the staple; the same player can continue with cards in hand
//Parameters: nr: Card to check
//            silent: Flag, if error should be displayed
//Returns   : bool: True, if card can be played
/*--------------------------------------------------------------------------*/
bool Rovhult::cardValid (CardWidget::NUMBERS nr, bool silent) const {
   switch (nr) {
   case CardWidget::TEN:
   case CardWidget::TWO:
      break;

   default:
      std::string error;
      if (played.numberOfCards ()) {
         CardWidget& lastPlayed (played.getTopCard ());

         if (lastPlayed.number () == CardWidget::SEVEN) {
            if (nr > CardWidget::SEVEN)
               error = _("After a 7, the played card must be equal or smaller!");
         }
         else
            if (nr < lastPlayed.number ())
               error = _("Played card must be equal or bigger!");

         if (error.size ()) {
            if (!silent)
               XMessageBox::Show (error, _("Invalid move"),
                                  XMessageBox::ERROR | XMessageBox::OK);
            return false;
         }
      }
   } // end-switch

   return true;
}

/*--------------------------------------------------------------------------*/
//Purpose   : Callback after clicking on a card in hand
//Parameters: player: ID of player
//            iCard: Offset of card in hand
/*--------------------------------------------------------------------------*/
void Rovhult::handSelected (unsigned int player, unsigned int pos) {
   TRACE3 ("Rovhult::handSelected (unsigned int, unsinged int) - Checking player "
           << player << "; Card at " << pos);
   Check3 (player < NUM_PLAYERS);
   Check3 (pos <= players[player].hand.numberOfCards ());

   CardWidget& card (players[player].hand.at (pos));
   TRACE1 ("Rovhult::handSelected (unsigned int, unsinged int) - Card " << pos
           << " = " << card);

   if (!cardValid (card.number ()))
       return;

   playCardsFromHand (player, pos);

   actPlayer = executeMove (player, card.number ());

   if (actPlayer > 0) {
      // Start a timer to perform the computer-moves
      makeComputerMoves ();
   }
   else
      if (!actPlayer)
         enablePlayer (0);
}

/*--------------------------------------------------------------------------*/
//Purpose   : Move card (and cards with equal number below) from player to
//            played staple. The cards are replaced, if the staple contains
//            cards
//Parameters: player: ID of player who played the last card
//            pos: Offset of card in hand
//Returns   : CardWidget::NUMBERS: Number of played card
/*--------------------------------------------------------------------------*/
CardWidget::NUMBERS Rovhult::playCardsFromHand (unsigned int player, unsigned int pos) {
   CardWidget& card (players[player].hand.at (pos));

   do {
      CardWidget& movedCard (players[player].hand.remove (pos, true));
      if (movedCard.number () == CardWidget::TEN)
         played.clear ();
      else {
         movedCard.set_relief (GTK_RELIEF_NONE);
         played.append (movedCard);
      }
   } while (pos-- && (players[player].hand.at (pos).number () == card.number ()));

   // If staple contains cards and no 10 was played (except if hand is empty):
   // Fill up cards til player has 3 (or one, in case of a ten)
   if ((card.number () != CardWidget::TEN) || (!players[player].hand.numberOfCards ()))
      fillUpPile (players[player].hand, card.number () != CardWidget::TEN ? 3 : 1);

   return card.number ();
}

/*--------------------------------------------------------------------------*/
//Purpose   : Executes the move -> Check consequences for next in round and
//            calculate next player
//Parameters: player: ID of player who played the last card
//            nr: Played card
//Returns   : int: The next player
/*--------------------------------------------------------------------------*/
int Rovhult::executeMove (unsigned int player, CardWidget::NUMBERS nr) {
   TRACE3 ("Rovhult::executeMove (unsigned int, CardWidget::NUMBERS) - Player "
           << player);
   Check3 (player < NUM_PLAYERS);

   status.pop (1);
   std::string stat;

   // If last 4 cards have the same number or ten was played: Don't increase
   // player (except of course, if actual player don't have anymore cards)
   if (!((nr == CardWidget::TEN) || clearPlayedIf4Equal ())
       || (player != nextAvailablePlayer ((player - 1) & 0x3))) {
      if (nr != CardWidget::UNREACHABLE)
         player = nextAvailablePlayer (player);

      if (nextAvailablePlayer (player) == -1) {
         stat = _("Player %1 lost");
         stat.replace (stat.find ("%1"), 2, (char)(player + '0'));
         status.push (1, stat);
         statGame = STOPPED;
         // TODO: Deactivate menu: pMenuEnd->set_sensitive (false);
         return -1;
      }

      if (nr == CardWidget::EIGHT) {
         stat = _("Skipping player %1; ");
         stat.replace (stat.find ("%1"), 2, (char)(player + '0'));
         player = nextAvailablePlayer (player);
      }

      // Check if next player has fitting card
      if (!playerCanContinue (player, nr)) {
         if (nr != CardWidget::UNREACHABLE) {
            stat = stat + _("Player %1 can't continue -> Getting whole pile. ");
            stat.replace (stat.find ("%1"), 2, (char)(player + '0'));
         }

         movePlayedCardsToLooser (player);
         player = nextAvailablePlayer (player);
      }
   }
   else
      stat = _("Pile cleared; ");

   stat = stat + _("Turn of player %1");
   stat.replace (stat.find ("%1"), 2, (char)(player + '0'));
   status.push (1, stat);
   return player;
}

/*--------------------------------------------------------------------------*/
//Purpose   : Callback after selection top card on played pile -> Moves all
//            its card to the passed player
//Parameters: player: ID of player picking up the cards
/*--------------------------------------------------------------------------*/
void Rovhult::takeCards (unsigned int player) {
   TRACE2 ("Rovhult::takeCards (unsigned int) - " << player);
   actPlayer = executeMove (player, CardWidget::UNREACHABLE);

   // Start a timer to perform the computer-moves
   makeComputerMoves ();
}

/*--------------------------------------------------------------------------*/
//Purpose   : Checks if the passed pile has a card which can be played
//Parameters: player: ID of player to analyze
//            card: Last played card
/*--------------------------------------------------------------------------*/
bool Rovhult::playerCanContinue (unsigned int player, CardWidget::NUMBERS card) const {
   TRACE3 ("Rovhult::playerCanContinue (unsigned int, CardWidget::NUMBERS) const - "
           << player << "; Card: " << card);
   Check3 (player < NUM_PLAYERS);

   if (card == CardWidget::UNREACHABLE)
      return false;
 
   if (players[player].hand.numberOfCards ())
      return playerHandCanContinue (players[player].hand, card);

   // Check pile: Analyze only visible cards; if there are none, return true
   bool hasNoVisibleCards (true);
   for (int i (0); i < 3; ++i)
      if (players[player].reserve[i].numberOfCards () > 1) {
         TRACE5 ("Rovhult::playerCanContinue (unsigned int, CardWidget::NUMBERS) const"
                 " - Checking pile " << i << "; "
                 << players[player].reserve[i].numberOfCards () << " cards");
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

/*--------------------------------------------------------------------------*/
//Purpose   : Checks if the passed pile has a card which can be played
//Parameters: player: ID of player to analyze
//            card: Last played card
/*--------------------------------------------------------------------------*/
bool Rovhult::playerHandCanContinue (const ICardPile& pile, CardWidget::NUMBERS card) const {
   TRACE3 ("Rovhult::playerHandCanContinue (const ICardPile&, CardWidget::NUMBERS) const"
           << " - Card " << card << " in " << pile.numberOfCards () << " cards");
   Check3 (pile.numberOfCards ());

   // Check if first/last is smaller/bigger then passed one
   if (card == CardWidget::SEVEN) {
      if (pile.at (0).number () <= card)
         return true;
   }
   else {
      if (pile.at (pile.numberOfCards () - 1).number () >= card)
         return true;
   }

   TRACE3 ("Rovhult::playerHandCanContinue (const ICardPile&, CardWidget::NUMBERS"
           " - Special check");
   // Simple check failed -> Check for special card (2 or 10)
   if (pile.at (0).number () == CardWidget::TWO)
      return true;

   return pile.exists (CardWidget::TEN);
}

/*--------------------------------------------------------------------------*/
//Purpose   : Fills up the passed pile til it contains the specified number
//            of cards
//Parameters: pile: Pile to fill up
//            minCards: Minimal number of cards pile should hold
/*--------------------------------------------------------------------------*/
void Rovhult::fillUpPile (ICardPile& pile, unsigned int minCards) {
   TRACE3 ("Rovhult::fillUpPile (ICardPile&, unsinged int) - "
           << pile.numberOfCards () << " -> " << minCards);

   while ((pile.numberOfCards () < minCards) && staple.numberOfCards ()) {
      CardWidget& newCard (staple.removeTopCard ());
      newCard.showFace ();
      pile.insertSorted (newCard);

      TRACE8 ("Rovhult::fillUpPile (ICardPile&, unsinged int) - Appended card "
              << newCard);
   }
}

/*--------------------------------------------------------------------------*/
//Purpose   : Returns the number of equal cards from the played pile
//Returns   : unsigned int: Number of equal cards
/*--------------------------------------------------------------------------*/
unsigned int Rovhult::numberOfEqualTopCards () const {
   TRACE8 ("Rovhult::numberOfEqualTopCards () const");
   unsigned int nrCards (played.numberOfCards ());

   if (nrCards)
      --nrCards;
   else
      return 0;

   int i (1);
   CardWidget& card (played.getTopCard ());
   while (i <= nrCards) {
      TRACE9 ("Rovhult::numberOfEqualTopCards () const - Checking "
              << played.at (nrCards - i) << " with " << card);
      
      if (played.at (nrCards - i).number () != card.number ()) {
         TRACE8 ("Rovhult::numberOfEqualTopCards () const - found " << i);
         break;
      }
      ++i;
   }
   return i;
}

/*--------------------------------------------------------------------------*/
//Purpose   : Clears the played staple if the last 4 cards are equal
//Returns   : bool: True, if 4 equal cards found
/*--------------------------------------------------------------------------*/
bool Rovhult::clearPlayedIf4Equal () {
   TRACE8 ("Rovhult::clearPlayedIf4Equal ()");

   int cards (numberOfEqualTopCards ()); Check3 (cards <= 4);
   if (cards < 4)
      return false;

   TRACE7 ("Rovhult::clearPlayedIf4Equal () - found 4");
   played.clear ();
   return true;
}

/*--------------------------------------------------------------------------*/
//Purpose   : Method to move the cards of the actual round to the winner
//Parameters: nrLooser: Nr. of player getting all played cards
/*--------------------------------------------------------------------------*/
void Rovhult::movePlayedCardsToLooser (unsigned int nrLooser) {
   TRACE8 ("Rovhult::movePlayedCardsToLooser () - Player " << nrLooser << " gets "
           << played.numberOfCards () << " cards");
   Check3 (nrLooser < NUM_PLAYERS);

   while (played.numberOfCards ())
      players[nrLooser].hand.insertSorted (played.remove (0));
}

/*--------------------------------------------------------------------------*/
//Purpose   : Checks which player has still cards left
//Parameters: actPlayer: ID of actual player
//Returns   : int: ID of player or -1 (if none can continue)
/*--------------------------------------------------------------------------*/
int Rovhult::nextAvailablePlayer (unsigned int actPlayer) const {
   // We assume (without checking), that acutal player still has cards
   for (unsigned int i (1); i < NUM_PLAYERS; ++i) {
      unsigned int player ((actPlayer + i) & 0x3);

      if (players[player].hand.numberOfCards ())
         return player;

      for (int j (0); j < 3; ++j)
         if (players[player].reserve[j].numberOfCards ())
            return player;
   }
   return -1;
}

/*--------------------------------------------------------------------------*/
//Purpose   : Remove cards from everything which can hold them and unregister
//            any signals (DND)
/*--------------------------------------------------------------------------*/
void Rovhult::cleanTable () {
   if (statGame == PREPLAYING)
      unregisterDND ();

   staple.clear ();                                             // Clear staple
   for (int i (0); i < NUM_PLAYERS; ++i) {            // Clear cards of players
      for (int j (0); j < 3; ++j) {
         players[i].reserve[j].clear ();
      }

      players[i].hand.clear ();
   }
   played.clear ();

   players[0].hand.setStyle (ICardPile::NORMAL);
   disableLastPlayer ();
}

/*--------------------------------------------------------------------------*/
//Purpose   : Prepares the card for drag´n´drop (starting from the table,
//            ending on hand or ending on table, starting from hand)
//Parameters: card: Card to prepare for drag´n´drop
//            player: Number of player holding card
//            pile: Number of pile on reserve holding card
/*--------------------------------------------------------------------------*/
void Rovhult::registerTableDND (CardWidget& card, unsigned int player,
                                    unsigned int pile) {
   static Gdk_Colormap color (get_colormap ());
   static Gdk_Bitmap bitmap;

   // Card accepts drops from hand and drags from table
   card.drag_dest_set (GTK_DEST_DEFAULT_ALL, &dndTypeHand, 1, GDK_ACTION_COPY);
   card.drag_source_set (GDK_BUTTON1_MASK, &dndTypeTable, 1, GDK_ACTION_COPY);
   card.drag_source_set_icon (color, const_cast <Gdk_Pixmap&> (card.getImage ()), bitmap);
   card.drag_data_received.connect
      (bind (slot (this, &Rovhult::cardDroppedOnTable), (player << 16) + pile));
   card.drag_data_get.connect (bind (slot (this, &Rovhult::getDropData),
                                     player, pile));
}

/*--------------------------------------------------------------------------*/
//Purpose   : Prepares the card for drag´n´drop (starting from the hand
//            ending on table or ending on hand, starting from table)
//Parameters: card: Card to prepare for drag´n´drop
//            player: Number of player holding card
//            pile: Number of pile on reserve holding card
/*--------------------------------------------------------------------------*/
void Rovhult::registerHandDND (CardWidget& card, unsigned int player,
                                    unsigned int iCard) {
   static Gdk_Colormap color (get_colormap ());
   static Gdk_Bitmap bitmap;

   // Card accepts drops from table and drags from hand
   card.drag_dest_set (GTK_DEST_DEFAULT_ALL, &dndTypeTable, 1, GDK_ACTION_COPY);
   card.drag_source_set (GDK_BUTTON1_MASK, &dndTypeHand, 1, GDK_ACTION_COPY);
   card.drag_source_set_icon (color, const_cast <Gdk_Pixmap&> (card.getImage ()), bitmap);
   card.drag_data_received.connect
      (bind (slot (this, &Rovhult::cardDroppedOnHand), (player << 16) + iCard));
   card.drag_data_get.connect (bind (slot (this, &Rovhult::getDropData),
                                     player, iCard));
}

/*--------------------------------------------------------------------------*/
//Purpose   : Stops the drag´n´drop abilities of the passed card
//Parameters: card: Card to unregister of dnd
/*--------------------------------------------------------------------------*/
void Rovhult::unregisterDND (CardWidget& card) const {
   card.drag_dest_unset ();
   card.drag_source_unset ();
}

/*--------------------------------------------------------------------------*/
//Purpose   : Stops the drag´n´drop abilities of the passed card
//Parameters: card: Card to unregister of dnd
/*--------------------------------------------------------------------------*/
void Rovhult::unregisterDND () const {
   Check3 (players[0].hand.numberOfCards () == 3);
   Check3 (statGame == PREPLAYING);

   for (int i (0); i < 3; ++i) {
      CardWidget& card (players[0].hand.at (i));
      unregisterDND (card);
      Check3 (players[0].reserve[i].numberOfCards () == 2);
      unregisterDND (players[0].reserve[i].getTopCard ());
   }
}

/*--------------------------------------------------------------------------*/
//Purpose   : Deals the cards
/*--------------------------------------------------------------------------*/
void Rovhult::dealCards () {
   TRACE9 ("Rovhult::dealCards ()");
   Check3 (staple.numberOfCards () > 36);

   // Show cards on table: For all players put 6 cards on table (only the
   // (upper visible) and 3 (visible ones) in hand
   for (unsigned int i (0); i < NUM_PLAYERS; ++i)
      for (unsigned int j (0); j < 3; ++j) {
         for (unsigned int k (0); k < 2; ++k) {           // Set cards on table
            CardWidget& card (staple.removeTopCard ());
            players[i].reserve[j].setTopCard (card, k);
         } // end-for two cards pro pile (in reserve)

         // Put card into hand
         CardWidget& card (staple.removeTopCard ());
         card.showFace ();
         players[i].hand.insertSorted (card);
      }

   // Enable drag-and-drop for cards in hand (of human player)
   for (unsigned int i (0); i < players[i].hand.numberOfCards (); ++i) {
      registerHandDND (players[0].hand.at (i), 0, i);
      registerTableDND (players[0].reserve[i].getTopCard (), 0, i);
   }

   played.hide ();

   Check3 (staple.numberOfCards ());
   CardWidget& card (staple.getTopCard ());
   pileTop = card.clicked.connect (slot (this, &Rovhult::finishedExchange));
   card.add_accelerator ("clicked", *(get_toplevel ()->get_accel_group ()),
                         ' ', 0, GtkAccelFlags (0));

   status.pop (1);
   status.push (1, _("Exchange the cards in your hand with the one on the "
                     "table (with drag and drop) - press space (or click on staple) "
                     "if finished"));
}

/*--------------------------------------------------------------------------*/
//Purpose   : Callback after dropping a card onto (cards on) table
//Parameters: pContext: Context of the drag (contains things like source,
//                      target, action, ...)
//            pData: Describes the thing which was dropped
//            info: Describes the type of pData (should be 0)
//            time: Timestamp of the drag
//            player: Number of player
//            pile: Number of pile
//Requieres : pContext, pData not NULL; Expects info to be 0
/*--------------------------------------------------------------------------*/
void Rovhult::cardDroppedOnTable (GdkDragContext* pContext, gint, gint,
                                      GtkSelectionData* pData, guint info,
                                      guint32 time, unsigned int playerPile) {
   if (info == 1)
      return;

   Check3 (pContext); Check3 (pData);
   Check3 (!pContext->is_source);
   Check3 (pData->length == (sizeof (int) << 1));
   Check3 (pData->format == 8);

   unsigned int player (playerPile >> 16);
   unsigned int pile (playerPile & 0xffff);

   Check3 (player < NUM_PLAYERS);
   Check3 (pile < 3);
   
   unsigned int* pValues (reinterpret_cast <unsigned int*> (pData->data));
   Check3 (pValues);

   TRACE1 ("Rovhult::cardDroppedOnTable (...) - Data = "
           << *pValues << '/' << pValues[1] << " <-> " << player << '/' << pile);

   Gdk_DragContext gdc (pContext);

   // Check if player matches
   if (player != *pValues)
      drag_finish (gdc, false, false, time);
   else {
      CardWidget& cardTable (players[player].reserve[pile].removeTopCard ());
      CardWidget& cardHand (players[player].hand.remove (pValues[1]));

      TRACE1 ("Rovhult::cardDroppedOnTable (...) - Exchanging cards "
              << cardHand.id () << "<->" << cardTable.id ());

      // End old dnd
      drag_finish (gdc, true, false, time);

      unregisterDND (cardHand);
      unregisterDND (cardTable);

      players[player].reserve[pile].setTopCard (cardHand);
      players[player].hand.insert (cardTable, pValues[1]);

      registerHandDND (cardTable, player, pValues[1]);
      registerTableDND (cardHand, player, pile);
   }
}

/*--------------------------------------------------------------------------*/
//Purpose   : Callback after dropping a card onto onto hand
//Parameters: pContext: Context of the drag (contains things like source,
//                      target, action, ...)
//            pData: Describes the thing which was dropped
//            info: Describes the type of pData (should be 0)
//            time: Timestamp of the drag
//            player: Number of player
//            card: Number of card
//Requieres : pContext, pData not NULL; Expects info to be 0
/*--------------------------------------------------------------------------*/
void Rovhult::cardDroppedOnHand (GdkDragContext* pContext, gint, gint,
                                     GtkSelectionData* pData, guint info,
                                     guint32 time, unsigned int playerCard) {
   if (!info)
      return;

   Check3 (pContext); Check3 (pData);
   Check3 (!pContext->is_source);
   Check3 (pData->length == (sizeof (int) << 1));
   Check3 (pData->format == 8);

   unsigned int player (playerCard >> 16);
   unsigned int card (playerCard & 0xffff);

   Check3 (player < NUM_PLAYERS);
   Check3 (card < players[player].hand.numberOfCards ());

   unsigned int* pValues (reinterpret_cast <unsigned int*> (pData->data));
   Check3 (pValues);

   TRACE1 ("Rovhult::cardDroppedOnHand (...) - Data = "
           << *pValues << '/' << pValues[1] << " <-> " << player << '/' << card);

   Gdk_DragContext gdc (pContext);

   // Check if player matches
   if (player != *pValues)
      drag_finish (gdc, false, false, time);
   else {
      CardWidget& cardTable (players[player].reserve[pValues[1]].removeTopCard ());
      CardWidget& cardHand (players[player].hand.remove (card));

      TRACE1 ("Rovhult::cardDroppedOnHand (...) - Exchanging cards "
              << cardHand.id () << "<->" << cardTable.id ());

      // End old DND
      drag_finish (gdc, true, false, time);
      unregisterDND (cardHand);
      unregisterDND (cardTable);

      // Swap cards
      players[player].reserve[pValues[1]].setTopCard (cardHand);
      players[player].hand.insert (cardTable, card);

      // Adapt dnd-settigns
      registerHandDND (cardTable, player, card);
      registerTableDND (cardHand, player, pValues[1]);
   }
}

/*--------------------------------------------------------------------------*/
//Purpose   : Callback to query the data to drop
//Parameters: pContext: Context of the drag (contains things like source,
//                      target, action, ...)
//            pData: Describes the thing which was dropped
//            time: Timestamp of the drag
//            player: Number of player
//            cardPos: Position of card (either in hand or pile on table)
//Requieres : pContext, pData not NULL; Expects info to be 0
/*--------------------------------------------------------------------------*/
void Rovhult::getDropData (GdkDragContext* pContext, GtkSelectionData* pData,
                               guint info, guint32 time, unsigned int player,
                               unsigned int cardPos) {
   Check3 (pContext); Check3 (pData); Check3 (info < 2);
   Check3 (pContext->is_source);

   unsigned int data[] = { player, cardPos };
   gtk_selection_data_set (pData, pData->target, 8, reinterpret_cast <guchar*> (data),
                           sizeof (data));
}

/*--------------------------------------------------------------------------*/
//Purpose   : Retrieves the card at given position. If there are cards in the
//            hand, they are considered; else the cards in the reserve
//Parameters: player: Player in turn
//            pos: Position of (last) card to play
//Returns   : CardWidget*: Pointer to specified card or NULL if no card
/*--------------------------------------------------------------------------*/
CardWidget* Rovhult::cardAtPos (unsigned int player, unsigned int pos) const {
   TRACE2 ("Rovhult::cardAtPos (unsigned int, unsigned int) - "
           " For player " << player << " at position " << pos);
   Check3 ((players[player].hand.numberOfCards ())
           ? (pos < players[player].hand.numberOfCards ())
           : (pos < 3));

   return ((players[player].hand.numberOfCards ())
           ? &players[player].hand.at (pos)
           : (players[player].reserve[pos].numberOfCards ()
              ? &players[player].reserve[pos].getTopCard () : NULL));
}

/*--------------------------------------------------------------------------*/
//Purpose   : Flips the cards the user is about to play (and the ones with
//            similar numbers below)
//Parameters: player: Player in turn
//            pos: Position of (last) card to play
/*--------------------------------------------------------------------------*/
void Rovhult::flipCards2Play (unsigned int player, unsigned int pos) {
   TRACE2 ("Rovhult::flipCards2Pplay (unsigned int, unsigned int) - "
           " For player " << player << " at position " << pos);
   Check3 (player < NUM_PLAYERS);

   bool playFromHand (players[player].hand.numberOfCards ());
   Check3 (playFromHand
           ? (pos < players[player].hand.numberOfCards ())
           : (pos < 3));

   CardWidget* card (cardAtPos (player, pos));
   CardWidget::NUMBERS nr (card->number ());
   card->showFace ();
   do {
      if (playFromHand || card->showsFace ()) {
         card->showFace ();
         card->set_relief (GTK_RELIEF_NORMAL);
         if (card->width () < card->getImageWidth ())
            players[player].hand.resize (pos, ICardPile::COMPRESSED);
      }
   } while (pos
            && ((card = cardAtPos (player, --pos)))
            && (card->number () == nr));
}

/*--------------------------------------------------------------------------*/
//Purpose   : Finds an executes the turn of a (computer control.ed) player
//Returns   : int: The next player
/*--------------------------------------------------------------------------*/
int Rovhult::makeTurn (unsigned int player) {
   TRACE2 ("Rovhult::makeTurn (unsigned int) - Player " << player);
   static int pos2Play (-1);

   if (pos2Play == -1) {
      if ((pos2Play = findCard2Play (player)) < 0)
         pos2Play = ~pos2Play;
      else
         flipCards2Play (player, pos2Play);
   }
   else {
      TRACE2 ("Rovhult::makeTurn (unsigned int) - play card " << pos2Play);
      Check3 (pos2Play >= 0);

      unsigned int pos (pos2Play);
      pos2Play = -1;
      if (players[player].hand.numberOfCards ()) {
         Check3 (pos < players[player].hand.numberOfCards ());

         if (players[player].hand.at (pos).number () == CardWidget::TEN)
            played.clear ();

         CardWidget::NUMBERS nr (playCardsFromHand (player, pos));
         player = executeMove (player, nr);
      }
      else {
         Check3 (pos < 3);

         // If cards are visible
         if (players[player].reserve[pos].numberOfCards () > 1)
            return doPileSelected (player, pos);                // Execute move
         else {
            // If card is valid: Play it
            CardWidget& card (players[player].reserve[pos].getTopCard ());
            if (cardValid (card.number (), true))
               player = doPileSelected (player, pos);
            else {
               // Card is not valid: Take up pile
               players[player].reserve[pos].removeTopCard ();
               card.set_relief (GTK_RELIEF_NONE);
               played.append (card);
               player = executeMove (player, CardWidget::UNREACHABLE);
            }
         }
      }
   }
   return player;
}

/*--------------------------------------------------------------------------*/
//Purpose   : Checks if there are only special cards up to the passed position
//Parameters: player: Player to inspect
//            pos: Upper position of cards to inspect
//Returns   : bool: True, if there are only special cards
/*--------------------------------------------------------------------------*/
bool Rovhult::existOnlySpecialCards (unsigned int player, unsigned int pos) const {
   Check3 (player < NUM_PLAYERS);
   Check3 ((players[player].hand.numberOfCards ())
           ? (pos < players[player].hand.numberOfCards ())
           : (pos < 3));

   do {
       if (!isSpecialCard (cardAtPos (player, pos)->number ()))
          return false;
   } while (pos--);

   return true;
}

/*--------------------------------------------------------------------------*/
//Purpose   : Finds the next card to play (for a computer controlled player)
//Parameters: player: Player to inspect
//Returns   : int: Position of card to play; if it is negative, than the cards
//                 was played from an "hidden" staple (and should not be
//                 displayed)
/*--------------------------------------------------------------------------*/
int Rovhult::findCard2Play (unsigned int player) const {
   TRACE2 ("Rovhult::findCard2Play (unsigned int) - Player " << player);
   Check3 (player < NUM_PLAYERS);

   // Search for minimal card to play; this is either a card equal or bigger
   // or - if no previous card is played or the last card played was a 7 -
   // the smallest available
   CardWidget::NUMBERS cardMin (CardWidget::THREE);
   if (played.numberOfCards ()
       && (played.getTopCard ().number () != CardWidget::SEVEN)
       && (played.getTopCard ().number () != CardWidget::TWO))
      cardMin = played.getTopCard ().number ();

   TRACE5 ("Rovhult::findCard2Play (unsigned int) - Card to beat " << cardMin);

   // Check if to play from hand or to play from reserve
   unsigned int nrCards (players[player].hand.numberOfCards ());
   if (nrCards) {
      unsigned int pos (players[player].hand.findFirstEqualOrBigger (cardMin));
      TRACE6 ("Rovhult::findCard2Play (unsigned int) - First matching card"
              " at pos " << pos);
   
      // Check if no matching normal card is found or found card is bigger than
      // the played 7. If so, use special card instead
      // We know one card must match as "playerCanContinue" reported this player
      // as valid
      if ((pos == (unsigned int)-1)
          || (played.numberOfCards ()
              && ((played.getTopCard ().number () == CardWidget::SEVEN)
                  && players[player].hand.at (pos).number () > CardWidget::SEVEN))) {
         TRACE7 ("Rovhult::findCard2Play (unsigned int) -  Ordinary cards don't"
                 " match -> Searching for special card");

         if (players[player].hand.at (0).number () == CardWidget::TWO)
            pos = 0;
         else {
            pos = players[player].hand.findFirstEqualOrBigger (CardWidget::TEN);
            Check3 (pos != -1);
            Check3 (players[player].hand.at (pos).number () == CardWidget::TEN);
         }
         TRACE7 ("Rovhult::findCard2Play (unsigned int) -  Using special card "
                 << players[player].hand.at (pos) << " at pos " << pos);
      }
      else {
         Check3 ((cardMin == CardWidget::SEVEN)
                 ? (players[player].hand.at (pos).number () <= CardWidget::SEVEN)
                 : (players[player].hand.at (pos).number () >= cardMin));

         // The search of CardWidget does not know (and shall not know anything)
         // about the special meaning of the tens, so skip them by yourself
         if (players[player].hand.at (pos).number () == CardWidget::TEN) {
            TRACE7 ("Rovhult::findCard2Play (unsigned int) -  Skipping 10"
                    " at pos " << pos);

            unsigned int npos = players[player].hand.findLastEqual (pos);
            if ((npos + 1) < nrCards)     // Cards after 10 available? Yes: Use
               pos = npos + 1;
            else                                        // Else try to play a 2
               if (players[player].hand.at (0).number () == CardWidget::TWO)
                  pos = 0;
         }

         TRACE5 ("Rovhult::findCard2Play (unsigned int) -  Playing "
                 << players[player].hand.at (pos) << " at pos " << pos);
      }

      // Now find the last of equal cards; get rid of all of them if:
      // - they would complete 4
      // - there are are only special cards left
      // - it's not a special card which is
      //     * not the highest card
      //     * it´s the first card
      //     * it's a not that high card (up to 9)
      unsigned int npos (players[player].hand.findLastEqual (pos));
      Check3 (npos < players[player].hand.numberOfCards ());
      if (((numberOfEqualTopCards () + npos - pos) == 4)
          || (existOnlySpecialCards (player, players[player].hand.numberOfCards () - 1))
          || ((!isSpecialCard (players[player].hand.at (0).number ()))
              && ((npos != (players[player].hand.numberOfCards () - 1))
                  || (pos == 0)
                  || (players[player].hand.at (pos).number () < CardWidget::TEN))))
         pos = npos;

      return pos;
   }
   else {
      TRACE5 ("Rovhult::findCard2Play (unsigned int) -  Analyzing reserve");

      // Play first visible cards
      bool cardShowsFace (false);
      for (unsigned int i (0); i < 3; ++i) {
         const ICardPile* pPile (&players[player].reserve[i]);
         if (pPile->numberOfCards ()) {
            CardWidget& card (pPile->getTopCard ());
            if (card.showsFace ()) {
               // If card can be played: Search for last equal card
               if (cardValid (card.number (), true)) {
                  unsigned int lastEqual (i);

                  while ((lastEqual < 2)
                         && (pPile = &players[player].reserve[lastEqual + 1])
                         && pPile->numberOfCards ()
                         && pPile->topCardShowsFace ()
                         && (pPile->getTopCard ().number ()
                             == card.number ()))
                     ++lastEqual;

                  // Only play all cards, if it is not a special card
                  // or there are only special cards remaining
                  if (!isSpecialCard (card.number ())
                      // Tests for special cards: First line checks for only equal
                      // the second line works for mixed special cards
                      || (!cardShowsFace
                          && ((lastEqual == 2)
                              || (pPile->getTopCard ().number () == CardWidget::TEN))))
                     i = lastEqual;

                  TRACE7 ("Rovhult::findCard2Play (unsigned int) -  Playing "
                          "visible card " << card << " at pos " << i);
                  return i;
               }

               cardShowsFace = true;
            }
         }
      }

      // No card visible: Play the first
      if (!cardShowsFace) {
         unsigned int i (0);
         while (!players[player].reserve[i].numberOfCards ()) {
            ++i;
            Check3 (i < 3);
         }

         CardWidget& card (players[player].reserve[i].getTopCard ());
         TRACE7 ("Rovhult::findCard2Play (unsigned int) -  Playing invisible "
                 "card " << card << " at pos " << i);
         return cardValid (card.number (), true) ? i : ~i;
      }
      Check3 (0);
   }
}


/*--------------------------------------------------------------------------*/
//Purpose   : Activates the computer player
/*--------------------------------------------------------------------------*/
void Rovhult::makeComputerMoves () {
   TRACE9 ("Rovhult::makeComputerMoves () - *** Start timer ***");
   Gtk::Main::timeout.connect (slot (this, &Rovhult::makeComputerMove), 1000);
   disableLastPlayer ();
}
