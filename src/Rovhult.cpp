// PROJECT     : Cardgames
// SUBSYSTEM   : Rovhult
// REFERENCES  :
// TODO        :
// BUGS        :
// AUTHOR      : Markus Schwab
// CREATED     : 28.3.2002
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
#include <array>
#include <sstream>
#include <string_view>
#include <typeinfo>
#include <vector>

#include <glibmm/main.h>
#include <glibmm/value.h>

#include <gdkmm/contentprovider.h>
#include <gdkmm/drag.h>
#include <gdkmm/texture.h>

#include <gtkmm/dragsource.h>
#include <gtkmm/droptarget.h>
#include <gtkmm/messagedialog.h>
#include <gtkmm/statusbar.h>

#include <XGP/XDialog.h>

#include <YGP/Check.h>
#include <YGP/ConnMgr.h>
#include <YGP/Trace.h>

#include <CardValue.h>

#include <card/ComputerPlayer.h>
#include <card/Message.h>
#include <card/Random.h>
#include <card/Widget.h>
#include <card/Window.h>

#include "Rovhult.h"

Card::Widget::NUMBERS Rovhult::cardNuke(Card::Widget::TEN);
Card::Widget::NUMBERS Rovhult::cardSkip(Card::Widget::EIGHT);
Card::Widget::NUMBERS Rovhult::cardReverse(Card::Widget::SEVEN);

//-----------------------------------------------------------------------------
/// Defaultconstructor; all widgets are created
/// \param parent Parent widget (box) to display the game in
/// \param statusbar Status bar widget to display information about the game
/// \param cardset Cardset to use
/// \param names Vector of players
/// \param posPlayer Position of player for the server
/// \param mxSerialize Mutex to serialize messages from the server
//-----------------------------------------------------------------------------
Rovhult::Rovhult(Gtk::Box& parent, Gtk::Statusbar& statusbar, Card::Set& cardset, const std::vector<Card::Player*>& player,
                 unsigned int posPlayer, Card::MessageLock& mxSerialize)
    : Game(parent, statusbar, cardset, player, posPlayer, mxSerialize, 16, 20),
      played(Card::IPile::VERY_COMPRESSED, Card::IPile::SHOWFACE), staple(Card::IPile::VERY_COMPRESSED), aExchanged(0),
      remoteTarget(-1U), cEndgame(0), aTableDND(), aHandDND(), aHandData(), aTableData() {
    TRACE9("Rovhult::Rovhult(Gtk::Box& Gtk::Statusbar&, CardSet&, const std::vector<Glib::ustring>&)");

    staple.show();
    attach(staple, 3, 2, 1, 5);
    Check3(cards.size());

    // Show and attach card-piles
    changeNames(player);
    for (unsigned int i(0); i < NUM_PLAYERS; ++i) {
        for (int j(0); j < 3; ++j) {
            players[i].reserve[j].setStyle(Card::IPile::QUITE_COMPRESSED);
            players[i].reserve[j].show();
            attach(players[i].reserve[j], COLS_PLAYER[i] + (j << 1), ROWS_PLAYER[i], 1, 2);
            TRACE9("Rovhult::Rovhult() - Set at: " << COLS_PLAYER[i] + (j << 1) << '/' << ROWS_PLAYER[i]);
        }

        players[i].name.show();
        players[i].name.set_hexpand();
        players[i].name.set_vexpand();
        players[i].name.set_margin(1);
        attach(players[i].name, COLS_PLAYER[i], ROWS_PLAYER[i] + ((i == 2) ? 2 : 5), 5, 1);

        players[i].hand.setStyle(i ? Card::IPile::QUITE_COMPRESSED : Card::IPile::NORMAL);
        players[i].hand.setShowOption(i ? Card::IPile::SHOWBACK : Card::IPile::SHOWFACE);
        players[i].hand.show();
        players[i].hand.set_hexpand();
        players[i].hand.set_vexpand();
        attach(players[i].hand, COLS_PLAYER[i], ROWS_PLAYER[i] + ((i == 2) ? -3 : 3), 5, 2);
        TRACE9("Rovhult::Rovhult() - 2nd set at: " << COLS_PLAYER[i] << '/' << ROWS_PLAYER[i] + ((i == 2) ? -3 : 3));
    }

    resizeCards();
    played.show();
    staple.setShowOption(Card::IPile::SHOWBACK);

    played.set_margin(1);
    attach(played, 7, 7, 5, 6);
}

//-----------------------------------------------------------------------------
/// Destructor
//-----------------------------------------------------------------------------
Rovhult::~Rovhult() {
    TRACE8("Rovhult::~Rovhult()");
    clean();
}

//-----------------------------------------------------------------------------
/// Starts the game
//-----------------------------------------------------------------------------
void Rovhult::start() {
    Game::start();

    setGameStatus(EXCHANGE);
    TRACE1("Rovhult::start() - randomiseCards");
    if (randomiseCardsToPile(staple)) {
        TRACE1("Rovhult::start() - randomiseCards finished");
        dealCards();
        TRACE1("Rovhult::start() - cards dealt");

        aExchanged = 0;

        if (getConnectionMgr().getMode() != YGP::ConnectionMgr::CLIENT) {
            setNextPlayer(Card::randomNumber(4));
            broadcastStartPlayer(currentPlayer());
        }

        cEndgame = 0;
    }
}

//-----------------------------------------------------------------------------
/// Callback after finishing card-exchange
/// \param iCard Offset of card in hand (unused)
//-----------------------------------------------------------------------------
void Rovhult::finishedExchange(unsigned int /*iCard*/) {
    unregisterDND();
    disableHuman();
    setGameStatus(EXCHANGED);

    sortReserve(0);
    players[0].hand.setStyle(Card::IPile::COMPRESSED);

    sendExchangedCards(0);

    exchangeAutoplayerCards();

    players[0].hand.sortByNumber();
    sortReserve(0);
    players[0].hand.setStyle(Card::IPile::COMPRESSED);

    status.pop();
    displayTurn(currentPlayer());

    if ((getConnectionMgr().getMode() == YGP::ConnectionMgr::NONE) || (aExchanged == 0xf)) {
        setGameStatus(PLAYING);
        makeNextMoves();
    }
    else
        status.push(_("Waiting for other player to exchange their cards ..."));
}

