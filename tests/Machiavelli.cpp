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

// Unit tests of the rules of Machiavelli

#define BOOST_TEST_MODULE Machiavelli
#include <boost/test/unit_test.hpp>

#include <cstring>

#include "MachiavelliRules.h"

#include "TestUtil.h"

using namespace MachiavelliRules;
using Card::Value;
using Test::card;
using Test::cards;

namespace MachiavelliRules {
std::ostream& operator<<(std::ostream& out, PileError error) { return out << static_cast<int>(error); }
std::ostream& operator<<(std::ostream& out, MoveError error) { return out << static_cast<int>(error); }
std::ostream& operator<<(std::ostream& out, const Transfer& t) {
    return out << '{' << static_cast<int>(t.pile) << ' ' << t.first << '-' << t.last << " @" << t.destPos << '}';
}
} // namespace MachiavelliRules

namespace {

/// Creates a pile out of the passed cards (as they were added one after another)
Pile pile(std::string_view text) { return Pile::of(cards(text)); }

/// Creates a table out of the passed piles
Table table(std::initializer_list<std::string_view> piles) {
    Table result;
    for (const auto& p : piles)
        result.push_back(pile(p));
    return result;
}

/// Result of getSeries (and optionally sortColourSerie)
struct Serie {
    unsigned int nrs;
    std::map<unsigned int, unsigned int> aPos;
    std::vector<unsigned int> aOrder;
    Card::Cards cards;
    unsigned int posCard;
    unsigned int start;
};

Serie series(std::string_view text, unsigned int pos, bool doubles, bool sort) {
    Serie s{0, {}, {}, cards(text), pos, -1U};
    s.nrs = getSeries(s.cards, s.posCard, s.aPos, s.aOrder, doubles);
    if (sort)
        s.start = sortColourSerie(s.cards, s.aPos, s.aOrder);
    return s;
}

} // namespace

BOOST_AUTO_TEST_CASE(card_distance) {
    BOOST_TEST(cardDistance(card("H5"), card("S5")) == 0);
    BOOST_TEST(cardDistance(card("H5"), card("S6")) == 99);
    BOOST_TEST(cardDistance(card("H7"), card("H5")) == 2);
    BOOST_TEST(cardDistance(card("H5"), card("H7")) == -2);
    BOOST_TEST(cardDistance(card("H5"), card("H5")) == 0);

    // The ace counts as one next to a two or three
    BOOST_TEST(cardDistance(card("HA"), card("H2")) == -1);
    BOOST_TEST(cardDistance(card("H2"), card("HA")) == 1);
    BOOST_TEST(cardDistance(card("H3"), card("HA")) == 2);
    BOOST_TEST(cardDistance(card("H4"), card("HA")) == -10);
    BOOST_TEST(cardDistance(card("HA"), card("HK")) == 1);
    BOOST_TEST(cardDistance(card("H2"), card("HA"), ACE) == -12);
    BOOST_TEST(cardDistance(card("H9"), card("HA"), ONE) == 8);
    BOOST_TEST(cardDistance(card("HA"), card("H9"), ONE) == -8);
}

BOOST_AUTO_TEST_CASE(pile_type) {
    BOOST_TEST(pile("").type == UNDEFINED);
    BOOST_TEST(pile("H5").type == UNDEFINED);
    BOOST_TEST(pile("H5 S5").type == NUMBER);
    BOOST_TEST(pile("H5 H6").type == COLOUR);
    BOOST_TEST(pile("H5 S5 C5").type == NUMBER);

    // The type is only defined when the pile has two cards
    Pile p(pile("H5 H6 H7"));
    BOOST_TEST(p.remove(0) == card("H5"));
    BOOST_TEST(p.type == COLOUR);
    p.insert(card("S6"), 0);
    BOOST_TEST(p.type == COLOUR);
    p.remove(2);
    BOOST_TEST(p.type == NUMBER);
    p.remove(0);
    BOOST_TEST(p.type == UNDEFINED);
}

