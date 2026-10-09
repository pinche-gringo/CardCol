#ifndef ROVHULTRULES_H
#define ROVHULTRULES_H

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

/**Rules and computer player of the Rovhult cardgame, independent of its
   display.

   Hands are expected to be sorted by number (see Card::lessByNumber); cards
   are identified by their position in the hand. Every player has 3 reserve
   piles, each holding (from bottom to top) a hidden and a visible card; the
   top card of a pile is visible, if the pile holds 2 cards.
 */
namespace RovhultRules {

static constexpr unsigned int NUM_PLAYERS = 4;
/// Number of reserve piles of each player
static constexpr unsigned int NUM_RESERVE = 3;
/// Number of cards a player should hold in the hand (while there are cards on the staple)
static constexpr unsigned int CARDS_IN_HAND = 3;
/// Number of counted moves in the endgame (see playRandomly), after which the
/// game is ended (as the players might not be able to finish)
static constexpr unsigned int MAX_ENDGAME_MOVES = 1000;

/// Cards with a special meaning (configurable by the user)
struct Options {
    Card::Value::NUMBERS nuke{Card::Value::TEN};      ///< Clears the played cards; the player continues
    Card::Value::NUMBERS skip{Card::Value::EIGHT};    ///< Skips the next player
    Card::Value::NUMBERS reverse{Card::Value::SEVEN}; ///< The next card must be equal or smaller
};

/// Cards of a player
struct Player {
    Card::Cards hand;                             ///< Cards in the hand (sorted by number)
    std::array<Card::Cards, NUM_RESERVE> reserve; ///< Reserve piles (bottom card first)

    /// Checks if the player has any cards left
    bool hasCards() const;
    /// Checks if the top card of the passed reserve pile is visible
    bool topVisible(unsigned int pile) const { return reserve[pile].size() > 1; }
};

/// State of the game, needed to decide which cards can be played
struct Table {
    std::array<Player, NUM_PLAYERS> players;
    Card::Cards played; ///< Cards played on the table (since the last clearing)
};

/// A move of a player
struct Move {
    enum Source {
        HAND,    ///< Plays the cards start - end from the hand
        RESERVE, ///< Plays the top cards of the reserve piles start - end
        TAKE     ///< Takes the played cards
    };
    Source source{TAKE};
    unsigned int start{-1U};
    unsigned int end{-1U};

    bool operator==(const Move& other) const = default;
};

/// Consequences of a move
struct Turn {
    bool clearPlayed{false}; ///< Flag, if the played cards are removed
    bool finished{false};    ///< Flag, if the moving player has no cards left
    int skipped{-1};         ///< Player who is skipped (or -1)
    int loser{-1};           ///< Player who lost the game (if it has ended; else -1)
    unsigned int next{0};    ///< Next player in turn (if the game has not ended)
};

/// Reasons why a move is not allowed
enum class PlayError { NONE, SMALLER_AFTER_REVERSE, EQUAL_OR_BIGGER, VISIBLE_CARDS_FIRST, INVALID_MOVE };

/// Returns the (untranslated) message describing the passed error; the
/// SMALLER_AFTER_REVERSE message contains a "%1" for the name of the reverse card
const char* describe(PlayError error);

unsigned int valueOf(const Card::Value& card, const Options& options);
int compareCards(const Card::Value& lhs, const Card::Value& rhs, const Options& options);
bool isSpecialCard(Card::Value::NUMBERS nr);

PlayError checkCard(Card::Value::NUMBERS nr, const Card::Cards& played, const Options& options);
PlayError checkReserve(const Player& player, unsigned int pile);
PlayError checkMove(const Table& table, unsigned int player, const Move& move, const Options& options);
unsigned int firstPileToPlay(const Player& player, unsigned int pile);

unsigned int numberOfEqualTopCards(const Card::Cards& played);
int nextAvailablePlayer(const Table& table, unsigned int actPlayer);
unsigned int handSizeAfterMove(Card::Value::NUMBERS lastPlayed, unsigned int handSize, const Options& options);
Turn nextTurn(const Table& table, unsigned int player, const Options& options);

/// \name Dealing
//@{
void insertSorted(Card::Cards& hand, const Card::Value& card);
std::array<Player, NUM_PLAYERS> deal(Card::Cards& staple, unsigned int first);
void fillUp(Card::Cards& hand, Card::Cards& staple, unsigned int minCards);
void sortReserve(Player& player, const Options& options);
//@}

/// \name Computer player
//@{
void exchangeCards(Player& player, const Options& options);
bool playRandomly(unsigned int& cEndgame);
int loserOfEndlessGame(unsigned int cEndgame, const Table& table);
Move selectMove(const Table& table, unsigned int player, const Options& options);
Move selectRandomMove(const Table& table, unsigned int player, const Options& options);
//@}

} // namespace RovhultRules

#endif