//-----------------------------------------------------------------------------
/// Exchanges the cards of the computer-players
//-----------------------------------------------------------------------------
void Rovhult::exchangeAutoplayerCards() {
    if (getConnectionMgr().getMode() != YGP::ConnectionMgr::CLIENT)
        for (unsigned int i(getConnectionMgr().getClients().size() + 1); i < NUM_PLAYERS; ++i) {
            RovhultRules::Player cards(playerCardsOf(i));
            RovhultRules::exchangeCards(cards, options());
            arrangeCards(i, cards);

            Check3(players[i].hand.size() == 3);
            Check3(players[i].reserve[0].size() == 2);
            Check3(players[i].reserve[1].size() == 2);
            Check3(players[i].reserve[2].size() == 2);
            sendExchangedCards(i);
        }
}

//----------------------------------------------------------------------------
/// Broadcasts the exchanged cards to the clients
/// \param player Player whose cards to send
//----------------------------------------------------------------------------
void Rovhult::sendExchangedCards(unsigned int player) {
    // Send starting positions to the clients
    if (getConnectionMgr().getMode() != YGP::ConnectionMgr::NONE) {
        std::ostringstream msg;
        msg << "Exchange=" << players[player].hand[0]->id() << ' ' << players[player].hand[1]->id() << ' '
            << players[player].hand[2]->id() << ' ' << players[player].reserve[0].getTopCard().id() << ' '
            << players[player].reserve[1].getTopCard().id() << ' ' << players[player].reserve[2].getTopCard().id()
            << ";Player=" << ((player + posServer) & 0x3);

        // The server echoes the message also to its sender, which ignores it
        // by the passed player (see applyExchange)
        broadcastMessage(msg.str());
        aExchanged |= (1 << player);
    }
}

//-----------------------------------------------------------------------------
/// Sorts the cards on the reserve piles
/// \param player Player whose cards should be sorted
//-----------------------------------------------------------------------------
void Rovhult::sortReserve(unsigned int player) {
    RovhultRules::Player cards(playerCardsOf(player));
    RovhultRules::sortReserve(cards, options());
    arrangeCards(player, cards);
}

//-----------------------------------------------------------------------------
/// Arranges the cards in the hand and the top cards of the reserve piles of
/// the player as passed
/// \param player Player whose cards should be arranged
/// \param arranged New arrangement of the cards (consisting of the same cards
///     in the hand and on top of the reserve piles)
//-----------------------------------------------------------------------------
void Rovhult::arrangeCards(unsigned int player, const RovhultRules::Player& arranged) {
    playerCards& cards(players[player]);
    Check3(arranged.hand.size() == cards.hand.size());

    std::vector<Card::Widget*> available;
    while (cards.hand.size())
        available.push_back(&cards.hand.removeTopCard());
    for (auto& pile : cards.reserve) {
        Check3(pile.size());
        available.push_back(&pile.removeTopCard());
    }

    auto take([&available](const Card::Value& value) -> Card::Widget& {
        const auto card(std::ranges::find_if(available, [&value](const Card::Widget* c) { return c->id() == value.id(); }));
        Check3(card != available.end());
        Card::Widget& result(**card);
        available.erase(card);
        return result;
    });
    for (const auto& card : arranged.hand)
        cards.hand.setTopCard(take(card));
    for (unsigned int i(0); i < cards.reserve.size(); ++i)
        cards.reserve[i].setTopCard(take(arranged.reserve[i].back()), true);
    Check3(available.empty());
}

//-----------------------------------------------------------------------------
/// Returns the cards of the passed player (for the rules)
/// \param player Player whose cards to return
/// \returns RovhultRules::Player Cards of the player
//-----------------------------------------------------------------------------
RovhultRules::Player Rovhult::playerCardsOf(unsigned int player) const {
    RovhultRules::Player result;
    result.hand = players[player].hand.values();
    for (unsigned int i(0); i < result.reserve.size(); ++i)
        result.reserve[i] = players[player].reserve[i].values();
    return result;
}

//-----------------------------------------------------------------------------
/// Returns the actual state of the game (for the rules)
/// \returns RovhultRules::Table Cards of the players and the played cards
//-----------------------------------------------------------------------------
RovhultRules::Table Rovhult::currentTable() const {
    RovhultRules::Table table;
    for (unsigned int i(0); i < NUM_PLAYERS; ++i)
        table.players[i] = playerCardsOf(i);
    table.played = played.values();
    return table;
}

//-----------------------------------------------------------------------------
/// Enables the cards of the human player
/// \param player Player to enable
//-----------------------------------------------------------------------------
bool Rovhult::enableHuman() {
    Check3(activeCards.empty());

    if (players[0].hand.size()) {
        TRACE2("Rovhult::enableHuman() - Has " << players[0].hand.size() << " card(s) in the hand");

        for (int i(players[0].hand.size() - 1); i >= 0; --i)
            activeCards.push_back(players[0].hand[i]->signal_clicked().connect(bind(mem_fun(*this, &Rovhult::handSelected), i)));
    }
    else {
        TRACE2("Rovhult::enableHuman() - Enable reserve of human");

        for (int i(0); i < 3; ++i)
            if (players[0].reserve[i].size()) {
                TRACE8("Rovhult::enableHuman() - Pile " << i << " has " << players[0].reserve[i].size() << " card(s)");
                activeCards.push_back(
                    players[0].reserve[i].getTopCard().signal_clicked().connect(bind(mem_fun(*this, &Rovhult::pileSelected), i)));
            }
    }

    if (played.size()) {
        TRACE2("Rovhult::enablHuman(unsigned int) - Enable last played card");
        activeCards.push_back(played.getTopCard().signal_clicked().connect(mem_fun(*this, &Rovhult::takeCards)));
    }
    return Game::enableHuman();
}

//-----------------------------------------------------------------------------
/// Callback after clicking on a card on the table
/// \param pile Offset of selected pile
//-----------------------------------------------------------------------------
void Rovhult::pileSelected(unsigned int pile) {
    TRACE1("Rovhult::pileSelected(unsinged int) - Pile " << pile);
    Check3(pile < 3);

    Card::IPile& actPile(players[0].reserve[pile]);
    Card::Widget& card(actPile.getTopCard());
    bool showsFace(card.showsFace());

    // If played from bottom of pile (with invisible cards): Flip card first
    if (!showsFace) {
        if (const RovhultRules::PlayError error(RovhultRules::checkReserve(playerCardsOf(0), pile));
            error != RovhultRules::PlayError::NONE) {
            Gtk::MessageDialog dlg(_(RovhultRules::describe(error)), false, Gtk::MessageType::ERROR);
            dlg.set_title(_("Invalid move"));
            XGP::runModal(dlg);
            return;
        }
        card.showFace();
    }
    TRACE1("Rovhult::pileSelected(unsinged int) - Card " << card);

    if (!cardValid(card.number())) { // If selected card is not valid: Return
        if (!showsFace) {
            played.Card::IPile::append(actPile.removeTopCard());

            // Inform the others about the move
            if (getConnectionMgr().getMode() != YGP::ConnectionMgr::NONE) {
                // Send played card to all clients (if any)
                std::ostringstream msg;
                msg << "Play=" << card.id() << ";Target=" << (pile + 5);
                sendMove(msg.str());
            }
            movePlayedCardsToLoser(0);
        }
        return;
    }

    // Inform the others about the move
    if (getConnectionMgr().getMode() != YGP::ConnectionMgr::NONE) {
        // Send played card to all clients (if any)
        std::ostringstream msg;
        msg << "Play=" << card.id() << ";Target=" << (pile + 1);
        sendMove(msg.str());
    }

    // If face of card was visible: Just go on (as the user knows what he has
    // selected); if not: Continue after GUI update.
    if (showsFace)
        doPileSelected(0, pile);
    else
        Glib::signal_timeout().connect(bind(mem_fun(*this, &Rovhult::doPileSelected), 0, pile), Card::ComputerPlayer::TIMEOUT);
}