BOOST_AUTO_TEST_CASE(position_for_card) {
    BOOST_TEST(getPosition4Card(pile(""), card("C2")) == 0U);

    // Numbered piles: Append cards with the same number, but not twice the same card
    BOOST_TEST(getPosition4Card(pile("H5 S5 C5"), card("D5")) == 3U);
    BOOST_TEST(getPosition4Card(pile("H5 S5 C5"), card("H5")) == -1U);
    BOOST_TEST(getPosition4Card(pile("H5 S5 C5"), card("H6")) == -1U);
    BOOST_TEST(getPosition4Card(pile("H5"), card("C5")) == 1U);
    BOOST_TEST(getPosition4Card(pile("H5"), card("H6")) == 1U);
    BOOST_TEST(getPosition4Card(pile("H5"), card("H4")) == 0U);

    // Coloured piles: At the borders
    BOOST_TEST(getPosition4Card(pile("H5 H6 H7"), card("H4")) == 0U);
    BOOST_TEST(getPosition4Card(pile("H5 H6 H7"), card("H8")) == 3U);
    BOOST_TEST(getPosition4Card(pile("H5 H6 H7"), card("H6")) == -1U);
    BOOST_TEST(getPosition4Card(pile("H5 H6 H7"), card("S8")) == -1U);
    BOOST_TEST(getPosition4Card(pile("H5 H6 H7"), card("S5")) == -1U);

    // Aces either below the two or above the king
    BOOST_TEST(getPosition4Card(pile("H2 H3 H4"), card("HA")) == 0U);
    BOOST_TEST(getPosition4Card(pile("HJ HQ HK"), card("HA")) == 3U);
    BOOST_TEST(getPosition4Card(pile("HA H2 H3"), card("HA")) == -1U);
    BOOST_TEST(getPosition4Card(pile("HQ HK HA"), card("H2")) == -1U);
    BOOST_TEST(getPosition4Card(pile("HA H2 H3"), card("H4")) == 3U);
}

BOOST_AUTO_TEST_CASE(position_of_colour) {
    BOOST_TEST(getPosOfColour(pile("H5 S5 C5"), Value::SPADES) == 1);
    BOOST_TEST(getPosOfColour(pile("H5 S5 C5"), Value::DIAMONDS) == -1);
    BOOST_TEST(getPosOfColour(pile("H5 H6 H7"), Value::HEARTS) == 0);
    BOOST_TEST(getPosOfColour(pile("H5 H6 H7"), Value::SPADES) == -1);
}

BOOST_AUTO_TEST_CASE(check_piles) {
    unsigned int pos(0);
    BOOST_TEST(checkPile(pile("H5 H6 H7")) == PileError::NONE);
    BOOST_TEST(checkPile(pile("HA H2 H3")) == PileError::NONE);
    BOOST_TEST(checkPile(pile("HQ HK HA")) == PileError::NONE);
    BOOST_TEST(checkPile(pile("H5 S5 C5 D5")) == PileError::NONE);
    BOOST_TEST(checkPile(pile("H5 H6")) == PileError::NOT_ENOUGH_CARDS);
    BOOST_TEST(checkPile(Pile(cards("H5 H6 H7"), UNDEFINED)) == PileError::INVALID_TYPE);
    BOOST_TEST(checkPile(pile("HK HA H2"), pos) == PileError::CARD_DOES_NOT_FIT);
    BOOST_TEST(pos == 1U);
    BOOST_TEST(checkPile(pile("H5 H6 H8 H9"), pos) == PileError::CARD_DOES_NOT_FIT);
    BOOST_TEST(pos == 2U);
    BOOST_TEST(checkPile(pile("H5 H6 S7"), pos) == PileError::CARD_DOES_NOT_FIT);
    BOOST_TEST(pos == 2U);
    BOOST_TEST(checkPile(pile("H5 S5 C6"), pos) == PileError::CARD_DOES_NOT_FIT);
    BOOST_TEST(pos == 2U);
    BOOST_TEST(checkPile(pile("H5 S5 H5")) == PileError::NONE); // Not checked: The GUI doesn't allow that

    BOOST_TEST(firstInvalidPile(table({"H5 H6 H7", "S5 C5 D5"})) == -1);
    BOOST_TEST(firstInvalidPile(table({"H5 H6 H7", "S5 C5"})) == 1);

    BOOST_TEST(std::strcmp(describe(PileError::NOT_ENOUGH_CARDS), "Not enough cards (must be at least 3)!") == 0);
    BOOST_TEST(std::strcmp(describe(PileError::CARD_DOES_NOT_FIT), "Card %1 does not fit!") == 0);
    BOOST_TEST(std::strcmp(describe(PileError::NONE), "") == 0);
}

