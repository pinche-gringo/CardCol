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

#include <CardSet.h>
#include <CardPile.h>

#include <Game.h>


namespace Gtk {
   class Box;
};

using namespace Gtk;

// Class to handle the Twopart-cardgame
class Twopart : public Game {
 public:
   // Manager functions
   Twopart (Gtk::Box& parent, Gtk::Statusbar& statusbar, CardSet& cardset);
   ~Twopart ();

 private:
   // Status of game
   enum { PLAYING2 = Game::LAST};

   // Protected manager functions
   Twopart (const Twopart&);
   const Twopart& operator= (const Twopart&);

   // Event-handling
   void cardSelected (unsigned int player, unsigned int iCard);
   void playedSelected (unsigned int player);

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
   static unsigned int playersInBitfield (unsigned int bfPlayers) {
      unsigned int cPlayers (0);
      for (unsigned int i (0); i < NUM_PLAYERS; ++i)
         if (bfPlayers & (1 << i))
            ++cPlayers;
      return cPlayers; }
   unsigned int removePlayersWithoutCards ();

   virtual void start ();
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
   void startPartTwo (unsigned int player);

   int makeNextMove ();
   void makeNextMoves ();
   int endRound ();

   static char sortOrder[4];
   static bool compByColorAccTrumps (const CardWidget* a, const CardWidget* b);

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

   CardWidget* pTrump;

   CardHInfoPile played;
   CardVInfoPile staple;                                     // Cards on staple
   struct {
      CardHPile hand;                         // For players: Cards in the hand
      CardHPile won;                            // Reserve-cards (for end-game)
   } players[NUM_PLAYERS];

   Connection pileTop;

   Widget* pMenuNew;
   Widget* pMenuEnd;
};

#endif
