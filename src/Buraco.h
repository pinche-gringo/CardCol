#ifndef BURACO_H
#define BURACO_H

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


#include <map>
#include <string>
#include <vector>

#include <gtkmm/label.h>
#include <gtkmm/frame.h>

#include <card/Set.h>
#include <card/Pile.h>
#include <card/Widget.h>

#include "BuracoPile.h"

#include <card/Game.h>


namespace Gtk {
   class ScrolledWindow;
}

namespace Card {
   class ScoreDlg;
}


/**Class handling the Buraco cardgame
 */
class Buraco: public Card::Game {
   friend class Settings;
   friend class CardgameAppl;
   friend class CardgameCollection;

 public:
   Buraco(Gtk::Box& parent, Gtk::Statusbar& statusbar, Card::Set& cardset,
          const std::vector<Card::Player*>& player, unsigned int posPlayer, YGP::Mutex& mxSerialize);
   virtual ~Buraco();

   virtual void start();
   virtual void clean();
   virtual const char* name() { return "Buraco"; }
   virtual void playOpen(bool);
   virtual void addMenus(Glib::RefPtr<Gtk::UIManager> mgrUI);
   virtual void removeMenus(Glib::RefPtr<Gtk::UIManager> mgrUI);

   virtual void changeNames(const std::vector<Card::Player*>& newPlayer);
   virtual void resizeCards();

   virtual unsigned int numberOfDecks() const { return 4; }
   virtual unsigned int numberOfJokers() const { return 3; }

   virtual bool handleMessage(unsigned int player, const std::string& msg);

   static unsigned int getPoints(const Card::Widget& card);
   static bool isJoker(const Card::Widget& card);
   static int cardDistance(const Card::Widget& a, const Card::Widget& b);
   static int cardDistance(const Card::Widget& a, const Card::Widget& b, bool aceIsOne);

 private:
   Buraco(const Buraco& other);
   const Buraco& operator=(const Buraco& other);

   static const unsigned int NUM_PLAYERS = 4;              // Number of players

   //@Section Virtual methods
   virtual void makeMove(unsigned int player);
   virtual bool enableHuman();
   virtual void disableHuman();

   virtual Card::IPile* getPileOfPlayer(unsigned int player, unsigned int pile);
   virtual bool executeRemoteMove(Card::IPile& pile, unsigned int target);
   virtual unsigned int getActTarget() const;
   void endTurn(unsigned int player, int card2Dump);

   //@Section Event handling
   void cardSelected(unsigned int iCard);
   void dumpedSelected();
   void doDelayedDumpedSelected();
   void doDumpedSelected();
   void stapleSelected();
   void doStapleSelected();
   bool doRegisterHand(unsigned int first, unsigned int last);

   void undoMove();
   void undoLast(unsigned int player);
   void sortHand();
   void sortHandByColour();

   //@Section helper methods
   void addBuraco4HumanAndEnable();
   void enableHumanHand();
   void enableCard(unsigned int pos);
   static bool containsOnlyJoker(const Card::IPile& pile);
   static bool containsNoJoker(const Card::IPile& pile);
   static bool showJoker(Card::IPile* pile, unsigned int cJokers, bool show);
   void addBuraco(unsigned int player);
   void playCards();
   int  executeMove(unsigned int player, unsigned int& pos1Play, unsigned int& pos2Play);
   void endGame();
   bool canClosePile(unsigned int player, unsigned int pile) const;
   bool canGetRidOfCards(unsigned int player) const;
   bool canPlayCards(unsigned int player, unsigned int cards, unsigned int pile = -1U) const;

   void sendMoveCard(unsigned int pile, unsigned int from, unsigned int to) const;
   static bool pileHasFittingPair(const Card::IPile& pile, const Card::Widget& card, bool withJokers=false);
   static bool pileHasFittingPair(const Card::IPile& pile, const Card::Widget* exclude=NULL);
   static bool compByNumberWithJokers(const Card::Widget* a, const Card::Widget* b);
   static bool compByColourWithJokers(const Card::Widget* a, const Card::Widget* b);
   void makeTeamNames(std::vector<Card::Player*>& names) const;
   void setStartPlayer();
   bool cleanup();

