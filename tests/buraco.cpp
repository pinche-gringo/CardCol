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

// Unit tests of the rules of Buraco

#define BOOST_TEST_MODULE Buraco
#include <boost/test/unit_test.hpp>

#include "BuracoRules.h"

#include "TestUtil.h"

using namespace BuracoRules;
using Card::Value;
using Test::card;
using Test::cards;

namespace BuracoRules {
std::ostream& operator<<(std::ostream& out, PlayError error) { return out << static_cast<int>(error); }
std::ostream& operator<<(std::ostream& out, HandStatus status) { return out << static_cast<int>(status); }
} // namespace BuracoRules

namespace {

/// Creates a table, where player 0 has the passed cards and his team the passed piles
Table tableWith(const char* hand, std::initializer_list<const char*> piles = {}) {
    Table table;
    table.hands[0] = cards(hand);
    for (const char* p : piles)
        table.piles[0].push_back(cards(p));
    table.dumped = 1;
    return table;
}

} // namespace

BOOST_AUTO_TEST_CASE(jokers_and_points) {
    BOOST_TEST(isJoker(card("C2")));
    BOOST_TEST(isJoker(card("Jo")));
    BOOST_TEST(!isJoker(card("C3")));
    BOOST_TEST(!isJoker(card("SA")));

    BOOST_TEST(pointsOf(card("H2")) == 25u);
    BOOST_TEST(pointsOf(card("H3")) == 5u);
    BOOST_TEST(pointsOf(card("H8")) == 10u);
    BOOST_TEST(pointsOf(card("HA")) == 20u);
    BOOST_TEST(pointsOf(card("Jo")) == 50u);
    BOOST_TEST(pointsOf(cards("H2 H3 HK Jo")) == 90u);

    BOOST_TEST(containsOnlyJoker(cards("")));
    BOOST_TEST(containsOnlyJoker(cards("C2 Jo")));
    BOOST_TEST(!containsOnlyJoker(cards("C2 C3")));
    BOOST_TEST(containsNoJoker(cards("C3 HA")));
    BOOST_TEST(!containsNoJoker(cards("C3 D2")));
}

BOOST_AUTO_TEST_CASE(distance_of_cards) {
    BOOST_TEST(cardDistance(card("H5"), card("H3")) == 2);
    BOOST_TEST(cardDistance(card("H3"), card("H5")) == -2);
    BOOST_TEST(cardDistance(card("H5"), card("C5")) == 0);
    BOOST_TEST(cardDistance(card("H5"), card("C6")) == 99);
    BOOST_TEST(cardDistance(card("C2"), card("Jo")) == 0);
    BOOST_TEST(cardDistance(card("C2"), card("C3")) == 99);

    // The ace is directly before the 3 (as 2s are always jokers) or after the king
    BOOST_TEST(cardDistance(card("HA"), card("H3")) == -1);
    BOOST_TEST(cardDistance(card("H3"), card("HA")) == 1);
    BOOST_TEST(cardDistance(card("H3"), card("HA"), false) == -11);
    BOOST_TEST(cardDistance(card("HA"), card("HK")) == 1);
}

BOOST_AUTO_TEST_CASE(sorting) {
    Card::Cards hand(cards("Jo C2 SA H3 D3"));
    std::ranges::stable_sort(hand, lessByNumberWithJokers);
    BOOST_TEST(hand == cards("H3 D3 SA C2 Jo"), hand);

    hand = cards("Jo C2 SA H3 C5");
    std::ranges::sort(hand, lessByColourWithJokers);
    BOOST_TEST(hand == cards("C5 SA H3 C2 Jo"), hand);

    hand = cards("H3 C5 C5 HK");
    BOOST_TEST(insertSorted(hand, card("D5")) == 3u);
    BOOST_TEST(hand == cards("H3 C5 C5 D5 HK"), hand);
    BOOST_TEST(findSorted(hand, card("S5")) == 1);
    BOOST_TEST(findSorted(hand, card("SA")) == -1);
}

