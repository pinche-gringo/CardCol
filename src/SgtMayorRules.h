#ifndef SGTMAYORRULES_H
#define SGTMAYORRULES_H

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
#include <bitset>
#include <optional>

#include <card/Value.h>

/**Rules and computer player of the Sgt. Mayor cardgame, independent of its
   display.

   Three players play with a deck without the two of clubs (17 cards each).
   The start player needs 6 tricks, the next one 3 and the last one (who
   selects the special colour (trump)) 8. Players having made more tricks
   than needed exchange (in the next round) a bad card with a good one of a
   player having made less. The game ends if a player has collected
   (configurable, default END_POINTS) more tricks than needed.

   Hands are expected to be sorted by colour (see Card::lessByColour);
   cards are identified by their position in the hand.
 */
namespace SgtMayorRules {

static constexpr unsigned int NUM_PLAYERS = 3;
/// Number of cards each player gets (and therefore number of tricks)
static constexpr unsigned int CARDS_PER_PLAYER = (Card::Value::CARDS_PER_DECK - 1) / NUM_PLAYERS;
/// Default number of points (tricks made more than needed) ending the game
static constexpr unsigned int END_POINTS = 10;
/// Tricks needed by the players, starting with the start player
static constexpr std::array<unsigned int, NUM_PLAYERS> NEEDED_TRICKS{6, 3, 8};
/// Card which is not used in the game
static constexpr Card::Value UNUSED_CARD(Card::Value::of(Card::Value::CLUBS, Card::Value::TWO));

/// Information about the actual round, needed to decide which cards can be played
struct Table {
    Card::Cards trick;                     ///< Cards played in the actual trick
    Card::Value::COLOURS trump;            ///< Special colour (trump)
    std::array<std::bitset<13>, 4> played; ///< Cards played so far (per colour, by number)
    /// Colours players are assumed to miss: Bit (colour + 4 * player); only
    /// an estimation of the computer player (see selectCardToPlay)
    unsigned int outOfColours;

    Table() : trick(), trump(Card::Value::CLUBS), played(), outOfColours(0) {}

    /// Prepares the table for a new round (the trump is set after the exchange of cards)
    void newRound();
    /// Registers the passed card as played (in the trick)
    void play(const Card::Value& card);
    /// Removes the trick (after it has been won)
    void clearTrick() { trick.clear(); }
    /// Notes that a player can't follow the colour, after he played the
    /// passed card (which must not yet be in the trick)
    void noteNotFollowing(unsigned int player, const Card::Value& card);

    /// Checks if the passed card is the highest card of its colour, which
    /// has not been played
    bool isHighest(const Card::Value& card) const;
};

/// Reasons why a card must not be played
enum class PlayError { NONE, FOLLOW_COLOUR };

/// Returns the (untranslated) message describing the passed error
const char* describe(PlayError error);

/// \name Dealing and the start of a round
//@{
/// Checks if the passed card is used in the game
constexpr bool isUsed(const Card::Value& card) { return card != UNUSED_CARD; }
/// Returns the start player of the round following the one started by the passed player
constexpr unsigned int nextStartPlayer(unsigned int startPlayer) { return (startPlayer + 1) % NUM_PLAYERS; }
/// Returns the player selecting the special colour
constexpr unsigned int trumpPlayer(unsigned int startPlayer) { return (startPlayer + 2) % NUM_PLAYERS; }
/// Returns the number of tricks the passed player needs
constexpr unsigned int neededTricks(unsigned int player, unsigned int startPlayer) {
    return NEEDED_TRICKS[(player + NUM_PLAYERS - startPlayer) % NUM_PLAYERS];
}
//@}

/// \name Exchange of cards
//@{
/// Exchange of a bad card from a player having too much tricks with a good
/// card of a player having too less
struct Exchange {
    unsigned int playerBad;  ///< Player giving away a bad card (made too much tricks)
    unsigned int playerGood; ///< Player giving away a good card (made too less tricks)
};
std::optional<Exchange> nextExchange(const std::array<int, NUM_PLAYERS>& diffTricks, unsigned int startPlayer);
void applyExchange(std::array<int, NUM_PLAYERS>& diffTricks, const Exchange& exchange);
bool isValidExchange(const Card::Cards& handGood, unsigned int posGood, const Card::Value& bad);
//@}

/// \name Playing
//@{
PlayError checkPlay(const Card::Cards& hand, unsigned int pos, const Table& table);
unsigned int trickWinner(const Card::Cards& trick, Card::Value::COLOURS trump);
std::array<int, NUM_PLAYERS> roundScore(const std::array<unsigned int, NUM_PLAYERS>& tricks, unsigned int startPlayer);
bool isGameOver(const std::array<int, NUM_PLAYERS>& points, unsigned int endPoints = END_POINTS);
unsigned int winner(const std::array<int, NUM_PLAYERS>& points);
//@}

/// \name Computer player
//@{
Card::Value::COLOURS selectTrump(const Card::Cards& hand);
unsigned int selectBadCard(const Card::Cards& hand);
unsigned int selectGoodCard(const Card::Cards& handGood, const Card::Value& bad);
unsigned int selectCardToPlay(const Card::Cards& hand, unsigned int player, unsigned int startPlayer, Table& table);
//@}

} // namespace SgtMayorRules

#endif
