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

// Simulation of complete Rovhult games played by four computer players,
// checking the invariants of the game after every move. As in the GUI player
// 0 counts as human (whose moves are made with the strategy of the computer
// player): The computer players start to count their moves (to prevent
// endless games) only after he finished.
//
// As the strategy of the computer player is deterministic, two players can
// repeat the same moves endlessly. In the GUI the human has to break such a
// cycle; here the human "gets impatient" after HUMAN_PATIENCE moves: The
// counting is started then and the human plays like the computer players.
//
// Even then the computer players might not end the game (their random moves
// don't break every cycle, see RovhultRules::playRandomly); such games are
// reported as warning (and must be rare).

#define BOOST_TEST_MODULE RovhultSimulation
#include <boost/test/unit_test.hpp>

#include <algorithm>

#include "RovhultRules.h"

#include "TestUtil.h"

using namespace RovhultRules;
using Card::Value;

namespace {

constexpr unsigned int HUMAN = 0;            ///< Player counting as human
constexpr unsigned int MAX_MOVES = 10000;    ///< Moves after which the game is considered as endless
constexpr unsigned int MAX_ENDGAME = 2000;   ///< Counted moves after which the game is considered as endless
constexpr unsigned int HUMAN_PATIENCE = 500; ///< Moves after which the human starts to break cycles

/// State of a simulated game
struct Game {
    Table table;
    Card::Cards staple;  ///< Cards not dealt yet (top card last)
    Card::Cards removed; ///< Cards removed from the game (by clearing the played cards)
    Options options;