BOOST_AUTO_TEST_CASE(moving_cards_tracks_positions) {
    Card::Cards hand(cards("C3 C4 C5 C6"));
    unsigned int tracked(1);
    moveCard(hand, 3, 0, &tracked);
    BOOST_TEST(hand == cards("C4 C5 C6 C3"), hand);
    BOOST_TEST(tracked == 0u);

    tracked = 3;
    moveCard(hand, 0, 3, &tracked);
    BOOST_TEST(hand == cards("C3 C4 C5 C6"), hand);
    BOOST_TEST(tracked == 0u);

    tracked = 2;
    moveCard(hand, 0, 3, &tracked);
    BOOST_TEST(tracked == 3u);
}

BOOST_AUTO_TEST_CASE(fitting_pairs) {
    BOOST_TEST(hasFittingPair(cards("H3 H4"), card("H5")));
    BOOST_TEST(hasFittingPair(cards("H4 H6"), card("H5")));
    BOOST_TEST(hasFittingPair(cards("C5 D5"), card("H5")));
    BOOST_TEST(!hasFittingPair(cards("C5 D7"), card("H5")));
    BOOST_TEST(!hasFittingPair(cards("H3 H7"), card("H5")));
    BOOST_TEST(hasFittingPair(cards("HQ HK"), card("HA")));
    BOOST_TEST(hasFittingPair(cards("H3 H4"), card("HA")));
    BOOST_TEST(!hasFittingPair(cards("HK H3"), card("HA")), "K-A-3 is not a series");

    // The card itself is not counted
    BOOST_TEST(!hasFittingPair(cards("H5 C5"), card("H5"), 0));
    BOOST_TEST(hasFittingPair(cards("H5 C5 S5"), card("H5"), 0));

    // With jokers a fitting card is enough
    BOOST_TEST(pileHasFittingPair(cards("C9 C10 Jo"), card("C9"), 0, true));
    BOOST_TEST(!pileHasFittingPair(cards("C9 C10 Jo"), card("C9"), 0, false));
    BOOST_TEST(!pileHasFittingPair(cards("C9 C10 H4"), card("C9"), 0, true));

    BOOST_TEST(pileHasFittingPair(cards("C9 SK C10")));
    BOOST_TEST(!pileHasFittingPair(cards("C9 SK HQ")));
    BOOST_TEST(pileHasFittingPair(cards("C9 SK C10"), 2), "Excluded card is not inspected (but found as partner)");
    BOOST_TEST(!pileHasFittingPair(cards("C9 SK"), 1));
}

BOOST_AUTO_TEST_CASE(series_in_hand) {
    const Card::Cards hand(cards("H3 H4 H5 C5 S5 SK"));
    Series series(getSeries(hand, 2));
    BOOST_TEST(series.equal == 3u);
    BOOST_TEST(series.positions.size() == 3u);
    BOOST_TEST(series.positions[0] == 0u);
    BOOST_TEST(series.positions[1] == 1u);
    BOOST_TEST(series.positions[2] == 2u);

    Card::Cards sorted(hand);
    unsigned int tracked(2);
    const unsigned int first(sortColourSerie(sorted, series, &tracked));
    BOOST_TEST(first == 3u);
    BOOST_TEST(Card::Cards(sorted.begin() + first, sorted.end()) == cards("H3 H4 H5"), sorted);
    BOOST_TEST(tracked == 5u);

    // Cards without direct contact are not part of the series
    series = getSeries(cards("H3 H5 H6"), 2);
    BOOST_TEST(series.positions.size() == 2u);
    BOOST_TEST(series.equal == 1u);
}

