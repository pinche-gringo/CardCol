//$Id$

//PROJECT     : Cardgames
//SUBSYSTEM   : Rovhult
//REFERENCES  :
//TODO        :
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
//Parameters: parent: Parent widget to display the game in
//            statusbar: Status bar widget to display information about the game
//            cardset: Cardset to use
//            names: Vector of player-names
/*--------------------------------------------------------------------------*/
Rovhult::Rovhult (Gtk::Box& parent, Gtk::Statusbar& statusbar,
                  CardSet& cardset, const vector<string>& names)
   : Game (parent, statusbar, cardset, names, 16, 20)
     , staple (ICardPile::VERY_COMPRESSED)
     , played (ICardPile::VERY_COMPRESSED, ICardPile::SHOWFACE) {
   staple.show ();
   attach (staple, 3, 4, 2, 7, 0, 0);

   Check3 (cards.numberOfCards ());
   unsigned int width (cards.getCard (0).getImageWidth ());
   unsigned int height (cards.getCard (0).getImageHeight ());

   // Show and attach card-piles
   changeNames (names);
   for (int i (0); i < NUM_PLAYERS; ++i) {
      for (int j (0); j < 3; ++j) {
         players[i].reserve[j].setStyle (ICardPile::QUITE_COMPRESSED);
         players[i].reserve[j].show ();
         attach (players[i].reserve[j], COLS_PLAYER[i] + (j << 1),
                 COLS_PLAYER[i] + 1 + (j << 1), ROWS_PLAYER[i],
                 ROWS_PLAYER[i] + 2, 0, 0, 0);

         TRACE9 ("Rovhult::Rovhult () - Set at: "
                 << COLS_PLAYER[i]  + (j << 1) << '/' << ROWS_PLAYER[i]);

         players[i].reserve[j].set_usize (width, height + 5);
      }

      players[i].name.show ();
      attach (players[i].name, COLS_PLAYER[i], COLS_PLAYER[i] + 5,
              ROWS_PLAYER[i] + (i ? 5 : 2),
              ROWS_PLAYER[i] + (i ? 6 : 3),
              GTK_EXPAND, GTK_EXPAND, 1);

      players[i].hand.set_usize (width * 3, height);

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

   played.set_usize (width, height);
   staple.set_usize (width, height + 50);

   attach (played, 7, 11, 5, 14, 0, 0, 1);
}

/*--------------------------------------------------------------------------*/
//Purpose   : Destructor
/*--------------------------------------------------------------------------*/
Rovhult::~Rovhult () {
   TRACE8 ("Rovhult::~Rovhult ()");
}


/*--------------------------------------------------------------------------*/
//Purpose   : Starts the game
/*--------------------------------------------------------------------------*/
void Rovhult::start () {
   Game::start ();

   setGameStatus (PREPLAYING);
   randomizeCardsToPile (staple);
   dealCards ();

   pos1Play = pos2Play = -1U;
}

/*--------------------------------------------------------------------------*/
//Purpose   : Callback after finishing card-exchange
/*--------------------------------------------------------------------------*/
void Rovhult::finishedExchange () {
   played.show ();

   exchangeAutoplayerCards ();
   sortReserve (0);


   // Remove drag´n´drop abilities and compress cards
   for (int i (0); i < NUM_PLAYERS; ++i)
      players[i].hand.sortByNumber ();

   unregisterDND ();

   players[0].hand.setStyle (ICardPile::COMPRESSED);
   enableHuman ();
   staple.getTopCard ().remove_accelerator (*(get_toplevel ()->get_accel_group ()),
                                            ' ', 0);
   pileTop.disconnect ();

   displayTurn (0);
   setGameStatus (PLAYING);
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

      sortReserve (i);
   }
}

/*--------------------------------------------------------------------------*/
//Purpose   : Sorts the cards on the reserve piles
//Parameters: player: Player whose cards should be sorted
/*--------------------------------------------------------------------------*/
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

