// PROJECT     : Cardgames
// SUBSYSTEM   : Sgt. Mayor
// REFERENCES  :
// TODO        :
// BUGS        :
// AUTHOR      : Markus Schwab
// CREATED     : 9.10.2026
// COPYRIGHT   : Copyright (C) 2004 - 2009, 2026

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

#include <algorithm>

#include <cardgames-cfg.h>

#include <YGP/Check.h>
#include <YGP/Trace.h>

#include <card/Cards.h>

#include "SgtMayorRules.h"

using Card::Value;

namespace SgtMayorRules {

namespace {

//-----------------------------------------------------------------------------
/// Returns the position of the first card equal or bigger (ordered by colour
/// and number) than the passed one
/// \param cards Cards to search (sorted by colour)
/// \param card Card to compare with
/// \returns unsigned int Position of the card or -1U
/// \remarks Could be moved to card/Cards.h (it's IPile::find1EqualOrBiggerByColour)
//-----------------------------------------------------------------------------
unsigned int find1EqualOrBiggerByColour(const Card::Cards& cards, const Value& card) {
    auto i(std::ranges::lower_bound(cards, card, Card::lessByColour));
    return (i == cards.end()) ? -1U : static_cast<unsigned int>(i - cards.begin());
}

//----------------------------------------------------------------------------
/// Tries to get the trick with a trump card; returns a bad card, if there's no
/// trump.
/// \param hand Cards to play from
/// \param trump Special colour
/// \returns unsigned int Position of card to play
//----------------------------------------------------------------------------
unsigned int tryToGetTrickWithTrump(const Card::Cards& hand, Value::COLOURS trump) {
    TRACE8("SgtMayorRules::tryToGetTrickWithTrump(const Card::Cards&, COLOURS) - Size " << hand.size());
    Check3(hand.size());

    unsigned int pos(Card::find(hand, trump));
    if (pos == -1U)
        pos = Card::findLowestCard(hand, trump);
    return pos;
}

} // namespace

//-----------------------------------------------------------------------------
/// Prepares the table for a new round: Nothing is played, except the (unused)
/// two of clubs
//-----------------------------------------------------------------------------
void Table::newRound() {
    trick.clear();
    for (auto& colour : played)
        colour.reset();
    played[UNUSED_CARD.colour()].set(UNUSED_CARD.number());
    outOfColours = 0;
}

//-----------------------------------------------------------------------------
/// Registers the passed card as played (in the trick)
/// \param card Card played
//-----------------------------------------------------------------------------
void Table::play(const Value& card) {
    trick.push_back(card);
    played[card.colour()].set(card.number());
}

//-----------------------------------------------------------------------------
/// Notes that a player can't follow the colour, after he played the passed
/// card. If he doesn't play a trump either, it's assumed he hasn't one.
/// \param player Player having played the card
/// \param card Card played (not yet in the trick)
//-----------------------------------------------------------------------------
void Table::noteNotFollowing(unsigned int player, const Value& card) {
    Check1(player < NUM_PLAYERS);
    if (trick.size() && (card.colour() != trick[0].colour())) {
        outOfColours |= ((1 << trick[0].colour()) << (player << 2));
        if (card.colour() != trump)
            outOfColours |= ((1 << trump) << (player << 2));
    }
}

//----------------------------------------------------------------------------
/// Checks if the passed card is the highest card of its colour, which has
/// not been played.
/// \param card Card to inspect
/// \returns bool True, if card is the highest unplayed one
//----------------------------------------------------------------------------
bool Table::isHighest(const Value& card) const {
    TRACE8("SgtMayorRules::Table::isHighest(const Value&) - " << card);

    int nr(card.number());
    while (++nr <= Value::ACE) {
        if (!played[card.colour()][nr])
            return false;
    }
    return true;
}

//-----------------------------------------------------------------------------
/// Returns the (untranslated) message describing the passed error
/// \param error Error to describe
/// \returns const char* Message (to be translated with gettext)
//-----------------------------------------------------------------------------
const char* describe(PlayError error) {
    switch (error) {
    case PlayError::NONE:
        break;
    case PlayError::FOLLOW_COLOUR:
        return N_("Play a card with an equal colour as the first played one!");
    }
    return "";
}

//-----------------------------------------------------------------------------
/// Returns the next exchange of cards: A player having made more tricks than
/// needed (starting the search with the start player) exchanges with the
/// (next) player having made less.
/// \param diffTricks Tricks the players made more (or less) than needed
/// \param startPlayer Player starting the round
/// \returns std::optional<Exchange> Players exchanging cards; empty if no (more) exchange
//-----------------------------------------------------------------------------
std::optional<Exchange> nextExchange(const std::array<int, NUM_PLAYERS>& diffTricks, unsigned int startPlayer) {
    Check3((diffTricks[0] + diffTricks[1]) == -diffTricks[2]);

    for (unsigned int i(startPlayer); (i - startPlayer) < NUM_PLAYERS; ++i) {
        TRACE9("SgtMayorRules::nextExchange(...) - " << i % NUM_PLAYERS << "'s tricks: " << diffTricks[i % NUM_PLAYERS]);
        if (diffTricks[i % NUM_PLAYERS] > 0) {
            Check3((diffTricks[(i + 1) % NUM_PLAYERS] < 0) || (diffTricks[(i + 2) % NUM_PLAYERS] < 0));
            for (unsigned int j(1); j < NUM_PLAYERS; ++j)
                if (diffTricks[(j + i) % NUM_PLAYERS] < 0)
                    return Exchange{i % NUM_PLAYERS, (i + j) % NUM_PLAYERS};
        }
    }
    return std::nullopt;
}

//-----------------------------------------------------------------------------
/// Registers an exchange of cards: Each exchanged card compensates one trick
/// \param diffTricks Tricks the players made more (or less) than needed; updated
/// \param exchange Exchange of cards
//-----------------------------------------------------------------------------
void applyExchange(std::array<int, NUM_PLAYERS>& diffTricks, const Exchange& exchange) {
    Check1(exchange.playerBad < NUM_PLAYERS);
    Check1(exchange.playerGood < NUM_PLAYERS);
    --diffTricks[exchange.playerBad];
    ++diffTricks[exchange.playerGood];
}

//-----------------------------------------------------------------------------
/// Checks if the good card is a valid replacement for the passed bad card:
/// It must have the same colour (if the player has that colour)
/// \param handGood Cards of the player giving away the good card
/// \param posGood Position of the good card
/// \param bad Bad card received
/// \returns bool True, if the good card can be given away
//-----------------------------------------------------------------------------
bool isValidExchange(const Card::Cards& handGood, unsigned int posGood, const Value& bad) {
    Check1(posGood < handGood.size());
    return (handGood[posGood].colour() == bad.colour()) || !Card::exists(handGood, bad.colour());
}

//-----------------------------------------------------------------------------
/// Checks if the card at the passed position can be played
/// \param hand Cards of the player
/// \param pos Position of the card to play
/// \param table Actual state of the round
/// \returns PlayError Reason why the card must not be played (or NONE)
//-----------------------------------------------------------------------------
PlayError checkPlay(const Card::Cards& hand, unsigned int pos, const Table& table) {
    Check1(pos < hand.size());
    // The same colour must be played again (if available)
    if (table.trick.size()) {
        Value::COLOURS colour(table.trick[0].colour());
        if ((hand[pos].colour() != colour) && Card::exists(hand, colour))
            return PlayError::FOLLOW_COLOUR;
    }
    return PlayError::NONE;
}

//-----------------------------------------------------------------------------
/// Checks who has played the highest card and would therefore win the trick
/// \param trick Played cards
/// \param trump Special colour
/// \returns unsigned int Position (in the trick) of the highest card
//-----------------------------------------------------------------------------
unsigned int trickWinner(const Card::Cards& trick, Value::COLOURS trump) {
    Check1(trick.size());
    Value::NUMBERS nr(trick[0].number());
    Value::COLOURS col(trick[0].colour());
    unsigned int best(0);

    for (unsigned int i(1); i < trick.size(); ++i) {
        TRACE8("SgtMayorRules::trickWinner(const Card::Cards&, COLOURS) - Comparing " << trick[i - 1] << " - " << trick[i]);
        if ((col != trump) && (trick[i].colour() == trump)) {
            TRACE9("SgtMayorRules::trickWinner(const Card::Cards&, COLOURS) - Found trump ");
            col = trump;
            nr = trick[i].number();
            best = i;
            continue;
        }

        if ((trick[i].number() > nr) && (trick[i].colour() == col)) {
            TRACE9("SgtMayorRules::trickWinner(const Card::Cards&, COLOURS) - New best card " << trick[i]);
            nr = trick[i].number();
            best = i;
        }
    }
    return best;
}

//-----------------------------------------------------------------------------
/// Calculates the score of a round: The tricks each player made more (or
/// less) than needed
/// \param tricks Tricks made by each player
/// \param startPlayer Player having started the round
/// \returns std::array<int, NUM_PLAYERS> Score of each player
//-----------------------------------------------------------------------------
std::array<int, NUM_PLAYERS> roundScore(const std::array<unsigned int, NUM_PLAYERS>& tricks, unsigned int startPlayer) {
    std::array<int, NUM_PLAYERS> score{};
    for (unsigned int i(0); i < NUM_PLAYERS; ++i) {
        score[i] = static_cast<int>(tricks[i]) - static_cast<int>(neededTricks(i, startPlayer));
        TRACE9("SgtMayorRules::roundScore(...) - Player " << i << " made " << tricks[i] << " = " << score[i]);
    }
    return score;
}

//-----------------------------------------------------------------------------
/// Checks if the game has ended; that is if a player has enough points
/// \param points Points of the players
/// \param endPoints Points ending the game
/// \returns bool True, if the game has ended
//-----------------------------------------------------------------------------
bool isGameOver(const std::array<int, NUM_PLAYERS>& points, unsigned int endPoints) {
    return points[winner(points)] >= static_cast<int>(endPoints);
}

//-----------------------------------------------------------------------------
/// Returns the player having the most points (the first one, if there are
/// more)
/// \param points Points of the players
/// \returns unsigned int Player with the most points
//-----------------------------------------------------------------------------
unsigned int winner(const std::array<int, NUM_PLAYERS>& points) { return std::ranges::max_element(points) - points.begin(); }

//-----------------------------------------------------------------------------
/// Selects the special colour: The colour with the most cards (or the
/// highest ones)
/// \param hand Cards of the player (sorted by colour)
/// \returns Value::COLOURS Special colour
/// \remarks The last card of the hand is not taken into account and each
///     colour is only compared with the one before
//-----------------------------------------------------------------------------
Value::COLOURS selectTrump(const Card::Cards& hand) {
    Check1(hand.size());
    std::array<int, 4> number{};
    std::array<int, 4> points{};

    for (unsigned int i(0); i < (hand.size() - 1); ++i) {
        ++number[hand[i].colour()];
        points[hand[i].colour()] += hand[i].number() + 1;
    }

    unsigned int trumpColour(0);
    for (unsigned int i(1); i < 4; ++i) {
        if ((number[i] > number[i - 1]) || ((number[i] == number[i - 1]) && (points[i] > points[i - 1])))
            trumpColour = i;
    }
    return static_cast<Value::COLOURS>(trumpColour);
}

//-----------------------------------------------------------------------------
/// Selects the bad card a player gives away, who made too much tricks: The
/// lowest one
/// \param hand Cards of the player
/// \returns unsigned int Position of the card to give away
//-----------------------------------------------------------------------------
unsigned int selectBadCard(const Card::Cards& hand) {
    Check1(hand.size());
    return Card::findLowestCard(hand);
}

//-----------------------------------------------------------------------------
/// Selects the good card a player gives away, who made too less tricks: The
/// highest card of the colour of the received card, or the lowest card, if he
/// doesn't have that colour
/// \param handGood Cards of the player (sorted by colour)
/// \param bad Card received
/// \returns unsigned int Position of the card to give away
//-----------------------------------------------------------------------------
unsigned int selectGoodCard(const Card::Cards& handGood, const Value& bad) {
    Check1(handGood.size());
    int pos(Card::findLastEqualOrBiggerColour(handGood, bad.colour()));
    return (pos == -1) ? Card::findLowestCard(handGood) : static_cast<unsigned int>(pos);
}

//-----------------------------------------------------------------------------
/// Searches for the card the computer player plays
/// \param hand Cards of the player (sorted by colour)
/// \param player Player to play
/// \param startPlayer Player having started the round
/// \param table Actual state of the round; the colours the player is assumed
///     to miss are updated
/// \returns unsigned int Position of the card to play
//-----------------------------------------------------------------------------
unsigned int selectCardToPlay(const Card::Cards& hand, unsigned int player, unsigned int startPlayer, Table& table) {
    TRACE8("SgtMayorRules::selectCardToPlay(...) - Player " << player);
    Check1(player < NUM_PLAYERS);
    Check1(hand.size());

    const Value::COLOURS trump(table.trump);
    const Card::Cards& played(table.trick);
    unsigned int& bfColours(table.outOfColours);
    const auto& playedCards(table.played);

    unsigned int pos(0);
    // Get the number of cards of each colour
    std::array<unsigned int, 4> cColours{};
    std::array<unsigned int, 4> posColours{-1U, -1U, -1U, -1U};
    for (unsigned int i(0); i < (hand.size() - 1); ++i) {
        ++cColours[hand[i].colour()];
        if (hand[i].colour() != hand[i + 1].colour())
            posColours[hand[i].colour()] = i;
    }
    posColours[hand[hand.size() - 1].colour()] = hand.size() - 1;
    ++cColours[hand[hand.size() - 1].colour()];
    TRACE8("SgtMayorRules::selectCardToPlay(...) - Nr: " << cColours[0] << '/' << cColours[1] << '/' << cColours[2] << '/'
                                                         << cColours[3]);
    TRACE9("SgtMayorRules::selectCardToPlay(...) - Out: " << std::hex << bfColours << std::dec);

    switch (played.size()) {
    case 0: {
        // - Play trumps?
        unsigned int trumpsLeft(13 - playedCards[trump].count());
        TRACE8("SgtMayorRules::selectCardToPlay(...) - Trumps: " << trumpsLeft << '/' << playedCards[trump].count());
        Check3(trumpsLeft <= 13);
        // If others have trumps left, but we have more; if player is not the
        // startplayer, assume, that the 3rd player has no more trumps left
        if ((trumpsLeft > cColours[trump]) && ((trumpsLeft / ((player == startPlayer) ? 3 : 2)) < cColours[trump])) {
            Check3(hand.size() > posColours[trump]);
            pos = posColours[trump];
            if (!table.isHighest(hand[posColours[trump]]))
                pos -= cColours[trump] - 1;
            break;
        }
        trumpsLeft -= cColours[trump];

        // - Have dead cards?
        int maxDiff(0);
        Value::COLOURS maxColour(Value::HEARTS);
        for (unsigned int i(0); i < cColours.size(); ++i) {
            int diff(static_cast<int>(cColours[i] - (13 - playedCards[i].count()) / 3));
            if ((diff > maxDiff) && (static_cast<int>(i) != trump)) {
                maxDiff = diff;
                maxColour = static_cast<Value::COLOURS>(i);
            }
        }
        TRACE8("SgtMayorRules::selectCardToPlay(...) - Dead cards: " << maxDiff << ": " << static_cast<int>(maxColour));
        if (maxDiff) {
            Check3(posColours[maxColour] != -1U);
            pos = posColours[maxColour];
            Check3((playedCards[maxColour].count() + cColours[maxColour]) <= 13);
            if (!table.isHighest(hand[posColours[maxColour]]) ||
                (trumpsLeft && (((playedCards[maxColour].count() + cColours[maxColour]) > 11) ||
                                ((bfColours & (0x111 << maxColour)) && !((bfColours >> maxColour) & (bfColours >> trump))))))
                pos -= cColours[maxColour] - 1;
            break;
        }

        // Try to find the highest card
        pos = 0;
        while (pos < hand.size()) {
            pos = Card::findLastEqualColour(hand, pos);
            if (table.isHighest(hand[pos]) && (hand[pos].colour() != trump) &&
                !(trumpsLeft && (bfColours & (0x110 << hand[pos].colour()))))
                break;
            ++pos;
        }

        if (pos >= hand.size())
            pos = Card::findLowestCard(hand, trump);
        break;
    }

    case 1: {
        const Value::COLOURS colour(played[0].colour());
        Check3((playedCards[colour].count() + cColours[colour]) <= 13);
        bool nextHasntColour((bfColours & ((1 << colour) << (((player + 1) % NUM_PLAYERS) << 2))) ||
                             ((playedCards[colour].count() + cColours[colour]) > 12));
        pos = posColours[colour];
        TRACE9("SgtMayorRules::selectCardToPlay(...) - Play: " << static_cast<int>(pos) << "; Next: " << nextHasntColour);

        if (pos == -1U) {
            Check3((playedCards[trump].count() + cColours[trump]) <= 13);
            bfColours |= ((1 << colour) << (player << 2));
            pos = posColours[trump];
            if ((pos == -1U) ||
                (nextHasntColour && !table.isHighest(hand[pos]) && ((cColours[trump] + playedCards[trump].count()) < 13))) {
                pos = Card::findLowestCard(hand, trump);
                bfColours |= (1 << trump);
            }
            else
                pos -= cColours[trump] - 1;
        }
        else
            // If the next is know to not have the colour or player has not the
            // highest left of this colour, play a low one
            if (nextHasntColour || !table.isHighest(hand[pos]) || played[0].number() > hand[pos].number())
                pos -= cColours[colour] - 1;
        break;
    }

    case 2:
        // The trick is taken by the second player with a trump. Either play a
        // small card or use a bigger trump.
        if ((played[0].colour() != trump) && (played[1].colour() == trump)) {
            pos = Card::find(hand, played[0].colour());
            if (pos == -1U) {
                pos = find1EqualOrBiggerByColour(hand, played[1]);
                if (pos == -1U) {
                    bfColours |= ((1 << played[0].colour()) << (player << 2));
                    pos = Card::findLowestCard(hand, trump);
                    if (hand[pos].colour() != trump)
                        bfColours |= (1 << trump);
                }
            }
        }
        else {
            pos = find1EqualOrBiggerByColour(
                hand, ((played[0].colour() != played[1].colour()) || (played[0].number() > played[1].number())) ? played[0]
                                                                                                                : played[1]);
            if ((pos == -1U) || (hand[pos].colour() != played[0].colour())) {
                pos = (Card::exists(hand, played[0].colour())
                           ? Card::find(hand, played[0].colour())
                           : (bfColours |= ((1 << played[0].colour()) << (player << 2)), tryToGetTrickWithTrump(hand, trump)));
                if (hand[pos].colour() != trump)
                    bfColours |= (1 << trump);
            }
        }
        break;

    default:
        Check3(0);
    }
    Check3(pos < hand.size());
    return pos;
}

} // namespace SgtMayorRules
