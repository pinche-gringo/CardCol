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

// Simulation of complete Buraco games played by four computer players,
// checking the invariants of the game after every move.
//
// The driver mirrors the game flow of the Buraco class (for computer players
// only): A player takes a card (or the dumped cards), plays cards to the piles
// on the table (one move at a time) and ends his turn by dumping a card. After
// every move (and twice after dumping) the "cleanup" is performed, which
// removes a finished pile (cerrado), gives the taken dumped cards to the
// player, gives him the reserve or ends the round.

#define BOOST_TEST_MODULE BuracoSimulation
#include <boost/test/unit_test.hpp>

#include <algorithm>
#include <numeric>

#include "BuracoRules.h"

#include "TestUtil.h"

using namespace BuracoRules;
using Card::Value;

namespace {

constexpr int END_POINTS = 2000;           ///< Points ending the game (see Buraco::ENDPOINTS)
constexpr unsigned int CARDS_TO_DEAL = 11; ///< Default of Buraco::CARDS2DEAL
constexpr unsigned int MAX_ROUNDS = 200;
constexpr unsigned int MAX_MOVES = 5000; ///< Maximal number of moves (incl. dumping) per round

/// Statistics over all simulated games
struct Statistics {
    unsigned int games{0};
    unsigned int rounds{0};
    unsigned int roundsGoingOut{0};
    unsigned int roundsStapleEmpty{0};
    unsigned int reservesTaken{0};
    unsigned int pickUps{0};
    unsigned int cerrados{0};
    unsigned int moves{0};
    std::size_t minStaple{-1UL}; ///< Minimal number of cards left on the staple
};
Statistics stats;

/// State of a round, as the Buraco class holds it
class Round {
  public:
    Round(unsigned int seed, unsigned int startPlayer);

    /// Plays the round
    /// \returns RoundScore Score of the round
    RoundScore play();

  private:
    bool cleanup(unsigned int player);
    bool playCards(unsigned int player);
    void checkInvariants() const;
    void checkPiles(unsigned int team) const;
    void playToPile(unsigned int player, unsigned int pile, unsigned int pos, unsigned int first, unsigned int last);
    RoundScore endRound();

