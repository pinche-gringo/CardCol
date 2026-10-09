// PROJECT     : Cardgames
// SUBSYSTEM   : Machiavelli
// REFERENCES  :
// TODO        :
// BUGS        :
// AUTHOR      : Markus Schwab
// CREATED     : 05.11.2003
// COPYRIGHT   : Copyright (C) 2003 - 2006, 2008, 2009, 2024, 2026

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

#include <cardgames-cfg.h>

#include <YGP/ANumeric.h>
#include <YGP/Check.h>
#include <YGP/Trace.h>

#include "MachiPile.h"

//-----------------------------------------------------------------------------
/// Default constructor
//-----------------------------------------------------------------------------
MachiPile::MachiPile() : Card::HPile(COMPRESSED, SHOWFACE), type(MachiavelliRules::UNDEFINED) {}

//-----------------------------------------------------------------------------
/// Destructor
//-----------------------------------------------------------------------------
MachiPile::~MachiPile() = default;

//----------------------------------------------------------------------------
/// Sets a new top card of the pile.
/// \param newCard Card to set as uppermost card of the pile
/// \pre \c newCard must be a valid card
//----------------------------------------------------------------------------
void MachiPile::setTopCard(Card::Widget& newCard) {
    TRACE9("MachiPile::setTopCard(Card::Widget&) - " << newCard);
    Card::HPile::setTopCard(newCard);
    analysePile();
}

//----------------------------------------------------------------------------
/// Inserts a card into the pile.
/// \param card Card to insert into the pile
/// \param pos Zero-based offset of where to insert the card
/// \pre \c newCard must be a valid card
/// \returns unsigned int Position where card was inserted
//----------------------------------------------------------------------------
unsigned int MachiPile::insert(Card::Widget& card, unsigned int pos) {
    TRACE9("MachiPile::insert(Card::Widget&, unsigned int) - " << card << " to " << pos);
    unsigned int rc(Card::HPile::insert(card, pos));
    analysePile();
    return rc;
}

//----------------------------------------------------------------------------
/// Removes the passed card from the pile.
/// \param card Card to remove from the pile
/// \param pos Zero-based offset of where to insert the card
/// \pre \c newCard must be a valid card
//----------------------------------------------------------------------------
Card::Widget& MachiPile::remove(Card::Widget& card) {
    Card::Widget& rcard(Card::HPile::remove(card));
    analysePile();
    return rcard;
}

//----------------------------------------------------------------------------
/// Removes the passed card from the pile.
/// \param card Card to remove from the pile
/// \param pos Zero-based offset of where to insert the card
/// \pre \c newCard must be a valid card
//----------------------------------------------------------------------------
Card::Widget& MachiPile::remove(Card::Widget& card, bool visible) {
    Card::Widget& rcard(Card::HPile::remove(card, visible));
    analysePile();
    return rcard;
}

//----------------------------------------------------------------------------
/// Removes the passed card from the pile.
/// \param card Card to remove from the pile
/// \param pos Zero-based offset of where to insert the card
/// \pre \c newCard must be a valid card
//----------------------------------------------------------------------------
Card::Widget& MachiPile::remove(unsigned int pos) {
    TRACE9("MachiPile::remove(unsigned int) - " << pos);
    Card::Widget& card(Card::HPile::remove(pos));
    analysePile();
    return card;
}

//----------------------------------------------------------------------------
/// Removes the passed card from the pile.
/// \param card Card to remove from the pile
/// \param pos Zero-based offset of where to insert the card
/// \pre \c newCard must be a valid card
//----------------------------------------------------------------------------
Card::Widget& MachiPile::remove(unsigned int pos, bool visible) {
    Card::Widget& card(Card::HPile::remove(pos, visible));
    analysePile();
    return card;
}

//----------------------------------------------------------------------------
/// Analyses the pile and stores its characteristics
//----------------------------------------------------------------------------
void MachiPile::analysePile() { type = MachiavelliRules::analysePile(values(), type); }

//----------------------------------------------------------------------------
/// Checks the integrity of the object
/// \throw PileError In case of an invalid pile (with a describing message)
//----------------------------------------------------------------------------
void MachiPile::checkIntegrity() const {
    unsigned int pos(0);
    const MachiavelliRules::PileError rc(MachiavelliRules::checkPile(rules(), pos));
    if (rc != MachiavelliRules::PileError::NONE) {
        Glib::ustring error(_(MachiavelliRules::describe(rc)));
        if (rc == MachiavelliRules::PileError::CARD_DOES_NOT_FIT)
            error.replace(error.find("%1"), 2, YGP::ANumeric::toString(pos));
        throw PileError(error);
    }
}

//----------------------------------------------------------------------------
/// Marks all cards in the pile
//----------------------------------------------------------------------------
void MachiPile::mark() const {
    for (auto i : *this)
        i->mark();
}

//----------------------------------------------------------------------------
/// Unmarks all cards in the pile
//----------------------------------------------------------------------------
void MachiPile::unmark() const {
    for (auto i : *this)
        i->unmark();
}

//----------------------------------------------------------------------------
/// Unmarks all cards in the pile
//----------------------------------------------------------------------------
void MachiPile::markValidity() const {
    try {
        checkIntegrity();
        unmark();
    }
    catch (PileError&) {
        mark();
    }
}
