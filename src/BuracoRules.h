#ifndef BURACORULES_H
#define BURACORULES_H

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
#include <vector>

#include <card/Value.h>

/**Rules and computer player of the Buraco cardgame, independent of its
   display.

   The game is played with 4 decks (each with 3 jokers); so cards with the
   same value (and ID) exist several times. Cards are therefore identified by
   their position (in a hand or in a pile on the table).

   2s and jokers are both "jokers" (also called monos).

   The positions inside a pile (see PileInfo) are stored like the original
   bitfields: A value bigger than 6 (NONE) means "no such card".
 */
namespace BuracoRules {

static constexpr unsigned int NUM_PLAYERS = 4;
static constexpr unsigned int NUM_TEAMS = NUM_PLAYERS >> 1;
/// Number of decks the game is played with
static constexpr unsigned int NUM_DECKS = 4;
/// Number of jokers (per deck)
static constexpr unsigned int NUM_JOKERS = 3;
/// Number of cards of a finished pile (a cerrado)
static constexpr unsigned int CERRADO = 7;
/// Position in a pile meaning "no such card"
static constexpr unsigned int NONE = 7;
/// Bonus for the team ending the game
static constexpr int GOING_OUT_BONUS = 100;
/// Bonus/penalty for a team having taken (or not) its reserve
static constexpr int RESERVE_BONUS = 100;

/// \name Cards
//@{
bool isJoker(const Card::Value& card);
unsigned int pointsOf(const Card::Value& card);
unsigned int pointsOf(const Card::Cards& cards);
int cardDistance(const Card::Value& a, const Card::Value& b, bool aceIsOne = true);
bool lessByNumberWithJokers(const Card::Value& a, const Card::Value& b);
bool lessByColourWithJokers(const Card::Value& a, const Card::Value& b);
bool containsOnlyJoker(const Card::Cards& cards);
bool containsNoJoker(const Card::Cards& cards);

unsigned int findFittingCard(const Card::Cards& cards, const Card::Value& card, unsigned int start = 0);
bool hasFittingPair(const Card::Cards& cards, const Card::Value& card, unsigned int skip = -1U);
bool pileHasFittingPair(const Card::Cards& cards, const Card::Value& card, unsigned int pos, bool withJokers = false);
bool pileHasFittingPair(const Card::Cards& cards, unsigned int exclude = -1U);

unsigned int insertSorted(Card::Cards& cards, const Card::Value& card);
int findSorted(const Card::Cards& cards, const Card::Value& card);
void moveCard(Card::Cards& cards, unsigned int dest, unsigned int source, unsigned int* tracked = nullptr);

/// Cards in the hand forming a series with a card (see getSeries)
struct Series {
    std::map<unsigned int, unsigned int> positions; ///< Positions of the cards of the same colour (key: distance + 2)
    std::vector<unsigned int> order;                ///< Order in which the cards of the same colour have been found
    unsigned int equal;                             ///< Number of cards with the same number (incl. the card itself)
};
Series getSeries(const Card::Cards& cards, unsigned int pos);
unsigned int sortColourSerie(Card::Cards& cards, const Series& series, unsigned int* tracked = nullptr);
//@}

/// \name Piles on the table
//@{
enum PileType { UNDEFINED, NUMBER, COLOUR };

/// Characteristics of a pile on the table
struct PileInfo {
    unsigned int posFirst{NONE}; ///< Position of the first card, which is no joker
    unsigned int posLast{NONE};  ///< Position of the last card, which is no joker
    unsigned int posJoker{NONE}; ///< Position of the joker
    PileType type{UNDEFINED};    ///< Type of the pile
    unsigned int points{0};      ///< Points the pile gets, when it is finished
};
PileInfo analysePile(const Card::Cards& pile);
bool getPosition4Card(const Card::Cards& pile, const Card::Value& card, unsigned int& pos, unsigned int& move);
int pilePoints(const PileInfo& info, unsigned int size);
int pilePoints(const Card::Cards& pile);
bool isValidPile(const Card::Cards& pile);
bool startsMonoPile(const Card::Cards& pile, const Card::Value& card);
//@}

/// Information about the actual game, needed to check moves and to decide which cards to play
struct Table {
    std::array<Card::Cards, NUM_PLAYERS> hands;                ///< Cards in the hands of the players
    std::array<std::vector<Card::Cards>, NUM_TEAMS> piles;     ///< Piles on the table (incl. the finished ones)
    std::array<bool, NUM_TEAMS> reserve{true, true};           ///< Flags, if the teams still have their reserve
    std::array<int, NUM_TEAMS> points{};                       ///< Points of the finished piles (cerrados)
    std::array<unsigned int, NUM_TEAMS> unfinishedMonoPiles{}; ///< Number of started piles of monos
    std::array<unsigned int, NUM_TEAMS> buraco{0x3, 0x3};      ///< Partner (player >> 1), who took the reserve (0x3: none)
    unsigned int dumped{0};                                    ///< Number of dumped cards
    bool pickUpPlayed{false}; ///< Flag, if the dumped cards have been taken (and not yet received)
    bool startGame{false};    ///< Flag, if this is the first move of the game
};

bool canGetRidOfCards(const Card::Cards& hand);
bool canPlayCards(const Table& table, unsigned int player, unsigned int cards, unsigned int pile = -1U);
bool canClosePile(const Table& table, unsigned int player, unsigned int pile);
int cardFitsOnPile(const Table& table, unsigned int player, unsigned int pile, unsigned int card);
bool pilesComplete(const std::vector<Card::Cards>& piles, unsigned int except = -1U);

/// Reasons why a move must not be done
enum class PlayError {
    NONE,
    PILES_INCOMPLETE,
    PICK_UP_MONO,
    PICK_UP_NO_PAIR,
    PICK_UP_WOULD_END,
    FILL_OTHER_PILES,
    NO_VALID_NEW_PILE,
    MONO_PILE_UNFINISHED,
    NO_CERRADO,
    NOT_ENOUGH_CARDS,
    NO_JOKER_ON_NEW_PILE,
    CARD_DOES_NOT_FIT
};

/// Returns the (untranslated) message describing the passed error
const char* describe(PlayError error);

PlayError checkPickUp(const Table& table, unsigned int player, const Card::Value& top);
PlayError checkNewPile(const Table& table, unsigned int player, unsigned int card);
PlayError checkAddToPile(const Table& table, unsigned int player, unsigned int card, unsigned int pile);
PlayError checkDump(const Table& table, unsigned int player);

/// \name Game flow and scoring
//@{
/// Cards of a round, as dealt
struct Deal {
    std::array<Card::Cards, NUM_PLAYERS> hands; ///< Cards of the players (sorted)
    std::array<Card::Cards, NUM_TEAMS> reserve; ///< Reserve of the teams
    Card::Value dumped;                         ///< First dumped card
    Card::Cards staple;                         ///< Remaining cards (the top card is the last one)
};
unsigned int cardsInHand(unsigned int cardsToDeal);
Deal deal(Card::Cards staple, unsigned int cardsToDeal);
unsigned int startPlayer();

/// What happens, if a player has no more cards (except of jokers)
enum class HandStatus { PLAYING, TAKE_RESERVE, GOING_OUT };
HandStatus handStatus(const Card::Cards& hand, bool hasReserve);
void takeReserve(Card::Cards& hand, Card::Cards reserve);

/// Score of a round
struct RoundScore {
    std::array<int, NUM_TEAMS> bonus; ///< Points of the finished piles and bonus for (not) having taken the reserve
    std::array<int, NUM_TEAMS> cards; ///< Points of the cards on the table, minus the ones in the hands
};
RoundScore roundScore(const Table& table);
//@}

/// \name Computer player
//@{
/// Move of a joker within a pile on the table (like IPile::move(to, from))
struct JokerMove {
    unsigned int pile; ///< Pile of the team of the player
    unsigned int from; ///< Actual position of the joker
    unsigned int to;   ///< New position of the joker
};

/// Move the computer player wants to make
struct Move {
    enum Kind { DUMP, ADD_TO_PILE, NEW_PILE };
    Kind kind{DUMP};
    unsigned int pile{0};              ///< Pile of the team to play to (NEW_PILE: Index of the new pile)
    unsigned int pos{0};               ///< Position in the pile to insert the cards
    unsigned int first{0};             ///< Position of first card to play/dump in the hand
    unsigned int last{0};              ///< Position of last card to play/dump in the hand
    std::vector<JokerMove> jokerMoves; ///< Moves of jokers in the piles of the team (to perform first)
};

/// Cards the computer player plays together with the taken dumped card
struct PickUp {
    unsigned int first;    ///< Position of first card to play in the hand
    unsigned int last;     ///< Position of last card to play in the hand
    unsigned int posTaken; ///< Position of the taken card within the new pile
};

bool takeDumped(const Table& table, unsigned int player, const Card::Value& top);
PickUp playPickedUp(Table& table, unsigned int player, const Card::Value& top);
unsigned int cardFitsOnPlayedPile(Table& table, unsigned int player, unsigned int card, std::vector<JokerMove>& moves);
Move selectMove(Table& table, unsigned int player);
//@}

} // namespace BuracoRules

#endif
