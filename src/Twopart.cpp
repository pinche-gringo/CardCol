// PROJECT     : Cardgames
// SUBSYSTEM   : Twopart
// REFERENCES  :
// TODO        :
// BUGS        :
// AUTHOR      : Markus Schwab
// CREATED     : 20.7.2002
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
#include <array>
#include <memory>
#include <optional>
#include <sstream>

#include <glibmm/main.h>

#include <gtkmm/box.h>
#include <gtkmm/messagedialog.h>
#include <gtkmm/statusbar.h>

#include <giomm/menu.h>
#include <giomm/simpleaction.h>
#include <giomm/simpleactiongroup.h>

#include <YGP/Check.h>
#include <YGP/ConnMgr.h>
#include <YGP/Trace.h>

#include <XGP/XDialog.h>

#include <card/ComputerPlayer.h>
#include <card/Images.h>
#include <card/Message.h>
#include <card/Player.h>
#include <card/Random.h>
#include <card/Set.h>
#include <card/Widget.h>
#include <card/Window.h>

#include "Twopart.h"

Card::Value::COLOURS Twopart::sortTrump(Card::Value::CLUBS);

//-----------------------------------------------------------------------------
/// Defaultconstructor; all widget are created
/// \param parent Parent widget to display the game in
/// \param statusbar Status bar widget to display information about the game
/// \param cardset Cardset to use
/// \param players Vector of player
/// \param posPlayer Position of player for the server
/// \param mxSerialize Mutex to serialize messages from the server
//-----------------------------------------------------------------------------
Twopart::Twopart(Gtk::Box& parent, Gtk::Statusbar& statusbar, Card::Set& cardset, const std::vector<Card::Player*>& player,
                 unsigned int posPlayer, Card::MessageLock& mxSerialize)
    : Game(parent, statusbar, cardset, player, posPlayer, mxSerialize, 12, 15), table(), pTrump(nullptr),
      played(Card::IPile::COMPRESSED, Card::IPile::SHOWFACE), staple(Card::IPile::VERY_COMPRESSED, Card::IPile::SHOWBACK),
      idxMenu(-1) {
    staple.show();
    staple.set_margin(5);
    staple.set_halign(Gtk::Align::START);
    staple.set_valign(Gtk::Align::START);
    attach(staple, COL_STAPLE, ROW_STAPLE, 1, 1);

    // Show and attach card-piles
    changeNames(player);
    for (unsigned int i(0); i < NUM_PLAYERS; ++i) {
        players[i].name.show();
        players[i].name.set_hexpand();
        players[i].name.set_vexpand();
        players[i].name.set_margin_start(1);
        players[i].name.set_margin_end(1);
        attach(players[i].name, COLS_PLAYER[i], ROWS_PLAYER[i] + (i ? 1 : 3), i ? 3 : 15, 1);
        TRACE9("Twopart::Twopart() - Name at: " << COLS_PLAYER[i] << '/' << ROWS_PLAYER[i] + (i ? 1 : 3));

        players[i].won.show();
        players[i].won.set_margin_start(1);
        players[i].won.set_margin_end(1);
        attach(players[i].won, COLS_PLAYER[i] + (i ? 1 : 3), ROWS_PLAYER[i] + (i ? -2 : 2), (i ? 3 : 15) - (i ? 1 : 3), 1);
        TRACE9("Twopart::Twopart() - Won pile at: " << COLS_PLAYER[i] + 1 << '/' << ROWS_PLAYER[i] + (i ? -2 : 2));

        players[i].hand.show();
        players[i].hand.set_margin_start(1);
        players[i].hand.set_margin_end(1);
        if (!i)
            players[i].hand.set_halign(Gtk::Align::CENTER);
        attach(players[i].hand, COLS_PLAYER[i], ROWS_PLAYER[i], i ? 3 : 15, 1);
        TRACE9("Twopart::Twopart() - Hand at: " << COLS_PLAYER[i] << '/' << ROWS_PLAYER[i]);

        players[i].won.setShowOption(Card::IPile::SHOWBACK);
        players[i].hand.setShowOption(i ? Card::IPile::SHOWBACK : Card::IPile::SHOWFACE);
    }

    played.show();
    played.set_margin_top(5);
    played.set_margin_bottom(5);
    // Remark: Show the cards centered and don't stretch the pile vertically;
    //     else the (cropped) pictures of the compressed cards request more
    //     width, leaving space between them
    played.set_halign(Gtk::Align::CENTER);
    played.set_valign(Gtk::Align::CENTER);
    attach(played, 3, 6, 8, 3);
    resizeCards();
}

