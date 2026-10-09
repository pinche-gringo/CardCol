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

// Unit tests of the rules of Twopart

#define BOOST_TEST_MODULE Twopart
#include <boost/test/unit_test.hpp>

#include <set>
#include <string>

#include "TwopartRules.h"

#include "TestUtil.h"

using namespace TwopartRules;
using Card::Value;
using Test::card;
using Test::cards;

namespace TwopartRules {
std::ostream& operator<<(std::ostream& out, PlayError error) { return out << static_cast<int>(error); }
} // namespace TwopartRules

namespace {

/// Creates a table for part 2 with the passed trump
Table partTwo(Value::COLOURS trump) {
    Table table;
    table.trump = trump;
    table.partTwo = true;
    return table;
}

/// Returns the played cards as text (or "-" if the player picks up)
std::string toString(const std::optional<Play>& play) {
    return play ? (std::to_string(play->start) + '-' + std::to_string(play->end)) : std::string("-");
}

} // namespace

BOOST_AUTO_TEST_SUITE(validity)

BOOST_AUTO_TEST_CASE(part_one_allows_one_card) {
    const Table table;
    const Card::Cards hand(cards("D2 S5 HA"));
    for (unsigned int i(0); i < hand.size(); ++i)
        BOOST_TEST(checkPlay(hand, i, i, cards("CA DK"), table) == PlayError::NONE);
    BOOST_TEST(checkPlay(hand, 0, 1, cards(""), table) == PlayError::ONLY_ONE_CARD);
}

BOOST_AUTO_TEST_CASE(part_two_same_colour_bigger) {
    const Table table(partTwo(Value::CLUBS));
    const Card::Cards hand(cards("D5 D9 S3 C2 CQ"));
    BOOST_TEST(checkPlay(hand, 1, 1, cards("D7"), table) == PlayError::NONE);
    BOOST_TEST(checkPlay(hand, 0, 0, cards("D7"), table) == PlayError::NOT_BIGGER);
    BOOST_TEST(checkPlay(hand, 2, 2, cards("D7"), table) == PlayError::NOT_BIGGER); // Other colour
    BOOST_TEST(checkPlay(hand, 0, 0, cards(""), table) == PlayError::NONE);         // Everything starts
}

BOOST_AUTO_TEST_CASE(part_two_trumps) {
    const Table table(partTwo(Value::CLUBS));
    const Card::Cards hand(cards("D5 C2 CQ"));
    BOOST_TEST(checkPlay(hand, 1, 1, cards("DA"), table) == PlayError::NONE); // Every trump beats other colours
    BOOST_TEST(checkPlay(hand, 2, 2, cards("CT"), table) == PlayError::NONE);
    BOOST_TEST(checkPlay(hand, 1, 1, cards("CT"), table) == PlayError::NOT_BIGGER);
    BOOST_TEST(checkPlay(hand, 0, 0, cards("C3"), table) == PlayError::NOT_BIGGER);
}

BOOST_AUTO_TEST_CASE(part_two_series) {
    const Table table(partTwo(Value::HEARTS));
    const Card::Cards hand(cards("S3 S4 S5 S7"));
    // Only the last card of a serie must beat the played card (as the GUI checks it)
    BOOST_TEST(checkPlay(hand, 0, 2, cards("S4"), table) == PlayError::NONE);
    BOOST_TEST(checkPlay(hand, 0, 3, cards("S2"), table) == PlayError::NO_SERIE);
    BOOST_TEST(checkPlay(hand, 2, 3, cards(""), table) == PlayError::NO_SERIE);
}

BOOST_AUTO_TEST_CASE(messages) {
    BOOST_TEST(std::string(describe(PlayError::NONE)).empty());
    BOOST_TEST(std::string(describe(PlayError::NOT_BIGGER)) ==
               "The played card must have the same colour and must be bigger (or be a trump)!");
    BOOST_TEST(std::string(describe(PlayError::NOT_BIGGER, true)) ==
               "The played cards must have the same colour and must be bigger (or be trumps)!");
    BOOST_TEST(!std::string(describe(PlayError::NO_SERIE)).empty());
    BOOST_TEST(!std::string(describe(PlayError::ONLY_ONE_CARD)).empty());
}