//-----------------------------------------------------------------------------
/// Executes the move from a pile: Moves the cards and enables next
/// \param player ID of player
/// \param pile Offset of selected pile
/// \returns bool false
//-----------------------------------------------------------------------------
bool Rovhult::doPileSelected(unsigned int player, unsigned int pile) {
    TRACE1("Rovhult::doPileSelected(unsigned int, unsinged int) - " << player << '/' << pile);
    Check1(player < NUM_PLAYERS);
    Check1(pile < 3);

    Card::IPile* actPile(&players[player].reserve[pile]);
    Check3(actPile->size());
    const unsigned int first(RovhultRules::firstPileToPlay(playerCardsOf(player), pile));
    Card::Widget& card(actPile->getTopCard());
    card.showFace();
    Card::PileWindows& animPiles(animateCards2(played, *actPile, actPile->size() - 1, actPile->size() - 1));
    animPiles.sigAnimation.connect(bind(mem_fun(*this, &Rovhult::executeMove), player));

    while (pile > first) {
        actPile = &players[player].reserve[--pile];
        Card::Widget& sameCard(actPile->getTopCard());
        sameCard.showFace();
        animPiles.addWindow(*actPile, actPile->size() - 1, actPile->size() - 1);
    }

    return false;
}

//-----------------------------------------------------------------------------
/// Check if played card is valid (see RovhultRules::checkCard)
/// \param nr Card to check
/// \param silent Flag, if error should be displayed
/// \returns bool True, if card can be played
//-----------------------------------------------------------------------------
bool Rovhult::cardValid(Card::Widget::NUMBERS nr, bool silent) const {
    TRACE5("Rovhult::cardValid(Card::Widget::NUMBERS, bool) const - Checking " << nr << " in " << played.size() << " cards");

    const RovhultRules::PlayError error(RovhultRules::checkCard(nr, played.values(), options()));
    if (error != RovhultRules::PlayError::NONE) {
        if (!silent) {
            Glib::ustring msg(_(RovhultRules::describe(error)));
            if (error == RovhultRules::PlayError::SMALLER_AFTER_REVERSE)
                msg.replace(msg.find("%1"), 2, CardValue::get()[cardReverse]);

            Gtk::MessageDialog dlg(msg, false, Gtk::MessageType::ERROR);
            dlg.set_title(_("Invalid move"));
            XGP::runModal(dlg);
        }
        return false;
    }
    return true;
}

//-----------------------------------------------------------------------------
/// Callback after clicking on a card in hand
/// \param pos Offset of card in hand
//-----------------------------------------------------------------------------
void Rovhult::handSelected(unsigned int pos) {
    TRACE3("Rovhult::handSelected(unsinged int) - Checking card at " << pos);
    Check3(pos < players[0].hand.size());

    Card::Widget& card(*players[0].hand[pos]);
    TRACE1("Rovhult::handSelected(unsinged int) - Card " << pos << " = " << card);

    if (!cardValid(card.number()))
        return;

    // Inform the others about the move
    unsigned int start(players[0].hand.findFirstEqual(pos));
    if (getConnectionMgr().getMode() != YGP::ConnectionMgr::NONE) {
        // Send played card to all clients (if any)
        std::ostringstream msg;
        msg << "Play=";
        for (unsigned int i(start); i < pos; ++i)
            msg << players[0].hand[i]->id() << ' ';
        msg << players[0].hand[pos]->id() << ";Target=0";

        sendMove(msg.str());
    }

    playCardsFromHand(0, start, pos);
}

//-----------------------------------------------------------------------------
/// Move card (and cards with equal number below) from player to played
/// staple. The cards are replaced, if the staple contains cards
/// \param player ID of player who played the last card
/// \param start Offset of first card in hand to play
/// \param end Offset of last card in hand to play
//-----------------------------------------------------------------------------
void Rovhult::playCardsFromHand(unsigned int player, unsigned int start, unsigned int end) {
    TRACE5("Rovhult::playCardsFromHand(unsigned int, unsigned int, unsigned int)"
           " - Player "
           << player << " from " << start << " to " << end);
    Check3(end < players[player].hand.size());
    Check3(start <= end);
    animateCards(played, players[player].hand, start, end)
        .sigAnimation.connect(bind(mem_fun(*this, &Rovhult::executeMove), player));
}

//-----------------------------------------------------------------------------
/// Unmarks any played card and moves the played cards to the passed player
/// \param player ID of actual player
/// \param start First card to play
/// \param end Last card to play
/// \returns bool False, to stop the time
//-----------------------------------------------------------------------------
bool Rovhult::unmarkAndMoveToLoser(unsigned int player, unsigned int start, unsigned int end) {
    TRACE8("Rovhult::unmarkAndMoveToLoser(3x unsigned int) - Player " << player << "; Cards: " << start << '/' << end);
    for (unsigned int i(start); i <= end; ++i) {
        Card::Widget& card(players[player].reserve[i].removeTopCard());
        card.unmark();
        players[player].hand.setTopCard(card);
    }
    movePlayedCardsToLoser(player);
    return false;
}

//-----------------------------------------------------------------------------
/// Unmarks any played card and executes the move
/// \param player ID of actual player
/// \param count Number of cards to unmark
/// \returns bool False, to stop the time
//-----------------------------------------------------------------------------
void Rovhult::unmarkAndExecuteMove(unsigned int player, unsigned int count) {
    TRACE8("Rovhult::unmarkAndExecuteMove(2x unsigned int) - Player " << player << "; Cards: " << count);
    Check1(count <= played.size());
    while (count)
        played[played.size() - count--]->unmark();
    executeMove(player);
}

