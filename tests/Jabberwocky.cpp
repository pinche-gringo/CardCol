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

// Unit tests of the rules of Jabberwocky

#define BOOST_TEST_MODULE Jabberwocky
#include <boost/test/unit_test.hpp>

#include "JabberwockyRules.h"

#include "TestUtil.h"

using namespace JabberwockyRules;
using Card::Value;
using Test::card;
using Test::cards;

namespace JabberwockyRules {
std::ostream& operator<<(std::ostream& out, PlayError error) { return out << static_cast<int>(error); }
std::ostream& operator<<(std::ostream& out, BidError error) { return out << static_cast<int>(error); }
} // namespace JabberwockyRules

namespace {

/// Creates a table with the passed trump card, round and the cards of the
/// actual trick
Table withTrick(std::string_view trump, std::string_view trick, unsigned int round = 0) {
    Table table(card(trump), round);
    for (const auto& c : cards(trick))
        table.play(c);
    return table;
}

/// Returns the passed cards sorted (trumps last)
Card::Cards sorted(std::string_view text, Value::COLOURS trump) {
    Card::Cards result(cards(text));
    std::ranges::sort(result, [trump](const Value& a, const Value& b) { return lessByColourAccTrump(a, b, trump); });
    return result;
}

} // namespace

BOOST_AUTO_TEST_SUITE(game_flow)

BOOST_AUTO_TEST_CASE(tricks_per_round) {
    const std::array<unsigned int, NUM_ROUNDS> expected{3, 4, 5, 6, 7, 8, 9, 8, 7, 6, 5, 4, 3};
    for (unsigned int i(0); i < NUM_ROUNDS; ++i)
        BOOST_TEST(tricksOfRound(i) == expected[i]);
    // The cards (incl. the trump card) must suffice
    BOOST_TEST(((NUM_PLAYERS * tricksOfRound(6)) + 1) <= Value::CARDS_PER_DECK);
}

BOOST_AUTO_TEST_CASE(start_player_rotates) {
    BOOST_TEST(nextStartPlayer(0) == 1u);
    BOOST_TEST(nextStartPlayer(3) == 0u);
}

BOOST_AUTO_TEST_CASE(trumps_are_sorted_last) {
    BOOST_TEST(sorted("SA H2 C5 S3 D9 C2", Value::SPADES) == cards("H2 C2 C5 D9 S3 SA"));
    BOOST_TEST(sorted("SA H2 C5 S3 D9 C2", Value::CLUBS) == cards("D9 S3 SA H2 C2 C5"));
}

BOOST_AUTO_TEST_CASE(positions_of_colours) {
    const ColourPositions positions(positionsOfColours(sorted("H2 C2 C5 S3 SA", Value::SPADES)));
    const ColourPositions expected{2, -1, 4, 0};
    BOOST_TEST(positions == expected);
}

BOOST_AUTO_TEST_CASE(trump_card_counts_as_played) {
    const Table table(card("H7"), 2);
    BOOST_TEST(table.played[Value::HEARTS].count() == 1u);
    BOOST_TEST(table.played[Value::HEARTS][Value::SEVEN]);
    BOOST_TEST(table.tricks() == 5u);
}

BOOST_AUTO_TEST_SUITE_END()

BOOST_AUTO_TEST_SUITE(scoring)

BOOST_AUTO_TEST_CASE(trick_winner) {
    BOOST_TEST(trickWinner(cards("C5 CK C2 CA"), Value::HEARTS) == 3u);
    // Only the colour played first counts ...
    BOOST_TEST(trickWinner(cards("C5 DA SA C6"), Value::HEARTS) == 3u);
    // ... except trumps
    BOOST_TEST(trickWinner(cards("C5 H2 CA C6"), Value::HEARTS) == 1u);
    BOOST_TEST(trickWinner(cards("C5 H2 HJ H3"), Value::HEARTS) == 2u);
    // A lower trump doesn't beat a higher one (neither if trumps are played first)
    BOOST_TEST(trickWinner(cards("HK H5"), Value::HEARTS) == 0u);
    BOOST_TEST(trickWinner(cards("C2 HK H5 CA"), Value::HEARTS) == 1u);
}

BOOST_AUTO_TEST_CASE(points_for_met_bids) {
    const std::array<int, NUM_PLAYERS> expected{1, 0, 1, 0};
    BOOST_TEST(roundScore({0, 2, 3, 1}, {0, 1, 3, 0}) == expected);
}

