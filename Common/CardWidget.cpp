//$Id$

//PROJECT     : Cardgames
//SUBSYSTEM   : Common
//REFERENCES  :
//TODO        :
//BUGS        :
//REVISION    : $Revision$
//AUTHOR      : Markus Schwab
//CREATED     : 27.03.2002
//COPYRIGHT   : Anticopyright (A) 2002, 2003

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


#include <gtkmm/misc.h>

#include <cardgames-cfg.h>

#include <YGP/Check.h>
#include <YGP/Trace_.h>

#include "CardWidget.h"


CardWidget::COLOURS CardWidget::transColour[4] = { CLUBS, SPADES, HEARTS, DIAMONDS };


/*--------------------------------------------------------------------------*/
//Purpose   : Constructor; creates a cardwidget with the passed index of a pixmap
//Parameters: set: Images of cards
//            card: Number of image inside the set to display
//            visible: Flag, if card should be displayed visible
/*--------------------------------------------------------------------------*/
CardWidget::CardWidget (const CardImages& set, unsigned int card, bool visible)
   : isVisible (visible), nrCard (card), deck (set) {
   TRACE3 ("CardWidget::CardWidget (const CardImages&, unsinged int, bool) - "
           << card << " (" << visible << ')');
   
   img.set (isVisible ? deck.getCardImage (nrCard) : deck.getCardBackground ());
   img.set_alignment (0.0, 0.0);
   img.show ();

   add (img);
   add_events (Gdk::EXPOSURE_MASK | Gdk::BUTTON_PRESS_MASK | Gdk::BUTTON_RELEASE_MASK
               | Gdk::ENTER_NOTIFY_MASK | Gdk::LEAVE_NOTIFY_MASK);
}

/*--------------------------------------------------------------------------*/
//Purpose   : Copyconstructor; copies the image for the passed cardwidget
//Parameters: other: Card to copy
/*--------------------------------------------------------------------------*/
CardWidget::CardWidget (const CardWidget& other)
   : isVisible (other.isVisible), nrCard (other.nrCard), deck (other.deck) {
   TRACE3 ("CardWidget::CardWidget (const CardWidget&) - "
           << nrCard << " (" << isVisible << ')');

   img.set (isVisible ? deck.getCardImage (nrCard) : deck.getCardBackground ());
   img.set_alignment (0.0, 0.0);
   img.set_padding (0, 0);
   img.show ();

   add (img);
   add_events (Gdk::EXPOSURE_MASK | Gdk::BUTTON_PRESS_MASK | Gdk::BUTTON_RELEASE_MASK
               | Gdk::ENTER_NOTIFY_MASK | Gdk::LEAVE_NOTIFY_MASK);
}

/*--------------------------------------------------------------------------*/
//Purpose   : Destructor
/*--------------------------------------------------------------------------*/
CardWidget::~CardWidget () {
   TRACE9 ("CardWidget::~CardWidget () - " << *this);
}


/*--------------------------------------------------------------------------*/
//Purpose   : Shows either the cardimage of the image of the deck
/*--------------------------------------------------------------------------*/
void CardWidget::showFace (bool visible) {
   isVisible = visible;
   update ();
}

/*--------------------------------------------------------------------------*/
//Purpose   : Returns the number of the card as character
//Returns   : char: Character describing number of card
/*--------------------------------------------------------------------------*/
char CardWidget::numberStr () const {
   static Glib::ustring specialCards (_("TJQKA"));
   return ((number () >= CardWidget::TEN)
           ? specialCards[number ()  - CardWidget::TEN]
           : number () + '2');
}

/*--------------------------------------------------------------------------*/
//Purpose   : Returns the colour of the card as character
//Returns   : char: Character describing colour of card
/*--------------------------------------------------------------------------*/
char CardWidget::colourStr () const {
   // Letters describing the colours (clubs, spades, hearts, diamonds)
   static Glib::ustring colours (_("CSHD"));
   return colours[nrCard & 0x3];
}

/*--------------------------------------------------------------------------*/
//Purpose   : Returns the colour of the card as character
//Returns   : char: Character describing colour of card
/*--------------------------------------------------------------------------*/
void CardWidget::update () {
   TRACE3 ("CardWidget::update () - Card " << nrCard);

   img.set (isVisible ? deck.getCardImage (nrCard) : deck.getCardBackground ());
}

/*--------------------------------------------------------------------------*/
//Purpose   : Callback after clicking a CardWidget
/*--------------------------------------------------------------------------*/
void CardWidget::on_clicked () {
   TRACE9 ("CardWidget::on_clicked () - " << *this);
}

/*--------------------------------------------------------------------------*/
//Purpose   : Callback after clicking a CardWidget
/*--------------------------------------------------------------------------*/
bool CardWidget::on_button_release_event (GdkEventButton* ev) {
   Check1 (ev);
   TRACE9 ("CardWidget::on_button_release_event (GdkEventButton*) - "
           << ev->button << "; X: " << ev->x - 1 << "; Y: " << ev->y - 1
           << "; W: " << get_width () << "; H: " << get_height ());

   // It button 1 is released within the image: Generate a clicked signal
   if ((ev->button == 1)
       && ((ev->x - 1) < get_width ()) && ((ev->y - 1) < get_height ())) {
      clicked_.emit ();
      on_clicked ();
   }
   return false;
}

/*--------------------------------------------------------------------------*/
//Purpose   : Output operator; Writes number and colour of the card
/*--------------------------------------------------------------------------*/
std::ostream& operator<< (std::ostream& out, const CardWidget& card) {
   if (card.nrCard >= 52)
      out << "Joker";
   else
      out << card.colourStr () << card.numberStr ();
   return out;
}
