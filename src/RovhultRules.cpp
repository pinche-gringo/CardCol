// PROJECT     : Cardgames
// SUBSYSTEM   : Rovhult
// REFERENCES  :
// TODO        :
// BUGS        :
// AUTHOR      : Markus Schwab
// CREATED     : 9.10.2026
// COPYRIGHT   : Copyright (C) 2002 - 2018, 2024, 2026

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
#include <card/Random.h>

#include "RovhultRules.h"

using Card::Value;

namespace RovhultRules {

namespace {

//-----------------------------------------------------------------------------
/// Checks if the passed card can be played
/// \param nr Number of the card
/// \param played Played cards
/// \param options Special cards
/// \returns bool True, if the card can be played
//-----------------------------------------------------------------------------
bool canPlay(Value::NUMBERS nr, const Card::Cards& played, const Options& options) {
    return checkCard(nr, played, options) == PlayError::NONE;
}

//-----------------------------------------------------------------------------
/// Skips the cards with the passed number, if the card at the passed position
/// has it
/// \param nr Number to skip
/// \param hand Cards to inspect (sorted by number)
/// \param pos Position to start
/// \returns int Position of the first card behind the skipped ones (or -1)
//-----------------------------------------------------------------------------
int skip(Value::NUMBERS nr, const Card::Cards& hand, unsigned int pos) {
    if (hand[pos].number() == nr) {
        pos = Card::findLastEqual(hand, pos) + 1;
        return (pos < hand.size()) ? static_cast<int>(pos) : -1;
    }
    return pos;
}

//-----------------------------------------------------------------------------
/// Checks if there are only special cards in the passed range
/// \param cards Cards to inspect
/// \param start Lower position of cards to inspect
/// \param end Upper position of cards to inspect
/// \returns bool True, if there are only special cards
//-----------------------------------------------------------------------------
bool existOnlySpecialCards(const Card::Cards& cards, unsigned int start, unsigned int end) {
    Check3(start <= end);
    Check3(end < cards.size());

    do {
        if (!isSpecialCard(cards[start].number()))
            return false;
    }
    while (++start <= end);
    return true;
}

//-----------------------------------------------------------------------------
/// Retrieves the minimal and maximal (visible, not special) reserve card of
/// the player, if he has no cards in the hand
/// \param player Player whose cards to analyse
/// \param min Returns the minimal card
/// \param max Returns the maximal card
/// \returns bool true, if cardinfo is available
//-----------------------------------------------------------------------------
bool getPileLimits(const Player& player, Value::NUMBERS& min, Value::NUMBERS& max) {
    if (player.hand.size())
        return false;

    bool cardFound(false);
    for (unsigned int i(0); i < NUM_RESERVE; ++i)
        if (player.topVisible(i)) {
            const Value::NUMBERS nr(player.reserve[i].back().number());
            if (isSpecialCard(nr))
                break;

            if (cardFound)
                max = nr;
            else {
                min = max = nr;
                cardFound = true;
            }
        }
    return cardFound;
}

//-----------------------------------------------------------------------------
/// Finds the cards to play from the hand
/// \param table Actual state of the game
/// \param player Player in turn (having cards in the hand)
/// \param options Special cards
/// \returns Move Move to execute
//-----------------------------------------------------------------------------
Move selectFromHand(const Table& table, unsigned int player, const Options& options) {
    const Card::Cards& hand(table.players[player].hand);
    const Card::Cards& played(table.played);
    Check3(hand.size());

    // Search for minimal card to play; this is either a card equal or
    // bigger or - if no previous card is played or the last card played was
    // a reverse card (7) - the smallest available
    Value::NUMBERS cardMin(Value::THREE);
    if (played.size() && (played.back().number() != options.reverse) && (played.back().number() != Value::TWO))
        cardMin = played.back().number();
    TRACE5("RovhultRules::selectFromHand(...) - Card to beat " << cardMin);

    // Special handling if cards of next player are know: Try to give him the
    // whole pile
    Value::NUMBERS nextMin(Value::TWO), nextMax(Value::TWO);
    int hpPos(-1);
    const int next(nextAvailablePlayer(table, player));
    unsigned int start =
        (played.size() && (next != -1) && getPileLimits(table.players[next], nextMin, nextMax) &&
         (((nextMin > options.reverse) && ((hpPos = Card::find(hand, options.reverse)) != -1) &&
           canPlay(options.reverse, played, options)) ||
          ((((hpPos = Card::findFirstEqualOrBigger(hand, static_cast<Value::NUMBERS>(nextMax + 1))) != -1) &&
            ((hpPos = skip(options.reverse, hand, hpPos)) != -1) && ((hpPos = skip(options.nuke, hand, hpPos)) != -1)) &&
           canPlay(hand[hpPos].number(), played, options))))
            ? hpPos
            : Card::findFirstEqualOrBigger(hand, cardMin);
    TRACE6("RovhultRules::selectFromHand(...) - First matching card at pos " << start);

    unsigned int end(-1U);
    // Check if no matching normal card is found or found card is bigger than
    // the played reverse card (7). If so, use special card instead.
    if ((start == -1U) ||
        (played.size() && ((played.back().number() == options.reverse) && (hand[start].number() > options.reverse)))) {
        TRACE7("RovhultRules::selectFromHand(...) - Ordinary cards don't match -> Searching for special card");

        if (hand[0].number() == Value::TWO)
            start = 0;
        else {
            start = Card::findFirstEqualOrBigger(hand, options.nuke);
            if ((start == -1U) || (hand[start].number() != options.nuke)) {
                TRACE7("RovhultRules::selectFromHand(...) - Can't continue!");
                return {};
            }
        }
    }
    else {
        // If player would continue with a card coming directly before the
        // reverse card (6), but has also a reverse card (7), play that card
        // instead
        if (static_cast<int>(hand[start].number()) == (static_cast<int>(options.reverse) - 1)) {
            if ((end = Card::find(hand, options.reverse, start)) != -1U) {
                TRACE8("RovhultRules::selectFromHand(...) - Exchanging " << hand[start] << " with " << hand[end]);
                start = end;
            }
        }
        else
            // The search does not know about the special meaning of tens, so
            // skip them by yourself, but use a TWO (if available) in case a
            // TEN is/are the last card(s) (or the card after them can't be
            // played; possible if the nuke card is configured below the
            // reverse card)
            if (((end = skip(options.nuke, hand, start)) == -1U) || !canPlay(hand[end].number(), played, options)) {
                if (hand[0].number() == Value::TWO)
                    start = 0;
            }
            else
                start = end;
    }
    TRACE5("RovhultRules::selectFromHand(...) - Playing " << hand[start] << " at pos " << start);

    // Now find the last of equal cards; get rid of all of them if:
    // - they would complete 4
    // - there are are only special cards left
    // - it's not a special card which is
    //     * not the highest card
    //     * it's the first card
    //     * it's a not that high card (up to 9)
    const unsigned int last(Card::findLastEqual(hand, start));
    Check3(last < hand.size());
    end = (((numberOfEqualTopCards(played) + last - start) == 3) ||
           ((start ? existOnlySpecialCards(hand, 0, start - 1) : true) &&
            ((last < (hand.size() - 1)) ? existOnlySpecialCards(hand, last + 1, hand.size() - 1) : true)) ||
           ((!isSpecialCard(hand[start].number())) &&
            ((last != (hand.size() - 1)) || !start || (hand[start].number() < options.nuke))))
              ? last
              : start;
    return {Move::HAND, start, end};
}

//-----------------------------------------------------------------------------
/// Finds the cards to play from the reserve piles
/// \param table Actual state of the game
/// \param player Player in turn (having no cards in the hand)
/// \param options Special cards
/// \returns Move Move to execute
//-----------------------------------------------------------------------------
Move selectFromReserve(const Table& table, unsigned int player, const Options& options) {
    const Player& actPlayer(table.players[player]);

    // Bitfield for lower cards: Bit 0: Cards visible; Bit 1: Normal cards
    int bfLowerCardsInfo(0);

    // Play first visible cards
    for (unsigned int start(0); start < NUM_RESERVE; ++start)
        if (actPlayer.topVisible(start)) {
            const Value& card(actPlayer.reserve[start].back());
            bfLowerCardsInfo |= 0x1;

            // If card can be played: Search for last equal card
            if (canPlay(card.number(), table.played, options)) {
                unsigned int end(start);

                // Don't play all cards, if it is a special card and "normal"
                // cards remain
                if (!(isSpecialCard(card.number()) && (bfLowerCardsInfo & 0x2)))
                    while ((end < (NUM_RESERVE - 1)) && actPlayer.topVisible(end + 1) &&
                           (actPlayer.reserve[end + 1].back().number() == card.number()))
                        ++end;

                TRACE7("RovhultRules::selectFromReserve(...) - Playing visible card " << card << " at pos " << end);
                return {Move::RESERVE, start, end};
            }
        }

    if (bfLowerCardsInfo)
        return {}; // No valid card found

    // No card visible: Play the first
    unsigned int start(0);
    while ((start < NUM_RESERVE) && actPlayer.reserve[start].empty())
        ++start;
    Check3(start < NUM_RESERVE);
    TRACE7("RovhultRules::selectFromReserve(...) - Playing invisible card at pos " << start);
    return {Move::RESERVE, start, start};
}

} // namespace

//-----------------------------------------------------------------------------
/// Checks if the player has any cards left
/// \returns bool True, if there are cards in the hand or on the reserve piles
//-----------------------------------------------------------------------------
bool Player::hasCards() const {
    return hand.size() || std::ranges::any_of(reserve, [](const Card::Cards& pile) { return !pile.empty(); });
}

//-----------------------------------------------------------------------------
/// Returns the (untranslated) message describing the passed error
/// \param error Error to describe
/// \returns const char* Message (to be translated with gettext)
//-----------------------------------------------------------------------------
const char* describe(PlayError error) {
    switch (error) {
    case PlayError::NONE:
        break;
    case PlayError::SMALLER_AFTER_REVERSE:
        return N_("After a %1, the played card must be equal or smaller!");
    case PlayError::EQUAL_OR_BIGGER:
        return N_("The played card must be equal or bigger!");
    case PlayError::VISIBLE_CARDS_FIRST:
        return N_("You must first play the visible cards!");
    case PlayError::INVALID_MOVE:
        return N_("Invalid move");
    }
    return "";
}

//-----------------------------------------------------------------------------
/// Values a card; this ranges from 3 to 9, J, K, A, 2, 10 (respectively the
/// nuke card)
/// \param card Card to value
/// \param options Special cards
/// \returns unsigned int Value representing the card
//-----------------------------------------------------------------------------
unsigned int valueOf(const Value& card, const Options& options) {
    return ((card.number() == Value::TWO) ? Value::ACE + 1 : (card.number() == options.nuke) ? Value::ACE + 2 : card.number());
}

//-----------------------------------------------------------------------------
/// Compares two cards according the rules of Rovhult
/// \param lhs, rhs Cards to compare
/// \param options Special cards
/// \returns int <0, if lhs is smaller; 0 if equal or >0 if bigger
//-----------------------------------------------------------------------------
int compareCards(const Value& lhs, const Value& rhs, const Options& options) {
    return static_cast<int>(valueOf(lhs, options)) - static_cast<int>(valueOf(rhs, options));
}

//-----------------------------------------------------------------------------
/// Checks if the passed number is a special card for the computer player.
/// \param nr Number to check
/// \returns bool True for twos and tens
/// \note The ten is used, even if the nuke card is configured differently
//-----------------------------------------------------------------------------
bool isSpecialCard(Value::NUMBERS nr) { return (nr == Value::TEN) || (nr == Value::TWO); }

//-----------------------------------------------------------------------------
/// Check if played card is valid (equal or bigger) The following cards have
/// special meaning:
///   - 2: Can be played always
///   - reverse (7): The next card must be equal or *smaller*
///   - skip (8): Skips the next player
///   - nuke (10): Clears the staple; the same player can continue
/// \param nr Card to check
/// \param played Cards played so far
/// \param options Special cards
/// \returns PlayError Reason why the card must not be played (or NONE)
//-----------------------------------------------------------------------------
PlayError checkCard(Value::NUMBERS nr, const Card::Cards& played, const Options& options) {
    if ((nr != Value::TWO) && (nr != options.nuke) && played.size()) {
        const Value::NUMBERS lastPlayed(played.back().number());
        if (lastPlayed == options.reverse) {
            if (nr > options.reverse)
                return PlayError::SMALLER_AFTER_REVERSE;
        }
        else if (nr < lastPlayed)
            return PlayError::EQUAL_OR_BIGGER;
    }
    return PlayError::NONE;
}

//-----------------------------------------------------------------------------
/// Checks if the top card of the passed reserve pile can be selected: Hidden
/// cards can only be played, if there are no visible ones
/// \param player Cards of the player
/// \param pile Reserve pile to play from (must not be empty)
/// \returns PlayError Reason why the pile can't be selected (or NONE)
//-----------------------------------------------------------------------------
PlayError checkReserve(const Player& player, unsigned int pile) {
    Check1(pile < NUM_RESERVE);
    Check1(player.reserve[pile].size());
    if (!player.topVisible(pile))
        for (unsigned int i(0); i < NUM_RESERVE; ++i)
            if (player.topVisible(i))
                return PlayError::VISIBLE_CARDS_FIRST;
    return PlayError::NONE;
}

//-----------------------------------------------------------------------------
/// Returns the first reserve pile played, when selecting the passed one: The
/// lower piles having a visible top card with the same number are played too
/// \param player Cards of the player
/// \param pile Selected reserve pile (must not be empty)
/// \returns unsigned int First pile to play
//-----------------------------------------------------------------------------
unsigned int firstPileToPlay(const Player& player, unsigned int pile) {
    Check1(pile < NUM_RESERVE);
    Check1(player.reserve[pile].size());
    const Value::NUMBERS nr(player.reserve[pile].back().number());
    while (pile && player.topVisible(pile - 1) && (player.reserve[pile - 1].back().number() == nr))
        --pile;
    return pile;
}

//-----------------------------------------------------------------------------
/// Checks if the passed move is allowed. Playing a hidden reserve card, which
/// turns out to be invalid, is allowed (the player then takes the played
/// cards)
/// \param table Actual state of the game
/// \param player Player making the move
/// \param move Move to check
/// \param options Special cards
/// \returns PlayError Reason why the move is not allowed (or NONE)
//-----------------------------------------------------------------------------
PlayError checkMove(const Table& table, unsigned int player, const Move& move, const Options& options) {
    Check1(player < NUM_PLAYERS);
    const Player& actPlayer(table.players[player]);

    switch (move.source) {
    case Move::HAND:
        if (actPlayer.hand.empty() || (move.start > move.end) || (move.end >= actPlayer.hand.size()))
            return PlayError::INVALID_MOVE;
        for (unsigned int i(move.start + 1); i <= move.end; ++i)
            if (actPlayer.hand[i].number() != actPlayer.hand[move.start].number())
                return PlayError::INVALID_MOVE;
        return checkCard(actPlayer.hand[move.start].number(), table.played, options);

    case Move::RESERVE:
        if (actPlayer.hand.size() || (move.start > move.end) || (move.end >= NUM_RESERVE) || actPlayer.reserve[move.end].empty())
            return PlayError::INVALID_MOVE;
        if (actPlayer.topVisible(move.end)) {
            if (move.start != firstPileToPlay(actPlayer, move.end))
                return PlayError::INVALID_MOVE;
            return checkCard(actPlayer.reserve[move.end].back().number(), table.played, options);
        }
        return (move.start != move.end) ? PlayError::INVALID_MOVE : checkReserve(actPlayer, move.end);

    case Move::TAKE:
        break;
    }
    return table.played.empty() ? PlayError::INVALID_MOVE : PlayError::NONE;
}

//-----------------------------------------------------------------------------
/// Returns the number of equal cards on top of the played pile
/// \param played Played cards
/// \returns unsigned int Number of equal cards
//-----------------------------------------------------------------------------
unsigned int numberOfEqualTopCards(const Card::Cards& played) {
    if (played.empty())
        return 0;
    return played.size() - Card::findFirstEqual(played, played.size() - 1);
}

//-----------------------------------------------------------------------------
/// Checks which player (after the passed one) has still cards left
/// \param table Actual state of the game
/// \param actPlayer ID of actual player
/// \returns int ID of player or -1 (if none can continue)
//-----------------------------------------------------------------------------
int nextAvailablePlayer(const Table& table, unsigned int actPlayer) {
    for (unsigned int i(1); i < NUM_PLAYERS; ++i) {
        actPlayer = (actPlayer + 1) % NUM_PLAYERS;
        if (table.players[actPlayer].hasCards())
            return actPlayer;
    }
    return -1;
}

//-----------------------------------------------------------------------------
/// Returns the number of cards the hand should be filled up to (from the
/// staple) after a move: 3, or (after a nuke card) 1 if the hand is empty
/// \param lastPlayed Number of the last played card
/// \param handSize Number of cards in the hand
/// \param options Special cards
/// \returns unsigned int Minimal number of cards in the hand (0: no filling)
//-----------------------------------------------------------------------------
unsigned int handSizeAfterMove(Value::NUMBERS lastPlayed, unsigned int handSize, const Options& options) {
    if (lastPlayed != options.nuke)
        return CARDS_IN_HAND;
    return handSize ? 0 : 1;
}

//-----------------------------------------------------------------------------
/// Calculates the consequences of a move (after filling up the hand of the
/// player): If the last 4 cards have the same number or a nuke card was
/// played, the played cards are removed and the player continues (except of
/// course, if he doesn't have any cards left). The game ends, if only one
/// player has cards left
/// \param table Actual state of the game (before removing the played cards)
/// \param player ID of player who played the last card
/// \param options Special cards
/// \returns Turn Consequences of the move
//-----------------------------------------------------------------------------
Turn nextTurn(const Table& table, unsigned int player, const Options& options) {
    Check1(player < NUM_PLAYERS);
    Check1(table.played.size());

    Turn turn;
    const Value::NUMBERS nr(table.played.back().number());
    Check3(numberOfEqualTopCards(table.played) <= 4);
    turn.clearPlayed = (nr == options.nuke) || (numberOfEqualTopCards(table.played) >= 4);
    turn.finished = static_cast<int>(player) != nextAvailablePlayer(table, (player + NUM_PLAYERS - 1) % NUM_PLAYERS);
    turn.next = player;

    if (!turn.clearPlayed || turn.finished) {
        int next(nextAvailablePlayer(table, player));
        Check1(next != -1);
        if (next == -1) { // Can't happen (the game would have ended before)
            turn.loser = player;
            return turn;
        }

        if (nextAvailablePlayer(table, next) == -1) {
            turn.loser = next;
            return turn;
        }

        if (nr == options.skip) {
            turn.skipped = next;
            next = nextAvailablePlayer(table, next);
        }
        turn.next = next;
    }
    return turn;
}

//-----------------------------------------------------------------------------
/// Inserts the card into the hand (sorted by number; after equal cards)
/// \param hand Cards sorted by number
/// \param card Card to insert
//-----------------------------------------------------------------------------
void insertSorted(Card::Cards& hand, const Value& card) {
    hand.insert(std::ranges::upper_bound(hand, card, Card::lessByNumber), card);
}

//-----------------------------------------------------------------------------
/// Deals the cards: Every player gets (one after the other) 2 cards on each
/// reserve pile (the first one hidden) and 1 card for the hand
/// \param staple Cards to deal from (the top card is the last one)
/// \param first Player to deal first to
/// \returns std::array<Player, NUM_PLAYERS> Cards of the players
//-----------------------------------------------------------------------------
std::array<Player, NUM_PLAYERS> deal(Card::Cards& staple, unsigned int first) {
    Check1(staple.size() > (NUM_PLAYERS * NUM_RESERVE * 3));

    std::array<Player, NUM_PLAYERS> players;
    for (unsigned int i(0); i < NUM_PLAYERS; ++i) {
        Player& player(players[(first + i) % NUM_PLAYERS]);
        for (auto& pile : player.reserve) {
            for (unsigned int k(0); k < 2; ++k) {
                pile.push_back(staple.back());
                staple.pop_back();
            }

            insertSorted(player.hand, staple.back());
            staple.pop_back();
        }
    }
    return players;
}

//-----------------------------------------------------------------------------
/// Fills up the hand from the staple til it contains the specified number of
/// cards
/// \param hand Hand to fill up
/// \param staple Cards to take from (the top card is the last one)
/// \param minCards Minimal number of cards the hand should hold
//-----------------------------------------------------------------------------
void fillUp(Card::Cards& hand, Card::Cards& staple, unsigned int minCards) {
    while ((hand.size() < minCards) && staple.size()) {
        insertSorted(hand, staple.back());
        staple.pop_back();
    }
}

//-----------------------------------------------------------------------------
/// Sorts the (top) cards on the reserve piles (ascending by value)
/// \param player Player whose cards should be sorted
/// \param options Special cards
//-----------------------------------------------------------------------------
void sortReserve(Player& player, const Options& options) {
    for (int j(0); j < 2; ++j)
        for (int k(j); k >= 0; --k) {
            Check3(player.reserve[k].size() && player.reserve[k + 1].size());
            if (compareCards(player.reserve[k + 1].back(), player.reserve[k].back(), options) < 0)
                std::swap(player.reserve[k + 1].back(), player.reserve[k].back());
        }
}

//-----------------------------------------------------------------------------
/// Exchanges the cards of a computer player before the game starts: Up to 3
/// times the biggest card in the hand is exchanged with the smallest top card
/// of the reserve piles, if the hand card is bigger. Afterwards the hand and
/// the reserve piles are sorted.
/// \param player Cards of the player (3 in the hand, 2 on every reserve pile)
/// \param options Special cards
//-----------------------------------------------------------------------------
void exchangeCards(Player& player, const Options& options) {
    Check1(player.hand.size() == CARDS_IN_HAND);

    for (unsigned int j(0); j < 3; ++j) {
        unsigned int posPile(0);
        unsigned int posHand(0);

        // Search for smallest card in pile and biggest in hand
        for (unsigned int k(1); k < 3; ++k) {
            if (compareCards(player.reserve[k].back(), player.reserve[posPile].back(), options) < 0)
                posPile = k;

            if (compareCards(player.hand[k], player.hand[posHand], options) > 0)
                posHand = k;
        }

        // and exchange them, if hand is bigger than pile
        if (compareCards(player.reserve[posPile].back(), player.hand[posHand], options) < 0) {
            TRACE3("RovhultRules::exchangeCards(...) - exchanging card " << player.hand[posHand] << " in hand (" << posHand
                                                                         << ") with card on pile " << posPile);
            std::swap(player.reserve[posPile].back(), player.hand[posHand]);
        }
    }

    std::ranges::stable_sort(player.hand, Card::lessByNumber);
    sortReserve(player, options);
}

//-----------------------------------------------------------------------------
/// Counts the moves of the computer players (to prevent endless games),
/// after the counting has been started (by setting the counter to 1). Every
/// 8th move after the 30th the computer plays a random card.
/// \param cEndgame Counter of moves in the endgame (0: not counting)
/// \param player Cards of the player in turn
/// \returns bool True, if the player should play a random card (see selectRandomCard)
//-----------------------------------------------------------------------------
bool playRandomly(unsigned int& cEndgame, const Player& player) {
    if (cEndgame)
        ++cEndgame;
    return (cEndgame > 30) && !(cEndgame & 0x7) && player.hand.size();
}

//-----------------------------------------------------------------------------
/// Finds the next move of a computer player
/// \param table Actual state of the game
/// \param player Player in turn
/// \param options Special cards
/// \returns Move Move to execute
//-----------------------------------------------------------------------------
Move selectMove(const Table& table, unsigned int player, const Options& options) {
    TRACE2("RovhultRules::selectMove(...) - Player " << player);
    Check1(player < NUM_PLAYERS);

    return table.players[player].hand.size() ? selectFromHand(table, player, options) : selectFromReserve(table, player, options);
}

//-----------------------------------------------------------------------------
/// Selects a random card of the hand; if it can't be played, the played
/// cards are taken
/// \param hand Cards of the player (not empty)
/// \param played Played cards
/// \param options Special cards
/// \returns Move Move to execute
//-----------------------------------------------------------------------------
Move selectRandomCard(const Card::Cards& hand, const Card::Cards& played, const Options& options) {
    Check1(hand.size());
    const unsigned int pos(Card::randomNumber(hand.size()));
    return canPlay(hand[pos].number(), played, options) ? Move{Move::HAND, pos, pos} : Move{};
}

} // namespace RovhultRules
