//$Id$

//PROJECT     : Cardgames
//SUBSYSTEM   : Common
//REFERENCES  :
//TODO        :
//BUGS        :
//REVISION    : $Revision$
//AUTHOR      : Markus Schwab
//CREATED     : 8.5.2002
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


#include <cstdlib>

#define CHECK 9
#define TRACELEVEL 2
#include <Check.h>
#include <Trace_.h>

#include "CardImgs.h"
#include "CardWidget.h"

#include "CardSet.h"


//-----------------------------------------------------------------------------
/// Destructor
//-----------------------------------------------------------------------------
CardSet::~CardSet () {
   TRACE9 ("CardSet::~CardSet ()");
   clear ();
}


//-----------------------------------------------------------------------------
/// Adds a set of cards (with images specified by decks) to set
/// \param decks: Class holding the images to add
//-----------------------------------------------------------------------------
void CardSet::addPacket (const CardImages& decks) {
   TRACE9 ("CardSet::addPacket (const CardImages&)");

   for (int i (0); i < decks.size (); ++i) {
      CardWidget* card (new CardWidget (decks, i, true)); Check3 (card);
      card->show ();
      cards_.push_back (card);
   } // endfor

}

//-----------------------------------------------------------------------------
/// Shuffles the cards in the deck
//-----------------------------------------------------------------------------
void CardSet::shuffle () {
   TRACE2 ("CardSet::shuffle ()");

   unsigned int nr;
   for (int i (size ()); i > 0;) {
      nr = rand () % i--;
      TRACE2 ("CardSet::shuffle () - " << i << " = " << nr);
      std::swap (cards_[i], cards_[nr]);
   }
}

//-----------------------------------------------------------------------------
/// Sets the specified card in the passed slot of the set
/// \param pos: Slot (position) of the card to set
/// \param card: Card to swap
/// \pre The \c card must not be in a position before \c pos.
//-----------------------------------------------------------------------------
void CardSet::set (unsigned int pos, unsigned int card) {
   TRACE2 ("CardSet::set (unsigned int, unsigned int) - [" << pos << "] = " << card);
   Check1 (card < cards_.size ());

   for (std::vector<CardWidget*>::iterator i (cards_.begin () + pos);
        i != cards_.end (); ++i)
      if ((*i)->id () == card) {
         std::vector<CardWidget*>::iterator t (cards_.begin () + pos);
         std::swap (*t, *i);
         return;
       }
   Check3 (0);
}

//-----------------------------------------------------------------------------
/// Retrieves the specified card
/// \returns \c CardWidget&: Reference to CardWidget
//-----------------------------------------------------------------------------
CardWidget& CardSet::getCard (unsigned int nr) const {
   TRACE3 ("CardSet::getCard (unsigned int) - " << nr);
   Check3 (nr < size ()); Check3 (cards_[nr]);

   return *cards_[nr];
}

//-----------------------------------------------------------------------------
/// Actualizes the card set (after changes of the images)
//-----------------------------------------------------------------------------
void CardSet::update () const {
   TRACE3 ("CardSet::update () const");

   for (std::vector<CardWidget*>::const_iterator i (cards_.begin ());
        i != cards_.end (); ++i)
      (*i)->update ();
}

//-----------------------------------------------------------------------------
/// Removes all cards from the set
//-----------------------------------------------------------------------------
void CardSet::clear () {
   for (std::vector<CardWidget*>::iterator i (cards_.begin ());
        i != cards_.end (); ++i)
      delete *i;
   cards_.clear ();
}
