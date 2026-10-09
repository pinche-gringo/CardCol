// PROJECT     : Cardgames
// SUBSYSTEM   : Machiavelli
// REFERENCES  :
// TODO        : Rewrite reorderTableToFit() to only iterate once over the piles
// BUGS        :
// AUTHOR      : Markus Schwab
// CREATED     : 9.10.2026
// COPYRIGHT   : Copyright (C) 2003 - 2009, 2011, 2012, 2024, 2026

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

#include "MachiavelliRules.h"

using Card::Value;

namespace MachiavelliRules {

namespace {

/// Card missing on a pile, to be able to add it from the hand
struct MissingCard {
    unsigned int pile;     ///< Pile, to which the card would fit
    int nr;                ///< Number of the card (might be -1 below a two)
    Value::COLOURS colour; ///< Colour of the card

    explicit MissingCard(unsigned int pile) : pile(pile), nr(Value::UNREACHABLE), colour() {}
};
using Missing = std::vector<MissingCard>;

//-----------------------------------------------------------------------------
/// Removes the passed element from the found matching cards
/// \param elem Element to remove from the found ones
/// \param aPos Map holding the positions of the cards
/// \param aOrder Sorted order of the cards
//-----------------------------------------------------------------------------
void deleteElement(unsigned int elem, std::map<unsigned int, unsigned int>& aPos, std::vector<unsigned int>& aOrder) {
    Check3(aPos.find(elem) != aPos.end());
    Check3(std::ranges::find(aOrder, elem) != aOrder.end());
    aOrder.erase(std::ranges::find(aOrder, elem));
    aPos.erase(aPos.find(elem));
}

//-----------------------------------------------------------------------------
/// Adds "bordering" cards to the missing cards.
///
/// "Bordering cards" are defined as either the cards which would fit on the
/// edge for coloured piles or the missing colour in a numbered pile.
/// \param table Piles on the table
/// \param iPile Offset of pile to get the "bordering" cards from
/// \param which Defines for coloured piles at which end of the pile a card
///     should be added (0x1: left; 0x2: right; can be or-ed together)
/// \param missing Missing cards to add to
//-----------------------------------------------------------------------------
void addBorderCards2Missing(const Table& table, unsigned int iPile, unsigned int which, Missing& missing) {
    TRACE8("MachiavelliRules::addBorderCards2Missing(...) - Pile: " << iPile << "; Which: " << which);
    Check1(iPile < table.size());

    const Pile& pile(table[iPile]);
    Check3(pile.size() > 2);
    Check3(pile.type != UNDEFINED);

    MissingCard card(iPile);
    if (pile.type == COLOUR) {
        card.colour = pile[0].colour();
        if ((which & 0x1) && (pile[0].number() != Value::ACE)) {
            card.nr = pile[0].number() - 1;
            missing.push_back(card);
        }
        if ((which & 0x2) && (pile[pile.size() - 1].number() != Value::ACE)) {
            card.nr = pile[pile.size() - 1].number() + 1;
            missing.push_back(card);
        }
    }
    else {
        unsigned int fUsed(0);
        for (const auto& p : pile.cards) {
            Check2(p.number() == pile[0].number());
            fUsed |= 1 << p.colour();
        }

        card.nr = pile[0].number();
        for (unsigned int i(0); i < 4; ++i)
            if (!(fUsed & (1 << i))) {
                card.colour = static_cast<Value::COLOURS>(i);
                missing.push_back(card);
                break;
            }
    }
}

//-----------------------------------------------------------------------------
/// Searches the hand, if it has a serie of 3 or more cards
/// \param hand Cards of the player; might be re-ordered
/// \param table Piles on the table
/// \param move Set to the move playing the serie to a new pile
/// \returns bool True, if there's a serie
/// \pre The hand must have at least 3 cards
//-----------------------------------------------------------------------------
bool playSerie(Card::Cards& hand, const Table& table, Move& move) {
    Check3(hand.size() >= 3);

    // Check for 3 cards belonging to a serie
    for (unsigned int i(0); i < (hand.size() - 2); ++i) {
        TRACE8("MachiavelliRules::playSerie(...) - Analyzing card " << hand[i]);

        std::map<unsigned int, unsigned int> aPos; // diff, pos
        std::vector<unsigned int> aOrder;
        unsigned int posCard(i);
        unsigned int nrs(getSeries(hand, posCard, aPos, aOrder, false));

        // Play the bigger of the found matching cards, if there are >= 3
        if ((nrs > aPos.size()) ? (nrs > 2) : (aPos.size() > 2)) {
            if (nrs <= aPos.size()) {
                i = sortColourSerie(hand, aPos, aOrder);
                nrs = aPos.size();
            }
            Check3(nrs >= 3);

            move = Move(table.size(), {{HAND, i, i + nrs - 1, 0}});
            return true;
        }
    }
    return false;
}

//-----------------------------------------------------------------------------
/// Checks if the passed card fits on a pile on the table, either directly
/// or by splitting the pile
/// \param card Card which might be added to a pile on the table
/// \param offset Offset of the card in the hand of the player
/// \param table Piles on the table
/// \param move Set to the move playing the card
/// \returns bool True, if the card fits on a pile
//-----------------------------------------------------------------------------
bool cardFitsOnPile(const Value& card, unsigned int offset, const Table& table, Move& move) {
    TRACE8("MachiavelliRules::cardFitsOnPile(...) - Adding card " << card << '?');

    for (unsigned int iPile(0); iPile < table.size(); ++iPile) {
        const Pile& pile(table[iPile]);
        Check2(pile.type != UNDEFINED);

        // Check if the card can be added to an existing pile
        unsigned int dest(getPosition4Card(pile, card));
        if (dest != -1U) {
            move = Move(iPile, {{HAND, offset, offset, dest}});
            return true;
        }

        // Or can the card be added by splitting the pile?
        int diff(cardDistance(card, pile[0]));
        if ((pile.type == COLOUR) ? ((diff < 0) || (card.colour() != pile[0].colour())) : diff)
            continue;

        int pos(pile.size() - static_cast<unsigned int>(diff));
        TRACE7("MachiavelliRules::cardFitsOnPile(...) - Splitting " << iPile << " at " << pos << " (" << diff << ") of "
                                                                    << pile.size());

        // A new pile can be made directly (enough cards on both sides)
        if ((pos > 2) && (diff > 1)) {
            pos = ++diff;
            move = Move(table.size(), {{HAND, offset, offset, 0}, {iPile, static_cast<unsigned int>(pos), pile.size() - 1, 1}});
            return true;
        }
    }
    return false;
}

//-----------------------------------------------------------------------------
/// Tries to play a card indirectly by filling up a pile missing one card,
/// so that in the next turn the card can be played
/// \param table Piles on the table
/// \param missing Cards missing on piles of the table
/// \param move Set to the move re-ordering the table
/// \returns bool True, if the table can be re-ordered
//-----------------------------------------------------------------------------
bool reorderTableToFit4(const Table& table, const Missing& missing, Move& move) {
    TRACE8("MachiavelliRules::reorderTableToFit4()");

    // Try to find any of the missing cards
    for (const auto& i : missing) {
        TRACE3("MachiavelliRules::reorderTableToFit4() - " << Value::strColour(i.colour) << i.nr << " for pile " << i.pile);

        for (unsigned int t(0); t < table.size(); ++t) {
            if (i.pile == t)
                continue;

            const Pile& pile(table[t]);
            Check3(pile.type != UNDEFINED);
            if (pile.type == COLOUR) {
                if ((pile[0].colour() == i.colour) && (i.nr >= 0)) {
                    TRACE6("MachiavelliRules::reorderTableToFit4() - Inspecting coloured pile " << t);

                    unsigned int pos(Card::findFirstEqualOrBigger(pile.cards, static_cast<Value::NUMBERS>(i.nr)));
                    Check3((pos == -1U) || (pos < pile.size()));
                    if ((pile.size() > 3) && (pos != -1U) && (pile[pos].number() == i.nr)) {
                        TRACE8("MachiavelliRules::reorderTableToFit4() - Card found: " << pos);

                        if (!pos || (pos == (pile.size() - 1))) {
                            move = Move(i.pile, {{t, pos, pos, getPosition4Card(table[i.pile], pile[pos])}});
                            return true;
                        }
                        else if ((pos > 3) && (pos < (pile.size() - 2))) {
                            move = Move(table.size(), {{t, pos, pile.size() - 1, 0}});
                            return true;
                        }
                    }
                } // endif pile has the right colour
            } // endif coloured pile
            else if ((pile.size() == 4) && (pile[0].number() == i.nr)) {
                TRACE6("MachiavelliRules::reorderTableToFit4() - Inspecting numbered pile " << t);

                for (unsigned int pos(0); pos < pile.size(); ++pos)
                    if (pile[pos].colour() == i.colour) {
                        move = Move(i.pile, {{t, pos, pos, getPosition4Card(table[i.pile], pile[pos])}});
                        return true;
                    }
            }
        }
    }
    return false;
}

//-----------------------------------------------------------------------------
/// Tries to play a card by rearranging two piles on the table
/// \param hand Cards of the player
/// \param table Piles on the table
/// \param missing Cards missing on piles of the table
/// \param move Set to the move to make
/// \returns bool True, if table can be re-ordered
//-----------------------------------------------------------------------------
bool reorderTableToFit3(const Card::Cards& hand, const Table& table, Missing& missing, Move& move) {
    for (unsigned int p(0); p < hand.size(); ++p) {
        Card::Cards work;

        // Try to find two cards from the table (from different piles)
        for (unsigned int t(0); t < table.size(); ++t) {
            const Pile& pile(table[t]);
            Check2(pile.type != UNDEFINED);
            if (pile.size() < 4)
                continue;

            for (unsigned int i(0); (i = getFittingCard(pile.cards, hand[p], i)) < pile.size(); ++i) {
                if ((hand[p].number() == pile[i].number()) && (hand[p].colour() == pile[i].colour()))
                    continue;
                TRACE7("MachiavelliRules::reorderTableToFit3(...) - Hand: " << hand[p] << " with " << pile[i] << " on pile "
                                                                            << t);

                int diff(i);
                if (diff && (diff != static_cast<int>(pile.size() - 1)) &&
                    ((diff < 3) || (diff > static_cast<int>(pile.size() - 4))))
                    continue;

                // hand[p] card in hand
                // pile (t) 1st pile on table
                // pile[i] 1st matching card
                diff = cardDistance(hand[p], pile[i]);
                TRACE6("MachiavelliRules::reorderTableToFit3(...) - Matching " << pile[i] << " differs " << diff);
                work.push_back(hand[p]);
                work.push_back(pile[i]);

                // Now try to find a 3rd matching card somewhere on the table
                for (unsigned int o(t + 1); o < table.size(); ++o) {
                    unsigned int c, nr;
                    if (hasMatching3rd(table[o], work, c, nr)) {
                        Check3(c < table[o].size());
                        TRACE6("MachiavelliRules::reorderTableToFit3(...) - 3rd: " << table[o][c]);

                        // o 2nd pile on table
                        // c 2nd matching card
                        if (table[o].size() < 4) {
                            addBorderCards2Missing(table, o, 1 << (c == 0), missing);
                            if (missing.size() && (missing.back().nr == pile[i].number()) &&
                                (missing.back().colour == pile[i].colour()))
                                missing.pop_back();
                            continue;
                        }

                        // Does the first pile need to be split up?
                        if (i && (i != (pile.size() - 1))) {
                            TRACE7("MachiavelliRules::reorderTableToFit3(...) - Splitting at " << i);
                            work.clear();
                            o = t;
                            c = i + 1;
                            nr = pile.size() - c;
                        }

                        if (work.size()) {
                            int diff2(cardDistance(hand[p], table[o][c]));
                            TRACE8("MachiavelliRules::reorderTableToFit3(...) - Hand " << p
                                                                                       << " differs from 2nd table: " << diff2);
                            TRACE8("MachiavelliRules::reorderTableToFit3(...) - Piles " << t << " (" << i << ") and " << o << " ("
                                                                                        << c << ')');

                            const unsigned int posDest2((diff2 > diff) ? (diff2 == -1) : ((diff2 < 0) ? 2 : 1));
                            move = Move(table.size(), {{HAND, p, p, 0}, {t, i, i, diff < 0}, {o, c, c, posDest2}});
                        }
                        else {
                            TRACE8("MachiavelliRules::reorderTableToFit3(...) - Pile " << o << "; Card " << c << '-'
                                                                                       << (c + nr - 1));
                            Check3((c + nr) <= table[o].size());
                            move = Move(table.size(), {{o, c, c + nr - 1, 0}});
                        }
                        return true;
                    } // endif pile has matching card
                } // endfor all following piles

                work.clear();
            } // end-for all matching cards in the pile
        } // end-for all table piles
    } // end-for all cards

    return reorderTableToFit4(table, missing, move);
}

//-----------------------------------------------------------------------------
/// Tries to play a card by moving one card from one pile on the table to
/// another one, so that a card from the hand also fits.
/// \param hand Cards of the player
/// \param table Piles on the table
/// \param missing Cards missing on piles of the table
/// \param move Set to the move to make
/// \returns bool True, if cards to play have been found
//-----------------------------------------------------------------------------
bool reorderTableToFit2(const Card::Cards& hand, const Table& table, Missing& missing, Move& move) {
    for (unsigned int p(0); p < hand.size(); ++p) {
        // Try to find a pile, where the card from the hand differs by 2 from
        // any end.
        for (unsigned int t(0); t < table.size(); ++t) {
            Check2(table[t].type != UNDEFINED);
            if ((table[t].type == NUMBER) || (table[t][0].colour() != hand[p].colour()))
                continue;

            int diff(cardDistance(hand[p], table[t][0], ONE));
            if ((diff == -2) || ((diff - static_cast<int>(table[t].size())) == 1)) {
                TRACE8("MachiavelliRules::reorderTableToFit2(...) - With move: " << hand[p] << "; Diff: " << diff);

                for (unsigned int o(0); o < table.size(); ++o) {
                    const Pile& other(table[o]);
                    Check2(other.type != UNDEFINED);
                    if ((o == t) || ((other.type == COLOUR) && (other[0].colour() != hand[p].colour())))
                        continue;

                    int pos(getPosOfColour(other, hand[p].colour()));
                    if (pos <= 0)
                        pos = 0;
                    int diffTable(cardDistance(hand[p], other[pos], ONE));
                    unsigned int c(other.size());
                    if (other.type == NUMBER) {
                        if (diffTable == ((diff == -2) ? -1 : 1)) {
                            for (c = 0; c < other.size(); ++c)
                                if (other[c].colour() == hand[p].colour())
                                    break;
                            Check3(c < other.size());
                        }
                    }
                    else {
                        if (diff == -2) {
                            if ((diffTable + 2) == static_cast<int>(other.size()))
                                c = other.size() - 1;
                        }
                        else if ((diffTable == 1))
                            c = 0;
                    }

                    // Move card, if one has been found
                    if (c != other.size()) {
                        if (other.size() < 4)
                            addBorderCards2Missing(table, o, 1 << (c == 0), missing);
                        else {
                            TRACE8("MachiavelliRules::reorderTableToFit2(...) - Hand: " << hand[p] << " (" << p << ')');
                            TRACE8("MachiavelliRules::reorderTableToFit2(...) - Pile: " << o << "; Card " << other[c] << " (" << c
                                                                                        << ')');
                            const unsigned int posDest((diff < 0) ? 0 : table[t].size());
                            move = Move(t, {{HAND, p, p, posDest}, {o, c, c, posDest + (diff < diffTable)}});
                            return true;
                        }
                    }
                }
            }
        }
    }

    return reorderTableToFit3(hand, table, missing, move);
}

//-----------------------------------------------------------------------------
/// Tries to play a card by rearranging the cards on the table. First it checks
/// if two cards in the hand can be extended by a card on the table.
/// If that fails it calls other methods to perform other checks.
/// \param hand Cards of the player; might be re-ordered
/// \param table Piles on the table
/// \param missing Cards missing on piles of the table
/// \param move Set to the move to make
/// \returns bool True, if table can be re-ordered
//-----------------------------------------------------------------------------
bool reorderTableToFit(Card::Cards& hand, const Table& table, Missing& missing, Move& move) {
    for (unsigned int p(0); p < hand.size(); ++p) {
        unsigned int h(p);
        Card::Cards work;

        // First try to make piles with two cards from the hand
        while ((h = getFittingCard(hand, hand[p], h + 1)) < hand.size()) {
            int diff(cardDistance(hand[h], hand[p]));
            TRACE5("MachiavelliRules::reorderTableToFit(...) - " << hand[h] << "<->" << hand[p] << ": " << diff);
            if ((diff == 0) && (hand[p].id() == hand[h].id()))
                continue;

            work.push_back(hand[p]);
            work.push_back(hand[h]);
            TRACE8("MachiavelliRules::reorderTableToFit(...) - Pair: " << hand[p] << " - " << hand[h]);

            // Try to add from the table
            for (unsigned int t(0); t < table.size(); ++t) {
                const Pile& pile(table[t]);
                Check3(pile.size() > 2);
                Check2(pile.type != UNDEFINED);

                unsigned int c, nr;
                if (hasMatching3rd(pile, work, c, nr)) {
                    Check3(c < pile.size());
                    if (pile.size() < 4) {
                        // Don't memorise card as missing, if the hand holds two equal
                        // numbers and the pile is also a numbered one
                        if (diff || (pile.type == COLOUR))
                            addBorderCards2Missing(table, t, 1 << (c == 0), missing);
                        continue;
                    }

                    // hand[h] is the 1st card from the hand
                    // hand[p] is the 2nd card from the hand
                    // pile[c] is the card from the second pile (t)
                    TRACE6("MachiavelliRules::reorderTableToFit(...) - Hand " << p << "; " << h);

                    // Sort the card in the hands in the right order;
                    // pos1Play points to the first, pos2Play to the second
                    // (hand[h] is lower: move hand[p] behind it)
                    unsigned int pos1Play;
                    if (diff < 0) {
                        pos1Play = h - 1;
                        moveCard(hand, pos1Play + 1, p);
                    }
                    else {
                        pos1Play = p;
                        moveCard(hand, pos1Play + 1, h);
                    }
                    const unsigned int pos2Play(pos1Play + 1);
                    TRACE3("MachiavelliRules::reorderTableToFit(...) - Pile " << t << "; Cards " << c << '-' << (c + nr - 1));

                    const unsigned int posDest((cardDistance(pile[c], hand[pos2Play]) == 1) ? 2 : 0);
                    move = Move(table.size(), {{HAND, pos1Play, pos2Play, 0}, {t, c, c + nr - 1, posDest}});
                    return true;
                } // endif pile has matching card
            } // end-for all table piles
            work.clear();
        } // end-while card has a fitting one
    }

    return reorderTableToFit2(hand, table, missing, move);
}

} // namespace

//----------------------------------------------------------------------------
/// Returns the distance between two cards. The ace also counts as one (if the
/// other card is a 2 or a 3)
/// \param a Card to compare
/// \param b Card to compare
/// \param aceIsOne Flag, if aces should (also) be treated as one
/// \returns int Distance of the two passed cards (a - b); 0 for equal numbers
///     of different colours, 99 for not matching cards
//----------------------------------------------------------------------------
int cardDistance(const Value& a, const Value& b, AceFlag aceIsOne) {
    TRACE9("MachiavelliRules::cardDistance(2x const Value&, AceFlag) - " << a << "<->" << b);

    if (a.colour() != b.colour())
        return (a.number() == b.number()) ? 0 : 99;

    if (aceIsOne != ACE) { // Special handling of the ace like 1
        if (a.number() == Value::ACE) {
            if ((aceIsOne == ONE) || ((b.number() < Value::FOUR) && (b.number() != Value::ACE)))
                return -static_cast<int>(b.number()) - 1;
        }
        else if (b.number() == Value::ACE)
            if ((aceIsOne == ONE) || ((a.number() < Value::FOUR) && (a.number() != Value::ACE)))
                return static_cast<int>(a.number()) + 1;
    }

    return a.number() - b.number();
}

//----------------------------------------------------------------------------
/// Creates a pile out of the passed cards (as if they were added one after
/// another)
/// \param cards Cards of the pile
/// \returns Pile Created pile
//----------------------------------------------------------------------------
Pile Pile::of(const Card::Cards& cards) {
    Pile pile;
    for (const auto& card : cards)
        pile.insert(card, pile.size());
    return pile;
}

//----------------------------------------------------------------------------
/// Inserts a card into the pile (updating its type)
/// \param card Card to insert into the pile
/// \param pos Zero-based offset of where to insert the card
//----------------------------------------------------------------------------
void Pile::insert(const Value& card, unsigned int pos) {
    Check1(pos <= cards.size());
    cards.insert(cards.begin() + pos, card);
    type = analysePile(cards, type);
}

//----------------------------------------------------------------------------
/// Removes a card from the pile (updating its type)
/// \param pos Zero-based offset of the card to remove
/// \returns Value Removed card
//----------------------------------------------------------------------------
Value Pile::remove(unsigned int pos) {
    Check1(pos < cards.size());
    Value card(cards[pos]);
    cards.erase(cards.begin() + pos);
    type = analysePile(cards, type);
    return card;
}

//----------------------------------------------------------------------------
/// Returns the type of a pile after it has been changed: The type is defined,
/// when the pile has two cards; with less cards it is undefined; else it
/// doesn't change.
/// \param cards Cards of the pile
/// \param type Type of the pile before the change
/// \returns PileType Type of the pile after the change
//----------------------------------------------------------------------------
PileType analysePile(const Card::Cards& cards, PileType type) {
    if (cards.size() == 2)
        return (cards[0].number() == cards[1].number()) ? NUMBER : COLOUR;
    else if (cards.size() < 2)
        return UNDEFINED;
    return type;
}

//----------------------------------------------------------------------------
/// Returns the position in the pile where the card can be played to.
/// - Card played on an empty pile -> Valid
/// - Check if the card "fits": Either the same number as the other (first and
///   last) card, or the same colour and the number in serie.
/// \param pile Pile to inspect
/// \param card Card to inspect.
/// \returns unsigned int Position of card in pile or -1U
/// \pre Coloured piles must be sorted strict ascending
//----------------------------------------------------------------------------
unsigned int getPosition4Card(const Pile& pile, const Value& card) {
    TRACE1("MachiavelliRules::getPosition4Card(const Pile&, const Value&) - " << card);

    if (pile.empty())
        return 0;

    // First test numbered piles (btw. undefined piles)
    if (pile.type != COLOUR) {
        if ((pile[0].number() == card.number()) && (Card::findID(pile.cards, card.id()) == -1))
            return pile.size();
        else if (pile.type == NUMBER)
            return -1U;
    }

    // Now check for matching colour
    if (pile[0].colour() == card.colour()) {
        Value cmp(pile[0]);
        if (cardDistance(card, cmp, ((cmp.number() == Value::ACE) && (pile.size() > 1) ? ONE : BOTH)) == -1)
            return 0;

        cmp = pile[pile.size() - 1];
        if (cardDistance(card, cmp, ((cmp.number() == Value::ACE) && (pile.size() > 1) ? ACE : BOTH)) == 1)
            return pile.size();
    }

    return -1U;
}

//-----------------------------------------------------------------------------
/// Returns the position of the first card having the passed colour
/// \param pile Pile to inspect
/// \param colour Colour to find
/// \returns int Position of card or -1
//-----------------------------------------------------------------------------
int getPosOfColour(const Pile& pile, Value::COLOURS colour) {
    TRACE9("MachiavelliRules::getPosOfColour(const Pile&, Value::COLOURS) - " << colour);
    Check3(pile.size());

    if (pile.type == NUMBER)
        return Card::find(pile.cards, colour);
    else
        return (pile[0].colour() == colour) ? 0 : -1;
}

//----------------------------------------------------------------------------
/// Checks if the pile has a card matching to the ones passed in pair
/// \param pile Pile to inspect
/// \param pair Vector holding the pair to match; cleared, if the pile must be
///     split up, before the matching card can be used
/// \param match Set to
///    - Position of the card which matches the pair (if this card can be
///      played directly)
///    - Position where the pile has to split, so that the matching card can be
///      played
/// \param nr Number of cards which have to be moved (1, if the card can be
///    played directly, else the number of cards to move to "free" the matching
///    one)
/// \return bool True, if a matching card can be found
/// \remarks \c match and \c nr might be changed, even if no matching card is
///     found!
//----------------------------------------------------------------------------
bool hasMatching3rd(const Pile& pile, Card::Cards& pair, unsigned int& match, unsigned int& nr) {
    Check1(pair.size() == 2);
    const int size(pile.size());
    Value card(pile[0]);

    // For numbered pile find right colour (if pair has the same colour)
    if ((pile.type == NUMBER) && (pair[0].colour() == pair[1].colour())) {
        int pos(getPosOfColour(pile, pair[1].colour()));
        if (pos >= 0)
            card = pile[pos];
    }

    int diff(cardDistance(pair[1], pair[0]));
    int diffTable(cardDistance(pair[0], card,
                               (diff < 0)                                                     ? ONE
                               : ((pair[0].number() == Value::ACE) && (pair[1].number() < 5)) ? ACE
                                                                                              : BOTH));
    TRACE1("MachiavelliRules::hasMatching3rd(...) - Differences: " << diff << '/' << diffTable);

    if (diff < 0) {
        diff = -diff;
        --diffTable;
    }

    nr = 1;
    match = pile.size();
    switch (diff) {
    case 0: // Equal numbers
        if (diffTable) {
            if ((diffTable > 2) && (pile.type == COLOUR) && (((size - 4) > diffTable) || ((size - 1) == diffTable)) &&
                (pair[0].id() != pile[diffTable].id()) && (pair[1].id() != pile[diffTable].id())) {
                match = diffTable;
                nr = size - match;
                if (((size - 4) > diffTable) && ((size - 1) != diffTable))
                    pair.clear();
            }
        }
        else {
            if (pile.type == NUMBER) {
                for (match = 0; match < pile.size(); ++match)
                    if ((pair[0].id() != pile[match].id()) && (pair[1].id() != pile[match].id()))
                        break;
            }
            else if ((pair[0].id() != card.id()) && (pair[1].id() != card.id()))
                match = 0;
        }
        break;

    case 1:
        TRACE9("MachiavelliRules::hasMatching3rd(...) - Checking " << pair[0] << '-' << pair[1] << '-' << card);
        if ((pair[0].colour() == card.colour()) && (pile.type == COLOUR)) {
            Check3(pair[1].colour() == card.colour());
            if ((diffTable == -2) || (diffTable == 1))
                match = 0;
            else if (((size - diffTable) == 3) || (diffTable == size))
                match = size - 1;
            else {
                bool bot(diffTable > 3);
                bool top(diffTable < (size - 7));
                if ((size > 6) && bot && top) {
                    pair.clear();
                    match = bot ? diffTable : (diffTable + 3);
                    nr = size - match;
                }
            }
        }
        break;

    case 2:
        if ((pair[0].colour() == card.colour()) && (diffTable < 0) && (pile.type == COLOUR)) {
            Check3(pair[1].colour() == card.colour());
            if ((diffTable == 1) || ((size - 1) == diffTable))
                match = diffTable - 1;
            else {
                bool bot(diffTable > 3);
                bool top(diffTable < (size - 7));
                if ((size > 6) && bot && top) {
                    pair.clear();
                    match = bot ? diffTable : (diffTable + 3);
                    nr = size - match;
                }
            }
        }
        break;

    default:
        Check3(0);
    } // end-switch

    TRACE5("MachiavelliRules::hasMatching3rd(...) - Match " << ((match != pile.size()) ? 'Y' : 'N') << "; " << nr);
    return match != pile.size();
}

//-----------------------------------------------------------------------------
/// Returns the (untranslated) message describing the passed error
/// \param error Error to describe
/// \returns const char* Message (to be translated with gettext)
//-----------------------------------------------------------------------------
const char* describe(PileError error) {
    switch (error) {
    case PileError::NONE:
        break;
    case PileError::NOT_ENOUGH_CARDS:
        return N_("Not enough cards (must be at least 3)!");
    case PileError::INVALID_TYPE:
        return N_("Invalid type!");
    case PileError::CARD_DOES_NOT_FIT:
        return N_("Card %1 does not fit!");
    }
    return "";
}

//-----------------------------------------------------------------------------
/// Checks if the passed pile is valid
/// \param pile Pile to check
/// \param pos Set to the position of the card not fitting (for CARD_DOES_NOT_FIT)
/// \returns PileError Reason why the pile is invalid
//-----------------------------------------------------------------------------
PileError checkPile(const Pile& pile, unsigned int& pos) {
    if (pile.size() < MIN_PILE_SIZE)
        return PileError::NOT_ENOUGH_CARDS;

    if (pile.type == UNDEFINED)
        return PileError::INVALID_TYPE;

    for (unsigned int i(0); (i + 1) < pile.size(); ++i)
        if ((pile.type == COLOUR)
                ? ((pile[i].colour() != pile[i + 1].colour()) || (cardDistance(pile[i + 1], pile[i], i ? ACE : ONE) != 1))
                : (pile[i].number() != pile[i + 1].number())) {
            pos = i + 1;
            return PileError::CARD_DOES_NOT_FIT;
        }
    return PileError::NONE;
}

//-----------------------------------------------------------------------------
/// Checks if the passed pile is valid
/// \param pile Pile to check
/// \returns PileError Reason why the pile is invalid
//-----------------------------------------------------------------------------
PileError checkPile(const Pile& pile) {
    unsigned int pos(0);
    return checkPile(pile, pos);
}

//-----------------------------------------------------------------------------
/// Returns the first invalid pile on the table
/// \param table Piles on the table
/// \returns int Position of the first invalid pile or -1, if all are valid
//-----------------------------------------------------------------------------
int firstInvalidPile(const Table& table) {
    for (unsigned int i(0); i < table.size(); ++i)
        if (checkPile(table[i]) != PileError::NONE)
            return i;
    return -1;
}

//-----------------------------------------------------------------------------
/// Returns the number of cards moved from the hand
/// \returns unsigned int Number of cards
//-----------------------------------------------------------------------------
unsigned int Move::cardsFromHand() const {
    unsigned int nr(0);
    for (const auto& transfer : transfers)
        if (transfer.pile == HAND)
            nr += transfer.number();
    return nr;
}

//-----------------------------------------------------------------------------
/// Executes the passed move (like the GUI does): A new pile is created at the
/// end of the table, if needed, then the cards of the transfers are moved one
/// after another to their positions. Piles getting empty are NOT removed.
/// \param hand Cards of the player
/// \param table Piles on the table
/// \param move Move to execute
/// \pre The move must be valid (positions in range)
//-----------------------------------------------------------------------------
void applyMove(Card::Cards& hand, Table& table, const Move& move) {
    Check1(move.dest <= table.size());
    if (move.dest == table.size())
        table.emplace_back();

    for (const auto& transfer : move.transfers) {
        Check1(transfer.first <= transfer.last);
        Check1(transfer.pile != move.dest);
        unsigned int dest(transfer.destPos);
        for (unsigned int i(transfer.first); i <= transfer.last; ++i) {
            Value card;
            if (transfer.pile == HAND) {
                card = hand[transfer.first];
                hand.erase(hand.begin() + transfer.first);
            }
            else
                card = table[transfer.pile].remove(transfer.first);
            table[move.dest].insert(card, dest++);
        }
    }
}

//-----------------------------------------------------------------------------
/// Returns the (untranslated) message describing the passed error
/// \param error Error to describe
/// \returns const char* Message (to be translated with gettext)
//-----------------------------------------------------------------------------
const char* describe(MoveError error) {
    switch (error) {
    case MoveError::NONE:
        break;
    case MoveError::DOES_NOT_FIT:
        return N_("This card does not fit on that pile!");
    case MoveError::SPLIT_ORIGIN_FIRST:
        return N_("Can't move this card - try splitting the origin first!");
    }
    return "";
}

//-----------------------------------------------------------------------------
/// Checks if the human player can move the card he dropped on a pile and
/// calculates which cards are moved:
/// - From the hand exactly the dropped card is moved
/// - From a coloured pile to a new pile the dropped card and all following
///   ones are moved
/// - From a coloured pile to the left of a pile (without type or with a
///   single card) the dropped card and all previous ones are moved
/// \param hand Cards in the hand of the player
/// \param table Piles on the table
/// \param srcPile Pile from which the card is taken (HAND for the hand)
/// \param pos Position of the dragged card in its source
/// \param destPile Pile to which the card is dropped (number of piles for a
///     new pile); must be different to \c srcPile
/// \param move Set to the move to execute
/// \returns MoveError Reason why the move is not possible
//-----------------------------------------------------------------------------
MoveError checkMove(const Card::Cards& hand, const Table& table, unsigned int srcPile, unsigned int pos, unsigned int destPile,
                    Move& move) {
    const bool fromHand(srcPile == HAND);
    Check1(fromHand || (srcPile < table.size()));
    Check1(destPile <= table.size());
    Check1(srcPile != destPile);

    const Card::Cards& src(fromHand ? hand : table[srcPile].cards);
    Check1(pos < src.size());
    const Value moved(src[pos]);
    unsigned int nr((fromHand || (table[srcPile].type == NUMBER)) ? 1 : (src.size() - pos));
    unsigned int iCard(0);

    if (destPile < table.size()) {
        const Pile& pile(table[destPile]);
        iCard = getPosition4Card(pile, moved);
        if (iCard == -1U)
            return MoveError::DOES_NOT_FIT;

        if (!fromHand) {
            const Pile& source(table[srcPile]);

            // Check if only cards from an edge are moved to a numbered pile
            if ((source.type == COLOUR) && ((pos != (source.size() - 1)) && pos) && (moved.number() == pile[0].number()))
                return MoveError::SPLIT_ORIGIN_FIRST;

            // Move only one card from/to a numbered pile
            if ((pile.type == NUMBER) ||
                ((pile.type == UNDEFINED) ? ((pile.size() == 1) && (pile[0].number() == moved.number())) : !iCard) ||
                (source.type == NUMBER) || ((source.type != NUMBER) && (iCard && (moved.number() == Value::ACE))))
                nr = 1;
            else
                // Move left part of pile, if inserted to the left of the target
                if (!iCard) {
                    nr = pos + 1;
                    pos = 0;
                }
        }
    }

    move = Move(destPile, {{srcPile, pos, pos + nr - 1, iCard}});
    return MoveError::NONE;
}

//----------------------------------------------------------------------------
/// Finds the next player still having cards
/// \param handSizes Number of cards in the hands of the players
/// \param player Player to find next player to
/// \return unsigned int Next player having cards
/// \remarks If no player has cards the passed player is returned
//----------------------------------------------------------------------------
unsigned int findNextPlayer(const std::array<unsigned int, NUM_PLAYERS>& handSizes, unsigned int player) {
    for (unsigned int i(0); i < NUM_PLAYERS; ++i) {
        player = (player + 1) % NUM_PLAYERS;
        if (handSizes[player])
            break;
    }
    Check3(player < NUM_PLAYERS);
    return player;
}

//----------------------------------------------------------------------------
/// Checks if the game is over: That is the case, if only the next player still
/// has cards (he has lost)
/// \param handSizes Number of cards in the hands of the players
/// \param nextPlayer Next player (see findNextPlayer)
/// \returns bool True, if the game is over
//----------------------------------------------------------------------------
bool isGameOver(const std::array<unsigned int, NUM_PLAYERS>& handSizes, unsigned int nextPlayer) {
    return nextPlayer == findNextPlayer(handSizes, nextPlayer);
}

//----------------------------------------------------------------------------
/// Returns the position where a computer player puts a dealt card into his
/// hand: Before the first card with an equal or higher number (or at the end)
/// \param hand Cards in the hand of the player (sorted by number)
/// \param card Dealt card
/// \returns unsigned int Position for the card
//----------------------------------------------------------------------------
unsigned int dealPosition(const Card::Cards& hand, const Value& card) {
    return std::ranges::lower_bound(hand, card, Card::lessByNumber) - hand.begin();
}

//-----------------------------------------------------------------------------
/// Returns a card fitting to the passed on (having a distance of -2 to 2)
/// \param cards Cards to inspect
/// \param card Card where to find a fitting one to
/// \param start Position where to start the search
/// \returns unsigned int Position of matching card or the size of \c cards
//-----------------------------------------------------------------------------
unsigned int getFittingCard(const Card::Cards& cards, const Value& card, unsigned int start) {
    while ((start < cards.size()) && ((static_cast<unsigned int>(cardDistance(card, cards[start]) + 2)) >= 5))
        ++start;
    return std::min(start, static_cast<unsigned int>(cards.size()));
}

//-----------------------------------------------------------------------------
/// Moves the card at position source to position dest
/// \param cards Cards to manipulate
/// \param dest New position of card
/// \param source Position of card to move
//-----------------------------------------------------------------------------
void moveCard(Card::Cards& cards, unsigned int dest, unsigned int source) {
    Check1(source < cards.size());
    Check1(dest < cards.size());
    Value card(cards[source]);
    cards.erase(cards.begin() + source);
    cards.insert(cards.begin() + dest, card);
}

//----------------------------------------------------------------------------
/// Gets a series of cards matching the passed one (with a distance of -2 to 2
/// - see cardDistance); like Card::IPile::getSeries().
/// \param cards Cards to inspect; when filtering doubles a found card might be
///     moved to the position of the (first) double
/// \param posCard Position of the card to compare; updated, if it is moved
/// \param aPos Map holding the positions of the cards in the pile (indexed by
///     their difference + 2 to the card)
/// \param aOrder Order in which the cards have been found
/// \param doubles True, if the same card (id) can be included more than once
/// \returns unsigned int The number of cards with the same number
//----------------------------------------------------------------------------
unsigned int getSeries(Card::Cards& cards, unsigned int& posCard, std::map<unsigned int, unsigned int>& aPos,
                       std::vector<unsigned int>& aOrder, bool doubles) {
    Check1(posCard < cards.size());
    const Value card(cards[posCard]);
    TRACE1("MachiavelliRules::getSeries(...) for " << card);
    unsigned int nrs(0);
    unsigned int bCols(0x4);

    std::vector<unsigned int> foundCards;
    foundCards.reserve(4);
    unsigned int cDoubles(0);
    if (!doubles)
        foundCards.push_back(card.id());

    for (unsigned int p(0); ((p = getFittingCard(cards, card, p)) < cards.size()); ++p) {
        // The card itself goes always in the middle (the 3rd position)
        if (p == posCard) {
            aPos[2] = p;
            aOrder.push_back(2);
            ++nrs;
        }
        else {
            int diff(cardDistance(cards[p], card));
            TRACE1("MachiavelliRules::getSeries(...) - " << cards[p] << " differs " << diff);
            Check3(static_cast<unsigned int>(diff + 2) < 5);
            if (diff) {
                diff += 2;
                if (!(bCols & (1 << diff))) {
                    Check3(aPos.find(diff) == aPos.end());
                    bCols |= (1 << diff);
                    aPos[diff] = p;
                    aOrder.push_back(diff);
                    if (aPos.size() == 7)
                        break;
                }
            }
            else {
                // Filter out doubles (if specififed)
                if (!doubles) {
                    if (std::ranges::contains(foundCards, cards[p].id())) {
                        ++cDoubles;
                        continue;
                    }

                    foundCards.push_back(cards[p].id());
                    TRACE1("MachiavelliRules::getSeries(...) - Adding non-double " << cards[p]);

                    if (cDoubles) {
                        const unsigned int dest(p - cDoubles);
                        moveCard(cards, dest, p);
                        if ((posCard >= dest) && (posCard < p))
                            ++posCard;
                    }
                }
                ++nrs;
            }
        }
    }

    // Check if the series of colors is a valid one
    TRACE1("MachiavelliRules::getSeries(...) - Serie: " << std::hex << bCols << std::dec);
    Check3(bCols & 0x4);

    // Delete cards having no direct access to the analysed one
    if ((bCols & 0x3) == 0x1) {
        Check3(aPos.find(1) == aPos.end());
        deleteElement(0, aPos, aOrder);
        bCols &= ~0x1;
    }
    if ((bCols & 0x18) == 0x10) {
        Check3(aPos.find(3) == aPos.end());
        deleteElement(4, aPos, aOrder);
        bCols &= ~0x10;
    }

    // Special handling of series of colours for an ace, to avoid the problem
    // with 3-K-A of one colour.
    if (card.number() == Value::ACE) {
        if ((bCols & 0xa) == 0xa) {
            if ((bCols & 0x18) == 0x18) {
                if (aPos.find(0) != aPos.end())
                    deleteElement(0, aPos, aOrder);
                if (aPos.find(1) != aPos.end())
                    deleteElement(1, aPos, aOrder);
            }
            else {
                if (aPos.find(3) != aPos.end())
                    deleteElement(3, aPos, aOrder);
                if (aPos.find(4) != aPos.end())
                    deleteElement(4, aPos, aOrder);
            }
        }
    }

    return nrs;
}

//----------------------------------------------------------------------------
/// Sorts a series of matching cards (found by getSeries) to the end of the
/// cards; like Card::IPile::sortColourSerie()
/// \param cards Cards to sort
/// \param aPos Map holding the positions of the cards
/// \param aOrder Order of the found cards
/// \returns unsigned int Position of start of sorted serie
//----------------------------------------------------------------------------
unsigned int sortColourSerie(Card::Cards& cards, std::map<unsigned int, unsigned int>& aPos, std::vector<unsigned int>& aOrder) {
    unsigned int pos(1);
    for (auto p(aOrder.rbegin()); p != aOrder.rend(); ++p) {
        auto v(aPos.find(*p));
        Check3(v != aPos.end());
        TRACE9("MachiavelliRules::sortColourSerie(...) - Moving " << v->second << " to end " << ((*p < 2) ? *p : 0));
        // Move the card to the end of the staple; If it belongs before the
        // first card move it before the other cards.
        moveCard(cards, cards.size() - pos, v->second);
        unsigned int oldOrder(*p);
        if (((p + 1) != aOrder.rend()) && (*(p + 1) < oldOrder))
            ++pos;
    }
    TRACE9("MachiavelliRules::sortColourSerie(...) - Moved to pos " << cards.size() - aPos.size());
    return cards.size() - aPos.size();
}

//-----------------------------------------------------------------------------
/// Searches the move of the computer player: Either a serie from the hand, a
/// card from the hand fitting (somehow) to a pile on the table or a
/// re-ordering of the table (enabling playing a card)
/// \param hand Cards of the player (sorted by number); might be re-ordered
/// \param table Piles on the table (which must be valid)
/// \returns Move Move to make; empty if the player can't play any more
//-----------------------------------------------------------------------------
Move selectMove(Card::Cards& hand, const Table& table) {
    TRACE2("MachiavelliRules::selectMove(...) - Hand " << hand);
    Move move;
    if ((hand.size() > 2) && playSerie(hand, table, move))
        return move;

    if (table.size()) {
        // Check if any cards fits somewhere/somehow on an existing pile
        for (unsigned int p(0); p < hand.size(); ++p)
            if (cardFitsOnPile(hand[p], p, table, move))
                return move;

        // Try to re-order the piles to enable playing of other cards
        Missing missing;
        if (reorderTableToFit(hand, table, missing, move))
            return move;
    }
    return Move();
}

} // namespace MachiavelliRules