BOOST_AUTO_TEST_SUITE_END()

BOOST_AUTO_TEST_SUITE(valid_plays)

BOOST_AUTO_TEST_CASE(colour_must_be_followed) {
    const Card::Cards hand(sorted("C9 D4 H5", Value::SPADES));
    const Table table(withTrick("S2", "C7"));
    BOOST_TEST(checkPlay(hand, 1, table) == PlayError::NONE); // C9
    BOOST_TEST(checkPlay(hand, 0, table) == PlayError::FOLLOW_COLOUR);
    BOOST_TEST(checkPlay(hand, 2, table) == PlayError::FOLLOW_COLOUR);
    // Without that colour every card is fine
    BOOST_TEST(checkPlay(sorted("D4 H5", Value::SPADES), 1, table) == PlayError::NONE);
}

BOOST_AUTO_TEST_CASE(trumps_lead_only_when_played) {
    const Card::Cards hand(sorted("D4 H5 S9", Value::SPADES));
    Table table(card("S2"), 0);
    BOOST_TEST(checkPlay(hand, 2, table) == PlayError::NO_TRUMP_TO_START);
    BOOST_TEST(checkPlay(hand, 0, table) == PlayError::NONE);
    // ... except if there is nothing else
    BOOST_TEST(checkPlay(cards("S9 SK"), 0, table) == PlayError::NONE);

    table.play(card("SJ"));
    table.clearTrick();
    BOOST_TEST(checkPlay(hand, 2, table) == PlayError::NONE);
}

BOOST_AUTO_TEST_CASE(bids) {
    BOOST_TEST(checkBid(3, 0, false, 0) == BidError::NONE);
    BOOST_TEST(checkBid(4, 0, false, 0) == BidError::INVALID);
    BOOST_TEST(checkBid(1, 2, false, 0) == BidError::NONE);
    BOOST_TEST(checkBid(1, 2, true, 0) == BidError::SUM_EQUALS_TRICKS);
    BOOST_TEST(checkBid(2, 2, true, 0) == BidError::NONE);
    BOOST_TEST(checkBid(0, 2, true, 0) == BidError::NONE);
}

BOOST_AUTO_TEST_CASE(descriptions) {
    BOOST_TEST(std::string(describe(PlayError::NONE)).empty());
    BOOST_TEST(std::string(describe(PlayError::FOLLOW_COLOUR)) ==
               "Play first cards with an equal colour as the first played one!");
    BOOST_TEST(!std::string(describe(PlayError::NO_TRUMP_TO_START)).empty());
    BOOST_TEST(!std::string(describe(BidError::SUM_EQUALS_TRICKS)).empty());
    BOOST_TEST(std::string(describe(BidError::INVALID)) == "Invalid bid!");
}

BOOST_AUTO_TEST_SUITE_END()

BOOST_AUTO_TEST_SUITE(computer_player)

BOOST_AUTO_TEST_CASE(estimates_tricks) {
    // Round 0 (3 tricks): High trumps and cards from ten upwards count
    BOOST_TEST(estimateTricks(cards("C2 D3 H4"), Value::SPADES, 0) == 0u);
    BOOST_TEST(estimateTricks(cards("CA DK ST"), Value::SPADES, 0) == 3u);
    // Many cards left: Every trump counts
    BOOST_TEST(estimateTricks(cards("S2 S3 H4"), Value::SPADES, 0) == 2u);
    // Round 6 (9 tricks; 15 cards left): Only trumps above eight count fully;
    // other cards must be at least a king
    BOOST_TEST(estimateTricks(cards("S2 S3 SA CK CQ D2 D3 D4 D5"), Value::SPADES, 6) == 3u);
    BOOST_TEST(estimateTricks(cards("S2 S3 SA CJ CQ D2 D3 D4 D5"), Value::SPADES, 6) == 2u);
}

BOOST_AUTO_TEST_CASE(last_bidder_avoids_sum_of_tricks) {
    const Card::Cards hand(cards("CA DK ST")); // Estimated 3 tricks
    BOOST_TEST(selectBid(hand, Value::SPADES, 0, false, 0) == 3u);
    BOOST_TEST(selectBid(hand, Value::SPADES, 0, true, 1) == 3u);
    // Can't bid more than all tricks
    BOOST_TEST(selectBid(hand, Value::SPADES, 0, true, 0) == 2u);
    // Can't bid less than nothing
    BOOST_TEST(selectBid(cards("C2 D3 H4"), Value::SPADES, 0, true, 3) == 1u);
    // Else randomly one more or less
    for (unsigned int seed(1); seed < 50; ++seed) {
        Card::seedRandom(seed);
        const unsigned int bid(selectBid(cards("CA D3 H4"), Value::SPADES, 0, true, 2));
        BOOST_TEST(((bid == 0u) || (bid == 2u)));
    }
}

