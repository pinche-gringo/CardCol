#ifndef BURAZNO_H
#define BURAZNO_H

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


#include <string>
#include <vector>

#include <gtkmm/label.h>
#include <gtkmm/button.h>

#include <CardSet.h>
#include <CardPile.h>
#include <CardWidget.h>

#include <Game.h>


class Burazno : public Game {
 public:
   Burazno (Gtk::Box& parent, Gtk::Statusbar& statusbar, CardSet& cardset,
            const std::vector<std::string>& names);
   virtual ~Burazno ();

   virtual void start ();
   virtual void clean ();
   virtual const char* name () { return "Burazno"; }

 private:
   Burazno (const Burazno& other);
   const Burazno& operator= (const Burazno& other);

   static const unsigned int NUM_PLAYERS = 4;              // Number of players

   //@Section Virtual methods
   virtual int makeMove (unsigned int player);
   virtual bool enableHuman ();

   //@Section Event handling
   void cardSelected (unsigned int iCard);

   //@Section helper methods
   void randomizeClonedCardsToPile (ICardPile& pile);

   //@Section DND
   void registerDND (unsigned int start, unsigned int end);
   void registerDND (unsigned int iCard);
   void unregisterDND (unsigned int card) const;
   void getDropData (const Glib::RefPtr<Gdk::DragContext>& pContext,
                     GtkSelectionData* pData, guint info, guint32 time,
                     unsigned int cardPos);
   void cardDropped (const Glib::RefPtr<Gdk::DragContext>& pContext, gint, gint,
                     GtkSelectionData* pData, guint info, guint32 time,
                     unsigned int card);

   CardHPile hands[NUM_PLAYERS];              // For players: Cards in the hand
   std::vector<CardVPile*> tablePiles[NUM_PLAYERS >> 1];

   PseudoInfoPile staple;
   PseudoInfoPile dumped;

   std::vector<CardWidget*> deck;

   static std::vector<Gtk::TargetEntry> dndType;
};

#endif
