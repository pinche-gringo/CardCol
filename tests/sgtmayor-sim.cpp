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

// Simulation of complete Sgt. Mayor games played by three computer players,
// checking the invariants of the game after every move

#define BOOST_TEST_MODULE SgtMayorSimulation
#include <boost/test/unit_test.hpp>

#include <numeric>

#include "SgtMayorRules.h"

#include "TestUtil.h"

using namespace SgtMayorRules;
using Card::Value;

namespace {

constexpr unsigned int MAX_ROUNDS = 1000;
constexpr unsigned int MAX_EXCHANGES = 2 * CARDS_PER_PLAYER;

/// State of a game, as held by the GUI (see SgtMayor)
struct Game {
    unsigned int startPlayer;
    std::array<int, NUM_PLAYERS> diffTricks{}; ///< Result of the last round
    std::array<int, NUM_PLAYERS> points{};     ///< Points of the game
};

/// Returns the cards used in the game (a deck without the two of clubs)
Card::Cards cardsOfGame() {
    Card::Cards result(Card::createDeck());
    Test::removeCard(result, UNUSED_CARD);
    return result;
}

/// Checks that no card is lost or duplicated
void checkCards(const std::array<Card::Cards, NUM_PLAYERS>& hands, const std::array<Card::Cards, NUM_PLAYERS>& won,
                const Table& table) {
    Card::Cards all(table.trick);
    for (unsigned int i(0); i < NUM_PLAYERS; ++i) {
        all.insert(all.end(), hands[i].begin(), hands[i].end());
        all.insert(all.end(), won[i].begin(), won[i].end());
    }
    BOOST_TEST_REQUIRE(Test::sameCards(all, cardsOfGame()));

    // Every card in a hand is unplayed; every other card is played
    std::size_t played(0);
    for (const auto& colour : table.played)
        played += colour.count();
    unsigned int inHands(0);
    for (const auto& hand : hands) {
        inHands += hand.size();
        for (const auto& card : hand)
            BOOST_TEST_REQUIRE(!table.played[card.colour()][card.number()], "Card " << card << " in hand is marked as played");
    }
    BOOST_TEST_REQUIRE(played == (Value::CARDS_PER_DECK - inHands));
}

/// Plays a round (from dealing to scoring)
/// \param game State of the game; updated
/// \param seed Seed of the round
void playRound(Game& game, unsigned int seed) {
    // Deal the cards from the top of the pile (as SgtMayor::start)
    Card::Cards deck(Test::shuffledDeck(seed));
    game.startPlayer = nextStartPlayer(game.startPlayer);

    std::array<Card::Cards, NUM_PLAYERS> hands;
    for (unsigned int i(0); i < NUM_PLAYERS; ++i)
        for (unsigned int j(0); j < CARDS_PER_PLAYER;) {
            if (isUsed(deck.back())) {
                hands[i].push_back(deck.back());
                ++j;
            }
            deck.pop_back();
        }
    // The unused card might be the last one (see SgtMayor::start)
    if (deck.size()) {
        BOOST_TEST_REQUIRE(!isUsed(deck.back()));
        deck.pop_back();
    }
    BOOST_TEST_REQUIRE(deck.empty());
    for (auto& hand : hands)
        hand = Test::sortedByColour(hand);

    std::array<Card::Cards, NUM_PLAYERS> won;
    Table table;
    table.newRound();
    checkCards(hands, won, table);

    // Players having made too much tricks in the last round exchange cards
    unsigned int exchanges(0);
    const int expectedExchanges(std::accumulate(game.diffTricks.begin(), game.diffTricks.end(), 0,
                                                [](int sum, int diff) { return sum + std::max(diff, 0); }));
    while (const auto ex = nextExchange(game.diffTricks, game.startPlayer)) {
        BOOST_TEST_REQUIRE(++exchanges <= MAX_EXCHANGES, "Exchanging doesn't end");
        BOOST_TEST_REQUIRE(ex->playerBad != ex->playerGood);
        BOOST_TEST_REQUIRE(game.diffTricks[ex->playerBad] > 0);
        BOOST_TEST_REQUIRE(game.diffTricks[ex->playerGood] < 0);

        Card::Cards& handBad(hands[ex->playerBad]);
        Card::Cards& handGood(hands[ex->playerGood]);
        const unsigned int posBad(selectBadCard(handBad));
        BOOST_TEST_REQUIRE(posBad < handBad.size());
        const Value bad(handBad[posBad]);
        const unsigned int posGood(selectGoodCard(handGood, bad));
        BOOST_TEST_REQUIRE(posGood < handGood.size());
        BOOST_TEST_REQUIRE(isValidExchange(handGood, posGood, bad),
                           "Invalid exchange of " << bad << " with " << handGood[posGood] << " out of " << handGood);
        const Value good(handGood[posGood]);

        handBad.erase(handBad.begin() + posBad);
        handGood.erase(handGood.begin() + posGood);
        handGood.push_back(bad);
        handBad.push_back(good);
        handBad = Test::sortedByColour(handBad);
        handGood = Test::sortedByColour(handGood);

        applyExchange(game.diffTricks, *ex);
        checkCards(hands, won, table);
    }
    BOOST_TEST_REQUIRE(static_cast<int>(exchanges) == expectedExchanges);
    BOOST_TEST_REQUIRE((game.diffTricks == std::array<int, NUM_PLAYERS>{}));
    for (const auto& hand : hands)
        BOOST_TEST_REQUIRE(hand.size() == CARDS_PER_PLAYER);

    // Select the trump and play
    table.trump = selectTrump(hands[trumpPlayer(game.startPlayer)]);
    BOOST_TEST_REQUIRE(table.trump <= Value::HEARTS);

    unsigned int leader(game.startPlayer);
    for (unsigned int trick(0); trick < CARDS_PER_PLAYER; ++trick) {
        for (unsigned int i(0); i < NUM_PLAYERS; ++i) {
            const unsigned int player((leader + i) % NUM_PLAYERS);
            Card::Cards& hand(hands[player]);
            BOOST_TEST_CONTEXT("Trick " << trick << ", player " << player << ", trump " << static_cast<int>(table.trump)
                                        << ", hand " << Card::Cards(hand) << ", trick " << Card::Cards(table.trick)) {
                BOOST_TEST_REQUIRE(hand.size() == (CARDS_PER_PLAYER - trick));
                const unsigned int pos(selectCardToPlay(hand, player, game.startPlayer, table));
                BOOST_TEST_REQUIRE(pos < hand.size());
                BOOST_TEST_REQUIRE(static_cast<int>(checkPlay(hand, pos, table)) == static_cast<int>(PlayError::NONE),
                                   "Computer player plays " << hand[pos] << ": " << describe(checkPlay(hand, pos, table)));
                table.play(hand[pos]);
                hand.erase(hand.begin() + pos);
                checkCards(hands, won, table);
            }
        }

        const unsigned int best(trickWinner(table.trick, table.trump));
        BOOST_TEST_REQUIRE(best < NUM_PLAYERS);
        // The winner played the highest trump, or (if there is none) the highest card of the first colour
        const Value::COLOURS colour(Card::exists(table.trick, table.trump) ? table.trump : table.trick[0].colour());
        BOOST_TEST_REQUIRE(table.trick[best].colour() == colour);
        for (const auto& card : table.trick)
            BOOST_TEST_REQUIRE(((card.colour() != colour) || (card.number() <= table.trick[best].number())));

        leader = (leader + best) % NUM_PLAYERS;
        won[leader].insert(won[leader].end(), table.trick.begin(), table.trick.end());
        table.clearTrick();
        checkCards(hands, won, table);
    }

    std::array<unsigned int, NUM_PLAYERS> tricks{};
    for (unsigned int i(0); i < NUM_PLAYERS; ++i) {
        BOOST_TEST_REQUIRE(hands[i].empty());
        BOOST_TEST_REQUIRE((won[i].size() % NUM_PLAYERS) == 0u);
        tricks[i] = won[i].size() / NUM_PLAYERS;
    }
    BOOST_TEST_REQUIRE(std::accumulate(tricks.begin(), tricks.end(), 0u) == CARDS_PER_PLAYER);

    game.diffTricks = roundScore(tricks, game.startPlayer);
    BOOST_TEST_REQUIRE(std::accumulate(game.diffTricks.begin(), game.diffTricks.end(), 0) == 0);
    for (unsigned int i(0); i < NUM_PLAYERS; ++i) {
        BOOST_TEST_REQUIRE(game.diffTricks[i] ==
                           (static_cast<int>(tricks[i]) - static_cast<int>(neededTricks(i, game.startPlayer))));
        game.points[i] += game.diffTricks[i];
    }
    BOOST_TEST_REQUIRE(std::accumulate(game.points.begin(), game.points.end(), 0) == 0);
}

} // namespace

BOOST_AUTO_TEST_CASE(computer_players_play_complete_games) {
    const Test::Seeds seeds;
    unsigned long rounds(0);
    for (unsigned int seed(seeds.first); seed < seeds.end(); ++seed) {
        BOOST_TEST_CONTEXT("Seed " << seed) {
            Card::seedRandom(seed);
            Game game{Card::randomNumber(NUM_PLAYERS)};
            unsigned int round(0);
            while (!isGameOver(game.points)) {
                BOOST_TEST_REQUIRE(round < MAX_ROUNDS, "Game doesn't end");
                BOOST_TEST_CONTEXT("Round " << round) { playRound(game, seed * MAX_ROUNDS + round); }
                ++round;
            }
            const unsigned int best(winner(game.points));
            BOOST_TEST_REQUIRE(game.points[best] >= static_cast<int>(END_POINTS));
            BOOST_TEST_REQUIRE(game.points[best] == *std::ranges::max_element(game.points));
            rounds += round;
        }
    }
    BOOST_TEST_MESSAGE("Played " << seeds.count << " games with " << rounds << " rounds");
}
