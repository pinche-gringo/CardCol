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

// Unit tests of the rules of Rovhult

#define BOOST_TEST_MODULE Rovhult
#include <boost/test/unit_test.hpp>

#include <algorithm>
#include <cstring>

#include "RovhultRules.h"

#include "TestUtil.h"

using namespace RovhultRules;
using Card::Value;
using Test::card;
using Test::cards;

namespace RovhultRules {
std::ostream& operator<<(std::ostream& out, PlayError error) { return out << static_cast<int>(error); }
std::ostream& operator<<(std::ostream& out, const Move& move) {
    return out << static_cast<int>(move.source) << ':' << static_cast<int>(move.start) << '-' << static_cast<int>(move.end);
}
} // namespace RovhultRules

namespace {

const Options DEFAULT;

/// Creates the cards of a player
/// \param hand Cards in the hand (sorted by the function)
/// \param reserve Cards of the reserve piles (bottom card first)
Player player(std::string_view hand, const std::array<std::string_view, NUM_RESERVE>& reserve = {"", "", ""}) {
    Player result;
    result.hand = cards(hand);
    std::ranges::stable_sort(result.hand, Card::lessByNumber);
    for (unsigned int i(0); i < NUM_RESERVE; ++i)
        result.reserve[i] = cards(reserve[i]);
    return result;
}

/// Creates a table, where the players 1 - 3 have cards in the hand and a full
/// reserve, and the passed player 0 and played cards
Table table(const Player& first, std::string_view played) {
    Table result;
    result.players[0] = first;
    result.players[1] = player("CJ", {"C3 C4", "C5 C6", "C8 C9"});
    result.players[2] = player("DJ", {"D3 D4", "D5 D6", "D8 D9"});
    result.players[3] = player("SJ", {"S3 S4", "S5 S6", "S8 S9"});
    result.played = cards(played);
    return result;
}

const Move TAKE{};
Move hand(unsigned int start, unsigned int end) { return {Move::HAND, start, end}; }
Move reserve(unsigned int start, unsigned int end) { return {Move::RESERVE, start, end}; }

} // namespace

BOOST_AUTO_TEST_SUITE(values)

BOOST_AUTO_TEST_CASE(order_of_cards) {
    // 3 - A, 2, 10
    BOOST_TEST(compareCards(card("C3"), card("SA"), DEFAULT) < 0);
    BOOST_TEST(compareCards(card("SA"), card("C2"), DEFAULT) < 0);
    BOOST_TEST(compareCards(card("C2"), card("H10"), DEFAULT) < 0);
    BOOST_TEST(compareCards(card("C9"), card("H9"), DEFAULT) == 0);
    BOOST_TEST(valueOf(card("H10"), DEFAULT) == Value::ACE + 2u);
    BOOST_TEST(valueOf(card("HJ"), DEFAULT) == static_cast<unsigned int>(Value::JACK));

    const Options jackNuke{Value::JACK, Value::EIGHT, Value::SEVEN};
    BOOST_TEST(compareCards(card("H10"), card("SQ"), jackNuke) < 0);
    BOOST_TEST(compareCards(card("C2"), card("HJ"), jackNuke) < 0);
}

BOOST_AUTO_TEST_CASE(special_cards) {
    BOOST_TEST(isSpecialCard(Value::TWO));
    BOOST_TEST(isSpecialCard(Value::TEN));
    BOOST_TEST(!isSpecialCard(Value::SEVEN));
    BOOST_TEST(!isSpecialCard(Value::ACE));
}

BOOST_AUTO_TEST_SUITE_END()

BOOST_AUTO_TEST_SUITE(validity)

BOOST_AUTO_TEST_CASE(every_card_on_empty_pile) {
    for (int nr(Value::TWO); nr <= Value::ACE; ++nr)
        BOOST_TEST(checkCard(static_cast<Value::NUMBERS>(nr), {}, DEFAULT) == PlayError::NONE);
}

