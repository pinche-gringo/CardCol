//$Id$

//PROJECT     : Cardgames
//SUBSYSTEM   : Common
//REFERENCES  :
//TODO        :
//BUGS        :
//REVISION    : $Revision$
//AUTHOR      : Markus Schwab
//CREATED     : 03.04.2002
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


#define DEBUG 9
#include <Check.h>
#include <Trace_.h>

#include "CardWidget.h"

#include "CardPile.h"


/*--------------------------------------------------------------------------*/
//Purpose   : Constructor; adds all controls to the dialog
//Parameters: set: Specifier for type of cardset
/*--------------------------------------------------------------------------*/
CardPile::CardPile (Style s) : style (s) {
   TRACE3 ("CardPile::CardPile (Style) - " << (int)style);
}

/*--------------------------------------------------------------------------*/
//Purpose   : Destructor
/*--------------------------------------------------------------------------*/
CardPile::~CardPile () {
   TRACE9 ("CardPile::~CardPile ()");
}


/*--------------------------------------------------------------------------*/
//Purpose   : Sets the top card of the pile
//Parameters: newCard: New top-card
//            cardVisible: Flag, if the card is visible (or just back is shown)
/*--------------------------------------------------------------------------*/
void CardPile::setTopCard (CardWidget* newCard, bool cardVisible) {
   TRACE5 ("CardPile::setTopCard (CardWidget*, bool) - New size: " << cards.size () + 1);

   pack_start (*newCard, false);
   if ((style > NORMAL) && cards.size ()) {
      CardWidget* card (cards[cards.size () - 1]); Check3 (card);
      card->set_usize (card->width (), style == COMPRESSED ? 5 : 1);
   } 
   cards.push_back (newCard);
}

/*--------------------------------------------------------------------------*/
//Purpose   : Sets the top card of the pile
//Parameters: new: New top-card
//            cardVisible: Flag, if the card is visible (or just back is shown)
/*--------------------------------------------------------------------------*/
CardWidget* CardPile::removeTopCard () {
   TRACE5 ("CardPile::removeTopCard () - New size: " << cards.size () - 1);

   Check3 (cards.size () > 0);
   remove (*cards[cards.size () - 1]);
   cards.pop_back ();
}
