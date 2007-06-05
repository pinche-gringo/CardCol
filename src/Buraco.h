#ifndef BURACO_H
#define BURACO_H

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


#include <map>
#include <string>
#include <vector>

#include <gtkmm/label.h>
#include <gtkmm/frame.h>

#include <CardSet.h>
#include <CardPile.h>
#include <CardWidget.h>

#include "BuracoPile.h"

#include <Game.h>


namespace Gtk {
   class ScrolledWindow;
}

class ScoreDlg;


/**Class handling the Buraco cardgame
 */
class Buraco : public Game {
   friend class Settings;
   friend class CardgameAppl;
   friend class CardgameCollection;

 public:
   Buraco (Gtk::Box& parent, Gtk::Statusbar& statusbar, CardSet& cardset,
           const std::vector<Player*>& player, unsigned int posPlayer,
           YGP::Mutex& mxSerialize);
   virtual ~Buraco ();

   virtual void start ();
   virtual void clean ();
   virtual const char* name () { return "Buraco"; }
   virtual void playOpen (bool);
   virtual void addMenus (Glib::RefPtr<Gtk::UIManager> mgrUI);
   virtual void removeMenus (Glib::RefPtr<Gtk::UIManager> mgrUI);

   virtual void changeNames (const std::vector<Player*>& newPlayer);
   virtual void resizeCards ();

   virtual unsigned int numberOfDecks () const { return 4; }
   virtual unsigned int numberOfJokers () const { return 3; }

   virtual bool handleMessage (unsigned int player, const std::string& msg) throw (YGP::ParseError, YGP::CommError);

   static unsigned int getPoints (const CardWidget& card);
   static bool isJoker (const CardWidget& card);
   static int cardDistance (const CardWidget& a, const CardWidget& b);
   static int cardDistance (const CardWidget& a, const CardWidget& b,
                            bool aceIsOne);

 private:
   Buraco (const Buraco& other);
   const Buraco& operator= (const Buraco& other);

   static const unsigned int NUM_PLAYERS = 4;              // Number of players

   //@Section Virtual methods
   virtual void makeMove (unsigned int player);
   virtual bool enableHuman ();
   virtual void disableHuman ();

   virtual ICardPile* getPileOfPlayer (unsigned int player, unsigned int pile);
   virtual bool executeRemoteMove (ICardPile& pile, unsigned int target) throw (YGP::ParseError);
   virtual unsigned int getActTarget () const;

   //@Section Event handling
   void cardSelected (unsigned int iCard);
   void dumpedSelected ();
   void doDumpedSelected ();
   void stapleSelected ();
   void doStapleSelected ();
   bool doRegisterHand (unsigned int first, unsigned int last);

   void undoMove ();
   void undoLast (unsigned int player);
   void sortHand ();
   void sortHandByColour ();

   //@Section helper methods
   void enableHumanHand ();
   void enableCard (unsigned int pos);
   static bool containsOnlyJoker (const std::vector<CardWidget*>& pile);
   static bool containsNoJoker (const std::vector<CardWidget*>& pile);
   static bool hideJoker (ICardPile* pile, unsigned int cJokers);
   void addBuraco (unsigned int player);
   void playCards ();
   int  executeMove (unsigned int player, unsigned int& pos1Play, unsigned int& pos2Play);
   void endGame ();
   bool canClosePile (unsigned int player, unsigned int pile) const;
   bool canGetRidOfCards (unsigned int player) const;
   bool canPlayCards (unsigned int player, unsigned int cards,
                      unsigned int pile = -1U) const;

   void sendMoveCard (unsigned int pile, unsigned int from, unsigned int to) const;
   static bool pileHasFittingPair (const ICardPile& pile, const CardWidget& card,
                                   bool withJokers = false);
   static bool pileHasFittingPair (const ICardPile& pile, const CardWidget* exclude = NULL);
   static bool compByNumberWithJokers (const CardWidget* a, const CardWidget* b);
   static bool compByColourWithJokers (const CardWidget* a, const CardWidget* b);
   void makeTeamNames (std::vector<Player*>& names) const;
   void setStartPlayer ();
   bool cleanup ();

   //@Section to handle piles on table
   BuracoPile& makeNewPile (unsigned int team);
   unsigned int cardFitsOnPlayedPile (unsigned int player, unsigned int card);
   int  cardFitsOnPile (unsigned int pile, const CardWidget& card) const;
   void removeCerrado (unsigned int player, BuracoPile& pile);
   void cleanCerrado (unsigned int player);
   void updateInfo ();
   bool humanPilesOK (unsigned int except = -1U) const;

   //@Section DND
   void registerTableDND (unsigned int pile, unsigned int start, unsigned int end);
   void registerTableDND (CardWidget& card, unsigned int nr);
   void unregisterTableDND (CardWidget& card);
   void registerHandDND (unsigned int start, unsigned int end);
   void registerHandDND (unsigned int iCard);
   void unregisterHandDND (CardWidget& card);
   void getDropData (const Glib::RefPtr<Gdk::DragContext>& pContext,
                     Gtk::SelectionData& data, guint info, guint32 time,
                     unsigned int cardPos);
   void cardDropped (const Glib::RefPtr<Gdk::DragContext>& pContext, gint, gint,
                     const Gtk::SelectionData& data, guint info, guint32 time,
                     unsigned int card);
   void cardDroppedOnTable (const Glib::RefPtr<Gdk::DragContext>& pContext, gint,
                            gint, const Gtk::SelectionData& pData, guint,
                            guint32 time, unsigned int cardPile);

   Gtk::Label names[NUM_PLAYERS];                        // Names of the player
   CardHPile hands[NUM_PLAYERS];              // For all players: Cards in hand
   std::vector<BuracoPile*> tablePiles[NUM_PLAYERS >> 1];     // Piles on table
   std::vector<CardWidget*> reserve[NUM_PLAYERS >> 1];  // New staple for teams
   int points[NUM_PLAYERS >> 1];                       // Number of points/team
   unsigned int unfinishedMonoPiles[NUM_PLAYERS >> 1];

   Gtk::ScrolledWindow* scrlTable[NUM_PLAYERS >> 1];   // Scroll-ctrls for table

   std::vector<Player*> nameTeams;
   unsigned int startPlayer;

   Gtk::Frame frameInfo;
   Gtk::Label info;
   Gtk::HBox  boxTeam[2];

   Gtk::Label       newPile;
   CardVInfoPile    staple;
   CardVInfoPile    dumped;
   SigC::Connection dumpedTop;
   SigC::Connection stapleTop;

   typedef struct {
      SigC::Connection connReceive;
      SigC::Connection connGet;
   } CONNECTIONS;
   std::map<CardWidget*, CONNECTIONS> aDNDHand;
   std::map<CardWidget*, SigC::Connection> aDNDTable;

   ICardPile takenDumpedCards;

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

      void assign (unsigned int targetPile, unsigned int targetPos, unsigned int pos) {
	 destPile = targetPile;
	 destPos = targetPos;
	 srcPos = pos;
	 monoPos = 7;
	 pickUp = 0; }
   } undoValue;
   undoValue undo;

   ScoreDlg* pScoreDlg;

   Gtk::UIManager::ui_merge_id idMrg;
   Glib::RefPtr<Gtk::Action> menuUndo;
   Glib::RefPtr<Gtk::Action> menuSort;
   Glib::RefPtr<Gtk::Action> menuSort2;

   static unsigned int ENDPOINTS;
   static unsigned int CARDS2DEAL;
};

#endif