BOOST_AUTO_TEST_CASE(analysing_piles) {
    PileInfo info(analysePile(cards("H3 H4 H5")));
    BOOST_TEST(info.type == COLOUR);
    BOOST_TEST(info.posFirst == 0u);
    BOOST_TEST(info.posLast == 2u);
    BOOST_TEST(info.posJoker == NONE);
    BOOST_TEST(info.points == 400u);

    info = analysePile(cards("H3 Jo H5"));
    BOOST_TEST(info.posJoker == 1u);
    BOOST_TEST(info.points == 200u);

    BOOST_TEST(analysePile(cards("HA CA SA")).points == 500u);
    BOOST_TEST(analysePile(cards("HA C2 SA")).points == 300u);
    BOOST_TEST(analysePile(cards("HA CA SA")).type == NUMBER);

    info = analysePile(cards("C2 H2 D2"));
    BOOST_TEST(info.type == NUMBER);
    BOOST_TEST(info.posFirst == NONE);
    BOOST_TEST(info.points == 2000u);
    BOOST_TEST(analysePile(cards("C2 Jo D2")).points == 1000u, "Sucio");

    BOOST_TEST(pilePoints(cards("H3 H4 H5 H6 H7 H8 H9")) == 400);
    BOOST_TEST(pilePoints(cards("H3 H4 H5 Jo H7 H8 H9")) == 200);
    BOOST_TEST(pilePoints(cards("H3 H4 H5 H6 H7")) == 0);
    BOOST_TEST(pilePoints(cards("C2 H2 D2 Jo")) == -1000);
    BOOST_TEST(pilePoints(cards("C2 H2 D2 S2 C2 H2 D2")) == 2000);
}

BOOST_AUTO_TEST_CASE(position_of_card_in_pile) {
    unsigned int pos(0), move(0);
    const Card::Cards run(cards("H4 H5 H6"));
    BOOST_TEST(getPosition4Card(run, card("H7"), pos, move));
    BOOST_TEST(pos == 3u);
    BOOST_TEST(move == -1U);
    BOOST_TEST(getPosition4Card(run, card("H3"), pos, move));
    BOOST_TEST(pos == 0u);
    BOOST_TEST(!getPosition4Card(run, card("H9"), pos, move));
    BOOST_TEST(!getPosition4Card(run, card("C7"), pos, move));
    BOOST_TEST(getPosition4Card(run, card("Jo"), pos, move));
    BOOST_TEST(pos == 0u);

    BOOST_TEST(getPosition4Card(cards("C4 D4 S4"), card("H4"), pos, move));
    BOOST_TEST(pos == 3u);
    BOOST_TEST(!getPosition4Card(cards("C4 D4 S4"), card("H5"), pos, move));

    // Card replacing the joker: The joker is moved to the start
    BOOST_TEST(getPosition4Card(cards("H4 Jo H6"), card("H5"), pos, move));
    BOOST_TEST(move == 0u);
    BOOST_TEST(pos == 2u);

    // Normal card on piles of monos
    BOOST_TEST(getPosition4Card(cards("Jo"), card("H9"), pos, move));
    BOOST_TEST(pos == 0u);
    BOOST_TEST(!getPosition4Card(cards("C2 C2 Jo"), card("H9"), pos, move), "Regression: accessed not existing card");
    BOOST_TEST(getPosition4Card(cards("C2 C2 Jo"), card("D2"), pos, move));
}

BOOST_AUTO_TEST_CASE(valid_piles) {
    BOOST_TEST(isValidPile(cards("H3 H4 H5")));
    BOOST_TEST(isValidPile(cards("HA H3 H4")));
    BOOST_TEST(isValidPile(cards("HQ HK HA")));
    BOOST_TEST(isValidPile(cards("H4 Jo H6")));
    BOOST_TEST(isValidPile(cards("Jo H4 H5")));
    BOOST_TEST(isValidPile(cards("C9 H9 Jo S9")));
    BOOST_TEST(isValidPile(cards("C2 C2 Jo")));
    BOOST_TEST(!isValidPile(cards("HK HA H3")));
    BOOST_TEST(!isValidPile(cards("Jo S4 S5 S6 S8")));
    BOOST_TEST(!isValidPile(cards("H4 Jo Jo H7")));
    BOOST_TEST(!isValidPile(cards("H4 H6 H5")));
    BOOST_TEST(!isValidPile(cards("H4 C5 H6")));
    BOOST_TEST(!isValidPile(cards("H3 H4 H5 H6 H7 H8 H9 H10")));
}