BOOST_AUTO_TEST_CASE(equal_or_bigger) {
    const Card::Cards played(cards("C5 H9"));
    BOOST_TEST(checkCard(Value::NINE, played, DEFAULT) == PlayError::NONE);
    BOOST_TEST(checkCard(Value::ACE, played, DEFAULT) == PlayError::NONE);
    BOOST_TEST(checkCard(Value::EIGHT, played, DEFAULT) == PlayError::EQUAL_OR_BIGGER);
    BOOST_TEST(checkCard(Value::THREE, played, DEFAULT) == PlayError::EQUAL_OR_BIGGER);
}

BOOST_AUTO_TEST_CASE(two_and_nuke_always) {
    BOOST_TEST(checkCard(Value::TWO, cards("SA"), DEFAULT) == PlayError::NONE);
    BOOST_TEST(checkCard(Value::TEN, cards("SA"), DEFAULT) == PlayError::NONE);
    BOOST_TEST(checkCard(Value::TWO, cards("S7"), DEFAULT) == PlayError::NONE);
    BOOST_TEST(checkCard(Value::TEN, cards("S7"), DEFAULT) == PlayError::NONE);
}

BOOST_AUTO_TEST_CASE(smaller_after_reverse) {
    const Card::Cards played(cards("C5 H7"));
    BOOST_TEST(checkCard(Value::SEVEN, played, DEFAULT) == PlayError::NONE);
    BOOST_TEST(checkCard(Value::THREE, played, DEFAULT) == PlayError::NONE);
    BOOST_TEST(checkCard(Value::EIGHT, played, DEFAULT) == PlayError::SMALLER_AFTER_REVERSE);
    BOOST_TEST(checkCard(Value::ACE, played, DEFAULT) == PlayError::SMALLER_AFTER_REVERSE);

    // After a two everything is possible
    BOOST_TEST(checkCard(Value::THREE, cards("H7 C2"), DEFAULT) == PlayError::NONE);
}

BOOST_AUTO_TEST_CASE(configured_special_cards) {
    const Options options{Value::JACK, Value::NINE, Value::FIVE};
    BOOST_TEST(checkCard(Value::JACK, cards("SA"), options) == PlayError::NONE);
    BOOST_TEST(checkCard(Value::TEN, cards("SA"), options) == PlayError::EQUAL_OR_BIGGER);
    BOOST_TEST(checkCard(Value::SIX, cards("S5"), options) == PlayError::SMALLER_AFTER_REVERSE);
    BOOST_TEST(checkCard(Value::FOUR, cards("S5"), options) == PlayError::NONE);
    BOOST_TEST(checkCard(Value::THREE, cards("S7"), options) == PlayError::EQUAL_OR_BIGGER);
}

BOOST_AUTO_TEST_CASE(messages) {
    for (auto error :
         {PlayError::SMALLER_AFTER_REVERSE, PlayError::EQUAL_OR_BIGGER, PlayError::VISIBLE_CARDS_FIRST, PlayError::INVALID_MOVE})
        BOOST_TEST(std::strlen(describe(error)) > 0u);
    BOOST_TEST(std::string(describe(PlayError::SMALLER_AFTER_REVERSE)).find("%1") != std::string::npos);
    BOOST_TEST(std::strlen(describe(PlayError::NONE)) == 0u);
}

BOOST_AUTO_TEST_CASE(visible_reserve_cards_first) {
    const Player cardsLeft(player("", {"C3", "C4 HK", ""}));
    BOOST_TEST(checkReserve(cardsLeft, 0) == PlayError::VISIBLE_CARDS_FIRST);
    BOOST_TEST(checkReserve(cardsLeft, 1) == PlayError::NONE);
    BOOST_TEST(checkReserve(player("", {"C3", "C4", ""}), 1) == PlayError::NONE);
}

BOOST_AUTO_TEST_CASE(equal_reserve_cards_played_together) {
    const Player cardsLeft(player("", {"C3 H8", "C4 S8", "C5 D8"}));
    BOOST_TEST(firstPileToPlay(cardsLeft, 2) == 0u);
    BOOST_TEST(firstPileToPlay(cardsLeft, 1) == 0u);
    BOOST_TEST(firstPileToPlay(player("", {"C3 H7", "C4 S8", "C5 D8"}), 2) == 1u);
    BOOST_TEST(firstPileToPlay(player("", {"C3 H8", "C4", "C5 D8"}), 2) == 2u);
}

