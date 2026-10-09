#ifndef JABBERWOCKYRULES_H
#define JABBERWOCKYRULES_H

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

#include <card/Value.h>

/**Rules and computer player of the Jabberwocky cardgame, independent of its
   display.

   Hands are expected to be sorted by colour, with the trumps as last colour
   (see JabberwockyRules::lessByColourAccTrump); cards are identified by their
   position in the hand.
 */
namespace JabberwockyRules {

static constexpr unsigned int NUM_PLAYERS = 4;
/// Number of rounds of a game
static constexpr unsigned int NUM_ROUNDS = 13;

/// Array holding the position of the last card of each colour (or -1)
using ColourPositions = std::array<int, 4>;

/// Returns the number of tricks (and so the number of cards dealt to each
/// player) of the passed round (0 - NUM_ROUNDS - 1): 3, 4, ... 9, 8, ... 3
constexpr unsigned int tricksOfRound(unsigned int round) { return (round < 7) ? (round + 3) : (15 - round); }

/// Returns the player starting (bidding and playing) the round after the passed one
constexpr unsigned int nextStartPlayer(unsigned int previous) { return (previous + 1) % NUM_PLAYERS; }

/// Returns the position of the passed colour, when sorting with the passed trump
constexpr unsigned int sortOrder(Card::Value::COLOURS colour, Card::Value::COLOURS trump) {
    return (static_cast<unsigned int>(colour) - trump + 3) % NUM_PLAYERS;
}
/// Orders cards by colour (trumps last) and inside the colour by number
constexpr bool lessByColourAccTrump(const Card::Value& a, const Card::Value& b, Card::Value::COLOURS trump) {
    return (a.colour() == b.colour()) ? (a.number() < b.number()) : (sortOrder(a.colour(), trump) < sortOrder(b.colour(), trump));
}

/// Information about the actual round, needed to decide which cards can be played
struct Table {
    Card::Value trump;                                        ///< Card defining the trump (shown on the table)
    unsigned int round;                                       ///< Actual round (0 - NUM_ROUNDS - 1)
    Card::Cards trick;                                        ///< Cards played in the actual trick
    std::array<std::bitset<13>, 4> played;                    ///< Played cards (incl. the trick and the trump card)
    std::array<std::array<bool, 4>, NUM_PLAYERS> outOfColour; ///< Colours, a player (presumably) doesn't have

    Table(const Card::Value& trumpCard, unsigned int actRound);

    Card::Value::COLOURS trumpColour() const { return trump.colour(); }
    unsigned int tricks() const { return tricksOfRound(round); }

    /// Registers the passed card as played (in the trick)
    void play(const Card::Value& card);
    /// Removes the trick (after it has been won)
    void clearTrick() { trick.clear(); }
};

/// Reasons why a card must not be played
enum class PlayError { NONE, FOLLOW_COLOUR, NO_TRUMP_TO_START };
/// Reasons why a bid is not allowed
enum class BidError { NONE, INVALID, SUM_EQUALS_TRICKS };

/// Returns the (untranslated) message describing the passed error
const char* describe(PlayError error);
/// Returns the (untranslated) message describing the passed error
const char* describe(BidError error);

PlayError checkPlay(const Card::Cards& hand, unsigned int pos, const Table& table);
BidError checkBid(unsigned int bid, unsigned int sumOtherBids, bool lastBid, unsigned int round);

unsigned int trickWinner(const Card::Cards& trick, Card::Value::COLOURS trump);
std::array<int, NUM_PLAYERS> roundScore(const std::array<unsigned int, NUM_PLAYERS>& bids,
                                        const std::array<unsigned int, NUM_PLAYERS>& tricks);

ColourPositions positionsOfColours(const Card::Cards& hand);

/// \name Computer player
//@{
unsigned int estimateTricks(const Card::Cards& hand, Card::Value::COLOURS trump, unsigned int round,
                            unsigned int cardsInGame = Card::Value::CARDS_PER_DECK);
unsigned int selectBid(const Card::Cards& hand, Card::Value::COLOURS trump, unsigned int round, bool lastBid,
                       unsigned int sumOtherBids, unsigned int cardsInGame = Card::Value::CARDS_PER_DECK);
unsigned int selectCardToPlay(const Card::Cards& hand, unsigned int player, unsigned int bid, unsigned int tricksWon,
                              Table& table);
//@}

} // namespace JabberwockyRules

#endif