BOOST_AUTO_TEST_CASE(getting_rid_of_cards) {
    BOOST_TEST(!canGetRidOfCards(cards("H3 H3 C9")));
    BOOST_TEST(canGetRidOfCards(cards("H3 H3 C9 Jo")));
    BOOST_TEST(canGetRidOfCards(cards("H3 H3 Jo")));
    BOOST_TEST(canGetRidOfCards(cards("H3 C7 C9 Jo")), "C7 Jo C9");
    BOOST_TEST(!canGetRidOfCards(cards("H3 C7 D9 Jo")));
}

BOOST_AUTO_TEST_CASE(playing_cards_without_reserve) {
    Table table(tableWith("H3 H3 H3 C9", {"S4 S5 S6"}));
    table.reserve = {false, false};

    // Playing the last cards needs a cerrado (or closing a pile)
    BOOST_TEST(!canPlayCards(table, 0, 3));
    BOOST_TEST(canPlayCards(table, 0, 1, 0));
    table.points[0] = 200;
    BOOST_TEST(canPlayCards(table, 0, 3));

    // With the reserve everything can be played
    table.points[0] = 0;
    table.reserve[0] = true;
    BOOST_TEST(canPlayCards(table, 0, 3));

    // Unfinished piles of monos prevent ending the game
    table.reserve[0] = false;
    table.points[0] = 200;
    table.unfinishedMonoPiles[0] = 1;
    BOOST_TEST(!canPlayCards(table, 0, 3));
}

BOOST_AUTO_TEST_CASE(closing_pile_with_last_cards) {
    Table table(tableWith("H8 H9", {"H3 H4 H5 H6 H7"}));
    table.reserve = {false, false};
    BOOST_TEST(canClosePile(table, 0, 0));
    BOOST_TEST(canPlayCards(table, 0, 1, 0));

    table.hands[0] = cards("H8 C9");
    BOOST_TEST(!canClosePile(table, 0, 0));
    BOOST_TEST(!canPlayCards(table, 0, 1, 0));

    // Regression: A normal card checked against a pile of monos
    table.piles[0][0] = cards("C2 D2 C2 H2 S2");
    table.hands[0] = cards("HJ Jo");
    BOOST_TEST(!canClosePile(table, 0, 0));
}

BOOST_AUTO_TEST_CASE(card_fits_on_pile) {
    const Table table(tableWith("H7 HJ C2 C9 D9", {"H4 H5 H6", "Jo", "C2 H2 Jo"}));
    BOOST_TEST(cardFitsOnPile(table, 0, 0, 0) == 3);
    BOOST_TEST(cardFitsOnPile(table, 0, 0, 1) == -1);
    BOOST_TEST(cardFitsOnPile(table, 0, 0, 2) == 0);

    // On a single joker only cards having a pair
    BOOST_TEST(cardFitsOnPile(table, 0, 1, 3) == 0);
    BOOST_TEST(cardFitsOnPile(table, 0, 1, 0) == -1);

    // On monos only jokers
    BOOST_TEST(cardFitsOnPile(table, 0, 2, 2) == 0);
    BOOST_TEST(cardFitsOnPile(table, 0, 2, 3) == -1);
}

BOOST_AUTO_TEST_CASE(checking_pick_up) {
    Table table(tableWith("H5 C5 SK"));
    table.dumped = 3;
    BOOST_TEST(checkPickUp(table, 0, card("D5")) == PlayError::NONE);
    BOOST_TEST(checkPickUp(table, 0, card("Jo")) == PlayError::PICK_UP_MONO);
    BOOST_TEST(checkPickUp(table, 0, card("D2")) == PlayError::PICK_UP_MONO);
    BOOST_TEST(checkPickUp(table, 0, card("D9")) == PlayError::PICK_UP_NO_PAIR);

    table.hands[0] = cards("H5 C5");
    table.dumped = 2;
    BOOST_TEST(checkPickUp(table, 0, card("D5")) == PlayError::NONE);
    table.reserve[0] = false;
    BOOST_TEST(checkPickUp(table, 0, card("D5")) == PlayError::PICK_UP_WOULD_END);
    table.points[0] = 200;
    BOOST_TEST(checkPickUp(table, 0, card("D5")) == PlayError::NONE);

    // The first card of the game can always be taken
    table.startGame = true;
    BOOST_TEST(checkPickUp(table, 0, card("Jo")) == PlayError::NONE);
}