BOOST_AUTO_TEST_CASE(moves) {
    const Table withHand(table(player("C5 H5 D9"), "S7"));
    BOOST_TEST(checkMove(withHand, 0, hand(0, 1), DEFAULT) == PlayError::NONE);
    BOOST_TEST(checkMove(withHand, 0, hand(1, 1), DEFAULT) == PlayError::NONE);
    BOOST_TEST(checkMove(withHand, 0, hand(1, 2), DEFAULT) == PlayError::INVALID_MOVE);
    BOOST_TEST(checkMove(withHand, 0, hand(2, 2), DEFAULT) == PlayError::SMALLER_AFTER_REVERSE);
    BOOST_TEST(checkMove(withHand, 0, hand(3, 3), DEFAULT) == PlayError::INVALID_MOVE);
    BOOST_TEST(checkMove(withHand, 0, TAKE, DEFAULT) == PlayError::NONE);
    BOOST_TEST(checkMove(withHand, 0, reserve(0, 0), DEFAULT) == PlayError::INVALID_MOVE);

    // Nothing to take
    BOOST_TEST(checkMove(table(player("C5"), ""), 0, TAKE, DEFAULT) == PlayError::INVALID_MOVE);

    const Table withReserve(table(player("", {"C3 H8", "C4 S8", "C5"}), "D6"));
    BOOST_TEST(checkMove(withReserve, 0, reserve(0, 1), DEFAULT) == PlayError::NONE);
    BOOST_TEST(checkMove(withReserve, 0, reserve(1, 1), DEFAULT) == PlayError::INVALID_MOVE);
    BOOST_TEST(checkMove(withReserve, 0, reserve(2, 2), DEFAULT) == PlayError::VISIBLE_CARDS_FIRST);
    BOOST_TEST(checkMove(withReserve, 0, hand(0, 0), DEFAULT) == PlayError::INVALID_MOVE);
    BOOST_TEST(checkMove(table(player("", {"C3 H5", "C4", ""}), "D6"), 0, reserve(0, 0), DEFAULT) == PlayError::EQUAL_OR_BIGGER);

    // A hidden card can be played, even if it is invalid (the player takes the pile then)
    BOOST_TEST(checkMove(table(player("", {"C3", "C4", ""}), "DA"), 0, reserve(1, 1), DEFAULT) == PlayError::NONE);
    BOOST_TEST(checkMove(table(player("", {"C3", "C4", ""}), "DA"), 0, reserve(0, 1), DEFAULT) == PlayError::INVALID_MOVE);
}

BOOST_AUTO_TEST_SUITE_END()

BOOST_AUTO_TEST_SUITE(flow)

BOOST_AUTO_TEST_CASE(equal_top_cards) {
    BOOST_TEST(numberOfEqualTopCards({}) == 0u);
    BOOST_TEST(numberOfEqualTopCards(cards("C5")) == 1u);
    BOOST_TEST(numberOfEqualTopCards(cards("C5 H5 D6 S6 C6")) == 3u);
    BOOST_TEST(numberOfEqualTopCards(cards("C6 H6 D6 S6")) == 4u);
}

BOOST_AUTO_TEST_CASE(next_available_player) {
    Table game(table(player("C5"), ""));
    BOOST_TEST(nextAvailablePlayer(game, 0) == 1);
    BOOST_TEST(nextAvailablePlayer(game, 3) == 0);
    game.players[1] = Player();
    game.players[2] = Player();
    BOOST_TEST(nextAvailablePlayer(game, 0) == 3);
    game.players[3] = Player();
    BOOST_TEST(nextAvailablePlayer(game, 0) == -1);
}