/*--------------------------------------------------------------------------*/
//Purpose   : Enables the cards of the human player
//Parameters: player: Player to enable
/*--------------------------------------------------------------------------*/
int Rovhult::enableHuman () {
   disableHuman (); Check3 (activeCards.empty ());

   if (players[0].hand.numberOfCards ()) {
      TRACE2 ("Rovhult::enableHuman () - Has " << players[0].hand.numberOfCards ()
              << " card(s) in the hand");

      for (int i (players[0].hand.numberOfCards ()); i;)
         activeCards.push_back
            (players[0].hand.at (--i).clicked.connect_after
             (bind (slot (this, &Rovhult::handSelected), i)));
   }
   else {
      TRACE2 ("Rovhult::enableHuman () - Enable reserve of human");

      for (int i (0); i < 3; ++i)
         if (players[0].reserve[i].numberOfCards ()) {
            TRACE8 ("Rovhult::enableHuman () - Pile " << i << " has "
                    << players[0].reserve[i].numberOfCards () << " card(s)");
            activeCards.push_back
               (players[0].reserve[i].getTopCard ().clicked.connect_after
                (bind (slot (this, &Rovhult::pileSelected), i)));
         }
   }

   if (played.numberOfCards ()) {
      TRACE2 ("Rovhult::enablePlayer (unsigned int) - Enable last played card");
      activeCards.push_back (played.getTopCard ().clicked.connect_after
                             (slot (this, &Rovhult::takeCards)));
   }
   return Game::enableHuman ();
}

/*--------------------------------------------------------------------------*/
//Purpose   : Callback after clicking on a card on table
//Parameters: pile: Offset of selected pile
/*--------------------------------------------------------------------------*/
void Rovhult::pileSelected (unsigned int pile) {
   Check3 (pile < 3);
   TRACE1 ("Rovhult::pileSelected (unsinged int) - Position " << pile);

   ICardPile& actPile (players[0].reserve[pile]);
   CardWidget& card (actPile.getTopCard ());
   bool showsFace (card.showsFace ());

   // If played from bottom of pile (with invisible cards): Flip card first
   if (!showsFace)
      card.showFace ();

   TRACE1 ("Rovhult::pileSelected (unsinged int) - Card " << card);

   if (!cardValid (card.number ()))  { // If selected card is not valid: Return
      actPile.removeTopCard ();
      played.append (card);
      takeCards ();
      return;
   }

   // If face of card was visible: Just go on (as the user knows what he has
   // selected); if not: Wait a while to let the GUI update and continue then.
   if (showsFace)
      playFromPile (pile);
   else
      Gtk::Main::timeout.connect (bind (slot (this, &Rovhult::playFromPile),
                                        pile), 1000);
}