//-----------------------------------------------------------------------------
/// Destructor
//-----------------------------------------------------------------------------
Twopart::~Twopart() {
    TRACE9("Twopart::~Twopart()");
    clean();
}

//-----------------------------------------------------------------------------
/// Starts the game
//-----------------------------------------------------------------------------
void Twopart::start() {
    TRACE8("Twopart::start()");
    Game::start();
    if (randomiseCardsToPile(staple)) {
        // Show cards on the table: For all players put 3 cards in hand
        for (unsigned int i(0); i < NUM_PLAYERS; ++i)
            for (unsigned int j(0); j < TwopartRules::CARDS_IN_HAND; ++j)
                players[(i - posServer) % NUM_PLAYERS].hand.insertSorted(staple.removeTopCard());

        const unsigned int startPlayer(table.startPlayer);
        table.reset();
        table.startPlayer = startPlayer;

        for (unsigned int i(1); i < NUM_PLAYERS; ++i) {
            players[i].hand.setStyle(Card::IPile::COMPRESSED);
            players[i].won.setStyle(Card::IPile::VERY_COMPRESSED);
        }
        players[0].hand.setStyle(Card::IPile::NORMAL);
        players[0].won.setStyle(Card::IPile::QUITE_COMPRESSED);

        if (getConnectionMgr().getMode() != YGP::ConnectionMgr::CLIENT) {
            setNextPlayer(table.startPlayer = Card::randomNumber(NUM_PLAYERS));
            broadcastStartPlayer(table.startPlayer);
            displayTurn(currentPlayer());
            makeNextMoves();
        }
    }
}

//-----------------------------------------------------------------------------
/// Enables the cards of the passed player
/// \returns bool Flag, if timer should be continued; False
/// \remarks Depending of the status of the game (PLAYING2) also the top card
///     of the played pile is enabled
//-----------------------------------------------------------------------------
bool Twopart::enableHuman() {
    Check3(activeCards.empty());
    Check3(gameStatus() >= PLAYING);

    TRACE2("Twopart::enableHuman() - Has " << players[0].hand.size() << " cards");
    for (int i(players[0].hand.size() - 1); i >= 0; --i)
        activeCards.push_back(players[0].hand[i]->signal_clicked().connect(bind(mem_fun(*this, (&Twopart::cardSelected)), i)));

    if ((gameStatus() == PLAYING2) && (played.size()))
        activeCards.push_back(played.getTopCard().signal_clicked().connect(mem_fun(*this, (&Twopart::playedSelected))));

    return Game::enableHuman();
}

//-----------------------------------------------------------------------------
/// Moving played cards (of last person) to the passed player
/// \param player ID of player who should get the cards
/// \returns unsigned int Next player
/// \pre Call only in part 2 of the game
//-----------------------------------------------------------------------------
unsigned int Twopart::pickUpPlayedPile(unsigned int player) {
    Check1(player < NUM_PLAYERS);
    Check3(gameStatus() == PLAYING2);

    // Move played cards to player
    const TwopartRules::PickUp pickUp(table.pickUp(player, handSizes()));
    TRACE3("Twopart::pickUpPlayedPile(unsigned int) - Player " << player << " picks up played pile at " << pickUp.start);
    Card::PileWindow* anim;
    anim = &animateCards(players[player].hand, played, pickUp.start, played.size() - 1);
    anim->sigAnimation.connect(bind(mem_fun(*this, &Twopart::endPickup), player));

    Glib::ustring stat(_("%1 can't continue -> Picking up last cards; "));
    Check3(actPlayers.size() > player);
    Check3(actPlayers[player]);
    stat.replace(stat.find("%1"), 2, actPlayers[player]->getName());

    displayTurn(pickUp.next, stat);
    return pickUp.next;
}

