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

// Simulation of complete Twopart games played by four computer players,
// checking the invariants of the game after every move. The flow mirrors the
// one of the class Twopart.

#define BOOST_TEST_MODULE TwopartSimulation
#include <boost/test/unit_test.hpp>

#include <algorithm>
#include <set>
#include <sstream>
#include <string>
#include <vector>

#include "TwopartRules.h"

#include "TestUtil.h"

using namespace TwopartRules;
using Card::Value;

namespace {

constexpr unsigned int MAX_MOVES = 5000;
/// Number of moves, after which the states are recorded to detect endless games
constexpr unsigned int RECORD_STATES = 500;

/// Statistics over all games
struct Statistics {
    unsigned int games = 0;
    unsigned int moves = 0;
    unsigned int pickUps = 0;
    unsigned int ties = 0;
    unsigned int playersWithoutCards = 0; ///< Players starting part 2 without cards
    std::vector<unsigned int> endless;    ///< Seeds of games, where the computer players repeat their moves forever
    std::array<unsigned int, NUM_PLAYERS> lost{};
};

/// State of a game
struct Game {
    Card::Cards stock;
    std::array<Card::Cards, NUM_PLAYERS> hands;
    std::array<Card::Cards, NUM_PLAYERS> won;
    Card::Cards played;
    Card::Cards removed; ///< Cards out of the game (played in part 2)
    Table table;

    HandSizes handSizes() const {
        HandSizes sizes{};
        for (unsigned int i(0); i < NUM_PLAYERS; ++i)
            sizes[i] = hands[i].size();
        return sizes;
    }

    /// Returns a description of the state of the game (when the passed player is about to move)
    std::string state(unsigned int player) const {
        std::ostringstream out;
        out << player << ':' << table.bfPlayers << ':' << table.offPos << ':';
        for (unsigned int i(0); i < table.offPos; ++i)
            out << table.startPos[i] << ',';
        for (const auto& hand : hands)
            out << '|' << hand;
        out << '|' << played;
        return out.str();
    }

    /// Checks that no card has been lost or duplicated
    void checkCards() const {
        Card::Cards all(stock);
        for (unsigned int i(0); i < NUM_PLAYERS; ++i) {
            all.insert(all.end(), hands[i].begin(), hands[i].end());
            all.insert(all.end(), won[i].begin(), won[i].end());
        }
        all.insert(all.end(), played.begin(), played.end());
        all.insert(all.end(), removed.begin(), removed.end());
        BOOST_TEST_REQUIRE(Test::sameCards(all, Card::createDeck()));
    }

    /// Checks the hands are sorted as the GUI sorts them
    void checkSorted() const {
        for (const auto& hand : hands)
            if (table.partTwo)
                BOOST_TEST_REQUIRE(std::ranges::is_sorted(
                    hand, [this](const Value& a, const Value& b) { return lessByColourAccTrumps(a, b, *table.trump); }));
            else
                BOOST_TEST_REQUIRE(std::ranges::is_sorted(hand, Card::lessByNumber));
    }

