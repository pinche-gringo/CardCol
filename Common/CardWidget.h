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

#include <gdkmm/pixmap.h>
#include <gtkmm/button.h>

#include <CardImgs.h>


// Class to display a card on the screen
class CardWidget : public Gtk::Button {
 public:
   CardWidget (const CardImages& set, unsigned int card, bool showFace = true);
   CardWidget (const CardWidget&);
   ~CardWidget ();

   // Methods to show card. Note that just the image is changed
   void flip () { showFace (!isVisible); }
   void showFace (bool visible = true);
   void showBack ()  { showFace (false); }
   bool showsFace () const { return isVisible; }

   typedef enum { CLUBS = 0, DIAMONDS, SPADES, HEARTS } COLORS;
   typedef enum { TWO = 0, THREE, FOUR, FIVE, SIX, SEVEN, EIGHT, NINE, TEN,
                  JACK, QUEEN, KING, ACE, UNREACHABLE } NUMBERS;

   unsigned int id () const { return nrCard; }
   COLORS color () const { return transColor[nrCard & 0x3]; }
   NUMBERS number () const {
      return static_cast <NUMBERS> ((deck.numberOfCards () - 1 - nrCard) >> 2); }
   char numberStr () const;
   char colorStr () const;

   const Glib::RefPtr<Gdk::Pixmap> getShownImage () const {
      return isVisible ? deck.getCardImage (nrCard) : deck.getCardBackground (); }
   const Glib::RefPtr<Gdk::Pixmap> getImage () const { return deck.getCardImage (nrCard); }
   unsigned int getImageHeight () const {
      int x, y;
      deck.getCardImage (nrCard)->get_size (x, y);
      return y; }
   unsigned int getImageWidth () const {
      int x, y;
      deck.getCardImage (nrCard)->get_size (x, y);
      return x; }

   int compareNumber (CardWidget& other) const { return number () - other.number (); }

   friend std::ostream& operator<< (std::ostream& out, const CardWidget& card) {
      out << card.colorStr () << card.numberStr ();
      return out; }

   void update ();

 private:
   CardWidget ();

   bool isVisible;
   unsigned int nrCard;
   const CardImages& deck;

   static COLORS transColor[4];
};

#endif