//-----------------------------------------------------------------------------
/// Executes the move -> Check consequences for next in round and calculate
/// next player
/// \param player ID of player who played the last card
//-----------------------------------------------------------------------------
void Rovhult::executeMove(unsigned int player) {
    TRACE3("Rovhult::executeMove(unsigned int) - Player " << player);
    Check3(player < NUM_PLAYERS);

    // If staple contains cards and no 10 was played (except if hand is empty):
    // Fill up cards til player has 3 (or one, in case of a ten)
    const RovhultRules::Options opts(options());
    fillUpPile(players[player].hand,
               RovhultRules::handSizeAfterMove(played.getTopCard().number(), players[player].hand.size(), opts));

    const RovhultRules::Turn turn(RovhultRules::nextTurn(currentTable(), player, opts));
    Glib::ustring stat;
    if (turn.clearPlayed) {
        played.clear();
        stat = _("Pile cleared; ");
    }

    // Start counting the moves (to prevent endless games), when the last
    // (local or remote) human finished
    if (turn.finished && !cEndgame && (typeid(*actPlayers[player]) != typeid(Card::ComputerPlayer)) && noMoreHumans())
        cEndgame = 1;

    if (turn.loser != -1) {
        status.pop();
        stat = _("%1 lost");
        stat.replace(stat.find("%1"), 2, actPlayers[turn.loser]->getName());
        status.push(stat);
        setGameStatus(STOPPED);
        return;
    }

    if (turn.skipped != -1) {
        stat = _("Skipping %1; ");
        stat.replace(stat.find("%1"), 2, actPlayers[turn.skipped]->getName());
    }

    player = turn.next;
    Check3(actPlayers.size() > player);
    Check3(actPlayers[player]);
    displayTurn(player, stat);
    setNextPlayer(player);
    makeNextMoves();
}

//-----------------------------------------------------------------------------
/// Callback after selection top card on played pile -> Moves all its card to
/// the passed player
/// \param player ID of player picking up the cards
//-----------------------------------------------------------------------------
void Rovhult::takeCards() {
    TRACE2("Rovhult::takeCards()");

    if (getConnectionMgr().getMode() != YGP::ConnectionMgr::NONE) {
        std::ostringstream msg;
        msg << "Play=" << played.getTopCard().id() << ";Target=4";
        sendMove(msg.str());
    }

    movePlayedCardsToLoser(0);
}

//-----------------------------------------------------------------------------
/// Fills up the passed pile til it contains the specified number of cards
/// \param pile Pile to fill up
/// \param minCards Minimal number of cards pile should hold
//-----------------------------------------------------------------------------
void Rovhult::fillUpPile(Card::IPile& pile, unsigned int minCards) {
    TRACE3("Rovhult::fillUpPile(Card::IPile&, unsinged int) - " << pile.size() << " -> " << minCards);

    while ((pile.size() < minCards) && staple.size())
        pile.insertSorted(staple.removeTopCard());
}

//-----------------------------------------------------------------------------
/// Method to move the cards of the actual round to the winner
/// \param nrLoser Nr. of player getting all played cards
//-----------------------------------------------------------------------------
void Rovhult::movePlayedCardsToLoser(unsigned int nrLoser) {
    TRACE8("Rovhult::movePlayedCardsToLoser() - Player " << nrLoser << " gets " << played.size() << " cards");
    Check3(nrLoser < NUM_PLAYERS);
    Check3(played.size());
    Check3(actPlayers[nrLoser]);

    animateCards(players[nrLoser].hand, played, 0, played.size() - 1)
        .sigAnimation.connect(bind(mem_fun(*this, &Rovhult::cardsTaken), nrLoser));
    Glib::ustring stat(_("%1 can't continue -> Taking the whole pile. "));
    stat.replace(stat.find("%1"), 2, actPlayers[nrLoser]->getName());
    displayTurn(nrLoser = nextAvailablePlayer(nrLoser), stat);
    setNextPlayer(nrLoser);
}

//-----------------------------------------------------------------------------
/// Callback after cards of played pile have been animated toh the hand of
/// a player
/// \param player Player who took the cards
//-----------------------------------------------------------------------------
void Rovhult::cardsTaken(unsigned int player) {
    players[player].hand.sortByNumber();
    makeNextMoves();
}

//-----------------------------------------------------------------------------
/// Checks which player has still cards left
/// \param actPlayer ID of actual player
/// \returns int ID of player or -1 (if none can continue)
//-----------------------------------------------------------------------------
int Rovhult::nextAvailablePlayer(unsigned int actPlayer) const {
    return RovhultRules::nextAvailablePlayer(currentTable(), actPlayer);
}

//-----------------------------------------------------------------------------
/// Remove cards from everything which can hold them and unregister any
/// signals (DND)
//-----------------------------------------------------------------------------
void Rovhult::clean() {
    TRACE8("Rovhult::clean() - Status: " << gameStatus());

    if (gameStatus() == EXCHANGE)
        unregisterDND();

    staple.clear();                // Clear staple
    for (auto& player : players) { // Clear cards of players
        for (auto& reserve : player.reserve)
            reserve.clear();

        player.hand.clear();
    }
    played.clear();
    players[0].hand.setStyle(Card::IPile::NORMAL);
    Game::clean();
}

//-----------------------------------------------------------------------------
/// Prepares the card for drag'n'drop (starting from the table, ending on the
/// hand or ending on the table, starting from the hand)
/// \param card Card to prepare for drag'n'drop
/// \param pile Number of pile on reserve holding card
//-----------------------------------------------------------------------------
void Rovhult::registerTableDND(Card::Widget& card, unsigned int pile) {
    TRACE8("Rovhult::registerTableDND(Card::Widget&, unsigned int) - " << card << " for pile " << pile);
    Check1(pile < 3);
    Check3(gameStatus() == EXCHANGE);

    // Card accepts drops from hand (offering the hand-position as payload)
    // and drags from table (offering its own pile-position as payload)
    Glib::RefPtr<Gtk::DropTarget> dst(Gtk::DropTarget::create(G_TYPE_UINT, Gdk::DragAction::MOVE));
    dst->signal_drop().connect(
        [this, pile](const Glib::ValueBase& value, double, double) -> bool {
            Glib::Value<unsigned int> v;
            v.init(value.gobj());
            return cardDroppedOnTable(v.get(), pile);
        },
        false);
    card.add_controller(dst);
    aTableDND[&card] = dst;

    Glib::RefPtr<Gtk::DragSource> src(Gtk::DragSource::create());
    src->set_actions(Gdk::DragAction::MOVE);
    src->signal_prepare().connect(
        [pile](double, double) -> Glib::RefPtr<Gdk::ContentProvider> {
            Glib::Value<unsigned int> v;
            v.init(Glib::Value<unsigned int>::value_type());
            v.set(pile);
            return Gdk::ContentProvider::create(v);
        },
        false);
    src->signal_drag_begin().connect(
        [&card, src](const Glib::RefPtr<Gdk::Drag>&) { src->set_icon(Gdk::Texture::create_for_pixbuf(card.getImage()), 0, 0); },
        false);
    card.add_controller(src);
    aTableData[&card] = src;
}