BOOST_AUTO_TEST_CASE(checking_new_pile) {
    Table table(tableWith("H3 H3 H3 C9 C10 D7 SK"));
    BOOST_TEST(checkNewPile(table, 0, 0) == PlayError::NONE);
    BOOST_TEST(checkNewPile(table, 0, 3) == PlayError::NO_VALID_NEW_PILE);
    BOOST_TEST(checkNewPile(table, 0, 6) == PlayError::NO_VALID_NEW_PILE);

    table.hands[0].push_back(card("Jo"));
    BOOST_TEST(checkNewPile(table, 0, 3) == PlayError::NONE, "Pair with joker");
    table.pickUpPlayed = true;
    BOOST_TEST(checkNewPile(table, 0, 3) == PlayError::NO_VALID_NEW_PILE, "Not with a joker for the picked up pile");

    table = tableWith("H3 H3 H3 C9", {"S4 S5"});
    BOOST_TEST(checkNewPile(table, 0, 0) == PlayError::FILL_OTHER_PILES);
    table.piles[0][0].push_back(card("S6"));
    table.reserve[0] = false;
    BOOST_TEST(checkNewPile(table, 0, 0) == PlayError::NO_CERRADO);
    table.unfinishedMonoPiles[0] = 1;
    BOOST_TEST(checkNewPile(table, 0, 0) == PlayError::MONO_PILE_UNFINISHED);
}

BOOST_AUTO_TEST_CASE(checking_adding_card) {
    Table table(tableWith("H7 H9 Jo SK", {"H4 H5 H6", "C8 D8"}));
    BOOST_TEST(checkAddToPile(table, 0, 0, 0) == PlayError::FILL_OTHER_PILES);
    BOOST_TEST(checkAddToPile(table, 0, 0, 1) == PlayError::CARD_DOES_NOT_FIT, "The pile to play to may be incomplete");
    table.piles[0][1].push_back(card("S8"));
    BOOST_TEST(checkAddToPile(table, 0, 0, 0) == PlayError::NONE);
    BOOST_TEST(checkAddToPile(table, 0, 1, 0) == PlayError::CARD_DOES_NOT_FIT);
    BOOST_TEST(checkAddToPile(table, 0, 2, 0) == PlayError::NONE);
    table.pickUpPlayed = true;
    BOOST_TEST(checkAddToPile(table, 0, 2, 0) == PlayError::NO_JOKER_ON_NEW_PILE);

    table = tableWith("H7 C9", {"H4 H5 H6"});
    table.reserve[0] = false;
    BOOST_TEST(checkAddToPile(table, 0, 0, 0) == PlayError::NO_CERRADO);

    BOOST_TEST(checkDump(table, 0) == PlayError::NONE);
    table.piles[0].push_back(cards("C9 D9"));
    BOOST_TEST(checkDump(table, 0) == PlayError::PILES_INCOMPLETE);
    BOOST_TEST(std::string(describe(PlayError::PILES_INCOMPLETE)) == "Every pile on the table must have at least 3 cards!");
}

BOOST_AUTO_TEST_CASE(dealing) {
    const Card::Cards deck(Test::shuffledDeck(1, NUM_DECKS, NUM_JOKERS));
    BOOST_TEST(deck.size() == 220u);

    const Deal cards(deal(deck, 11));
    Card::Cards all(cards.staple);
    for (const auto& hand : cards.hands) {
        BOOST_TEST(hand.size() == 12u);
        BOOST_TEST(std::ranges::is_sorted(hand, lessByNumberWithJokers));
        all.insert(all.end(), hand.begin(), hand.end());
    }
    for (const auto& reserve : cards.reserve) {
        BOOST_TEST(reserve.size() == 11u);
        all.insert(all.end(), reserve.begin(), reserve.end());
    }
    all.push_back(cards.dumped);
    BOOST_TEST(cards.staple.size() == 220u - 4 * 12 - 2 * 11 - 1);
    BOOST_TEST(Test::sameCards(all, deck));

    // The first player gets the top cards of the staple
    BOOST_TEST(Test::sameCards(cards.hands[0], Card::Cards(deck.end() - 12, deck.end())));
    BOOST_TEST(cards.dumped == deck[deck.size() - 4 * 12 - 2 * 11 - 1]);
}

