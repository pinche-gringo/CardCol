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


namespace YGP {
   class Tokenize;
}


class ScoreDlg;


// Class to handle the Hearts cardgame
class SgtMayor : public Game {
   friend class Settings;
   friend class CardgameAppl;
   friend class CardgameCollection;

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

   virtual bool handleMessage (unsigned int player, const std::string& message) throw (YGP::ParseError, YGP::CommError);

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
   bool unmarkExchanged (CardWidget* cardHuman);
   bool unmark (unsigned int card);

   //@Section Virtual methods
   virtual int makeMove (unsigned int player);
   virtual bool enableHuman ();

   //@Section Helper methods
   bool selectTrump ();
   void exchangeCards (unsigned int playerBad, unsigned int posBad, unsigned int playerGood);
   void exchangeCards (unsigned int playerBad, unsigned int posBad,
		       unsigned int playerGood, unsigned int posGood);
   void exchangeCards (unsigned int playerBad, unsigned int playerGood);
   void directExchange (unsigned int playerBad, unsigned int posBad,
			unsigned int playerGood, unsigned int posGood);
   void delayedExchange (unsigned int playerBad, unsigned int posBad,
			 unsigned int playerGood, unsigned int posGood);
   void doExchangeCards (unsigned int playerBad, unsigned int posBad,
			 unsigned int playerGood, unsigned int posGood);
   void showNeededTicks ();
   void showTrump (CardWidget::COLOURS);
   void doShowTrump (CardWidget::COLOURS);
   void makeExchange ();
   void displayExchangeStatus ();
   void startPlaying ();
   unsigned int playCard (unsigned int player, unsigned int card);
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
   bool isHighest (const CardWidget& card) const;
   unsigned int tryToGetTickWithTrump (const ICardPile& pile) const;

   virtual void addMenus (Glib::RefPtr<Gtk::UIManager> mgrUI);
   virtual void removeMenus (Glib::RefPtr<Gtk::UIManager> mgrUI);
   virtual void showWonCards (bool show = true, unsigned int style = -1U);

   static const unsigned int NUM_PLAYERS = 3;              // Number of players

   struct {
      CardHPile  hand;                        // For players: Cards in the hand
      CardHPile  won;                                              // Won ticks
      Gtk::Label name;
      Gtk::Label neededTicks;
   } players[NUM_PLAYERS];
   CardHPile played;
   CardWidget* pTrump;

   unsigned int bfColours;

   unsigned int startPlayer;
   std::bitset<13>playedCards[4];
   int diffTicks[NUM_PLAYERS];

   Gtk::UIManager::ui_merge_id idMrg;

   ScoreDlg* pScoreDlg;

   static const unsigned int COLS_PLAYER[NUM_PLAYERS];
   static const unsigned int ROWS_PLAYER[NUM_PLAYERS];

   static unsigned int ENDTRICKS;
};

#endif
