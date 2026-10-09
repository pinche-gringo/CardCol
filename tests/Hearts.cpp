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

// Unit tests of the rules of Hearts

#define BOOST_TEST_MODULE Hearts
#include <boost/test/unit_test.hpp>

#include "HeartsRules.h"

#include "TestUtil.h"

using namespace HeartsRules;
using Card::Value;
using Test::card;
using Test::cards;

namespace HeartsRules {
std::ostream& operator<<(std::ostream& out, PlayError error) { return out << static_cast<int>(error); }
} // namespace HeartsRules

namespace {

/// Creates a table, as it is after the first trick (with nothing of interest played)
Table afterFirstTrick() {
    Table table;
    for (const auto& c : cards("C2 C3 C4 C5"))
        table.play(c);
    table.clearTrick();
    return table;
}

/// Creates a table with the passed trick
Table withTrick(Table table, std::string_view trick) {
    for (const auto& c : cards(trick))
        table.play(c);
    return table;
}

} // namespace

BOOST_AUTO_TEST_SUITE(scoring)

BOOST_AUTO_TEST_CASE(points) {
    BOOST_TEST(pointsOf(cards("H2 HA C5")) == 2u);
    BOOST_TEST(pointsOf(cards("SQ")) == 13u);
    BOOST_TEST(pointsOf(cards("SK SA DQ CQ")) == 0u);
    BOOST_TEST(pointsOf(Card::createDeck()) == ALL_POINTS);
}

BOOST_AUTO_TEST_CASE(round_score) {
    const std::array<int, NUM_PLAYERS> normal{3, 0, 13, 10};
    BOOST_TEST(roundScore({3, 0, 13, 10}) == normal);
}

BOOST_AUTO_TEST_CASE(shooting_the_moon) {
    const std::array<int, NUM_PLAYERS> moon{26, 26, 0, 26};
    BOOST_TEST(roundScore({0, 0, 26, 0}) == moon);
}

BOOST_AUTO_TEST_CASE(trick_winner) {
    BOOST_TEST(trickWinner(cards("C5 CK C2 CA")) == 3u);
    // Only the colour played first counts
    BOOST_TEST(trickWinner(cards("C5 HA SA C6")) == 3u);
    BOOST_TEST(trickWinner(cards("D9 HA SA C6")) == 0u);
}

BOOST_AUTO_TEST_CASE(start_player_holds_the_two_of_clubs) {
    std::array<Card::Cards, NUM_PLAYERS> hands{cards("C3 D2"), cards("C4"), cards("C2 H3"), cards("S2")};
    BOOST_TEST(startPlayer(hands) == 2u);
}

BOOST_AUTO_TEST_SUITE_END()

BOOST_AUTO_TEST_SUITE(valid_plays)

BOOST_AUTO_TEST_CASE(game_starts_with_the_two_of_clubs) {
    const Card::Cards hand(cards("C2 C9 D4"));
    BOOST_TEST(checkPlay(hand, 0, Table()) == PlayError::NONE);
    BOOST_TEST(checkPlay(hand, 1, Table()) == PlayError::START_WITH_TWO_OF_CLUBS);
}

BOOST_AUTO_TEST_CASE(colour_must_be_followed) {
    const Card::Cards hand(cards("C9 D4 H5"));
    const Table table(withTrick(afterFirstTrick(), "C7"));
    BOOST_TEST(checkPlay(hand, 0, table) == PlayError::NONE);
    BOOST_TEST(checkPlay(hand, 1, table) == PlayError::FOLLOW_COLOUR);
    BOOST_TEST(checkPlay(cards("D4 H5"), 1, table) == PlayError::NONE);
}

BOOST_AUTO_TEST_CASE(hearts_lead_only_when_broken) {
    const Card::Cards hand(cards("D4 H5"));
    Table table(afterFirstTrick());
    BOOST_TEST(checkPlay(hand, 1, table) == PlayError::NO_HEART_TO_START);
    // ... except if there is nothing else
    BOOST_TEST(checkPlay(cards("H5 HK"), 0, table) == PlayError::NONE);

    table = withTrick(table, "D2 D3 D5 H2");
    table.clearTrick();
    BOOST_TEST(checkPlay(hand, 1, table) == PlayError::NONE);
}

BOOST_AUTO_TEST_CASE(no_points_in_first_round) {
    const Table table(withTrick(Table(), "C2"));
    BOOST_TEST(checkPlay(cards("D4 SQ H5"), 1, table) == PlayError::NO_QUEEN_OF_SPADES_IN_FIRST_ROUND);
    BOOST_TEST(checkPlay(cards("D4 SQ H5"), 2, table) == PlayError::NO_HEART_IN_FIRST_ROUND);
    BOOST_TEST(checkPlay(cards("D4 SQ H5"), 0, table) == PlayError::NONE);
}

BOOST_AUTO_TEST_SUITE_END()

BOOST_AUTO_TEST_SUITE(computer_player)

