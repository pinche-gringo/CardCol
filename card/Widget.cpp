//PROJECT     : Cardgames
//SUBSYSTEM   : libCard
//REFERENCES  :
//TODO        :
//BUGS        :
//AUTHOR      : Markus Schwab
//CREATED     : 27.03.2002
//COPYRIGHT   : Copyright (C) 2002 - 2018

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


#include <cardgames-cfg.h>

#define TRACELEVEL 1
#include <YGP/Check.h>
#include <YGP/Trace.h>

#include "Widget.h"


namespace Card {


const Images* Widget::deck(NULL);
Widget::COLOURS Widget::transColour[4] = { CLUBS, SPADES, HEARTS, DIAMONDS };


//-----------------------------------------------------------------------------
/// Constructor; creates a cardwidget with the passed index of a pixmap
/// \param card Number of image inside the set to display
/// \param visible Flag, if card should be displayed visible
//-----------------------------------------------------------------------------
Widget::Widget(const unsigned int card, bool visible)
   : clicked_(), rightClicked_(), img(), isVisible(visible), nrCard(card) {
   TRACE3("Widget::Widget(const Images&, unsinged int, bool) - "
          << card << " (" << visible << ')');
   Check1(deck);

   img.set(isVisible ? deck->getCardImage(nrCard) : deck->getCardBackground());
   img.set_halign(Gtk::Align::START);
   img.set_valign(Gtk::Align::START);
   img.show();

   append(img);
   initGestures();
}

//-----------------------------------------------------------------------------
/// Copyconstructor; copies the image for the passed cardwidget
/// \param other Card to copy
//-----------------------------------------------------------------------------
Widget::Widget(const Widget& other)
   : Gtk::Box(), clicked_(), rightClicked_(), img(), isVisible(other.isVisible), nrCard(other.nrCard) {
   TRACE3("Widget::Widget(const Widget&) - " << nrCard << "(" << isVisible << ')');
   Check1(deck);

   img.set(isVisible ? deck->getCardImage(nrCard) : deck->getCardBackground());
   img.set_halign(Gtk::Align::START);
   img.set_valign(Gtk::Align::START);
   img.show();

   append(img);
   initGestures();
}

//-----------------------------------------------------------------------------
/// Destructor
//-----------------------------------------------------------------------------
Widget::~Widget() {
   TRACE9("Widget::~Widget() - " << *this);
}

//-----------------------------------------------------------------------------
/// Installs the gesture-controllers used to detect clicks on the card
//-----------------------------------------------------------------------------
void Widget::initGestures() {
   leftClick = Gtk::GestureClick::create();
   leftClick->set_button(GDK_BUTTON_PRIMARY);
   leftClick->signal_released().connect
      (sigc::mem_fun(*this, &Widget::on_left_released));
   add_controller(leftClick);

   rightClick = Gtk::GestureClick::create();
   rightClick->set_button(GDK_BUTTON_SECONDARY);
   rightClick->signal_released().connect
      (sigc::mem_fun(*this, &Widget::on_right_released));
   add_controller(rightClick);
}


//-----------------------------------------------------------------------------
/// Shows either the cardimage or the image of the deck
//-----------------------------------------------------------------------------
void Widget::showFace(bool visible) {
   isVisible = visible;
   update();
}

//-----------------------------------------------------------------------------
/// Returns the number of the card as character
/// \returns char Character describing number of card
//-----------------------------------------------------------------------------
char Widget::strNumber(Widget::NUMBERS nr) {
   static Glib::ustring specialCards(_("TJQKA"));
   return ((nr >= Widget::TEN) ? specialCards[nr  - Widget::TEN] : nr + '2');
}

//-----------------------------------------------------------------------------
/// Returns the colour of the card as character
/// \returns char Character describing colour of card
//-----------------------------------------------------------------------------
char Widget::strColour(Widget::COLOURS col) {
   // Letters describing the colours(clubs, spades, hearts, diamonds)
   static Glib::ustring colours(_("CDSH"));
   return colours[col];
}

//-----------------------------------------------------------------------------
/// Updates the widget; i.e. shows its image according to the status
//-----------------------------------------------------------------------------
void Widget::update() {
   TRACE3("Widget::update() - Card " << nrCard);

   img.set (isVisible ? deck->getCardImage (nrCard) : deck->getCardBackground ());
   // Glib::RefPtr<Gdk::Pixbuf> current(getShownImage());
   // Glib::RefPtr<Gdk::Pixbuf> pic(isVisible ? deck->getCardImage(nrCard) : deck->getCardBackground());
   // TRACE1("Update" << (pic != current));
   // if (pic != current) {
   //     TRACE1("Update sizes " << pic->get_width() << '/' << current->get_width() << '/' << pic->get_height() << '/' << current->get_height());
   //     if ((pic->get_width() != current->get_width()) || (pic->get_height() != current->get_height()))
   //        pic = Gdk::Pixbuf::create_subpixbuf(current, 0, 0, pic->get_width(), pic->get_height());
   //     img.set(pic);
   // }
}

//-----------------------------------------------------------------------------
/// Callback after clicking a Widget
//-----------------------------------------------------------------------------
void Widget::on_clicked() {
   TRACE9("Widget::on_clicked() - " << *this);
}

//-----------------------------------------------------------------------------
/// Callback after releasing the left mouse button on a Widget
//-----------------------------------------------------------------------------
void Widget::on_left_released(int, double x, double y) {
   TRACE9("Widget::on_left_released(int, double, double) - X: " << x << "; Y: " << y
          << "; W: " << get_width() << "; H: " << get_height());

   // If released within the image: Generate a clicked signal
   if ((x || y) && (x < get_width()) && (y < get_height())) {
      clicked_.emit();
      on_clicked();
   }
}

//-----------------------------------------------------------------------------
/// Callback after releasing the right mouse button on a Widget
//-----------------------------------------------------------------------------
void Widget::on_right_released(int, double x, double y) {
   TRACE9("Widget::on_right_released(int, double, double) - X: " << x << "; Y: " << y
          << "; W: " << get_width() << "; H: " << get_height());

   if ((x || y) && (x < get_width()) && (y < get_height()))
      rightClicked_.emit(x, y);
}

//-----------------------------------------------------------------------------
/// Output operator; Writes number and colour of the card
//-----------------------------------------------------------------------------
std::ostream& operator<<(std::ostream& out, const Widget& card) {
   if (card.nrCard >= 52)
      out << "Joker";
   else
      out << card.colourStr() << card.numberStr();
   return out;
}

//-----------------------------------------------------------------------------
/// Marks the card; this is done by changing the saturation
//-----------------------------------------------------------------------------
void Widget::mark() {
   Glib::RefPtr<Gdk::Pixbuf> pic(getShownImage());
   Glib::RefPtr<Gdk::Pixbuf> dest(pic->copy());

   pic->saturate_and_pixelate(dest, 0.5, true);
   img.set(dest);
}

//-----------------------------------------------------------------------------
/// Unmarks the card; this is done by changing the saturation back
//-----------------------------------------------------------------------------
void Widget::unmark() {
   update();
}

//-----------------------------------------------------------------------------
/// Returns an empty card (without any image)
/// \returns CardWidget* Empty card
/// \remarks Don't use; but if you do, you are responsible of deleting it
//-----------------------------------------------------------------------------
Widget* Widget::getEmpty() {
   return new Widget;
}

//-----------------------------------------------------------------------------
/// Sets the minimum size of a widget; that is, the widget’s size request will
/// be at least width by height.
//-----------------------------------------------------------------------------
void Widget::set_size_request(int width, int height) {
   Glib::RefPtr<Gdk::Pixbuf> pic(getShownImage());
   Glib::RefPtr<Gdk::Pixbuf> dest(Gdk::Pixbuf::create_subpixbuf(pic, 0, 0,
                                                                width < 0 ? pic->get_width() : width,
                                                                height < 0 ? pic->get_height() : height));

   img.set(dest);
   Gtk::Box::set_size_request(width, height);
}

}
