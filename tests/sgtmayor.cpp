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

// Unit tests of the rules of Sgt. Mayor

#define BOOST_TEST_MODULE SgtMayor
#include <boost/test/unit_test.hpp>

#include <cstring>
#include <numeric>

#include "SgtMayorRules.h"

#include "TestUtil.h"

using namespace SgtMayorRules;
using Card::Value;
using Test::card;
using Test::cards;

namespace SgtMayorRules {
std::ostream& operator<<(std::ostream& out, PlayError error) { return out << static_cast<int>(error); }
} // namespace SgtMayorRules

namespace {

/// Creates a table for a new round with the passed trump and trick
Table withTrick(Value::COLOURS trump, std::string_view trick) {
    Table table;
    table.newRound();
    table.trump = trump;
    for (const auto& c : cards(trick))
        table.play(c);
    return table;
}

/// Creates a hand (sorted by colour)
Card::Cards hand(std::string_view text) { return Test::sortedByColour(cards(text)); }

} // namespace

BOOST_AUTO_TEST_SUITE(start_of_round)

BOOST_AUTO_TEST_CASE(cards_in_game) {
    BOOST_TEST(CARDS_PER_PLAYER == 17u);
    BOOST_TEST(!isUsed(card("C2")));
    BOOST_TEST(isUsed(card("C3")));
    BOOST_TEST(isUsed(card("D2")));
}

BOOST_AUTO_TEST_CASE(needed_tricks) {
    BOOST_TEST(std::accumulate(NEEDED_TRICKS.begin(), NEEDED_TRICKS.end(), 0u) == CARDS_PER_PLAYER);
    BOOST_TEST(neededTricks(1, 1) == 6u);
    BOOST_TEST(neededTricks(2, 1) == 3u);
    BOOST_TEST(neededTricks(0, 1) == 8u);
    BOOST_TEST(trumpPlayer(1) == 0u);
    BOOST_TEST(neededTricks(trumpPlayer(2), 2) == 8u);
    BOOST_TEST(nextStartPlayer(0) == 1u);
    BOOST_TEST(nextStartPlayer(2) == 0u);
}

BOOST_AUTO_TEST_CASE(new_round) {
    Table table(withTrick(Value::SPADES, "S5 S7"));
    table.outOfColours = 0x123;
    table.newRound();
    BOOST_TEST(table.trick.empty());
    BOOST_TEST(table.outOfColours == 0u);
    BOOST_TEST(table.played[Value::CLUBS].count() == 1u);
    BOOST_TEST(table.played[Value::SPADES].none());
    // The (unused) two of clubs counts as played
    BOOST_TEST(table.isHighest(card("CA")));
    BOOST_TEST(!table.isHighest(card("CK")));
}

BOOST_AUTO_TEST_CASE(highest_card) {
    Table table(withTrick(Value::HEARTS, "SA SK"));
    BOOST_TEST(table.isHighest(card("SQ")));
    BOOST_TEST(!table.isHighest(card("SJ")));
    BOOST_TEST(table.isHighest(card("DA")));
}

BOOST_AUTO_TEST_SUITE_END()

BOOST_AUTO_TEST_SUITE(playing)

BOOST_AUTO_TEST_CASE(follow_colour) {
    const Card::Cards h(hand("C3 D4 S7 SA H2"));
    const Table table(withTrick(Value::HEARTS, "S5"));
    BOOST_TEST(checkPlay(h, 2, table) == PlayError::NONE);
    BOOST_TEST(checkPlay(h, 3, table) == PlayError::NONE);
    BOOST_TEST(checkPlay(h, 0, table) == PlayError::FOLLOW_COLOUR);
    BOOST_TEST(checkPlay(h, 4, table) == PlayError::FOLLOW_COLOUR);
    BOOST_TEST(std::strcmp(describe(PlayError::FOLLOW_COLOUR), "Play a card with an equal colour as the first played one!") == 0);
    BOOST_TEST(*describe(PlayError::NONE) == '\0');
}

BOOST_AUTO_TEST_CASE(no_colour_any_card) {
    const Card::Cards h(hand("C3 D4 H2"));
    const Table table(withTrick(Value::HEARTS, "S5 S9"));
    for (unsigned int i(0); i < h.size(); ++i)
        BOOST_TEST(checkPlay(h, i, table) == PlayError::NONE);
}

BOOST_AUTO_TEST_CASE(lead_any_card) {
    const Card::Cards h(hand("C3 D4 S7 H2"));
    const Table table(withTrick(Value::HEARTS, ""));
    for (unsigned int i(0); i < h.size(); ++i)
        BOOST_TEST(checkPlay(h, i, table) == PlayError::NONE);
}