    Table table;
    std::array<Card::Cards, NUM_TEAMS> reserve;
    std::array<std::vector<bool>, NUM_TEAMS> closed; ///< Flags, if the piles have been closed (and counted)
    Card::Cards dumped;
    Card::Cards staple;
    unsigned int current;
    bool startTurn{true};
    unsigned int wentOut{-1U}; ///< Team which ended the round
};

//-----------------------------------------------------------------------------
/// Deals the cards
//-----------------------------------------------------------------------------
Round::Round(unsigned int seed, unsigned int startPlayer) : current(startPlayer) {
    Deal cards(deal(Test::shuffledDeck(seed, NUM_DECKS, NUM_JOKERS), CARDS_TO_DEAL));
    table.hands = cards.hands;
    reserve = cards.reserve;
    dumped.push_back(cards.dumped);
    staple = cards.staple;

    for (const auto& hand : table.hands)
        BOOST_TEST_REQUIRE(hand.size() == cardsInHand(CARDS_TO_DEAL));
    for (const auto& r : reserve)
        BOOST_TEST_REQUIRE(r.size() == CARDS_TO_DEAL);

    table.dumped = 1;
    table.startGame = true;
    checkInvariants();
}

//-----------------------------------------------------------------------------
/// Checks that no card has been lost or duplicated and that the table is consistent
//-----------------------------------------------------------------------------
void Round::checkInvariants() const {
    // All cards of 4 decks with 3 jokers each; jokers have the IDs 52 - 54
    std::array<int, Value::CARDS_PER_DECK + NUM_JOKERS> count{};
    auto add([&count](const Card::Cards& cards) {
        for (const auto& c : cards) {
            BOOST_TEST_REQUIRE(c.id() < count.size());
            ++count[c.id()];
        }
    });
    for (const auto& hand : table.hands)
        add(hand);
    for (unsigned int t(0); t < NUM_TEAMS; ++t) {
        for (const auto& pile : table.piles[t])
            add(pile);
        add(reserve[t]);
    }
    add(dumped);
    add(staple);
    for (unsigned int i(0); i < count.size(); ++i)
        BOOST_TEST_REQUIRE(count[i] == static_cast<int>(NUM_DECKS), "Card " << Value(i) << " exists " << count[i] << " times");

    BOOST_TEST_REQUIRE(table.dumped == dumped.size());
    for (unsigned int t(0); t < NUM_TEAMS; ++t) {
        BOOST_TEST_REQUIRE(table.reserve[t] == !reserve[t].empty());
        BOOST_TEST_REQUIRE(closed[t].size() == table.piles[t].size());

        // Started piles of monos are counted
        unsigned int monoPiles(0);
        int points(0);
        for (unsigned int p(0); p < table.piles[t].size(); ++p) {
            const Card::Cards& pile(table.piles[t][p]);
            BOOST_TEST_REQUIRE(pile.size() <= CERRADO);
            BOOST_TEST_REQUIRE(isValidPile(pile), "Invalid pile " << pile);
            if (closed[t][p]) {
                BOOST_TEST_REQUIRE(pile.size() == CERRADO);
                points += pilePoints(pile);
            }
            else if (containsOnlyJoker(pile) && (pile.size() > 1))
                ++monoPiles;
        }
        BOOST_TEST_REQUIRE(table.unfinishedMonoPiles[t] == monoPiles);
        BOOST_TEST_REQUIRE(table.points[t] == (points + ((wentOut == t) ? GOING_OUT_BONUS : 0)));
    }
}

//-----------------------------------------------------------------------------
/// Checks that the piles of the team have at least 3 cards
/// \param team Team to inspect
//-----------------------------------------------------------------------------
void Round::checkPiles(unsigned int team) const {
    for (const auto& pile : table.piles[team])
        BOOST_TEST_REQUIRE(pile.size() >= 3, "Pile with less than 3 cards: " << pile);
}

//-----------------------------------------------------------------------------
/// Cleanup after a move (see Buraco::cleanup)
/// \param player Player who made the last move
/// \returns bool True, if the round ended
//-----------------------------------------------------------------------------
bool Round::cleanup(unsigned int player) {
    const unsigned int team(player & 1);

    // Remove (only) the first cerrado
    for (unsigned int p(0); p < table.piles[team].size(); ++p)
        if ((table.piles[team][p].size() == CERRADO) && !closed[team][p]) {
            closed[team][p] = true;
            table.points[team] += pilePoints(table.piles[team][p]);
            if (pilePoints(table.piles[team][p]) > 999)
                --table.unfinishedMonoPiles[team];
            ++stats.cerrados;
            break;
        }

    Card::Cards& hand(table.hands[player]);
    if (table.pickUpPlayed) {
        BOOST_TEST_REQUIRE(dumped.size());
        table.pickUpPlayed = false;
        hand.insert(hand.end(), dumped.begin(), dumped.end());
        dumped.clear();
        table.dumped = 0;
        std::ranges::sort(hand, lessByNumberWithJokers);
    }

    switch (handStatus(hand, table.reserve[team])) {
    case HandStatus::TAKE_RESERVE:
        takeReserve(hand, reserve[team]);
        reserve[team].clear();
        table.reserve[team] = false;
        table.buraco[team] = player >> 1;
        ++stats.reservesTaken;
        break;

    case HandStatus::GOING_OUT:
        BOOST_TEST_REQUIRE(table.points[team] > 100, "Going out without cerrado");
        table.points[team] += GOING_OUT_BONUS;
        wentOut = team;
        ++stats.roundsGoingOut;
        return true;

    case HandStatus::PLAYING:
        break;
    }
    checkInvariants();
    return false;
}

//-----------------------------------------------------------------------------
/// Moves the cards of the player to the pile (like flipCards2Play and animateCards)
/// \param player Player playing
/// \param pile Pile to play to
/// \param pos Position in pile to insert the cards
/// \param first First card to play
/// \param last Last card to play
//-----------------------------------------------------------------------------
void Round::playToPile(unsigned int player, unsigned int pile, unsigned int pos, unsigned int first, unsigned int last) {
    Card::Cards& hand(table.hands[player]);
    auto& piles(table.piles[player & 1]);
    BOOST_TEST_REQUIRE(first <= last);
    BOOST_TEST_REQUIRE(last < hand.size());
    BOOST_TEST_REQUIRE(pile < piles.size());
    BOOST_TEST_REQUIRE(pos <= piles[pile].size());
    piles[pile].insert(piles[pile].begin() + pos, hand.begin() + first, hand.begin() + last + 1);
    hand.erase(hand.begin() + first, hand.begin() + last + 1);
}

//-----------------------------------------------------------------------------
/// Makes one move of the player (see Buraco::playCards)
/// \param player Player in turn
/// \returns bool True, if the turn has been ended
//-----------------------------------------------------------------------------
bool Round::playCards(unsigned int player) {
    const unsigned int team(player & 1);
    Card::Cards& hand(table.hands[player]);
    BOOST_TEST_REQUIRE(!hand.empty());

    if (startTurn && (table.buraco[team] == (player >> 1)))
        table.buraco[team] = 0x3;

    if (startTurn) {
        BOOST_TEST_REQUIRE(dumped.size());
        startTurn = false;

        const Value top(dumped.back());
        if (takeDumped(table, player, top)) {
            ++stats.pickUps;
            if (!table.startGame) {
                BOOST_TEST(static_cast<int>(checkPickUp(table, player, top)) == static_cast<int>(PlayError::NONE),
                           "Computer player picks up " << top << ": " << describe(checkPickUp(table, player, top)));
                if (dumped.size() > 1)
                    table.pickUpPlayed = true;

                Card::Cards before(hand);
                const PickUp cards(playPickedUp(table, player, top));
                BOOST_TEST_REQUIRE(Test::sameCards(before, hand));
                BOOST_TEST_REQUIRE(cards.first <= cards.last);
                BOOST_TEST_REQUIRE(cards.last < hand.size());

                table.piles[team].emplace_back();
                closed[team].push_back(false);
                playToPile(player, table.piles[team].size() - 1, 0, cards.first, cards.last);
                Card::Cards& pile(table.piles[team].back());
                BOOST_TEST_REQUIRE(cards.posTaken <= pile.size());
                pile.insert(pile.begin() + cards.posTaken, top);
                dumped.pop_back();
                --table.dumped;
                BOOST_TEST_REQUIRE(pile.size() >= 3, "New pile with picked up card too small: " << pile);
                checkInvariants();
                return false;
            }
            insertSorted(hand, top);
            dumped.pop_back();
            --table.dumped;
        }
        else {
            if (staple.empty()) {
                // Remark: The Buraco class doesn't handle this case (it crashes)
                ++stats.roundsStapleEmpty;
                return true;
            }
            insertSorted(hand, staple.back());
            staple.pop_back();
            stats.minStaple = std::min(stats.minStaple, staple.size());
        }
        table.startGame = false;
    }

    const Table before(table);
    Move move(selectMove(table, player));
    ++stats.moves;
    BOOST_TEST_REQUIRE(Test::sameCards(before.hands[player], hand));
    BOOST_TEST_REQUIRE(move.first <= move.last);
    BOOST_TEST_REQUIRE(move.last < hand.size());

    // Moved jokers must be in the piles they are moved in
    for (const auto& m : move.jokerMoves) {
        BOOST_TEST_REQUIRE(m.pile < table.piles[team].size());
        BOOST_TEST_REQUIRE(Test::sameCards(before.piles[team][m.pile], table.piles[team][m.pile]));
        BOOST_TEST_REQUIRE(((move.kind == Move::ADD_TO_PILE) && (move.pile == m.pile)),
                           "Joker moved in " << before.piles[team][m.pile] << " without playing a card to it (move " << move.kind
                                             << " to " << move.pile << "; hand " << before.hands[player] << ')');
    }

    switch (move.kind) {
    case Move::DUMP:
        BOOST_TEST_REQUIRE(static_cast<int>(checkDump(table, player)) == static_cast<int>(PlayError::NONE));
        dumped.push_back(hand[move.first]);
        hand.erase(hand.begin() + move.first);
        ++table.dumped;
        startTurn = true;
        return true;

    case Move::NEW_PILE: {
        BOOST_TEST_REQUIRE(move.pile == table.piles[team].size());
        table.piles[team].emplace_back();
        closed[team].push_back(false);
        playToPile(player, move.pile, 0, move.first, move.last);
        const Card::Cards& pile(table.piles[team][move.pile]);
        BOOST_TEST_REQUIRE(pile.size() >= 3, "New pile too small: " << pile);
        BOOST_TEST_REQUIRE(isValidPile(pile), "Invalid new pile " << pile);
        break;
    }

    case Move::ADD_TO_PILE: {
        BOOST_TEST_REQUIRE(move.first == move.last);
        const Value card(hand[move.first]);
        // The move must be valid (like for a human player)
        const unsigned int pos(Card::findID(before.hands[player], card.id()));
        BOOST_TEST_REQUIRE(static_cast<int>(checkAddToPile(before, player, pos, move.pile)) == static_cast<int>(PlayError::NONE),
                           "Computer player plays " << card << " to " << before.piles[team][move.pile] << ": "
                                                    << describe(checkAddToPile(before, player, pos, move.pile)));
        playToPile(player, move.pile, move.pos, move.first, move.last);
        const Card::Cards& pile(table.piles[team][move.pile]);
        BOOST_TEST_REQUIRE(isValidPile(pile), "Playing " << card << " to " << before.piles[team][move.pile] << " gives " << pile);
        break;
    }
    }
    return false;
}

//-----------------------------------------------------------------------------
/// Calculates the score at the end of the round and checks it
/// \returns RoundScore Score of the round
//-----------------------------------------------------------------------------
RoundScore Round::endRound() {
    checkInvariants();
    const RoundScore score(roundScore(table));
    for (unsigned int t(0); t < NUM_TEAMS; ++t) {
        BOOST_TEST_REQUIRE(score.bonus[t] == table.points[t] + (table.reserve[t] ? -RESERVE_BONUS : RESERVE_BONUS));

        int sum(0), monoPiles(0);
        for (const auto& pile : table.piles[t]) {
            sum += pointsOf(pile);
            if (pilePoints(pile) < 0)
                monoPiles += 1000;
        }
        const int cards(score.cards[t] + monoPiles + static_cast<int>(pointsOf(table.hands[t]) + pointsOf(table.hands[t + 2])));
        BOOST_TEST_REQUIRE(((cards == sum) || (cards == -sum)));
    }
    return score;
}

//-----------------------------------------------------------------------------
/// Plays the round
/// \returns RoundScore Score of the round
//-----------------------------------------------------------------------------
RoundScore Round::play() {
    ++stats.rounds;
    for (unsigned int moves(0); moves < MAX_MOVES; ++moves) {
        BOOST_TEST_CONTEXT("Move " << moves << ", player " << current << ", hand " << table.hands[current]) {
            // makeMove: Cleanup the last move; then play
            if (cleanup(startTurn ? ((current + NUM_PLAYERS - 1) % NUM_PLAYERS) : current))
                return endRound();

            if (playCards(current)) {
                if (!startTurn) // Staple is empty
                    return endRound();

                // turnEnded: Cleanup the player who ended the turn
                checkPiles(current & 1);
                const unsigned int player(current);
                current = (current + 1) % NUM_PLAYERS;
                if (cleanup(player))
                    return endRound();
            }
            else
                checkInvariants();
        }
    }
    BOOST_FAIL("Round doesn't end");
    return {};
}

} // namespace

