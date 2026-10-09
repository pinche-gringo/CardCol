// PROJECT     : Cardgames
// SUBSYSTEM   : Hearts
// REFERENCES  :
// TODO        :
// BUGS        :
// AUTHOR      : Markus Schwab
// CREATED     : 9.10.2026
// COPYRIGHT   : Copyright (C) 2002 - 2018, 2024, 2026

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

#include <YGP/Check.h>
#include <YGP/Trace.h>

#include <card/Cards.h>

#include "HeartsRules.h"

using Card::Value;

namespace HeartsRules {

namespace {

//-----------------------------------------------------------------------------
/// Find a lower card than the previously played ones
/// \param hand Cards from which to play
/// \param positions Array with positions of cards
/// \param table Actual state of the round
/// \returns unsigned int Position of card to play
//-----------------------------------------------------------------------------
unsigned int findLowerCard(const Card::Cards& hand, const ColourPositions& positions, const Table& table) {
    const Card::Cards& played(table.trick);
    Value::COLOURS colour(played[0].colour());
    unsigned int posWinner(trickWinner(played));

    // Play queen of spades, if there's already a higher card in the pile
    if ((colour == Value::SPADES) && (played[posWinner].number() > Value::QUEEN))
        return ((hand[positions[Value::SPADES]].number() == Value::QUEEN) || (numberOfCards(positions, Value::SPADES) == 1) ||
                (hand[positions[Value::SPADES] - 1].number() != Value::QUEEN))
                   ? positions[Value::SPADES]
                   : positions[Value::SPADES] - 1;

    // Play high card of the colour, if last player and pile contains no
    // counting card, except if that would mean to play the queen of spades.
    if ((played.size() == (NUM_PLAYERS - 1)) && !pointsOf(played))
        return ((colour != Value::SPADES) || (hand[positions[Value::SPADES]].number() != Value::QUEEN) ||
                numberOfCards(positions, Value::SPADES) == 1)
                   ? positions[colour]
                   : positions[colour] - 1;

    // Play highest card lower than the previously played ones
    Value::NUMBERS highest(played[posWinner].number());
    TRACE9("HeartsRules::findLowerCard(...) - Try to be below " << played[posWinner]);

    // Search for a lower card
    unsigned int nrCards(numberOfCards(positions, colour));
    int card(positions[colour] + 1);
    Check3(card >= 1);
    do {
        Check3(hand[card - 1].colour() == colour);
        if (hand[--card].number() < highest) {
            // Found a lower card; test if the highest is the ace of spades
            // and you have the queen and are about to play the king
            if ((colour == Value::SPADES) && (highest == Value::ACE) && card && hand[card - 1].is(Value::SPADES, Value::QUEEN))
                --card;

            TRACE5("HeartsRules::findLowerCard(...) - Playing card at " << card << ": " << hand[card]);
            return card;
        }
    }
    while (--nrCards);

    // Try to not play the queen of spades, if possible
    if ((colour == Value::SPADES) && (card < positions[Value::SPADES]) && (hand[card].number() == Value::QUEEN)) {
        Check3(hand[card + 1].colour() == Value::SPADES);
        ++card;
    }

    if (played.size() == (NUM_PLAYERS - 1))
        card = positions[colour];
    TRACE5("HeartsRules::findLowerCard(...) - Forced to play card at " << card << ": " << hand[card]);
    return card;
}

//-----------------------------------------------------------------------------
/// Find the worst card to play (defined as having the highest number of bad
/// points (like the queen of spades with 13 points and every heart with 1
/// point) or the highest numbered card).
/// \param hand Cards from which to play
/// \param positions Array with positions of cards
/// \param table Actual state of the round
/// \returns unsigned int Position of card to play or -1
//-----------------------------------------------------------------------------
unsigned int findWorstCard(const Card::Cards& hand, const ColourPositions& positions, const Table& table) {
    // If there are already some tricks won search for queen of spades or any heart
    if (table.cardsWon) {
        TRACE5("HeartsRules::findWorstCard(...) - Searching for SQ");
        if (positions[Value::SPADES] > 0)
            for (int pos((positions[Value::DIAMONDS] >= 0) ? positions[Value::DIAMONDS] + 1
                                                           : ((positions[Value::CLUBS] >= 0) ? positions[Value::CLUBS] + 1 : 0));
                 pos <= positions[Value::SPADES]; ++pos) {
                Check3(hand[pos].colour() == Value::SPADES);
                if (hand[pos].number() >= Value::QUEEN)
                    return static_cast<unsigned int>(pos);
            }

        // No high spade found: Try to play a heart
        TRACE5("HeartsRules::findWorstCard(...) - Searching for hearts");
        if (positions[Value::HEARTS] != -1)
            return positions[Value::HEARTS];
    }

    // If everything else failes: Play a high card
    TRACE5("HeartsRules::findWorstCard(...) - Searching for high cards");
    for (int card(Value::ACE); card >= Value::TWO; --card) {
        int pos(0);
        while ((pos = Card::find(hand, static_cast<Value::NUMBERS>(card), pos)) != -1) {
            Value::COLOURS colour(hand[pos].colour());
            if (table.cardsWon || ((colour == Value::SPADES) ? (card != Value::QUEEN) : (colour != Value::HEARTS)))
                return pos;
            ++pos;
        }
    }
    Check3(0);
    return -1U;
}

} // namespace

//-----------------------------------------------------------------------------
/// Registers the passed card as played (in the trick)
/// \param card Card played
//-----------------------------------------------------------------------------
void Table::play(const Value& card) {
    trick.push_back(card);
    ++played[card.colour()];
    if (card.is(Value::SPADES, Value::QUEEN))
        playedSQ = true;
}

//-----------------------------------------------------------------------------
/// Removes the trick (after it has been won)
//-----------------------------------------------------------------------------
void Table::clearTrick() {
    cardsWon += trick.size();
    trick.clear();
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
        return N_("You must play a card with an equal colour as the first played one!");
    case PlayError::START_WITH_TWO_OF_CLUBS:
        return N_("The game must be started with the two of clubs!");
    case PlayError::NO_HEART_TO_START:
        return N_("You can't start with a heart, if they have not been played before!");
    case PlayError::NO_QUEEN_OF_SPADES_IN_FIRST_ROUND:
        return N_("The queen of spades can't be played in the first round!");
    case PlayError::NO_HEART_IN_FIRST_ROUND:
        return N_("Hearts can't be played in the first round!");
    }
    return "";
}

