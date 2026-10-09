#ifndef MACHIAVELLIRULES_H
#define MACHIAVELLIRULES_H

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
#include <map>
#include <utility>
#include <vector>

#include <card/Value.h>

/**Rules and computer player of the Machiavelli cardgame, independent of its
   display.

   The table consists of piles, which are either a serie of (at least 3)
   cards of the same colour or (at least 3) cards having the same number
   (but different colours). Cards are identified by their position in the
   hand respectively in the pile; as the game is played with 4 decks equal
   cards exist.
 */
namespace MachiavelliRules {

static constexpr unsigned int NUM_PLAYERS = 4;
/// Number of decks the game is played with
static constexpr unsigned int NUM_DECKS = 4;
/// Number of cards dealt to each player
static constexpr unsigned int CARDS_PER_PLAYER = 7;
/// Minimal number of cards of a (valid) pile on the table
static constexpr unsigned int MIN_PILE_SIZE = 3;

/// Type of a pile on the table
enum PileType { UNDEFINED, NUMBER, COLOUR };

/// Flag, how aces are handled when calculating the distance of two cards
enum AceFlag {
    ACE,  ///< Ace is only the highest card
    BOTH, ///< Ace is (also) one, if the other card is a 2 or a 3
    ONE   ///< Ace is only one
};

int cardDistance(const Card::Value& a, const Card::Value& b, AceFlag aceIsOne = BOTH);

/// A pile on the table
struct Pile {
    Card::Cards cards; ///< Cards of the pile
    PileType type;     ///< Type of the pile; defined, when the pile gets its second card

    Pile() : cards(), type(UNDEFINED) {}
    Pile(Card::Cards cards, PileType type) : cards(std::move(cards)), type(type) {}

    /// Creates a pile out of the passed cards (as if they were added one after another)
    static Pile of(const Card::Cards& cards);

    unsigned int size() const { return cards.size(); }
    bool empty() const { return cards.empty(); }
    const Card::Value& operator[](unsigned int pos) const { return cards[pos]; }

    void insert(const Card::Value& card, unsigned int pos);
    Card::Value remove(unsigned int pos);
};

/// The piles on the table
using Table = std::vector<Pile>;

PileType analysePile(const Card::Cards& cards, PileType type);

unsigned int getPosition4Card(const Pile& pile, const Card::Value& card);
int getPosOfColour(const Pile& pile, Card::Value::COLOURS colour);
bool hasMatching3rd(const Pile& pile, Card::Cards& pair, unsigned int& match, unsigned int& nr);

/// Reasons why a pile on the table is invalid
enum class PileError { NONE, NOT_ENOUGH_CARDS, INVALID_TYPE, CARD_DOES_NOT_FIT };

/// Returns the (untranslated) message describing the passed error; for
/// CARD_DOES_NOT_FIT it contains %1 for the (1-based) position of the card
const char* describe(PileError error);

PileError checkPile(const Pile& pile, unsigned int& pos);
PileError checkPile(const Pile& pile);
int firstInvalidPile(const Table& table);

/// Position of the "pile" of the hand (as source of cards)
static constexpr unsigned int HAND = -1U;

/// Cards moved from a source to the destination of a move
struct Transfer {
    unsigned int pile;    ///< Source pile on the table (or HAND)
    unsigned int first;   ///< Position of the first card to move (in the source)
    unsigned int last;    ///< Position of the last card to move (in the source)
    unsigned int destPos; ///< Position of the first card in the destination (after the previous transfers)

    unsigned int number() const { return last - first + 1; }
    bool operator==(const Transfer& other) const = default;
};

/// A move: Cards moved from the hand and/or from piles on the table to one
/// pile on the table. The transfers are executed one after another (as the
/// animations of the GUI do).
struct Move {
    unsigned int dest;               ///< Destination pile (the number of piles for a new pile)
    std::vector<Transfer> transfers; ///< Cards to move

    Move() : dest(0), transfers() {}
    Move(unsigned int dest, std::vector<Transfer> transfers) : dest(dest), transfers(std::move(transfers)) {}

    bool empty() const { return transfers.empty(); }
    /// Returns the number of cards moved from the hand
    unsigned int cardsFromHand() const;
};

void applyMove(Card::Cards& hand, Table& table, const Move& move);

/// Reasons why a move of the human player is not possible
enum class MoveError { NONE, DOES_NOT_FIT, SPLIT_ORIGIN_FIRST };

/// Returns the (untranslated) message describing the passed error
const char* describe(MoveError error);

MoveError checkMove(const Card::Cards& hand, const Table& table, unsigned int srcPile, unsigned int pos, unsigned int destPile,
                    Move& move);

unsigned int findNextPlayer(const std::array<unsigned int, NUM_PLAYERS>& handSizes, unsigned int player);
bool isGameOver(const std::array<unsigned int, NUM_PLAYERS>& handSizes, unsigned int nextPlayer);
unsigned int dealPosition(const Card::Cards& hand, const Card::Value& card);

/// \name Helpers on cards (like the ones of Card::IPile, without display)
//@{
unsigned int getFittingCard(const Card::Cards& cards, const Card::Value& card, unsigned int start);
unsigned int getSeries(Card::Cards& cards, unsigned int& posCard, std::map<unsigned int, unsigned int>& aPos,
                       std::vector<unsigned int>& aOrder, bool doubles = true);
unsigned int sortColourSerie(Card::Cards& cards, std::map<unsigned int, unsigned int>& aPos, std::vector<unsigned int>& aOrder);
void moveCard(Card::Cards& cards, unsigned int dest, unsigned int source);
//@}

/// \name Computer player
//@{
Move selectMove(Card::Cards& hand, const Table& table);
//@}

} // namespace MachiavelliRules

#endif