BOOST_AUTO_TEST_SUITE_END()

BOOST_AUTO_TEST_SUITE(helpers)

BOOST_AUTO_TEST_CASE(series) {
    const Card::Cards hand(cards("D3 D4 D5 S6 S8"));
    BOOST_TEST(findEndOfSerie(hand, 0) == 2u);
    BOOST_TEST(findEndOfSerie(hand, 1) == 2u);
    BOOST_TEST(findEndOfSerie(hand, 3) == 3u);
    BOOST_TEST(findEndOfSerie(hand, 4) == 4u);
    BOOST_TEST(findStartOfSerie(hand, 2) == 0u);
    BOOST_TEST(findStartOfSerie(hand, 0) == 0u);
    BOOST_TEST(findStartOfSerie(hand, 3) == 3u);
}

BOOST_AUTO_TEST_CASE(sorting_with_trumps) {
    Card::Cards hand(cards("C2 HA D5 S3 C9"));
    sortByColourAccTrumps(hand, Value::CLUBS); // Order: The colours after the trump, the trump last
    BOOST_TEST(hand == cards("D5 S3 HA C2 C9"));
    sortByColourAccTrumps(hand, Value::SPADES);
    BOOST_TEST(hand == cards("HA C2 C9 D5 S3"));
    BOOST_TEST(lessByColourAccTrumps(card("DA"), card("C2"), Value::CLUBS));
    BOOST_TEST(!lessByColourAccTrumps(card("C2"), card("DA"), Value::CLUBS));
    BOOST_TEST(lessByColourAccTrumps(card("C2"), card("C3"), Value::CLUBS));
}

BOOST_AUTO_TEST_CASE(analysis) {
    const Card::Cards played(cards("D5 S9 H5 C9 DA"));
    Analysis result(analyzePlayed(played, 0, played.size(), Value::SPADES));
    BOOST_TEST(result.max == Value::ACE);
    BOOST_TEST(result.maxPos == 4);
    BOOST_TEST(result.maxEqual == Value::NINE);
    BOOST_TEST(result.maxEqualPos == 1);
    BOOST_TEST(result.trumps == 1);

    result = analyzePlayed(played, 2, 3, std::nullopt);
    BOOST_TEST(result.maxEqual == -1);
    BOOST_TEST(result.maxEqualPos == -1);
    BOOST_TEST(result.maxPos == 4);
    BOOST_TEST(result.trumps == 0);

    result = analyzePlayed(played, 5, 0, std::nullopt);
    BOOST_TEST(result.max == -1);
}

BOOST_AUTO_TEST_CASE(players) {
    Table table;
    table.bfPlayers = 0b1010;
    BOOST_TEST(table.nextPlayer(1) == 3);
    BOOST_TEST(table.nextPlayer(3) == 1);
    BOOST_TEST(table.nextPlayer(2) == 3);
    BOOST_TEST(playersInBitfield(table.bfPlayers) == 2u);
    table.bfPlayers = 0;
    BOOST_TEST(table.nextPlayer(1) == -1);

    table.bfPlayers = 0b1011;
    table.startPlayer = 3;
    BOOST_TEST(table.pos2Player(0) == 3u);
    BOOST_TEST(table.pos2Player(1) == 0u);
    BOOST_TEST(table.pos2Player(2) == 1u);
    BOOST_TEST(table.pos2Player(3) == 3u);

    BOOST_TEST(findNextPlayerWithCards({0, 2, 0, 0}, 1) == 1);
    BOOST_TEST(findNextPlayerWithCards({0, 2, 0, 1}, 1) == 3);
    BOOST_TEST(findNextPlayerWithCards({0, 0, 0, 0}, 1) == -1);
}

BOOST_AUTO_TEST_SUITE_END()

BOOST_AUTO_TEST_SUITE(part_one)