BOOST_AUTO_TEST_CASE(filling_up_the_hand) {
    BOOST_TEST(handSizeAfterMove(Value::FIVE, 1, DEFAULT) == CARDS_IN_HAND);
    BOOST_TEST(handSizeAfterMove(Value::FIVE, 0, DEFAULT) == CARDS_IN_HAND);
    BOOST_TEST(handSizeAfterMove(Value::TEN, 2, DEFAULT) == 0u);
    BOOST_TEST(handSizeAfterMove(Value::TEN, 0, DEFAULT) == 1u);
    BOOST_TEST(handSizeAfterMove(Value::TEN, 0, Options{Value::JACK, Value::EIGHT, Value::SEVEN}) == CARDS_IN_HAND);

    Card::Cards hand(cards("C5 HK"));
    Card::Cards staple(cards("D3 S9 C4"));
    fillUp(hand, staple, 3);
    BOOST_TEST(hand == cards("C4 C5 HK"), hand);
    BOOST_TEST(staple == cards("D3 S9"), staple);

    hand.clear();
    fillUp(hand, staple, 3);
    BOOST_TEST(hand == cards("D3 S9"), hand);
    BOOST_TEST(staple.empty());
}

BOOST_AUTO_TEST_CASE(normal_move) {
    const Turn turn(nextTurn(table(player("C5"), "D4 H5"), 0, DEFAULT));
    BOOST_TEST(!turn.clearPlayed);
    BOOST_TEST(!turn.finished);
    BOOST_TEST(turn.loser == -1);
    BOOST_TEST(turn.skipped == -1);
    BOOST_TEST(turn.next == 1u);
}

BOOST_AUTO_TEST_CASE(nuke_clears_and_player_continues) {
    const Turn turn(nextTurn(table(player("C5"), "D4 H10"), 0, DEFAULT));
    BOOST_TEST(turn.clearPlayed);
    BOOST_TEST(turn.next == 0u);

    // Not if he has no cards left
    const Turn finished(nextTurn(table(player(""), "D4 H10"), 0, DEFAULT));
    BOOST_TEST(finished.clearPlayed);
    BOOST_TEST(finished.finished);
    BOOST_TEST(finished.next == 1u);
}

BOOST_AUTO_TEST_CASE(four_equal_cards_clear) {
    BOOST_TEST(nextTurn(table(player("C5"), "D9 H9 S9 C9"), 0, DEFAULT).clearPlayed);
    BOOST_TEST(nextTurn(table(player("C5"), "D9 H9 S9 C9"), 0, DEFAULT).next == 0u);
    BOOST_TEST(!nextTurn(table(player("C5"), "D9 H9 S9"), 0, DEFAULT).clearPlayed);
}

BOOST_AUTO_TEST_CASE(skip_card) {
    const Turn turn(nextTurn(table(player("C5"), "D4 H8"), 0, DEFAULT));
    BOOST_TEST(turn.skipped == 1);
    BOOST_TEST(turn.next == 2u);

    // Skipping uses the configured card
    const Turn configured(nextTurn(table(player("C5"), "D4 H8"), 0, Options{Value::TEN, Value::NINE, Value::SEVEN}));
    BOOST_TEST(configured.skipped == -1);
    BOOST_TEST(configured.next == 1u);
}

BOOST_AUTO_TEST_CASE(finished_players_are_skipped) {
    Table game(table(player(""), "D4 H5"));
    game.players[1] = Player();
    const Turn turn(nextTurn(game, 3, DEFAULT));
    BOOST_TEST(turn.next == 2u);
}

BOOST_AUTO_TEST_CASE(game_ends_with_last_player) {
    Table game(table(player(""), "D4 H5"));
    game.players[1] = Player();
    game.players[3] = Player();
    const Turn turn(nextTurn(game, 0, DEFAULT));
    BOOST_TEST(turn.finished);
    BOOST_TEST(turn.loser == 2);
}

BOOST_AUTO_TEST_SUITE_END()

BOOST_AUTO_TEST_SUITE(dealing)

