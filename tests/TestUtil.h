#ifndef TESTUTIL_H
#define TESTUTIL_H

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

// Helpers for the tests of the rules

#include <algorithm>
#include <cstdlib>
#include <map>
#include <sstream>
#include <stdexcept>
#include <string>
#include <string_view>

#include <card/Cards.h>
#include <card/Random.h>

namespace Test {

/// Creates a card out of its description: The colour (C, D, S, H) followed
/// by the number (2 - 9, T or 10, J, Q, K, A); "Jo" is a joker
inline Card::Value card(std::string_view text) {
    if (text == "Jo")
        return Card::Value(Card::Value::CARDS_PER_DECK);
    if (text.size() < 2)
        throw std::invalid_argument("Invalid card " + std::string(text));

    static const std::string colours("CDSH");
    static const std::string numbers("23456789TJQKA");
    const auto colour(colours.find(text[0]));
    const auto number((text.substr(1) == "10") ? 8 : ((text.size() == 2) ? numbers.find(text[1]) : std::string::npos));
    if ((colour == std::string::npos) || (number == std::string::npos))
        throw std::invalid_argument("Invalid card " + std::string(text));
    return Card::Value::of(static_cast<Card::Value::COLOURS>(colour), static_cast<Card::Value::NUMBERS>(number));
}

/// Creates cards out of their (blank-separated) descriptions, like "C2 SQ HA"
inline Card::Cards cards(std::string_view text) {
    Card::Cards result;
    std::istringstream in{std::string(text)};
    std::string word;
    while (in >> word)
        result.push_back(card(word));
    return result;
}

/// Returns the passed cards sorted by colour
inline Card::Cards sortedByColour(Card::Cards cards) {
    std::ranges::sort(cards, Card::lessByColour);
    return cards;
}

/// Checks if the passed cards are a permutation of the other ones
inline bool sameCards(Card::Cards a, Card::Cards b) {
    std::ranges::sort(a, Card::lessByID);
    std::ranges::sort(b, Card::lessByID);
    return a == b;
}

/// Checks if the passed cards are a subset of the other ones (respecting
/// duplicates)
inline bool containsAll(const Card::Cards& cards, const Card::Cards& subset) {
    std::map<unsigned int, int> count;
    for (const auto& c : cards)
        ++count[c.id()];
    for (const auto& c : subset)
        if (--count[c.id()] < 0)
            return false;
    return true;
}

/// Removes the passed card from the cards
/// \returns bool True, if the card has been found
inline bool removeCard(Card::Cards& cards, const Card::Value& card) {
    auto i(std::ranges::find(cards, card));
    if (i == cards.end())
        return false;
    cards.erase(i);
    return true;
}

/// Returns an unsigned value from the environment (or the passed default)
inline unsigned int fromEnvironment(const char* name, unsigned int defaultValue) {
    const char* value(std::getenv(name));
    return (value && *value) ? static_cast<unsigned int>(std::strtoul(value, nullptr, 0)) : defaultValue;
}

/// Seeds the simulations play: CARDCOL_SIM_GAMES games (default: 200),
/// starting with CARDCOL_SIM_FIRST_SEED (default: 1)
struct Seeds {
    unsigned int first;
    unsigned int count;

    Seeds()
        : first(fromEnvironment("CARDCOL_SIM_FIRST_SEED", 1)), count(fromEnvironment("CARDCOL_SIM_GAMES", 200)) {}

    unsigned int end() const { return first + count; }
};

/// Shuffles a new deck, after initialising the random engine with the seed
inline Card::Cards shuffledDeck(unsigned int seed, unsigned int decks = 1, unsigned int jokers = 0) {
    Card::seedRandom(seed);
    Card::Cards deck(Card::createDeck(decks, jokers));
    Card::shuffle(deck);
    return deck;
}

} // namespace Test

#endif
