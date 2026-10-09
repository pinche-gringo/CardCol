// PROJECT     : Cardgames
// SUBSYSTEM   : Buraco
// REFERENCES  :
// TODO        :
// BUGS        :
// AUTHOR      : Markus Schwab
// CREATED     : 27.09.2003
// COPYRIGHT   : Copyright (C) 2003 - 2005, 2007, 2008, 2010, 2026

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

#include <YGP/Check.h>
#include <YGP/Trace.h>

#include <card/Widget.h>

#include "BuracoPile.h"

//-----------------------------------------------------------------------------
/// Constructor
//-----------------------------------------------------------------------------
BuracoPile::BuracoPile() : Card::VPile(COMPRESSED, SHOWFACE), status() {}

//-----------------------------------------------------------------------------
/// Destructor
//-----------------------------------------------------------------------------
BuracoPile::~BuracoPile() = default;

//----------------------------------------------------------------------------
/// Sets a new top card of the pile.
/// \param newCard Card to set as uppermost card of the pile
/// \pre \c newCard must be a valid card
//----------------------------------------------------------------------------
void BuracoPile::setTopCard(Card::Widget& newCard) {
    TRACE9("BuracoPile::setTopCard(Card::Widget&) - " << newCard);
    Card::VPile::setTopCard(newCard);
    analysePile();
}

//----------------------------------------------------------------------------
/// Inserts a card into the pile.
/// \param card Card to insert into the pile
/// \param pos Zero-based offset of where to insert the card
/// \pre \c newCard must be a valid card
/// \returns unsigned int Position where card was inserted
//----------------------------------------------------------------------------
unsigned int BuracoPile::insert(Card::Widget& card, unsigned int pos) {
    TRACE9("BuracoPile::insert(Card::Widget&, unsigned int) - " << card << " to " << pos);
    unsigned int rc(Card::VPile::insert(card, pos));
    analysePile();
    return rc;
}

//----------------------------------------------------------------------------
/// Removes the passed card from the pile.
/// \param card Card to remove from the pile
/// \pre \c newCard must be a valid card
//----------------------------------------------------------------------------
Card::Widget& BuracoPile::remove(Card::Widget& card) {
    Card::Widget& rcard(Card::VPile::remove(card));
    analysePile();
    return rcard;
}

//----------------------------------------------------------------------------
/// Removes the passed card from the pile.
/// \param card Card to remove from the pile
/// \param pos Zero-based offset of where to insert the card
/// \pre \c newCard must be a valid card
//----------------------------------------------------------------------------
Card::Widget& BuracoPile::remove(Card::Widget& card, bool visible) {
    Card::Widget& rcard(Card::VPile::remove(card, visible));
    analysePile();
    return rcard;
}

//----------------------------------------------------------------------------
/// Removes the passed card from the pile.
/// \param pos Zero-based offset of card to remove
/// \pre \c newCard must be a valid card
//----------------------------------------------------------------------------
Card::Widget& BuracoPile::remove(unsigned int pos) {
    TRACE9("BuracoPile::remove(unsigned int) - " << pos);
    Card::Widget& card(Card::VPile::remove(pos));
    analysePile();
    return card;
}

//----------------------------------------------------------------------------
/// Removes the passed card from the pile.
/// \param pos Zero-based offset of card to remove
/// \param visible Flag if the card face should be shown
/// \pre \c newCard must be a valid card
//----------------------------------------------------------------------------
Card::Widget& BuracoPile::remove(unsigned int pos, bool visible) {
    Card::Widget& card(Card::VPile::remove(pos, visible));
    analysePile();
    return card;
}
