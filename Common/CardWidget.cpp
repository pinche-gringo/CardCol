//$Id$

//PROJECT     : Cardgames
//SUBSYSTEM   : libCard
//REFERENCES  :
//TODO        :
//BUGS        :
//REVISION    : $Revision$
//AUTHOR      : Markus Schwab
//CREATED     : 27.03.2002
//COPYRIGHT   : Copyright (C) 2002 - 2008

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


#include <gtkmm/misc.h>

#include <cardgames-cfg.h>

#include <YGP/Check.h>
#include <YGP/Trace.h>

#include "CardWidget.h"


const CardImages* CardWidget::deck (NULL);
CardWidget::COLOURS CardWidget::transColour[4] = { CLUBS, SPADES, HEARTS, DIAMONDS };


//-----------------------------------------------------------------------------
/// Constructor; creates a cardwidget with the passed index of a pixmap
/// \param card: Number of image inside the set to display
/// \param visible: Flag, if card should be displayed visible
//-----------------------------------------------------------------------------
CardWidget::CardWidget (const unsigned int card, bool visible)
   : isVisible (visible), nrCard (card) {
   TRACE3 ("CardWidget::CardWidget (const CardImages&, unsinged int, bool) - "
           << card << " (" << visible << ')');
   Check1 (deck);

   img.set (isVisible ? deck->getCardImage (nrCard) : deck->getCardBackground ());
   img.set_alignment (0.0, 0.0);
   img.show ();

   add (img);
   add_events (Gdk::EXPOSURE_MASK | Gdk::BUTTON_PRESS_MASK | Gdk::BUTTON_RELEASE_MASK
               | Gdk::ENTER_NOTIFY_MASK | Gdk::LEAVE_NOTIFY_MASK);
}

//-----------------------------------------------------------------------------
/// Copyconstructor; copies the image for the passed cardwidget
/// \param other: Card to copy
//-----------------------------------------------------------------------------
CardWidget::CardWidget (const CardWidget& other)
   : isVisible (other.isVisible), nrCard (other.nrCard) {
   TRACE3 ("CardWidget::CardWidget (const CardWidget&) - " << nrCard << " (" << isVisible << ')');
   Check1 (deck);

   img.set (isVisible ? deck->getCardImage (nrCard) : deck->getCardBackground ());
   img.set_alignment (0.0, 0.0);
   img.set_padding (0, 0);
   img.show ();

   add (img);
   add_events (Gdk::EXPOSURE_MASK | Gdk::BUTTON_PRESS_MASK | Gdk::BUTTON_RELEASE_MASK
               | Gdk::ENTER_NOTIFY_MASK | Gdk::LEAVE_NOTIFY_MASK);
}

//-----------------------------------------------------------------------------
/// Destructor
//-----------------------------------------------------------------------------
CardWidget::~CardWidget () {
   TRACE9 ("CardWidget::~CardWidget () - " << *this);
}


//-----------------------------------------------------------------------------
/// Shows either the cardimage or the image of the deck
//-----------------------------------------------------------------------------
void CardWidget::showFace (bool visible) {
   isVisible = visible;
   update ();
}

//-----------------------------------------------------------------------------
/// Returns the number of the card as character
/// \returns \c char: Character describing number of card
//-----------------------------------------------------------------------------
char CardWidget::strNumber (CardWidget::NUMBERS nr) {
   static Glib::ustring specialCards (_("TJQKA"));
   return ((nr >= CardWidget::TEN) ? specialCards[nr  - CardWidget::TEN] : nr + '2');
}

//-----------------------------------------------------------------------------
/// Returns the colour of the card as character
/// \returns \c char: Character describing colour of card
//-----------------------------------------------------------------------------
char CardWidget::strColour (CardWidget::COLOURS col) {
   // Letters describing the colours (clubs, spades, hearts, diamonds)
   static Glib::ustring colours (_("CDSH"));
   return colours[col];
}

//-----------------------------------------------------------------------------
/// Returns the colour of the card as character
/// \returns \c char: Character describing colour of card
//-----------------------------------------------------------------------------
void CardWidget::update () {
   TRACE3 ("CardWidget::update () - Card " << nrCard);

   img.set (isVisible ? deck->getCardImage (nrCard) : deck->getCardBackground ());
}

//-----------------------------------------------------------------------------
/// Callback after clicking a CardWidget
//-----------------------------------------------------------------------------
void CardWidget::on_clicked () {
   TRACE9 ("CardWidget::on_clicked () - " << *this);
}

//-----------------------------------------------------------------------------
/// Callback after releasing the button on a CardWidget
//-----------------------------------------------------------------------------
bool CardWidget::on_button_release_event (GdkEventButton* ev) {
   Check1 (ev);
   TRACE9 ("CardWidget::on_button_release_event (GdkEventButton*) - "
           << ev->button << "; X: " << ev->x << "; Y: " << ev->y
           << "; W: " << get_width () << "; H: " << get_height ());

   // It button 1 is released within the image: Generate a clicked signal
   if ((ev->button == 1) && (ev->x || ev->y)
       && (ev->x < get_width ()) && (ev->y < get_height ())) {
      clicked_.emit ();
      on_clicked ();
   }
   return false;
}

//-----------------------------------------------------------------------------
/// Output operator; Writes number and colour of the card
//-----------------------------------------------------------------------------
std::ostream& operator<< (std::ostream& out, const CardWidget& card) {
   if (card.nrCard >= 52)
      out << "Joker";
   else
      out << card.colourStr () << card.numberStr ();
   return out;
}

//-----------------------------------------------------------------------------
/// Marks the card; this is done by changing the saturation
//-----------------------------------------------------------------------------
void CardWidget::mark () {
   Glib::RefPtr<Gdk::Pixbuf> pic (getShownImage ());
   Glib::RefPtr<Gdk::Pixbuf> dest (pic->copy ());

   pic->saturate_and_pixelate (dest, 0.5, true);
   img.set (dest);
}

//-----------------------------------------------------------------------------
/// Unmarks the card; this is done by changing the saturation back
//-----------------------------------------------------------------------------
void CardWidget::unmark () {
   update ();
}

//-----------------------------------------------------------------------------
/// Returns an empty card (without any image)
/// \returns CardWidget*: Empty card
/// \remarks Don't use; but if you do, you are responsible of deleting the it
//-----------------------------------------------------------------------------
CardWidget* CardWidget::getEmpty () {
   return new CardWidget;
}
