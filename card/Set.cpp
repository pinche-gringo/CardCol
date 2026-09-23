// PROJECT     : Cardgames
// SUBSYSTEM   : Common
// REFERENCES  :
// TODO        :
// BUGS        :
// AUTHOR      : Markus Schwab
// CREATED     : 8.5.2002
// COPYRIGHT   : Copyright (C) 2002 - 2018, 2026

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

#include <cstdlib>

#include <YGP/Check.h>
#include <YGP/Trace.h>

#include "Images.h"
#include "Widget.h"

#include "Set.h"

namespace Card {

//-----------------------------------------------------------------------------
/// Destructor
//-----------------------------------------------------------------------------
Set::~Set() {
    TRACE9("Set::~Set()");
    clear();
}

//-----------------------------------------------------------------------------
/// Adds a set of cards (with images specified by decks) to set
/// \param decks Class holding the images to add
//-----------------------------------------------------------------------------
void Set::addPacket(const Images& decks) {
    TRACE9("Set::addPacket(const Images&)");

    Widget::setDeck(decks);
    for (unsigned int i(0); i < decks.size(); ++i) {
        Widget* card(new Widget(i, true));
        Check3(card);
        card->show();
        cards_.push_back(card);
    } // endfor
}

//-----------------------------------------------------------------------------
/// Shuffles the cards in the deck
//-----------------------------------------------------------------------------
void Set::shuffle() {
    TRACE2("Set::shuffle()");

    unsigned int nr;
    for (int i(size()); i > 0;) {
        nr = rand() % i--;
        TRACE2("Set::shuffle() - " << i << " = " << nr);
        std::swap(cards_[i], cards_[nr]);
    }
}

//-----------------------------------------------------------------------------
/// Sets the specified card in the passed slot of the set
/// \param pos Slot (position) of the card to set
/// \param card Card to swap
/// \pre The \c card must not be in a position before \c pos.
//-----------------------------------------------------------------------------
void Set::set(unsigned int pos, unsigned int card) {
    TRACE2("Set::set(unsigned int, unsigned int) - [" << pos << "] = " << card);
    Check1(card < cards_.size());

    for (std::vector<Widget*>::iterator i(cards_.begin() + pos); i != cards_.end(); ++i)
        if ((*i)->id() == card) {
            std::vector<Widget*>::iterator t(cards_.begin() + pos);
            std::swap(*t, *i);
            return;
        }
    Check3(0);
}

//-----------------------------------------------------------------------------
/// Retrieves the specified card
/// \returns Widget& Reference to Widget
//-----------------------------------------------------------------------------
Widget& Set::getCard(unsigned int nr) const {
    TRACE3("Set::getCard(unsigned int) - " << nr);
    Check3(nr < size());
    Check3(cards_[nr]);

    return *cards_[nr];
}

//-----------------------------------------------------------------------------
/// Actualises the card set (after changes of the images)
//-----------------------------------------------------------------------------
void Set::update() const {
    TRACE3("Set::update() const");

    for (std::vector<Widget*>::const_iterator i(cards_.begin()); i != cards_.end(); ++i)
        (*i)->update();
}

//-----------------------------------------------------------------------------
/// Removes all cards from the set
//-----------------------------------------------------------------------------
void Set::clear() {
    for (std::vector<Widget*>::iterator i(cards_.begin()); i != cards_.end(); ++i)
        delete *i;
    cards_.clear();
}

} // namespace Card