BOOST_AUTO_TEST_CASE(highest_card_wins) {
    Table table;
    table.startPlayer = 1;
    const Card::Cards played(cards("D3 SK H5 C7"));
    const HandSizes hands{3, 3, 3, 3};
    for (unsigned int i(0); i < 3; ++i) {
        const TurnResult result(table.endTurn(1 + i, Card::Cards(played.begin(), played.begin() + i + 1), hands));
        BOOST_TEST(!result.endOfRound);
        BOOST_TEST(result.next == (2 + i) % NUM_PLAYERS);
    }
    const TurnResult result(table.endTurn(0, played, hands));
    BOOST_TEST(result.endOfRound);
    BOOST_TEST(!result.endOfPart);
    BOOST_TEST(result.winner == 2); // SK has been played by the second player
    BOOST_TEST(result.next == 2u);
    BOOST_TEST(table.startPlayer == 2u);
    BOOST_TEST(table.bfPlayers == ALL_PLAYERS);
    BOOST_TEST(table.startPos[0] == played.size());
    table.playedCardsWon();
    BOOST_TEST(table.startPos[0] == 0u);
}

BOOST_AUTO_TEST_CASE(tie_continues_round) {
    Table table;
    table.startPlayer = 0;
    Card::Cards played(cards("D9 S4 H9 C2"));
    const HandSizes hands{3, 3, 3, 3};
    for (unsigned int i(0); i < 3; ++i)
        table.endTurn(i, Card::Cards(played.begin(), played.begin() + i + 1), hands);
    TurnResult result(table.endTurn(3, played, hands));
    BOOST_TEST(result.endOfRound);
    BOOST_TEST(result.winner == -1);
    BOOST_TEST(result.next == 0u);
    BOOST_TEST(table.bfPlayers == 0b0101u); // Players with the nines continue
    BOOST_TEST(table.startPos[0] == 4u);

    played.push_back(card("SA"));
    result = table.endTurn(0, played, hands);
    BOOST_TEST(!result.endOfRound);
    BOOST_TEST(result.next == 2u);
    played.push_back(card("D5"));
    result = table.endTurn(2, played, hands);
    BOOST_TEST(result.endOfRound);
    BOOST_TEST(result.winner == 0); // Wins all played cards with SA
    BOOST_TEST(result.next == 0u);
    BOOST_TEST(table.bfPlayers == ALL_PLAYERS);
}

BOOST_AUTO_TEST_CASE(tie_with_player_without_cards) {
    Table table;
    table.startPlayer = 0;
    const Card::Cards played(cards("D9 S4 H9 C2"));
    const HandSizes hands{0, 3, 3, 3};
    for (unsigned int i(0); i < 3; ++i)
        table.endTurn(i, Card::Cards(played.begin(), played.begin() + i + 1), hands);
    const TurnResult result(table.endTurn(3, played, hands));
    BOOST_TEST(result.endOfRound);
    BOOST_TEST(result.winner == 2); // The only one with a nine and cards left
    BOOST_TEST(result.next == 2u);
    BOOST_TEST(table.bfPlayers == 0b1110u);
}

BOOST_AUTO_TEST_CASE(end_of_part) {
    Table table;
    table.startPlayer = 1;
    table.bfPlayers = table.bfOldPlayers = 0b0010;
    const TurnResult result(table.endTurn(1, cards("D5"), {0, 0, 0, 0}));
    BOOST_TEST(result.endOfRound);
    BOOST_TEST(result.endOfPart);
    BOOST_TEST(result.winner == 1);
    BOOST_TEST(result.next == 1u); // The winner starts part 2
    BOOST_TEST(table.nextPlayer(1) == -1);
}

BOOST_AUTO_TEST_CASE(start_part_two) {
    Table table(partTwo(Value::HEARTS));
    table.partTwo = false;
    table.bfPlayers = 0;
    const std::array<Card::Cards, NUM_PLAYERS> won{cards("C2 D9"), cards(""), cards("SA H3 S4"), cards("")};
    unsigned int player(2);
    const Receivers receivers(table.startPartTwo(won, player));
    BOOST_TEST(player == 2u);
    BOOST_TEST(table.partTwo);
    BOOST_TEST(table.bfPlayers == ALL_PLAYERS);
    // Cards up to five go alternately to the players without cards (starting with the top card)
    BOOST_TEST(receivers[0] == std::vector<unsigned int>({0, 1}));
    BOOST_TEST(receivers[1].empty());
    BOOST_TEST(receivers[2] == std::vector<unsigned int>({3, 1, 2}));
    BOOST_TEST(receivers[3].empty());
}

