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

// Simulation of complete Hearts games played by four computer players,
// checking the invariants of the game after every move

#define BOOST_TEST_MODULE HeartsSimulation
#include <boost/test/unit_test.hpp>

#include <numeric>

#include "HeartsRules.h"

#include "TestUtil.h"

using namespace HeartsRules;

namespace {

constexpr int END_POINTS = 100;     ///< Points ending the game (see Hearts::ENDPOINTS)
constexpr unsigned int MAX_ROUNDS = 1000;

/// Plays a round (from dealing to scoring)
/// \param seed Seed of the round
/// \param exchange Offset of the player to give the cards to (0: No exchange)
/// \returns std::array<int, NUM_PLAYERS> Score of the round
std::array<int, NUM_PLAYERS> playRound(unsigned int seed, unsigned int exchange) {
    const Card::Cards deck(Test::shuffledDeck(seed));
    const unsigned int cardsPerPlayer(deck.size() / NUM_PLAYERS);

    std::array<Card::Cards, NUM_PLAYERS> hands;
    for (unsigned int i(0); i < NUM_PLAYERS; ++i)
        hands[i] = Test::sortedByColour(Card::Cards(deck.begin() + i * cardsPerPlayer, deck.begin() + (i + 1) * cardsPerPlayer));

    if (exchange) {
        std::array<Card::Cards, NUM_PLAYERS> given;
        for (unsigned int i(0); i < NUM_PLAYERS; ++i) {
            given[i] = selectCardsToExchange(hands[i]);
            BOOST_TEST_REQUIRE(given[i].size() == CARDS_TO_EXCHANGE);
            for (const auto& card : given[i])
                BOOST_TEST_REQUIRE(Test::removeCard(hands[i], card), "Exchanged card " << card << " not in hand");
        }
        for (unsigned int i(0); i < NUM_PLAYERS; ++i) {
            Card::Cards& target(hands[(i + exchange) % NUM_PLAYERS]);
            target.insert(target.end(), given[i].begin(), given[i].end());
            target = Test::sortedByColour(target);
        }
    }

    Card::Cards all;
    for (const auto& hand : hands) {
        BOOST_TEST_REQUIRE(hand.size() == cardsPerPlayer);
        all.insert(all.end(), hand.begin(), hand.end());
    }
    BOOST_TEST_REQUIRE(Test::sameCards(all, Card::createDeck()));

    Table table;
    std::array<Card::Cards, NUM_PLAYERS> won;
    unsigned int leader(startPlayer(hands));
    BOOST_TEST_REQUIRE(hands[leader][0].is(Card::Value::CLUBS, Card::Value::TWO));

    for (unsigned int trick(0); trick < cardsPerPlayer; ++trick) {
        for (unsigned int i(0); i < NUM_PLAYERS; ++i) {
            const unsigned int player((leader + i) % NUM_PLAYERS);
            Card::Cards& hand(hands[player]);
            BOOST_TEST_CONTEXT("Trick " << trick << ", player " << player << ", hand " << Card::Cards(hand) << ", trick "
                                        << Card::Cards(table.trick)) {
                const unsigned int pos(selectCardToPlay(hand, table));
                BOOST_TEST_REQUIRE(pos < hand.size());
                BOOST_TEST_REQUIRE(static_cast<int>(checkPlay(hand, pos, table)) == static_cast<int>(PlayError::NONE),
                                   "Computer player plays " << hand[pos] << ": " << describe(checkPlay(hand, pos, table)));
                table.play(hand[pos]);
                hand.erase(hand.begin() + pos);
            }
        }

        leader = (leader + trickWinner(table.trick)) % NUM_PLAYERS;
        won[leader].insert(won[leader].end(), table.trick.begin(), table.trick.end());
        table.clearTrick();
    }

    std::array<unsigned int, NUM_PLAYERS> points{};
    all.clear();
    for (unsigned int i(0); i < NUM_PLAYERS; ++i) {
        BOOST_TEST_REQUIRE(hands[i].empty());
        points[i] = pointsOf(won[i]);
        all.insert(all.end(), won[i].begin(), won[i].end());
    }
    BOOST_TEST_REQUIRE(Test::sameCards(all, Card::createDeck()));
    BOOST_TEST_REQUIRE(std::accumulate(points.begin(), points.end(), 0u) == ALL_POINTS);

    const auto score(roundScore(points));
    const int sum(std::accumulate(score.begin(), score.end(), 0));
    BOOST_TEST_REQUIRE(((sum == static_cast<int>(ALL_POINTS)) || (sum == static_cast<int>(3 * ALL_POINTS))));
    return score;
}

} // namespace

BOOST_AUTO_TEST_CASE(computer_players_play_complete_games) {
    const Test::Seeds seeds;
    for (unsigned int seed(seeds.first); seed < seeds.end(); ++seed) {
        BOOST_TEST_CONTEXT("Seed " << seed) {
            std::array<int, NUM_PLAYERS> total{};
            unsigned int exchange(3); // As in Hearts: Exchange with 3, 2, 1, 0 (none), ...
            unsigned int round(0);
            while (*std::ranges::max_element(total) < END_POINTS) {
                BOOST_TEST_REQUIRE(round < MAX_ROUNDS, "Game doesn't end");
                BOOST_TEST_CONTEXT("Round " << round) {
                    const auto score(playRound(seed * MAX_ROUNDS + round, exchange));
                    for (unsigned int i(0); i < NUM_PLAYERS; ++i)
                        total[i] += score[i];
                }
                exchange = (exchange + NUM_PLAYERS - 1) % NUM_PLAYERS;
                ++round;
            }
        }
    }
}
