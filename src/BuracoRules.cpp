// PROJECT     : Cardgames
// SUBSYSTEM   : Buraco
// REFERENCES  :
// TODO        :
// BUGS        :
// AUTHOR      : Markus Schwab
// CREATED     : 9.10.2026
// COPYRIGHT   : Copyright (C) 2003 - 2018, 2024, 2026

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

#include <cardgames-cfg.h>

#include <algorithm>

#include <YGP/Check.h>
#include <YGP/Trace.h>

#include <card/Cards.h>
#include <card/Random.h>

#include "BuracoRules.h"

using Card::Value;

namespace BuracoRules {

namespace {

//-----------------------------------------------------------------------------
/// Helper-function to correct the found matching cards (see getSeries)
/// \param elem Element to remove from the found ones
/// \param series Found cards
//-----------------------------------------------------------------------------
void deleteElement(unsigned int elem, Series& series) {
    Check3(series.positions.find(elem) != series.positions.end());
    Check3(std::ranges::find(series.order, elem) != series.order.end());
    series.order.erase(std::ranges::find(series.order, elem));
    series.positions.erase(series.positions.find(elem));
}

} // namespace

//-----------------------------------------------------------------------------
/// Checks if the passed card is a joker (a joker or a 2)
/// \param card Card to inspect
/// \returns bool True if card is a joker
//-----------------------------------------------------------------------------
bool isJoker(const Value& card) { return ((card.number() == Value::TWO) || (card.number() > Value::ACE)); }

//-----------------------------------------------------------------------------
/// Returns the value of the passed card
/// \param card Card to inspect
/// \returns unsigned int Value of the card
//-----------------------------------------------------------------------------
unsigned int pointsOf(const Value& card) {
    // Card:                 2   3  4  5  6  7  8   9   10  J   Q   K   A   Joker
    static constexpr std::array<unsigned int, 14> values{25, 5, 5, 5, 5, 5, 10, 10, 10, 10, 10, 10, 20, 50};
    Check3(card.number() < static_cast<int>(values.size()));
    return values[card.number()];
}

//-----------------------------------------------------------------------------
/// Returns the sum of the points of the passed cards
/// \param cards Cards to inspect
/// \returns unsigned int Points of the cards
//-----------------------------------------------------------------------------
unsigned int pointsOf(const Card::Cards& cards) {
    unsigned int sum(0);
    for (const auto& c : cards)
        sum += pointsOf(c);
    return sum;
}

//----------------------------------------------------------------------------
/// Returns the distance between two cards. The ace also counts as one (if the
/// other card is lower than an eight) and 2's are equal to jokers.
/// \param a Card to compare
/// \param b Card to compare
/// \param aceIsOne Flag, if aces should (also) be treated as one
/// \returns int Distance of the two passed cards (a - b); 99 if they don't fit
//----------------------------------------------------------------------------
int cardDistance(const Value& a, const Value& b, bool aceIsOne) {
    // Special handling of jokers
    bool aJoker(isJoker(a));
    bool bJoker(isJoker(b));
    if (aJoker || bJoker)
        return aJoker && bJoker ? 0 : 99;

    if (a.colour() != b.colour())
        return (a.number() == b.number()) ? 0 : 99;

    if (aceIsOne) { // Special handling of the ace like 1
        if ((a.number() == Value::ACE) && (b.number() < Value::EIGHT))
            return -static_cast<int>(b.number());
        else if ((b.number() == Value::ACE) && (a.number() < Value::EIGHT))
            return static_cast<int>(a.number());
    }
    return a.number() - b.number();
}

//-----------------------------------------------------------------------------
/// Compares the cards with regard of the number and with special
/// consideration of joker cards (2s are sorted after the aces; jokers last)
/// \param a Card to compare
/// \param b Card to compare
/// \returns bool True, if a < b
//-----------------------------------------------------------------------------
bool lessByNumberWithJokers(const Value& a, const Value& b) {
    // Card:                 2   3  4  5  6  7  8  9  10 J  Q   K  A   Joker
    static constexpr std::array<unsigned char, 14> values{12, 0, 1, 2, 3, 4, 5, 6, 7, 8, 9, 10, 11, 13};
    Check3(a.number() < static_cast<int>(values.size()));
    Check3(b.number() < static_cast<int>(values.size()));
    return values[a.number()] < values[b.number()];
}

//-----------------------------------------------------------------------------
/// Compares the cards with regard of the colour and with special
/// consideration of joker cards (which are sorted last)
/// \param a Card to compare
/// \param b Card to compare
/// \returns bool True, if a < b
//-----------------------------------------------------------------------------
bool lessByColourWithJokers(const Value& a, const Value& b) {
    switch (a.number()) {
    case Value::TWO:
        return b.number() == Value::UNREACHABLE;

    case Value::UNREACHABLE:
        return false;

    default:
        return (isJoker(b) ? true : ((a.colour() == b.colour()) ? a.number() < b.number() : a.colour() < b.colour()));
    } // endswitch
}

//-----------------------------------------------------------------------------
/// Checks, if the passed cards contain no cards except jokers or 2s. This is
/// also true for no cards.
/// \param cards Cards to inspect
/// \returns bool True, if there are only jokers (or there are no cards)
//-----------------------------------------------------------------------------
bool containsOnlyJoker(const Card::Cards& cards) { return std::ranges::all_of(cards, isJoker); }

//-----------------------------------------------------------------------------
/// Checks, if the passed cards contain neither jokers nor 2s.
/// \param cards Cards to inspect
/// \returns bool True, if there are no jokers
//-----------------------------------------------------------------------------
bool containsNoJoker(const Card::Cards& cards) { return std::ranges::none_of(cards, isJoker); }

//-----------------------------------------------------------------------------
/// Returns the position of a card fitting to the passed one (having a
/// distance of at most 2; see cardDistance)
/// \param cards Cards to search in
/// \param card Card where to find a fitting one to
/// \param start Position where to start the search
/// \returns unsigned int Position of the fitting card or cards.size()
//-----------------------------------------------------------------------------
unsigned int findFittingCard(const Card::Cards& cards, const Value& card, unsigned int start) {
    for (; start < cards.size(); ++start)
        if ((static_cast<unsigned int>(cardDistance(card, cards[start]) + 2)) < 5)
            break;
    return std::min(start, static_cast<unsigned int>(cards.size()));
}

//-----------------------------------------------------------------------------
/// Checks if the passed cards contain a pair matching the passed card
/// \param cards Cards to inspect
/// \param card Card where to find a pair to
/// \param skip Position of the passed card within the cards (which is not
///     inspected); -1U if the card is not part of the cards
/// \returns bool True, if the cards contain a matching pair
//-----------------------------------------------------------------------------
bool hasFittingPair(const Card::Cards& cards, const Value& card, unsigned int skip) {
    TRACE3("BuracoRules::hasFittingPair(const Card::Cards&, const Value&, unsigned int) - " << card);
    unsigned int nrs(0);
    unsigned int bCols(0);

    for (unsigned int p(findFittingCard(cards, card)); p < cards.size(); p = findFittingCard(cards, card, p + 1)) {
        if (p == skip) // Skip card if its the passed one
            continue;

        int diff(cardDistance(cards[p], card));
        if (diff) {
            diff = (diff < 0) ? (diff + 2) : (diff + 1);
            Check3(diff < 4);
            if (!(bCols & (1 << diff))) {
                // The card is valid, if either a card bordering the one the
                // inspect and this one has been found. Note that for aces the
                // bordering card must be in the same direction as the card to
                // inspect (e.g. K-A-2 is not valid; only Q-K-A!)
                if ((card.number() == Value::ACE) ? (bCols & (0x1 << (diff ^ 0x1)))
                                                  : (bCols & (diff ? (0x5 << (diff - 1)) : 0x2)))
                    return true;
                bCols |= (1 << diff);
            }
        }
        else if (++nrs == 2)
            return true;
    }
    return false;
}

//-----------------------------------------------------------------------------
/// Checks if the passed cards contain a pair matching the passed card
/// \param cards Cards to inspect
/// \param card Card where to find a pair to
/// \param pos Position of the card within the cards (-1U: Not part of them)
/// \param withJokers Flag, if jokers should be inspected (a fitting card and
///     a joker are enough then)
/// \returns bool True, if the cards contain a matching pair
//-----------------------------------------------------------------------------
bool pileHasFittingPair(const Card::Cards& cards, const Value& card, unsigned int pos, bool withJokers) {
    if (withJokers) {
        unsigned int i(findFittingCard(cards, card));
        // Remark: The original code compared the card at the end of the pile, if there was no fitting card
        if ((i < cards.size()) && (i == pos))
            i = findFittingCard(cards, card, i + 1);
        if ((i < cards.size()) && !containsNoJoker(cards))
            return true;
    }
    return hasFittingPair(cards, card, pos);
}

//-----------------------------------------------------------------------------
/// Checks if the passed cards contain a pair (two fitting cards)
/// \param cards Cards to inspect
/// \param exclude Position of a card to not inspect (-1U: none)
/// \returns bool True, if the cards contain a matching pair
//-----------------------------------------------------------------------------
bool pileHasFittingPair(const Card::Cards& cards, unsigned int exclude) {
    for (unsigned int p(0); p < cards.size(); ++p)
        if (p != exclude)
            if ((findFittingCard(cards, cards[p]) != p) || (findFittingCard(cards, cards[p], p + 1) != cards.size()))
                return true;
    return false;
}

//-----------------------------------------------------------------------------
/// Inserts a card into cards sorted by number (see lessByNumberWithJokers)
/// \param cards Cards to insert into
/// \param card Card to insert
/// \returns unsigned int Position where the card has been inserted
//-----------------------------------------------------------------------------
unsigned int insertSorted(Card::Cards& cards, const Value& card) {
    auto i(std::ranges::upper_bound(cards, card, lessByNumberWithJokers));
    unsigned int pos(i - cards.begin());
    cards.insert(i, card);
    return pos;
}

//-----------------------------------------------------------------------------
/// Returns the position of the first card being equal or bigger (by number;
/// see lessByNumberWithJokers) than the passed one
/// \param cards Cards to search in (sorted by number)
/// \param card Card to search for
/// \returns int Position of the found card or -1
//-----------------------------------------------------------------------------
int findSorted(const Card::Cards& cards, const Value& card) {
    auto i(std::ranges::lower_bound(cards, card, lessByNumberWithJokers));
    return (i == cards.end()) ? -1 : static_cast<int>(i - cards.begin());
}

//-----------------------------------------------------------------------------
/// Moves the card at position source to position dest (like IPile::move)
/// \param cards Cards to change
/// \param dest New position of the card
/// \param source Position of card to move
/// \param tracked Position of a card, which is updated to reflect the move
//-----------------------------------------------------------------------------
void moveCard(Card::Cards& cards, unsigned int dest, unsigned int source, unsigned int* tracked) {
    Check1(source < cards.size());
    Value card(cards[source]);
    cards.erase(cards.begin() + source);
    Check1(dest <= cards.size());
    cards.insert(cards.begin() + dest, card);

    if (tracked) {
        if (*tracked == source)
            *tracked = dest;
        else {
            if (*tracked > source)
                --*tracked;
            if (*tracked >= dest)
                ++*tracked;
        }
    }
}

//----------------------------------------------------------------------------
/// Gets the series of cards matching the one at the passed position: The
/// number of cards with the same number and the cards of the same colour in
/// a row.
/// \param cards Cards to inspect
/// \param pos Position of card to inspect
/// \returns Series Found matching cards
//----------------------------------------------------------------------------
Series getSeries(const Card::Cards& cards, unsigned int pos) {
    Check1(pos < cards.size());
    const Value& card(cards[pos]);
    TRACE5("BuracoRules::getSeries(const Card::Cards&, unsigned int) - " << card);

    Series series;
    series.equal = 0;
    unsigned int bCols(0x4);
    for (unsigned int p(findFittingCard(cards, card)); p < cards.size(); p = findFittingCard(cards, card, p + 1)) {
        // The card itself goes always in the middle (the 3rd position)
        if (p == pos) {
            series.positions[2] = p;
            series.order.push_back(2);
            ++series.equal;
        }
        else {
            int diff(cardDistance(cards[p], card));
            Check3(static_cast<unsigned int>(diff + 2) < 5);
            if (diff) {
                diff += 2;
                if (!(bCols & (1 << diff))) {
                    Check3(series.positions.find(diff) == series.positions.end());
                    bCols |= (1 << diff);
                    series.positions[diff] = p;
                    series.order.push_back(diff);
                }
            }
            else
                ++series.equal;
        }
    }

    // Delete cards having no direct access to the analysed one
    if ((bCols & 0x3) == 0x1) {
        deleteElement(0, series);
        bCols &= ~0x1;
    }
    if ((bCols & 0x18) == 0x10) {
        deleteElement(4, series);
        bCols &= ~0x10;
    }

    // Special handling of series of colours for an ace, to avoid the problem
    // with 3-K-A of one colour.
    if ((card.number() == Value::ACE) && ((bCols & 0xa) == 0xa)) {
        for (unsigned int elem : ((bCols & 0x18) == 0x18) ? std::array<unsigned int, 2>{0, 1} : std::array<unsigned int, 2>{3, 4})
            if (series.positions.find(elem) != series.positions.end())
                deleteElement(elem, series);
    }
    return series;
}

//----------------------------------------------------------------------------
/// Sorts a series of matching cards (of the same colour) to the end of the
/// cards (like IPile::sortColourSerie)
/// \param cards Cards to change
/// \param series Series of cards to sort to the end
/// \param tracked Position of a card, which is updated to reflect the moves
/// \returns unsigned int Position of start of sorted serie
//----------------------------------------------------------------------------
unsigned int sortColourSerie(Card::Cards& cards, const Series& series, unsigned int* tracked) {
    unsigned int pos(1);
    for (auto p(series.order.rbegin()); p != series.order.rend(); ++p) {
        auto v(series.positions.find(*p));
        Check3(v != series.positions.end());
        // Move the card to the end of the staple; If it belongs before the
        // first card move it before the other cards.
        // Remark: The positions are not adapted to the moves (as in the original code)
        moveCard(cards, cards.size() - pos, v->second, tracked);
        if (((p + 1) != series.order.rend()) && (*(p + 1) < *p))
            ++pos;
    }
    return cards.size() - series.positions.size();
}

//----------------------------------------------------------------------------
/// Analyses the pile and returns its characteristics.
/// \param pile Pile to analyse
/// \returns PileInfo Characteristics of the pile
//----------------------------------------------------------------------------
PileInfo analysePile(const Card::Cards& pile) {
    Check1(pile.size() <= CERRADO);
    PileInfo status;
    for (unsigned int i(0); i < pile.size(); ++i)
        if (isJoker(pile[i])) {
            Check3((status.posJoker > 6) || (status.posFirst > 6));
            status.posJoker = i;
        }
        else
            ((status.posFirst > 6) ? status.posFirst : status.posLast) = i;

    if (status.posLast > 6)
        status.posLast = status.posFirst;

    status.type = ((status.posFirst != status.posLast)
                       ? ((pile[status.posFirst].number() == pile[status.posLast].number()) ? NUMBER : COLOUR)
                       : UNDEFINED);

    // If there is no first "normal" card, it must be a pile of monos
    if (status.posFirst > 6) {
        if (pile.size() > 1) // Only set type, if there are at least two cards
            status.type = NUMBER;
        Check2(pile.size() ? (status.posJoker < 7) : 1);
        status.points = 2000;

        // Correct points if it is "sucio"
        if (std::ranges::any_of(pile, [](const Value& c) { return c.number() == Value::UNREACHABLE; }))
            status.points = 1000;
    }
    else {
        status.points = (((status.type == NUMBER) && (pile[status.posFirst].number() == Value::ACE)) ? 300 : 200);
        if (status.posJoker > 6)
            status.points += 200;
    }

    TRACE8("BuracoRules::analysePile(const Card::Cards&) - " << status.posFirst << '/' << status.posLast << '/' << status.posJoker
                                                             << ": " << status.points);
    return status;
}

//----------------------------------------------------------------------------
/// Returns the position in the pile where the card can be played to.
/// - Card played on an empty pile -> Valid (but check for fitting tripple or
///   pair with joker)
/// - Joker played on a pile without joker -> Valid (but check for fitting
///    card, if the pile has only one card)
/// - Ordinary card: Check if the card "fits": Either the same number as the
///   other (first and last) card, or the same colour and the number in serie.
/// \param pile Pile to inspect
/// \param card Card to inspect.
/// \param pos Position of card to play, or -1U if can't be played
/// \param move Position to move joker to or -1U
/// \returns bool True, if card fits on pile
/// \pre Coloured piles must be sorted strict ascending
//----------------------------------------------------------------------------
bool getPosition4Card(const Card::Cards& pile, const Value& card, unsigned int& pos, unsigned int& move) {
    TRACE6("BuracoRules::getPosition4Card(const Card::Cards&, const Value&, 2x unsigned int&) - " << card);
    const PileInfo status(analysePile(pile));
    Check2((status.posJoker < pile.size() || (status.posJoker > 6)));
    move = -1U;

    // Card played on an empty pile or a pile with only one joker -> Valid
    if ((status.posFirst > 6) && ((status.posJoker < 1) || (status.posJoker > 6))) {
        Check3(status.type == UNDEFINED);
        pos = 0;
        return true;
    }

    // Joker played on a pile without joker: Valid
    if (isJoker(card) && ((status.posFirst > 6) || (status.posJoker > 6))) {
        pos = ((status.type == NUMBER)
                   ? 1
                   : ((((status.posFirst < 7)) && (pile[status.posFirst].number() == Value::ACE)) ? (status.posLast + 1) : 0));
        return true;
    }

    // Fix: A normal card doesn't fit on a pile of (at least two) monos. The
    //      original code accessed the (not existing) first normal card then.
    if (status.posFirst > 6)
        return false;
    Check2(status.posFirst <= status.posLast);
    Check2(status.posLast < pile.size());

    // Else check if the pile is a numberd or a coloured one
    const Value& first(pile[status.posFirst]);
    const Value& last(pile[status.posLast]);
    if (first.number() == card.number()) {
        if (status.type != COLOUR) {
            pos = status.posJoker ? pile.size() : 0;
            return true;
        }
    }
    else if ((first.colour() == card.colour()) && (status.type != NUMBER)) {
        // This code assumes that the (potentially) coloured pile is sorted
        // from lower card to higher cards (strict ascending).
        // First check, if a joker in between other cards can be replaced
        if ((status.posJoker < 7) && (cardDistance(card, first) == static_cast<int>(status.posJoker))) {
            if ((status.posJoker && (status.posJoker < status.posLast)) || (card.number() == Value::ACE))
                move =
                    ((((status.posJoker > status.posFirst) ? first.number() : card.number()) == Value::ACE) ? status.posLast : 0);
            pos = status.posJoker;
            if ((move != -1U) && (move < pos))
                ++pos;
            return true;
        }

        // Possible difference the card can have: 1 or two if joker at one end
        unsigned int maxDiff(
            (status.posJoker > 6) ? 1 : (((status.posJoker < status.posFirst) || (status.posJoker > status.posLast)) ? 2 : 1));
        if ((status.posFirst == status.posLast) || (first.number() != Value::ACE)) {
            unsigned int diff(cardDistance(first, card, first.number() < Value::FIVE));
            TRACE9("BuracoRules::getPosition4Card(...) - Diff " << diff << "; max: " << maxDiff);
            Check3(diff);

            if (diff && (diff <= maxDiff)) {
                pos = status.posFirst - diff + 1;
                if ((status.posJoker < 7)) {
                    if ((diff == 2)
                            ? ((status.posJoker > status.posFirst) && ((first.number() != Value::ACE) || (pile.size() < 3)))
                            : ((card.number() == Value::ACE) ? (!status.posJoker && (pos == 1))
                                                             : ((card.number() == Value::KING) && (!pos && status.posJoker)))) {
                        move = ((((status.posJoker > status.posFirst) ? first.number() : card.number()) == Value::ACE)
                                    ? status.posLast
                                    : 0);
                        move ? --pos : ++pos;
                    }
                }
                return true;
            }
        }

        // Test if card fits at other end
        unsigned int diff(cardDistance(card, last, status.posFirst == status.posLast));
        TRACE9("BuracoRules::getPosition4Card(...) - Diff (end): " << diff << "; max: " << maxDiff);
        if (diff && (diff <= maxDiff)) {
            pos = status.posLast + diff;
            if ((diff == 2) && (status.posJoker < status.posLast)) {
                Check3(status.posFirst > status.posJoker);

                move = status.posLast;
                --pos;
            }
            return true;
        }
    }
    return false;
}

//-----------------------------------------------------------------------------
/// Returns the points a pile on the table counts at the end of the round
/// \param info Characteristics of the pile
/// \param size Number of cards in the pile
/// \returns int Points of a cerrado, -1000 for an unfinished pile of monos, else 0
//-----------------------------------------------------------------------------
int pilePoints(const PileInfo& info, unsigned int size) {
    return ((size == CERRADO) ? static_cast<int>(info.points) : ((info.points >= 1000) ? -1000 : 0));
}

//-----------------------------------------------------------------------------
/// Returns the points a pile on the table counts at the end of the round
/// \param pile Pile to inspect
/// \returns int Points of a cerrado, -1000 for an unfinished pile of monos, else 0
//-----------------------------------------------------------------------------
int pilePoints(const Card::Cards& pile) { return pilePoints(analysePile(pile), pile.size()); }

//-----------------------------------------------------------------------------
/// Checks if the passed pile is a valid one: Either only monos, or at most
/// one joker and cards with the same number or a series of the same colour
/// (sorted ascending, the joker filling a gap or extending the series; the
/// ace counting before the 3 (as 2s are always jokers) or after the king).
/// \param pile Pile to inspect
/// \returns bool True, if the pile is valid
/// \remarks Used to verify the moves (e.g. of the computer player)
//-----------------------------------------------------------------------------
bool isValidPile(const Card::Cards& pile) {
    if (pile.size() > CERRADO)
        return false;

    const auto jokers(std::ranges::count_if(pile, isJoker));
    if (static_cast<std::size_t>(jokers) == pile.size())
        return true;
    if (jokers > 1)
        return false;

    const auto first(std::ranges::find_if_not(pile, isJoker));
    if (std::ranges::all_of(pile, [&first](const Value& c) { return isJoker(c) || (c.number() == first->number()); }))
        return true;

    if (!std::ranges::all_of(pile, [&first](const Value& c) { return isJoker(c) || (c.colour() == first->colour()); }))
        return false;

    const int posFirst(first - pile.begin());
    for (bool aceLow : {false, true}) {
        auto valueOf([aceLow](const Value& c) {
            return static_cast<int>(((c.number() == Value::ACE) && aceLow) ? Value::TWO : c.number());
        });
        const int start(valueOf(*first) - posFirst);
        bool ok((start >= Value::TWO) && ((start + static_cast<int>(pile.size()) - 1) <= Value::ACE));
        for (unsigned int i(0); ok && (i < pile.size()); ++i)
            ok = isJoker(pile[i]) || (valueOf(pile[i]) == (start + static_cast<int>(i)));
        if (ok)
            return true;
    }
    return false;
}

//-----------------------------------------------------------------------------
/// Checks if playing the passed card on the pile starts a pile of monos
/// \param pile Pile to inspect
/// \param card Card to play
/// \returns bool True, if a pile of monos is started
//-----------------------------------------------------------------------------
bool startsMonoPile(const Card::Cards& pile, const Value& card) {
    return (pile.size() == 1) && isJoker(pile.back()) && isJoker(card);
}

//-----------------------------------------------------------------------------
/// Checks if the player can get rid of all cards in his hand except of the
/// jokers
/// \param hand Cards of the player
/// \returns bool True: if all cards can be played
/// \remarks This method does not check for triplets anymore!
//-----------------------------------------------------------------------------
bool canGetRidOfCards(const Card::Cards& hand) {
    std::vector<bool> used(hand.size());

    unsigned int cJokers(0);
    unsigned int piles(0);
    for (unsigned int i(0); i < hand.size(); ++i) {
        if (used[i])
            continue;

        if (isJoker(hand[i])) {
            used[i] = true;
            ++cJokers;
            continue;
        }

        // If there are equal cards (and the first card is not already marked as
        // used: Mark both card as used
        unsigned int o(findFittingCard(hand, hand[i], i + 1));
        if ((o < hand.size()) && !used[o]) {
            ++piles;
            used[i] = used[o] = true;
        }
    }

    const auto cUsed(static_cast<std::size_t>(std::ranges::count(used, true)));
    TRACE8("BuracoRules::canGetRidOfCards(const Card::Cards&) - " << cUsed << '/' << hand.size() << "; " << cJokers
                                                                  << " Joker for " << piles << " piles");
    return (((cUsed + 1) >= hand.size()) && (piles <= cJokers));
}

//-----------------------------------------------------------------------------
/// Checks if the player can play the specified number of cards; a player can
/// only play all of his cards, if:
///   - The team has a cerrado
///   - The team still has the reserve
///   - The player can close a pile, with the cards to play
///   - After the turn there are no unfinished mono-piles
/// \param table Actual game
/// \param player Player to analyse
/// \param cards Number of cards player wants to play
/// \param pile Pile player is going to play its card to (or -1 for a new one)
/// \returns bool True, if card can be played
//-----------------------------------------------------------------------------
bool canPlayCards(const Table& table, unsigned int player, unsigned int cards, unsigned int pile) {
    TRACE7("BuracoRules::canPlayCards(...) - Player " << player << " playing " << cards << " cards to " << pile);
    Check1(player < NUM_PLAYERS);
    const unsigned int team(player & 1);
    const auto& piles(table.piles[team]);
    Check1((pile == -1U) || (piles.size() > pile));
    Check1((pile == -1U) || ((piles[pile].size() + cards) <= CERRADO));
    Check1(table.hands[player].size() >= cards);
    Check1(cards <= CERRADO);

    unsigned int cCards(table.hands[player].size());
    if (table.pickUpPlayed)
        cCards += table.dumped;

    if ((cCards <= (cards + 1)) && !table.reserve[team]) {
        bool canPlay((table.points[team] > 100) ||
                     ((pile != -1U) && (((piles[pile].size() + cards) >= CERRADO) ||
                                        (((piles[pile].size() + cards) == 6) && ((table.hands[player].size() - cards) == 1) &&
                                         canClosePile(table, player, pile)))) ||
                     (cards >= CERRADO));
        return ((!table.unfinishedMonoPiles[team]) ||
                        ((table.unfinishedMonoPiles[team] == 1) && (pile != -1U) && (pilePoints(piles[pile]) < 0))
                    ? canPlay
                    : false);
    }
    return true;
}

//----------------------------------------------------------------------------
/// Checks if the player can with his two cards left close the passed pile
/// \param table Actual game
/// \param player Player to inspect
/// \param pile Pile to analyse
/// \return bool True, if the remaining cards of the player can make a
///        cerrado for this pile
/// \pre The player must have only two cards; the pile 5
//----------------------------------------------------------------------------
bool canClosePile(const Table& table, unsigned int player, unsigned int pile) {
    Check1(player < NUM_PLAYERS);
    Check1(table.piles[player & 1].size() > pile);
    const Card::Cards& hand(table.hands[player]);
    Check1(table.piles[player & 1][pile].size() == 5);
    Check1(hand.size() == 2);

    Card::Cards copy(table.piles[player & 1][pile]);
    unsigned int pos, move;
    for (unsigned int i(0); i < 2; ++i) {
        if (getPosition4Card(copy, hand[i], pos, move)) {
            Check3(pos <= copy.size());
            if (move != -1U) {
                Check3(move <= copy.size());
                moveCard(copy, move, analysePile(copy).posJoker);
            }
            copy.insert(copy.begin() + pos, hand[i]);

            if (getPosition4Card(copy, hand[!i], pos, move))
                return true;
            // Remark: As in the original code, a moved joker is not moved back
            copy.erase(copy.begin() + pos);
        }
    }
    return false;
}

//-----------------------------------------------------------------------------
/// Checks if the passed card (of the hand of the player) fits on the passed pile
/// \param table Actual game
/// \param player Player playing the card
/// \param iPile Pile (of the team of the player) to inspect
/// \param card Position of card to check in the hand of the player
/// \returns Position where card can be played to, or -1 if card does not fit
//-----------------------------------------------------------------------------
int cardFitsOnPile(const Table& table, unsigned int player, unsigned int iPile, unsigned int card) {
    Check1(player < NUM_PLAYERS);
    Check1(iPile < table.piles[player & 1].size());
    const Card::Cards& pile(table.piles[player & 1][iPile]);
    const Card::Cards& hand(table.hands[player]);
    Check1(card < hand.size());
    Check2(pile.size());
    Check2(pile.size() < CERRADO);
    const Value& actCard(hand[card]);
    const PileInfo info(analysePile(pile));

    // Card played on a joker: Valid is:
    //   - A joker; if there are at least 3 jokers (on table + in hand)
    //   - Any card, which has a pair (if there's only one joker on the table)
    if (info.posFirst > 6) {
        if (info.posJoker)
            return isJoker(actCard) ? 0 : -1;
        else {
            unsigned int pCard(findFittingCard(hand, actCard));
            if (pCard == card)
                pCard = findFittingCard(hand, actCard, pCard + 1);
            return (pCard == hand.size()) ? -1 : 0;
        }
    }

    unsigned int pos, move;
    if (getPosition4Card(pile, actCard, pos, move)) {
        Check3(pos <= pile.size());
        if ((pile.size() > 1) || isJoker(actCard) || !containsNoJoker(hand))
            return pos;

        const Value& pileCard(pile[info.posFirst]);
        int dist(cardDistance(pileCard, actCard));
        Check3((dist > -2) && (dist < 2));
        int cmp(0);

        unsigned int pCard(0);
        do {
            pCard = findFittingCard(hand, pileCard, pCard);
            // Remark: The original code dereferenced the end of the hand, if no card was found
            if ((pCard < hand.size()) && (pCard == card))
                pCard = findFittingCard(hand, pileCard, pCard + 1);
            if (pCard == hand.size())
                return -1;

            cmp = dist - cardDistance(hand[pCard], actCard);
            ++pCard;
        }
        while ((cmp != -dist) && (cmp != (dist << 1)));
        return pos;
    }
    return -1;
}

//-----------------------------------------------------------------------------
/// Checks if the piles on the table are valid (have at least 3 cards)
/// \param piles Piles to inspect
/// \param except Pile which can be invalid
/// \returns bool True, if the piles are OK
//-----------------------------------------------------------------------------
bool pilesComplete(const std::vector<Card::Cards>& piles, unsigned int except) {
    for (unsigned int i(0); i < piles.size(); ++i)
        if ((i != except) && (piles[i].size() < 3))
            return false;
    return true;
}

//-----------------------------------------------------------------------------
/// Returns the (untranslated) message describing the passed error
/// \param error Error to describe
/// \returns const char* Message (to translate)
//-----------------------------------------------------------------------------
const char* describe(PlayError error) {
    switch (error) {
    case PlayError::NONE:
        return "";
    case PlayError::PILES_INCOMPLETE:
        return N_("Every pile on the table must have at least 3 cards!");
    case PlayError::PICK_UP_MONO:
        return N_("You can't pick up monos!");
    case PlayError::PICK_UP_NO_PAIR:
        return N_("You need a fitting pair to pick up the pile of dumped cards!");
    case PlayError::PICK_UP_WOULD_END:
        return N_("Picking up the staple would leave you without cards\n"
                  "and you can't end the game now!");
    case PlayError::FILL_OTHER_PILES:
        return N_("You need to fill up other piles first!");
    case PlayError::NO_VALID_NEW_PILE:
        return N_("There are no cards to make a valid new pile!");
    case PlayError::MONO_PILE_UNFINISHED:
        return N_("You can't end the game (a pile of monos is not finished)!");
    case PlayError::NO_CERRADO:
        return N_("You can't end the game (there's no \"cerrado\")!");
    case PlayError::NOT_ENOUGH_CARDS:
        return N_("Not enough cards to make new pile!");
    case PlayError::NO_JOKER_ON_NEW_PILE:
        return N_("You may not start this new pile with a joker!");
    case PlayError::CARD_DOES_NOT_FIT:
        return N_("This card does not fit on that pile!");
    }
    return "";
}

//-----------------------------------------------------------------------------
/// Checks if the player can pick up the dumped cards
/// \param table Actual game
/// \param player Player picking up the dumped cards
/// \param top Top card of the dumped cards
/// \returns PlayError Reason why the cards can't be picked up (or NONE)
//-----------------------------------------------------------------------------
PlayError checkPickUp(const Table& table, unsigned int player, const Value& top) {
    Check1(player < NUM_PLAYERS);
    if (table.startGame)
        return PlayError::NONE;

    if (isJoker(top))
        return PlayError::PICK_UP_MONO;

    if (!pileHasFittingPair(table.hands[player], top, -1U))
        return PlayError::PICK_UP_NO_PAIR;

    // Don't allow picking up the pile, if that would force the game
    // to end without having neither buraco nor reserve
    if (((table.dumped + table.hands[player].size()) < 5) && (table.points[player & 1] < 200) && !table.reserve[player & 1])
        return PlayError::PICK_UP_WOULD_END;
    return PlayError::NONE;
}

//-----------------------------------------------------------------------------
/// Checks if the player can start a new pile with the passed card
/// \param table Actual game
/// \param player Player playing the card
/// \param card Position of card in the hand of the player
/// \returns PlayError Reason why the card can't be played (or NONE)
//-----------------------------------------------------------------------------
PlayError checkNewPile(const Table& table, unsigned int player, unsigned int card) {
    Check1(player < NUM_PLAYERS);
    const Card::Cards& hand(table.hands[player]);
    Check1(card < hand.size());
    const unsigned int team(player & 1);

    // Remark: The original code passed the "pile" (-1U >> 8) to except, which doesn't exist
    if (!pilesComplete(table.piles[team], -1U >> 8))
        return PlayError::FILL_OTHER_PILES;

    if (!(isJoker(hand[card]) ? pileHasFittingPair(hand, card) : pileHasFittingPair(hand, hand[card], card, !table.pickUpPlayed)))
        return PlayError::NO_VALID_NEW_PILE;

    // Only allow dropping on new pile while having < 5 cards, if the game
    // can be ended, or there is still the reserve
    if (!canPlayCards(table, player, 3))
        return table.unfinishedMonoPiles[team] ? PlayError::MONO_PILE_UNFINISHED
                                               : ((hand.size() <= 5) ? PlayError::NO_CERRADO : PlayError::NOT_ENOUGH_CARDS);
    return PlayError::NONE;
}

//-----------------------------------------------------------------------------
/// Checks if the player can add the passed card to the passed pile
/// \param table Actual game
/// \param player Player playing the card
/// \param card Position of card in the hand of the player
/// \param pile Pile (of the team of the player) to play to
/// \returns PlayError Reason why the card can't be played (or NONE)
//-----------------------------------------------------------------------------
PlayError checkAddToPile(const Table& table, unsigned int player, unsigned int card, unsigned int pile) {
    Check1(player < NUM_PLAYERS);
    const Card::Cards& hand(table.hands[player]);
    Check1(card < hand.size());
    const unsigned int team(player & 1);
    Check1(pile < table.piles[team].size());

    // Check if all piles (except those to which card is dropped) are valid
    if (!pilesComplete(table.piles[team], pile))
        return PlayError::FILL_OTHER_PILES;

    // Can't use the new pile (with the picked up card) with a joker
    if (isJoker(hand[card]) && table.pickUpPlayed)
        return PlayError::NO_JOKER_ON_NEW_PILE;

    if (cardFitsOnPile(table, player, pile, card) == -1)
        return PlayError::CARD_DOES_NOT_FIT;

    // Only allow dropping of last card, if the game can be ended, or there
    // is still the reserve
    if (!canPlayCards(table, player, 1, pile))
        return table.unfinishedMonoPiles[team] ? PlayError::MONO_PILE_UNFINISHED : PlayError::NO_CERRADO;
    return PlayError::NONE;
}

//-----------------------------------------------------------------------------
/// Checks if the player can end his turn (by dumping a card)
/// \param table Actual game
/// \param player Player ending his turn
/// \returns PlayError Reason why the turn can't be ended (or NONE)
//-----------------------------------------------------------------------------
PlayError checkDump(const Table& table, unsigned int player) {
    Check1(player < NUM_PLAYERS);
    return pilesComplete(table.piles[player & 1]) ? PlayError::NONE : PlayError::PILES_INCOMPLETE;
}

//-----------------------------------------------------------------------------
/// Returns the number of cards a player gets
/// \param cardsToDeal Number of cards to deal (as configured; the size of the reserve)
/// \returns unsigned int Number of cards in the hand
//-----------------------------------------------------------------------------
unsigned int cardsInHand(unsigned int cardsToDeal) { return cardsToDeal + 1; }

//-----------------------------------------------------------------------------
/// Deals the cards (as the GUI does it)
/// \param staple Shuffled cards (the top card is the last one)
/// \param cardsToDeal Number of cards to deal (as configured)
/// \returns Deal Dealt cards
//-----------------------------------------------------------------------------
Deal deal(Card::Cards staple, unsigned int cardsToDeal) {
    Deal result;
    const unsigned int inHand(cardsInHand(cardsToDeal));
    Check1(staple.size() > (NUM_PLAYERS * inHand + NUM_TEAMS * cardsToDeal));

    for (auto& hand : result.hands) {
        hand.assign(staple.end() - inHand, staple.end());
        staple.resize(staple.size() - inHand);
    }

    for (auto& reserve : result.reserve)
        for (unsigned int j(0); j < cardsToDeal; ++j) {
            reserve.push_back(staple.back());
            staple.pop_back();
        }

    for (auto& hand : result.hands)
        std::ranges::sort(hand, lessByNumberWithJokers);

    result.dumped = staple.back();
    staple.pop_back();
    result.staple = std::move(staple);
    return result;
}

//-----------------------------------------------------------------------------
/// Returns the player starting the first round
/// \returns unsigned int Random player
//-----------------------------------------------------------------------------
unsigned int startPlayer() { return Card::randomNumber(NUM_PLAYERS); }

//-----------------------------------------------------------------------------
/// Checks what happens, if the player has no cards left (except of jokers)
/// \param hand Cards of the player
/// \param hasReserve Flag, if the team of the player has still its reserve
/// \returns HandStatus TAKE_RESERVE: The player gets the reserve;
///     GOING_OUT: The round ends; PLAYING: Nothing happens
//-----------------------------------------------------------------------------
HandStatus handStatus(const Card::Cards& hand, bool hasReserve) {
    if (containsOnlyJoker(hand)) {
        if (hasReserve)
            return HandStatus::TAKE_RESERVE;
        if (hand.empty())
            return HandStatus::GOING_OUT;
    }
    return HandStatus::PLAYING;
}

//-----------------------------------------------------------------------------
/// Adds the reserve to the hand (sorted, before the remaining cards)
/// \param hand Cards of the player
/// \param reserve Reserve of the team
//-----------------------------------------------------------------------------
void takeReserve(Card::Cards& hand, Card::Cards reserve) {
    std::ranges::sort(reserve, lessByNumberWithJokers);
    hand.insert(hand.begin(), reserve.begin(), reserve.end());
}

//-----------------------------------------------------------------------------
/// Calculates the score of the round
/// \param table Game at the end of the round (the points must contain the
///     bonus for ending the game)
/// \returns RoundScore Points of the round
//-----------------------------------------------------------------------------
RoundScore roundScore(const Table& table) {
    RoundScore score;
    for (unsigned int i(0); i < NUM_TEAMS; ++i) {
        score.bonus[i] = table.points[i] + (table.reserve[i] ? -RESERVE_BONUS : RESERVE_BONUS);

        // Sum up all cards on the table
        int sum(0);
        int monoPile(0);
        for (const auto& p : table.piles[i]) {
            Check3(p.size() > 2);
            // Substract 1000 points for every started cerrado of monos
            if (pilePoints(p) < 0)
                monoPile += 1000;
            sum += pointsOf(p);
        }
        TRACE5("BuracoRules::roundScore(const Table&) - Points of team " << i << " on table: " << sum << '/' << monoPile);
        score.cards[i] = ((score.bonus[i] < (table.reserve[i] ? 100 : 300)) ? -sum : sum) - monoPile;
    }

    for (unsigned int i(0); i < NUM_PLAYERS; ++i)
        score.cards[i & 1] -= pointsOf(table.hands[i]);
    return score;
}

//-----------------------------------------------------------------------------
/// Decides if the computer player takes the top card of the dumped cards
/// (and plays it with others of his hand to a new pile; except in the first
/// move of the game)
/// \param table Actual game
/// \param player Player in turn
/// \param top Top card of the dumped cards
/// \returns bool True, if the dumped card(s) should be taken
//-----------------------------------------------------------------------------
bool takeDumped(const Table& table, unsigned int player, const Value& top) {
    Check1(player < NUM_PLAYERS);
    const Card::Cards& hand(table.hands[player]);
    return table.startGame
               ? (isJoker(top) || (findFittingCard(hand, top) != hand.size()))
               : ((!isJoker(top)) && pileHasFittingPair(hand, top, -1U) &&
                  ((table.points[player & 1] > 100) || table.reserve[player & 1] || ((table.dumped + hand.size()) > 4)));
}

//-----------------------------------------------------------------------------
/// Searches the cards to play with the taken dumped card to a new pile.
/// The cards are sorted to the end of the hand of the player.
/// \param table Actual game; the hand of the player is re-arranged
/// \param player Player in turn
/// \param top Taken card (not in the hand of the player)
/// \returns PickUp Position of the cards to play and of the taken card
//-----------------------------------------------------------------------------
PickUp playPickedUp(Table& table, unsigned int player, const Value& top) {
    Check1(player < NUM_PLAYERS);
    Check1(!table.startGame);
    Card::Cards& hand(table.hands[player]);

    unsigned int posTop(insertSorted(hand, top));
    Series series(getSeries(hand, posTop));
    unsigned int nrs(series.equal);
    TRACE8("BuracoRules::playPickedUp(...) - Sizes: " << nrs << "<->" << series.positions.size());
    Check3((nrs >= 3) || (series.positions.size() >= 3));

    // Sanity checks: Don't dump more than 7 cards
    if (nrs > CERRADO)
        nrs = CERRADO;

    // Don't dump all cards, if this would result in finishing the game
    // This can only be a numbered pile; as coloured piles return only 3 cards
    // Remark: table.dumped still contains the taken card (and the hand too)
    // Fix: Only if there are still (at least) 3 cards to play; else the new
    //      pile would have only 2 cards (e.g. taking H3 with D3 S3 in the hand)
    if ((table.points[player & 1] < 101) && ((table.dumped - 1 + hand.size()) < 5) && (nrs > 3))
        --nrs;

    PickUp result;
    if (nrs < series.positions.size()) {
        nrs = series.positions.size();
        result.first = sortColourSerie(hand, series, &posTop);
        Check2(posTop >= result.first);
        Check2(posTop < (result.first + nrs));
        result.posTaken = posTop - result.first;
    }
    else {
        result.posTaken = nrs - 1;
        result.first = findSorted(hand, top);
    }
    result.last = result.first + nrs - 2;
    Check3((result.last - result.first) >= 1);

    hand.erase(hand.begin() + posTop);
    return result;
}

//-----------------------------------------------------------------------------
/// Checks if the passed card can be put on one of the existing piles
/// \param table Actual game; jokers in the piles of the team might be moved
/// \param player Player to inspect
/// \param iCard Position of card to inspect in the hand of the player
/// \param moves Moves of jokers in the piles; a performed move is added
/// \returns unsigned int Value describing the pile (and the offset of the card)
///     to play to (pile << 16 + position); -1U if none
//-----------------------------------------------------------------------------
unsigned int cardFitsOnPlayedPile(Table& table, unsigned int player, unsigned int iCard, std::vector<JokerMove>& moves) {
    Check1(player < NUM_PLAYERS);
    const unsigned int team(player & 1);
    const Card::Cards& hand(table.hands[player]);
    Check1(iCard < hand.size());
    const Value& card(hand[iCard]);
    auto& piles(table.piles[team]);
    TRACE5("BuracoRules::cardFitsOnPlayedPile(...) - Card " << card << " of player " << player);

    unsigned int bestPile(-1U);
    unsigned int maxPoints(0);
    unsigned int size(0);
    for (unsigned int p(0); p < piles.size(); ++p) {
        const Card::Cards& pile(piles[p]);
        if (pile.size() == CERRADO) // Skip finished piles
            continue;
        Check3(pile.size() >= 3);
        const PileInfo info(analysePile(pile));

        // Play joker, if you can make a cerrado (7 in a row) - but only if the
        // one having picked up the reserve already played (the missing card
        // might be in there) and the oponent can't finish.
        if (isJoker(card) ? ((((pile.size() == 6) && (info.posJoker > 6)) && ((hand.size() - iCard) < 7) &&
                              (((!table.reserve[team] && ((table.buraco[team] == 0x3) || (hand.size() < 3))) ||
                                (table.points[team] > 100)) ||
                               !table.reserve[!team] || (table.points[!team] > 100))) ||
                             (info.posFirst > 6))
                          : (cardFitsOnPile(table, player, p, iCard) != -1)) {
            // Always play on a joker pile (don't bother checking for a second one)
            if (info.points >= 1000) {
                bestPile = p;
                break;
            }
            if ((size < pile.size()) || ((size == pile.size()) && (maxPoints < info.points))) {
                size = pile.size();
                maxPoints = info.points;
                bestPile = p;
            }
        }
    }

    if (bestPile != -1U) {
        unsigned int pos(0), move(-1U);
        Card::Cards& pile(piles[bestPile]);
        getPosition4Card(pile, card, pos, move);
        Check3(pos <= pile.size());
        if ((move != -1U) && canPlayCards(table, player, 1, bestPile)) {
            const unsigned int posJoker(analysePile(pile).posJoker);
            Check3(move <= pile.size());
            Check3(move != posJoker);
            Check3(posJoker != NONE);
            moves.push_back({bestPile, posJoker, move});
            moveCard(pile, move, posJoker);
        }
        return (bestPile << 16) + pos;
    }
    return -1U;
}

//-----------------------------------------------------------------------------
/// Selects the move of the computer player (after having taken a card); only
/// one move (playing cards to one pile) is made at once.
/// \param table Actual game; the hand of the player might be re-arranged, jokers
///     in the piles moved and the number of unfinished piles of monos increased
/// \param player Player in turn
/// \returns Move Move to perform
//-----------------------------------------------------------------------------
Move selectMove(Table& table, unsigned int player) {
    TRACE6("BuracoRules::selectMove(Table&, unsigned int) - " << player);
    Check1(player < NUM_PLAYERS);
    const unsigned int team(player & 1);
    Card::Cards& hand(table.hands[player]);
    auto& piles(table.piles[team]);
    Check1(hand.size());

    Move result;

    // Check if any card can be added to an existing pile
    for (unsigned int p(0); p < hand.size(); ++p) {
        const std::size_t moved(result.jokerMoves.size());
        unsigned int target(cardFitsOnPlayedPile(table, player, p, result.jokerMoves));
        if (target != -1U) {
            if (canPlayCards(table, player, 1, target >> 16)) {
                result.kind = Move::ADD_TO_PILE;
                result.pile = target >> 16;
                result.pos = target & 0xff;
                result.first = result.last = p;
                return result;
            }

            // Fix: Move a joker moved for the card back, as the card is not
            //      played (else the pile would be invalid; e.g. a joker
            //      replaced by the 7 in S4 S5 S6 Jo S8 moved to the front)
            if (result.jokerMoves.size() > moved) {
                const JokerMove& m(result.jokerMoves.back());
                moveCard(piles[m.pile], m.from, m.to);
                result.jokerMoves.pop_back();
            }
        }
    }

    // Check for 3 cards belonging to a serie
    unsigned int i(0);
    for (; i < hand.size(); ++i) {
        Series series(getSeries(hand, i));
        unsigned int nrs(series.equal);

        // Play found cards (if any)
        //   - Play jokers if there are at least 5 and the team has still the
        //     reserve and the other team has no burraco and the reserve
        //   - Play the bigger of the found matching cards, if there are >= 3
        // Remark: The hands inspected for jokers are kept as in the original code
        if (isJoker(hand[i]) ? ((((nrs > 4) && table.reserve[team]) || (nrs > 5)) &&
                                ((table.points[(player + 1) & 1] < 101) || (nrs > 6) ||
                                 ((table.hands[(player + 2) % 3].size() > 5) && (table.hands[(player + 1) % 3].size() > 3))))
                             : ((nrs > series.positions.size()) ? (nrs > 2) : (series.positions.size() > 2))) {
            unsigned int firstPos(i);
            if (nrs < series.positions.size()) {
                firstPos = sortColourSerie(hand, series);
                nrs = series.positions.size();
            }
            if (nrs > CERRADO)
                nrs = CERRADO;

            bool canPlay(canPlayCards(table, player, nrs));
            if (canPlay || (nrs > 5)) {
                if (!canPlay)
                    nrs = 3;

                if (isJoker(hand[firstPos]))
                    ++table.unfinishedMonoPiles[team];

                // Create new pile with the found cards
                result.kind = Move::NEW_PILE;
                result.pile = piles.size();
                result.first = firstPos;
                result.last = firstPos + nrs - 1;
                return result;
            }
            else
                std::ranges::sort(hand, lessByNumberWithJokers);
        }
    }

    // Check if all cards in the hand can (and should) be played
    if (!table.unfinishedMonoPiles[team] && ((table.points[team] > 100) || (table.reserve[team] && canGetRidOfCards(hand)))) {
        // If pile still has normal cards (no joker)
        for (unsigned int ci(0); !((ci == hand.size()) || isJoker(hand[ci])); ++ci) {
            unsigned int next(findFittingCard(hand, hand[ci], ci + 1));
            if ((next != hand.size()) && isJoker(hand.back())) {
                TRACE5("BuracoRules::selectMove(Table&, unsigned int) - Have two with joker: " << hand[ci] << " and "
                                                                                               << hand[next]);
                int diff(cardDistance(hand[next], hand[ci]));
                Check3(diff ? hand[next].colour() == hand[ci].colour() : true);
                const unsigned int size(hand.size());
                if (diff < 0) {
                    Check3(diff >= -2);
                    moveCard(hand, size - 2, next);
                    moveCard(hand, size + ((diff == -2) ? -1 : -2), ci);
                }
                else {
                    Check3(diff <= 2);
                    moveCard(hand, size - 1, next);
                    moveCard(hand, size - 1 - diff, ci);
                }

                // Create a new pile with the found pair and a joker
                result.kind = Move::NEW_PILE;
                result.pile = piles.size();
                result.first = hand.size() - 3;
                result.last = hand.size() - 1;
                return result;
            }
        }
    }

    // Play all jokers if team has a cerrado, or leave one, if the player has
    // >= 2 normal cards left.
    if (hand.size() && (table.points[team] > 100)) {
        if ((isJoker(hand.back())) && ((hand.size() <= 2) || ((!isJoker(hand[1])) || isJoker(hand[hand.size() - 2])))) {
            unsigned int bestPile(-1U);
            unsigned int size(0);
            for (unsigned int p(0); p < piles.size(); ++p) {
                const PileInfo info(analysePile(piles[p]));
                if (((piles[p].size() < CERRADO) && (info.posJoker > 6)) &&
                    ((piles[p].size() > size) ||
                     ((piles[p].size() == size) && (info.points > analysePile(piles[bestPile]).points)) ||
                     (info.points >= 1000))) {
                    bestPile = p;
                    size = piles[p].size();
                }
            }

            if (bestPile != -1U) {
                unsigned int pos, move;
                getPosition4Card(piles[bestPile], hand.back(), pos, move);
                result.kind = Move::ADD_TO_PILE;
                result.pile = bestPile;
                result.pos = pos;
                result.first = result.last = hand.size() - 1;
                return result;
            }
        }
    }

    // No more cards to put down: Find a card to dump
    for (i = 0; i < hand.size() - 1; ++i) {
        unsigned int p(findFittingCard(hand, hand[i]));
        if (p == i)
            p = findFittingCard(hand, hand[i], p + 1);
        if (p == hand.size())
            break;
    }

    while (i && isJoker(hand[i])) // Try to not dump jokers
        --i;

    Check3(i < hand.size());
    result.kind = Move::DUMP;
    result.first = result.last = i;
    return result;
}

} // namespace BuracoRules