//-----------------------------------------------------------------------------
/// Checks if the card at the passed position can be played
/// \param hand Cards of the player (sorted by colour)
/// \param pos Position of the card to play
/// \param table Actual state of the round
/// \returns PlayError Reason why the card must not be played (or NONE)
//-----------------------------------------------------------------------------
PlayError checkPlay(const Card::Cards& hand, unsigned int pos, const Table& table) {
    Check1(pos < hand.size());
    const Value& card(hand[pos]);
    const Value::COLOURS colour(card.colour());
    // Flag if a heart can be played; that is if hearts have been played before
    // or the player has nothing else (as the hand is sorted by colour)
    const bool heartAllowed((colour != Value::HEARTS) || table.played[Value::HEARTS] || (hand[0].colour() == Value::HEARTS));

    if (table.trick.size()) {
        // The same colour must be played again (if available)
        Value::COLOURS first(table.trick[0].colour());
        if ((colour != first) && Card::exists(hand, first))
            return PlayError::FOLLOW_COLOUR;
    }
    else {
        // The game must be started with the two of clubs
        if (!table.cardsWon && !card.is(Value::CLUBS, Value::TWO))
            return PlayError::START_WITH_TWO_OF_CLUBS;

        // One can start with a heart only if there has been one played before
        if (!heartAllowed)
            return PlayError::NO_HEART_TO_START;
    }

    // The queen of spades and hearts can't be played in the first round
    if (!table.cardsWon) {
        if (card.is(Value::SPADES, Value::QUEEN))
            return PlayError::NO_QUEEN_OF_SPADES_IN_FIRST_ROUND;
        if (!heartAllowed)
            return PlayError::NO_HEART_IN_FIRST_ROUND;
    }
    return PlayError::NONE;
}

//-----------------------------------------------------------------------------
/// Checks who has played the highest card and would therefore win the trick
/// \param trick Played cards
/// \returns unsigned int Position (in the trick) of the highest card
//-----------------------------------------------------------------------------
unsigned int trickWinner(const Card::Cards& trick) {
    Check1(trick.size());
    Value::COLOURS colour(trick[0].colour());
    Value::NUMBERS highest(trick[0].number());
    unsigned int pos(0);
    for (unsigned int i(1); i < trick.size(); ++i)
        if ((trick[i].colour() == colour) && (trick[i].number() > highest)) {
            pos = i;
            highest = trick[i].number();
        }
    return pos;
}

