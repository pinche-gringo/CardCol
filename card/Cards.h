#ifndef CARD_CARDS_H
#define CARD_CARDS_H

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

// Algorithms on sequences of cards, which don't depend on their display.
//
// They work on every random access range, whose elements can be converted
// to a Card::Value by an (ADL-found) function value(), like Card::Cards
// (std::vector<Card::Value>) or the Card::Widget pointers of a Card::IPile.
// Positions are returned as int with -1 meaning "not found", like the
// methods of Card::IPile.

#include <algorithm>
#include <ranges>

#include <card/Random.h>
#include <card/Value.h>

namespace Card {

/// \name Comparison of cards
//@{
/// Orders cards by colour and inside the colour by number
constexpr bool lessByColour(const Value& a, const Value& b) {
    return (a.colour() == b.colour()) ? (a.number() < b.number()) : (a.colour() < b.colour());
}
/// Orders cards by number only
constexpr bool lessByNumber(const Value& a, const Value& b) { return a.number() < b.number(); }
/// Orders cards by their ID
constexpr bool lessByID(const Value& a, const Value& b) { return a.id() < b.id(); }
//@}

/// \name Linear searches
//@{
template <class R> int find(const R& cards, Value::NUMBERS nr, unsigned int start = 0) {
    for (; start < std::ranges::size(cards); ++start)
        if (value(cards[start]).number() == nr)
            return start;
    return -1;
}

template <class R> int find(const R& cards, Value::COLOURS colour, unsigned int start = 0) {
    for (; start < std::ranges::size(cards); ++start)
        if (value(cards[start]).colour() == colour)
            return start;
    return -1;
}

template <class R> int findID(const R& cards, unsigned int id, unsigned int start = 0) {
    for (; start < std::ranges::size(cards); ++start)
        if (value(cards[start]).id() == id)
            return start;
    return -1;
}

template <class R> int find(const R& cards, Value::COLOURS colour, Value::NUMBERS nr, unsigned int start = 0) {
    for (; start < std::ranges::size(cards); ++start)
        if (value(cards[start]).is(colour, nr))
            return start;
    return -1;
}

template <class R> bool exists(const R& cards, Value::NUMBERS nr) { return find(cards, nr) != -1; }
template <class R> bool exists(const R& cards, Value::COLOURS colour) { return find(cards, colour) != -1; }
template <class R> bool exists(const R& cards, Value::COLOURS colour, Value::NUMBERS nr) {
    return find(cards, colour, nr) != -1;
}

template <class R> unsigned int count(const R& cards, Value::COLOURS colour) {
    return std::ranges::count_if(cards, [colour](const auto& c) { return value(c).colour() == colour; });
}
template <class R> unsigned int count(const R& cards, Value::NUMBERS nr) {
    return std::ranges::count_if(cards, [nr](const auto& c) { return value(c).number() == nr; });
}

/// Returns the position of the (first) card with the lowest number
/// \pre The cards must not be empty
template <class R> unsigned int findLowestCard(const R& cards) {
    unsigned int pos(0);
    for (unsigned int i(1); i < std::ranges::size(cards); ++i)
        if (value(cards[i]).number() < value(cards[pos]).number())
            pos = i;
    return pos;
}

/// Returns the position of the (first) card with the lowest number, ignoring
/// cards of the passed colour (if the first card has the excluded colour and
/// no lower card exists, 0 is returned)
/// \pre The cards must not be empty
template <class R> unsigned int findLowestCard(const R& cards, Value::COLOURS excludeColour) {
    unsigned int pos(0);
    for (unsigned int i(1); i < std::ranges::size(cards); ++i)
        if ((value(cards[i]).colour() != excludeColour) && (value(cards[i]).number() < value(cards[pos]).number()))
            pos = i;
    return pos;
}
//@}

/// \name Searches in sorted cards
//@{
/// Returns the position of the first card with a number equal or bigger than
/// the passed one or -1
/// \pre The cards must be sorted by number
template <class R> int findFirstEqualOrBigger(const R& cards, Value::NUMBERS nr) {
    auto i(std::ranges::partition_point(cards, [nr](const auto& c) { return value(c).number() < nr; }));
    return (i == std::ranges::end(cards)) ? -1 : static_cast<int>(i - std::ranges::begin(cards));
}

/// Returns the position of the first card with a colour equal or bigger than
/// the passed one or -1
/// \pre The cards must be sorted by colour
template <class R> int findFirstEqualOrBiggerColour(const R& cards, Value::COLOURS colour) {
    auto i(std::ranges::partition_point(cards, [colour](const auto& c) { return value(c).colour() < colour; }));
    return (i == std::ranges::end(cards)) ? -1 : static_cast<int>(i - std::ranges::begin(cards));
}

/// Returns the position of the last card having the same number as the one at
/// the passed position (searching only towards the end)
template <class R> unsigned int findLastEqual(const R& cards, unsigned int pos) {
    const Value::NUMBERS nr(value(cards[pos]).number());
    while ((++pos < std::ranges::size(cards)) && (value(cards[pos]).number() == nr))
        ;
    return pos - 1;
}

/// Returns the position of the last card having the same colour as the one at
/// the passed position (searching only towards the end)
template <class R> unsigned int findLastEqualColour(const R& cards, unsigned int pos) {
    const Value::COLOURS colour(value(cards[pos]).colour());
    while ((++pos < std::ranges::size(cards)) && (value(cards[pos]).colour() == colour))
        ;
    return pos - 1;
}

/// Returns the position of the first card having the same number as the one
/// at the passed position (searching only towards the start)
template <class R> unsigned int findFirstEqual(const R& cards, unsigned int pos) {
    const Value::NUMBERS nr(value(cards[pos]).number());
    while (pos && (value(cards[pos - 1]).number() == nr))
        --pos;
    return pos;
}

/// Returns the position of the first card having the same colour as the one
/// at the passed position (searching only towards the start)
template <class R> unsigned int findFirstEqualColour(const R& cards, unsigned int pos) {
    const Value::COLOURS colour(value(cards[pos]).colour());
    while (pos && (value(cards[pos - 1]).colour() == colour))
        --pos;
    return pos;
}

/// Returns the position of the last card of the passed colour or -1
/// \pre The cards must be sorted by colour
template <class R> int findLastEqualOrBiggerColour(const R& cards, Value::COLOURS colour) {
    int pos(findFirstEqualOrBiggerColour(cards, colour));
    return ((pos == -1) || (value(cards[pos]).colour() != colour)) ? -1 : static_cast<int>(findLastEqualColour(cards, pos));
}
//@}

/// Converts the passed cards to plain values
template <class R> Cards values(const R& cards) {
    Cards result;
    result.reserve(std::ranges::size(cards));
    for (const auto& card : cards)
        result.push_back(value(card));
    return result;
}

/// Shuffles the passed cards (using the engine of card/Random.h)
inline void shuffle(Cards& cards) { std::ranges::shuffle(cards, randomEngine()); }

/// Creates a (sorted) deck of cards
/// \param decks Number of decks
/// \param jokers Number of jokers (per deck)
inline Cards createDeck(unsigned int decks = 1, unsigned int jokers = 0) {
    Cards result;
    for (unsigned int d(0); d < decks; ++d)
        for (unsigned int i(0); i < (Value::CARDS_PER_DECK + jokers); ++i)
            result.emplace_back(i);
    return result;
}

} // namespace Card

#endif