//-----------------------------------------------------------------------------
/// Prepares the card for drag'n'drop (starting from the hand ending on table
/// or ending on hand, starting from table)
/// \param card Card to prepare for drag'n'drop
/// \param pile Number of pile on reserve holding card
//-----------------------------------------------------------------------------
void Rovhult::registerHandDND(Card::Widget& card, unsigned int iCard) {
    TRACE8("Rovhult::registerHandDND(Card::Widget&, unsigned int) - " << card << "; pos " << iCard);
    Check3(gameStatus() == EXCHANGE);

    // Card accepts drops from table (offering the table-pile as payload) and
    // drags from hand (offering its own hand-position as payload)
    Glib::RefPtr<Gtk::DropTarget> dst(Gtk::DropTarget::create(G_TYPE_UINT, Gdk::DragAction::MOVE));
    dst->signal_drop().connect(
        [this, iCard](const Glib::ValueBase& value, double, double) -> bool {
            Glib::Value<unsigned int> v;
            v.init(value.gobj());
            return cardDroppedOnHand(v.get(), iCard);
        },
        false);
    card.add_controller(dst);
    aHandDND[&card] = dst;

    Glib::RefPtr<Gtk::DragSource> src(Gtk::DragSource::create());
    src->set_actions(Gdk::DragAction::MOVE);
    src->signal_prepare().connect(
        [iCard](double, double) -> Glib::RefPtr<Gdk::ContentProvider> {
            Glib::Value<unsigned int> v;
            v.init(Glib::Value<unsigned int>::value_type());
            v.set(iCard);
            return Gdk::ContentProvider::create(v);
        },
        false);
    src->signal_drag_begin().connect(
        [&card, src](const Glib::RefPtr<Gdk::Drag>&) { src->set_icon(Gdk::Texture::create_for_pixbuf(card.getImage()), 0, 0); },
        false);
    card.add_controller(src);
    aHandData[&card] = src;

    TRACE9("Rovhult::registerHandDND(Card::Widget&, unsigned int) - Activate: " << activeCards.size() << '/'
                                                                                << activeCards.capacity());
    Check3(activeCards.size() >= iCard);
    sigc::connection conn(card.signal_clicked().connect(bind(mem_fun(*this, &Rovhult::finishedExchange), iCard)));
    if (activeCards.size() > iCard)
        activeCards[iCard] = conn;
    else
        activeCards.push_back(conn);
    TRACE9("Rovhult::registerHandDND(Card::Widget&, unsigned int) - End");
}

//-----------------------------------------------------------------------------
/// Stops the drag'n'drop abilities of the passed card
/// \param card Card to unregister of dnd
//-----------------------------------------------------------------------------
void Rovhult::unregisterDND(Card::Widget& card) const {
    if (const auto iDst(aHandDND.find(&card)); iDst != aHandDND.end())
        card.remove_controller(iDst->second);
    else if (const auto iTableDst(aTableDND.find(&card)); iTableDst != aTableDND.end())
        card.remove_controller(iTableDst->second);

    if (const auto iSrc(aHandData.find(&card)); iSrc != aHandData.end())
        card.remove_controller(iSrc->second);
    else if (const auto iTableSrc(aTableData.find(&card)); iTableSrc != aTableData.end())
        card.remove_controller(iTableSrc->second);
}

//-----------------------------------------------------------------------------
/// Stops the drag'n'drop abilities of the cards of player 0
/// \param card Card to unregister of dnd
//-----------------------------------------------------------------------------
void Rovhult::unregisterDND() {
    TRACE8("Rovhult::unregisterDND() - Status: " << gameStatus());
    Check3(gameStatus() == EXCHANGE);
    Check3(aHandDND.size() == players[0].hand.size());
    Check3(aTableDND.size() == players[0].hand.size());

    for (unsigned int i(0); i < players[0].hand.size(); ++i) {
        Card::Widget& card(*players[0].hand[i]);
        unregisterDND(card);
        Check3(players[0].reserve[i].size() == 2);
        activeCards[i].disconnect();
        disconnectCardInHand(card);
    }

    for (auto& i : players[0].reserve) {
        unregisterDND(i.getTopCard());
        disconnectCardOnTable(i.getTopCard());
    }

    Check3(aHandDND.empty());
    Check3(aHandData.empty());
    Check3(aTableDND.empty());
    Check3(aTableData.empty());
    activeCards.clear();
}

//-----------------------------------------------------------------------------
/// Disconnects the card (in the hand) from every connection hold
//-----------------------------------------------------------------------------
void Rovhult::disconnectCardInHand(const Card::Widget& card) {
    TRACE3("Rovhult::disconnectCardInHand(const Card::Widget&) - " << card);
    Check1(aHandDND.find(&card) != aHandDND.end());
    Check1(aHandData.find(&card) != aHandData.end());

    aHandDND.erase(&card);
    aHandData.erase(&card);
}

//-----------------------------------------------------------------------------
/// Disconnects the card (on the table) from every connection hold
//-----------------------------------------------------------------------------
void Rovhult::disconnectCardOnTable(const Card::Widget& card) {
    TRACE3("Rovhult::disconnectCardOnTable(const Card::Widget&) - " << card);
    Check1(aTableDND.find(&card) != aTableDND.end());
    Check1(aTableData.find(&card) != aTableData.end());

    aTableDND.erase(&card);
    aTableData.erase(&card);
}