BOOST_AUTO_TEST_CASE(matching_3rd_card) {
    unsigned int match(0), nr(0);

    // Numbered pile: Any card not already in the pair
    Card::Cards pair(cards("H5 S5"));
    BOOST_TEST(hasMatching3rd(pile("H5 C5 D5"), pair, match, nr));
    BOOST_TEST(match == 1U);
    BOOST_TEST(nr == 1U);
    BOOST_TEST(pair.size() == 2U);
    BOOST_TEST(!hasMatching3rd(pile("H6 C6 D6"), pair, match, nr));

    // Coloured pile: Border cards
    pair = cards("H5 H6");
    BOOST_TEST(hasMatching3rd(pile("H7 H8 H9"), pair, match, nr));
    BOOST_TEST(match == 0U);
    pair = cards("H9 HT");
    BOOST_TEST(hasMatching3rd(pile("H5 H6 H7 H8"), pair, match, nr));
    BOOST_TEST(match == 3U);
    BOOST_TEST(nr == 1U);

    // Pair of the same colour: Numbered piles are not used
    pair = cards("H5 H6");
    BOOST_TEST(!hasMatching3rd(pile("S4 H4 C4"), pair, match, nr));

    // A long pile must be split up to get the card (the pair is cleared then)
    pair = cards("H6 H7");
    BOOST_TEST(hasMatching3rd(pile("H2 H3 H4 H5 H6 H7 H8 H9 HT HJ HQ HK"), pair, match, nr));
    BOOST_TEST(pair.empty());
    BOOST_TEST(match == 4U);
    BOOST_TEST(nr == 8U);
}

BOOST_AUTO_TEST_CASE(fitting_card) {
    const Card::Cards hand(cards("S5 H3 H7 H8 C6 H5"));
    BOOST_TEST(getFittingCard(hand, card("H6"), 0) == 2U);
    BOOST_TEST(getFittingCard(hand, card("H6"), 3) == 3U);
    BOOST_TEST(getFittingCard(hand, card("H6"), 4) == 4U); // Same number
    BOOST_TEST(getFittingCard(hand, card("H6"), 5) == 5U);
    BOOST_TEST(getFittingCard(hand, card("H6"), 6) == 6U);
    BOOST_TEST(getFittingCard(hand, card("D9"), 0) == 6U);
    BOOST_TEST(getFittingCard(cards("H2 H5"), card("HA"), 0) == 0U);
}

// The expected values of the following tests have been determined with the
// original implementation in Card::IPile (getSeries and sortColourSerie)
BOOST_AUTO_TEST_CASE(series_of_numbers) {
    // Without doubles: Moves the non-doubles before the doubles
    Serie s(series("C5 C5 H5 S5", 0, false, false));
    BOOST_TEST(s.nrs == 3U);
    BOOST_TEST((s.aPos == std::map<unsigned int, unsigned int>{{2, 0}}));
    BOOST_TEST(s.cards == cards("C5 H5 S5 C5"), "Cards " << s.cards);
    BOOST_TEST(s.posCard == 0U);

    s = series("C5 C5 H5 S5", 0, true, false);
    BOOST_TEST(s.nrs == 4U);
    BOOST_TEST(s.cards == cards("C5 C5 H5 S5"), "Cards unchanged");

    // The (tracked) analysed card is moved as well
    s = series("C5 C5 H5 C5 S5", 1, false, false);
    BOOST_TEST(s.nrs == 3U);
    BOOST_TEST(s.cards == cards("C5 H5 S5 C5 C5"), "Cards " << s.cards);
    BOOST_TEST(s.posCard == 3U);
}

BOOST_AUTO_TEST_CASE(series_of_colours) {
    Serie s(series("C2 C3 C4 C5 C6", 2, true, true));
    BOOST_TEST(s.nrs == 1U);
    BOOST_TEST((s.aPos == std::map<unsigned int, unsigned int>{{0, 0}, {1, 1}, {2, 2}, {3, 3}, {4, 4}}));
    BOOST_TEST((s.aOrder == std::vector<unsigned int>{0, 1, 2, 3, 4}));
    BOOST_TEST(s.start == 0U);
    BOOST_TEST(s.cards == cards("C2 C3 C4 C5 C6"), "Cards " << s.cards);

    // Cards without direct connection to the analysed one are removed
    s = series("C3 C5 C6 C7", 3, true, true);
    BOOST_TEST((s.aPos == std::map<unsigned int, unsigned int>{{0, 1}, {1, 2}, {2, 3}}));
    BOOST_TEST(s.start == 1U);
    BOOST_TEST(s.cards == cards("C3 C5 C6 C7"), "Cards " << s.cards);

    // Aces: Either before the two or after the king (not both)
    s = series("CQ CK CA C2 C3", 2, true, true);
    BOOST_TEST((s.aPos == std::map<unsigned int, unsigned int>{{2, 2}, {3, 3}, {4, 4}}));
    BOOST_TEST(s.start == 2U);
    s = series("CK CA C2", 1, true, true);
    BOOST_TEST((s.aPos == std::map<unsigned int, unsigned int>{{1, 0}, {2, 1}}));
    BOOST_TEST(s.start == 1U);
    BOOST_TEST(s.cards == cards("C2 CK CA"), "Cards " << s.cards);
    s = series("C2 C3 C9 CA", 3, true, true);
    BOOST_TEST((s.aOrder == std::vector<unsigned int>{3, 4, 2}));
    BOOST_TEST(s.start == 1U);
    BOOST_TEST(s.cards == cards("C9 CA C2 C3"), "Cards " << s.cards);

    // The serie is sorted to the end
    s = series("D4 C4 CJ C4 CQ CK", 4, false, true);
    BOOST_TEST(s.nrs == 1U);
    BOOST_TEST((s.aOrder == std::vector<unsigned int>{1, 2, 3}));
    BOOST_TEST(s.start == 3U);
    BOOST_TEST(s.cards == cards("D4 C4 C4 CJ CQ CK"), "Cards " << s.cards);
}

