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

// Simulation of complete Machiavelli games played by four computer players,
// checking the invariants of the game after every move

#define BOOST_TEST_MODULE MachiavelliSimulation
#include <boost/test/unit_test.hpp>

#include "MachiavelliRules.h"

#include "TestUtil.h"

using namespace MachiavelliRules;

namespace {

constexpr unsigned int MAX_TURNS = 1000;         ///< Turns after which a game is considered as endless
constexpr unsigned int MAX_MOVES_PER_TURN = 200; ///< Moves of one player after which a turn is considered as endless

/// Statistics over all games
struct Statistics {
    unsigned int games = 0;
    unsigned int lost = 0;      ///< Games ending with a loser (the only player with cards)
    unsigned int stalled = 0;   ///< Games ending, because the staple is empty (the GUI stops then)
    unsigned long moves = 0;    ///< Moves of all players
    unsigned long reorders = 0; ///< Moves only re-ordering the table
    unsigned int maxTurns = 0;
    unsigned int maxMovesPerTurn = 0;
};

/// State of a game
struct Game {
    std::array<Card::Cards, NUM_PLAYERS> hands;
    Table table;
    Card::Cards staple;

    std::array<unsigned int, NUM_PLAYERS> handSizes() const {
        std::array<unsigned int, NUM_PLAYERS> sizes{};
        for (unsigned int i(0); i < NUM_PLAYERS; ++i)
            sizes[i] = hands[i].size();
        return sizes;
    }

    /// Returns all cards of the game
    Card::Cards allCards() const {
        Card::Cards all(staple);
        for (const auto& hand : hands)
            all.insert(all.end(), hand.begin(), hand.end());
        for (const auto& pile : table)
            all.insert(all.end(), pile.cards.begin(), pile.cards.end());
        return all;
    }