BOOST_AUTO_TEST_CASE(start_part_two_everybody_won) {
    Table table(partTwo(Value::HEARTS));
    table.bfPlayers = 0;
    const std::array<Card::Cards, NUM_PLAYERS> won{cards("C2"), cards("D3"), cards("S4 HA"), cards("H5")};
    unsigned int player(0);
    const Receivers receivers(table.startPartTwo(won, player));
    BOOST_TEST(player == 0u);
    for (unsigned int i(0); i < NUM_PLAYERS; ++i)
        BOOST_TEST(receivers[i] == std::vector<unsigned int>(won[i].size(), i));
}

BOOST_AUTO_TEST_CASE(start_part_two_starter_without_cards) {
    // Regression: The winner of the last round gives away all his (low) cards;
    // he must not start part 2 without cards (simulation seed 2237)
    Table table(partTwo(Value::HEARTS));
    table.bfPlayers = 0;
    const std::array<Card::Cards, NUM_PLAYERS> won{cards(""), cards("C3 D4"), cards("HA"), cards("")};
    unsigned int player(1);
    const Receivers receivers(table.startPartTwo(won, player));
    BOOST_TEST(receivers[1] == std::vector<unsigned int>({3, 0}));
    BOOST_TEST(receivers[2] == std::vector<unsigned int>({2}));
    BOOST_TEST(player == 2u);
    BOOST_TEST(table.bfPlayers == 0b1101u); // Player 1 has no cards left
}

BOOST_AUTO_TEST_SUITE_END()

BOOST_AUTO_TEST_SUITE(part_two)

BOOST_AUTO_TEST_CASE(round_ends) {
    Table table(partTwo(Value::HEARTS));
    table.bfPlayers = 0b1000;
    table.registerPlay(0);
    const TurnResult result(table.endTurn(3, cards("D5"), {2, 0, 4, 1}));
    BOOST_TEST(result.endOfRound);
    BOOST_TEST(!result.endOfPart);
    BOOST_TEST(result.next == 3u);
    BOOST_TEST(table.bfPlayers == 0b1101u);
    BOOST_TEST(table.offPos == 0u);
}

BOOST_AUTO_TEST_CASE(round_ends_with_player_without_cards) {
    Table table(partTwo(Value::HEARTS));
    table.bfPlayers = 0b1000;
    const TurnResult result(table.endTurn(3, cards("D5"), {2, 0, 4, 0}));
    BOOST_TEST(result.endOfRound);
    BOOST_TEST(result.next == 0u);
}

BOOST_AUTO_TEST_CASE(game_over) {
    Table table(partTwo(Value::HEARTS));
    table.bfPlayers = 0b0110;
    const TurnResult result(table.endTurn(1, cards("D5"), {0, 0, 5, 0}));
    BOOST_TEST(!result.endOfRound);
    BOOST_TEST(result.endOfPart);
    BOOST_TEST(result.next == 2u); // The loser
}

BOOST_AUTO_TEST_CASE(pick_up) {
    Table table(partTwo(Value::HEARTS));
    table.registerPlay(0);
    BOOST_TEST(table.endTurn(0, cards("D5"), {2, 3, 3, 3}).next == 1u);
    table.registerPlay(1);
    BOOST_TEST(table.endTurn(1, cards("D5 D6 D7"), {2, 1, 3, 3}).next == 2u);
    BOOST_TEST(table.offPos == 2u);

    PickUp pickUp(table.pickUp(2, {2, 1, 3, 3}));
    BOOST_TEST(pickUp.start == 1u); // Only the cards played last
    BOOST_TEST(pickUp.next == 3u);
    BOOST_TEST(table.offPos == 1u);
    BOOST_TEST(table.bfPlayers == 0b1011u); // Two players re-enter the round: 0 and 1

    pickUp = table.pickUp(3, {2, 1, 5, 3});
    BOOST_TEST(pickUp.start == 0u);
    BOOST_TEST(pickUp.next == 0u);
    BOOST_TEST(table.offPos == 0u);
    BOOST_TEST(table.bfPlayers == ALL_PLAYERS);
}

