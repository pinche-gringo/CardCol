// PROJECT     : Cardgames
// SUBSYSTEM   : libCard
// REFERENCES  :
// TODO        :
// BUGS        :
// AUTHOR      : Markus Schwab
// CREATED     : 02.08.2006
// COPYRIGHT   : Copyright (C) 2006, 2007, 2026

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

#include "card/Widget.h"

#include "CardValue.h"

CardValue* CardValue::instance(NULL);

//-----------------------------------------------------------------------------
/// Default constructor; Fills the values
//-----------------------------------------------------------------------------
CardValue::CardValue() {
    for (unsigned nr(Card::Widget::THREE); nr < Card::Widget::TEN; ++nr)
        insert(std::make_pair(nr, std::string(1, Card::Widget::strNumber(static_cast<Card::Widget::NUMBERS>(nr)))));
    insert(std::make_pair(Card::Widget::TEN, _("10")));
    insert(std::make_pair(Card::Widget::JACK, _("Jack")));
    insert(std::make_pair(Card::Widget::QUEEN, _("Queen")));
    insert(std::make_pair(Card::Widget::KING, _("King")));
    insert(std::make_pair(Card::Widget::ACE, _("Ace")));
}

//-----------------------------------------------------------------------------
/// Destructor
//-----------------------------------------------------------------------------
CardValue::~CardValue() {}