//-----------------------------------------------------------------------------
/// Returns the number of cards in the hands of the players
/// \returns TwopartRules::HandSizes Number of cards of each player
//-----------------------------------------------------------------------------
TwopartRules::HandSizes Twopart::handSizes() const {
    TwopartRules::HandSizes sizes{};
    for (unsigned int i(0); i < NUM_PLAYERS; ++i)
        sizes[i] = players[i].hand.size();
    return sizes;
}

//-----------------------------------------------------------------------------
/// Callback after clicking the top card of the played pile
/// \pre Call only in part 2 of the game
//-----------------------------------------------------------------------------
void Twopart::playedSelected() {
    TRACE3("Twopart::playedSelected(unsigned int) - Human picks up played pile - " << played.size());
    Check3(gameStatus() == PLAYING2);
    Check3(table.bfPlayers);

    if (getConnectionMgr().getMode() != YGP::ConnectionMgr::NONE) {
        Check3(table.startPos[table.offPos - 1] < played.size());
        std::ostringstream msg;
        msg << "Play=" << played[table.startPos[table.offPos - 1]]->id() << ";Target=1";
        sendMove(msg.str());
    }
    setNextPlayer(pickUpPlayedPile(0));
    disableHuman();
}

//-----------------------------------------------------------------------------
/// Callback after clicking on a card in hand
/// \param pos Offset of card in hand
//-----------------------------------------------------------------------------
void Twopart::cardSelected(unsigned int pos) {
    TRACE5("Twopart::cardSelected(unsigned int) - Position " << pos);
    Check3(pos < players[0].hand.size());
    Check3(gameStatus() >= PLAYING);

    // Perform validity-check in part 2: Card must have the same colour and be
    // bigger than the last played card or be a (bigger) trump
    unsigned int start(pos);
    if (gameStatus() == PLAYING2) {
        const Card::Cards hand(players[0].hand.values());
        start = TwopartRules::findStartOfSerie(hand, pos);
        const TwopartRules::PlayError error(TwopartRules::checkPlay(hand, start, pos, played.values(), table));
        if (error != TwopartRules::PlayError::NONE) {
            Gtk::MessageDialog dlg(ngettext(TwopartRules::describe(error), TwopartRules::describe(error, true), pos - start + 1),
                                   false, Gtk::MessageType::ERROR);
            dlg.set_title(PACKAGE " - Twopart");
            XGP::runModal(dlg);
            return;
        }
        table.registerPlay(played.size());
        animateCards(played, players[0].hand, start, pos).sigAnimation.connect(bind(mem_fun(*this, &Twopart::endTurn), 0));
    }
    else
        animateCard(played, players[0].hand, pos).sigAnimation.connect(bind(mem_fun(*this, &Twopart::endTurn), 0));

    // Inform the others about the move
    if (getConnectionMgr().getMode() != YGP::ConnectionMgr::NONE) {
        // Send played card to all clients (if any)
        std::ostringstream msg;
        msg << "Play=";
        for (unsigned int i(start); i < pos; ++i)
            msg << players[0].hand[i]->id() << ' ';
        msg << players[0].hand[pos]->id() << ";Target=0";

        sendMove(msg.str());
    }
}

