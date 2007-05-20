#ifndef CARDWINDOW_H
#define CARDWINDOW_H

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


#include <gtkmm/window.h>


class ICardPile;
class CardWidget;


/**Window holding exactly one card. This window can be used to animate a
 * card or fully show it if its put in a pile.
 */
class CardWindow : public Gtk::Window {
 public:
   CardWindow (CardWidget& card);
   ~CardWindow () { }

   CardWidget& getCard () const { return *(CardWidget*)*get_children ().begin (); }

 private:
   CardWindow ();
   CardWindow (const CardWindow&);
   CardWindow& operator= (const CardWindow&);
};


/**Window holding a pile of cards, which can be used for animation.
 */
class CardPileWindow : public Gtk::Window {
 public:
   CardPileWindow (bool horizontal = true);
   ~CardPileWindow ();

   ICardPile& getPile () const { return *pile; }

 private:
   CardPileWindow ();
   CardPileWindow (const CardPileWindow&);
   CardPileWindow& operator= (const CardPileWindow&);

   ICardPile* pile;
};


#endif
