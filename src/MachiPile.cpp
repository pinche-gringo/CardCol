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
}
