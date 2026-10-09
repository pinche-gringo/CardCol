#ifndef TWOPARTRULES_H
#define TWOPARTRULES_H

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
#include <optional>
#include <vector>

#include <card/Value.h>

/**Rules and computer player of the Twopart cardgame, independent of its
   display.

   The game has two parts:
     - Part 1: Every player has 3 cards in his hand (sorted by number) and
       plays one of them; afterwards he draws a card from the stock. The
       highest card of a round wins the played cards; if cards with an equal
       number have been played, the players having played the highest of
       those continue the round. The last card drawn from the stock defines
       the trump.
     - Part 2: The won cards are taken into the hand (sorted by colour, trumps
       last). Players have to beat the last played cards (with a bigger card of
       the same colour or a trump) or pick them up. The player keeping cards at
       last has lost.

   Cards are identified by their position in the hand (or the played cards).
 */
namespace TwopartRules {

static constexpr unsigned int NUM_PLAYERS = 4;
/// Number of cards in the hand of the players in part 1
static constexpr unsigned int CARDS_IN_HAND = 3;
/// Bitfield containing all players
static constexpr unsigned int ALL_PLAYERS = (1 << NUM_PLAYERS) - 1;

/// Colour of the trump; not set while there are cards on the stock
using Trump = std::optional<Card::Value::COLOURS>;
/// Number of cards in the hand of each player
using HandSizes = std::array<unsigned int, NUM_PLAYERS>;

/// Cards to play: Positions of the first and last card in the hand
struct Play {
    unsigned int start; ///< Position of first card to play
    unsigned int end;   ///< Position of last card to play
};

/// Result of the analysis of the played cards
struct Analysis {
    int max;         ///< Highest number (or -1)
    int maxPos;      ///< Position of (first) card with the highest number (or -1)
    int maxEqual;    ///< Highest number played more than once (or -1)
    int maxEqualPos; ///< Position of first card with that number (or -1)
    int trumps;      ///< Number of trumps
};

/// Result of the end of a turn (after playing cards)
struct TurnResult {
    unsigned int next; ///< Player to continue; at the end of a part: The one starting part 2 / having lost
    int winner;        ///< Part 1: Player winning the played cards at the end of a round (-1: Nobody)
    bool endOfRound;   ///< Flag, if the round has ended (part 2: the played cards are out of the game)
    bool endOfPart;    ///< Flag, if the part has ended (part 2: the game is over)
};

/// Result of picking up the played cards (in part 2)
struct PickUp {
    unsigned int start; ///< Position of the first played card to pick up (up to the last one)
    unsigned int next;  ///< Player to continue
};

/// Receivers of the won cards at the start of part 2: For every player the
/// receiver of each of his won cards, starting with the top card
using Receivers = std::array<std::vector<unsigned int>, NUM_PLAYERS>;

/// State of the game
struct Table {
    unsigned int bfPlayers;    ///< Players still in the round (bitfield)
    unsigned int bfOldPlayers; ///< Players at the start of the round (bitfield)
    unsigned int startPlayer;  ///< Player starting the round
    /// Part 1: [0] holds the start of the round in the played cards; part 2:
    /// Start of the plays still on the played cards
    std::array<unsigned int, NUM_PLAYERS> startPos;
    unsigned int offPos; ///< Number of plays in startPos (part 2)
    Trump trump;         ///< Colour of the trump
    bool partTwo;        ///< Flag, if part 2 is played

    Table() { reset(); }

    void reset();

    void removePlayer(unsigned int player) { bfPlayers &= ~(1U << player); }
    void addPlayer(unsigned int player) { bfPlayers |= 1U << player; }
    bool isInRound(unsigned int player) const { return bfPlayers & (1U << player); }
    /// Checks if the passed player is the only one left in the round
    bool isLastInRound(unsigned int player) const { return !(bfPlayers & ~(1U << player)); }

    int nextPlayer(unsigned int player) const;
    unsigned int pos2Player(unsigned int pos) const;
    unsigned int removePlayersWithoutCards(const HandSizes& hands);

    void registerPlay(unsigned int posInPlayed);
    TurnResult endTurn(unsigned int player, const Card::Cards& played, const HandSizes& hands);
    /// Registers, that the played cards have been moved to the winner (part 1)
    void playedCardsWon() { startPos[0] = 0; }
    PickUp pickUp(unsigned int player, const HandSizes& hands);
    Receivers startPartTwo(const std::array<Card::Cards, NUM_PLAYERS>& won, unsigned int& player);

  private:
    unsigned int endRoundPartOne(const Card::Cards& played, const HandSizes& hands, int& winner);
    int endRoundPartTwo(unsigned int player, const HandSizes& hands);
};

/// Reasons why cards must not be played
enum class PlayError { NONE, NOT_BIGGER, NO_SERIE, ONLY_ONE_CARD };

/// Returns the (untranslated) message describing the passed error (in the
/// plural form, if more than one card has been played)
const char* describe(PlayError error, bool plural = false);

PlayError checkPlay(const Card::Cards& hand, unsigned int start, unsigned int end, const Card::Cards& played, const Table& table);

unsigned int playersInBitfield(unsigned int bfPlayers);
int findNextPlayerWithCards(const HandSizes& hands, unsigned int player);
Analysis analyzePlayed(const Card::Cards& played, unsigned int start, unsigned int cards, const Trump& trump);

unsigned int findEndOfSerie(const Card::Cards& hand, unsigned int start);
unsigned int findStartOfSerie(const Card::Cards& hand, unsigned int start);

bool lessByColourAccTrumps(const Card::Value& a, const Card::Value& b, Card::Value::COLOURS trump);
void sortByColourAccTrumps(Card::Cards& cards, Card::Value::COLOURS trump);

/// \name Computer player
//@{
std::optional<Play> selectCardsToPlay(unsigned int player, const Card::Cards& hand, const Card::Cards& played,
                                      unsigned int wonCards, unsigned int stockSize, const Table& table);
unsigned int findSmallestCard(const Card::Cards& hand, Card::Value::COLOURS trump);
int findBigger(const Card::Cards& hand, Card::Value::NUMBERS nr, const Trump& trump);
//@}

} // namespace TwopartRules

#endif
