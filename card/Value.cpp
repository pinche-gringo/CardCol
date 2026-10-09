// PROJECT     : Cardgames
// SUBSYSTEM   : Common
// REFERENCES  :
// TODO        :
// BUGS        :
// AUTHOR      : Markus Schwab
// CREATED     : 9.10.2026
// COPYRIGHT   : Copyright (C) 2026

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

#include <ostream>
#include <string>

#include <cardgames-cfg.h>

#include "Value.h"

namespace Card {

//-----------------------------------------------------------------------------
/// Returns the character describing the passed number
/// \param nr Number of the card
/// \returns char Character describing the number
//-----------------------------------------------------------------------------
char Value::strNumber(NUMBERS nr) {
    static const std::string specialCards(_("TJQKA"));
    return static_cast<char>((nr >= TEN) ? specialCards[nr - TEN] : nr + '2');
}

//-----------------------------------------------------------------------------
/// Returns the character describing the passed colour
/// \param col Colour of the card
/// \returns char Character describing the colour
//-----------------------------------------------------------------------------
char Value::strColour(COLOURS col) {
    // Letters describing the colours (clubs, diamonds, spades, hearts)
    static const std::string colours(_("CDSH"));
    return colours[col];
}

//-----------------------------------------------------------------------------
/// Prints the card to the passed stream
/// \param out Stream to print to
/// \param card Card to print
/// \returns std::ostream& The passed stream
//-----------------------------------------------------------------------------
std::ostream& operator<<(std::ostream& out, const Value& card) {
    if (card.isJoker())
        out << "Joker";
    else
        out << card.colourStr() << card.numberStr();
    return out;
}

//-----------------------------------------------------------------------------
/// Prints the passed cards (separated by blanks) to the passed stream
/// \param out Stream to print to
/// \param cards Cards to print
/// \returns std::ostream& The passed stream
//-----------------------------------------------------------------------------
std::ostream& operator<<(std::ostream& out, const Cards& cards) {
    for (auto i(cards.begin()); i != cards.end(); ++i)
        out << ((i == cards.begin()) ? "" : " ") << *i;
    return out;
}

} // namespace Card
