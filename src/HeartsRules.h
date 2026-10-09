#ifndef HEARTSRULES_H
#define HEARTSRULES_H

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

#include <array>

#include <card/Value.h>

/**Rules and computer player of the Hearts cardgame, independent of its
   display.

   Hands are expected to be sorted by colour (see Card::lessByColour);
   cards are identified by their position in the hand.
 */
namespace HeartsRules {

static constexpr unsigned int NUM_PLAYERS = 4;
/// Number of cards each player exchanges before playing
static constexpr unsigned int CARDS_TO_EXCHANGE = 3;
/// Points of all cards of a round
static constexpr unsigned int ALL_POINTS = 26;

/// Array holding the position of the last card of each colour (or -1)
using ColourPositions = std::array<int, 4>;

/// Information about the actual round, needed to decide which cards can be played
struct Table {
    Card::Cards trick;                  ///< Cards played in the actual trick
    std::array<unsigned int, 4> played; ///< Number of played cards of each colour (incl. the trick)
    bool playedSQ;                      ///< Flag, if the queen of spades has been played
    unsigned int cardsWon;              ///< Number of cards won (by all players) so far
    unsigned int cardsInGame;           ///< Number of cards of the game

    Table() : trick(), played{}, playedSQ(false), cardsWon(0), cardsInGame(Card::Value::CARDS_PER_DECK) {}

    /// Registers the passed card as played (in the trick)
    void play(const Card::Value& card);
    /// Removes the trick (after it has been won)
    void clearTrick();
};

/// Reasons why a card must not be played
enum class PlayError { NONE, FOLLOW_COLOUR, START_WITH_TWO_OF_CLUBS, NO_HEART_TO_START, NO_QUEEN_OF_SPADES_IN_FIRST_ROUND, NO_HEART_IN_FIRST_ROUND };

/// Returns the (untranslated) message describing the passed error
const char* describe(PlayError error);

PlayError checkPlay(const Card::Cards& hand, unsigned int pos, const Table& table);

unsigned int trickWinner(const Card::Cards& trick);
unsigned int pointsOf(const Card::Cards& cards);
std::array<int, NUM_PLAYERS> roundScore(const std::array<unsigned int, NUM_PLAYERS>& points);
unsigned int startPlayer(const std::array<Card::Cards, NUM_PLAYERS>& hands);

ColourPositions positionsOfColours(const Card::Cards& hand);
unsigned int numberOfCards(const ColourPositions& positions, Card::Value::COLOURS colour);

/// \name Computer player
//@{
Card::Cards selectCardsToExchange(const Card::Cards& hand);
unsigned int selectCardToPlay(const Card::Cards& hand, const Table& table);
//@}

} // namespace HeartsRules

#endif