BOOST_AUTO_TEST_CASE(pick_ups_are_counted_per_round) {
    Table table(partTwo(Value::HEARTS));
    table.registerPlay(0);
    table.endTurn(0, cards("D5"), {2, 3, 3, 3});
    table.pickUp(1, {2, 3, 3, 3});
    BOOST_TEST(table.pickUps == 1u);

    table.registerPlay(1);
    table.bfPlayers = 0b0001;
    BOOST_TEST(table.endTurn(0, cards("D4 D5"), {1, 4, 3, 3}).endOfRound);
    BOOST_TEST(table.pickUps == 0u);
}

BOOST_AUTO_TEST_SUITE_END()

BOOST_AUTO_TEST_SUITE(computer_player)

BOOST_AUTO_TEST_CASE(bigger_card) {
    BOOST_TEST(findBigger(cards("D3 S5 H9 CT"), Value::FIVE, std::nullopt) == 1);
    BOOST_TEST(findBigger(cards("D3 S5 H9 CT"), Value::FIVE, Value::CLUBS) == 3);
    BOOST_TEST(findBigger(cards("D3 S5 H9 CT"), Value::ACE, Value::CLUBS) == -1);
}

BOOST_AUTO_TEST_CASE(avoid_pile) {
    Table table;
    table.bfPlayers = 0b1110;
    // The first card (three) would match the played one: Play the second
    BOOST_TEST(toString(selectCardsToPlay(1, cards("C3 S5 HK"), cards("D3"), 1, 30, table)) == "1-1");
    // Nothing matches: Play the smallest card
    BOOST_TEST(toString(selectCardsToPlay(1, cards("C2 S5 HK"), cards("D3"), 1, 30, table)) == "0-0");
}

BOOST_AUTO_TEST_CASE(last_player_beats_low_pile) {
    Table table;
    table.bfPlayers = 0b1000;
    BOOST_TEST(toString(selectCardsToPlay(3, cards("C7 C8 CA"), cards("D3 S4 H6"), 1, 20, table)) == "1-1");
    BOOST_TEST(toString(selectCardsToPlay(3, cards("C2 C8 CA"), cards("D3 S4 H6"), 1, 20, table)) == "0-0");
}

BOOST_AUTO_TEST_CASE(join_tie) {
    Table table;
    table.bfPlayers = 0b1100;
    // A pair of nines has been played: Play a nine too (regression: The check
    // for the pair accessed the played cards with the position in the hand)
    BOOST_TEST(toString(selectCardsToPlay(2, cards("D4 C9 D9"), cards("H9 S9"), 0, 30, table)) == "1-1");
}

BOOST_AUTO_TEST_CASE(get_pile_with_short_hand) {
    // Regression: The computer wanted to play the card at position 2, even
    // when having less than three cards (at the end of part 1)
    Table table;
    table.bfPlayers = 0b1010;
    BOOST_TEST(toString(selectCardsToPlay(3, cards("DT"), cards(""), 0, 0, table)) == "0-0");
    BOOST_TEST(toString(selectCardsToPlay(3, cards("D2 DT"), cards(""), 0, 0, table)) == "1-1");
    BOOST_TEST(toString(selectCardsToPlay(3, cards("D2 D5 DT"), cards(""), 0, 0, table)) == "2-2");
}

BOOST_AUTO_TEST_CASE(play_trump_if_pile_contains_trumps) {
    Table table;
    table.trump = Value::HEARTS;
    table.bfPlayers = 0b1100;
    // A trump in the pile; having a heart with a matching number: Play it
    BOOST_TEST(toString(selectCardsToPlay(2, cards("S5 H6 DQ"), cards("D3 H5 C6"), 1, 0, table)) == "1-1");
}