//-----------------------------------------------------------------------------
/// Finish of a turn, called after the card has been animated
/// \param player ID of player
/// \param start Offset of first card to play
/// \param end Offset of last card to play
/// \returns int Next player or -1 at end
//-----------------------------------------------------------------------------
void Twopart::endTurn(unsigned int player) {
    TRACE5("Twopart::endTurn(unsigned int) - Player: " << player);
    Check3(player < NUM_PLAYERS);

    if (staple.size()) {
        Card::Widget& card(staple.removeShownTopCard());
        players[player].hand.insertSorted(card);

        if (!staple.size()) {
            Check3(!pTrump);
            pTrump = std::make_unique<Card::Widget>(card);
            Check3(pTrump);
            table.trump = card.colour();
            staple.hide();
        }
    }

    // Check if every player is still in game or has already played; end round
    // if so or calculate next player if not
    const TwopartRules::TurnResult result(table.endTurn(player, played.values(), handSizes()));
    bool isAnimated(false);
    if (result.endOfRound) {
        // Show trump if not already visible
        // Remark: Check the parent, as new widgets are visible since GTK-4
        if (pTrump && !pTrump->get_parent()) {
            pTrump->showFace();
            pTrump->set_margin(5);
            pTrump->set_halign(Gtk::Align::START);
            pTrump->set_valign(Gtk::Align::START);
            attach(*pTrump, COL_STAPLE, ROW_STAPLE, 1, 1);
        }

        if (gameStatus() == PLAYING2)
            played.clear();
        else if (result.winner >= 0) {
            // Move the played cards to the winner
            setNextPlayer(NUM_PLAYERS - 1);
            animateCards(players[result.winner].won, played, 0, played.size() - 1)
                .sigAnimation.connect(bind(mem_fun(*this, &Twopart::endPickup), result.winner));
            isAnimated = true;
        }
    }

    // Check if the current part is terminated
    if (result.endOfPart) {
        Glib::ustring str;
        player = result.next;
        if (gameStatus() == PLAYING)
            str = _("First part ended; Part 2 starts %1");
        else {
            setGameStatus(STOPPED);
            str = _("%1 lost");
        }
        Check3(actPlayers.size() > player);
        Check3(actPlayers[player]);
        str.replace(str.find("%1"), 2, actPlayers[player]->getName());
        status.pop();
        status.push(str);
    }
    else {
        displayTurn(result.next);
        setNextPlayer(result.next);
        if (!isAnimated)
            makeNextMoves();
    }
}

//-----------------------------------------------------------------------------
/// Makes the move for the next player.
/// \param player Actual player
//-----------------------------------------------------------------------------
void Twopart::makeMove(unsigned int player) {
    TRACE5("Twopart::makeMove() - Turn of player " << player);
    Check3(gameStatus() >= PLAYING);

    // Cards of a remote player are already known (and flipped)
    const bool remote(isShowingCardsToPlay());
    unsigned int start(pos1Play), end(pos2Play);
    if (remote)
        pos1Play = pos2Play = -1U;

    std::optional<TwopartRules::Play> play;
    if (!remote) {
        Check3(player);
        play = TwopartRules::selectCardsToPlay(player, players[player].hand.values(), played.values(), players[player].won.size(),
                                               staple.size(), table);
        if (play) {
            start = play->start;
            end = play->end;
        }
    }

    if (remote || play) {
        if (gameStatus() == PLAYING2)
            table.registerPlay(played.size());

        // Show card(s) to play
        if (!remote)
            flipCards2Play(players[player].hand, start, end);
        animateCards(played, players[player].hand, start, end)
            .sigAnimation.connect(bind(mem_fun(*this, &Twopart::endTurn), player));
    }
    else {
        if ((gameStatus() == PLAYING2) && (getConnectionMgr().getMode() == YGP::ConnectionMgr::SERVER)) {
            std::ostringstream msg;
            Check3(table.startPos[table.offPos - 1] < played.size());
            msg << "Play=" << played[table.startPos[table.offPos - 1]]->id() << ";Target=1";
            broadcastMessage(msg.str());
        }
        setNextPlayer(pickUpPlayedPile(player));
    }
}

//-----------------------------------------------------------------------------
/// Callback after animating the played cards to a player
/// \param receiver Nr. of player getting all played cards
//-----------------------------------------------------------------------------
void Twopart::endPickup(unsigned int receiver) {
    TRACE7("Twopart::endPickup() - " << receiver);
    Check3(receiver < NUM_PLAYERS);
    if (!receiver && (gameStatus() == PLAYING))
        enableWonCards(players[0].won);

    if (gameStatus() == PLAYING2)
        players[receiver].hand.sort(compByColourAccTrumps);
    else {
        table.playedCardsWon();

        if (table.nextPlayer(receiver) < 0) {
            TRACE8("Twopart::endPickup() - Starting part 2");
            Glib::signal_idle().connect(bind(mem_fun(*this, &Twopart::startPartTwo), receiver));
            disableHuman();
            return;
        }
    }
    makeNextMoves();
}