    /// Deals the top card of the staple to the passed player (like
    /// Machiavelli::dealCard does for computer players)
    /// \returns bool False, if the staple is empty
    bool deal(unsigned int player) {
        if (staple.empty())
            return false;
        Card::Cards& hand(hands[player]);
        hand.insert(hand.begin() + dealPosition(hand, staple.back()), staple.back());
        staple.pop_back();
        return true;
    }
};

/// Prints the table
std::string toString(const Table& table) {
    std::ostringstream out;
    for (const auto& pile : table)
        out << '[' << pile.cards << "] ";
    return out.str();
}

/// Checks the passed move of the computer player and executes it
void checkAndApply(Game& game, unsigned int player, const Move& move, Statistics& stats) {
    Card::Cards& hand(game.hands[player]);
    BOOST_TEST_REQUIRE(move.dest <= game.table.size(), "Invalid destination " << move.dest);

    // Check the positions of the transfers (the destination growing with every transfer)
    unsigned int destSize((move.dest < game.table.size()) ? game.table[move.dest].size() : 0);
    std::vector<unsigned int> sizes;
    for (const auto& pile : game.table)
        sizes.push_back(pile.size());
    unsigned int handSize(hand.size());
    for (const auto& transfer : move.transfers) {
        BOOST_TEST_REQUIRE(transfer.first <= transfer.last);
        BOOST_TEST_REQUIRE(transfer.pile != move.dest);
        unsigned int& srcSize((transfer.pile == HAND) ? handSize : sizes.at(transfer.pile));
        BOOST_TEST_REQUIRE(transfer.last < srcSize, "Transfer from " << static_cast<int>(transfer.pile) << " out of range");
        BOOST_TEST_REQUIRE(transfer.destPos <= destSize, "Transfer to position " << transfer.destPos << " out of range");
        srcSize -= transfer.number();
        destSize += transfer.number();
    }

    const unsigned int fromHand(move.cardsFromHand());
    const unsigned int sizeBefore(hand.size());
    applyMove(hand, game.table, move);
    ++stats.moves;
    if (!fromHand)
        ++stats.reorders;

    // The move must result in valid piles (like the GUI checks after every
    // move of the computer player)
    BOOST_TEST_REQUIRE(firstInvalidPile(game.table) == -1, "Invalid piles after move: " << toString(game.table));
    BOOST_TEST_REQUIRE(hand.size() == (sizeBefore - fromHand));
}

/// Plays a complete game
void playGame(unsigned int seed, Statistics& stats) {
    const Card::Cards deck(Card::createDeck(NUM_DECKS));
    Game game;
    game.staple = Test::shuffledDeck(seed, NUM_DECKS);
    unsigned int player(Card::randomNumber(NUM_PLAYERS)); // Like Machiavelli::start

    // Deal (like Machiavelli::start: from the top of the staple; the hands of
    // the computer players are sorted by number)
    for (auto& hand : game.hands) {
        hand.assign(game.staple.end() - CARDS_PER_PLAYER, game.staple.end());
        game.staple.erase(game.staple.end() - CARDS_PER_PLAYER, game.staple.end());
        std::ranges::sort(hand, Card::lessByNumber);
    }
    BOOST_TEST_REQUIRE(Test::sameCards(game.allCards(), deck));
    BOOST_TEST_REQUIRE(game.deal(player));

    ++stats.games;
    for (unsigned int turn(0);; ++turn) {
        BOOST_TEST_REQUIRE(turn < MAX_TURNS, "Game doesn't end");
        stats.maxTurns = std::max(stats.maxTurns, turn);

        BOOST_TEST_CONTEXT("Turn " << turn << ", player " << player) {
            for (unsigned int moves(0);; ++moves) {
                BOOST_TEST_REQUIRE(moves < MAX_MOVES_PER_TURN, "Turn doesn't end");
                stats.maxMovesPerTurn = std::max(stats.maxMovesPerTurn, moves);

                Card::Cards& hand(game.hands[player]);
                const Card::Cards before(hand);
                const std::string tableBefore(toString(game.table));
                BOOST_TEST_CONTEXT("Hand " << before << ", table " << tableBefore) {
                    const Move move(selectMove(hand, game.table));
                    BOOST_TEST_REQUIRE(Test::sameCards(hand, before), "Computer player changed his cards");
                    if (!move.empty())
                        checkAndApply(game, player, move, stats);

                    // The hand stays sorted by number (the played cards are sorted
                    // to their end before playing them)
                    BOOST_TEST_REQUIRE(std::ranges::is_sorted(hand, Card::lessByNumber), "Hand not sorted: " << hand);
                    BOOST_TEST_REQUIRE(Test::sameCards(game.allCards(), deck), "Cards lost or duplicated");
                    if (move.empty())
                        break;
                }
            }
        }

        // Next player (like Machiavelli::makeMove)
        const unsigned int next(findNextPlayer(game.handSizes(), player));
        if (isGameOver(game.handSizes(), next)) {
            BOOST_TEST_REQUIRE(game.hands[next].size());
            for (unsigned int i(0); i < NUM_PLAYERS; ++i)
                if (i != next)
                    BOOST_TEST_REQUIRE(game.hands[i].empty());
            ++stats.lost;
            break;
        }
        player = next;

        // The GUI doesn't continue, if there's no card to deal
        if (!game.deal(player)) {
            ++stats.stalled;
            break;
        }
    }
    BOOST_TEST_REQUIRE(Test::sameCards(game.allCards(), deck));
}

} // namespace

BOOST_AUTO_TEST_CASE(computer_players_play_complete_games) {
    const Test::Seeds seeds;
    Statistics stats;
    for (unsigned int seed(seeds.first); seed < seeds.end(); ++seed) {
        BOOST_TEST_CONTEXT("Seed " << seed) { playGame(seed, stats); }
    }

    BOOST_TEST(stats.games == stats.lost + stats.stalled);
    BOOST_TEST_MESSAGE("Games: " << stats.games << " (lost: " << stats.lost << ", stalled: " << stats.stalled
                                 << "); moves: " << stats.moves << " (re-ordering only: " << stats.reorders
                                 << "); max. turns: " << stats.maxTurns << "; max. moves per turn: " << stats.maxMovesPerTurn);
}
