#ifndef ROVHULT_H
#define ROVHULT_H

//$Id$

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


#include <vector>

#include <CardSet.h>
#include <CardPile.h>

#include <Game.h>


namespace Gtk {
   class Box;
}

using namespace Gtk;

// Class to handle the Rovhult-cardgame
class Rovhult : public Game {
 public:
   // Manager functions
   Rovhult (Gtk::Box& parent, Gtk::Statusbar& statusbar, CardSet& cardset);
   ~Rovhult ();

 private:
   enum { PREPLAYING = Game::LAST };

   // Protected manager functions
   Rovhult (const Rovhult&);
   const Rovhult& operator= (const Rovhult&);

   // Drag and drop handling
   void getDropData (GdkDragContext *pContext, GtkSelectionData* pData,
                     guint info, guint32 time, unsigned int player, unsigned int cardPos);
   void cardDroppedOnTable (GdkDragContext* pContext, gint x, gint y,
                            GtkSelectionData* pData, guint info, guint32 time,
                            unsigned int playerPile);
   void cardDroppedOnHand (GdkDragContext* pContext, gint x, gint y,
                           GtkSelectionData* pData, guint info, guint32 time,
                           unsigned int playerCard);

   void registerHandDND (CardWidget& card, unsigned int player, unsigned int card);
   void registerTableDND (CardWidget& card, unsigned int player, unsigned int pile);
   void unregisterDND (CardWidget& card) const;
   void unregisterDND () const;
 
   // Event-handling
   void pileSelected (unsigned int player, unsigned int pile);
   void handSelected (unsigned int player, unsigned int iCard);
   void finishedExchange ();
   void takeCards (unsigned int player);

   int playFromPile (unsigned int player, unsigned int pile);
   int doPileSelected (unsigned int player, unsigned int pile);

   // Helper functions
   void movePlayedCardsToLooser (unsigned int nrLooser);
   int  nextAvailablePlayer (unsigned int actPlayer) const;
   int  makeComputerMove ();
   void makeComputerMoves ();
   void enablePlayer (unsigned int player);
   void cleanTable ();
   void dealCards ();
   void fillStaple ();
   CardWidget::NUMBERS playCardsFromHand (unsigned int player, unsigned int pos);
   void exchangeAutoplayerCards ();

   unsigned int numberOfEqualTopCards () const;
   bool clearPlayedIf4Equal ();
   void fillUpPile (ICardPile& pile, unsigned int minCards);

   bool playerCanContinue (unsigned int player, CardWidget::NUMBERS card) const;
   bool playerHandCanContinue (const ICardPile& pile, CardWidget::NUMBERS card) const;
   
   int makeTurn (unsigned int player);
   int findCard2Play (unsigned int player) const;
   void flipCards2Play (unsigned int player, unsigned int pos);

   static int compareCards (const CardWidget& lhs, const CardWidget& rhs);
   bool existOnlySpecialCards (unsigned int player, unsigned int pos) const;
   bool isSpecialCard (CardWidget::NUMBERS nr) const {
      return (nr == CardWidget::TEN) || (nr == CardWidget::TWO); }

   CardWidget* cardAtPos (unsigned int player, unsigned int pos) const;

   bool cardValid (CardWidget::NUMBERS nr, bool silent = false) const;
   int executeMove (unsigned int player, CardWidget::NUMBERS nr);

   void start ();

   static const unsigned int NUM_PLAYERS = 4;              // Number of players

   // Columns and rows for the cards of the players
   static const unsigned int COLS_PLAYER[NUM_PLAYERS];
   static const unsigned int ROWS_PLAYER[NUM_PLAYERS];

   // Variables for makeComputerMove
   int actPlayer;                                         // Player to continue
   bool restart;

   CardHInfoPile played;
   CardVInfoPile staple;                                     // Cards on staple
   struct {
      CardHPile hand;                     // For players: Cards in the hand
      CardVPile reserve[3];                     // Reserve-cards (for end-game)
   } players[NUM_PLAYERS];

   Connection pileTop;

   static GtkTargetEntry dndTypeTable;
   static GtkTargetEntry dndTypeHand;
};

#endif
