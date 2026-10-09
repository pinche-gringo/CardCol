// PROJECT     : Cardgames
// SUBSYSTEM   : Jabberwocky
// REFERENCES  :
// TODO        :
// BUGS        :
// AUTHOR      : Markus Schwab
// CREATED     : 9.10.2026
// COPYRIGHT   : Copyright (C) 2006 - 2018, 2024, 2026

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

#include "JabberwockyRules.h"

using Card::Value;

namespace JabberwockyRules {

namespace {

//----------------------------------------------------------------------------
/// Checks if the passed card might be the highest, considering the number of
/// tricks of the round
/// \param card Card to inspect
/// \param round Actual round
/// \return bool True, if card is likely highest unplayed one
//----------------------------------------------------------------------------
bool isHighEnough(const Value& card, unsigned int round) {
    return (card.number() >= static_cast<Value::NUMBERS>(Value::TEN + ((tricksOfRound(round) - 3) >> 1)));
}

//----------------------------------------------------------------------------
/// Checks if the passed card is the highest card of its colour, which has
/// not been played.
/// \param card Card to inspect
/// \param table Actual state of the round
/// \return bool True, if card is the highest unplayed one
//----------------------------------------------------------------------------
bool isHighest(const Value& card, const Table& table) {
    int nr(card.number());
    while (++nr <= Value::ACE)
        if (!table.played[card.colour()][nr])
            return false;
    return true;
}

//-----------------------------------------------------------------------------
/// Find the highest card lower than the passed one or the lowest card
/// of the colour of the passed card to match
/// \param cardCmp Card which should not be passed
/// \param hand Cards from which to play
/// \param posColour Position of last card in the hand with that colour
/// \returns unsigned int Position of card to play
//-----------------------------------------------------------------------------
unsigned int findLowerCard(const Value& cardCmp, const Card::Cards& hand, int posColour) {
    Check1(static_cast<unsigned int>(posColour) < hand.size());
    Check2(hand[posColour].colour() == cardCmp.colour());

    // Search for a lower card
    while ((posColour >= 0) && (hand[posColour].colour() == cardCmp.colour())) {
        if (hand[posColour].number() < cardCmp.number()) {
            TRACE5("JabberwockyRules::findLowerCard(...) - Playing card at " << posColour << ": " << hand[posColour]);
            return posColour;
        }
        --posColour;
    }

    TRACE5("JabberwockyRules::findLowerCard(...) - Forced to play card at " << posColour + 1);
    return posColour + 1;
}

//-----------------------------------------------------------------------------
/// Find the lowest card higher than the passed one
/// \param cardCmp Card which should not be passed
/// \param hand Cards from which to play
/// \param posColour Position of last card in the hand with that colour
/// \returns unsigned int Position of card to play or -1U
//-----------------------------------------------------------------------------
unsigned int findHigherCard(const Value& cardCmp, const Card::Cards& hand, unsigned int posColour) {
    Check1(posColour < hand.size());
    Check2(hand[posColour].colour() == cardCmp.colour());

    unsigned int pos(-1U);
    while (hand[posColour].number() > cardCmp.number()) {
        pos = posColour;
        if (!posColour-- || (hand[posColour].colour() != cardCmp.colour()))
            break;
    }

    TRACE5("JabberwockyRules::findHigherCard(...) - Playing card at " << pos);
    return pos;
}

//-----------------------------------------------------------------------------
/// Find the worst card to play (e.g. card not likely to win the trick)
/// \param hand Cards from which to play
/// \param positions Array with positions of cards
/// \param table Actual state of the round
/// \returns unsigned int Position of card to play
//-----------------------------------------------------------------------------
unsigned int findWorstCard(const Card::Cards& hand, const ColourPositions& positions, const Table& table) {
    Check1(hand.size());
    const Value::COLOURS trump(table.trumpColour());

    // Special handling of a hand full of trumps
    if (hand[0].colour() == trump)
        return 0;

    // Find lowest card (ignoring trumps)
    unsigned int pos(0);
    for (unsigned int i(0); i < 4; ++i)
        if ((positions[i] != -1) && (static_cast<unsigned int>(positions[i]) < (hand.size() - 1)) &&
            (hand[positions[i] + 1].colour() != trump))
            if ((hand[positions[i] + 1].number() < hand[pos].number()) ||
                ((hand[positions[i] + 1].number() == hand[pos].number()) &&
                 (table.played[hand[positions[i] + 1].colour()].count() < table.played[hand[pos].colour()].count())))
                pos = positions[i] + 1;

    TRACE8("JabberwockyRules::findWorstCard(...) - " << pos);
    return pos;
}

} // namespace

//-----------------------------------------------------------------------------
/// Constructor; registers the trump card as played
/// \param trumpCard Card defining the trump
/// \param actRound Actual round
//-----------------------------------------------------------------------------
Table::Table(const Value& trumpCard, unsigned int actRound)
    : trump(trumpCard), round(actRound), trick(), played(), outOfColour() {
    played[trump.colour()].set(trump.number());
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
/// Returns the (untranslated) message describing the passed error
/// \param error Error to describe
/// \returns const char* Message (to be translated with gettext)
//-----------------------------------------------------------------------------
const char* describe(PlayError error) {
    switch (error) {
    case PlayError::NONE:
        break;
    case PlayError::FOLLOW_COLOUR:
        return N_("Play first cards with an equal colour as the first played one!");
    case PlayError::NO_TRUMP_TO_START:
        return N_("You can't start with a trump,\nif they have not been played before!");
    }
    return "";
}

//-----------------------------------------------------------------------------
/// Returns the (untranslated) message describing the passed error
/// \param error Error to describe
/// \returns const char* Message (to be translated with gettext)
//-----------------------------------------------------------------------------
const char* describe(BidError error) {
    switch (error) {
    case BidError::NONE:
        break;
    case BidError::INVALID:
        return N_("Invalid bid!");
    case BidError::SUM_EQUALS_TRICKS:
        return N_("The sum of all bids must be different\nthan the number of possible tricks!");
    }
    return "";
}

//-----------------------------------------------------------------------------
/// Checks if the card at the passed position can be played
/// \param hand Cards of the player (sorted by colour, trumps last)
/// \param pos Position of the card to play
/// \param table Actual state of the round
/// \returns PlayError Reason why the card must not be played (or NONE)
//-----------------------------------------------------------------------------
PlayError checkPlay(const Card::Cards& hand, unsigned int pos, const Table& table) {
    Check1(pos < hand.size());
    const Value& card(hand[pos]);

    if (table.trick.size()) {
        // The same colour must be played again (if available)
        const Value::COLOURS first(table.trick[0].colour());
        if ((card.colour() != first) && Card::exists(hand, first))
            return PlayError::FOLLOW_COLOUR;
    }
    else {
        // One can start with a trump only if there has been one played before
        // (or if there is nothing else). Note that also the card shown as
        // trump is counted as played, so test accordingly
        const Value::COLOURS trump(table.trumpColour());
        if ((card.colour() == trump) && (table.played[trump].count() == 1) &&
            std::ranges::any_of(hand, [trump](const Value& c) { return c.colour() != trump; }))
            return PlayError::NO_TRUMP_TO_START;
    }
    return PlayError::NONE;
}

//-----------------------------------------------------------------------------
/// Checks if the passed bid is allowed
/// \param bid Bid of the player
/// \param sumOtherBids Sum of the bids of the other players
/// \param lastBid Flag, if the bid is the last one of the round
/// \param round Actual round
/// \returns BidError Reason why the bid is not allowed (or NONE)
/// \remarks The sum of all bids must differ from the number of tricks, so
///     not every player can meet his bid
//-----------------------------------------------------------------------------
BidError checkBid(unsigned int bid, unsigned int sumOtherBids, bool lastBid, unsigned int round) {
    if (bid > tricksOfRound(round))
        return BidError::INVALID;
    if (lastBid && ((sumOtherBids + bid) == tricksOfRound(round)))
        return BidError::SUM_EQUALS_TRICKS;
    return BidError::NONE;
}

//-----------------------------------------------------------------------------
/// Checks who has played the highest card and would therefore win the trick:
/// The highest trump or (if none has been played) the highest card of the
/// colour played first
/// \param trick Played cards
/// \param trump Colour of the trumps
/// \returns unsigned int Position (in the trick) of the highest card
//-----------------------------------------------------------------------------
unsigned int trickWinner(const Card::Cards& trick, Value::COLOURS trump) {
    Check1(trick.size());
    Value::NUMBERS nr(trick[0].number());
    Value::COLOURS col(trick[0].colour());
    unsigned int best(0);

    for (unsigned int i(1); i < trick.size(); ++i) {
        if ((col != trump) && (trick[i].colour() == trump)) {
            col = trump;
            nr = trick[i].number();
            best = i;
        }
        else if ((trick[i].number() > nr) && (trick[i].colour() == col)) {
            nr = trick[i].number();
            best = i;
        }
    }
    return best;
}

//-----------------------------------------------------------------------------
/// Calculates the score of a round: A player gets a point, if he has won
/// exactly the number of tricks he has bid.
/// \param bids Bids of the players
/// \param tricks Tricks won by each player
/// \returns std::array<int, NUM_PLAYERS> Score of each player
//-----------------------------------------------------------------------------
std::array<int, NUM_PLAYERS> roundScore(const std::array<unsigned int, NUM_PLAYERS>& bids,
                                        const std::array<unsigned int, NUM_PLAYERS>& tricks) {
    std::array<int, NUM_PLAYERS> score{};
    for (unsigned int i(0); i < NUM_PLAYERS; ++i)
        score[i] = (bids[i] == tricks[i]);
    return score;
}

//-----------------------------------------------------------------------------
/// Stores the last position of each colour in the hand
/// \param hand Cards to inspect (sorted by colour)
/// \returns ColourPositions Position of last card of each colour (or -1)
//-----------------------------------------------------------------------------
ColourPositions positionsOfColours(const Card::Cards& hand) {
    ColourPositions result;
    result.fill(-1);
    for (unsigned int i(0); i < hand.size(); ++i)
        result[hand[i].colour()] = i;

    TRACE9("JabberwockyRules::positionsOfColours(const Card::Cards&) - Pos. of cards: " << result[0] << ", " << result[1] << ", "
                                                                                        << result[2] << ", " << result[3]);
    return result;
}

//-----------------------------------------------------------------------------
/// Estimates the tricks the player will win with the passed cards
/// \param hand Cards of the player
/// \param trump Colour of the trumps
/// \param round Actual round
/// \param cardsInGame Number of cards of the game
/// \returns unsigned int Number of tricks player will (presumably) win
//-----------------------------------------------------------------------------
unsigned int estimateTricks(const Card::Cards& hand, Value::COLOURS trump, unsigned int round, unsigned int cardsInGame) {
    // Analyse cards to estimate tricks it will win
    unsigned int tricks(0);
    const unsigned int left(cardsInGame - 1 - NUM_PLAYERS * tricksOfRound(round));

    for (const auto& card : hand) {
        if (card.colour() == trump) {
            if ((card.number() > Value::EIGHT) || (left > 19))
                ++tricks;
            ++tricks;
        }
        else if (isHighEnough(card, round))
            tricks += 2;
    }

    TRACE5("JabberwockyRules::estimateTricks(...) - " << (tricks >> 1));
    return tricks >> 1;
}

//-----------------------------------------------------------------------------
/// Selects the bid of the computer player: The estimated tricks; the last
/// bidder takes care that the sum of all bids differs from the number of
/// tricks (by randomly bidding one more or less).
/// \param hand Cards of the player
/// \param trump Colour of the trumps
/// \param round Actual round
/// \param lastBid Flag, if the bid is the last one of the round
/// \param sumOtherBids Sum of the bids of the other players
/// \param cardsInGame Number of cards of the game
/// \returns unsigned int Bid
//-----------------------------------------------------------------------------
unsigned int selectBid(const Card::Cards& hand, Value::COLOURS trump, unsigned int round, bool lastBid, unsigned int sumOtherBids,
                       unsigned int cardsInGame) {
    unsigned int bid(estimateTricks(hand, trump, round, cardsInGame));
    if (checkBid(bid, sumOtherBids, lastBid, round) == BidError::SUM_EQUALS_TRICKS) {
        // Stay within 0 and the number of tricks
        if (!bid)
            bid = 1;
        else if (bid == tricksOfRound(round))
            --bid;
        else
            bid += Card::randomNumber(2) ? 1 : -1;
    }
    return bid;
}

//-----------------------------------------------------------------------------
/// Searches for the card the computer player plays
/// \param hand Cards of the player (sorted by colour, trumps last)
/// \param player Number of the player
/// \param bid Bid of the player
/// \param tricksWon Number of tricks the player has won so far
/// \param table Actual state of the round; the colours the player has (noticed
///     to be) out of are stored in it
/// \returns unsigned int Position of the card to play
//-----------------------------------------------------------------------------
unsigned int selectCardToPlay(const Card::Cards& hand, unsigned int player, unsigned int bid, unsigned int tricksWon,
                              Table& table) {
    TRACE3("JabberwockyRules::selectCardToPlay(...) - Player: " << player << "; missing tricks: "
                                                                << (static_cast<int>(bid) - static_cast<int>(tricksWon)));
    Check1(player < NUM_PLAYERS);
    Check1(hand.size());
    Check2(table.trick.size() < NUM_PLAYERS);

    const Card::Cards& played(table.trick);
    const Value::COLOURS trump(table.trumpColour());
    const ColourPositions aPosColours(positionsOfColours(hand));
    unsigned int pos2Play(-1U);

    // Already cards played?
    if (played.size()) {
        const unsigned int posWinner(trickWinner(played, trump));
        const Value& winner(played[posWinner]);
        const Value::COLOURS first(played[0].colour());
        TRACE4("JabberwockyRules::selectCardToPlay(...) - Winning card: " << posWinner << " (" << winner << ')');

        // If the player still has bids to fullfill
        if (bid > tricksWon) {
            // Can follow suit?
            if (aPosColours[first] == -1) {
                table.outOfColour[player][first] = true;

                unsigned int p(NUM_PLAYERS - played.size());
                Check3(p);
                while (--p) {
                    if (table.outOfColour[(player + p) % NUM_PLAYERS][first]) {
                        // Check if player can get the pile
                        pos2Play = (((aPosColours[trump] != -1) && (isHighest(hand[aPosColours[trump]], table) ||
                                                                    isHighEnough(hand[aPosColours[trump]], table.round)))
                                        ? aPosColours[trump]
                                        : findWorstCard(hand, aPosColours, table));
                        break;
                    }
                }
                // Other players still seem to have the colour
                if (!p) {
                    // Try to get the card with a trump; else put worst card
                    if ((aPosColours[trump] == -1) ||
                        ((winner.colour() == trump)
                             ? ((pos2Play = findHigherCard(winner, hand, aPosColours[winner.colour()])) == -1U)
                             : ((pos2Play = Card::findFirstEqualColour(hand, aPosColours[trump])) == -1U)))
                        pos2Play = findWorstCard(hand, aPosColours, table);
                }
                TRACE5("JabberwockyRules::selectCardToPlay(...) - Can't follow suit: " << pos2Play);
            }
            // Can follow suit
            else {
                const Value& highest(hand[aPosColours[first]]);

                // If a trump has been played after non-trump, play something low
                if (winner.colour() != first)
                    pos2Play = Card::findFirstEqualColour(hand, aPosColours[first]);
                // Last in turn
                else if (played.size() == (NUM_PLAYERS - 1))
                    pos2Play = ((highest.number() > winner.number()) ? findHigherCard(winner, hand, aPosColours[first])
                                                                     : Card::findFirstEqualColour(hand, aPosColours[first]));
                else if ((highest.number() > winner.number()) &&
                         (isHighest(highest, table) || isHighEnough(highest, table.round)))
                    pos2Play = aPosColours[first];
                else
                    pos2Play = Card::findFirstEqualColour(hand, aPosColours[first]);
                TRACE5("JabberwockyRules::selectCardToPlay(...) - Can follow suit: " << pos2Play);
            }
        }
        // Doesn't need any more tricks
        else if ((aPosColours[first] != -1) && (first == winner.colour()))
            pos2Play = findLowerCard(winner, hand, aPosColours[winner.colour()]);
        // The colour must be followed, even if the trick has been trumped (so
        // the card can't win anyway): Get rid of the highest one
        else if (aPosColours[first] != -1)
            pos2Play = aPosColours[first];
        else {
            unsigned int offset(0);
            for (unsigned int i(1); i < aPosColours.size(); ++i) {
                if ((static_cast<Value::COLOURS>(i) != trump) && (aPosColours[i] != -1) &&
                    ((aPosColours[offset] == -1) || ((hand[aPosColours[i]].number() > hand[aPosColours[offset]].number()) ||
                                                     ((hand[aPosColours[i]].number() == hand[aPosColours[offset]].number()) &&
                                                      (table.played[hand[aPosColours[i]].colour()].count() <
                                                       table.played[hand[aPosColours[offset]].colour()].count())))))
                    offset = i;
            }
            pos2Play = (aPosColours[offset] == -1) ? aPosColours[trump] : aPosColours[offset];
        }
    }
    // First card to play
    else {
        // If the player still has bids to fullfill
        if (bid < tricksWon) {
            // Try to eliminate trumps (if that's allowed)
            if (table.played[trump].count() && (aPosColours[trump] != -1) &&
                (isHighest(hand[aPosColours[trump]], table) || isHighEnough(hand[aPosColours[trump]], table.round)) &&
                (checkPlay(hand, aPosColours[trump], table) == PlayError::NONE))
                pos2Play = aPosColours[trump];
            else
                for (unsigned int i(0); i < 4; ++i)
                    if ((static_cast<Value::COLOURS>(i) != trump) && (aPosColours[i] != -1) &&
                        (isHighest(hand[aPosColours[i]], table) || isHighEnough(hand[aPosColours[i]], table.round)))
                        pos2Play = aPosColours[i];
        }

        if (pos2Play == -1U)
            pos2Play = findWorstCard(hand, aPosColours, table);
    }

    Check3(pos2Play < hand.size());
    TRACE1("JabberwockyRules::selectCardToPlay(...) - Playing: " << pos2Play << " (" << hand[pos2Play] << ')');
    return pos2Play;
}

} // namespace JabberwockyRules