//-----------------------------------------------------------------------------
/// Remove cards from everything which can hold them
//-----------------------------------------------------------------------------
void Twopart::clean() {
    staple.clear();                // Clear staple
    for (auto& player : players) { // Clear cards of players
        player.hand.clear();
        player.won.clear();
    }
    played.clear();

    staple.show();
    if (pTrump) {
        remove(*pTrump);
        pTrump.reset();
    }
    Game::clean();
}

//-----------------------------------------------------------------------------
/// Starts part two of the game
/// \param player Player starting part II
/// \returns int Value indicating if timer should continue
//-----------------------------------------------------------------------------
bool Twopart::startPartTwo(unsigned int player) {
    TRACE9("Twopart::startPartTwo(unsigned int) - Continuing with " << player);
    setGameStatus(PLAYING2);
    disableWonCards();

    // Prepare sorting according to trumps
    Check3(pTrump);
    Check3(table.trump == pTrump->colour());
    sortTrump = pTrump->colour();

    // Now move the cards from the won pile to the hand; if there are
    // players without cards give them the cards up to 5
    std::array<Card::Cards, NUM_PLAYERS> won;
    for (unsigned int i(0); i < NUM_PLAYERS; ++i)
        won[i] = players[i].won.values();
    const TwopartRules::Receivers receivers(table.startPartTwo(won, player)); // Might change player
    for (unsigned int i(0); i < NUM_PLAYERS; ++i)
        for (unsigned int receiver : receivers[i]) {
            Card::Widget& card(players[i].won.removeTopCard());
            TRACE8("Twopart::startPartTwo(unsigned int) - Moving card " << card << " to player " << receiver);
            players[receiver].hand.Card::IPile::append(card);
        }

    // Finally sort and show the cards
    for (unsigned int i(0); i < NUM_PLAYERS; ++i) {
        players[i].hand.sort(compByColourAccTrumps);
        players[i].hand.setStyle(i ? Card::IPile::VERY_COMPRESSED : Card::IPile::COMPRESSED);
    }

    setNextPlayer(player);
    makeNextMoves();
    return false;
}

//-----------------------------------------------------------------------------
/// Compares the cards in the pile with regard of the colour and with special
/// consideration of trumps
/// \param a Card to compare
/// \param b Card to compare
/// \returns bool True, if a < b
//-----------------------------------------------------------------------------
bool Twopart::compByColourAccTrumps(const Card::Widget* a, const Card::Widget* b) {
    Check3(a);
    Check3(b);
    return TwopartRules::lessByColourAccTrumps(*a, *b, sortTrump);
}

//-----------------------------------------------------------------------------
/// Shows or hides the cards of the computer player
/// \param open Flag if cards should be shown or hidden
//-----------------------------------------------------------------------------
void Twopart::playOpen(bool open) {
    Card::IPile::ShowOpt show(open ? Card::IPile::SHOWFACE : Card::IPile::SHOWBACK);

    for (unsigned int i(0); i < NUM_PLAYERS; ++i) {
        players[i].won.setShowOption(show);
        players[i].won.setStyle(open ? Card::IPile::COMPRESSED : Card::IPile::VERY_COMPRESSED);
        if (i)
            players[i].hand.setStyle((gameStatus() == PLAYING2) ? (open ? Card::IPile::COMPRESSED : Card::IPile::VERY_COMPRESSED)
                                                                : Card::IPile::COMPRESSED);
        players[i].hand.setShowOption(i ? show : Card::IPile::SHOWFACE);
    }
}

//-----------------------------------------------------------------------------
/// Changes the names of the playing people
/// \param newPlayer Array holding the new player
//-----------------------------------------------------------------------------
void Twopart::changeNames(const std::vector<Card::Player*>& newPlayer) {
    Game::changeNames(newPlayer);

    for (unsigned int i(0); i < NUM_PLAYERS; ++i)
        players[i].name.set_text(actPlayers[i]->getName());
}

//----------------------------------------------------------------------------
/// Converts a pile-number to the actual pile
/// \param newPlayer Array holding the new player
/// \param pile ID of the pile to return
/// \returns Card::IPile* Pile corresponding to the passed number or NULL
//----------------------------------------------------------------------------
Card::IPile* Twopart::getPileOfPlayer(unsigned int player, unsigned int pile) {
    TRACE8("Twopart::getPileOfPlayer(unsigned int, unsigned int) - Player " << player << "; Pile " << pile);
    if ((player >= NUM_PLAYERS) || (pile > 1))
        return nullptr;

    return &(pile ? played : players[player].hand);
}