BOOST_AUTO_TEST_CASE(computer_players_play_complete_games) {
    const Test::Seeds seeds;
    for (unsigned int seed(seeds.first); seed < seeds.end(); ++seed) {
        BOOST_TEST_CONTEXT("Seed " << seed) {
            ++stats.games;
            Card::seedRandom(seed);
            unsigned int start(startPlayer());
            std::array<int, NUM_TEAMS> total{};
            unsigned int round(0);
            while (*std::ranges::max_element(total) < END_POINTS) {
                BOOST_TEST_REQUIRE(round < MAX_ROUNDS, "Game doesn't end");
                BOOST_TEST_CONTEXT("Round " << round) {
                    Round r(seed * MAX_ROUNDS + round, start);
                    const RoundScore score(r.play());
                    for (unsigned int t(0); t < NUM_TEAMS; ++t)
                        total[t] += score.bonus[t] + score.cards[t];
                }
                start = (start + 1) % NUM_PLAYERS;
                ++round;
            }
        }
    }
    BOOST_TEST_MESSAGE("Games: " << stats.games << "; rounds: " << stats.rounds << " (going out: " << stats.roundsGoingOut
                                 << ", staple empty: " << stats.roundsStapleEmpty << "); moves: " << stats.moves
                                 << "; pick ups: " << stats.pickUps << "; reserves: " << stats.reservesTaken
                                 << "; cerrados: " << stats.cerrados << "; min. staple: " << stats.minStaple);
}
