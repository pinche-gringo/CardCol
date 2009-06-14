#ifndef SGTMAYOR_H
#define SGTMAYOR_H

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


#include <vector>
#include <bitset>

#include <gtkmm/label.h>

#include <card/Set.h>
#include <card/Pile.h>

#include <card/Game.h>


namespace YGP {
   class Tokenize;
}

namespace Card {
   class ScoreDlg;
}


// Class to handle the Hearts cardgame
class SgtMayor : public Card::Game {
   friend class Settings;
   friend class CardgameAppl;
   friend class CardgameCollection;

 public:
   SgtMayor (Gtk::Box& parent, Gtk::Statusbar& statusbar, Card::Set& cardset,
             const std::vector<Card::Player*>& player, unsigned int posPlayer,
             YGP::Mutex& mxSerialize);
   virtual ~SgtMayor ();

   virtual void start ();
   virtual void clean ();
   virtual void playOpen (bool open);
   virtual const char* name () { return "Sgt. Mayor"; }
   virtual void changeNames (const std::vector<Card::Player*>& newPlayer);
   virtual void resizeCards ();

   virtual bool handleMessage (unsigned int player, const std::string& message) throw (YGP::ParseError, YGP::CommError);

 protected:
   virtual Card::IPile* getPileOfPlayer (unsigned int player, unsigned int pile);

 private:
   // Protected manager functions
   SgtMayor (const SgtMayor& other);
   const SgtMayor& operator= (const SgtMayor& other);

   //@Section Event handling
   void cardSelected (unsigned int iCard);
   void cardExchange (unsigned int iCard);
   void cardColourSelect (unsigned int iCard);

   //@Section Virtual methods
   virtual void makeMove (unsigned int player);
   virtual bool enableHuman ();

   //@Section Helper methods
   bool selectTrump ();
   void exchangeCards (unsigned int playerBad, unsigned int posBad, unsigned int playerGood);
   void exchangeCards (unsigned int playerBad, unsigned int posBad,
		       unsigned int playerGood, unsigned int posGood);
   void exchangeCards (unsigned int playerBad, unsigned int playerGood);
   void exchange (unsigned int playerBad, unsigned int posBad,
		  unsigned int playerGood, unsigned int posGood);
   void exchgBack (unsigned int playerBad, unsigned int playerGood, unsigned int posGood);
   void exchgNext (Card::HPile* pileGood, Card::HPile* pileBad);
   void doExchangeCards (unsigned int playerBad, unsigned int posBad,
			 unsigned int playerGood, unsigned int posGood);
   void showNeededTricks ();
   void showTrump (Card::Widget::COLOURS);
   void doShowTrump (Card::Widget::COLOURS);
   void makeExchange ();
   void displayExchangeStatus ();
   void startPlaying ();
   void playCardDelayed (unsigned int player);
   unsigned int playCard (unsigned int player);
   static unsigned int calcNextPlayer (unsigned int player) {
      return (++player >= NUM_PLAYERS) ? 0 : player;
   }
   unsigned int convertPlayer (unsigned int player) {
      return (((player + posServer) < NUM_PLAYERS) ? player : player + posServer);
   }
   static std::string formatNumber (int nr);
   static bool readCardInfo (YGP::Tokenize& src, unsigned long& card, unsigned long& player);

   //@Section Computer player
   unsigned int findPos2Play (unsigned int player);
   bool isHighest (const Card::Widget& card) const;
   unsigned int tryToGetTrickWithTrump (const Card::IPile& pile) const;

   virtual void addMenus (Glib::RefPtr<Gtk::UIManager> mgrUI);
   virtual void removeMenus (Glib::RefPtr<Gtk::UIManager> mgrUI);
   virtual void showWonCards (bool show = true, unsigned int style = -1U);

   static const unsigned int NUM_PLAYERS = 3;              // Number of players

   struct playerCards {
      Card::HPile  hand;                      // For players: Cards in the hand
      Card::HPile  won;                                           // Won tricks
      Gtk::Label name;
      Gtk::Label neededTricks;

      playerCards () : hand (), won (), name (), neededTricks () { }

    private:
      playerCards (const playerCards&);
      playerCards& operator= (const playerCards&);
   } players[NUM_PLAYERS];
   Card::HPile played;
   Card::Widget* pTrump;

   unsigned int bfColours;

   unsigned int startPlayer;
   std::bitset<13> playedCards[4];
   int diffTricks[NUM_PLAYERS];

   Gtk::UIManager::ui_merge_id idMrg;
   Glib::RefPtr<Gtk::Action> menuSort;
   Glib::RefPtr<Gtk::Action> menuSort2;
   Glib::RefPtr<Gtk::Action> menuShowScoreDlg;

   Card::ScoreDlg* pScoreDlg;

   static const unsigned int COLS_PLAYER[NUM_PLAYERS];
   static const unsigned int ROWS_PLAYER[NUM_PLAYERS];

   static unsigned int ENDTRICKS;
};

#endif
