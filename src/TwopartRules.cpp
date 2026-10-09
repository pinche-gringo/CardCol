// PROJECT     : Cardgames
// SUBSYSTEM   : Twopart
// REFERENCES  :
// TODO        :
// BUGS        :
// AUTHOR      : Markus Schwab
// CREATED     : 9.10.2026
// COPYRIGHT   : Copyright (C) 2002 - 2009, 2011, 2024, 2026

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

#include <cardgames-cfg.h>

#include <algorithm>

#include <YGP/Check.h>
#include <YGP/Trace.h>

#include <card/Cards.h>

#include "TwopartRules.h"

using Card::Value;

namespace TwopartRules {

namespace {

//-----------------------------------------------------------------------------
/// Searches for the card(s) to play in part 1
/// \param player Player to inspect
/// \param hand Cards of the player (sorted by number)
/// \param played Played cards
/// \param wonCards Number of cards the player has won so far
/// \param stockSize Number of cards on the stock
/// \param table Actual state of the game
/// \returns unsigned int Position of the card to play
//-----------------------------------------------------------------------------
unsigned int selectCardPartOne(unsigned int player, const Card::Cards& hand, const Card::Cards& played, unsigned int wonCards,
                               unsigned int stockSize, const Table& table) {
    Check1(hand.size());
    const unsigned int startRound(table.startPos[0]);
    unsigned int points(0);
    unsigned int cHigh(0);

    // Analyze played staple
    for (const auto& card : played) {
        Value::NUMBERS nr(card.number());
        points += nr;
        if (nr >= Value::TEN)
            ++cHigh;
    }

    TRACE2("TwopartRules::selectCardPartOne(...) - Points: "
           << points << "; Avg: " << (played.size() ? (points / played.size()) : 0) << "; High: " << cHigh);
    if (played.size())
        points /= played.size();

    const Analysis last(analyzePlayed(played, startRound, played.size() - startRound, table.trump));
    const int maxNr(last.max);
    const int posMaxEqual(last.maxEqualPos);
    const int maxEqualNr(last.maxEqual);
    const int trumps(last.trumps);
    const bool isLast(table.isLastInRound(player));
    auto existsInRound([&played, startRound](Value::NUMBERS nr) { return Card::find(played, nr, startRound) != -1; });

    unsigned int start(-1U);
    unsigned int end(-1U);
    // Try to get the cards if there are loads of high cards (a third or more)
    // or if the average card played is at least a 8 or there are trumps inside
    if (((played.size() / 3) < cHigh) || (points >= Value::SEVEN) || trumps) {
        // Search for card whose number you own
        for (start = 0; start < hand.size(); ++start)
            if (existsInRound(hand[start].number()) && ((posMaxEqual == -1) || (hand[start].number() >= maxEqualNr))) {
                TRACE2("TwopartRules::selectCardPartOne(...) - Having equal card at " << start);

                // Use card if it's a trump
                if (table.trump && (*table.trump == hand[start].colour()))
                    return start;
                if (end == -1U)
                    end = start;
            }
        // Reset start to first found card (or 0)
        start = (end != -1U) ? end : 0;
        TRACE6("TwopartRules::selectCardPartOne(...) - First try (I): " << start);

        // Last player plays high card if higher (even if he would have an
        // equal card in case the pile is really good) or ...
        if ((((start != end) || (cHigh > 1) || (points >= Value::TEN)) && (posMaxEqual == -1) &&
             (isLast && trumps && ((end = findBigger(hand, static_cast<Value::NUMBERS>(maxNr), table.trump)) != -1U)))
            // ... the staple is being fighted for and player has high cards
            || (startRound && ((end = hand.size() - 1),
                               ((hand[end].number() == Value::ACE)) || ((playersInBitfield(table.bfOldPlayers) < startRound) &&
                                                                        (hand[end].number() >= maxNr) && (posMaxEqual == -1))))) {
            TRACE2("TwopartRules::selectCardPartOne(...) - Playing highest card at " << end);
            start = end;
        }
        return start;
    }

    // Try to get the cards if you don't have any close to the end of part 1
    // and you are the last or the pile really sucks and just one is left
    // (of course only if there are no doubles).
    if (!wonCards && (stockSize < 13) && (posMaxEqual == -1) &&
        (isLast || (!(table.bfPlayers & ~((1U << player) | (1U << table.nextPlayer((player + 1) % NUM_PLAYERS)))) &&
                    (points < Value::SIX))) &&
        (maxNr < hand[hand.size() - 1].number()))
        return hand.size() - 1; // Play the highest card (the hand might have less than 3 cards)

    // We don't want the pile; so try not to get it. To do so, play
    // the second (biggest) card, if the first card exists in the pile and
    // the second not (though the second card must also small).
    // An exception is also for the last player
    TRACE5("TwopartRules::selectCardPartOne(...) - Avoiding pile");
    start = ((hand.size() > 1) && existsInRound(hand[0].number()) && !existsInRound(hand[1].number()) &&
             ((isLast && (hand[1].number() < maxNr)) || (hand[1].number() <= Value::SEVEN)));
    TRACE5("TwopartRules::selectCardPartOne(...) - Avoiding returns " << start);

    // Final check: If you have to pick up the pile and you're the last,
    // use at least a high card (unless there are doubles)
    if (!start && (posMaxEqual == -1) && isLast && (hand.size() > 1) && (hand[start].number() > maxNr))
        start = 1;

    TRACE5("TwopartRules::selectCardPartOne(...) - Playing card at " << start);
    return start;
}

//-----------------------------------------------------------------------------
/// Searches for the card(s) to play in part 2
/// \param hand Cards of the player (sorted by colour, trumps last)
/// \param played Played cards
/// \param table Actual state of the game
/// \returns std::optional<Play> Cards to play; nothing if the player picks up
///     the played cards
//-----------------------------------------------------------------------------
std::optional<Play> selectCardsPartTwo(unsigned int player, const Card::Cards& hand, const Card::Cards& played,
                                       const Table& table) {
    Check1(hand.size());
    Check1(table.trump);
    const Value::COLOURS trump(*table.trump);

    // Find first fitting card
    unsigned int start(-1U);
    if (played.size()) {
        auto i(std::ranges::lower_bound(hand, played.back(),
                                        [trump](const Value& a, const Value& b) { return lessByColourAccTrumps(a, b, trump); }));
        if (i != hand.end())
            start = i - hand.begin();
    }
    else
        start = findSmallestCard(hand, trump);
    TRACE5("TwopartRules::selectCardsPartTwo(...) - First try (II): " << start);

    unsigned int end(start);
    if ((start == -1U) || (played.size() && (played.back().colour() != hand[start].colour()))) {
        TRACE5("TwopartRules::selectCardsPartTwo(...) - No card found; trying trump");
        if (played.back().colour() == trump)
            return {};

        for (start = hand.size(); start; --start)
            if (hand[start - 1].colour() != trump)
                break;

        if (!start)
            end = findEndOfSerie(hand, 0);
        else {
            end = hand.size() - 1;
            // Take up pile if no trump was found or if only a "small amount"
            // of trumps are left (like less than 4 or less than the half)
            // and you are not the last player
            if ((start == hand.size()) || (!table.isLastInRound(player) && (((end - start) < 4) || (start < (end - start)))))
                return {};
            end = start;
        }
    }
    else {
        // Card was found; now search for last card to play (only if not trump
        // or only trump left)
        if (!start || (hand[start].colour() != trump))
            end = findEndOfSerie(hand, start);
    }
    TRACE5("TwopartRules::selectCardsPartTwo(...) - Playing cards at " << start << " - " << end);
    return Play{start, end};
}

} // namespace

//-----------------------------------------------------------------------------
/// Resets the state for a new game
//-----------------------------------------------------------------------------
void Table::reset() {
    bfPlayers = bfOldPlayers = ALL_PLAYERS;
    startPlayer = 0;
    startPos.fill(0);
    offPos = 0;
    trump.reset();
    partTwo = false;
}

//-----------------------------------------------------------------------------
/// Finds the next player which can continue according to the bfPlayers
/// bitfield
/// \param player Number of player to start with
/// \returns int Number of next player (or -1)
//-----------------------------------------------------------------------------
int Table::nextPlayer(unsigned int player) const {
    if (!bfPlayers) // No players left: Return -1
        return -1;

    // Find first player (starting with the passed one) being still in game
    do {
        player = (player + 1) % NUM_PLAYERS;
    }
    while (!isInRound(player));
    return player;
}

//-----------------------------------------------------------------------------
/// Converts a position in the played cards of the round into the number of
/// the player (starting with startPlayer and considering only the players in
/// the round)
/// \param pos Position to convert
/// \returns unsigned int Number of player
//-----------------------------------------------------------------------------
unsigned int Table::pos2Player(unsigned int pos) const {
    TRACE9("TwopartRules::Table::pos2Player(unsigned int) - Pos to convert: " << pos << "; starting with player " << startPlayer);
    unsigned int start(startPlayer);
    while (pos) {
        start = (start + 1) & 0x3;
        if (isInRound(start))
            --pos;
    }
    return start;
}

//-----------------------------------------------------------------------------
/// Removes all players having no cards left from the round
/// \param hands Number of cards of the players
/// \returns unsigned int Number of players left
//-----------------------------------------------------------------------------
unsigned int Table::removePlayersWithoutCards(const HandSizes& hands) {
    unsigned int cPlayers(0);
    for (unsigned int i(0); i < NUM_PLAYERS; ++i)
        if (hands[i])
            ++cPlayers;
        else
            removePlayer(i);
    return cPlayers;
}

//-----------------------------------------------------------------------------
/// Registers, that cards are played onto the played cards (in part 2)
/// \param posInPlayed Position of the first played card
//-----------------------------------------------------------------------------
void Table::registerPlay(unsigned int posInPlayed) {
    Check1(partTwo);
    Check1(offPos < startPos.size());
    TRACE1("TwopartRules::Table::registerPlay(unsigned int) - Position: " << offPos << " -> " << posInPlayed);
    startPos[offPos++] = posInPlayed;
}

//-----------------------------------------------------------------------------
/// Finishes the turn of a player, after he has played his cards (and drawn
/// a card from the stock, if possible): Checks if every player in the round
/// has played (ending the round if so) and if the part (or the game) is over.
/// \param player Player who has played
/// \param played Played cards
/// \param hands Number of cards of the players
/// \returns TurnResult What happens next
//-----------------------------------------------------------------------------
TurnResult Table::endTurn(unsigned int player, const Card::Cards& played, const HandSizes& hands) {
    TRACE5("TwopartRules::Table::endTurn(...) - Player: " << player << "; Players: " << std::hex << bfPlayers << std::dec);
    Check1(player < NUM_PLAYERS);

    TurnResult result{player, -1, false, false};
    int next(-1);

    // Check if every player is still in game or has already played; end round
    // if so or calculate next player if not
    removePlayer(player);
    if (bfPlayers)
        next = nextPlayer(player);
    else {
        result.endOfRound = true;
        if (partTwo)
            next = endRoundPartTwo(player, hands);
        else {
            unsigned int nextPlayer(endRoundPartOne(played, hands, result.winner));
            next = static_cast<int>(nextPlayer);
            if (next < 0) { // Part 1 ends; the winner starts part 2
                result.next = ~nextPlayer;
                result.endOfPart = true;
                return result;
            }
        }
    }

    // In part 2: The game ends if only one player has cards left
    if (next < 0) {
        Check3(0);
        result.endOfPart = partTwo;
        return result;
    }
    result.next = next;
    if (partTwo)
        result.endOfPart = (next == findNextPlayerWithCards(hands, next));
    return result;
}

//-----------------------------------------------------------------------------
/// Ends a round in part 1: The player with the highest card wins the played
/// cards; if cards with equal numbers have been played, the players having the
/// highest pair continue the round
/// \param played Played cards
/// \param hands Number of cards of the players
/// \param winner Player winning the played cards (or -1)
/// \returns unsigned int Next player; ~winner if the part is over
//-----------------------------------------------------------------------------
unsigned int Table::endRoundPartOne(const Card::Cards& played, const HandSizes& hands, int& winner) {
    Check3(!bfPlayers);
    unsigned int nextPlayer(NUM_PLAYERS);

    bfPlayers = bfOldPlayers;
    unsigned int cPlayers(playersInBitfield(bfPlayers));

    TRACE8("TwopartRules::Table::endRoundPartOne(...) - Round has " << cPlayers << " players; Start = " << startPos[0] << " of "
                                                                    << played.size() << " cards");
    Check3((startPos[0] + cPlayers) == played.size());

    const Analysis last(analyzePlayed(played, startPos[0], cPlayers, trump));

    TRACE4("TwopartRules::Table::endRoundPartOne(...) - Player starting round: " << startPlayer << "; players: " << cPlayers);

    // Equal cards found
    if (last.maxEqualPos >= 0) {
        unsigned int bfPlayersOut(0);
        cPlayers = 0;

        // Add players having equal cards and having still cards left
        nextPlayer = pos2Player(last.maxEqualPos - startPos[0]);
        for (unsigned int i(startPos[0]); i < played.size(); ++i) {
            if ((played[i].number() == last.maxEqual) && hands[pos2Player(i - startPos[0])]) {
                TRACE5("TwopartRules::Table::endRoundPartOne(...) - Found equal cards; Player "
                       << pos2Player(i - startPos[0]) << (cPlayers ? " still in round" : " is winner"));
                ++cPlayers;
            }
            else
                bfPlayersOut |= (1 << pos2Player(i - startPos[0]));
        }
        bfPlayers &= ~bfPlayersOut;
        TRACE5("TwopartRules::Table::endRoundPartOne(...) - Found equal cards; " << cPlayers << " player(s) still in round ("
                                                                                 << std::hex << bfPlayers << std::dec << ')');

        if (!hands[nextPlayer] && cPlayers)
            nextPlayer = this->nextPlayer(nextPlayer);

        if (cPlayers < 2) { // Less than two players found: The first one wins
            winner = nextPlayer;
            bfPlayers = ALL_PLAYERS;
            cPlayers = removePlayersWithoutCards(hands);

            if (!cPlayers)
                nextPlayer = ~nextPlayer;
            else if (!hands[nextPlayer])
                nextPlayer = this->nextPlayer(nextPlayer);
        }
    }
    // All played cards are different: Winner is the one with highest card
    else {
        startPlayer = nextPlayer = pos2Player(last.maxPos - startPos[0]);
        winner = nextPlayer;
        TRACE5("TwopartRules::Table::endRoundPartOne(...) - Found winner: " << nextPlayer);

        bfPlayers = ALL_PLAYERS; // Set all players (having cards)
        removePlayersWithoutCards(hands);

        if (!hands[nextPlayer])
            nextPlayer = this->nextPlayer(nextPlayer);
        if (nextPlayer == -1U)
            nextPlayer = ~startPlayer;
    }
    startPos[0] = played.size();

    bfOldPlayers = bfPlayers;
    startPlayer = nextPlayer;
    TRACE8("TwopartRules::Table::endRoundPartOne(...) - Continuing with player " << nextPlayer);
    return nextPlayer;
}

//-----------------------------------------------------------------------------
/// Ends a round in part 2: The played cards are out of the game; every player
/// having cards plays again
/// \param player Player who played last
/// \param hands Number of cards of the players
/// \returns int Next player (or -1, if no one has cards left)
//-----------------------------------------------------------------------------
int Table::endRoundPartTwo(unsigned int player, const HandSizes& hands) {
    Check3(!bfPlayers);
    bfPlayers = ALL_PLAYERS;
    removePlayersWithoutCards(hands);
    offPos = 0;
    int next(hands[player] ? static_cast<int>(player) : nextPlayer(player));

    bfOldPlayers = bfPlayers;
    startPlayer = next;
    TRACE8("TwopartRules::Table::endRoundPartTwo(...) - Continuing with player " << next);
    return next;
}

//-----------------------------------------------------------------------------
/// The passed player picks up the cards played last (in part 2); afterwards
/// (up to) two players having cards are re-added to the round
/// \param player Player picking up the cards
/// \param hands Number of cards of the players (before picking up)
/// \returns PickUp Cards to pick up and next player
//-----------------------------------------------------------------------------
PickUp Table::pickUp(unsigned int player, const HandSizes& hands) {
    Check1(player < NUM_PLAYERS);
    Check1(partTwo);
    Check2(offPos > 0);
    Check3(offPos <= startPos.size());
    Check3(bfPlayers);

    PickUp result{startPos[--offPos], 0};
    TRACE3("TwopartRules::Table::pickUp(unsigned int, ...) - Player " << player << " picks up played cards at " << offPos << '('
                                                                      << result.start << ')');
    removePlayer(player);

    // Calculate players to re-enable: They are the number of players still
    // in game (with cards) minus the players still in round; but maximal 2
    unsigned int num(std::ranges::count_if(hands, [](unsigned int cards) { return cards != 0; }));
    num -= playersInBitfield(bfPlayers);
    if (num > 2)
        num = 2;
    TRACE8("TwopartRules::Table::pickUp(unsigned int, ...) - Adding " << num << " players to left "
                                                                      << playersInBitfield(bfPlayers));

    // Re-enable next two players (having cards); continue with first of them
    for (unsigned int i(0); num && (i < NUM_PLAYERS); ++i) {
        unsigned int next((player + i + 1) % NUM_PLAYERS);
        if (!isInRound(next) && hands[next]) {
            TRACE5("TwopartRules::Table::pickUp(unsigned int, ...) - Re-adding player " << next);
            addPlayer(next);
            --num;
        }
    }
    Check3(!num);

    result.next = nextPlayer(player);
    return result;
}

//-----------------------------------------------------------------------------
/// Starts part 2: The won cards are moved into the hands; players without won
/// cards get the low cards (up to five) of the others
/// \param won Won cards of the players
/// \param player Player starting part 2 (the winner of the last round); if he
///     has no cards afterwards (as all his cards were given away) the next
///     player having cards
/// \returns Receivers Receivers of the won cards
/// \remarks Like at the end of the rounds in part 2, only players having cards
///     take part
//-----------------------------------------------------------------------------
Receivers Table::startPartTwo(const std::array<Card::Cards, NUM_PLAYERS>& won, unsigned int& player) {
    TRACE9("TwopartRules::Table::startPartTwo(...) - Continuing with " << player);
    Check3(!bfPlayers);
    Check1(trump);

    partTwo = true;
    offPos = 0;
    startPlayer = -1U;

    // Check if there are players without cards
    unsigned int nrPlayers(0);
    for (unsigned int i(0); i < NUM_PLAYERS; ++i)
        if (won[i].empty()) {
            TRACE5("TwopartRules::Table::startPartTwo(...) - Player " << i << " has no cards");
            addPlayer(i);
            ++nrPlayers;
        }

    // Now move the cards from the won pile to the hand; if there are players
    // without cards give them the cards up to 5
    Receivers receivers;
    unsigned int victim(player);
    for (unsigned int i(0); i < NUM_PLAYERS; ++i)
        for (unsigned int j(won[i].size()); j; --j) {
            if (bfPlayers && (won[i][j - 1].number() <= Value::FIVE)) {
                receivers[i].push_back(pos2Player(++victim));
                victim %= nrPlayers;
            }
            else
                receivers[i].push_back(i);
        }

    // Only players having cards play
    HandSizes hands{};
    for (const auto& cards : receivers)
        for (unsigned int receiver : cards)
            ++hands[receiver];
    bfPlayers = ALL_PLAYERS;
    removePlayersWithoutCards(hands);
    bfOldPlayers = bfPlayers;
    if (!hands[player])
        player = nextPlayer(player);
    Check3(player < NUM_PLAYERS);
    return receivers;
}

namespace {

/// Marks a message with its plural form for translation (extracted by
/// xgettext like ngettext; see po/meson.build) and returns one of them
constexpr const char* NN_(const char* singular, const char* plural, bool usePlural) { return usePlural ? plural : singular; }

} // namespace

//-----------------------------------------------------------------------------
/// Returns the (untranslated) message describing the passed error
/// \param error Error to describe
/// \param plural Flag, if the plural form should be returned
/// \returns const char* Message (to be translated with gettext)
//-----------------------------------------------------------------------------
const char* describe(PlayError error, bool plural) {
    switch (error) {
    case PlayError::NONE:
        break;
    case PlayError::NOT_BIGGER:
        return NN_("The played card must have the same colour and must be bigger (or be a trump)!",
                   "The played cards must have the same colour and must be bigger (or be trumps)!", plural);
    case PlayError::NO_SERIE:
        return N_("The played cards must be a serie (same colour, increasing by one)!");
    case PlayError::ONLY_ONE_CARD:
        return N_("Only one card can be played in the first part!");
    }
    return "";
}

//-----------------------------------------------------------------------------
/// Checks if the passed cards can be played. In part 1 a single card can be
/// played; in part 2 the cards must be a serie and the last one must have the
/// same colour and be bigger than the last played card or be a (bigger) trump
/// \param hand Cards of the player
/// \param start Position of first card to play
/// \param end Position of last card to play
/// \param played Played cards
/// \param table Actual state of the game
/// \returns PlayError Reason why the cards must not be played (or NONE)
//-----------------------------------------------------------------------------
PlayError checkPlay(const Card::Cards& hand, unsigned int start, unsigned int end, const Card::Cards& played,
                    const Table& table) {
    Check1(start <= end);
    Check1(end < hand.size());

    if (!table.partTwo)
        return (start == end) ? PlayError::NONE : PlayError::ONLY_ONE_CARD;

    if (findEndOfSerie(hand, start) < end)
        return PlayError::NO_SERIE;

    if (played.size()) {
        Check1(table.trump);
        const Value& top(played.back());
        const Value::NUMBERS nr(hand[end].number());
        const Value::COLOURS colour(hand[end].colour());
        if ((colour == *table.trump) ? ((top.colour() == *table.trump) && (top.number() >= nr))
                                     : ((top.colour() != colour) || (top.number() >= nr)))
            return PlayError::NOT_BIGGER;
    }
    return PlayError::NONE;
}

//-----------------------------------------------------------------------------
/// Counts the players in the passed bitfield
/// \param bfPlayers Bitfield of players
/// \returns unsigned int Number of players
//-----------------------------------------------------------------------------
unsigned int playersInBitfield(unsigned int bfPlayers) {
    unsigned int cPlayers(0);
    for (unsigned int i(0); i < NUM_PLAYERS; ++i)
        if (bfPlayers & (1 << i))
            ++cPlayers;
    return cPlayers;
}

//-----------------------------------------------------------------------------
/// Finds the next player having cards
/// \param hands Number of cards of the players
/// \param player Number of player to start with
/// \returns int Number of next player (or -1)
//-----------------------------------------------------------------------------
int findNextPlayerWithCards(const HandSizes& hands, unsigned int player) {
    Check1(player < NUM_PLAYERS);
    unsigned int i(player);
    do {
        i = (i + 1) % NUM_PLAYERS;
        if (hands[i])
            return i;
    }
    while (i != player);
    return -1;
}

//-----------------------------------------------------------------------------
/// Analyzes the played cards and retrieves the highest card(s)
/// \param played Played cards
/// \param start Position from where to start analyzing
/// \param cards Number of cards to analyze
/// \param trump Colour of trump
/// \returns Analysis Highest (single and equal) cards and number of trumps
//-----------------------------------------------------------------------------
Analysis analyzePlayed(const Card::Cards& played, unsigned int start, unsigned int cards, const Trump& trump) {
    TRACE3("TwopartRules::analyzePlayed(...) - Analyzing cards [" << start << " to " << (cards + start) << ") of "
                                                                  << played.size());
    Analysis result{-1, -1, -1, -1, 0};
    cards += start;
    Check1(cards <= played.size());

    // Check if card is bigger then all previous
    for (; start < cards; ++start) {
        const int nr(static_cast<int>(played[start].number()));
        if (nr > result.max) {
            result.max = nr;
            result.maxPos = start;
        }

        // Add trumps
        if (trump && (*trump == played[start].colour()))
            ++result.trumps;

        // Check if card has equal cards
        for (unsigned int j(start + 1); j < cards; ++j)
            if (played[start].number() == played[j].number()) {
                if (nr > result.maxEqual) {
                    TRACE3("TwopartRules::analyzePlayed(...) - Found equal " << played[start].numberStr() << " at positions "
                                                                             << start << " and " << j);
                    result.maxEqual = nr;
                    result.maxEqualPos = start;
                    break;
                }
            }
    }
    TRACE3("TwopartRules::analyzePlayed(...) - Trumps: " << result.trumps);
    return result;
}

//-----------------------------------------------------------------------------
/// Searches for the last position of the cards which are in a serie (same
/// colour; number increasing by 1)
/// \param hand Cards to inspect
/// \param start Position to start
/// \returns unsigned int Position of last card in serie
//-----------------------------------------------------------------------------
unsigned int findEndOfSerie(const Card::Cards& hand, unsigned int start) {
    Check1(start < hand.size());
    Value::NUMBERS nr(hand[start].number());
    const Value::COLOURS colour(hand[start].colour());

    while ((++start < hand.size()) && (hand[start].number() == (nr + 1)) && (hand[start].colour() == colour))
        nr = hand[start].number();
    return start - 1;
}

//-----------------------------------------------------------------------------
/// Searches for the first position of the cards which are in a serie (same
/// colour; number decreasing by 1)
/// \param hand Cards to inspect
/// \param start Position to start
/// \returns unsigned int Position of first card in serie
//-----------------------------------------------------------------------------
unsigned int findStartOfSerie(const Card::Cards& hand, unsigned int start) {
    Check1(start < hand.size());
    Value::NUMBERS nr(hand[start].number());
    const Value::COLOURS colour(hand[start].colour());

    while ((--start < hand.size()) && (hand[start].number() == (nr - 1)) && (hand[start].colour() == colour))
        nr = hand[start].number();
    return start + 1;
}

//-----------------------------------------------------------------------------
/// Compares cards by colour (the trumps being the biggest colour) and inside
/// the colour by number
/// \param a Card to compare
/// \param b Card to compare
/// \param trump Colour of trump
/// \returns bool True, if a < b
//-----------------------------------------------------------------------------
bool lessByColourAccTrumps(const Value& a, const Value& b, Value::COLOURS trump) {
    // Order of the colours: The ones following the trump first, the trump last
    auto order([trump](Value::COLOURS colour) { return (static_cast<unsigned int>(colour) - trump + 3) % NUM_PLAYERS; });
    return (a.colour() == b.colour()) ? (a.number() < b.number()) : (order(a.colour()) < order(b.colour()));
}

//-----------------------------------------------------------------------------
/// Sorts the passed cards by colour (the trumps being the biggest colour)
/// \param cards Cards to sort
/// \param trump Colour of trump
//-----------------------------------------------------------------------------
void sortByColourAccTrumps(Card::Cards& cards, Value::COLOURS trump) {
    std::ranges::sort(cards, [trump](const Value& a, const Value& b) { return lessByColourAccTrumps(a, b, trump); });
}

//-----------------------------------------------------------------------------
/// Searches for the card(s) the computer player plays
/// \param player Player to inspect
/// \param hand Cards of the player (part 1: sorted by number; part 2: sorted
///     by colour with trumps last)
/// \param played Played cards
/// \param wonCards Number of cards the player has won so far
/// \param stockSize Number of cards on the stock
/// \param table Actual state of the game
/// \returns std::optional<Play> Cards to play; nothing if the player picks up
///     the played cards (only in part 2)
//-----------------------------------------------------------------------------
std::optional<Play> selectCardsToPlay(unsigned int player, const Card::Cards& hand, const Card::Cards& played,
                                      unsigned int wonCards, unsigned int stockSize, const Table& table) {
    Check1(player < NUM_PLAYERS);
    TRACE5("TwopartRules::selectCardsToPlay(...) - Player " << player);

    if (table.partTwo)
        return selectCardsPartTwo(player, hand, played, table);

    const unsigned int pos(selectCardPartOne(player, hand, played, wonCards, stockSize, table));
    return Play{pos, pos};
}

//-----------------------------------------------------------------------------
/// Searches for the smallest card in the hand (preferring the longest serie)
/// \param hand Cards to inspect (sorted by colour, trumps last)
/// \param trump Colour of trump
/// \returns unsigned int Position of smallest card
//-----------------------------------------------------------------------------
unsigned int findSmallestCard(const Card::Cards& hand, Value::COLOURS trump) {
    Value::NUMBERS nrMin(Value::UNREACHABLE);
    unsigned int cSerie(0);
    unsigned int pos(0);
    for (unsigned int i(0); i < hand.size(); ++i) {
        const Value& card(hand[i]);

        // Stop searching if a trump was found
        if ((card.colour() == trump) && i)
            break;

        if (nrMin >= card.number()) {
            unsigned int endPos(findEndOfSerie(hand, i));
            if ((card.number() == nrMin) && ((endPos - i) <= cSerie))
                continue;

            TRACE8("TwopartRules::findSmallestCard(...) - New smallest card at " << i << "; Cards: " << (endPos - i));
            cSerie = (endPos - i);
            nrMin = card.number();
            pos = i;
            i = endPos - 1;
        }
    }

    TRACE5("TwopartRules::findSmallestCard(...) - Smallest card at " << pos);
    return pos;
}

//-----------------------------------------------------------------------------
/// Finds a bigger (or equal) card, with respect to trumps
/// \param hand Cards to analyze (sorted by number)
/// \param nr Number of card to beat
/// \param trump Colour of trump
/// \returns int Pos to play (or -1, if no card is bigger)
//-----------------------------------------------------------------------------
int findBigger(const Card::Cards& hand, Value::NUMBERS nr, const Trump& trump) {
    // Find first bigger (or equal) card without checking for trumps
    int pos(Card::findFirstEqualOrBigger(hand, nr));

    // Now check if there's a bigger trump
    if (trump && (pos != -1)) {
        unsigned int newPos(pos);
        while (++newPos < hand.size())
            if (hand[newPos].colour() == *trump) {
                pos = newPos;
                break;
            }
    }

    TRACE3("TwopartRules::findBigger(...) - Pos " << pos);
    return pos;
}

} // namespace TwopartRules