BOOST_AUTO_TEST_CASE(apply_move) {
    Card::Cards hand(cards("C2 H5 S9"));
    Table t(table({"H2 H3 H4 H5 H6 H7 H8", "S8 C8 D8"}));
    applyMove(hand, t, Move(2, {{HAND, 1, 1, 0}, {0, 4, 6, 1}}));
    BOOST_TEST(hand == cards("C2 S9"));
    BOOST_TEST(t.size() == 3U);
    BOOST_TEST(t[0].cards == cards("H2 H3 H4 H5"));
    BOOST_TEST(t[2].cards == cards("H5 H6 H7 H8"));
    BOOST_TEST(t[2].type == COLOUR);

    applyMove(hand, t, Move(1, {{HAND, 1, 1, 1}}));
    BOOST_TEST(t[1].cards == cards("S8 S9 C8 D8"));
    BOOST_TEST(firstInvalidPile(t) == 1);
}

BOOST_AUTO_TEST_CASE(moves_of_human) {
    const Card::Cards hand(cards("C2 H4 S9"));
    const Table t(table({"H5 H6 H7 H8", "S8 C8 D8", "H6 H7 H8 H9 HT", "D2"}));
    Move move;

    // From the hand: One card
    BOOST_TEST(checkMove(hand, t, HAND, 1, 0, move) == MoveError::NONE);
    BOOST_TEST(move.dest == 0U);
    BOOST_TEST(move.transfers == (std::vector<Transfer>{{HAND, 1, 1, 0}}));
    BOOST_TEST(checkMove(hand, t, HAND, 2, 0, move) == MoveError::DOES_NOT_FIT);
    BOOST_TEST(checkMove(hand, t, HAND, 2, 1, move) == MoveError::DOES_NOT_FIT); // S8 is not the first card
    BOOST_TEST(checkMove(hand, t, HAND, 0, 4, move) == MoveError::NONE);
    BOOST_TEST(move.transfers == (std::vector<Transfer>{{HAND, 0, 0, 0}}));

    // From the table: To a new pile the rest of a coloured pile ...
    BOOST_TEST(checkMove(hand, t, 2, 3, 4, move) == MoveError::NONE);
    BOOST_TEST(move.transfers == (std::vector<Transfer>{{2, 3, 4, 0}}));
    // ... but only one card of a numbered one
    BOOST_TEST(checkMove(hand, t, 1, 1, 4, move) == MoveError::NONE);
    BOOST_TEST(move.transfers == (std::vector<Transfer>{{1, 1, 1, 0}}));
    // Only the border cards of coloured piles can be moved to numbered piles
    BOOST_TEST(checkMove(hand, t, 2, 2, 1, move) == MoveError::SPLIT_ORIGIN_FIRST);
    BOOST_TEST(checkMove(hand, t, 2, 4, 0, move) == MoveError::DOES_NOT_FIT);
    BOOST_TEST(checkMove(hand, t, 0, 3, 1, move) == MoveError::NONE);
    BOOST_TEST(move.transfers == (std::vector<Transfer>{{0, 3, 3, 3}}));
    // To the left of a single card the left part of the pile is moved
    BOOST_TEST(checkMove(hand, t, 2, 0, 3, move) == MoveError::DOES_NOT_FIT);
    const Table t2(table({"C4 C5 C6 C7", "C8"}));
    BOOST_TEST(checkMove(hand, t2, 0, 3, 1, move) == MoveError::NONE);
    BOOST_TEST(move.transfers == (std::vector<Transfer>{{0, 0, 3, 0}}));

    BOOST_TEST(std::strcmp(describe(MoveError::DOES_NOT_FIT), "This card does not fit on that pile!") == 0);
    BOOST_TEST(std::strcmp(describe(MoveError::SPLIT_ORIGIN_FIRST), "Can't move this card - try splitting the origin first!") ==
               0);
}