BOOST_AUTO_TEST_CASE(end_of_hand) {
    BOOST_TEST(handStatus(cards("H3 Jo"), true) == HandStatus::PLAYING);
    BOOST_TEST(handStatus(cards("C2 Jo"), true) == HandStatus::TAKE_RESERVE);
    BOOST_TEST(handStatus(cards(""), true) == HandStatus::TAKE_RESERVE);
    BOOST_TEST(handStatus(cards("C2 Jo"), false) == HandStatus::PLAYING);
    BOOST_TEST(handStatus(cards(""), false) == HandStatus::GOING_OUT);

    Card::Cards hand(cards("Jo"));
    takeReserve(hand, cards("SK C2 H3"));
    BOOST_TEST(hand == cards("H3 SK C2 Jo"), hand);
}

BOOST_AUTO_TEST_CASE(scoring) {
    Table table;
    table.reserve = {false, true};
    table.points = {200 + GOING_OUT_BONUS, 0};
    table.piles[0] = {cards("H3 H4 H5 Jo H7 H8 H9"), cards("C9 D9 S9")};
    table.piles[1] = {cards("C2 D2 Jo"), cards("SK SQ SJ")};
    table.hands[1] = cards("H3 Jo");
    table.hands[2] = cards("HA");
    table.hands[3] = cards("SA");

    const RoundScore score(roundScore(table));
    BOOST_TEST(score.bonus[0] == 400);
    BOOST_TEST(score.bonus[1] == -100);
    BOOST_TEST(score.cards[0] == (5 * 4 + 50 + 10 * 2 + 30) - 20);
    // The team without cerrado gets the points of the table negative, and
    // -1000 for the started pile of monos
    BOOST_TEST(score.cards[1] == -(100 + 30) - 1000 - (55 + 20));
}

BOOST_AUTO_TEST_CASE(computer_takes_dumped_cards) {
    Table table(tableWith("H5 C5 SK"));
    table.dumped = 3;
    BOOST_TEST(takeDumped(table, 0, card("D5")));
    BOOST_TEST(!takeDumped(table, 0, card("Jo")));
    BOOST_TEST(!takeDumped(table, 0, card("D9")));

    // The first card is taken, if it fits (or is a joker)
    table.startGame = true;
    BOOST_TEST(takeDumped(table, 0, card("Jo")));
    BOOST_TEST(takeDumped(table, 0, card("H6")));
    BOOST_TEST(!takeDumped(table, 0, card("D6")));
    BOOST_TEST(!takeDumped(table, 0, card("D9")));
}

BOOST_AUTO_TEST_CASE(computer_plays_taken_card) {
    Table table(tableWith("H3 C3 D9 SK"));
    table.dumped = 3;
    PickUp cards(playPickedUp(table, 0, card("S3")));
    BOOST_TEST(table.hands[0] == Test::cards("H3 C3 D9 SK"), table.hands[0]);
    BOOST_TEST(cards.first == 0u);
    BOOST_TEST(cards.last == 1u);
    BOOST_TEST(cards.posTaken == 2u);

    // Series of a colour
    table = tableWith("H4 H5 C9 SK");
    table.dumped = 3;
    cards = playPickedUp(table, 0, card("H6"));
    BOOST_TEST(Card::Cards(table.hands[0].begin() + cards.first, table.hands[0].begin() + cards.last + 1) == Test::cards("H4 H5"),
               table.hands[0]);
    BOOST_TEST(cards.posTaken == 2u);

    // Regression: Taking the card with only 2 cards of the same number in the
    // hand must play them both (else the pile would have only 2 cards)
    table = tableWith("D3 S3");
    table.dumped = 2;
    cards = playPickedUp(table, 0, card("H3"));
    BOOST_TEST((cards.last - cards.first + 2) == 3u);
}

