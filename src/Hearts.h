#ifndef HEARTS_H
#define HEARTS_H

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

#include <gtkmm/label.h>

#include <card/Set.h>
#include <card/Pile.h>

#include <card/Game.h>


namespace Card {
   class ScoreDlg;
}


// Class to handle the Hearts cardgame
class Hearts: public Card::Game {
   friend class Settings;
   friend class CardgameAppl;
   friend class CardgameCollection;

 public:
   Hearts(Gtk::Box& parent, Gtk::Statusbar& statusbar, Card::Set& cardset,
          const std::vector<Card::Player*>& player, unsigned int posPlayer, YGP::Mutex& mxSerialize);
   virtual ~Hearts();

   virtual void start();
   virtual void clean();
   virtual void playOpen(bool open);
   virtual const char* name() { return "Hearts"; }
   virtual void changeNames(const std::vector<Card::Player*>& newPlayer);
   virtual void resizeCards();

   virtual bool handleMessage(unsigned int player, const std::string& message);

 protected:
   virtual Card::IPile* getPileOfPlayer(unsigned int player, unsigned int pile);

 private:
   enum Status { EXCHANGE=Game::LAST };

   static void getPositionOfColours(Card::IPile& pile, int result[4]);

   // Protected manager functions
   Hearts(const Hearts& other);
   const Hearts& operator=(const Hearts& other);

   //@Section Event handling
   void cardSelected(unsigned int iCard);
   void takeCard(unsigned int iCard);
   void cardTaken();

   //@Section Virtual methods
   virtual void makeMove(unsigned int player);
   virtual bool enableHuman();

   //@Section Helper methods
   bool moveSelectedCardToPlayed(unsigned int player, unsigned int card);
   unsigned int  calcNextPlayer(unsigned int player);
   unsigned int  check4Winner() const;
   void exchangeCards();
   void finishExchangeCards();
   void finishMove();
   void takeWonCards(unsigned int player);
   bool cardsExchanged(unsigned int cards);
   static unsigned int numberOfCards(const int aPositions[4], Card::Widget::COLOURS colour);
   static unsigned int pointsOfPile(const Card::IPile& pile);

   //@Section Computer player
   unsigned int findPos2Play(unsigned int player);
   unsigned int findWorstCard(const Card::IPile& pile, const int aPositions[4]) const;
   unsigned int findLowerCard(const Card::IPile& pile, const int aPositions[4]) const;

   void startPlaying();

   virtual void addMenus(Glib::RefPtr<Gtk::UIManager> mgrUI);
   virtual void removeMenus(Glib::RefPtr<Gtk::UIManager> mgrUI);

   static const unsigned int NUM_PLAYERS = 4;              // Number of players

   bool playedSQ;               // Flag, if the queen of spades has been played
   unsigned int aPlayed[4];        // Array holding played cars for each colour

   unsigned int player2Exchange;  // ID of (next) player to exchange cards with
   Card::IPile aExchange[NUM_PLAYERS];      // Cards the players are exchanging

   struct playerCards {
      Card::IPile*  hand;                     // For players: Cards in the hand
      Card::IPile*  won;                                           // Won cards
      Gtk::Label name;

      playerCards(): hand(NULL), won(NULL), name() { }

    private:
      playerCards(const playerCards&);
      playerCards& operator=(const playerCards&);
   } players[NUM_PLAYERS];
   Card::HPile played;

   Card::ScoreDlg* pScoreDlg;

   Gtk::UIManager::ui_merge_id idMrg;
   Glib::RefPtr<Gtk::Action> menuSort;
   Glib::RefPtr<Gtk::Action> menuSort2;
   Glib::RefPtr<Gtk::Action> menuShowScoreDlg;

   static const unsigned int COLS_PLAYER[NUM_PLAYERS];
   static const unsigned int ROWS_PLAYER[NUM_PLAYERS];

   static unsigned int ENDPOINTS;
};

#endif
