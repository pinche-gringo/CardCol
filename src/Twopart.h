#ifndef TWOPART_H
#define TOWPART_H

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

#include <XApplication.h>


// Class to handle the -cardgame
class Twopart : public XApplication {
 public:
   // Manager functions
   Twopart ();
   ~Twopart ();

 private:
   // IDs for menus
   enum { NEW, EXIT, DEBUG, ABOUT };

   // Protected manager functions
   Twopart (const Twopart&);
   const Twopart& operator= (const Twopart&);

   // Event-handling
   virtual void command (int menu);
   void handSelected (unsigned int player, unsigned int iCard);

   // Helper functions
   void movePlayedCardsToPlayer (unsigned int nrLooser);
   int  nextAvailablePlayer (unsigned int actPlayer) const;
   void enablePlayer (unsigned int player);
   void disableLastPlayer ();
   void cleanTable ();
   void dealCards ();
   void fillStaple ();

   unsigned int pos2Player (unsigned int pos) const;

   int makeComputerMove ();
   void makeComputerMoves () {
      TRACE9 ("Twopart::makeComputerMoves () - *** Start timer ***");
      Gtk::Main::timeout.connect (slot (this, &Twopart::makeComputerMove), 100); }

   unsigned int endRound ();

   bool cardValid (CardWidget::NUMBERS nr);
   void executeMove (unsigned int player, CardWidget::NUMBERS nr);

   void loadCards ();

   static XApplication::MenuEntry Twopart::menuItems[];

   static const unsigned int NUM_PLAYERS = 4;              // Number of players

   unsigned int bfPlayers;           // Array indicating players still in round
   unsigned int actPlayer;          // Player who is in turn (needed for timer)
   unsigned int startPlayer;  // Player who started round (needed for endRound)

   // Columns and rows for the cards of the players
   static const unsigned int COLS_PLAYER[NUM_PLAYERS];
   static const unsigned int ROWS_PLAYER[NUM_PLAYERS];

   static const char* xpmAuthor[];
   static const char* xpmTwopart[];

   Gtk::Statusbar status;
   Gtk::Table     tblTable;

   CardImages cardFaces;
   CardSet cards;

   CardHInfoPile played;
   CardVInfoPile staple;                                     // Cards on staple
   struct {
      CardHPile hand;                         // For players: Cards in the hand
      CardHPile won;                            // Reserve-cards (for end-game)
   } players[NUM_PLAYERS];

   vector<Connection> activeCards;
   Connection pileTop;

   typedef OThread<Twopart> THRDAPPL;
   THRDAPPL* pThread;

   Widget* pMenuNew;

   static const unsigned int USED_CARDS = 52;

   static const unsigned int WIDTH = 760;
   static const unsigned int HEIGHT = 680;
};

#endif
