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
#include <gtkmm/statusbar.h>

#include <CardSet.h>
#include <CardPile.h>
#include <CardWidget.h>

#include <Game.h>


class Buraco : public Game {
 public:
   Buraco (Gtk::Box& parent, Gtk::Statusbar& statusbar, CardSet& cardset,
            const std::vector<std::string>& names);
   virtual ~Buraco ();

   virtual void start ();
   virtual void clean ();
   virtual const char* name () { return "Buraco"; }
   virtual void playOpen (bool);

 private:
   Buraco (const Buraco& other);
   const Buraco& operator= (const Buraco& other);

   static const unsigned int NUM_PLAYERS = 4;              // Number of players

   //@Section Virtual methods
   virtual int makeMove (unsigned int player);
   virtual bool enableHuman ();
   virtual void disableHuman ();

   //@Section Event handling
   void cardSelected (unsigned int iCard);
   void dumpedSelected ();
   void doDumpedSelected ();
   void stapleSelected ();
   void doStapleSelected ();

   //@Section helper methods
   void enableHumanHand ();
   void enableCard (unsigned int pos);
   static bool containsOnlyJoker (const std::vector<CardWidget*>& pile);
   static bool containsNoJoker (const std::vector<CardWidget*>& pile);
   void addReserve (unsigned int player, bool show = true);
   static bool isJoker (const CardWidget& card);
   unsigned int showCardsToPlay (unsigned int player);
   int  executeMove (unsigned int player);
   void endGame ();
   static bool cardFitsNext (ICardPile::const_iterator i);
   bool canGetRidOfCards (unsigned int player);
   bool canDumpCards (unsigned int player, unsigned int cards) const;
   static bool pileHasFittingPair (const ICardPile& pile,
                                   const CardWidget& card,
                                   bool pileHoldsCard = false);
   static bool pileHasFittingPair (const ICardPile& pile);

   //@Section to handle piles on table
   CardVPile& makeNewPile (unsigned int team);
   unsigned int cardFitsOnPlayedPile (unsigned int player, unsigned int card);
   int  cardFitsOnPile (ICardPile& pile, const CardWidget& card) const;
   void removeCerrado (unsigned int player, CardVPile& pile);
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
                     GtkSelectionData* pData, guint info, guint32 time,
                     unsigned int cardPos);
   void cardDropped (const Glib::RefPtr<Gdk::DragContext>& pContext, gint, gint,
                     GtkSelectionData* pData, guint info, guint32 time,
                     unsigned int card);
   void cardDroppedOnTable (const Glib::RefPtr<Gdk::DragContext>& pContext, gint,
                            gint, GtkSelectionData* pData, guint, guint32 time,
                            unsigned int cardPile);

   CardHPile hands[NUM_PLAYERS];              // For all players: Cards in hand
   std::vector<CardVPile*> tablePiles[NUM_PLAYERS >> 1];       // Piles on table
   std::vector<CardWidget*> reserve[NUM_PLAYERS >> 1];  // New staple for teams
   unsigned int cerrados[NUM_PLAYERS >> 1];          // Number of cerrados/team

   unsigned int startPlayer;

   Gtk::Statusbar info;
   Gtk::HBox      boxTeam[2];

   Gtk::Label     newPile;
   CardVInfoPile staple;
   CardVInfoPile dumped;
   SigC::Connection dumpedTop;
   SigC::Connection stapleTop;

   std::map<CardWidget*, SigC::Connection> aDNDHand;
   std::map<CardWidget*, SigC::Connection> aDNDTable;


   static std::vector<Gtk::TargetEntry> dndType;

   bool startTurn;
   unsigned int target;
   unsigned int pos1;
   unsigned int pos2;
};

#endif
