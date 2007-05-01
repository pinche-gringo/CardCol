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

#include <gdkmm/pixbuf.h>

#include <gtkmm/image.h>
#include <gtkmm/eventbox.h>

#include <CardImgs.h>


/**Class to display a card on the screen.

  This is actually an event-box and not a button, to avoid
  side-effects caused by the theme.
 */
class CardWidget : public Gtk::EventBox {
 public:
   CardWidget (const CardImages& set, unsigned int card, bool showFace = true);
   CardWidget (const CardWidget&);
   ~CardWidget ();

   // Methods to show card. Note that just the image is changed
   void flip () { showFace (!isVisible); }
   void showFace (bool visible = true);
   void showBack ()  { showFace (false); }
   bool showsFace () const { return isVisible; }

   typedef enum { CLUBS = 0, DIAMONDS, SPADES, HEARTS } COLOURS;
   typedef enum { TWO = 0, THREE, FOUR, FIVE, SIX, SEVEN, EIGHT, NINE, TEN,
                  JACK, QUEEN, KING, ACE, UNREACHABLE } NUMBERS;

   unsigned int id () const { return nrCard; }
   COLOURS colour () const { return transColour[nrCard & 0x3]; }
   NUMBERS number () const {
      return static_cast <NUMBERS> ((nrCard > 51) ? UNREACHABLE : (51 - nrCard) >> 2); }
   char numberStr () const { return strNumber (number ()); }
   char colourStr () const { return strColour (colour ()); }

   const Glib::RefPtr<Gdk::Pixbuf> getShownImage () const {
      return isVisible ? deck.getCardImage (nrCard) : deck.getCardBackground (); }
   const Glib::RefPtr<Gdk::Pixbuf> getImage () const { return deck.getCardImage (nrCard); }
   unsigned int getImageWidth () const { return deck.getCardImage (nrCard)->get_width (); }
   unsigned int getImageHeight () const { return deck.getCardImage (nrCard)->get_height (); }

   int compareNumber (CardWidget& other) const { return number () - other.number (); }

   friend std::ostream& operator<< (std::ostream& out, const CardWidget& card);
   void update ();

   sigc::signal<void> signal_clicked () { return clicked_; }

   void mark ();
   void unmark ();

   static char strNumber (CardWidget::NUMBERS nr);
   static char strColour (CardWidget::COLOURS col);

 protected:
  virtual void on_clicked ();
  virtual bool on_button_release_event (GdkEventButton* ev);

 private:
   CardWidget ();

   sigc::signal<void> clicked_;
   Gtk::Image img;

   bool isVisible;
   unsigned int nrCard;
   const CardImages& deck;

   static COLOURS transColour[4];
};

#endif
