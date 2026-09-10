#ifndef CARDWIDGET_H
#define CARDWIDGET_H

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
// along with libYGP.  If not, see <http://www.gnu.org/licenses/>.


#include <string>

#include <gdkmm/pixbuf.h>

#include <gtkmm/box.h>
#include <gtkmm/picture.h>
#include <gtkmm/gestureclick.h>

#include <card/Images.h>


namespace Card {

/**Class to display a card on the screen.

  This is actually a plain box and not a button, to avoid
  side-effects caused by the theme.
 */
class Widget: public Gtk::Box {
 public:
   Widget(unsigned int card, bool showFace=true);
   Widget(const Widget&);
   ~Widget();

   /// Sets the card deck to use
   /// \param carddeck Deck to use
   static void setDeck(const Images& carddeck) { deck = &carddeck; }

   // Methods to show card. Note that just the image is changed
   void flip() { showFace(!isVisible); }
   void showFace(bool visible=true);
   void showBack()  { showFace(false); }
   bool showsFace() const { return isVisible; }

   typedef enum { CLUBS=0, DIAMONDS, SPADES, HEARTS } COLOURS;
   typedef enum { TWO=0, THREE, FOUR, FIVE, SIX, SEVEN, EIGHT, NINE, TEN,
                  JACK, QUEEN, KING, ACE, UNREACHABLE } NUMBERS;

   unsigned int id() const { return nrCard; }
   COLOURS colour() const { return transColour[nrCard & 0x3]; }
   NUMBERS number() const {
      return  (nrCard > 51) ? UNREACHABLE : static_cast<NUMBERS>((51 - nrCard) >> 2); }
   char numberStr() const { return strNumber(number()); }
   char colourStr() const { return strColour(colour()); }

   const Glib::RefPtr<Gdk::Pixbuf> getShownImage() const {
      return isVisible ? deck->getCardImage(nrCard) : deck->getCardBackground(); }
   const Glib::RefPtr<Gdk::Pixbuf> getImage() const { return deck->getCardImage(nrCard); }
   unsigned int getImageWidth() const { return deck->getCardImage(nrCard)->get_width(); }
   unsigned int getImageHeight() const { return deck->getCardImage(nrCard)->get_height(); }

   int compareNumber(Widget& other) const { return number() - other.number(); }

   friend std::ostream& operator<<(std::ostream& out, const Widget& card);
   void update ();

   sigc::signal<void()> signal_clicked() { return clicked_; }
   sigc::signal<void(double, double)> signal_right_clicked() { return rightClicked_; }

    void mark();
    void unmark();

    void set_size_request(int width= -1, int height= -1);

   static Widget* getEmpty();

   static char strNumber(Widget::NUMBERS nr);
   static char strColour(Widget::COLOURS col);

 protected:
   virtual void on_clicked();
   virtual void on_left_released(int nPress, double x, double y);
   virtual void on_right_released(int nPress, double x, double y);

 private:
   Widget(): clicked_(), rightClicked_(), img(), isVisible(false), nrCard(0) { initGestures(); }

   void initGestures();

   sigc::signal<void()> clicked_;
   sigc::signal<void(double, double)> rightClicked_;
   Gtk::Picture img;

   Glib::RefPtr<Gtk::GestureClick> leftClick;
   Glib::RefPtr<Gtk::GestureClick> rightClick;

   bool isVisible;
   unsigned int nrCard;

   static const Images* deck;
   static COLOURS transColour[4];
};

}

#endif