//-----------------------------------------------------------------------------
/// Deals the cards
//-----------------------------------------------------------------------------
void Rovhult::dealCards() {
    TRACE9("Rovhult::dealCards()");
    Check3(staple.size() > 36);

    // Show cards on table: For all players put 6 cards on table (only the
    // (upper visible) and 3 (visible ones) in hand
    Card::Cards cards(staple.values());
    const auto dealt(RovhultRules::deal(cards, (NUM_PLAYERS - posServer) & 0x3));
    auto takeFromStaple([this](const Card::Value& value) -> Card::Widget& {
        Card::Widget* card(staple.get(value.id()));
        Check3(card);
        return staple.remove(*card);
    });
    for (unsigned int i(0); i < NUM_PLAYERS; ++i) {
        for (unsigned int j(0); j < players[i].reserve.size(); ++j)
            for (unsigned int k(0); k < 2; ++k) // Set cards on table (the lower one hidden)
                players[i].reserve[j].setTopCard(takeFromStaple(dealt[i].reserve[j][k]), k);

        for (const auto& card : dealt[i].hand) // Put cards (sorted) into hand
            players[i].hand.setTopCard(takeFromStaple(card));
    }

    // Enable drag-and-drop for cards in the hand (of human player)
    for (unsigned int i(0); i < players[0].hand.size(); ++i) {
        registerHandDND(*players[0].hand[i], i);
        registerTableDND(players[0].reserve[i].getTopCard(), i);
    }

    Check3(staple.size());

    status.pop();
    status.push(_("Exchange the cards in your hand with the one on the "
                  "table (with drag and drop) - click on one card to start playing"));
}

//-----------------------------------------------------------------------------
/// Callback after dropping a (hand-)card onto (cards on) table
/// \param handCard Offset of the dragged card in the hand
/// \param pile Number of pile the card was dropped onto
/// \returns bool True, drop accepted
//-----------------------------------------------------------------------------
bool Rovhult::cardDroppedOnTable(unsigned int handCard, unsigned int pile) {
    Check3(pile < 3);

    TRACE1("Rovhult::cardDroppedOnTable(...) - Data = " << handCard << " <-> " << pile);

    Glib::signal_idle().connect(bind(mem_fun(*this, &Rovhult::doSwapCards), pile, handCard));
    return true;
}

//-----------------------------------------------------------------------------
/// Swaps a card in the hand with one (top-card) on the table
/// \param pile Offset of pile whose top-card should be swapped
/// \param card Offset of card in the hand which should be swapped
/// \returns bool Always false
//-----------------------------------------------------------------------------
bool Rovhult::doSwapCards(unsigned int pile, unsigned int card) {
    Card::Widget& cardTable(players[0].reserve[pile].removeTopCard());
    Card::Widget& cardHand(players[0].hand.remove(card));

    TRACE1("Rovhult::doSwapCards(unsigned int, unsigned int) - Exchanging cards " << cardHand.id() << "<->" << cardTable.id());

    activeCards[card].disconnect();
    unregisterDND(cardHand);
    unregisterDND(cardTable);
    disconnectCardInHand(cardHand);
    disconnectCardOnTable(cardTable);

    // Swap cards
    players[0].reserve[pile].setTopCard(cardHand);
    players[0].hand.insert(cardTable, card);

    // Adapt dnd-settigns
    registerHandDND(cardTable, card);
    registerTableDND(cardHand, pile);
    return false;
}

//-----------------------------------------------------------------------------
/// Callback after dropping a (table-)card onto the hand
/// \param tablePile Offset of the pile the dragged card came from
/// \param card Offset the card was dropped onto in the hand
/// \returns bool True, drop accepted
//-----------------------------------------------------------------------------
bool Rovhult::cardDroppedOnHand(unsigned int tablePile, unsigned int card) {
    Check3(card < players[0].hand.size());

    TRACE1("Rovhult::cardDroppedOnHand(...) - Data = " << tablePile << " <-> " << card);

    Glib::signal_idle().connect(bind(mem_fun(*this, &Rovhult::doSwapCards), tablePile, card));
    return true;
}

//-----------------------------------------------------------------------------
/// Shows visually the card the non-human is about to play from a pile
/// \param player Player in question
/// \param pile Pile in question
/// \param invalid Flag, if the card is invalid (can't actually be played)
//-----------------------------------------------------------------------------
void Rovhult::showCardOfPile(unsigned int player, unsigned int pile, bool invalid) const {
    TRACE9("Rovhult::showCardOfPile(2x unsigned int) - " << player << "->" << pile);
    const Card::VPile& actPile(players[player].reserve[pile]);
    Check3(actPile.size());

    Card::Widget& card(actPile.getTopCard());
    ((actPile.size() > 1) || invalid) ? card.mark() : card.showFace();
}

//-----------------------------------------------------------------------------
/// Shows the cards the user is about to play
/// \param player Player in turn
/// \param start First card to play
/// \param end Last card to play
//-----------------------------------------------------------------------------
void Rovhult::showCards2Play(unsigned int player, unsigned int start, unsigned int end) {
    TRACE2("Rovhult::showCards2Play(3x unsigned int) - Player " << player << ": " << start << '/' << end);
    Check1(player < NUM_PLAYERS);
    Check1(start <= end);

    if (players[player].hand.size()) {
        Check1(end < players[player].hand.size());
        flipCards2Play(players[player].hand, start, end);
        animateCards(played, players[player].hand, start, end)
            .sigAnimation.connect(bind(mem_fun(*this, &Rovhult::executeMove), player));
    }
    else {
        Check1(end < players[player].reserve.size());
        Card::IPile& pile(players[player].reserve[start]);
        unsigned int target(end + 1);

        Card::PileWindows* animPiles(nullptr);
        if (cardValid(pile.getTopCard().number(), true)) {
            animPiles = &animateCards2(played, pile, pile.size() - 1, pile.size() - 1);
            animPiles->sigAnimation.connect(bind(mem_fun(*this, &Rovhult::unmarkAndExecuteMove), player, end - start + 1));
        }
        else {
            target += 4;
            Glib::signal_timeout().connect(bind(mem_fun(*this, &Rovhult::unmarkAndMoveToLoser), player, start, end),
                                           Card::ComputerPlayer::TIMEOUT);
        }
        showCardOfPile(player, start, target > 3);

        while (++start <= end) {
            showCardOfPile(player, start, target > 3);
            if (animPiles) {
                Check2(players[player].reserve[start].size() == 2);
                animPiles->addWindow(players[player].reserve[start], 1, 1);
            }
        }

        // Inform the others about the move; like the human (see doPileSelected)
        // the move is identified by the top card of the last pile to play,
        // the receivers add the equal cards on the piles before
        if (getConnectionMgr().getMode() == YGP::ConnectionMgr::SERVER) {
            // Send played card to all clients (if any)
            std::ostringstream msg;
            msg << "Play=" << players[player].reserve[end].getTopCard().id() << ";Target=" << target;
            broadcastMessage(msg.str());
        }
    }
}