   //@Section to handle piles on table
   BuracoPile& makeNewPile(unsigned int team);
   unsigned int cardFitsOnPlayedPile(unsigned int player, unsigned int card);
   int  cardFitsOnPile(unsigned int pile, const Card::Widget& card) const;
   void removeCerrado(unsigned int player, BuracoPile& pile);
   void cleanCerrado(unsigned int player);
   void updateInfo();
   bool humanPilesOK(unsigned int except=-1U) const;

   //@Section DND
   void registerTableDND(unsigned int pile, unsigned int start, unsigned int end);
   void registerTableDND(Card::Widget& card, unsigned int nr);
   void unregisterTableDND(Card::Widget& card);
   void registerHandDND(unsigned int start, unsigned int end);
   void registerHandDND(unsigned int iCard);
   void unregisterHandDND(Card::Widget& card);
   void getDropData(const Glib::RefPtr<Gdk::DragContext>& pContext, Gtk::SelectionData& data, guint info,
                    guint32 time, unsigned int cardPos);
   void cardDropped(const Glib::RefPtr<Gdk::DragContext>& pContext, gint, gint,
                    const Gtk::SelectionData& data, guint info, guint32 time, unsigned int card);
   void cardDroppedOnTable(const Glib::RefPtr<Gdk::DragContext>& pContext, gint, gint,
                           const Gtk::SelectionData& pData, guint, guint32 time, unsigned int cardPile);

   Gtk::Label names[NUM_PLAYERS];                        // Names of the player
   Card::HPile hands[NUM_PLAYERS];            // For all players: Cards in hand
   std::vector<BuracoPile*> tablePiles[NUM_PLAYERS >> 1];     // Piles on table
   std::vector<Card::Widget*> reserve[NUM_PLAYERS >> 1];  // New staple 4 teams
   int points[NUM_PLAYERS >> 1];                       // Number of points/team
   unsigned int unfinishedMonoPiles[NUM_PLAYERS >> 1];

   Gtk::ScrolledWindow* scrlTable[NUM_PLAYERS >> 1];   // Scroll-ctrls for table

   std::vector<Card::Player*> nameTeams;
   unsigned int startPlayer;

   Gtk::Label info;
   Gtk::HBox  boxTeam[NUM_PLAYERS >> 1];

   Gtk::Label       newPile;
   Card::VInfoPile  staple;
   Card::VInfoPile  dumped;
   sigc::connection dumpedTop;
   sigc::connection stapleTop;

   typedef struct {
      sigc::connection connReceive;
      sigc::connection connGet;
   } CONNECTIONS;
   std::map<Card::Widget*, CONNECTIONS> aDNDHand;
   std::map<Card::Widget*, sigc::connection> aDNDTable;

   static std::vector<Gtk::TargetEntry> dndType;

   struct {
      unsigned int startGame : 1;
      unsigned int startTurn : 1;
      unsigned int team1Buraco : 2;
      unsigned int team2Buraco : 2;
      unsigned int pickUpPlayed : 1;
   } gStatus;

   typedef struct undoValue {
      unsigned int destPile : 8;
      unsigned int destPos  : 3;
      unsigned int srcPos   : 7;
      unsigned int pickUp   : 1;
      unsigned int monoPos  : 3;

      void assign(unsigned int targetPile, unsigned int targetPos, unsigned int pos) {
	 destPile = targetPile;
	 destPos = targetPos;
	 srcPos = pos;
	 monoPos = 7;
	 pickUp = 0; }
   } undoValue;
   undoValue undo;

   Card::ScoreDlg* pScoreDlg;

   Gtk::UIManager::ui_merge_id idMrg;
   Glib::RefPtr<Gtk::Action> menuUndo;
   Glib::RefPtr<Gtk::Action> menuSort;
   Glib::RefPtr<Gtk::Action> menuSort2;
   Glib::RefPtr<Gtk::Action> menuShowScoreDlg;

   unsigned int target;               ///< Id identifying the target to play to

   static unsigned int ENDPOINTS;
   static unsigned int CARDS2DEAL;
};

#endif