    /// Checks that the players in the round have cards
    void checkPlayersInRound() const {
        for (unsigned int i(0); i < NUM_PLAYERS; ++i)
            if (table.isInRound(i))
                BOOST_TEST_REQUIRE(!hands[i].empty(), "Player " << i << " in round without cards");
    }
};

/// Inserts the card into the hand (sorted by number) like IPile::insertSorted
void insertSorted(Card::Cards& hand, const Value& card) {
    hand.insert(std::ranges::upper_bound(hand, card, Card::lessByNumber), card);
}

/// Plays a game
/// \param seed Seed of the game
/// \param stats Statistics to update
void playGame(unsigned int seed, Statistics& stats) {
    Game game;
    game.stock = Test::shuffledDeck(seed);
    for (unsigned int i(0); i < NUM_PLAYERS; ++i)
        for (unsigned int j(0); j < CARDS_IN_HAND; ++j) {
            insertSorted(game.hands[i], game.stock.back());
            game.stock.pop_back();
        }
    game.checkCards();

    Table& table(game.table);
    table.reset();
    unsigned int next(table.startPlayer = Card::randomNumber(NUM_PLAYERS));
    bool gameOver(false);
    std::set<std::string> states;

    for (unsigned int move(0); !gameOver; ++move) {
        BOOST_TEST_REQUIRE(move < MAX_MOVES, "Game doesn't end");
        ++stats.moves;
        const unsigned int player(next);
        BOOST_TEST_REQUIRE(player < NUM_PLAYERS);

        // Long games are checked for repetitions: The computer players (in
        // part 2) might pick up and play the same cards again and again
        if (move >= RECORD_STATES) {
            BOOST_TEST_REQUIRE(table.partTwo);
            if (!states.insert(game.state(player)).second) {
                stats.endless.push_back(seed);
                return;
            }
        }
        BOOST_TEST_REQUIRE(table.isInRound(player));
        Card::Cards& hand(game.hands[player]);

        BOOST_TEST_CONTEXT("Move " << move << ", part " << (table.partTwo ? 2 : 1) << ", player " << player << ", hand "
                                   << Card::Cards(hand) << ", played " << Card::Cards(game.played) << ", round start "
                                   << table.startPos[0] << ", players " << table.bfPlayers) {
            BOOST_TEST_REQUIRE(!hand.empty(), "Player has no cards");
            const bool partTwo(table.partTwo);
            const auto play(selectCardsToPlay(player, hand, game.played, game.won[player].size(), game.stock.size(), table));
            if (play) {
                BOOST_TEST_REQUIRE(play->start <= play->end);
                BOOST_TEST_REQUIRE(play->end < hand.size());
                const PlayError error(checkPlay(hand, play->start, play->end, game.played, table));
                BOOST_TEST_REQUIRE(static_cast<int>(error) == static_cast<int>(PlayError::NONE),
                                   "Computer plays " << Card::Cards(hand.begin() + play->start, hand.begin() + play->end + 1)
                                                     << ": " << describe(error));
                if (table.partTwo)
                    table.registerPlay(game.played.size());
                game.played.insert(game.played.end(), hand.begin() + play->start, hand.begin() + play->end + 1);
                hand.erase(hand.begin() + play->start, hand.begin() + play->end + 1);

                // Draw a card; the last one defines the trump
                if (game.stock.size()) {
                    const Value card(game.stock.back());
                    game.stock.pop_back();
                    insertSorted(hand, card);
                    if (game.stock.empty())
                        table.trump = card.colour();
                }

                const TurnResult result(table.endTurn(player, game.played, game.handSizes()));
                next = result.next;
                if (result.endOfRound) {
                    if (table.partTwo) {
                        game.removed.insert(game.removed.end(), game.played.begin(), game.played.end());
                        game.played.clear();
                    }
                    else if (result.winner >= 0) {
                        // Move the played cards to the winner (Twopart::endPickup)
                        BOOST_TEST_REQUIRE(static_cast<unsigned int>(result.winner) < NUM_PLAYERS);
                        Card::Cards& won(game.won[result.winner]);
                        won.insert(won.end(), game.played.begin(), game.played.end());
                        game.played.clear();
                        table.playedCardsWon();

                        if (table.nextPlayer(result.winner) < 0) {
                            BOOST_TEST_REQUIRE(result.endOfPart);
                            BOOST_TEST_REQUIRE(result.next == static_cast<unsigned int>(result.winner));
                            for (const auto& h : game.hands)
                                BOOST_TEST_REQUIRE(h.empty());
                            BOOST_TEST_REQUIRE(game.stock.empty());
                            BOOST_TEST_REQUIRE(table.trump.has_value());

                            // Start part 2 (Twopart::startPartTwo)
                            next = result.winner;
                            const Receivers receivers(table.startPartTwo(game.won, next));
                            for (unsigned int i(0); i < NUM_PLAYERS; ++i) {
                                BOOST_TEST_REQUIRE(receivers[i].size() == game.won[i].size());
                                for (unsigned int receiver : receivers[i]) {
                                    BOOST_TEST_REQUIRE(receiver < NUM_PLAYERS);
                                    game.hands[receiver].push_back(game.won[i].back());
                                    game.won[i].pop_back();
                                }
                            }
                            for (auto& h : game.hands) {
                                sortByColourAccTrumps(h, *table.trump);
                                if (h.empty())
                                    ++stats.playersWithoutCards;
                            }
                        }
                        else
                            BOOST_TEST_REQUIRE(!result.endOfPart);
                    }
                    else
                        ++stats.ties;
                }
                else
                    BOOST_TEST_REQUIRE(result.winner == -1);

                if (result.endOfPart && partTwo) {
                    // Game over: Only the loser has cards left
                    gameOver = true;
                    BOOST_TEST_REQUIRE(next < NUM_PLAYERS);
                    for (unsigned int i(0); i < NUM_PLAYERS; ++i)
                        BOOST_TEST_REQUIRE(game.hands[i].empty() == (i != next), "Player " << i << " still has cards");
                    ++stats.lost[next];
                }
            }
            else {
                BOOST_TEST_REQUIRE(table.partTwo, "Picking up in part 1");
                BOOST_TEST_REQUIRE(game.played.size());
                ++stats.pickUps;
                const PickUp pickUp(table.pickUp(player, game.handSizes()));
                BOOST_TEST_REQUIRE(pickUp.start < game.played.size());
                hand.insert(hand.end(), game.played.begin() + pickUp.start, game.played.end());
                game.played.erase(game.played.begin() + pickUp.start, game.played.end());
                sortByColourAccTrumps(hand, *table.trump);
                BOOST_TEST_REQUIRE(pickUp.next < NUM_PLAYERS);
                next = pickUp.next;
            }

            game.checkCards();
            game.checkSorted();
            if (!gameOver) {
                game.checkPlayersInRound();
                BOOST_TEST_REQUIRE(table.offPos <= NUM_PLAYERS);
                if (table.partTwo) {
                    // The plays on the table are in ascending order
                    for (unsigned int i(1); i < table.offPos; ++i)
                        BOOST_TEST_REQUIRE(table.startPos[i - 1] < table.startPos[i]);
                    BOOST_TEST_REQUIRE(
                        (table.offPos ? (table.startPos[table.offPos - 1] < game.played.size()) : game.played.empty()));
                }
                else
                    BOOST_TEST_REQUIRE(table.startPos[0] <= game.played.size());
            }
        }
    }
    ++stats.games;
}

} // namespace

BOOST_AUTO_TEST_CASE(computer_players_play_complete_games) {
    const Test::Seeds seeds;
    Statistics stats;
    for (unsigned int seed(seeds.first); seed < seeds.end(); ++seed) {
        BOOST_TEST_CONTEXT("Seed " << seed) { playGame(seed, stats); }
    }
    BOOST_TEST_MESSAGE("Games: " << stats.games << "; moves: " << stats.moves << "; pick-ups: " << stats.pickUps
                                 << "; ties: " << stats.ties << "; players without cards in part 2: " << stats.playersWithoutCards
                                 << "; lost: " << stats.lost[0] << "/" << stats.lost[1] << "/" << stats.lost[2] << "/"
                                 << stats.lost[3]);
    BOOST_TEST(stats.games + stats.endless.size() == seeds.count);

    // Known issue: The computer players can repeat their moves forever in part 2
    // (picking up the cards played last, instead of using their few trumps)
    if (stats.endless.size()) {
        std::ostringstream seedList;
        for (auto seed : stats.endless)
            seedList << ' ' << seed;
        BOOST_TEST_WARN(stats.endless.empty(), stats.endless.size() << " endless game(s); seeds:" << seedList.str());
    }
}