//----------------------------------------------------------------------------
/// Handles the messages the server might send for the Twopart cardgame
/// \param player ID of player sending the message
/// \param message Message received from the server
/// \returns bool True, if message has been completey processed
//----------------------------------------------------------------------------
bool Twopart::handleMessage(unsigned int player, const std::string& message) {
    TRACE1("Twopart::handleMessage(unsigned int player, const std::string&) - " << message << " (" << player << ')');
    bool rc(Game::handleMessage(player, message));
#ifdef WITH_NETWORK
    // The client starts playing, after receiving the startplayer
    if (Card::commandOf(message) == "ActPlayer") {
        TRACE1("Twopart::handleMessage(unsigned int player, const std::string&) - Next player: " << currentPlayer());
        table.startPlayer = currentPlayer();
        makeNextMoves();
    }
#endif
    return rc;
}

//----------------------------------------------------------------------------
/// Executes the remote move locally
/// \param pile Pile to move to/from
/// \param target ID of target as send by the partner
/// \pre Expects \c pos1Play and \c pos2Play to be set to the positions to play
//----------------------------------------------------------------------------
bool Twopart::executeRemoteMove(Card::IPile& pile, unsigned int target) {
    Check2(pos1Play != -1U);
    Check2(pos2Play != -1U);
    if (target) {
        TRACE7("Twopart::executeRemoteMove(Card::IPile&, unsigned int) - Target " << target);
        Check3(gameStatus() == PLAYING2);
        pos1Play = pos2Play = -1U;
        setNextPlayer(pickUpPlayedPile(currentPlayer()));
        return false;
    }
    return Game::executeRemoteMove(pile, target);
}

//-----------------------------------------------------------------------------
/// Adds game-specific menus
/// \param menu Top-level menu to add the game's submenu to
/// \param actions Action-group ("win"-scoped) to add the game's actions to
//-----------------------------------------------------------------------------
void Twopart::addMenus(const Glib::RefPtr<Gio::Menu>& menu, const Glib::RefPtr<Gio::SimpleActionGroup>& actions) {
    Check1(menu);
    Check1(actions);

    Glib::RefPtr<Gio::Menu> sub(Gio::Menu::create());
    actions->add_action("TwopartSort", mem_fun(*this, &Twopart::sortWonByNumber));
    sub->append(_("_Sort won cards (by number)"), "game.TwopartSort");
    actions->add_action("TwopartSortCol", mem_fun(*this, &Twopart::sortWonByColour));
    sub->append(_("Sort won cards (by _colour)"), "game.TwopartSortCol");

    idxMenu = menu->get_n_items();
    menu->append_submenu(_("_Twopart"), sub);
}

//-----------------------------------------------------------------------------
/// Removes the game-specific menus
/// \param menu Top-level menu to remove the game's submenu from
/// \param actions Action-group to remove the game's actions from
//-----------------------------------------------------------------------------
void Twopart::removeMenus(const Glib::RefPtr<Gio::Menu>& menu, const Glib::RefPtr<Gio::SimpleActionGroup>& actions) {
    Check1(menu);
    Check1(actions);
    if (idxMenu != -1) {
        menu->remove(idxMenu);
        idxMenu = -1;
    }
    actions->remove_action("TwopartSort");
    actions->remove_action("TwopartSortCol");
}

//-----------------------------------------------------------------------------
/// Actions to take when the cards are resized
/// \pre The cardsize must be set in Card::Images WIDTH/HEIGHT
//-----------------------------------------------------------------------------
void Twopart::resizeCards() {
    played.set_size_request(-1, Card::Images::HEIGHT);
    staple.set_size_request(Card::Images::WIDTH, Card::Images::HEIGHT);
    for (unsigned int i(0); i < NUM_PLAYERS; ++i) {
        players[i].won.set_size_request(Card::Images::WIDTH + 20, Card::Images::HEIGHT + 5);
        players[i].hand.set_size_request(i ? (Card::Images::WIDTH + (2 * 7)) : -1, Card::Images::HEIGHT + 5);
    }
}
