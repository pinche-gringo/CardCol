#ifndef CARDPILE_H
#define CARDPILE_H

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
#include <algo.h>

#include <gtk--/box.h>

class CardWidget;

// Class to display a pile of cards on the screen
class CardPile : public Gtk::VBox {
 public:
   typedef enum { NORMAL, COMPRESSED, VERY_COMPRESSED } Style;

   CardPile (Style style = NORMAL);
   virtual ~CardPile ();

   void setTopCardVisible (bool visible = true);
   void setTopCard (CardWidget* newCard, bool cardVisible = true);
   CardWidget* removeTopCard ();

   bool existCard (CardWidget* card) const {
      return find (cards.begin (), cards.end (), card) != cards.end (); }


   unsigned int getCardNumber () const { return cards.size (); }

 private:
   vector<CardWidget*> cards;

   Style style;
};

#endif
