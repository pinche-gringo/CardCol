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
#include <PathSrch.h>

#include <CardSet.h>
#include <CardPile.h>
#include <CardImgs.h>
#include <DeckSelect.h>

#include <XApplication.h>


// Class to handle the Twopart-cardgame
class Twopart : public XApplication {
 public:
   // Manager functions
   Twopart ();
   ~Twopart ();

 private:
   // IDs for menus
   enum { NEW, END, EXIT, DEBUG, CHGDECKS, SAVESET, ABOUT };

   // Status of game
   enum { INITIALIZING, STOPPED, TOSTOP, PLAYING, PLAYING2 };
   int statGame;

   // Protected manager functions
   Twopart (const Twopart&);
   const Twopart& operator= (const Twopart&);

   // Event-handling
   virtual void command (int menu);
   void cardSelected (unsigned int player, unsigned int iCard);
   void playedSelected (unsigned int player);
   void changeDecks (ICarddeckSelectDlg::commands cmd);

   // Helper functions
   void executeMove (unsigned int player, unsigned int iCard);
   void movePlayedCardsToPlayer (unsigned int nrPlayer, unsigned int start = 0);
   bool moveSelectedCardToPlayed (unsigned int player, unsigned int iCard);
   int  enableActPlayer ();
   void enablePlayer (unsigned int player);
   void disableLastPlayer ();
   void pickUpPlayedPile (unsigned int player);
   int  findNextPlayer (unsigned int player);
   int  findNextPlayerWithCards (unsigned int player);
   void removePlayer (unsigned int player) { bfPlayers &= ~(1 << player); }
   void addPlayer (unsigned int player) { bfPlayers |= 1 << player; }
   unsigned int removePlayersWithoutCards ();
   void startGame ();
   void cleanTable ();
   void dealCards ();
   void fillStaple ();
   void userWants2End (unsigned int input);
   void analyzeLastPlayed (unsigned int startPos, unsigned int cards, int& max,
                           int& maxPos, int& maxEqual, int& maxEqualPos) const;

   unsigned int pos2Player (unsigned int pos) const;
   int findPos2Play (unsigned int player) const;
   unsigned int findSmallestCard (unsigned int player) const;
   unsigned int findEndOfSerie (unsigned int player, unsigned int start) const;


   int startPartTwoTimerFnc ();
   void startPartTwo (unsigned int player) {
      TRACE9 ("Twopart::startPartTwo () - *** Start timer ***");
      // Delay starting of part two, in case of human player; as re-enabling a
      // signal (button-callback) inside the signal handler wreaks quite a bit
      // of havoc
      if (actPlayer = player)
         startPartTwoTimerFnc ();
      else
         Gtk::Main::timeout.connect (slot (this, &Twopart::startPartTwoTimerFnc), 100); }

   int makeNextMove ();
   void makeNextMoves () {
      TRACE9 ("Twopart::makeComputerMoves () - *** Start timer ***");
      Check3 ((statGame == PLAYING) || (statGame == PLAYING2));
      Gtk::Main::timeout.connect (slot (this, actPlayer
                                        ? &Twopart::makeNextMove
                                        : &Twopart::enableActPlayer),
                                  actPlayer ? 700 : 50); }

   int endRound ();

   static char sortOrder[4];
   static bool compByColorAccTrumps (const CardWidget* a, const CardWidget* b);

   void loadCards ();
   void changeCards (void* opt);

   static XApplication::MenuEntry Twopart::menuItems[];

   static const unsigned int NUM_PLAYERS = 4;              // Number of players

   unsigned int bfPlayers;           // Array indicating players still in round
   unsigned int actPlayer;          // Player who is in turn (needed for timer)

   // Variables to store positions during playing
   unsigned int startPos[NUM_PLAYERS - 1]; // Offset of cards played by players
   unsigned int offPos;                             // Offset in startPos-array
   unsigned int startPlayer;  // Player who started round (needed for endRound)
   unsigned int bfOldPlayers;  // Array indicating players while starting round

   bool restart;
   int  pos2Play;

   // Columns and rows for the cards of the players
   static const unsigned int COLS_PLAYER[NUM_PLAYERS];
   static const unsigned int ROWS_PLAYER[NUM_PLAYERS];

   static const char* xpmAuthor[];
   static const char* xpmTwopart[];

   Gtk::Statusbar status;
   Gtk::Table     tblTable;

   CardImages cardFaces;
   CardSet cards;
   CardWidget* pTrump;

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
   Widget* pMenuEnd;

   const ICarddeckSelectDlg* dlgChgDecks;

   std::string pathDeck;
   std::string pathBack;

   static const std::string NAME_INIFILE;

   static const unsigned int USED_CARDS = 52;

   static const unsigned int WIDTH = 760;
   static const unsigned int HEIGHT = 690;
};

#endif
