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

// Simulation of complete Jabberwocky games played by four computer players,
// checking the invariants of the game after every move

#define BOOST_TEST_MODULE JabberwockySimulation
#include <boost/test/unit_test.hpp>

#include <numeric>

#include "JabberwockyRules.h"

#include "TestUtil.h"

using namespace JabberwockyRules;
using Card::Value;

namespace {

constexpr unsigned int MAX_MOVES = 1000; ///< Maximal number of cards played in a round

/// Independent check of the winner of a trick: The highest trump or (if none
/// has been played) the highest card of the colour played first
bool isWinner(const Card::Cards& trick, unsigned int pos, Value::COLOURS trump) {
    const Value::COLOURS colour(Card::exists(trick, trump) ? trump : trick[0].colour());
    if (trick[pos].colour() != colour)
        return false;
    for (const auto& card : trick)
        if ((card.colour() == colour) && (card.number() > trick[pos].number()))
            return false;
    return true;
}

/// Plays a round (from dealing to scoring)
/// \param seed Seed of the round
/// \param round Number of the round
/// \param startPlayer Player starting to bid and to play
/// \returns std::array<int, NUM_PLAYERS> Score of the round
std::array<int, NUM_PLAYERS> playRound(unsigned int seed, unsigned int round, unsigned int startPlayer) {
    const Card::Cards deck(Test::shuffledDeck(seed));
    const unsigned int tricks(tricksOfRound(round));

    // Deal as Jabberwocky::start(): Each player gets his cards from the top of
    // the pile (its end); the next card defines the trump
    Card::Cards stock(deck);
    std::array<Card::Cards, NUM_PLAYERS> hands;
    for (unsigned int i(0); i < NUM_PLAYERS; ++i)
        for (unsigned int j(0); j < tricks; ++j) {
            hands[i].push_back(stock.back());
            stock.pop_back();
        }
    const Value trump(stock.back());
    stock.pop_back();
    for (auto& hand : hands) {
        std::ranges::sort(hand, [&trump](const Value& a, const Value& b) { return lessByColourAccTrump(a, b, trump.colour()); });
        BOOST_TEST_REQUIRE(hand.size() == tricks);
    }

    Table table(trump, round);
    std::array<Card::Cards, NUM_PLAYERS> won;
    auto checkConservation([&]() {
        Card::Cards all(stock);
        all.push_back(trump);
        all.insert(all.end(), table.trick.begin(), table.trick.end());
        for (unsigned int i(0); i < NUM_PLAYERS; ++i) {
            all.insert(all.end(), hands[i].begin(), hands[i].end());
            all.insert(all.end(), won[i].begin(), won[i].end());
        }
        BOOST_TEST_REQUIRE(Test::sameCards(all, Card::createDeck()), "Cards lost or duplicated");
    });
    checkConservation();

    // Bidding
    std::array<unsigned int, NUM_PLAYERS> bids{};
    unsigned int sumBids(0);
    for (unsigned int i(0); i < NUM_PLAYERS; ++i) {
        const unsigned int player((startPlayer + i) % NUM_PLAYERS);
        const bool last(i == (NUM_PLAYERS - 1));
        const unsigned int bid(selectBid(hands[player], trump.colour(), round, last, sumBids));
        BOOST_TEST_REQUIRE(estimateTricks(hands[player], trump.colour(), round) <= tricks);
        BOOST_TEST_REQUIRE(static_cast<int>(checkBid(bid, sumBids, last, round)) == static_cast<int>(BidError::NONE),
                           "Player " << player << " bids " << bid << " (others " << sumBids
                                     << "): " << describe(checkBid(bid, sumBids, last, round)));
        bids[player] = bid;
        sumBids += bid;
    }
    BOOST_TEST_REQUIRE(sumBids != tricks);

    // Playing
    std::array<unsigned int, NUM_PLAYERS> tricksWon{};
    unsigned int leader(startPlayer);
    unsigned int moves(0);
    while (hands[leader].size()) {
        for (unsigned int i(0); i < NUM_PLAYERS; ++i) {
            BOOST_TEST_REQUIRE(++moves <= MAX_MOVES, "Round doesn't end");
            const unsigned int player((leader + i) % NUM_PLAYERS);
            Card::Cards& hand(hands[player]);
            BOOST_TEST_CONTEXT("Trump " << trump << ", player " << player << ", bid " << bids[player] << ", won "
                                        << tricksWon[player] << ", hand " << Card::Cards(hand) << ", trick "
                                        << Card::Cards(table.trick)) {
                BOOST_TEST_REQUIRE(hand.size());
                const unsigned int pos(selectCardToPlay(hand, player, bids[player], tricksWon[player], table));
                BOOST_TEST_REQUIRE(pos < hand.size());
                BOOST_TEST_REQUIRE(static_cast<int>(checkPlay(hand, pos, table)) == static_cast<int>(PlayError::NONE),
                                   "Computer player plays " << hand[pos] << ": " << describe(checkPlay(hand, pos, table)));
                table.play(hand[pos]);
                hand.erase(hand.begin() + pos);
                checkConservation();
            }
        }

        const unsigned int posWinner(trickWinner(table.trick, trump.colour()));
        BOOST_TEST_REQUIRE(isWinner(table.trick, posWinner, trump.colour()),
                           "Wrong winner " << posWinner << " of " << Card::Cards(table.trick));
        leader = (leader + posWinner) % NUM_PLAYERS;
        won[leader].insert(won[leader].end(), table.trick.begin(), table.trick.end());
        ++tricksWon[leader];
        table.clearTrick();
        checkConservation();
    }

    for (unsigned int i(0); i < NUM_PLAYERS; ++i) {
        BOOST_TEST_REQUIRE(hands[i].empty());
        BOOST_TEST_REQUIRE(won[i].size() == (tricksWon[i] * NUM_PLAYERS));
    }
    BOOST_TEST_REQUIRE(std::accumulate(tricksWon.begin(), tricksWon.end(), 0u) == tricks);

    const auto score(roundScore(bids, tricksWon));
    for (unsigned int i(0); i < NUM_PLAYERS; ++i)
        BOOST_TEST_REQUIRE(score[i] == ((bids[i] == tricksWon[i]) ? 1 : 0));
    // As the sum of the bids differs from the number of tricks, not all players can meet their bid
    BOOST_TEST_REQUIRE(std::accumulate(score.begin(), score.end(), 0) < static_cast<int>(NUM_PLAYERS));
    return score;
}

} // namespace

BOOST_AUTO_TEST_CASE(computer_players_play_complete_games) {
    const Test::Seeds seeds;
    for (unsigned int seed(seeds.first); seed < seeds.end(); ++seed) {
        BOOST_TEST_CONTEXT("Seed " << seed) {
            // As Jabberwocky: Random start player, which changes every round
            Card::seedRandom(seed);
            unsigned int startPlayer(Card::randomNumber(NUM_PLAYERS));

            std::array<int, NUM_PLAYERS> total{};
            unsigned int round(0);
            for (; round < NUM_ROUNDS; ++round) {
                BOOST_TEST_CONTEXT("Round " << round) {
                    startPlayer = nextStartPlayer(startPlayer);
                    const auto score(playRound(seed * NUM_ROUNDS + round, round, startPlayer));
                    for (unsigned int i(0); i < NUM_PLAYERS; ++i)
                        total[i] += score[i];
                }
            }
            BOOST_TEST_REQUIRE(round == NUM_ROUNDS);
            BOOST_TEST_REQUIRE(*std::ranges::max_element(total) <= static_cast<int>(NUM_ROUNDS));
            BOOST_TEST_REQUIRE(*std::ranges::min_element(total) >= 0);
        }
    }
}
