#ifndef SGTMAYOR_H
#define SGTMAYOR_H

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
#include <bitset>

#include <gtkmm/label.h>

#include <CardSet.h>
#include <CardPile.h>

#include <Game.h>


// Class to handle the Hearts cardgame
class SgtMayor : public Game {
 public:
   SgtMayor (Gtk::Box& parent, Gtk::Statusbar& statusbar, CardSet& cardset,
             const std::vector<Player*>& player, unsigned int posPlayer,
             YGP::Mutex& mxSerialize);
   virtual ~SgtMayor ();

   virtual void start ();
   virtual void clean ();
   virtual void playOpen (bool open);
   virtual const char* name () { return "Sgt. Mayor"; }
   virtual void changeNames (const std::vector<Player*>& newPlayer);

   virtual bool handleMessage (unsigned int player, const std::string& message) throw (std::string);

 protected:
   virtual ICardPile* getPileOfPlayer (unsigned int player, unsigned int pile);

 private:
   // Protected manager functions
   SgtMayor (const SgtMayor& other);
   const SgtMayor& operator= (const SgtMayor& other);

   //@Section Event handling
   void cardSelected (unsigned int iCard);
   void cardExchange (unsigned int iCard);
   void cardColourSelect (unsigned int iCard);
   bool unmark (const CardWidget* card);

   //@Section Virtual methods
   virtual int makeMove (unsigned int player);
   virtual bool enableHuman ();

   //@Section Helper methods
   void exchangeCards (unsigned int playerBad, unsigned int posBad, unsigned int playerGood);
   void exchangeCards (unsigned int playerBad, unsigned int playerGood);
   void showTrump (CardWidget::COLOURS);
   void makeExchange ();
   void startPlaying ();
   unsigned int playCard (unsigned int player, unsigned int card);
   static unsigned int calcNextPlayer (unsigned int player) {
      return (++player >= NUM_PLAYERS) ? 0 : player;
   }

   //@Section Computer player
   unsigned int findPos2Play (unsigned int player);
   bool isHighest (const CardWidget& card) const;
   unsigned int tryToGetTickWithTrump (const ICardPile& pile) const;

   static const unsigned int NUM_PLAYERS = 3;              // Number of players

   struct {
      CardHPile  hand;                        // For players: Cards in the hand
      CardHPile  won;                                              // Won ticks
      Gtk::Label name;
      Gtk::Label neededTicks;
   } players[NUM_PLAYERS];
   CardHPile played;
   CardWidget* pExchange;
   CardWidget* pTrump;

   unsigned int startPlayer;
   std::bitset<52> playedCards;
   unsigned int playedColours[4];
   int diffTicks[NUM_PLAYERS];

   static const unsigned int COLS_PLAYER[NUM_PLAYERS];
   static const unsigned int ROWS_PLAYER[NUM_PLAYERS];
};

#endif
