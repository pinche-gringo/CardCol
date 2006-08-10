#ifndef JABBERWOCKY_H
#define JABBERWOCKY_H

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

#include <gtkmm/label.h>

#include <CardSet.h>
#include <CardPile.h>

#include <Game.h>


namespace Gtk {
   class Button;
   class SpinButton;
}


/**Class to handle the Jabberwocky card game
 */
class Jabberwocky : public Game {
 public:
   Jabberwocky (Gtk::Box& parent, Gtk::Statusbar& statusbar, CardSet& cardset,
		const std::vector<Player*>& player, unsigned int posPlayer,
		YGP::Mutex& mxSerialize);
   virtual ~Jabberwocky ();

   virtual void start ();
   virtual void clean ();
   virtual void playOpen (bool open);
   virtual const char* name () { return "Sgt. Mayor"; }
   virtual void changeNames (const std::vector<Player*>& newPlayer);

 protected:
   virtual ICardPile* getPileOfPlayer (unsigned int player, unsigned int pile);

 private:
   Jabberwocky ();
   Jabberwocky (const Jabberwocky& other);

   const Jabberwocky& operator= (const Jabberwocky& other);

   //@Section Virtual methods
   virtual int makeMove (unsigned int player);
   virtual bool enableHuman ();
   virtual void addMenus (Glib::RefPtr<Gtk::UIManager> mgrUI);
   virtual void removeMenus (Glib::RefPtr<Gtk::UIManager> mgrUI);

   //@Section helper methods
   void cardSelected (unsigned int pos);
   void makeBets (unsigned int start = 0, unsigned int end = NUM_PLAYERS - 1);
   void startGame ();
   void placedBet (Gtk::SpinButton* value, Gtk::Button* commit,
		   unsigned int start, unsigned int end);
   void showBet (unsigned int player);

   static const unsigned int NUM_PLAYERS = 4;              // Number of players

   struct {
      CardHPile  hand;                        // For players: Cards in the hand
      CardHPile  won;                                              // Won ticks
      Gtk::Label name;
      Gtk::Label neededTricks;

      unsigned int bet;
   } players[NUM_PLAYERS];
   CardHPile played;
   CardWidget* pTrump;

   unsigned int startPlayer;
   unsigned int actTricks;
   Gtk::UIManager::ui_merge_id idMrg;

   ScoreDlg* pScoreDlg;

   static const unsigned int COLS_PLAYER[NUM_PLAYERS];
   static const unsigned int ROWS_PLAYER[NUM_PLAYERS];
};

#endif