BOOST_AUTO_TEST_CASE(beat_with_serie) {
    const Table table(partTwo(Value::CLUBS));
    BOOST_TEST(toString(selectCardsToPlay(1, cards("D5 D6 D7 S3 C2"), cards("D4"), 0, 0, table)) == "0-2");
    BOOST_TEST(toString(selectCardsToPlay(1, cards("D2 D5 D6 S3 C2"), cards("D4"), 0, 0, table)) == "1-2");
}

BOOST_AUTO_TEST_CASE(pick_up_instead_of_few_trumps) {
    Table table(partTwo(Value::CLUBS));
    const Card::Cards hand(cards("D3 S3 HA C2 C9"));
    BOOST_TEST(toString(selectCardsToPlay(1, hand, cards("D4"), 0, 0, table)) == "-");
    table.bfPlayers = 0b0010; // The last player uses his smallest trump
    BOOST_TEST(toString(selectCardsToPlay(1, hand, cards("D4"), 0, 0, table)) == "3-3");
}

BOOST_AUTO_TEST_CASE(play_trumps_if_only_trumps) {
    const Table table(partTwo(Value::CLUBS));
    BOOST_TEST(toString(selectCardsToPlay(1, cards("C2 C3 C9"), cards("D4"), 0, 0, table)) == "0-1");
}

BOOST_AUTO_TEST_CASE(pick_up_if_trump_cant_be_beaten) {
    const Table table(partTwo(Value::CLUBS));
    BOOST_TEST(toString(selectCardsToPlay(1, cards("D3 C2"), cards("C5"), 0, 0, table)) == "-");
    BOOST_TEST(toString(selectCardsToPlay(1, cards("D3 C2 C7"), cards("C5"), 0, 0, table)) == "2-2");
}

BOOST_AUTO_TEST_CASE(varies_moves_in_long_rounds) {
    // Regression: Endless game (seed 12 of the simulation): The players beat
    // the C9 alternately with CT and CQ, the next one picks it up, ...
    Table table(partTwo(Value::CLUBS));
    const Card::Cards hand(cards("D4 D5 D6 S4 H2 H9 HT HQ C8 CQ"));
    const Card::Cards played(cards("S5 S6 S7 C9"));
    BOOST_TEST(toString(selectCardsToPlay(0, hand, played, 0, 0, table)) == "9-9");

    table.pickUps = PICKUPS_TO_VARY;
    std::set<std::string> moves;
    for (unsigned int seed(1); seed < 100; ++seed) {
        Card::seedRandom(seed);
        const auto play(selectCardsToPlay(0, hand, played, 0, 0, table));
        if (play)
            BOOST_TEST(checkPlay(hand, play->start, play->end, played, table) == PlayError::NONE);
        moves.insert(toString(play));
    }
    BOOST_TEST(moves.contains("-"));
    BOOST_TEST(moves.contains("9-9"));

    // Regression (seed 48710): The players lead their smallest card, the next
    // one picks it up, ...
    const Card::Cards lead(cards("S8 CK"));
    moves.clear();
    for (unsigned int seed(1); seed < 100; ++seed) {
        Card::seedRandom(seed);
        moves.insert(toString(selectCardsToPlay(2, lead, cards(""), 0, 0, table)));
    }
    BOOST_TEST((moves == std::set<std::string>{"0-0", "1-1"}));
}

BOOST_AUTO_TEST_CASE(start_with_smallest_serie) {
    const Table table(partTwo(Value::CLUBS));
    const Card::Cards hand(cards("D9 DT S2 H2 H3 H4 C2"));
    BOOST_TEST(findSmallestCard(hand, Value::CLUBS) == 3u); // The longest serie starting with a two
    BOOST_TEST(toString(selectCardsToPlay(1, hand, cards(""), 0, 0, table)) == "3-5");
    // Trumps are only played if there is nothing else
    BOOST_TEST(toString(selectCardsToPlay(1, cards("D9 C2"), cards(""), 0, 0, table)) == "0-0");
    BOOST_TEST(toString(selectCardsToPlay(1, cards("C2 C3 C5"), cards(""), 0, 0, table)) == "0-1");
}

BOOST_AUTO_TEST_SUITE_END()