BOOST_AUTO_TEST_CASE(follows_suit_even_if_trumped) {
    // Bid met: Get rid of the highest card of the colour (a trick, that is trumped, can't be won)
    const Card::Cards hand(sorted("H8 HT C8 C2", Value::SPADES));
    Table table(withTrick("SK", "C9 D8 ST", 4));
    const unsigned int pos(selectCardToPlay(hand, 0, 2, 2, table));
    BOOST_TEST(hand[pos] == card("C8"));
    BOOST_TEST(checkPlay(hand, pos, table) == PlayError::NONE);
}

BOOST_AUTO_TEST_CASE(stays_below_when_bid_is_met) {
    const Card::Cards hand(sorted("D3 D8 DK", Value::SPADES));
    Table table(withTrick("S2", "D9 D2"));
    BOOST_TEST(hand[selectCardToPlay(hand, 1, 0, 0, table)] == card("D8"));
}

BOOST_AUTO_TEST_CASE(discards_highest_card_when_bid_is_met) {
    // Can't follow suit: The highest (non-trump) card is thrown away
    const Card::Cards hand(sorted("SJ H7", Value::CLUBS));
    Table table(withTrick("C4", "C3 CA", 5));
    BOOST_TEST(hand[selectCardToPlay(hand, 1, 2, 2, table)] == card("SJ"));
}

BOOST_AUTO_TEST_CASE(takes_the_trick_as_last_player) {
    const Card::Cards hand(sorted("D3 DT DK", Value::SPADES));
    Table table(withTrick("S2", "D9 D2 D4"));
    BOOST_TEST(hand[selectCardToPlay(hand, 3, 1, 0, table)] == card("DT"));
}

BOOST_AUTO_TEST_CASE(trumps_if_void_and_needing_tricks) {
    const Card::Cards hand(sorted("H3 S4 S9", Value::SPADES));
    Table table(withTrick("S2", "D9 DA"));
    const unsigned int pos(selectCardToPlay(hand, 2, 1, 0, table));
    BOOST_TEST(hand[pos] == card("S4")); // Lowest trump
    BOOST_TEST(table.outOfColour[2][Value::DIAMONDS]);
}

BOOST_AUTO_TEST_CASE(overtrumps_if_needing_tricks) {
    const Card::Cards hand(sorted("H3 S4 S9 SQ", Value::SPADES));
    Table table(withTrick("S2", "D9 S8"));
    BOOST_TEST(hand[selectCardToPlay(hand, 2, 1, 0, table)] == card("S9"));
}

BOOST_AUTO_TEST_CASE(leads_lowest_card) {
    const Card::Cards hand(sorted("H3 D2 S4 C9", Value::SPADES));
    Table table(card("S2"), 0);
    BOOST_TEST(hand[selectCardToPlay(hand, 0, 1, 0, table)] == card("D2"));
}

BOOST_AUTO_TEST_CASE(leads_no_trump_before_trumps_are_played) {
    // Has won more than bid (that's when high cards are led), but must not
    // start with a trump
    const Card::Cards hand(sorted("D4 SA", Value::SPADES));
    Table table(card("S2"), 0);
    const unsigned int pos(selectCardToPlay(hand, 0, 0, 1, table));
    BOOST_TEST(checkPlay(hand, pos, table) == PlayError::NONE);
    BOOST_TEST(hand[pos] == card("D4"));

    // After trumps have been played, the highest trump is led
    table.play(card("SK"));
    table.clearTrick();
    BOOST_TEST(hand[selectCardToPlay(hand, 0, 0, 1, table)] == card("SA"));
}

BOOST_AUTO_TEST_CASE(leads_trump_if_nothing_else) {
    const Card::Cards hand(cards("S4 SA"));
    Table table(card("S2"), 0);
    const unsigned int pos(selectCardToPlay(hand, 0, 1, 0, table));
    BOOST_TEST(checkPlay(hand, pos, table) == PlayError::NONE);
}

BOOST_AUTO_TEST_SUITE_END()