BOOST_AUTO_TEST_CASE(exchanges_short_colours) {
    // Two clubs are given away completely, then the highest card
    const Card::Cards hand(Test::sortedByColour(cards("C3 C4 D2 D5 D7 D9 S2 S4 S6 S8 ST SJ H3")));
    const Card::Cards exchange(selectCardsToExchange(hand));
    BOOST_TEST(exchange.size() == CARDS_TO_EXCHANGE);
    BOOST_TEST(Test::containsAll(hand, exchange));
    BOOST_TEST(Test::containsAll(exchange, cards("C3 C4")));
}

BOOST_AUTO_TEST_CASE(exchanges_queen_of_spades_if_short_of_spades) {
    const Card::Cards hand(Test::sortedByColour(cards("C3 C4 C5 C6 D2 D5 D7 D9 S4 SQ H3 H4 H5")));
    const Card::Cards exchange(selectCardsToExchange(hand));
    BOOST_TEST(exchange.size() == CARDS_TO_EXCHANGE);
    BOOST_TEST(Test::containsAll(exchange, cards("SQ")));
}

BOOST_AUTO_TEST_CASE(exchanges_with_a_single_spade_after_the_clubs) {
    const Card::Cards hand(Test::sortedByColour(cards("CQ CK S9 H2 H3 H5 H6 H7 H9 HT HJ HQ HK")));
    const Card::Cards exchange(selectCardsToExchange(hand));
    BOOST_TEST(exchange.size() == CARDS_TO_EXCHANGE);
    BOOST_TEST(Test::containsAll(exchange, cards("CQ CK")));
}

BOOST_AUTO_TEST_CASE(exchanges_high_spades_at_the_start_of_the_hand) {
    const Card::Cards hand(Test::sortedByColour(cards("SK SA H2 H3 H4 H5 H6 H7 H8 H9 HT HJ HQ")));
    const Card::Cards exchange(selectCardsToExchange(hand));
    BOOST_TEST(exchange.size() == CARDS_TO_EXCHANGE);
    BOOST_TEST(Test::containsAll(exchange, cards("SK SA HQ")));
}

BOOST_AUTO_TEST_CASE(exchanges_three_distinct_cards_of_every_deal) {
    for (unsigned int seed(1); seed < 500; ++seed) {
        BOOST_TEST_CONTEXT("Seed " << seed) {
            const Card::Cards deck(Test::shuffledDeck(seed));
            const Card::Cards hand(Test::sortedByColour(Card::Cards(deck.begin(), deck.begin() + 13)));
            const Card::Cards exchange(selectCardsToExchange(hand));
            BOOST_TEST(exchange.size() == CARDS_TO_EXCHANGE);
            BOOST_TEST(Test::containsAll(hand, exchange));
        }
    }
}

BOOST_AUTO_TEST_CASE(follows_with_a_lower_card) {
    const Card::Cards hand(cards("D3 D8 DK"));
    const Table table(withTrick(afterFirstTrick(), "D9 H2"));
    BOOST_TEST(hand[selectCardToPlay(hand, table)] == card("D8"));
}

BOOST_AUTO_TEST_CASE(gets_rid_of_the_queen_of_spades_below_a_higher_spade) {
    const Card::Cards hand(cards("S3 SQ"));
    const Table table(withTrick(afterFirstTrick(), "SK"));
    BOOST_TEST(hand[selectCardToPlay(hand, table)] == card("SQ"));
}

BOOST_AUTO_TEST_CASE(throws_the_queen_of_spades_when_void) {
    const Card::Cards hand(cards("D3 SQ H9"));
    const Table table(withTrick(afterFirstTrick(), "C9"));
    BOOST_TEST(hand[selectCardToPlay(hand, table)] == card("SQ"));
}

BOOST_AUTO_TEST_CASE(throws_no_points_in_first_round) {
    const Card::Cards hand(cards("D3 SQ H9"));
    const Table table(withTrick(Table(), "C2"));
    BOOST_TEST(hand[selectCardToPlay(hand, table)] == card("D3"));
}

BOOST_AUTO_TEST_CASE(leads_spades_if_holding_many) {
    // Five of the 13 spades: Lead the lowest to drive out the others
    const Card::Cards hand(cards("D2 D5 S7 S8 S9 SQ SK H9"));
    BOOST_TEST(hand[selectCardToPlay(hand, afterFirstTrick())] == card("S7"));
}

BOOST_AUTO_TEST_CASE(leads_no_spades_if_others_have_none) {
    // The other players hold no spade: Lead a low card instead
    const Card::Cards hand(cards("D2 D5 S7 S8 S9 SQ SK H9"));
    Table table(afterFirstTrick());
    table.played[Value::SPADES] = 8;
    BOOST_TEST(hand[selectCardToPlay(hand, table)] == card("D2"));
}

BOOST_AUTO_TEST_CASE(starts_with_the_two_of_clubs) {
    const Card::Cards hand(cards("C2 C9 D4"));
    BOOST_TEST(selectCardToPlay(hand, Table()) == 0u);
}

BOOST_AUTO_TEST_SUITE_END()