    /// Returns all cards of the game
    Card::Cards allCards() const {
        Card::Cards all(staple);
        all.insert(all.end(), removed.begin(), removed.end());
        all.insert(all.end(), table.played.begin(), table.played.end());
        for (const auto& player : table.players) {
            all.insert(all.end(), player.hand.begin(), player.hand.end());
            for (const auto& pile : player.reserve)
                all.insert(all.end(), pile.begin(), pile.end());
        }
        return all;
    }
};

/// Statistics of the simulation
struct Statistics {
    unsigned long moves{0};
    unsigned int maxMoves{0};
    unsigned int impatient{0};
    unsigned int endless{0};
};

/// Returns the special cards to use for the passed seed: Every fourth game
/// uses random (but distinct) special cards
Options optionsFor(unsigned int seed) {
    Options options;
    if (!(seed % 4)) {
        std::array<Value::NUMBERS, 3> numbers{};
        for (unsigned int i(0); i < numbers.size(); ++i)
            do
                numbers[i] = static_cast<Value::NUMBERS>(Value::THREE + Card::randomNumber(Value::ACE - Value::THREE + 1));
            while (std::find(numbers.begin(), numbers.begin() + i, numbers[i]) != (numbers.begin() + i));
        options.nuke = numbers[0];
        options.skip = numbers[1];
        options.reverse = numbers[2];
    }
    return options;
}

/// Checks the invariants valid after every move
void checkInvariants(const Game& game) {
    BOOST_TEST_REQUIRE(Test::sameCards(game.allCards(), Card::createDeck()), "Cards lost or duplicated");
    BOOST_TEST_REQUIRE(numberOfEqualTopCards(game.table.played) < 4, "4 equal cards not removed");
    for (const auto& player : game.table.players) {
        BOOST_TEST_REQUIRE(std::ranges::is_sorted(player.hand, Card::lessByNumber), "Hand not sorted: " << player.hand);
        for (const auto& pile : player.reserve)
            BOOST_TEST_REQUIRE(pile.size() <= 2u);
        if (game.staple.size())
            BOOST_TEST_REQUIRE(player.hand.size(), "Hand not filled up");
    }
}

/// Moves the played cards into the hand of the player
void takePlayedCards(Game& game, unsigned int player) {
    BOOST_TEST_REQUIRE(game.table.played.size());
    for (const auto& card : game.table.played)
        insertSorted(game.table.players[player].hand, card);
    game.table.played.clear();
}

/// Deals the cards and lets the players exchange their cards
void dealAndExchange(Game& game, unsigned int first) {
    game.table.players = deal(game.staple, first);
    checkInvariants(game);
    for (auto& player : game.table.players) {
        BOOST_TEST_REQUIRE(player.hand.size() == CARDS_IN_HAND);
        for (unsigned int i(0); i < NUM_RESERVE; ++i) {
            BOOST_TEST_REQUIRE(player.reserve[i].size() == 2u);
            BOOST_TEST_REQUIRE(player.topVisible(i));
        }

        const Player before(player);
        exchangeCards(player, game.options);
        BOOST_TEST_REQUIRE(player.hand.size() == CARDS_IN_HAND);
        for (unsigned int i(0); i < NUM_RESERVE; ++i) {
            BOOST_TEST_REQUIRE(player.reserve[i].size() == 2u);
            BOOST_TEST_REQUIRE((player.reserve[i][0] == before.reserve[i][0]), "Hidden card exchanged");
        }
        for (unsigned int i(1); i < NUM_RESERVE; ++i)
            BOOST_TEST_REQUIRE(compareCards(player.reserve[i - 1].back(), player.reserve[i].back(), game.options) <= 0);
    }
    checkInvariants(game);
}

/// Plays a complete game
/// \param seed Seed of the game
/// \param stats Statistics to update
void playGame(unsigned int seed, Statistics& stats) {
    Game game;
    game.staple = Test::shuffledDeck(seed);
    game.options = optionsFor(seed);
    BOOST_TEST_CONTEXT("Nuke " << game.options.nuke << ", skip " << game.options.skip << ", reverse " << game.options.reverse) {
        dealAndExchange(game, seed % NUM_PLAYERS);

        unsigned int current(Card::randomNumber(NUM_PLAYERS));
        unsigned int cEndgame(0);
        bool impatient(false);
        for (unsigned int moves(0);; ++moves) {
            Player& player(game.table.players[current]);
            BOOST_TEST_CONTEXT("Move " << moves << ", player " << current << ", hand " << Card::Cards(player.hand) << ", reserve "
                                       << Card::Cards(player.reserve[0]) << " / " << Card::Cards(player.reserve[1]) << " / "
                                       << Card::Cards(player.reserve[2]) << ", played " << Card::Cards(game.table.played)) {
                BOOST_TEST_REQUIRE(moves < MAX_MOVES, "Game doesn't end");
                BOOST_TEST_REQUIRE(player.hasCards());
                if (cEndgame >= MAX_ENDGAME) {
                    BOOST_TEST_WARN(false, "Endgame doesn't end");
                    ++stats.endless;
                    return;
                }

                if ((moves == HUMAN_PATIENCE) && !cEndgame) {
                    cEndgame = 1;
                    impatient = true;
                    ++stats.impatient;
                }

                const Move move((((current != HUMAN) || impatient) && playRandomly(cEndgame, player))
                                    ? selectRandomCard(player.hand, game.table.played, game.options)
                                    : selectMove(game.table, current, game.options));
                const PlayError error(checkMove(game.table, current, move, game.options));
                BOOST_TEST_REQUIRE(static_cast<int>(error) == static_cast<int>(PlayError::NONE),
                                   "Invalid move " << move.source << ' ' << move.start << '-' << move.end << ": "
                                                   << describe(error));

                bool take(move.source == Move::TAKE);
                switch (move.source) {
                case Move::HAND:
                    game.table.played.insert(game.table.played.end(), player.hand.begin() + move.start,
                                             player.hand.begin() + move.end + 1);
                    player.hand.erase(player.hand.begin() + move.start, player.hand.begin() + move.end + 1);
                    break;

                case Move::RESERVE:
                    if (!player.topVisible(move.start) && (checkCard(player.reserve[move.start].back().number(),
                                                                     game.table.played, game.options) != PlayError::NONE)) {
                        // Hidden card can't be played: Take it and the played cards
                        insertSorted(player.hand, player.reserve[move.start].back());
                        player.reserve[move.start].pop_back();
                        take = true;
                    }
                    else
                        for (unsigned int i(move.start); i <= move.end; ++i) {
                            game.table.played.push_back(player.reserve[i].back());
                            player.reserve[i].pop_back();
                        }
                    break;

                case Move::TAKE:
                    break;
                }

                if (take) {
                    takePlayedCards(game, current);
                    const int next(nextAvailablePlayer(game.table, current));
                    BOOST_TEST_REQUIRE(next != -1);
                    current = next;
                    checkInvariants(game);
                    continue;
                }

                fillUp(player.hand, game.staple,
                       handSizeAfterMove(game.table.played.back().number(), player.hand.size(), game.options));

                const Turn turn(nextTurn(game.table, current, game.options));
                BOOST_TEST_REQUIRE(turn.finished == !player.hasCards());
                if (turn.clearPlayed) {
                    game.removed.insert(game.removed.end(), game.table.played.begin(), game.table.played.end());
                    game.table.played.clear();
                }
                checkInvariants(game);

                // Start counting the moves, when the human finished
                if (turn.finished && !cEndgame && (current == HUMAN))
                    cEndgame = 1;

                if (turn.loser != -1) {
                    for (unsigned int i(0); i < NUM_PLAYERS; ++i)
                        BOOST_TEST_REQUIRE(game.table.players[i].hasCards() == (static_cast<int>(i) == turn.loser),
                                           "Player " << i << " has cards at the end");
                    BOOST_TEST_REQUIRE(game.staple.empty());
                    stats.moves += moves + 1;
                    stats.maxMoves = std::max(stats.maxMoves, moves + 1);
                    return;
                }

                BOOST_TEST_REQUIRE(turn.next < NUM_PLAYERS);
                BOOST_TEST_REQUIRE(game.table.players[turn.next].hasCards());
                if (turn.skipped != -1) {
                    BOOST_TEST_REQUIRE(turn.skipped != static_cast<int>(turn.next));
                    BOOST_TEST_REQUIRE(game.table.players[turn.skipped].hasCards());
                }
                else if (turn.clearPlayed && !turn.finished)
                    BOOST_TEST_REQUIRE(turn.next == current, "Player doesn't continue after clearing");
                current = turn.next;
            }
        }
    }
}

} // namespace

BOOST_AUTO_TEST_CASE(computer_players_play_complete_games) {
    const Test::Seeds seeds;
    Statistics stats;
    for (unsigned int seed(seeds.first); seed < seeds.end(); ++seed)
        BOOST_TEST_CONTEXT("Seed " << seed) { playGame(seed, stats); }
    BOOST_TEST_MESSAGE("Played " << seeds.count << " games with " << stats.moves << " moves (max. " << stats.maxMoves
                                 << "); human got impatient in " << stats.impatient << " games; " << stats.endless
                                 << " endless games");
    BOOST_TEST(stats.endless * 100 <= seeds.count, stats.endless << " endless games");
}
