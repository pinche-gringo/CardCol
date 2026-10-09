#ifndef CARD_VALUE_H
#define CARD_VALUE_H

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

#include <array>
#include <iosfwd>
#include <vector>

namespace Card {

/**Value of a card (colour and number), independent of its display.

   The card is identified by the number of its image in the carddeck: The
   cards 0 - 51 are the normal cards (starting with the aces), 52 and above
   are jokers.
 */
class Value {
  public:
    enum COLOURS { CLUBS = 0, DIAMONDS, SPADES, HEARTS };
    enum NUMBERS { TWO = 0, THREE, FOUR, FIVE, SIX, SEVEN, EIGHT, NINE, TEN, JACK, QUEEN, KING, ACE, UNREACHABLE };

    /// Number of cards (without jokers) in a deck
    static constexpr unsigned int CARDS_PER_DECK = 52;

    constexpr explicit Value(unsigned int card = 0) : nrCard(card) {}
    /// Creates the value of the card having the passed colour and number
    static constexpr Value of(COLOURS colour, NUMBERS number) {
        return Value((CARDS_PER_DECK - 4) - (static_cast<unsigned int>(number) << 2) + idOfColour[colour]);
    }

    constexpr unsigned int id() const { return nrCard; }
    constexpr COLOURS colour() const { return transColour[nrCard & 0x3]; }
    constexpr NUMBERS number() const {
        return isJoker() ? UNREACHABLE : static_cast<NUMBERS>(((CARDS_PER_DECK - 1) - nrCard) >> 2);
    }
    constexpr bool isJoker() const { return nrCard >= CARDS_PER_DECK; }
    constexpr bool is(COLOURS col, NUMBERS nr) const { return (colour() == col) && (number() == nr); }

    char numberStr() const { return strNumber(number()); }
    char colourStr() const { return strColour(colour()); }

    static char strNumber(NUMBERS nr);
    static char strColour(COLOURS col);

    constexpr bool operator==(const Value& other) const = default;

    friend std::ostream& operator<<(std::ostream& out, const Value& card);

  protected:
    unsigned int nrCard;

  private:
    static constexpr std::array<COLOURS, 4> transColour{CLUBS, SPADES, HEARTS, DIAMONDS};
    static constexpr std::array<unsigned int, 4> idOfColour{0, 3, 1, 2};
};

/// Cards as plain values (without any display)
using Cards = std::vector<Value>;

/// Prints the cards (separated by blanks)
std::ostream& operator<<(std::ostream& out, const Cards& cards);

/// Accessor to the value of a card; overloaded for every type holding cards,
/// to enable the algorithms in card/Cards.h for it
constexpr const Value& value(const Value& card) { return card; }

} // namespace Card

#endif
