#ifndef HEARTS_H
#define HEARTS_H

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


#include <string>
#include <vector>

#include <gtkmm/label.h>

#include <CardSet.h>
#include <CardPile.h>

#include <Game.h>

#include <HeartsScore.h>


// Class to handle the Hearts cardgame
class Hearts : public Game {
 public:
   Hearts (Gtk::Box& parent, Gtk::Statusbar& statusbar, CardSet& cardset,
           const std::vector<std::string>& names);
   virtual ~Hearts ();

   virtual void start ();
   virtual void clean ();
   virtual void playOpen (bool open);
   virtual const char* name () { return "Hearts"; }
   virtual void changeNames (const std::vector<std::string>& newNames);

 private:
   enum Status { EXCHANGE = Game::LAST };

   // Protected manager functions
   Hearts (const Hearts& other);
   const Hearts& operator= (const Hearts& other);

   //@Section Event handling
   void cardSelected (unsigned int iCard);
   void takeCard (unsigned int iCard);

   //@Section Virtual methods
   virtual int makeMove (unsigned int player);
   virtual bool enableHuman ();

   //@Section Helper methods
   bool moveSelectedCardToPlayed (unsigned int player, unsigned int card);
   unsigned int  calcNextPlayer (unsigned int player);
   unsigned int  check4Winner ();
   void exchangeCards ();
   static void getPositionOfColors (ICardPile& pile, int result[4]);
   static unsigned int numberOfCards (const int aPositions[4], CardWidget::COLORS color);
   static unsigned int pointsOfPile (ICardPile& pile);

   //@Section Computer player
   unsigned int findPos2Play (unsigned int player);
   unsigned int findWorstCard (const ICardPile& pile, const int aPositions[4]);
   unsigned int findLowerCard (const ICardPile& pile, const int aPositions[4]);

   void startPlaying ();

   static const unsigned int NUM_PLAYERS = 4;              // Number of players

   bool playedSQ;               // Flag, if the queen of spades has been played
   unsigned int aPlayed[4];         // Array holding played cars for each color
   unsigned int pos2Play;                                   // Position to play
   unsigned int player2Exchange;  // ID of (next) player to exchange cards with

   struct {
      CardHPile  hand;                        // For players: Cards in the hand
      CardHPile  won;                           // Reserve-cards (for end-game)
      Gtk::Label name;
   } players[NUM_PLAYERS];
   CardHPile played;

   HeartsScoreDlg* pScoreDlg;

   static const unsigned int COLS_PLAYER[NUM_PLAYERS];
   static const unsigned int ROWS_PLAYER[NUM_PLAYERS];
};

#endif
