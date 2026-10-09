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

// Tests of the values of cards and the algorithms on them

#define BOOST_TEST_MODULE Card
#include <boost/test/unit_test.hpp>

#include <set>

#include "TestUtil.h"

using Card::Value;
using Test::cards;

BOOST_AUTO_TEST_SUITE(value)

BOOST_AUTO_TEST_CASE(ids_match_the_images_of_the_deck) {
    // The images start with the aces (clubs, spades, hearts, diamonds)
    BOOST_TEST(Value(0).is(Value::CLUBS, Value::ACE));
    BOOST_TEST(Value(1).is(Value::SPADES, Value::ACE));
    BOOST_TEST(Value(2).is(Value::HEARTS, Value::ACE));
    BOOST_TEST(Value(3).is(Value::DIAMONDS, Value::ACE));
    BOOST_TEST(Value(51).is(Value::DIAMONDS, Value::TWO));
    BOOST_TEST(Value(48).is(Value::CLUBS, Value::TWO));
}

BOOST_AUTO_TEST_CASE(of_is_the_inverse_of_colour_and_number) {
    std::set<unsigned int> ids;
    for (unsigned int c(Value::CLUBS); c <= Value::HEARTS; ++c)
        for (unsigned int n(Value::TWO); n <= Value::ACE; ++n) {
            const Value card(Value::of(static_cast<Value::COLOURS>(c), static_cast<Value::NUMBERS>(n)));
            BOOST_TEST(card.colour() == c);
            BOOST_TEST(card.number() == n);
            BOOST_TEST(!card.isJoker());
            ids.insert(card.id());
        }
    BOOST_TEST(ids.size() == Value::CARDS_PER_DECK);
    BOOST_TEST(*ids.rbegin() == Value::CARDS_PER_DECK - 1);
}

BOOST_AUTO_TEST_CASE(jokers) {
    BOOST_TEST(Value(52).isJoker());
    BOOST_TEST(Value(53).number() == Value::UNREACHABLE);
    std::ostringstream out;
    out << Value(52) << ' ' << Test::card("SQ") << ' ' << Test::card("H10");
    BOOST_TEST(out.str() == "Joker SQ HT");
}

BOOST_AUTO_TEST_SUITE_END()

BOOST_AUTO_TEST_SUITE(algorithms)

BOOST_AUTO_TEST_CASE(linear_searches) {
    const Card::Cards hand(cards("C2 D5 S5 HQ SQ"));
    BOOST_TEST(Card::find(hand, Value::FIVE) == 1);
    BOOST_TEST(Card::find(hand, Value::FIVE, 2) == 2);
    BOOST_TEST(Card::find(hand, Value::FIVE, 3) == -1);
    BOOST_TEST(Card::find(hand, Value::SPADES) == 2);
    BOOST_TEST(Card::find(hand, Value::SPADES, Value::QUEEN) == 4);
    BOOST_TEST(Card::findID(hand, Test::card("HQ").id()) == 3);
    BOOST_TEST(Card::exists(hand, Value::HEARTS));
    BOOST_TEST(!Card::exists(hand, Value::ACE));
    BOOST_TEST(Card::count(hand, Value::SPADES) == 2u);
    BOOST_TEST(Card::count(hand, Value::QUEEN) == 2u);
}

BOOST_AUTO_TEST_CASE(lowest_card) {
    BOOST_TEST(Card::findLowestCard(cards("HK D5 C3 S3")) == 2u);
    BOOST_TEST(Card::findLowestCard(cards("HK D5 C3 S3"), Value::CLUBS) == 3u);
    // If the first card is excluded and nothing is lower, its position is returned
    BOOST_TEST(Card::findLowestCard(cards("C2 D5"), Value::CLUBS) == 0u);
}

BOOST_AUTO_TEST_CASE(searches_in_cards_sorted_by_number) {
    const Card::Cards hand(cards("C2 D4 S4 H4 CK"));
    BOOST_TEST(Card::findFirstEqualOrBigger(hand, Value::THREE) == 1);
    BOOST_TEST(Card::findFirstEqualOrBigger(hand, Value::FOUR) == 1);
    BOOST_TEST(Card::findFirstEqualOrBigger(hand, Value::ACE) == -1);
    BOOST_TEST(Card::findLastEqual(hand, 1) == 3u);
    BOOST_TEST(Card::findFirstEqual(hand, 3) == 1u);
    BOOST_TEST(Card::findLastEqual(hand, 4) == 4u);
    BOOST_TEST(Card::findFirstEqual(hand, 0) == 0u);
}

BOOST_AUTO_TEST_CASE(searches_in_cards_sorted_by_colour) {
    const Card::Cards hand(Test::sortedByColour(cards("HA C2 S3 C9 SK")));
    BOOST_TEST(hand == cards("C2 C9 S3 SK HA"));
    BOOST_TEST(Card::findFirstEqualOrBiggerColour(hand, Value::DIAMONDS) == 2);
    BOOST_TEST(Card::findLastEqualOrBiggerColour(hand, Value::SPADES) == 3);
    BOOST_TEST(Card::findLastEqualOrBiggerColour(hand, Value::DIAMONDS) == -1);
    BOOST_TEST(Card::findFirstEqualColour(hand, 3) == 2u);
    BOOST_TEST(Card::findLastEqualColour(hand, 0) == 1u);
}

BOOST_AUTO_TEST_CASE(deck) {
    const Card::Cards deck(Card::createDeck(2, 2));
    BOOST_TEST(deck.size() == 108u);
    BOOST_TEST(Card::count(deck, Value::HEARTS) == 26u);
    BOOST_TEST(std::ranges::count_if(deck, [](const Value& c) { return c.isJoker(); }) == 4);
}

BOOST_AUTO_TEST_SUITE_END()

BOOST_AUTO_TEST_SUITE(random_engine)

BOOST_AUTO_TEST_CASE(seeding_replays_the_shuffle) {
    const Card::Cards first(Test::shuffledDeck(42));
    const unsigned int number(Card::randomNumber(1000));
    BOOST_TEST(Card::randomSeed() == 42u);

    BOOST_TEST(Test::shuffledDeck(42) == first);
    BOOST_TEST(Card::randomNumber(1000) == number);
    BOOST_TEST(Test::shuffledDeck(43) != first);
    BOOST_TEST(Test::sameCards(first, Card::createDeck()));
}

BOOST_AUTO_TEST_SUITE_END()