BOOST_AUTO_TEST_CASE(computer_plays_on_existing_pile) {
    Table table(tableWith("C9 H7 D10 SK", {"H4 H5 H6"}));
    const Move move(selectMove(table, 0));
    BOOST_TEST(move.kind == Move::ADD_TO_PILE);
    BOOST_TEST(move.pile == 0u);
    BOOST_TEST(move.pos == 3u);
    BOOST_TEST(table.hands[0][move.first] == card("H7"));
    BOOST_TEST(move.jokerMoves.empty());
}

BOOST_AUTO_TEST_CASE(computer_replaces_joker) {
    Table table(tableWith("C9 H5 D10 SK", {"H4 Jo H6"}));
    const Move move(selectMove(table, 0));
    BOOST_TEST(move.kind == Move::ADD_TO_PILE);
    BOOST_TEST(table.hands[0][move.first] == card("H5"));
    BOOST_TEST(move.jokerMoves.size() == 1u);
    BOOST_TEST(table.piles[0][0] == cards("Jo H4 H6"), table.piles[0][0]);
    BOOST_TEST(move.pos == 2u);
}

BOOST_AUTO_TEST_CASE(computer_keeps_pile_valid) {
    // Regression: The 7 would replace the joker, but then the pile can't be
    // closed anymore (so the card is not played): The joker must stay
    Table table(tableWith("S7 SA", {"S4 S5 S6 Jo S8"}));
    table.reserve = {false, false};
    const Move move(selectMove(table, 0));
    for (const auto& pile : table.piles[0])
        BOOST_TEST(isValidPile(pile), pile);
    if (move.kind != Move::ADD_TO_PILE) {
        BOOST_TEST(table.piles[0][0] == cards("S4 S5 S6 Jo S8"), table.piles[0][0]);
        BOOST_TEST(move.jokerMoves.empty());
    }
}

BOOST_AUTO_TEST_CASE(computer_makes_new_pile) {
    Table table(tableWith("C9 D9 S9 HK"));
    Move move(selectMove(table, 0));
    BOOST_TEST(move.kind == Move::NEW_PILE);
    BOOST_TEST(move.pile == 0u);
    BOOST_TEST(Card::Cards(table.hands[0].begin() + move.first, table.hands[0].begin() + move.last + 1) == cards("C9 D9 S9"));

    // Series of a colour
    table = tableWith("H4 C9 H5 HK H6");
    move = selectMove(table, 0);
    BOOST_TEST(move.kind == Move::NEW_PILE);
    BOOST_TEST(Card::Cards(table.hands[0].begin() + move.first, table.hands[0].begin() + move.last + 1) == cards("H4 H5 H6"),
               table.hands[0]);

    // A pair with a joker, if all cards can be played
    table = tableWith("C9 D9 Jo");
    move = selectMove(table, 0);
    BOOST_TEST(move.kind == Move::NEW_PILE);
    BOOST_TEST(move.last - move.first == 2u);
    BOOST_TEST(isValidPile(table.hands[0]), table.hands[0]);
}

BOOST_AUTO_TEST_CASE(computer_dumps_card) {
    Table table(tableWith("C9 HK"));
    Move move(selectMove(table, 0));
    BOOST_TEST(move.kind == Move::DUMP);
    BOOST_TEST(table.hands[0][move.first] == card("C9"));

    // Not a card with a fitting one
    table = tableWith("C9 C10 HK");
    move = selectMove(table, 0);
    BOOST_TEST(move.kind == Move::DUMP);
    BOOST_TEST(table.hands[0][move.first] == card("HK"));

    // Try to not dump jokers
    table = tableWith("C9 C9 Jo");
    table.reserve[0] = false;
    table.unfinishedMonoPiles[0] = 1;
    move = selectMove(table, 0);
    BOOST_TEST(move.kind == Move::DUMP);
    BOOST_TEST(!isJoker(table.hands[0][move.first]));
}