//-----------------------------------------------------------------------------
/// Finds an executes the turn of a (computer controled) player
//-----------------------------------------------------------------------------
void Rovhult::makeMove(unsigned int player) {
    TRACE2("Rovhult::makeMove(unsigned int) - Player " << player);

    // Move of a remote player: Execute the received move (its cards have
    // already been flipped by performCommand)
    if (isShowingCardsToPlay()) {
        const unsigned int start(pos1Play), end(pos2Play);
        pos1Play = pos2Play = -1U;
        makeRemoteMove(player, start, end);
        return;
    }

    const RovhultRules::Table table(currentTable());
    const RovhultRules::Move move(RovhultRules::playRandomly(cEndgame, table.players[player])
                                      ? RovhultRules::selectRandomCard(table.players[player].hand, table.played, options())
                                      : RovhultRules::selectMove(table, player, options()));
    TRACE8("Rovhult::makeMove(unsigned int) - Player " << player << "; Card: " << move.end);

    if (move.source != RovhultRules::Move::TAKE) {
        Check3(move.start <= move.end);
        showCards2Play(player, move.start, move.end);
    }
    else {
        if (getConnectionMgr().getMode() == YGP::ConnectionMgr::SERVER) {
            std::ostringstream msg;
            msg << "Play=" << played[played.size() - 1]->id() << ";Target=4";
            broadcastMessage(msg.str());
        }
        movePlayedCardsToLoser(player);
    }
}

//-----------------------------------------------------------------------------
/// Executes the move received from a remote player (as prepared by
/// executeRemoteMove). Like the move of a computer player the game continues
/// by itself after the (animated) move.
/// \param player Player making the move
/// \param start Position of the first card to play (from the hand)
/// \param end Position of the last card to play (from the hand)
//-----------------------------------------------------------------------------
void Rovhult::makeRemoteMove(unsigned int player, unsigned int start, unsigned int end) {
    TRACE2("Rovhult::makeRemoteMove(3x unsigned int) - Player " << player << "; Target " << remoteTarget << "; Cards " << start
                                                                << '/' << end);
    Check1(player < NUM_PLAYERS);

    const unsigned int target(remoteTarget);
    remoteTarget = -1U;

    // Check if the game has been ended in the meantime
    if (gameStatus() != PLAYING) {
        if (gameStatus() == TOSTOP)
            stop();
        return;
    }

    switch (target) {
    case 0: // Cards from the hand
        Check3(start <= end);
        Check3(end < players[player].hand.size());
        playCardsFromHand(player, start, end);
        break;

    case 1: // Top card of a reserve pile (and the equal cards on the piles before)
    case 2:
    case 3:
        doPileSelected(player, target - 1);
        break;

    default: // Pick up the played cards
        Check3(target == 4);
        movePlayedCardsToLoser(player);
    }
}

//-----------------------------------------------------------------------------
/// Shows or hides the cards of the computer player
/// \param open Flag if cards should be shown or hidden
//-----------------------------------------------------------------------------
void Rovhult::playOpen(bool open) {
    Card::IPile::ShowOpt show(open ? Card::IPile::SHOWFACE : Card::IPile::SHOWBACK);

    for (unsigned int i(1); i < NUM_PLAYERS; ++i) {
        players[i].hand.setShowOption(show);
        players[i].hand.setStyle((show == Card::IPile::SHOWFACE) ? Card::IPile::COMPRESSED : Card::IPile::QUITE_COMPRESSED);
    }
}

//-----------------------------------------------------------------------------
/// End the current game as soon as possible
//-----------------------------------------------------------------------------
void Rovhult::end(bool restart) {
    if (gameStatus() == EXCHANGE)
        unregisterDND();

    Game::end(restart);
}

//-----------------------------------------------------------------------------
/// Changes the names of the playing people
/// \param newPlayer Array holding the new player
//-----------------------------------------------------------------------------
void Rovhult::changeNames(const std::vector<Card::Player*>& newPlayer) {
    Game::changeNames(newPlayer);

    for (unsigned int i(0); i < NUM_PLAYERS; ++i)
        players[i].name.set_text(actPlayers[i]->getName());
}

//----------------------------------------------------------------------------
/// Converts a pile-number to the actual pile
/// \param player Actual player
/// \param pile ID of the pile to return
///    - 0: Play from hand
///    - 1 - 3: Play from pile 0 - 2
///    - 4: Pick up played pile
///    - 5 - 7: Tried to play from pile 0 - 2, but failed
/// \returns Card::IPile* Pile corresponding to the passed number or NULL
//----------------------------------------------------------------------------
Card::IPile* Rovhult::getPileOfPlayer(unsigned int player, unsigned int pile) {
    if ((player >= NUM_PLAYERS) || (pile > 7))
        return nullptr;

    TRACE8("Rovhult::getPileOfPlayer(unsigned int, unsigned int) - Player " << player << "; Pile " << pile);

    // The (invalid) card played from a reserve pile (5 - 7) is moved onto
    // the played cards by executeRemoteMove (after checking the card)
    Card::IPile& result(pile ? ((pile == 4)
                                    ? static_cast<Card::IPile&>(played)
                                    : static_cast<Card::IPile&>(players[player].reserve[(pile > 4) ? (pile - 5) : (pile - 1)]))
                             : static_cast<Card::IPile&>(players[player].hand));
    return result.size() ? &result : nullptr;
}

//----------------------------------------------------------------------------
/// Handles the messages the server might send for the Rovhult cardgame
/// \param player ID of player sending the message
/// \param message Message received from the server
/// \returns bool True, if message has been completey processed
//----------------------------------------------------------------------------
bool Rovhult::handleMessage(unsigned int player, const std::string& message) {
    TRACE1("Rovhult::handleMessage(unsigned int player, const std::string&) - " << message << " (" << player << ')');

#ifdef WITH_NETWORK
    const std::string_view cmd(Card::commandOf(message));
    if (cmd == "Exchange")
        return applyExchange(player, message);

    // Moves are only valid while playing; the server accepts them only from
    // the player in turn
    if (cmd == "Play") {
        if (gameStatus() != PLAYING)
            throw YGP::ParseError(N_("Unexpected move!"));
        if ((getConnectionMgr().getMode() == YGP::ConnectionMgr::SERVER) && (player != currentPlayer()))
            throw YGP::ParseError(N_("Move of a player not in turn!"));

        // Moves from a reserve pile and picking up the played cards are
        // specified by the top card of the respective pile (checked here, as
        // flipCards2Play would accept any card of the pile)
        const auto fields(Card::splitMessage(message));
        unsigned long target(0);
        if ((fields.size() >= 2) && (fields[1].key == "Target") && !stringToNumber(target, fields[1].value.c_str()) && target) {
            const Card::IPile* pile(getPileOfPlayer(currentPlayer(), target));
            const auto ids(Card::words(fields[0].value));
            unsigned long id(0);
            if (pile && ((ids.size() != 1) || stringToNumber(id, ids[0].c_str()) || (id != pile->getTopCard().id())))
                throw YGP::ParseError(N_("Invalid card specification!"));
        }
    }
#endif
    return Game::handleMessage(player, message);
}