BOOST_AUTO_TEST_CASE(deal_cards) {
    Card::Cards staple(Test::shuffledDeck(42));
    const Card::Cards deck(staple);
    const auto players(deal(staple, 2));
    BOOST_TEST(staple.size() == Value::CARDS_PER_DECK - 36u);

    Card::Cards all(staple);
    for (const auto& p : players) {
        BOOST_TEST(p.hand.size() == CARDS_IN_HAND);
        BOOST_TEST(std::ranges::is_sorted(p.hand, Card::lessByNumber));
        all.insert(all.end(), p.hand.begin(), p.hand.end());
        for (unsigned int i(0); i < NUM_RESERVE; ++i) {
            BOOST_TEST(p.reserve[i].size() == 2u);
            BOOST_TEST(p.topVisible(i));
            all.insert(all.end(), p.reserve[i].begin(), p.reserve[i].end());
        }
    }
    BOOST_TEST(Test::sameCards(all, deck));

    // Player 2 gets the first cards (from the top of the staple)
    BOOST_TEST((players[2].reserve[0][0] == deck[deck.size() - 1]));
    BOOST_TEST((players[2].reserve[0][1] == deck[deck.size() - 2]));
    BOOST_TEST((players[3].reserve[0][0] == deck[deck.size() - 10]));
}

BOOST_AUTO_TEST_CASE(sort_reserve) {
    Player p(player("", {"C3 H10", "C4 HA", "C5 H2"}));
    sortReserve(p, DEFAULT);
    BOOST_TEST(p.reserve[0] == cards("C3 HA"), p.reserve[0]);
    BOOST_TEST(p.reserve[1] == cards("C4 H2"), p.reserve[1]);
    BOOST_TEST(p.reserve[2] == cards("C5 H10"), p.reserve[2]);
}

BOOST_AUTO_TEST_CASE(exchange_high_cards_to_reserve) {
    Player p(player("H10 SA C2", {"C3 H4", "C4 S6", "C5 D9"}));
    exchangeCards(p, DEFAULT);
    BOOST_TEST(Test::sameCards(p.hand, cards("H4 S6 D9")), p.hand);
    BOOST_TEST(p.reserve[0] == cards("C3 SA"), p.reserve[0]);
    BOOST_TEST(p.reserve[1] == cards("C4 C2"), p.reserve[1]);
    BOOST_TEST(p.reserve[2] == cards("C5 H10"), p.reserve[2]);
}

BOOST_AUTO_TEST_CASE(exchange_keeps_low_hand) {
    Player p(player("H3 S4 C5", {"C3 HA", "C4 H2", "C5 H10"}));
    const Player before(p);
    exchangeCards(p, DEFAULT);
    BOOST_TEST(p.hand == before.hand);
    for (unsigned int i(0); i < NUM_RESERVE; ++i)
        BOOST_TEST(p.reserve[i] == before.reserve[i]);
}

BOOST_AUTO_TEST_SUITE_END()

BOOST_AUTO_TEST_SUITE(computer_player)

BOOST_AUTO_TEST_CASE(plays_lowest_valid_card) {
    BOOST_TEST(selectMove(table(player("C4 H9 SK"), "D6"), 0, DEFAULT) == hand(1, 1));
    BOOST_TEST(selectMove(table(player("C4 H9 SK"), ""), 0, DEFAULT) == hand(0, 0));
}

BOOST_AUTO_TEST_CASE(plays_all_equal_cards) { BOOST_TEST(selectMove(table(player("C9 H9 SK"), "D6"), 0, DEFAULT) == hand(0, 1)); }

BOOST_AUTO_TEST_CASE(completes_four_equal_cards) {
    // Even the last cards (as they complete 4)
    BOOST_TEST(selectMove(table(player("C3 CK HK"), "SK DK"), 0, DEFAULT) == hand(1, 2));
}

BOOST_AUTO_TEST_CASE(saves_the_nuke) { BOOST_TEST(selectMove(table(player("C10 H10 SK"), "D6"), 0, DEFAULT) == hand(2, 2)); }

BOOST_AUTO_TEST_CASE(uses_special_cards_if_needed) {
    BOOST_TEST(selectMove(table(player("C2 H5"), "DA"), 0, DEFAULT) == hand(0, 0));
    BOOST_TEST(selectMove(table(player("H5 C10"), "DA"), 0, DEFAULT) == hand(1, 1));
    BOOST_TEST(selectMove(table(player("H5 SK"), "DA"), 0, DEFAULT) == TAKE);
}

