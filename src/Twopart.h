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

#include <gtk--/label.h>

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
   Twopart (Gtk::Box& parent, Gtk::Statusbar& statusbar, CardSet& cardset,
            const vector<string>& names);
   ~Twopart ();

   virtual void start ();
   virtual void clean ();
   virtual void playOpen (bool open);
   virtual const char* name () { return "Twopart"; }

 private:
   // Status of game
   enum { PLAYING2 = Game::LAST};

   // Protected manager functions
   Twopart (const Twopart&);
   const Twopart& operator= (const Twopart&);

   // Event-handling
   void cardSelected (unsigned int iCard);
   void playedSelected ();

   // Helper functions
   int executeMove (unsigned int player, unsigned int start, unsigned int end);
   void movePlayedCardsToPlayer (unsigned int nrPlayer, unsigned int start = 0);
   bool moveSelectedCardToPlayed (unsigned int player, unsigned int start,
                                  unsigned int end);
   int enableHuman ();
   unsigned int pickUpPlayedPile (unsigned int player);
   int  findNextPlayer (unsigned int player) const;
   int  findNextPlayerWithCards (unsigned int player) const;
   void removePlayer (unsigned int player) { bfPlayers &= ~(1 << player); }
   void addPlayer (unsigned int player) { bfPlayers |= 1 << player; }
   static unsigned int playersInBitfield (unsigned int bfPlayers) {
      unsigned int cPlayers (0);
      for (unsigned int i (0); i < NUM_PLAYERS; ++i)
         if (bfPlayers & (1 << i))
            ++cPlayers;
      return cPlayers; }
   unsigned int removePlayersWithoutCards ();

   void dealCards ();
   void userWants2End (unsigned int input);
   void analyzeLastPlayed (unsigned int startPos, unsigned int cards, int& max,
                           int& maxPos, int& maxEqual, int& maxEqualPos,
                           int& trumps) const;

   unsigned int pos2Player (unsigned int pos) const;
   int findPos2Play (unsigned int player, unsigned int& start, unsigned int& end) const;
   unsigned int findSmallestCard (unsigned int player) const;
   unsigned int findEndOfSerie (unsigned int player, unsigned int start) const;
   unsigned int findStartOfSerie (unsigned int player, unsigned int start) const;
   int findBigger (const ICardPile& pile, CardWidget::NUMBERS nr) const;


   int startPartTwoTimerFnc (unsigned int player);
   void startPartTwo (unsigned int player);

   int makeMove (unsigned int player);
   int endRound (unsigned int player);

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
   unsigned int pos2Play;
   unsigned int pos1Play;

   // Columns and rows for the cards of the players
   static const unsigned int COLS_PLAYER[NUM_PLAYERS];
   static const unsigned int ROWS_PLAYER[NUM_PLAYERS];

   CardWidget* pTrump;

   CardHInfoPile played;
   CardVInfoPile staple;                                     // Cards on staple
   struct {
      CardHPile hand;                         // For players: Cards in the hand
      CardHPile won;                            // Reserve-cards (for end-game)
      Gtk::Label name;
   } players[NUM_PLAYERS];

   Widget* pMenuNew;
   Widget* pMenuEnd;
};

#endif
