//$Id$

//PROJECT     : Cardgames
//SUBSYSTEM   : MachiavelliPile
//REFERENCES  :
//TODO        :
//BUGS        :
//REVISION    : $Revision$
//AUTHOR      : Markus Schwab
//CREATED     : 05.11.2003
//COPYRIGHT   : Anticopyright (A) 2003

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


#define CHECK 9
#define TRACELEVEL 9
#include <Check.h>
#include <Trace_.h>

#include "MachiPile.h"


//-----------------------------------------------------------------------------
/// Default constructor
//-----------------------------------------------------------------------------
MachiPile::MachiPile () : CardHPile (COMPRESSED, SHOWFACE) {
   type = UNDEFINED;
}

//-----------------------------------------------------------------------------
/// Destructor
//-----------------------------------------------------------------------------
MachiPile::~MachiPile () {
}


//----------------------------------------------------------------------------
/// Sets a new top card of the pile.
/// \param newCard: Card to set as uppermost card of the pile
/// \pre \c newCard must be a valid card
//----------------------------------------------------------------------------
void MachiPile::setTopCard (CardWidget& newCard) {
   TRACE9 ("MachiPile::setTopCard (CardWidget&) - " << newCard);
   CardHPile::setTopCard (newCard);
   analyzePile ();
}

//----------------------------------------------------------------------------
/// Inserts a card into the pile.
/// \param card: Card to insert into the pile
/// \param pos: Zero-based offset of where to insert the card
/// \pre \c newCard must be a valid card
//----------------------------------------------------------------------------
void MachiPile::insert (CardWidget& card, unsigned int pos) {
   TRACE9 ("MachiPile::insert (CardWidget&, unsigned int) - " << card
           << " to " << pos);
   CardHPile::insert (card, pos);
   analyzePile ();
}


//----------------------------------------------------------------------------
/// Removes the passed card from the pile.
/// \param card: Card to remove from the pile
/// \param pos: Zero-based offset of where to insert the card
/// \pre \c newCard must be a valid card
//----------------------------------------------------------------------------
CardWidget& MachiPile::remove (CardWidget& card) {
   CardWidget& rcard (CardHPile::remove (card));
   analyzePile ();
   return rcard;
}

//----------------------------------------------------------------------------
/// Removes the passed card from the pile.
/// \param card: Card to remove from the pile
/// \param pos: Zero-based offset of where to insert the card
/// \pre \c newCard must be a valid card
//----------------------------------------------------------------------------
CardWidget& MachiPile::remove (CardWidget& card, bool visible) {
   CardWidget& rcard (CardHPile::remove (card, visible));
   analyzePile ();
   return rcard;
}

//----------------------------------------------------------------------------
/// Removes the passed card from the pile.
/// \param card: Card to remove from the pile
/// \param pos: Zero-based offset of where to insert the card
/// \pre \c newCard must be a valid card
//----------------------------------------------------------------------------
CardWidget& MachiPile::remove (unsigned int pos) {
   TRACE9 ("MachiPile::remove (unsigned int) - " << pos);
   CardWidget& card (CardHPile::remove (pos));
   analyzePile ();
   return card;
}

//----------------------------------------------------------------------------
/// Removes the passed card from the pile.
/// \param card: Card to remove from the pile
/// \param pos: Zero-based offset of where to insert the card
/// \pre \c newCard must be a valid card
//----------------------------------------------------------------------------
CardWidget& MachiPile::remove (unsigned int pos, bool visible) {
   CardWidget& card (CardHPile::remove (pos, visible));
   analyzePile ();
   return card;
}

//----------------------------------------------------------------------------
/// Returns the position in the pile where the card can be played to.
/// - Card played on an empty pile -> Valid
/// - Check if the card "fits": Either the same number as the other (first and
///   last) card, or the same colour and the number in serie.
/// \param card: Card to inspect. 
/// \returns unsigned int: Position of card in pile or -1U
/// \pre Coloured piles must be sorted strict ascending
//----------------------------------------------------------------------------
unsigned int MachiPile::getPosition4Card (const CardWidget& card) const {
   TRACE1 ("MachiPile::getPosition4Card (const CardWidget&) const - " << card);
   Check1 (size ());

   // First test numbered piles (btw. undefined piles)
   if (type != COLOUR) {
      if ((operator[] (0)->number () == card.number ())
          && (find (card.id ()) == -1))
         return size ();
      else if (type == NUMBER)
         return -1U;
   }

   // Now check for matching colour
   TRACE9 ("MachiPile::getPosition4Card (const CardWidget&) const - Checking colour");
   if (operator[] (0)->colour () == card.colour ()) {
      if (cardDistance (card, *operator[] (0)) == -1)
         return 0;
      
      if (cardDistance (card, *operator[] (size () - 1)) == 1)
         return size ();
   }

   return -1U;
}

//----------------------------------------------------------------------------
/// Returns the distance between two cards. The ace also counts as one (if the
/// other card is a 2 or a 3)
/// \param a: Card to compare
/// \param b: Card to compare
/// \param aceIsOne: Flag, if aces should (also) be treated as one
/// \returns \c int: Distance of the two passed cards (a - b)
//----------------------------------------------------------------------------
int MachiPile::cardDistance (const CardWidget& a, const CardWidget& b, bool aceIsOne) {
   TRACE9 ("MachiPile::cardDistance (const CardWidget&, const CardWidget&, bool) - "
           << a << "<->" << b);
   
   if (aceIsOne) {                        // Special handling of the ace like 1
      TRACE9 ("MachiPile::cardDistance (const CardWidget&, const CardWidget&) - "
               "Checking for Ace");
      if ((a.number () == CardWidget::ACE)
          && (b.number () < CardWidget::FOUR))
         return 1 - static_cast<int> (b.number ());
      else if ((b.number () == CardWidget::ACE)
               && (a.number () < CardWidget::FOUR))
         return static_cast<int> (a.number ()) - 1;
   }

   TRACE9 ("MachiPile::cardDistance (const CardWidget&, const CardWidget&, bool) - "
           "Distance: " << a.number () - b.number ());
   return a.number () - b.number ();
}