#ifdef WITH_NETWORK
//----------------------------------------------------------------------------
/// Applies the exchange of the cards of a partner; the message has the form
/// <tt>Exchange=<IDs of the 3 cards in the hand> <IDs of the top cards of the
/// reserve piles 0 - 2>;Player=<position of player (as seen from the
/// server)></tt>
/// \param player ID of player sending the message
/// \param message Received message
/// \returns bool True, as the message has been completey processed
/// \throw YGP::ParseError In case of an invalid message
//----------------------------------------------------------------------------
bool Rovhult::applyExchange(unsigned int player, const std::string& message) {
    TRACE1("Rovhult::applyExchange(unsigned int, const std::string&) - " << message << " (" << player << ')');

    const auto fields(Card::splitMessage(message));
    unsigned long sender(0);
    if ((fields.size() < 2) || (fields[1].key != "Player") || stringToNumber(sender, fields[1].value.c_str()) ||
        (sender >= NUM_PLAYERS))
        throw YGP::ParseError(N_("Invalid player!"));

    // Ignore the own exchange (echoed by the server; maybe even after all
    // exchanges have been received and the game has started)
    if (sender == posServer)
        return true;

    const unsigned int lPlayer((sender - posServer) & 0x3);
    const YGP::ConnectionMgr& cmgr(getConnectionMgr());
    if (((gameStatus() != EXCHANGE) && (gameStatus() != EXCHANGED)) || (aExchanged & (1 << lPlayer)) ||
        ((cmgr.getMode() == YGP::ConnectionMgr::SERVER) && (player != lPlayer)))
        throw YGP::ParseError(N_("Unexpected exchange of cards!"));

    // The cards to exchange: Those in the hand and the top cards of the reserve
    playerCards& exchanging(players[lPlayer]);
    if (exchanging.hand.size() != 3)
        throw YGP::ParseError(N_("Unexpected exchange of cards!"));
    std::vector<Card::Widget*> available;
    for (unsigned int i(0); i < 3; ++i) {
        if (exchanging.reserve[i].size() != 2)
            throw YGP::ParseError(N_("Unexpected exchange of cards!"));
        available.push_back(exchanging.hand[i]);
        available.push_back(&exchanging.reserve[i].getTopCard());
    }

    // Check the received cards, before changing anything
    const auto ids(Card::words(fields[0].value));
    if (ids.size() != available.size())
        throw YGP::ParseError(N_("Invalid card specification!"));

    std::array<Card::Widget*, 6> newCards{};
    for (unsigned int i(0); i < ids.size(); ++i) {
        unsigned long id(0);
        if (stringToNumber(id, ids[i].c_str()))
            throw YGP::ParseError(N_("Invalid card specification!"));

        TRACE8("Rovhult::applyExchange(unsigned int, const std::string&) - " << lPlayer << ": " << id);
        const auto card(std::ranges::find_if(available, [id](const Card::Widget* c) { return c->id() == id; }));
        if (card == available.end())
            throw YGP::ParseError(N_("Card not found!"));
        newCards[i] = *card;
        available.erase(card);
    }

    // Put the first 3 cards into the hand and the others onto the reserve piles
    for (unsigned int i(0); i < 3; ++i) {
        exchanging.hand.removeTopCard();
        exchanging.reserve[i].removeTopCard();
    }
    for (unsigned int i(0); i < 3; ++i) {
        exchanging.hand.setTopCard(*newCards[i]);
        exchanging.reserve[i].setTopCard(*newCards[i + 3], true);
    }
    exchanging.hand.sortByNumber();

    aExchanged |= (1 << lPlayer);
    TRACE2("Rovhult::applyExchange(unsigned int player, const std::string&) - Exchanged: " << std::hex << aExchanged << std::dec);

    // Inform the other clients (the sender ignores the echo)
    if (cmgr.getMode() == YGP::ConnectionMgr::SERVER)
        broadcastMessage(message);

    // Start playing, if every player (including the own) has exchanged
    if ((aExchanged == 0xf) && (gameStatus() == EXCHANGED)) {
        status.pop();
        setGameStatus(PLAYING);
        makeNextMoves();
    }
    return true;
}
#endif

//----------------------------------------------------------------------------
/// Executes the remote move locally
/// \param pile Pile to move to/from
/// \param target ID of target as send by the partner
/// \returns bool True, if the timer to execute the move should be set
//----------------------------------------------------------------------------
bool Rovhult::executeRemoteMove(Card::IPile& pile, unsigned int target) {
    TRACE7("Rovhult::executeRemoteMove(Card::IPile&, unsigned int) - Target " << target);
    Check3(gameStatus() == PLAYING);
    Check3(target <= 7);

    // A (hidden) card played from a reserve pile turned out to be invalid:
    // Put it onto the played cards; the player picks them all up
    if (target > 4) {
        Check3(&pile == &players[currentPlayer()].reserve[target - 5]);
        played.Card::IPile::append(pile.removeTopCard());
        target = 4;
    }

    // The move itself is executed by makeMove (after a delay)
    remoteTarget = target;
    return Game::executeRemoteMove(pile, target);
}

//-----------------------------------------------------------------------------
/// Actions to take when the cards are resized
/// \pre The cardsize must be set in CardImages::WIDTH/HEIGHT
//-----------------------------------------------------------------------------
void Rovhult::resizeCards() {
    for (auto& player : players) {
        for (auto& reserve : player.reserve)
            reserve.set_size_request(Card::Images::WIDTH, Card::Images::HEIGHT + 7);
        player.hand.set_size_request(Card::Images::WIDTH * 3, Card::Images::HEIGHT);
    }

    played.set_size_request(Card::Images::WIDTH, Card::Images::HEIGHT);
    staple.set_size_request(Card::Images::WIDTH, Card::Images::HEIGHT + 50);
}

//-----------------------------------------------------------------------------
/// Checks if there are only computer-player with cards left
/// \returns bool True, if only computer-players are left
//-----------------------------------------------------------------------------
bool Rovhult::noMoreHumans() const {
    int first(nextAvailablePlayer(0));
    int i(first);
    do {
        if (typeid(*actPlayers[i]) != typeid(Card::ComputerPlayer))
            return false;
        i = nextAvailablePlayer(i);
    }
    while (first < i);
    return true;
}
