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

#include <Thread.h>

#include <CardSet.h>
#include <CardPile.h>
#include <CardImgs.h>
#include <CardColl.h>

#include <XApplication.h>


// Class to handle the Rovhult-cardgame
class RovhultAppl : public XApplication {
 public:
   // Manager functions
   RovhultAppl ();
   ~RovhultAppl ();

   static void initI18n ();

 private:
   // IDs for menus
   enum { NEW, EXIT, ABOUT };

   void pileSelected (CardPile* parent);
   void handSelected (unsigned int player, unsigned int iCard);

   void getDropData (GdkDragContext *pContext, GtkSelectionData* pData,
                     guint info, guint32 time, unsigned int player, unsigned int cardPos);
   void cardDroppedOnTable (GdkDragContext* pContext, gint x, gint y,
                            GtkSelectionData* pData, guint info, guint32 time,
                            unsigned int playerPile);
   void cardDroppedOnHand (GdkDragContext* pContext, gint x, gint y,
                           GtkSelectionData* pData, guint info, guint32 time,
                           unsigned int playerCard);

   void finishedExchange ();

   // Protected manager functions
   RovhultAppl (const RovhultAppl&);
   const RovhultAppl& operator= (const RovhultAppl&);

   // Event-handling
   virtual void command (int menu);

   void cleanTable ();
   void dealCards ();
   void fillStaple ();
   void moveCardsToWinner (unsigned int nrWinner);

   void loadCards ();

   int getWinner () const;

   void registerHandDND (CardWidget& card, unsigned int player, unsigned int card);
   void registerTableDND (CardWidget& card, unsigned int player, unsigned int pile);
   void unregisterDND (CardWidget& card);
 
   static XApplication::MenuEntry RovhultAppl::menuItems[];

   static const unsigned int NUM_PLAYERS = 4;              // Number of players

   // Columns and rows for the cards of the players
   static const unsigned int COLS_PLAYER[NUM_PLAYERS];
   static const unsigned int ROWS_PLAYER[NUM_PLAYERS];

   static const char* xpmAuthor[];
   static const char* xpmRovhult[];

   Gtk::Statusbar status;
   Gtk::Table     tblTable;

   CardImages cardFaces;
   CardSet cards;

   CardCollection played;

   CardPile staple;                                          // Cards on staple
   CardCollection hands[NUM_PLAYERS];         // For players: Cards in the hand
   CardPile reserve[NUM_PLAYERS][3];            // Reserve-cards (for end-game)
   CardPile won[NUM_PLAYERS];             // Cards which player won during game

   Widget* pMenuNew;

   typedef OThread<RovhultAppl> THRDAPPL;
   THRDAPPL* pThread;

   static const unsigned int USED_CARDS = 52;

   static const unsigned int WIDTH = 760;
   static const unsigned int HEIGHT = 735;


   static GtkTargetEntry dndTypeTable;
   static GtkTargetEntry dndTypeHand;
};

#endif