BOOST_AUTO_TEST_CASE(winner_of_trick) {
    BOOST_TEST(trickWinner(cards("S5 S9 SA"), Value::HEARTS) == 2u);
    BOOST_TEST(trickWinner(cards("S5 C9 DA"), Value::HEARTS) == 0u);
    BOOST_TEST(trickWinner(cards("S5 H2 SA"), Value::HEARTS) == 1u);
    BOOST_TEST(trickWinner(cards("S5 H2 H9"), Value::HEARTS) == 2u);
    BOOST_TEST(trickWinner(cards("S5 H9 H2"), Value::HEARTS) == 1u);
    BOOST_TEST(trickWinner(cards("H5 SA H9"), Value::HEARTS) == 2u);
    BOOST_TEST(trickWinner(cards("H5 SA H3"), Value::HEARTS) == 0u);
}

BOOST_AUTO_TEST_CASE(note_not_following) {
    Table table(withTrick(Value::HEARTS, "S5"));
    table.noteNotFollowing(1, card("S9"));
    BOOST_TEST(table.outOfColours == 0u);
    table.noteNotFollowing(1, card("H9"));
    BOOST_TEST(table.outOfColours == (1u << (Value::SPADES + 4)));
    table.noteNotFollowing(2, card("C9"));
    BOOST_TEST(table.outOfColours == ((1u << (Value::SPADES + 4)) | (1u << (Value::SPADES + 8)) | (1u << (Value::HEARTS + 8))));
}

BOOST_AUTO_TEST_SUITE_END()

BOOST_AUTO_TEST_SUITE(scoring)

BOOST_AUTO_TEST_CASE(round_score) {
    // Start player 1 needs 6, player 2 needs 3 and player 0 needs 8
    const auto score(roundScore({10, 4, 3}, 1));
    BOOST_TEST(score[0] == 2);
    BOOST_TEST(score[1] == -2);
    BOOST_TEST(score[2] == 0);

    const auto exact(roundScore({6, 3, 8}, 0));
    BOOST_TEST((exact == std::array<int, NUM_PLAYERS>{}));
}

BOOST_AUTO_TEST_CASE(end_of_game) {
    BOOST_TEST(!isGameOver({9, -4, -5}));
    BOOST_TEST(isGameOver({10, -4, -6}));
    BOOST_TEST(isGameOver({-12, 1, 11}));
    BOOST_TEST(!isGameOver({-12, 1, 11}, 12));
    BOOST_TEST(winner({-12, 1, 11}) == 2u);
    BOOST_TEST(winner({5, 5, -10}) == 0u);
}

BOOST_AUTO_TEST_SUITE_END()

BOOST_AUTO_TEST_SUITE(exchange)

BOOST_AUTO_TEST_CASE(no_exchange) { BOOST_TEST(!nextExchange({0, 0, 0}, 1).has_value()); }

BOOST_AUTO_TEST_CASE(order_of_exchanges) {
    auto ex(nextExchange({2, -1, -1}, 0));
    BOOST_TEST_REQUIRE(ex.has_value());
    BOOST_TEST(ex->playerBad == 0u);
    BOOST_TEST(ex->playerGood == 1u);

    // The search for a player with too much tricks starts with the start player
    ex = nextExchange({-3, 1, 2}, 0);
    BOOST_TEST_REQUIRE(ex.has_value());
    BOOST_TEST(ex->playerBad == 1u);
    BOOST_TEST(ex->playerGood == 0u);
    ex = nextExchange({-3, 1, 2}, 2);
    BOOST_TEST_REQUIRE(ex.has_value());
    BOOST_TEST(ex->playerBad == 2u);
    BOOST_TEST(ex->playerGood == 0u);
}

BOOST_AUTO_TEST_CASE(exchanges_compensate_tricks) {
    std::array<int, NUM_PLAYERS> diff{-3, 1, 2};
    unsigned int exchanges(0);
    while (const auto ex = nextExchange(diff, 1)) {
        BOOST_TEST_REQUIRE(exchanges < 10u);
        applyExchange(diff, *ex);
        ++exchanges;
    }
    BOOST_TEST(exchanges == 3u);
    BOOST_TEST((diff == std::array<int, NUM_PLAYERS>{}));
}

BOOST_AUTO_TEST_CASE(cards_to_exchange) {
    BOOST_TEST(selectBadCard(hand("C9 D4 S3 HA")) == 2u);

    const Card::Cards good(hand("C3 D4 DK S2"));
    BOOST_TEST(selectGoodCard(good, card("D5")) == 2u); // Highest diamond
    BOOST_TEST(selectGoodCard(good, card("H5")) == 3u); // No heart: Lowest card
    BOOST_TEST(isValidExchange(good, 2, card("D5")));
    BOOST_TEST(isValidExchange(good, 1, card("D5")));
    BOOST_TEST(!isValidExchange(good, 0, card("D5")));
    BOOST_TEST(isValidExchange(good, 0, card("H5")));
}

