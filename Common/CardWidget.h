#ifndef CARDWIDGET_H
#define CARDWIDGET_H

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

#include <gdk--/pixmap.h>
#include <gtk--/button.h>

#include <CardImgs.h>


// Class to display a card on the screen
class CardWidget : public Gtk::Button {
 public:
   CardWidget (const CardImages& set, unsigned int card, bool visible = true);
   ~CardWidget ();

   // Methods to show card. Note that just the image is changed
   void flip () { setVisible (!isVisible); }
   void setVisible (bool visible = true);
   void setInvisible ()  { setVisible (false); }

   typedef enum { CLUBS = 0, DIAMONDS, HEARTS, SPADES } COLORS;
   typedef enum { TWO = 0, THREE, FOUR, FIVE, SIX, SEVEN, EIGHT, NINE, TEN,
                  JACK, QUEEN, KING, ACE, UNREACHABLE } NUMBERS;

   unsigned int id () const { return nrCard; }
   COLORS color () const { return transColor[nrCard & 0x3]; }
   NUMBERS number () const {
      return static_cast <NUMBERS> ((deck.numberOfCards () - 1 - nrCard) >> 2); }

   bool visible () const { return isVisible; }

   unsigned int getImageHeight () const {
      return const_cast<Gdk_Pixmap&> (deck.getCardImage (nrCard)).height (); }
   unsigned int getImageWidth () const {
      return const_cast<Gdk_Pixmap&> (deck.getCardImage (nrCard)).width (); }

   int compareNumber (CardWidget& other) const { return number () - other.number (); }

   friend ostream& operator<< (ostream& out, const CardWidget& card);

 private:
   CardWidget ();
   CardWidget (const CardWidget&);

   bool isVisible;
   unsigned int nrCard;
   const CardImages& deck;

   static COLORS transColor[4];
};

#endif