BOOST_AUTO_TEST_CASE(players_and_dealing) {
    BOOST_TEST(findNextPlayer({7, 7, 7, 7}, 0) == 1U);
    BOOST_TEST(findNextPlayer({7, 0, 0, 7}, 0) == 3U);
    BOOST_TEST(findNextPlayer({7, 0, 0, 7}, 3) == 0U);
    BOOST_TEST(!isGameOver({7, 0, 0, 7}, 3));
    BOOST_TEST(isGameOver({0, 0, 0, 7}, 3));

    BOOST_TEST(dealPosition(cards("C2 H5 S5 DK"), card("D5")) == 1U);
    BOOST_TEST(dealPosition(cards("C2 H5 S5 DK"), card("D9")) == 3U);
    BOOST_TEST(dealPosition(cards("C2 H5 S5 DK"), card("DA")) == 4U);
    BOOST_TEST(dealPosition(cards(""), card("DA")) == 0U);
}

BOOST_AUTO_TEST_CASE(computer_plays_series) {
    // Cards with equal numbers
    Card::Cards hand(cards("C2 C5 H5 S5 DK"));
    Move move(selectMove(hand, Table()));
    BOOST_TEST(move.dest == 0U);
    BOOST_TEST(move.transfers == (std::vector<Transfer>{{HAND, 1, 3, 0}}));

    // Cards of one colour (sorted to the end of the hand)
    hand = cards("H4 H5 C5 H6 D9");
    move = selectMove(hand, table({"S2 S3 S4"}));
    BOOST_TEST(move.dest == 1U);
    BOOST_TEST(move.transfers == (std::vector<Transfer>{{HAND, 2, 4, 0}}));
    BOOST_TEST(hand == cards("C5 D9 H4 H5 H6"), "Hand " << hand);

    // No serie, no table
    hand = cards("C2 H5 DK");
    BOOST_TEST(selectMove(hand, Table()).empty());
}

BOOST_AUTO_TEST_CASE(computer_adds_to_piles) {
    // Directly
    Card::Cards hand(cards("D2 S9"));
    Move move(selectMove(hand, table({"C5 C6 C7", "H9 D9 C9"})));
    BOOST_TEST(move.dest == 1U);
    BOOST_TEST(move.transfers == (std::vector<Transfer>{{HAND, 1, 1, 3}}));

    // By splitting a pile
    hand = cards("H5");
    move = selectMove(hand, table({"H2 H3 H4 H5 H6 H7 H8"}));
    BOOST_TEST(move.dest == 1U);
    BOOST_TEST(move.transfers == (std::vector<Transfer>{{HAND, 0, 0, 0}, {0, 4, 6, 1}}));

    // By moving a card from another pile
    hand = cards("H4");
    move = selectMove(hand, table({"H6 H7 H8", "H5 S5 C5 D5"}));
    BOOST_TEST(move.dest == 0U);
    BOOST_TEST(move.transfers == (std::vector<Transfer>{{HAND, 0, 0, 0}, {1, 0, 0, 1}}));

    // Nothing fits
    hand = cards("C2");
    BOOST_TEST(selectMove(hand, table({"H9 HT HJ"})).empty());
}

BOOST_AUTO_TEST_CASE(computer_plays_pair_with_card_of_table) {
    // Pair from the hand with a card from the table
    Card::Cards hand(cards("S4 H4 C9"));
    Move move(selectMove(hand, table({"D4 D5 D6 D7"})));
    BOOST_TEST(move.dest == 1U);
    BOOST_TEST(move.transfers == (std::vector<Transfer>{{HAND, 0, 1, 0}, {0, 0, 0, 0}}));

    // Not, if the pile on the table would get too small
    hand = cards("S4 H4 C9");
    BOOST_TEST(selectMove(hand, table({"D4 D5 D6"})).empty());

    // Regression: The pair with the lower card found second was played wrongly
    // (the higher card and the card behind the lower one; out of range, if the
    // lower card was the last one)
    hand = cards("D5 C4 HT D4");
    Table t(table({"D3 D4 D5 D6"}));
    move = selectMove(hand, t);
    BOOST_TEST(move.dest == 1U);
    BOOST_TEST(move.transfers == (std::vector<Transfer>{{HAND, 2, 3, 0}, {0, 0, 0, 0}}));
    BOOST_TEST(hand == cards("C4 HT D4 D5"), "Hand " << hand);
    applyMove(hand, t, move);
    BOOST_TEST(firstInvalidPile(t) == -1);
    BOOST_TEST(t[1].cards == cards("D3 D4 D5"));
}