BOOST_AUTO_TEST_SUITE_END()

BOOST_AUTO_TEST_SUITE(computer_player)

BOOST_AUTO_TEST_CASE(trump_most_cards) {
    BOOST_TEST(selectTrump(hand("C3 D4 H2 H5 H9 HK HA")) == Value::HEARTS);
    BOOST_TEST(selectTrump(hand("C3 C4 C5 D4 D7 S8")) == Value::CLUBS);
    // Equal number: The higher cards
    BOOST_TEST(selectTrump(hand("C3 C4 DK DA S2")) == Value::DIAMONDS);
}

BOOST_AUTO_TEST_CASE(trump_quirks) {
    // Faithful port: The last card is ignored and each colour is only compared
    // with the previous one; so spades (4) are selected instead of clubs (5)
    BOOST_TEST(selectTrump(hand("C3 C4 C5 C6 C7 D3 D4 S3 S4 S5 S6 H3")) == Value::SPADES);
}

BOOST_AUTO_TEST_CASE(lead_with_trumps) {
    const Card::Cards h(hand("C3 D4 H5 H7 H9 HJ HQ HA"));
    Table table(withTrick(Value::HEARTS, ""));
    BOOST_TEST(selectCardToPlay(h, 0, 0, table) == 7u); // Highest trump, as it is the highest left

    const Card::Cards noAce(hand("C3 D4 H5 H7 H9 HJ HQ HK"));
    BOOST_TEST(selectCardToPlay(noAce, 0, 0, table) == 2u); // Else the lowest trump
}

BOOST_AUTO_TEST_CASE(lead_dead_cards) {
    // Many clubs, no trumps: Play the clubs (the lowest, as the ace is out)
    const Card::Cards h(hand("C3 C4 C5 C6 C7 C8 C9 D4 S5"));
    Table table(withTrick(Value::HEARTS, ""));
    BOOST_TEST(selectCardToPlay(h, 1, 0, table) == 0u);
}

BOOST_AUTO_TEST_CASE(follow_with_highest) {
    Table table(withTrick(Value::HEARTS, "S5"));
    BOOST_TEST(selectCardToPlay(hand("C3 S7 SA H2"), 1, 0, table) == 2u);
    BOOST_TEST(selectCardToPlay(hand("C3 S7 SK H2"), 1, 0, table) == 1u);
    BOOST_TEST(table.outOfColours == 0u);
}

BOOST_AUTO_TEST_CASE(follow_low_if_next_is_out) {
    Table table(withTrick(Value::HEARTS, "S5"));
    table.outOfColours = (1u << Value::SPADES) << (2 << 2);
    BOOST_TEST(selectCardToPlay(hand("C3 S7 SA H2"), 1, 0, table) == 1u);
}

BOOST_AUTO_TEST_CASE(trump_if_out_of_colour) {
    Table table(withTrick(Value::HEARTS, "S5"));
    BOOST_TEST(selectCardToPlay(hand("C3 D9 H2 H8"), 1, 0, table) == 2u); // The lowest trump
    BOOST_TEST(table.outOfColours == (1u << (Value::SPADES + 4)));
}

BOOST_AUTO_TEST_CASE(third_after_trump) {
    // The second player trumped: Play a low card of the colour
    Table table(withTrick(Value::HEARTS, "S5 H3"));
    BOOST_TEST(selectCardToPlay(hand("C3 S2 SK"), 2, 0, table) == 1u);
    // ... or overtrump
    BOOST_TEST(selectCardToPlay(hand("C3 D2 H2 H9"), 2, 0, table) == 3u);
}

BOOST_AUTO_TEST_CASE(third_takes_trick) {
    Table table(withTrick(Value::HEARTS, "S5 S9"));
    BOOST_TEST(selectCardToPlay(hand("C3 S7 SJ SA"), 2, 0, table) == 2u); // Lowest card above S9
    BOOST_TEST(selectCardToPlay(hand("C3 S2 S7"), 2, 0, table) == 1u);    // Can't: Lowest spade
    BOOST_TEST(selectCardToPlay(hand("C3 D7 H4"), 2, 0, table) == 2u);    // Trump
    BOOST_TEST((table.outOfColours & (1u << (Value::SPADES + 8))) != 0u);
}

BOOST_AUTO_TEST_SUITE_END()
