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
   typedef enum { NORMAL, COMPRESSED, VERY_COMPRESSED, LAST } Style;

   CardPile (Style style = NORMAL, bool access = true);
   virtual ~CardPile ();

   void setStyle (Style s) { style = s; }
   void setAccessable (bool access);

   void flipTopCard ();
   void setTopCardVisible (bool visible = true);
   void setTopCardInVisible () { setTopCardVisible (false); }

   void setTopCards (const vector<CardWidget*>& staple);
   void setTopCards (const vector<CardWidget*>& staple, bool visible);
   void setTopCards (const vector<CardWidget*>& staple, bool visible, bool lastVisible) {
      setTopCards (staple, visible);
      setTopCardVisible (lastVisible); }

   void setTopCard (CardWidget& newCard);
   void setTopCard (CardWidget& newCard, bool visible) {
      newCard.setVisible (visible);
      setTopCard (newCard); }

   CardWidget& getTopCard () { return *cards[cards.size () - 1]; }
   CardWidget& removeTopCard ();
   void clear ();

   CardWidget* getCard (unsigned int id) const;

   bool existCard (CardWidget& card) const { existCard (&card); }
   bool existCard (CardWidget* card) const {
      return find (cards.begin (), cards.end (), card) != cards.end (); }

   unsigned int numberOfCards () const { return cards.size (); }

 private:
   vector<CardWidget*> cards;

   Style style;
   bool accessable;
};

#endif
