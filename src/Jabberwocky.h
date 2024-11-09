#ifndef JABBERWOCKY_H
#define JABBERWOCKY_H

//$Id$

// This file is part of CardCol.
//
// CardCol is free software: you can redistribute it and/or modify
// it under the terms of the GNU General Public License as published by
// the Free Software Foundation, either version 3 of the License, or
// (at your option) any later version.
//
// CardCol is distributed in the hope that it will be useful,
// but WITHOUT ANY WARRANTY; without even the implied warranty of
// MERCHANTABILITY or FITNESS FOR A PARTICULAR PURPOSE.  See the
// GNU General Public License for more details.
//
// You should have received a copy of the GNU General Public License
// along with CardCol.  If not, see <http://www.gnu.org/licenses/>.


#include <bitset>
#include <vector>

#include <gtkmm/label.h>

#include <YGP/ANumeric.h>

#include <card/Set.h>
#include <card/Pile.h>

#include <card/Game.h>


namespace Gtk {
   class Button;
   class SpinButton;
}


/**Class to handle the Jabberwocky card game
 */
class Jabberwocky : public Card::Game {
 public:
   Jabberwocky (Gtk::Box& parent, Gtk::Statusbar& statusbar, Card::Set& cardset,
		const std::vector<Card::Player*>& player, unsigned int posPlayer,
		YGP::Mutex& mxSerialize);
   virtual ~Jabberwocky ();

   virtual void start ();
   virtual void clean ();
   virtual void playOpen (bool open);
   virtual const char* name () { return "Jabberwocky"; }
   virtual void changeNames (const std::vector<Card::Player*>& newPlayer);
   virtual void resizeCards ();

   virtual bool handleMessage (unsigned int player, const std::string& message);

 protected:
   virtual Card::IPile* getPileOfPlayer (unsigned int player, unsigned int pile);

 private:
   Jabberwocky ();
   Jabberwocky (const Jabberwocky& other);

   const Jabberwocky& operator= (const Jabberwocky& other);

   //@Section Virtual methods
   virtual void makeMove (unsigned int player);
   virtual bool enableHuman ();
   virtual void addMenus (Glib::RefPtr<Gtk::UIManager> mgrUI);
   virtual void removeMenus (Glib::RefPtr<Gtk::UIManager> mgrUI);

   //@Section helper methods
   static unsigned int getTricks (unsigned int round) { return (round < 7) ? (round + 3) : (15 - round); }
   void cardSelected (unsigned int pos);
   void makeBids (unsigned int start = 0);
   void startGame ();
   void placedBid (Gtk::SpinButton* value, Gtk::Button* commit, unsigned int start);
   void showBid (unsigned int player);
   unsigned int calcTricks (unsigned int player) const;

   void showCards2Play (unsigned int player);
   int playCard (unsigned int player);
   void finishMove ();
   void getPositionOfColours (const Card::IPile& pile, int result[4]);
   bool isHighest (const Card::Widget& card) const;
   bool isHighEnough (const Card::Widget& card) const;
   unsigned int findHigherCard (const Card::Widget& cardCmp, const Card::IPile& pile, unsigned int aPosColour) const;
   unsigned int findLowerCard (const Card::Widget& cardCmp, const Card::IPile& pile, int aPosColour) const;
   unsigned int findWorstCard (const Card::IPile& card, const int aPositions[4]) const;
   unsigned int check4Winner () const;
   unsigned int sumBids () const;
   void takeWonCards (unsigned int player);

   static char sortOrder[4];
   static bool compByColourAccTrumps (const Card::Widget* a, const Card::Widget* b);

   static const unsigned int NUM_PLAYERS = 4;              // Number of players

   struct playerCards {
      Card::HPile  hand;                      // For players: Cards in the hand
      Card::HPile  won;                                            // Won ticks
      Gtk::Label name;
      YGP::ANumeric bid;

      playerCards () : hand (), won (), name (), bid () { }

    private:
      playerCards (const playerCards&);
      playerCards& operator= (const playerCards&);
   } players[NUM_PLAYERS];
   Card::HPile played;
   Card::Widget* pTrump;

   unsigned int startPlayer;
   unsigned int turn;
   Gtk::UIManager::ui_merge_id idMrg;

   // Variables needed by computer player
   std::bitset<13> playedCards[4];
   bool outOfColour[NUM_PLAYERS][4];

   Card::ScoreDlg*   pScoreDlg;

   Glib::RefPtr<Gtk::Action> menuSort;
   Glib::RefPtr<Gtk::Action> menuSort2;
   Glib::RefPtr<Gtk::Action> menuShowScoreDlg;

   static const unsigned int COLS_PLAYER[NUM_PLAYERS];
   static const unsigned int ROWS_PLAYER[NUM_PLAYERS];
};

#endif