//-----------------------------------------------------------------------------
/// Counts the points in the passed cards. The queen of spades counts 13
/// points and every heart 1 point
/// \param cards Cards to inspect
/// \returns unsigned int Number of points
//-----------------------------------------------------------------------------
unsigned int pointsOf(const Card::Cards& cards) {
    unsigned int points(0);
    for (const auto& card : cards) {
        if (card.colour() == Value::HEARTS)
            ++points;
        else if (card.is(Value::SPADES, Value::QUEEN))
            points += 13;
    }
    TRACE7("HeartsRules::pointsOf(const Card::Cards&) - Number of points: " << points);
    return points;
}

//-----------------------------------------------------------------------------
/// Calculates the score of a round out of the points the players won. If a
/// player won all points ("shooting the moon") he gets none and every other
/// player all.
/// \param points Points won by each player
/// \returns std::array<int, NUM_PLAYERS> Score of each player
//-----------------------------------------------------------------------------
std::array<int, NUM_PLAYERS> roundScore(const std::array<unsigned int, NUM_PLAYERS>& points) {
    std::array<int, NUM_PLAYERS> score{};
    for (unsigned int i(0); i < NUM_PLAYERS; ++i) {
        if (points[i] == ALL_POINTS) {
            score.fill(ALL_POINTS);
            score[i] = 0;
            break;
        }
        score[i] = points[i];
    }
    return score;
}

//-----------------------------------------------------------------------------
/// Returns the player starting the round; that is the one holding the two of
/// clubs
/// \param hands Cards of the players (sorted by colour)
/// \returns unsigned int Starting player
//-----------------------------------------------------------------------------
unsigned int startPlayer(const std::array<Card::Cards, NUM_PLAYERS>& hands) {
    for (unsigned int i(1); i < NUM_PLAYERS; ++i)
        if (hands[i].size() && hands[i][0].is(Value::CLUBS, Value::TWO))
            return i;
    return 0;
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

    TRACE9("HeartsRules::positionsOfColours(const Card::Cards&) - Pos. of cards: " << result[0] << ", " << result[1] << ", "
                                                                                  << result[2] << ", " << result[3]);
    return result;
}

//-----------------------------------------------------------------------------
/// Calculate the number of cards out of the positions
/// \param positions Array of positions
/// \param colour Colour whose number should be calculated
/// \returns unsigned int Number of cards for colour
//-----------------------------------------------------------------------------
unsigned int numberOfCards(const ColourPositions& positions, Value::COLOURS colour) {
    Check1(colour <= Value::HEARTS);
    unsigned int nr(0), col(static_cast<unsigned int>(colour));
    if (positions[colour] != -1) {
        nr = positions[colour] + 1;
        while (col)
            if (positions[--col] != -1) {
                nr -= positions[col] + 1;
                break;
            }
    }
    return nr;
}

