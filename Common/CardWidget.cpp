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

#define DEBUG 0
#include <Trace_.h>

#include "CardImgs.h"
#include "CardWidget.h"


/*--------------------------------------------------------------------------*/
//Purpose   : Constructor; adds all controls to the dialog
//Parameters: parent: Parent-window
//            image: Image to display
/*--------------------------------------------------------------------------*/
CardWidget::CardWidget (const CardImages& set, unsigned int card, bool visible = true)
   : isVisible (visible), nrCard (card), deck (set) {
   TRACE3 ("CardWidget::CardWidget (const CardImages&, unsinged int, bool) - "
           << card << " (" << visible << ')');

   add_pixmap (visible ? deck.getCardImage (nrCard) : deck.getCardBackground (),
               NULL);
}

/*--------------------------------------------------------------------------*/
//Purpose   : Destructor
/*--------------------------------------------------------------------------*/
CardWidget::~CardWidget () {
   TRACE9 ("CardWidget::~CardWidget ()");
}


/*--------------------------------------------------------------------------*/
//Purpose   : Destructor
/*--------------------------------------------------------------------------*/
void CardWidget::setVisible (bool visible) {
   remove ();
   isVisible = visible;
   add_pixmap (visible ? deck.getCardImage (nrCard) : deck.getCardBackground (),
               NULL);
}
