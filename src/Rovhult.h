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

#include <gtk--/table.h>
#include <gtk--/button.h>
#include <gtk--/statusbar.h>

#include <Mutex.h>
#include <Thread.h>

#include <CardSet.h>
#include <CardPile.h>
#include <CardImgs.h>

#include <XApplication.h>


// Class to handle the Rovhult-cardgame
class RovhultAppl : public XApplication {
 public:
   // Manager functions
   RovhultAppl ();
   ~RovhultAppl ();

 private:
   // IDs for menus
   enum { NEW, EXIT, ABOUT };

   // Protected manager functions
   RovhultAppl (const RovhultAppl&);
   const RovhultAppl& operator= (const RovhultAppl&);

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
 
   // Event-handling
   virtual void command (int menu);
   void pileSelected (unsigned int player, unsigned int pile);
   void handSelected (unsigned int player, unsigned int iCard);
   void finishedExchange ();
   void takeCards (unsigned int player);

   void doPileSelected (void* playerPile);

   // Helper functions
   void movePlayedCardsToLooser (unsigned int nrLooser);
   int  nextAvailablePlayer (unsigned int actPlayer) const;
   void makeComputerMoves (void* player);
   void enablePlayer (unsigned int player);
   void disableLastPlayer ();
   void cleanTable ();
   void dealCards ();
   void fillStaple ();
   void playCardsFromHand (unsigned int player, unsigned int pos);
   void exchangeAutoplayerCards ();

   void waitForThread ();

   bool clearPlayedIf4Equal ();
   void fillUpPile (ICardPile& pile, unsigned int minCards);

   bool playerCanContinue (unsigned int player, CardWidget::NUMBERS card) const;
   bool playerHandCanContinue (const ICardPile& pile, CardWidget::NUMBERS card) const;
   
   int makeTurn (unsigned int player);

   static int compareCards (const CardWidget& lhs, const CardWidget& rhs);

   bool cardValid (CardWidget::NUMBERS nr);
   int executeMove (unsigned int player, CardWidget::NUMBERS nr);

   void loadCards ();

   static XApplication::MenuEntry RovhultAppl::menuItems[];

   static const unsigned int NUM_PLAYERS = 4;              // Number of players

   // Columns and rows for the cards of the players
   static const unsigned int COLS_PLAYER[NUM_PLAYERS];
   static const unsigned int ROWS_PLAYER[NUM_PLAYERS];

   static const char* xpmAuthor[];
   static const char* xpmRovhult[];

   Gtk::Statusbar status;
   Gtk::Table     tblTable;

   Widget* pMenuNew;

   CardImages cardFaces;
   CardSet cards;

   CardHPile played;

   CardVPile staple;                                         // Cards on staple
   struct {
      CardHPile hand;                         // For players: Cards in the hand
      CardVPile reserve[3];                     // Reserve-cards (for end-game)
   } players[NUM_PLAYERS];

   vector<Connection> activeCards;
   Connection pileTop;

   typedef OThread<RovhultAppl> THRDAPPL;
   THRDAPPL* pThread;
   Mutex mutexThread;

   static const unsigned int USED_CARDS = 52;

   static const unsigned int WIDTH = 760;
   static const unsigned int HEIGHT = 735;


   static GtkTargetEntry dndTypeTable;
   static GtkTargetEntry dndTypeHand;
};

#endif
