//$Id$

//PROJECT     : Cardgames
//SUBSYSTEM   : Common
//REFERENCES  :
//TODO        :
//BUGS        :
//REVISION    : $Revision$
//AUTHOR      : Markus Schwab
//CREATED     : 8.5.2002
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


#include <stdlib.h>

#include <Check.h>
#include <Trace_.h>

#include "CardImgs.h"
#include "CardWidget.h"

#include "CardSet.h"


/*--------------------------------------------------------------------------*/
//Purpose   : Destructor
/*--------------------------------------------------------------------------*/
CardSet::~CardSet () {
   TRACE9 ("CardSet::~CardSet ()");
}


/*--------------------------------------------------------------------------*/
//Purpose   : Adds a set of cards (with images specified by decks) to set
//Parameters: decks: Class holding the images to add
/*--------------------------------------------------------------------------*/
void CardSet::addPacket (const CardImages& decks) {
   TRACE9 ("CardSet::addPacket (const CardImages&)");

   for (int i (0); i < decks.numberOfCards (); ++i) {
      CardWidget* card (new CardWidget (decks, i, true)); Check3 (card);

      cards_.push_back (card);
   } // endfor

}

/*--------------------------------------------------------------------------*/
//Purpose   : Shuffles the cards in the deck
/*--------------------------------------------------------------------------*/
void CardSet::shuffle () {
   TRACE1 ("CardSet::shuffle ()");

   unsigned int nr;
   for (int i (numberOfCards ()); i > 0;) {
      nr = rand () % i--;
      TRACE2 ("CardSet::shuffle () - " << i << " = " << nr);
      swap (cards_[i], cards_[nr]);
   }
}

/*--------------------------------------------------------------------------*/
//Purpose   : Retrieves the specified card
//Returns   : CardWidget&: Reference to CardWidget
/*--------------------------------------------------------------------------*/
CardWidget& CardSet::getCard (unsigned int nr) const {
   TRACE3 ("CardSet::getCard (unsigned int) - " << nr);
   Check3 (nr < numberOfCards ()); Check3 (cards_[nr]);

   return *cards_[nr];
}

/*--------------------------------------------------------------------------*/
//Purpose   : Actualizes the card set (after changes of the images)
/*--------------------------------------------------------------------------*/
void CardSet::update () const {
   TRACE3 ("CardSet::update () const");

   for (vector<CardWidget*>::const_iterator i (cards_.begin ());
        i != cards_.end (); ++i)
      (*i)->update ();
}