/*--------------------------------------------------------------------------*/
//Purpose   : Performs playing from a pile
//Parameters: pile: Offset of selected pile
//Returns   : int: Always 0 to stop the timer (if called from one, that's it)
/*--------------------------------------------------------------------------*/
int Rovhult::playFromPile (unsigned int pile) {
   Check1 (pile < 3);
   TRACE1 ("Rovhult::playFromPile (unsinged int) - Card at pos " << pile);

   setNextPlayer (doPileSelected (0, pile));
   makeNextMoves ();
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
   else
      played.append (card);

   while (pile) {
      actPile = &players[player].reserve[--pile];
      if (actPile->numberOfCards ()
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
//Parameters: iCard: Offset of card in hand
/*--------------------------------------------------------------------------*/
void Rovhult::handSelected (unsigned int pos) {
   TRACE3 ("Rovhult::handSelected (unsinged int) - Checking card at " << pos);
   Check3 (player < NUM_PLAYERS);
   Check3 (pos < players[0].hand.numberOfCards ());

   CardWidget& card (players[0].hand.at (pos));
   TRACE1 ("Rovhult::handSelected (unsinged int) - Card " << pos << " = " << card);

   if (!cardValid (card.number ()))
       return;

   playCardsFromHand (0, players[0].hand.findFirstEqual (pos), pos);

   setNextPlayer (executeMove (0, card.number ()));
   makeNextMoves ();
}

/*--------------------------------------------------------------------------*/
//Purpose   : Move card (and cards with equal number below) from player to
//            played staple. The cards are replaced, if the staple contains
//            cards
//Parameters: player: ID of player who played the last card
//            start: Offset of first card in hand to play
//            end: Offset of last card in hand to play
//Returns   : CardWidget::NUMBERS: Number of played card
/*--------------------------------------------------------------------------*/
CardWidget::NUMBERS Rovhult::playCardsFromHand (unsigned int player, unsigned int start,
                                                unsigned int end) {
   TRACE5 ("Rovhult::playCardsFromHand (unsigned int, unsigned int, unsigned int)"
           " - Player " << player << " from " << start << " to " << end);
   Check3 (end < players[player].hand.numberOfCards ());
   Check3 (start <= end);
   CardWidget::NUMBERS nr (players[player].hand.at (start).number ());

   do {
      CardWidget& movedCard (players[player].hand.remove (start));
      if (movedCard.number () == CardWidget::TEN)
         played.clear ();
      else
         played.append (movedCard);
   } while (start < end--);

   // If staple contains cards and no 10 was played (except if hand is empty):
   // Fill up cards til player has 3 (or one, in case of a ten)
   if ((nr != CardWidget::TEN) || (!players[player].hand.numberOfCards ()))
      fillUpPile (players[player].hand, (nr != CardWidget::TEN) ? 3 : 1);

   return nr;
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
           << player << "; Card " << nr);
   Check3 (player < NUM_PLAYERS);

   std::string stat;

   // If last 4 cards have the same number or ten was played: Don't increase
   // player (except of course, if actual player don't have anymore cards)
   if (!((nr == CardWidget::TEN) || clearPlayedIf4Equal ())
       || (player != nextAvailablePlayer ((player - 1) & 0x3))) {
      player = nextAvailablePlayer (player);

      if (nextAvailablePlayer (player) == -1) {
         status.pop (1);
         stat = _("%1 lost");
         stat.replace (stat.find ("%1"), 2, names[player]);
         status.push (1, stat);
         setGameStatus (STOPPED);
         return -1;
      }

      if (nr == CardWidget::EIGHT) {
         stat = _("Skipping %1; ");
         stat.replace (stat.find ("%1"), 2, names[player]);
         player = nextAvailablePlayer (player);
      }
   }
   else
      stat = _("Pile cleared; ");

   displayTurn (player, stat);
   return player;
}

/*--------------------------------------------------------------------------*/
//Purpose   : Callback after selection top card on played pile -> Moves all
//            its card to the passed player
//Parameters: player: ID of player picking up the cards
/*--------------------------------------------------------------------------*/
void Rovhult::takeCards () {
   TRACE2 ("Rovhult::takeCards ()");

   setNextPlayer (movePlayedCardsToLooser (0));
   makeNextMoves ();
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

   while ((pile.numberOfCards () < minCards) && staple.numberOfCards ())
      pile.insertSorted (staple.removeTopCard ());
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
//Returns   : unsigned int: Next player
/*--------------------------------------------------------------------------*/
unsigned int Rovhult::movePlayedCardsToLooser (unsigned int nrLooser) {
   TRACE8 ("Rovhult::movePlayedCardsToLooser () - Player " << nrLooser << " gets "
           << played.numberOfCards () << " cards");
   Check3 (nrLooser < NUM_PLAYERS);

   movePile (players[nrLooser].hand, played);
   players[nrLooser].hand.sortByNumber ();
   std::string stat (_("%1 can't continue -> Taking whole pile. "));
   stat.replace (stat.find ("%1"), 2, names[nrLooser]);
   displayTurn (nrLooser =  nextAvailablePlayer (nrLooser), stat);
   return nrLooser;
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
void Rovhult::clean () {
   TRACE8 ("Rovhult::clean () - Status: " << gameStatus ());

   if (gameStatus () == PREPLAYING)
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
   disableHuman ();
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
   Check3 (gameStatus () == PREPLAYING);

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
   Check3 (gameStatus () == PREPLAYING);

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
//Purpose   : Stops the drag´n´drop abilities of the cards of player 0
//Parameters: card: Card to unregister of dnd
/*--------------------------------------------------------------------------*/
void Rovhult::unregisterDND () const {
   TRACE9 ("Rovhult::unregisterDND () - Status: " << gameStatus ());
   Check3 (gameStatus () == PREPLAYING);

   for (int i (0); i < players[0].hand.numberOfCards (); ++i) {
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
                     "table (with drag and drop) - press space (or click on the"
                     " staple) if finished"));
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
//Purpose   : Shows the cards the user is about to play
//Parameters: player: Player in turn
//Returns   : unsigned int: New position to play
/*--------------------------------------------------------------------------*/
unsigned int Rovhult::showCards2Play (unsigned int player) {
   TRACE2 ("Rovhult::showCards2Play (unsigned int, unsigned int) - "
           "For player " << player);
   Check3 (player < NUM_PLAYERS);

   if (findCard2Play (player, pos1Play, pos2Play) >= 0)
      if (players[player].hand.numberOfCards ())
         flipCards2Play (players[player].hand, pos1Play, pos2Play);
      else
         players[player].reserve[pos2Play].getTopCard ().showFace ();
   return pos2Play;
}

/*--------------------------------------------------------------------------*/
//Purpose   : Finds an executes the turn of a (computer controled) player
//Returns   : int: The next player
/*--------------------------------------------------------------------------*/
int Rovhult::makeMove (unsigned int player) {
   TRACE2 ("Rovhult::makeMove (unsigned int) - Player " << player);

   if (pos2Play == -1U) {
      if (!playerCanContinue (player,
                              (played.numberOfCards ()
                               ? played.getTopCard ().number () : CardWidget::TWO)))
         player = movePlayedCardsToLooser (player);
      else
         showCards2Play (player);
   }
   else {
      TRACE2 ("Rovhult::makeMove (unsigned int) - play cards " << pos1Play
              << " to " << pos2Play);
      Check3 (pos1Play >= 0);
      Check3 (pos1Play <= pos2Play);

      if (players[player].hand.numberOfCards ()) {
         Check3 (pos2Play < players[player].hand.numberOfCards ());

         if (players[player].hand.at (pos1Play).number () == CardWidget::TEN) {
            Check3 (players[player].hand.at (pos2Play).number () == CardWidget::TEN);
            played.clear ();
         }

         player = executeMove (player, playCardsFromHand (player, pos1Play, pos2Play));
      }
      else {
         Check3 (pos1Play <= pos2Play); Check3 (pos2Play < 3);

         // If cards are visible
         if (players[player].reserve[pos1Play].numberOfCards () > 1)
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

/*--------------------------------------------------------------------------*/
//Purpose   : Checks if there are only special cards up to the passed position
//Parameters: pile: Pile to inspect
//            start: Lower position of cards to inspect
//            end: Upper position of cards to inspect
//Returns   : bool: True, if there are only special cards
/*--------------------------------------------------------------------------*/
bool Rovhult::existOnlySpecialCards (const ICardPile& pile, unsigned int start,
                                     unsigned int end) const {
   Check3 (start <= end);
   Check3 (end < pile.numberOfCards ());

   do {
      if (!isSpecialCard (pile.at (start).number ()))
         return false;
   } while (++start <= end);

   TRACE9 ("Rovhult::existOnlySpecialCards (const ICardPile&, unsigned int) - Yes");
   return true;
}

/*--------------------------------------------------------------------------*/
//Purpose   : Finds the next card to play (for a computer controlled player)
//Parameters: player: Player to inspect
//            start: Position of (first) card to play
//            end: Position of (last) card to play
//Returns   : int: Position of card to play; if it is negative, than the cards
//                 was played from an "hidden" staple (and should not be
//                 displayed)
/*--------------------------------------------------------------------------*/
int Rovhult::findCard2Play (unsigned int player, unsigned int& start,
                            unsigned int& end) const {
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
      // Special handling if cards of next player are know: Try to give him
      // the whole pile
      CardWidget::NUMBERS nextMin, nextMax;
      int hpPos (-1);
      start =
         (played.numberOfCards ()
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
              && cardValid (players[player].hand.at (hpPos).number (), true)))
         ? hpPos : players[player].hand.findFirstEqualOrBigger (cardMin);
   
      TRACE6 ("Rovhult::findCard2Play (unsigned int) - First matching card"
              " at pos " << start);
   
      // Check if no matching normal card is found or found card is bigger than
      // the played 7. If so, use special card instead
      // We know one card must match as "playerCanContinue" reported this player
      // as valid
      if ((start == -1U)
          || (played.numberOfCards ()
              && ((played.getTopCard ().number () == CardWidget::SEVEN)
                  && players[player].hand.at (start).number () > CardWidget::SEVEN))) {
         TRACE7 ("Rovhult::findCard2Play (unsigned int) -  Ordinary cards don't"
                 " match -> Searching for special card");

         if (players[player].hand.at (0).number () == CardWidget::TWO)
            start = 0;
         else {
            start = players[player].hand.findFirstEqualOrBigger (CardWidget::TEN);
            Check3 (start != -1);
            Check3 (players[player].hand.at (start).number () == CardWidget::TEN);
         }
         TRACE7 ("Rovhult::findCard2Play (unsigned int) -  Using special card "
                 << players[player].hand.at (start) << " at pos " << start);
      }
      else {
         Check3 ((cardMin == CardWidget::SEVEN)
                 ? (players[player].hand.at (start).number () <= CardWidget::SEVEN)
                 : (players[player].hand.at (start).number () >= cardMin));

         // If player would continue with a 6, but has also a 7, play that
         // card instead
         if (players[player].hand.at (start).number () == CardWidget::SIX) {
            if ((end = players[player].hand.find (CardWidget::SEVEN, start)) != -1) {
               TRACE9 ("Rovhult::findCard2Play (unsigned int) -  Exchanging "
                       << players[player].hand.at (start) << " with "
                       << players[player].hand.at (end));
               start = end;
            }
         }
         else
            // The search of CardWidget does not know (and shall not know anything)
            // about the special meaning of tens, so skip them by yourself,
            // but use a TWO (if available) in case a TEN is/are the last card(s)
            if ((end = skip (CardWidget::TEN, players[player].hand, start)) == -1U) {
               if (players[player].hand.at (0).number () == CardWidget::TWO)
                  start = 0;
            }
            else
               start = end;

         TRACE5 ("Rovhult::findCard2Play (unsigned int) -  Playing "
                 << players[player].hand.at (start) << " at pos " << start);
      }

      // Now find the last of equal cards; get rid of all of them if:
      // - they would complete 4
      // - there are are only special cards left
      // - it's not a special card which is
      //     * not the highest card
      //     * it's the first card
      //     * it's a not that high card (up to 9)
      unsigned int last (players[player].hand.findLastEqual (start));
      TRACE8 ("Rovhult::findCard2Play (unsigned int) -  Last equal at pos "
              << last << " (of " << players[player].hand.numberOfCards ()
              << " cards)");
      Check3 (last < players[player].hand.numberOfCards ());
      end = (((numberOfEqualTopCards () + last - start) == 3)
             || ((start
                  ? existOnlySpecialCards (players[player].hand, 0, start - 1) : 1)
                 && ((last < (players[player].hand.numberOfCards () - 1))
                     ? existOnlySpecialCards (players[player].hand, last + 1,
                                              players[player].hand.numberOfCards () - 1)
                     : 1))
             || ((!isSpecialCard (players[player].hand.at (start).number ()))
                 && ((last != (players[player].hand.numberOfCards () - 1))
                     || !start
                     || (players[player].hand.at (start).number () < CardWidget::TEN))))
         ? last : start;

      return start;
   }
   else {
      TRACE5 ("Rovhult::findCard2Play (unsigned int) -  Analyzing reserve");

      // Bitfield for lower cards: Bit 0: Cards visible; Bit 1: Normal cards
      int bfLowerCardsInfo (0);

      // Play first visible cards
      for (start = 0; start < 3; ++start) {
         const ICardPile* pPile (&players[player].reserve[start]);
         if (pPile->numberOfCards ()) {
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
                            && pPile->numberOfCards ()
                            && pPile->topCardShowsFace ()
                            && (pPile->getTopCard ().number ()
                                == card.number ()))
                        ++end;

                  if (!isSpecialCard (card.number ()))
                     bfLowerCardsInfo |= 0x2;

                  TRACE7 ("Rovhult::findCard2Play (unsigned int) -  Playing "
                          "visible card " << card << " at pos " << end);
                  return end;
               }
            }
         }
      }

      // No card visible: Play the first
      if (!bfLowerCardsInfo) {
         start = 0;
         while (!players[player].reserve[start].numberOfCards ()) {
            ++start;
            Check3 (start < 3);
         }

         CardWidget& card (players[player].reserve[end = start].getTopCard ());
         TRACE7 ("Rovhult::findCard2Play (unsigned int) -  Playing invisible "
                 "card " << card << " at pos " << start);
         return cardValid (card.number (), true) ? 0 : -1;
      }
      Check3 (0);
   }
}

/*--------------------------------------------------------------------------*/
//Purpose   : Retrieves the minimal and maximal card of the player
//Parameters: player: Player whose card to analyze
//            min: Returns the minimal card
//            max: Returns the maximal card
//Returns   : bool: true, if cardinfo is available
/*--------------------------------------------------------------------------*/
bool Rovhult::getPileLimits (unsigned int player, CardWidget::NUMBERS& min,
                             CardWidget::NUMBERS& max) const {
   if (players[player].hand.numberOfCards ())
      return false;

   TRACE3 ("Rovhult::getPileInfo (unsigned int, unsigned int&, unsigned int&)"
           " - Analyzing cards of player " << player);

   bool cardFound (false);
   for (unsigned int i (0); i < 3; ++i) {
      const ICardPile* pPile (&players[player].reserve[i]);
      if (pPile->numberOfCards ()) {
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


/*--------------------------------------------------------------------------*/
//Purpose   : Shows or hides the cards of the computer player
//Parameters: open: Flag if cards should be shown or hidden
/*--------------------------------------------------------------------------*/
void Rovhult::playOpen (bool open) {
   ICardPile::ShowOpt show (open ? ICardPile::SHOWFACE : ICardPile::SHOWBACK);

   for (int i (1); i < NUM_PLAYERS; ++i) {
      players[i].hand.setShowOption (show);
      players[i].hand.setStyle ((show == ICardPile::SHOWFACE)
                                ? ICardPile::COMPRESSED
                                : ICardPile::QUITE_COMPRESSED);
   }
}

/*--------------------------------------------------------------------------*/
//Purpose   : End the current game as soon as possible
/*--------------------------------------------------------------------------*/
void Rovhult::end (bool restart) {
   if (gameStatus () == PREPLAYING)
      unregisterDND ();

   Game::end (restart);
}


/*--------------------------------------------------------------------------*/
//Purpose   : Changes the names of the playing people
//Parameters: newNames: Array holding the new names of the players
/*--------------------------------------------------------------------------*/
void Rovhult::changeNames (const vector<string>& newNames) {
   Game::changeNames (newNames);

   for (int i (0); i < NUM_PLAYERS; ++i)
      players[i].name.set_text (names[i]);
}
