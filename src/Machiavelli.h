#ifndef MACHIAVELLI_H
#define MACHIAVELLI_H

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


#include <map>
#include <stack>
#include <vector>
#include <deque>

#include <gtkmm/label.h>
#include <gtkmm/button.h>
#include <gtkmm/scrolledwindow.h>

#include <XGP/AutoContainer.h>

#include <Game.h>
#include <CardWindow.h>
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

   virtual void addMenus (Glib::RefPtr<Gtk::UIManager> mgrUI);
   virtual void removeMenus (Glib::RefPtr<Gtk::UIManager> mgrUI);

   virtual unsigned int numberOfDecks () const { return 4; }
   virtual void resizeCards ();

   virtual bool handleMessage (unsigned int player, const std::string& msg) throw (YGP::ParseError, YGP::CommError);

 private:
   Machiavelli ();
   Machiavelli (const Machiavelli& other);
   const Machiavelli& operator= (const Machiavelli& other);

   static const unsigned int NUM_PLAYERS = 4;              // Number of players

   /// \name Virtual methods
   //@{
   virtual void makeMove (unsigned int player);
   virtual bool enableHuman ();
   virtual void disableHuman ();
   void changeNames (const std::vector<Player*>& newPlayer);

   virtual ICardPile* getPileOfPlayer (unsigned int player, unsigned int pile);
   virtual unsigned int getActTarget () const;
   //@}

   /// \name Helper methods
   //@{
   void setStartPlayer ();
   unsigned int findNextPlayer (unsigned int player) const;
   MachiPile& makeNewPile ();
   MachiPile& makeNewPile (unsigned int pos);
   void removePile (unsigned int pile);
   bool showCardsToPlay (unsigned int player);
   void dealCard (unsigned int player);
   void checkPiles (YGP::StatusObject& obj) const;
   void endGame (unsigned int looser);
   bool playSerie (ICardPile& playerPile);
   bool cardFitsOnPile (const CardWidget& card, unsigned int offset);
   bool reorderTableToFit (ICardPile& playerPile);
   bool reorderTableToFit2 (ICardPile& playerPile);
   bool reorderTableToFit3 (ICardPile& playerPile);
   bool reorderTableToFit4 ();
   void addBorderCards2Missing (unsigned int iPile, unsigned int which = -1U);
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
                     Gtk::SelectionData& data, guint, guint32 time,
                     unsigned int cardPos);
   void cardDropped (const Glib::RefPtr<Gdk::DragContext>& pContext, gint, gint,
                     const Gtk::SelectionData& pData, guint info, guint32 time,
                     unsigned int card);
   void cardDroppedOnTable (const Glib::RefPtr<Gdk::DragContext>& pContext, gint,
                            gint, const Gtk::SelectionData& data, guint info, guint32 time,
                            unsigned int cardPile);

   bool doRegisterHand (unsigned int first, unsigned int last);
   //@}

   /// \name Callback from events
   //@{
   void unmarkAndEnd (MachiPile* pile);
   void endTurn ();
   void doEndTurn ();
   void undoMove (unsigned int number);
   void removeUndoDlg (int);
   void sortHand ();
   void sortHandByColour ();
   //@}

   void endComputerMove ();

   Gtk::Label names[NUM_PLAYERS];                        // Names of the player
   CardHPile  hands[NUM_PLAYERS];             // For all players: Cards in hand

   XGP::AutoContainer piles;                              // Piles on the table
   std::vector<MachiPile*> tablePiles;     // Piles on table; for faster access

   unsigned int startPlayer;

   Gtk::Label       newPile;
   CardVInfoPile    staple;
   Gtk::Button      nextTurn;

   typedef struct {
      sigc::connection connReceive;
      sigc::connection connGet;
   } CONNECTIONS;
   std::map<CardWidget*, CONNECTIONS> aDNDHand;
   std::map<CardWidget*, CONNECTIONS> aDNDTable;

   unsigned int target;       // Target of the last move of the computer player

   // Structure holding undo-information
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

   // Structure to store which cards are missing on a pile, to be able to add from hand
   typedef struct missingCards {
      unsigned int        pile;
      CardWidget::NUMBERS nr;
      CardWidget::COLOURS colour;

      missingCards (unsigned int pile, CardWidget::NUMBERS nr, CardWidget::COLOURS colour)
	 : pile (pile), nr (nr), colour (colour) { }
      missingCards (unsigned int pile) : pile (pile) { }
   } missingCards;
   std::vector<missingCards> missing;

   XGP::MessageDlg* undoDlg;

   Glib::RefPtr<Gtk::Action> undo1;
   Glib::RefPtr<Gtk::Action> undoAll;
   Glib::RefPtr<Gtk::Action> nxtTurn;
   Gtk::UIManager::ui_merge_id idMrg;
};

#endif
