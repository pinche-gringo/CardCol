#ifndef MACHIAVELLI_H
#define MACHIAVELLI_H

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
#include <stack>
#include <vector>
#include <deque>

#include <gtkmm/label.h>
#include <gtkmm/button.h>
#include <gtkmm/scrolledwindow.h>

#include <XGP/AutoContainer.h>

#include <Game.h>
#include <CardWidget.h>

#include "MachiPile.h"


// Forward declarations
namespace YGP {
   class Mutex;
   class CardSet;
   class StatusObject;
}
namespace XGP {
   class MessageDlg;
}
namespace Gtk {
   class Statusbar;
}


/**Class handling the Machiavelli cardgame
 */
class Machiavelli : public Game {
 public:
   Machiavelli (Gtk::Box& parent, Gtk::Statusbar& statusbar, CardSet& cardset,
                const std::vector<Player*>& player, unsigned int posPlayer,
                YGP::Mutex& mxSerialize);
   virtual ~Machiavelli ();

   virtual void start ();
   virtual void clean ();
   virtual const char* name () { return "Machiavelli"; }
   virtual void playOpen (bool);

   virtual bool handleMessage (unsigned int player, const std::string& msg) throw (std::string);

 private:
   Machiavelli ();
   Machiavelli (const Machiavelli& other);
   const Machiavelli& operator= (const Machiavelli& other);

   static const unsigned int NUM_PLAYERS = 4;              // Number of players

   /// \name Virtual methods
   //@{
   virtual int makeMove (unsigned int player);
   virtual bool enableHuman ();
   virtual void disableHuman ();
   void changeNames (const std::vector<Player*>& newPlayer);

   virtual ICardPile& getPileOfPlayer (unsigned int player, unsigned int pile);
   virtual unsigned int getActTarget () const;
   //@}

   /// \name Helper methods
   //@{
   void setStartPlayer ();
   unsigned int findNextPlayer (unsigned int player) const;
   MachiPile& makeNewPile ();
   MachiPile& makeNewPile (unsigned int pos);
   void removePile (unsigned int pile);
   unsigned int showCardsToPlay (unsigned int player);
   void dealCard (unsigned int player);
   void checkPiles (YGP::StatusObject& obj) const;
   void endGame (unsigned int looser);
   bool hasSerie (ICardPile& playerPile);
   unsigned int cardFitsToPile (const CardWidget& card, unsigned int offset);
   unsigned int reorderTableToFit (ICardPile& playerPile);
   unsigned int reorderTableToFit2 (ICardPile& playerPile);
   unsigned int reorderTableToFit3 (ICardPile& playerPile);
   unsigned int reorderTableToFit4 (ICardPile& playerPile);
   //@}

   /// \name Drag-and-drop methods
   //@{
   void registerTableDND (unsigned int pile, unsigned int start, unsigned int end);
   void registerTableDND (CardWidget& card, unsigned int nr);
   void unregisterTableDND (CardWidget& card);
   void unregisterTableDND ();
   void registerHandDND (unsigned int start, unsigned int end);
   void registerHandDND (unsigned int iCard);
   void unregisterHandDND (CardWidget& card);
   void getDropData (const Glib::RefPtr<Gdk::DragContext>& pContext,
                     GtkSelectionData* pData, guint, guint32 time,
                     unsigned int cardPos);
   void cardDropped (const Glib::RefPtr<Gdk::DragContext>& pContext, gint, gint,
                     GtkSelectionData* pData, guint info, guint32 time,
                     unsigned int card);
   void cardDroppedOnTable (const Glib::RefPtr<Gdk::DragContext>& pContext, gint,
                            gint, GtkSelectionData* pData, guint info, guint32 time,
                            unsigned int cardPile);
   //@}

   /// \name Callback from events
   //@{
   void endTurn ();
   void undoMove (unsigned int number);
   //@}

   Gtk::Label names[NUM_PLAYERS];                        // Names of the player
   CardHPile  hands[NUM_PLAYERS];             // For all players: Cards in hand

   XGP::AutoContainer piles;                              // Piles on the table
   std::vector<MachiPile*> tablePiles;     // Piles on table; for faster access

   unsigned int startPlayer;

   Gtk::Label       newPile;
   CardVInfoPile    staple;
   Gtk::Button      nextTurn;

   typedef struct {
      SigC::Connection connReceive;
      SigC::Connection connGet;
   } CONNECTIONS;
   std::map<CardWidget*, CONNECTIONS> aDNDHand;
   std::map<CardWidget*, CONNECTIONS> aDNDTable;

   unsigned int target;       // Target of the last move of the computer player

   typedef struct undoValue {
      unsigned int destPos : 4;
      unsigned int srcPos : 4;
      unsigned int destPile : 8;
      unsigned int srcPile : 8;
      unsigned int number : 4;
      unsigned int create : 1;

      undoValue (unsigned int targetPile, unsigned int targetPos,
                 unsigned int pile, unsigned int pos, unsigned int nr)
          : destPos (targetPos), srcPos (pos), destPile (targetPile)
       , srcPile (pile), number (nr), create (false) { }
      undoValue (unsigned int targetPile, unsigned int targetPos,
                 unsigned int pile, unsigned int pos, unsigned int nr,
                 bool createPile)
          : destPos (targetPos), srcPos (pos), destPile (targetPile)
       , srcPile (pile), number (nr), create (createPile) { }
   } undoValue;

   std::stack<undoValue> undo;
   std::deque<unsigned int> posPiles;

   XGP::MessageDlg* undoDlg;
};

#endif