BOOST_AUTO_TEST_CASE(after_reverse) {
    BOOST_TEST(selectMove(table(player("H5 SK"), "D7"), 0, DEFAULT) == hand(0, 0));
    BOOST_TEST(selectMove(table(player("H8 SK C2"), "D7"), 0, DEFAULT) == hand(0, 0));
    BOOST_TEST(selectMove(table(player("H8 SK"), "D7"), 0, DEFAULT) == TAKE);
}

BOOST_AUTO_TEST_CASE(plays_reverse_instead_of_six) {
    BOOST_TEST(selectMove(table(player("H6 S7 SK"), "D4"), 0, DEFAULT) == hand(1, 1));
}

BOOST_AUTO_TEST_CASE(gives_pile_to_next_player) {
    // The next player has only visible high cards: Play a 7 (if possible)
    Table game(table(player("H5 S7 SK"), "D4"));
    game.players[1] = player("", {"C3 CK", "C4 CA", ""});
    BOOST_TEST(selectMove(game, 0, DEFAULT) == hand(1, 1));

    // ... but not if the 7 can't be played (bug: the computer played the 7 on a queen)
    game.played = cards("DQ");
    BOOST_TEST(selectMove(game, 0, DEFAULT) == hand(2, 2));
    BOOST_TEST(checkMove(game, 0, selectMove(game, 0, DEFAULT), DEFAULT) == PlayError::NONE);

    // Or play a card higher than the visible ones of the next player
    game.players[1] = player("", {"C3 C4", "C4 C6", ""});
    game.played = cards("D4");
    BOOST_TEST(selectMove(game, 0, DEFAULT) == hand(2, 2));
}

BOOST_AUTO_TEST_CASE(nuke_below_reverse) {
    // Bug: With the nuke card below the reverse card, the computer skipped the
    // nuke card and played the next (invalid) card
    const Options options{Value::FOUR, Value::JACK, Value::SIX};
    const Table game(table(player("D4 C7 SQ"), "D6 S6"));
    const Move move(selectMove(game, 0, options));
    BOOST_TEST(move == hand(0, 0));
    BOOST_TEST(checkMove(game, 0, move, options) == PlayError::NONE);
}

BOOST_AUTO_TEST_CASE(plays_from_reserve) {
    // Visible cards first (all equal ones)
    BOOST_TEST(selectMove(table(player("", {"C3 H8", "C4 S8", "C5 DK"}), "D6"), 0, DEFAULT) == reserve(0, 1));
    BOOST_TEST(selectMove(table(player("", {"C3 H5", "C4 S8", "C5 DK"}), "D6"), 0, DEFAULT) == reserve(1, 1));
    BOOST_TEST(selectMove(table(player("", {"C3 H5", "C4 S5", ""}), "D6"), 0, DEFAULT) == TAKE);

    // Then the hidden ones
    BOOST_TEST(selectMove(table(player("", {"", "C4", "C5"}), "DA"), 0, DEFAULT) == reserve(1, 1));
}

BOOST_AUTO_TEST_CASE(random_moves) {
    unsigned int cEndgame(0);
    const Player p(player("C5"));
    BOOST_TEST(!playRandomly(cEndgame, p));
    BOOST_TEST(cEndgame == 0u);

    cEndgame = 1;
    unsigned int randomMoves(0);
    for (unsigned int i(0); i < 64; ++i)
        randomMoves += playRandomly(cEndgame, p);
    BOOST_TEST(cEndgame == 65u);
    BOOST_TEST(randomMoves == 5u); // 32, 40, 48, 56, 64
    cEndgame = 39;
    BOOST_TEST(!playRandomly(cEndgame, player("", {"C3", "", ""})));

    Card::seedRandom(1);
    for (unsigned int i(0); i < 20; ++i) {
        const Move move(selectRandomCard(cards("C5 H8 SK"), cards("D8"), DEFAULT));
        BOOST_TEST(((move == TAKE) || (move == hand(1, 1)) || (move == hand(2, 2))), move);
    }
}

BOOST_AUTO_TEST_SUITE_END()
