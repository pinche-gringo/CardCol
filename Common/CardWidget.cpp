//$Id$

//PROJECT     : Cardgames
//SUBSYSTEM   : Common
//REFERENCES  :
//TODO        :
//BUGS        :
//REVISION    : $Revision$
//AUTHOR      : Markus Schwab
//CREATED     : 27.03.2002
//COPYRIGHT   : Anticopyright (A) 2002

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

#include <Check.h>
#include <Trace_.h>

#include <gtk--/pixmap.h>

#include <cardgames-cfg.h>

#include "CardWidget.h"


CardWidget::COLORS CardWidget::transColor[4] = { CLUBS, SPADES, HEARTS, DIAMONDS };


/*--------------------------------------------------------------------------*/
//Purpose   : Constructor; creates a cardwidget with the passed index of a pixmap
//Parameters: set: Images of cards
//            card: Number of image inside the set to display
//            visible: Flag, if card should be displayed visible
/*--------------------------------------------------------------------------*/
CardWidget::CardWidget (const CardImages& set, unsigned int card, bool visible = true)
   : isVisible (visible), nrCard (card), deck (set) {
   TRACE3 ("CardWidget::CardWidget (const CardImages&, unsinged int, bool) - "
           << card << " (" << visible << ')');

   add_pixmap (visible ? deck.getCardImage (nrCard) : deck.getCardBackground (),
               NULL);
   set_relief (GTK_RELIEF_NONE);
}

/*--------------------------------------------------------------------------*/
//Purpose   : Copyconstructor; copies the image for the passed cardwidget
//Parameters: other: Card to copy
/*--------------------------------------------------------------------------*/
CardWidget::CardWidget (const CardWidget& other)
   : isVisible (other.isVisible), nrCard (other.nrCard), deck (other.deck) {
   TRACE3 ("CardWidget::CardWidget (const CardWidget&) - "
           << nrCard << " (" << isVisible << ')');

   add_pixmap (isVisible ? deck.getCardImage (nrCard) : deck.getCardBackground (),
               NULL);
   set_relief (GTK_RELIEF_NONE);
}

/*--------------------------------------------------------------------------*/
//Purpose   : Destructor
/*--------------------------------------------------------------------------*/
CardWidget::~CardWidget () {
   TRACE9 ("CardWidget::~CardWidget ()");
}


/*--------------------------------------------------------------------------*/
//Purpose   : Shows either the cardimage of the image of the deck
/*--------------------------------------------------------------------------*/
void CardWidget::showFace (bool visible) {
   isVisible = visible;
   update ();
   Check3 (get_child ()); Check3 (Gtk::Pixmap::isA (get_child ()));
}

/*--------------------------------------------------------------------------*/
//Purpose   : Returns the number of the card as character
//Returns   : char: Character describing number of card
/*--------------------------------------------------------------------------*/
char CardWidget::numberStr () const {
   static char* specialCards = _("TJQKA");
   return ((number () >= CardWidget::TEN)
           ? specialCards[number ()  - CardWidget::TEN]
           : number () + '2');

}

/*--------------------------------------------------------------------------*/
//Purpose   : Returns the color of the card as character
//Returns   : char: Character describing color of card
/*--------------------------------------------------------------------------*/
char CardWidget::colorStr () const {
   // Letters describing the colors (clubs, spades, hearts, diamonds)
   static char* colors = _("CSHD");
   return colors[nrCard & 0x3];
}

/*--------------------------------------------------------------------------*/
//Purpose   : Returns the color of the card as character
//Returns   : char: Character describing color of card
/*--------------------------------------------------------------------------*/
void CardWidget::update () {
   TRACE3 ("CardWidget::update () - Card " << nrCard);

   remove ();
   add_pixmap (isVisible ? deck.getCardImage (nrCard) : deck.getCardBackground (),
               NULL);

   dynamic_cast <Gtk::Pixmap*> (get_child ())->set_alignment (0.0, 0.0);
}