//-----------------------------------------------------------------------------
/// Selects the cards the computer player gives away before playing. Get rid
/// of cards according the following algorithm:
///    - If nr. of clubs or diamonds are < 3 -> Use them
///    - If nr. of spades < 5 get rid of high spades (especially the queen)
///    - Get rid of high cards
/// \param hand Cards of the player (sorted by colour)
/// \returns Card::Cards Cards to exchange
//-----------------------------------------------------------------------------
Card::Cards selectCardsToExchange(const Card::Cards& hand) {
    Card::Cards source(hand);
    Card::Cards result;
    auto take([&source, &result](unsigned int start, unsigned int end) {
        Check3(start <= end);
        Check3(end < source.size());
        result.insert(result.end(), source.begin() + start, source.begin() + end + 1);
        source.erase(source.begin() + start, source.begin() + end + 1);
    });

    ColourPositions posColours(positionsOfColours(source));

    unsigned int moved(0);
    unsigned int cCards(numberOfCards(posColours, Value::CLUBS));
    // If nr. of clubs or diamonds are < 3 -> Use them
    if (cCards && (cCards < CARDS_TO_EXCHANGE)) {
        TRACE3("HeartsRules::selectCardsToExchange(...) - Getting rid of all clubs: 0 - " << cCards - 1);
        take(0, posColours[Value::CLUBS]);
        moved = cCards;
    }
    cCards = numberOfCards(posColours, Value::DIAMONDS);
    if (cCards && (cCards < (CARDS_TO_EXCHANGE - moved))) {
        TRACE3("HeartsRules::selectCardsToExchange(...) - Getting rid of all diamonds");
        take(posColours[Value::DIAMONDS] - moved - cCards + 1, posColours[Value::DIAMONDS] - moved);
        moved += cCards;
    }

    // If nr. of spades < 5 get rid of high spades (especially the queen)
    cCards = numberOfCards(posColours, Value::SPADES);
    if (cCards && (cCards < 5)) {
        // Search for the queen of spades and get rid of cards equal or bigger
        unsigned int start(posColours[Value::SPADES] - moved - cCards + 1);
        while ((start <= (posColours[Value::SPADES] - moved - 1)) && (source[start].number() < Value::QUEEN)) {
            Check3(source[start].colour() == Value::SPADES);
            ++start;
        }

        if (source[start].is(Value::SPADES, Value::QUEEN)) {
            TRACE3("HeartsRules::selectCardsToExchange(...) - Getting rid of queen of spades at " << start);
            take(start, start);
            moved++;
        }

        if ((moved < CARDS_TO_EXCHANGE) && (start < (posColours[Value::SPADES] - moved))) {
            start = posColours[Value::SPADES] - moved;
            cCards = 2 - moved;
            TRACE3("HeartsRules::selectCardsToExchange(...) - Getting rid of all high spades: " << start - cCards << " - "
                                                                                              << start);
            take(start - cCards, start);
            moved += cCards + 1;
        }
    }

    // Get rid of high cards
    for (unsigned int nr(Value::ACE); moved < CARDS_TO_EXCHANGE; --nr) {
        Check3(nr > Value::TWO);
        int cardPos(Card::find(source, static_cast<Value::NUMBERS>(nr)));
        if (cardPos != -1) {
            TRACE3("HeartsRules::selectCardsToExchange(...) - Getting rid of high card at " << cardPos);
            take(cardPos, cardPos);
            moved++;
        }
    }
    return result;
}

//-----------------------------------------------------------------------------
/// Searches for the card the computer player plays
/// \param hand Cards of the player (sorted by colour)
/// \param table Actual state of the round
/// \returns unsigned int Position of the card to play
//-----------------------------------------------------------------------------
unsigned int selectCardToPlay(const Card::Cards& hand, const Table& table) {
    Check1(hand.size());
    ColourPositions aPos(positionsOfColours(hand));

    // The two of clubs starts the game (it's the lowest club, so the first card)
    if (!table.cardsWon && table.trick.empty())
        return 0;

    if (table.trick.size()) {
        // Check if cards of the same colour are available
        return (aPos[table.trick[0].colour()] == -1) ? findWorstCard(hand, aPos, table) : findLowerCard(hand, aPos, table);
    }

    // Player starts the round: If he has loads of spades: Play them
    unsigned int nrSpades(numberOfCards(aPos, Value::SPADES));
    unsigned int missingSpades(table.cardsInGame - nrSpades - table.played[Value::SPADES]);
    // If there are still spades left (with other players) and either the player has no high
    // spades or loads of spades: Play them
    if (missingSpades && !table.playedSQ &&
        (((aPos[Value::SPADES] != -1) && (hand[aPos[Value::SPADES]].number() < Value::QUEEN)) ||
         (((missingSpades / 3) + 1) < nrSpades))) {
        unsigned int pos((aPos[Value::DIAMONDS] >= 0) ? aPos[Value::DIAMONDS] + 1
                                                      : ((aPos[Value::CLUBS] >= 0) ? aPos[Value::CLUBS] + 1 : 0));
        TRACE5("HeartsRules::selectCardToPlay(...) - Starting with spade at " << pos << " (" << hand[pos] << ')');
        return pos;
    }

    // Else: Search for a low card
    for (unsigned int card(Value::TWO); card <= Value::ACE; ++card) {
        int pos(0);
        while ((pos = Card::find(hand, static_cast<Value::NUMBERS>(card), pos)) != -1) {
            Value::COLOURS colour(hand[pos].colour());
            // Play the lowest card, if there are still cards of that colour
            // owned by other players and - if it is a heart - there are
            // already played hearts.
            if ((table.played[colour] + numberOfCards(aPos, colour)) < (table.cardsInGame / NUM_PLAYERS)) {
                if ((colour != Value::HEARTS) || table.played[Value::HEARTS]) {
                    TRACE5("HeartsRules::selectCardToPlay(...) - Starting with " << hand[pos]);
                    return pos;
                }
            }
            ++pos;
        }
    }
    TRACE1("HeartsRules::selectCardToPlay(...) - All cards");
    return 0;
}

} // namespace HeartsRules
