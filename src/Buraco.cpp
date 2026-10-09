// PROJECT     : Cardgames
// SUBSYSTEM   : Buraco
// REFERENCES  :
// TODO        :
// BUGS        :
// AUTHOR      : Markus Schwab
// CREATED     : 24.02.2003
// COPYRIGHT   : Copyright (C) 2003 - 2018, 2024, 2026

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
#include <memory>
#include <ranges>
#include <sstream>

#include <gtk/gtk.h>

#include <glibmm/main.h>
#include <glibmm/value.h>

#include <gdkmm/contentprovider.h>
#include <gdkmm/texture.h>

#include <gtkmm/box.h>
#include <gtkmm/dragsource.h>
#include <gtkmm/droptarget.h>
#include <gtkmm/messagedialog.h>
#include <gtkmm/scrolledwindow.h>
#include <gtkmm/statusbar.h>
#include <gtkmm/window.h>

#include <giomm/menu.h>
#include <giomm/simpleaction.h>
#include <giomm/simpleactiongroup.h>

#include <XGP/XDialog.h>

#include <YGP/ANumeric.h>
#include <YGP/AttrParse.h>
#include <YGP/Check.h>
#include <YGP/ConnMgr.h>
#include <YGP/Trace.h>

#include <card/ComputerPlayer.h>
#include <card/Human.h>
#include <card/Images.h>
#include <card/Message.h>
#include <card/ScoreDlg.h>
#include <card/Window.h>

#include "Buraco.h"

unsigned int Buraco::ENDPOINTS(2000);
unsigned int Buraco::CARDS2DEAL(11);

namespace {
/// Projection returning the raw pointer of a table-pile (for std::ranges algorithms)
[[maybe_unused]] constexpr auto getPtr([](const std::unique_ptr<BuracoPile>& pile) { return pile.get(); });
} // namespace

//-----------------------------------------------------------------------------
/// Constructor
/// \param parent Parent widget to display the game in
/// \param statusbar Status bar widget to display information about the game
/// \param cardset Cardset to use
/// \param player Vector of player
/// \param posPlayer Position of player for the server
/// \param mxSerialize Mutex to serialize messages from the server
//-----------------------------------------------------------------------------
Buraco::Buraco(Gtk::Box& parent, Gtk::Statusbar& statusbar, Card::Set& cardset, const std::vector<Card::Player*>& player,
               unsigned int posPlayer, Card::MessageLock& mxSerialize)
    : Game(parent, statusbar, cardset, player, posPlayer, mxSerialize, 3, 10), nameTeams(), startPlayer(-1U), frameInfo(), info(),
      newPile(_("New pile")), staple(Card::IPile::TOTALLY_COMPRESSED, Card::IPile::SHOWBACK),
      dumped(Card::IPile::TOTALLY_COMPRESSED, Card::IPile::SHOWFACE), dumpedTop(), stapleTop(), aDNDHand(), aDNDTable(),
      gStatus(), undo(), pScoreDlg(nullptr), idxMenu(-1), menuUndo(), menuSort(), menuSort2(), menuShowScoreDlg(), target(-1U) {
    TRACE9("Buraco::Buraco(Box&, Statusbar&, Card::Set&, const std::vector<Glib::ustring>&)");

    for (unsigned int i(0); i < NUM_TEAMS; ++i) {
        scrlTable[i].set_child(boxTeam[i]);
        scrlTable[i].set_policy(Gtk::PolicyType::AUTOMATIC, Gtk::PolicyType::AUTOMATIC);
        scrlTable[i].show();
    }

    TRACE9("Buraco::Buraco(Box&, Statusbar&, Card::Set&, const std::vector<Glib::ustring>&) - Init common staples");

    newPile.set_margin(5);
    newPile.set_hexpand();
    newPile.set_vexpand();
    boxTeam[0].append(newPile);

    for (unsigned int i(1); i < NUM_PLAYERS; ++i) {
        hands[i].set_hexpand();
        hands[i].set_margin(5);
        attach(hands[i], (3 - i) << 2, 0, 2, 1);
        names[i].set_hexpand();
        attach(names[i], (3 - i) << 2, 1, 2, 1);
        hands[i].show();
        names[i].show();
    }
    hands[0].setStyle(Card::IPile::COMPRESSED);
    hands[0].setShowOption(Card::IPile::SHOWFACE);
    hands[0].show();
    names[0].show();

    TRACE9("Buraco::Buraco(Box&, Statusbar&, Card::Set&, const std::vector<Glib::ustring>&) - Attach widgets");
    hands[0].set_hexpand();
    hands[0].set_margin_start(1);
    hands[0].set_margin_end(1);
    hands[0].set_margin_top(5);
    hands[0].set_margin_bottom(5);
    attach(hands[0], 3, 4, 7, 1);
    names[0].set_hexpand();
    names[0].set_margin_start(1);
    names[0].set_margin_end(1);
    names[0].set_margin_top(5);
    names[0].set_margin_bottom(5);
    attach(names[0], 3, 5, 7, 1);
    staple.set_margin_start(5);
    staple.set_margin_end(5);
    attach(staple, 0, 4, 1, 2);
    dumped.set_margin_start(1);
    dumped.set_margin_end(1);
    dumped.set_margin_top(5);
    dumped.set_margin_bottom(5);
    attach(dumped, 1, 4, 1, 2);
    scrlTable[1].set_hexpand();
    scrlTable[1].set_vexpand();
    scrlTable[1].set_margin_top(5);
    scrlTable[1].set_margin_bottom(5);
    attach(scrlTable[1], 0, 2, 10, 1);
    scrlTable[0].set_hexpand();
    scrlTable[0].set_vexpand();
    scrlTable[0].set_margin_top(5);
    scrlTable[0].set_margin_bottom(5);
    attach(scrlTable[0], 0, 3, 10, 1);

    TRACE9("Buraco::Buraco(Box&, Statusbar&, Card::Set&, const std::vector<Glib::ustring>&) - Show widgets");
    newPile.show();
    staple.show();
    dumped.show();
    boxTeam[0].show();
    boxTeam[1].show();

    // Remark: Gtk::Statusbar is a plain Gtk::Widget under GTK4 (no longer a
    // box that can hold extra children), so the info-frame is appended as a
    // sibling to the statusbar into its (horizontal) parent container instead.
    frameInfo.set_margin(5);
    frameInfo.set_halign(Gtk::Align::END);
    frameInfo.set_hexpand(false);
    info.show();
    frameInfo.set_child(info);
    frameInfo.show();
    if (Gtk::Box* boxStatus = dynamic_cast<Gtk::Box*>(statusbar.get_parent()))
        boxStatus->append(frameInfo);

    changeNames(player);
    resizeCards();
}

//-----------------------------------------------------------------------------
/// Destructor
//-----------------------------------------------------------------------------
Buraco::~Buraco() {
    TRACE9("Buraco::~Buraco()");
    clean();
    if (Gtk::Box* boxStatus = dynamic_cast<Gtk::Box*>(frameInfo.get_parent()))
        boxStatus->remove(frameInfo);
    delete pScoreDlg;

    // Free team names
    for (auto& nameTeam : nameTeams)
        delete nameTeam;
}

//-----------------------------------------------------------------------------
/// Removes a cerrado from the table
/// \param team Team to inspect
/// \remarks As every move can only make one cerrado; only the first is
///     removed.
//-----------------------------------------------------------------------------
void Buraco::cleanCerrado(unsigned int player) {
    Check1(player < NUM_PLAYERS);

    for (const auto& p : tablePiles[player & 1]) {
        Check3(p);
        Check3(p->size() <= 7);
        if ((p->size() == 7) && p->get_visible()) {
            removeCerrado(player, *p);
            return;
        }
    }
}

//-----------------------------------------------------------------------------
/// Makes the move for the next player.
/// \param player Actual player
//-----------------------------------------------------------------------------
void Buraco::makeMove(unsigned int player) {
    TRACE5("Buraco::makeMove(unsigned int) - Turn of player " << player);
    Check1(player);
    Check1(player < NUM_PLAYERS);

    if (cleanup()) // Cleanup; end if game has been finished
        return;

    // Turn back jokers
    if (undo.pickUp) {
        unsigned int oldPlayer(gStatus.startTurn ? ((player - 1) & 0x3) : player);
        Card::HPile* pile(&hands[oldPlayer]);

        TRACE5("Buraco::makeMove(unsigned int) - Player with monos: " << oldPlayer);
        if (oldPlayer && (pile->size() > CARDS2DEAL)) {
            Check3(pile->size() > CARDS2DEAL);
            showJoker(pile, pile->size() - CARDS2DEAL, false);
        }
        undo.pickUp = 0;
    }

    if (gStatus.startTurn && (gameStatus() == STOPPED))
        return;
    playCards();
}

//-----------------------------------------------------------------------------
/// Cleanup of piles after each turn
/// \returns bool True, if game has been ended
//-----------------------------------------------------------------------------
bool Buraco::cleanup() {
    unsigned int player(currentPlayer());
    if (gStatus.startTurn)
        player = (player + NUM_PLAYERS - 1) & 0x3;
    TRACE5("Buraco::cleanup() - " << player);

    // First cleanup cerrado made in the last turn
    cleanCerrado(player);

    Card::IPile& source(hands[player]);
    if (gStatus.pickUpPlayed) {
        Check3(dumped.size());
        gStatus.pickUpPlayed = 0;
        source.getCards(dumped);
        hands[player].sort(compByNumberWithJokers);
    }

    switch (BuracoRules::handStatus(source.values(), !reserve[player & 1].empty())) {
    case BuracoRules::HandStatus::TAKE_RESERVE:
        addBuraco(player);
        break;

    case BuracoRules::HandStatus::GOING_OUT:
        Check3(points[player & 1] > 100);
        points[player & 1] += BuracoRules::GOING_OUT_BONUS;
        endGame();
        return true;

    case BuracoRules::HandStatus::PLAYING:
        break;
    }
    return false;
}

//-----------------------------------------------------------------------------
/// Searches for cards to play and shows them in the hand of the
/// actual player. They are moved to the end and then animated to its target
//-----------------------------------------------------------------------------
void Buraco::playCards() {
    unsigned int player(currentPlayer());
    TRACE2("Buraco::playCards() - " << player << " (" << gStatus.startGame << '/' << gStatus.startTurn << ')');
    Check1(gameStatus() == PLAYING);
    Check1(!hands[player].empty());

    if (gStatus.startTurn && (((player & 1) ? gStatus.team2Buraco : gStatus.team1Buraco) == (player >> 1)))
        ((player & 1) ? gStatus.team2Buraco : gStatus.team1Buraco) = 0x3;

    Card::IPile& playerPile(hands[player]);
    if (gStatus.startTurn) {
        Check3(dumped.size());
        gStatus.startTurn = 0;

        Card::Widget& dumpedCard(dumped.getTopCard());
        if (BuracoRules::takeDumped(makeTable(), player, dumpedCard)) {
            if (getConnectionMgr().getMode() != YGP::ConnectionMgr::NONE) {
                // Send played card to all clients (if any)
                std::ostringstream msg;
                msg << "Play=" << dumpedCard.id() << ";Target=3";

                sendMove(msg.str());
            }

            if (!gStatus.startGame) {
                BuracoRules::Table table(makeTable());
                if (dumped.size() > 1)
                    gStatus.pickUpPlayed = 1;

                // Arrange the cards to play (with the taken card) at the end of the hand.
                // Remark: The taken card stays on the dumped cards (for the animation)
                const BuracoRules::PickUp cards(BuracoRules::playPickedUp(table, player, dumpedCard));
                arrangeHand(player, table.hands[player]);
                unsigned int pos1Play(cards.first), pos2Play(cards.last);
                const unsigned int posTarget(cards.posTaken);
                TRACE1("posTarget: " << posTarget);
                Check3((pos2Play - pos1Play) >= 1);

                // The partners receive the picked up card (as card in the hand; see above), the
                // cards of the hand to play to the new pile and then the picked up card to add to it
                const unsigned int newPos(tablePiles[player & 1].size() << 16);
                target = newPos;
                flipCards2Play(playerPile, pos1Play, pos2Play);
                if (getConnectionMgr().getMode() == YGP::ConnectionMgr::SERVER) {
                    std::ostringstream msg;
                    msg << "Play=" << dumpedCard.id() << ";Target=" << newPos + posTarget + 100;
                    broadcastMessage(msg.str());
                }

                BuracoPile& newPile(makeNewPile(player & 1));
                Card::PileWindows& win(animateCards2(newPile, playerPile, pos1Play, pos2Play));
                win.sigAnimation.connect(mem_fun(*this, &Buraco::makeNextMoves));
                win.addWindow(posTarget, dumped, dumped.size() - 1, dumped.size() - 1);
                return;
            }
            playerPile.insertSorted(dumped.removeTopCard(), compByNumberWithJokers);
        }
        else {
            if (getConnectionMgr().getMode() != YGP::ConnectionMgr::NONE) {
                // Send played card to all clients (if any)
                std::ostringstream msg;
                msg << "Play=" << staple.getTopCard().id() << ";Target=2";

                sendMove(msg.str());
            }

            playerPile.insertSorted(staple.removeTopCard(), compByNumberWithJokers);
        }

        gStatus.startGame = 0;
        dumpedCard.show();
    }

    target = executeMove(player, pos1Play, pos2Play);
    TRACE1("Buraco::playCards() - " << pos1Play << '/' << pos2Play << " -> " << std::hex << target << std::dec);

    flipCards2Play(playerPile, pos1Play, pos2Play);
    Check3(pos1Play <= pos2Play);
    Check3(pos2Play < hands[player].size());
    if (target != -1U)
        animateCards(*tablePiles[player & 1][target >> 16], target & 0xff, playerPile, pos1Play, pos2Play)
            .sigAnimation.connect(mem_fun(*this, &Buraco::makeNextMoves));
    else {
        Check3(pos1Play == pos2Play);
        endTurn(player, pos1Play);
    }
}

//-----------------------------------------------------------------------------
/// Ends the actual turn by dumping a card
/// \param player Number of player ending the turn
/// \param card2Dump Offset of card to dump in th ehand of th ecurrent player
//-----------------------------------------------------------------------------
void Buraco::endTurn(unsigned int player, unsigned int card2Dump) {
    Check1(player < NUM_PLAYERS);
    Card::IPile& playerPile(hands[player]);
    Check1(playerPile.size() > card2Dump);

    gStatus.startTurn = 1;
    animateCard(dumped, playerPile, card2Dump).sigAnimation.connect(mem_fun(*this, &Buraco::turnEnded));

    ++player &= 0x3;
    displayTurn(player);
    setNextPlayer(player);
}

//-----------------------------------------------------------------------------
/// Actions after the card to end the turn has been dumped: Cleans up the
/// cards of the player who ended the turn (removing a cerrado, giving him the
/// reserve or ending the game, if he has no cards left) and activates the next
/// player.
/// \remarks The cleanup must be done here (and not only when the next player
///     starts his turn), as the next player might be a remote player, whose
///     moves are only received.
//-----------------------------------------------------------------------------
void Buraco::turnEnded() {
    TRACE8("Buraco::turnEnded() - Next player: " << currentPlayer());
    if ((gameStatus() != PLAYING) || !cleanup())
        makeNextMoves();
}

//-----------------------------------------------------------------------------
/// Executes a move for the passed player; only one move is made at every
/// timer-iteration
/// \param player Actual player
/// \param pos1Play First card to play
/// \param pos2Play Last card to play
/// \returns unsigned int ID for target (32 Bit: Pile << 16 + Position); -1U
///     if the card at \c pos1Play should be dumped
//-----------------------------------------------------------------------------
unsigned int Buraco::executeMove(unsigned int player, unsigned int& pos1Play, unsigned int& pos2Play) {
    TRACE6("Buraco::executeMove(player) - " << player);
    const unsigned int team(player & 1);

    BuracoRules::Table table(makeTable());
    const BuracoRules::Move move(BuracoRules::selectMove(table, player));

    // Perform the changes the computer player made to prepare the move
    arrangeHand(player, table.hands[player]);
    for (const auto& m : move.jokerMoves) {
        Check3(m.pile < tablePiles[team].size());
        sendMoveCard(m.pile, m.from, m.to);
        tablePiles[team][m.pile]->move(m.to, m.from);
    }
    unfinishedMonoPiles[team] = table.unfinishedMonoPiles[team];

    pos1Play = move.first;
    pos2Play = move.last;
    switch (move.kind) {
    case BuracoRules::Move::NEW_PILE:
        Check3(move.pile == tablePiles[team].size());
        makeNewPile(team);
        [[fallthrough]];

    case BuracoRules::Move::ADD_TO_PILE:
        return (move.pile << 16) + move.pos;

    case BuracoRules::Move::DUMP:
        break;
    }
    return -1U;
}

//-----------------------------------------------------------------------------
/// Starts the game by dealing the cards
//-----------------------------------------------------------------------------
void Buraco::start() {
    TRACE9("Buraco::start()");
    Game::start();

    if (pScoreDlg) {
        unsigned int player;
        int points;
        pScoreDlg->getMaxPoints(points, player);
        if (points >= static_cast<int>(ENDPOINTS)) {
            menuShowScoreDlg->set_enabled(false);
            delete pScoreDlg;
            pScoreDlg = nullptr;
        }
    }

    if (randomiseCardsToPile(staple)) {
        for (unsigned int i(1); i < NUM_PLAYERS; ++i) {
            hands[i].setStyle(Card::IPile::QUITE_COMPRESSED);
            hands[i].setShowOption(Card::IPile::SHOWBACK);
        }
        for (unsigned int i(0); i < NUM_PLAYERS; ++i)
            hands[(i - posServer) & 0x3].getCards(staple, staple.size() - BuracoRules::cardsInHand(CARDS2DEAL),
                                                  staple.size() - 1);

        for (unsigned int i(0); i < reserve.size(); ++i)
            for (unsigned int j(0); j < CARDS2DEAL; ++j)
                reserve[(i - posServer) & 1].push_back(&staple.removeTopCard());

        for (auto& hand : hands)
            hand.sort(compByNumberWithJokers);

        dumped.setTopCard(staple.removeTopCard());

        gStatus.startTurn = gStatus.startGame = 1;
        gStatus.team1Buraco = gStatus.team2Buraco = 0x3;
        gStatus.pickUpPlayed = 0;

        points[0] = points[1] = 0;
        unfinishedMonoPiles[0] = unfinishedMonoPiles[1] = 0;
        updateInfo();

#ifdef WITH_NETWORK
        movedPile = -1U;

        // A client waits for the server to send the startplayer (see handleMessage)
        if (getConnectionMgr().getMode() == YGP::ConnectionMgr::CLIENT) {
            dumped.getTopCard().hide();
            return;
        }
#endif

        // Set random startplayer (if not already set)
        if (startPlayer == -1U)
            startPlayer = BuracoRules::startPlayer();
        setStartPlayer();
    }
}

//----------------------------------------------------------------------------
/// Sets the startplayer; including showing it in the status bar
/// \param player Player to start the game
//----------------------------------------------------------------------------
void Buraco::setStartPlayer() {
    if (getConnectionMgr().getMode() != YGP::ConnectionMgr::CLIENT) {
        setNextPlayer(startPlayer);
        broadcastStartPlayer(startPlayer);
    }

    if (startPlayer)
        dumped.getTopCard().hide();
    else
        dumped.getTopCard().show();
    displayTurn(startPlayer++);
    startPlayer &= 0x3;
    makeNextMoves();
}

//-----------------------------------------------------------------------------
/// Remove cards from everything which can hold them
//-----------------------------------------------------------------------------
void Buraco::clean() {
    TRACE9("Buraco::clean()");
    disableHuman();
    for (auto& hand : hands)
        hand.clear();

    staple.clear();
    dumped.clear();

    for (unsigned int i(0); i < NUM_TEAMS; ++i) {
        for (const auto& p : tablePiles[i])
            boxTeam[i].remove(*p);
        tablePiles[i].clear();
    }

    for (auto& i : reserve)
        i.clear();

    Game::clean();
}

//-----------------------------------------------------------------------------
/// Enables the cards the human can pick up.
/// \returns bool False
//-----------------------------------------------------------------------------
bool Buraco::enableHuman() {
    Check3(stapleTop.empty());
    Check3(dumpedTop.empty());

    if (gameStatus() == PLAYING)
        cleanup();

    if (staple.size())
        stapleTop = staple.getTopCard().signal_clicked().connect(mem_fun(*this, &Buraco::stapleSelected));
    if (dumped.size())
        dumpedTop = dumped.getTopCard().signal_clicked().connect(mem_fun(*this, &Buraco::dumpedSelected));
    Check3(stapleTop.connected());
    Check3(dumpedTop.connected());
    return false;
}

//-----------------------------------------------------------------------------
/// Enables the cards in the hand of the human player
//-----------------------------------------------------------------------------
void Buraco::enableHumanHand() {
    TRACE2("Buraco::enableHumanHand() - Human has " << hands[0].size() << " cards");
    Check1(activeCards.empty());
    Check1(gameStatus() == PLAYING);

    Check3(hands[0].size());
    for (unsigned int i(0); i < hands[0].size(); ++i) {
        registerHandDND(i);
        enableCard(i);
    }
    Check3(aDNDHand.size() == hands[0].size());

    {
        Glib::RefPtr<Gtk::DropTarget> dropNew(Gtk::DropTarget::create(G_TYPE_UINT, Gdk::DragAction::MOVE));
        dropNew->signal_drop().connect(sigc::bind(mem_fun(*this, &Buraco::cardDroppedOnTable), -1U), false);
        newPile.add_controller(dropNew);
        aDNDTable[nullptr] = dropNew;
    }

    for (unsigned int i(0); i < tablePiles[0].size(); ++i) {
        Check3(tablePiles[0][i]);
        for (unsigned int j(0); j < tablePiles[0][i]->size(); ++j)
            registerTableDND(*(*tablePiles[0][i])[j], (i << 8) + j);
    }

    menuSort->set_enabled();
    menuSort2->set_enabled();
}

//-----------------------------------------------------------------------------
/// Disables the cards the human player can select
/// This method must not assume that cards are activated
//-----------------------------------------------------------------------------
void Buraco::disableHuman() {
    TRACE2("Buraco::disableHuman() - DND: " << aDNDHand.size() << "; " << aDNDTable.size());
    Game::disableHuman();
    menuSort->set_enabled(false);
    menuSort2->set_enabled(false);

    if (aDNDHand.size())
        for (auto& i : hands[0])
            unregisterHandDND(*i);
    Check3(aDNDHand.empty());

    if (aDNDTable.size()) {
        for (auto& i : tablePiles[0]) {
            Check3(i);
            for (unsigned int j(0); j < i->size(); ++j)
                unregisterTableDND(*(*i)[j]);
        }
        newPile.remove_controller(aDNDTable[nullptr]);
        aDNDTable.erase(nullptr);
    }
    Check3(aDNDTable.empty());

    if (dumpedTop.connected())
        dumpedTop.disconnect();
    if (stapleTop.connected())
        stapleTop.disconnect();
}

//-----------------------------------------------------------------------------
/// Callback after clicking on a card in the hand
/// \param iCard Offset of card in hand
//-----------------------------------------------------------------------------
void Buraco::cardSelected(unsigned int iCard) {
    TRACE5("Buraco::cardSelected(unsigned int) - Position " << iCard);
    Check1(iCard < hands[0].size());
    Check1(gameStatus() == PLAYING);
    gStatus.startGame = 0;

    // Check if all piles are valid
    if (const BuracoRules::PlayError error(BuracoRules::checkDump(makeTable(), 0)); error != BuracoRules::PlayError::NONE) {
        showInvalidMove(error);
        return;
    }

    if (getConnectionMgr().getMode() != YGP::ConnectionMgr::NONE) {
        // Send played card to all clients (if any)
        std::ostringstream msg;
        msg << "Play=" << hands[0][iCard]->id() << ";Target=1";

        sendMove(msg.str());
    }

    // If the player has no more cards left (except of jokers) he gets the
    // reserve (or the game ends) after the card has been dumped (see turnEnded).
    // Remark: This can't be done here, as the dumped card is still in the hand
    // (until the animation ends), which would also move it.
    // The whole hand is disabled now (before the reserve is added in turnEnded)
    disableHuman();
    animateCard(dumped, hands[0], iCard).sigAnimation.connect(mem_fun(*this, &Buraco::turnEnded));
    menuUndo->set_enabled(false);

    gStatus.startTurn = 1;
    setNextPlayer(1);
    displayTurn(1);
}

//-----------------------------------------------------------------------------
/// Callback after clicking on the staple
//-----------------------------------------------------------------------------
void Buraco::stapleSelected() {
    TRACE5("Buraco::stapleSelected()");
    Check3(staple.size());
    Check3(stapleTop.connected());

    // Move top card to human and enable the cards in his hand, when idle
    // (means: *after* this signalhandler terminates)
    Glib::signal_idle().connect(bind_return(mem_fun(*this, &Buraco::doStapleSelected), false));
}

//-----------------------------------------------------------------------------
/// Delayed callback after clicking on the staple
//-----------------------------------------------------------------------------
void Buraco::doStapleSelected() {
    TRACE5("Buraco::doStapleSelected()");
    Check2(staple.size());
    Check3(!stapleTop.connected());
    Check3(!dumpedTop.connected());

    dumpedTop.disconnect();
    stapleTop.disconnect();

    dumpedTop.disconnect();
    stapleTop.disconnect();

    if (gameStatus() != STOPPED) {
        if (getConnectionMgr().getMode() != YGP::ConnectionMgr::NONE) {
            // Send played card to all clients (if any)
            std::ostringstream msg;
            msg << "Play=" << staple.getTopCard().id() << ";Target=2";

            sendMove(msg.str());
        }

        unsigned int player(currentPlayer());
        Card::Window& win(animateCard(hands[player], staple, staple.size() - 1));
        if (!player)
            win.sigAnimation.connect(mem_fun(*this, &Buraco::enableHumanHand));
    }
    else {
        dumped.Card::IPile::append(staple.removeTopCard());
        enableHuman();
    }
}

//-----------------------------------------------------------------------------
/// Callback after clicking on the dumped staple
//-----------------------------------------------------------------------------
void Buraco::dumpedSelected() {
    TRACE5("Buraco::dumpedSelected()");
    Check3(dumped.size());
    Check3(stapleTop.connected());
    Check3(dumpedTop.connected());
    Check2(!gStatus.pickUpPlayed);
    disableHuman();

    // Move top card to human and enable the cards in his hand, when idle
    // (means: *after* this signalhandler terminates)
    Glib::signal_idle().connect(bind_return(mem_fun(*this, &Buraco::doDelayedDumpedSelected), false));
}

//-----------------------------------------------------------------------------
/// Handles the user clicking the top card on the dumped staple
/// This is intened to be called after the callback has been de-registered to
/// to prevent side-effects caused by timing-issues
//-----------------------------------------------------------------------------
void Buraco::doDelayedDumpedSelected() {
    TRACE5("Buraco::doDelayedDumpedSelected()");
    disableHuman();

    if (gameStatus() != STOPPED) {
        Card::IPile* target(nullptr);
        Card::Widget& card(dumped.getTopCard());
        if (const BuracoRules::PlayError error(BuracoRules::checkPickUp(makeTable(), 0, card));
            error != BuracoRules::PlayError::NONE) {
            showInvalidMove(error);
            enableHuman();
            return;
        }

        if (getConnectionMgr().getMode() != YGP::ConnectionMgr::NONE) {
            // Send played card to all clients (if any)
            std::ostringstream msg;
            msg << "Play=" << card.id() << ";Target=3";

            sendMove(msg.str());
        }

        // Special handling of player starting the game and can choose one of the
        // first two cards
        if (gStatus.startGame) {
            Check3(dumped.size() == 1);
            target = &hands[0];
            card.show();
        }
        else {
            Check3(BuracoRules::pileHasFittingPair(hands[0].values(), card, -1U));

            if (getConnectionMgr().getMode() != YGP::ConnectionMgr::NONE) {
                // Send played card to all clients (if any)
                std::ostringstream msg;
                msg << "Play=" << card.id() << ";Target=" << (tablePiles[0].size() << 16) + 100;

                sendMove(msg.str());
            }

            // Create new pile with picked up card
            target = &makeNewPile(0);
            if (dumped.size() > 1)
                gStatus.pickUpPlayed = 1;
        }
        Check2(target);
        Card::Window& win(animateCard(*target, dumped, dumped.size() - 1));
        win.sigAnimation.connect(mem_fun(*this, &Buraco::enableHumanHand));
    }
    else {
        staple.Card::IPile::append(dumped.removeTopCard());
        enableHuman();
    }
}

//-----------------------------------------------------------------------------
/// Action after picking up the card from the dumped staple
//-----------------------------------------------------------------------------
void Buraco::doDumpedSelected() {
    TRACE5("Buraco::doDumpedSelected() - " << gStatus.startGame);
    Check3(dumped.size());
    Check3(gameStatus() == PLAYING);
    unsigned int player(currentPlayer());

    dumped.getTopCard().show();
    hands[player].getCards(dumped);
}

//-----------------------------------------------------------------------------
/// Enables a card in the hand of the player
//-----------------------------------------------------------------------------
void Buraco::enableCard(unsigned int pos) {
    TRACE9("Buraco::enableCard(unsigned int) - Enabling card " << pos);
    Check1(pos < hands[0].size());

    activeCards.push_back(hands[0][pos]->signal_clicked().connect(bind(mem_fun(*this, &Buraco::cardSelected), pos)));
}

//-----------------------------------------------------------------------------
/// Prepares the card for drag'n'drop
/// \param iCard Number of card in hand
//-----------------------------------------------------------------------------
void Buraco::registerHandDND(unsigned int iCard) {
    Check1(iCard < hands[0].size());
    TRACE9("Buraco::registerHandDND(unsigned int) - Card: " << iCard << " (" << *hands[0][iCard] << " = " << hands[0][iCard]
                                                            << ')');

    Card::Widget& card(*hands[0][iCard]);
    Check3(aDNDHand.find(&card) == aDNDHand.end());

    // Card accepts drops from hand and can be dragged itself
    Glib::RefPtr<Gtk::DropTarget> drop(Gtk::DropTarget::create(G_TYPE_UINT, Gdk::DragAction::MOVE));
    drop->signal_drop().connect(sigc::bind(mem_fun(*this, &Buraco::cardDropped), iCard), false);
    card.add_controller(drop);

    Glib::RefPtr<Gtk::DragSource> src(Gtk::DragSource::create());
    src->set_actions(Gdk::DragAction::MOVE);
    src->set_icon(Gdk::Texture::create_for_pixbuf(card.getImage()), 0, 0);
    src->signal_prepare().connect(sigc::bind(mem_fun(*this, &Buraco::prepareHandDrag), iCard), false);
    card.add_controller(src);

    aDNDHand[&card].drag = src;
    aDNDHand[&card].drop = drop;
}

//-----------------------------------------------------------------------------
/// Stops the drag'n'drop abilities of the passed card
/// \param card Card to unregister of dnd
//-----------------------------------------------------------------------------
void Buraco::unregisterHandDND(Card::Widget& card) {
    TRACE9("Buraco::unregisterHandDND(Card::Widget&) - Card: " << card << " -> Address: " << &card);
    Check1(aDNDHand.size());

    auto i(aDNDHand.find(&card));
    Check1(i != aDNDHand.end());

    card.remove_controller(i->second.drop);
    card.remove_controller(i->second.drag);
    aDNDHand.erase(i);
}

//-----------------------------------------------------------------------------
/// Prepares the passed region of cards for drag'n'drop
/// \param pile Pile whose cards should be registered
/// \param start Number of first card to prepare for DND
/// \param end Number of last card to prepare for DND
/// \pre \c start < \c end; \c end <= Number of cards
//-----------------------------------------------------------------------------
void Buraco::registerTableDND(unsigned int pile, unsigned int start, unsigned int end) {
    TRACE9("Buraco::registerTableDND(unsigned int, unsigned int, unsigned int)" << " - " << pile << '[' << start << '-' << end
                                                                                << ']');
    Check1(pile < tablePiles[0].size());
    Check1(start <= end);
    Check1(end < tablePiles[0][pile]->size());

    Card::IPile& tmp(*tablePiles[0][pile]);
    pile <<= 8;
    for (; start <= end; ++start) {
        Card::Widget& card(*tmp[start]);
        unregisterTableDND(card);
        registerTableDND(card, pile + start);
    }
}

//-----------------------------------------------------------------------------
/// Prepares the card for drag'n'drop
/// \param card Card to register
/// \param nr Number of card in pile
//-----------------------------------------------------------------------------
void Buraco::registerTableDND(Card::Widget& card, unsigned int nr) {
    TRACE9("Buraco::registerTableDND(Card::Widget&, unsigned int) - " << card << " = " << std::hex << nr << " - " << &card
                                                                      << std::dec);

    // Card accepts drops from hand
    Glib::RefPtr<Gtk::DropTarget> drop(Gtk::DropTarget::create(G_TYPE_UINT, Gdk::DragAction::MOVE));
    drop->signal_drop().connect(sigc::bind(mem_fun(*this, &Buraco::cardDroppedOnTable), nr), false);
    card.add_controller(drop);
    aDNDTable[&card] = drop;
}

//-----------------------------------------------------------------------------
/// Stops the drag'n'drop abilities of the passed card
/// \param card Card to de-register
//-----------------------------------------------------------------------------
void Buraco::unregisterTableDND(Card::Widget& card) {
    TRACE9("Buraco::unregisterTableDND(unsigned int) - Card: " << card << " - " << &card);
    Check1(aDNDTable.size() > 1);

    auto i(aDNDTable.find(&card));
    Check1(i != aDNDTable.end());

    card.remove_controller(i->second);
    aDNDTable.erase(i);
}

//-----------------------------------------------------------------------------
/// Callback after dropping a card (within the hand)
/// \param pContext Context of the drag (contains things like source,
///    target, action, ...)
/// \param data Describes the thing which was dropped
/// \param info Describes the type of data (should be 0)
/// \param time Timestamp of the drag
/// \param card Number of card where something was dropped at
/// \pre \c pContext not NULL; Expects \c info to be 0
//-----------------------------------------------------------------------------
bool Buraco::cardDropped(const Glib::ValueBase& value, double, double, unsigned int card) {
    Check3(card < hands[0].size());

    Glib::Value<guint> v;
    v.init(value.gobj());
    unsigned int valueDropped(v.get());
    Check3(valueDropped < hands[0].size());
    TRACE1("Buraco::cardDropped(...) - Inserting card " << valueDropped << " at pos " << card);

    Card::Widget& cardMoved(hands[0].remove(valueDropped));
    hands[0].insert(cardMoved, card); // Insert moved card

    // Adapt dnd-settigns
    if (valueDropped < card) {
        unsigned int temp(card);
        card = valueDropped;
        valueDropped = temp;
    }

    // DND within hand directly after picking up the buraco disables undoing
    if (undo.pickUp)
        menuUndo->set_enabled(false);

    Glib::signal_idle().connect(bind(mem_fun(*this, &Buraco::doRegisterHand), card, valueDropped));
    return true;
}

//-----------------------------------------------------------------------------
/// Prepares the passed region of cards for drag'n'drop
/// \param except Pile which can be invalid
/// \returns bool True, if the piles are OK
//-----------------------------------------------------------------------------
bool Buraco::doRegisterHand(unsigned int first, unsigned int last) {
    TRACE9("Buraco::doRegisterHand(unsigned int, unsigned int) - [" << first << '-' << last);
    Check1(last < hands[0].size());
    Check1(first <= last);

    registerHandDND(first, last);
    Check3(aDNDHand.size() == hands[0].size());
    return false;
}

//-----------------------------------------------------------------------------
/// Callback after dropping a card on the table
/// \param pContext Context of the drag (contains things like source,
///     target, action, ...)
/// \param data Describes the thing which was dropped
/// \param info Describes the type of data (should be 0)
/// \param time Timestamp of the drag
/// \param iCard Combination of card and pile on which card was dropped
/// \pre \c pContext not NULL;
//-----------------------------------------------------------------------------
bool Buraco::cardDroppedOnTable(const Glib::ValueBase& value, double, double, unsigned int iCard) {
    TRACE1("Buraco::cardDroppedOnTable(...) - Card dropped on " << std::hex << iCard << std::dec);

    Glib::Value<guint> v;
    v.init(value.gobj());
    unsigned int valueDropped(v.get());
    TRACE1("Buraco::cardDroppedOnTable(...) - Inserting card " << valueDropped << " in pile");
    Check3(valueDropped < hands[0].size());

    Card::Widget& moved(*hands[0][valueDropped]);
    TRACE4("Buraco::cardDroppedOnTable(...) - Card dropped: " << moved);

    // Move dropped card to a (new) pile on the table
    const BuracoRules::Table table(makeTable());
    unsigned int iPile;
    BuracoPile* pile(nullptr);
    if (iCard == -1U) { // If card was dropped on the new pile: Create pile
        if (const BuracoRules::PlayError error(BuracoRules::checkNewPile(table, 0, valueDropped));
            error != BuracoRules::PlayError::NONE) {
            showInvalidMove(error);
            return false;
        }

        iPile = tablePiles[0].size();
        pile = &makeNewPile(0);
        Check3(tablePiles[0].size());
        iCard = 0;
    }
    else {
        // Else check pile to use
        Check1((iCard >> 8) < tablePiles[0].size());
        iPile = iCard >> 8;
        if (const BuracoRules::PlayError error(BuracoRules::checkAddToPile(table, 0, valueDropped, iPile));
            error != BuracoRules::PlayError::NONE) {
            showInvalidMove(error);
            return false;
        }
        pile = tablePiles[0][iPile].get();

        if (BuracoRules::startsMonoPile(table.piles[0][iPile], moved))
            ++unfinishedMonoPiles[0];
    }

    // End old drag
    activeCards[valueDropped].disconnect();
    activeCards.erase(activeCards.begin() + valueDropped);

    // Unregister old card
    hands[0].remove(valueDropped);
    unregisterHandDND(moved);

    // Insert card into pile and register it for DND
    unsigned int move(-1U);
    pile->getPosition4Card(moved, iCard, move);
    Check3(iCard <= pile->size());

    TRACE4("Buraco::cardDroppedOnTable(...) - Undo:  " << iPile << "; " << iCard << "; " << valueDropped << ": "
                                                       << (gStatus.pickUpPlayed ? dumped.size() : 0) << '/'
                                                       << pile->getPosJoker());
    undo.assign(iPile, iCard, valueDropped);

    if (move != -1U) {
        Check3(move <= pile->size());
        Check3(move != pile->getPosJoker());
        Check3(pile->getPosJoker() != 7);
        undo.monoPos = pile->getPosJoker();
        sendMoveCard(iPile, pile->getPosJoker(), move);
        pile->move(move, pile->getPosJoker());
    }

    // Send move
    if (getConnectionMgr().getMode() != YGP::ConnectionMgr::NONE) {
        std::ostringstream msg;
        msg << "Play=" << moved.id() << ";Target=" << (iPile << 16) + iCard + 100;
        sendMove(msg.str());
    }

    pile->insert(moved, iCard);
    registerTableDND(moved, (iPile << 8) + iCard);
    if (iCard < (pile->size() - 1))
        registerTableDND(iPile, iCard + 1, pile->size() - 1);

    // Remove pile, if it contains 7 cards
    if (pile->size() == 7)
        removeCerrado(0, *pile);

    // Accept again the jokers, if the pile has has now three cards (jokers are
    // disabled, if the human picked up the dumped pile.
    unsigned int size(hands[0].size());
    if ((pile->size() == 3) && gStatus.pickUpPlayed) {
        hands[0].getCards(dumped);
        menuUndo->set_enabled(false);
        gStatus.pickUpPlayed = 0;

        for (unsigned int i(size); i < hands[0].size(); ++i) {
            enableCard(i);
            registerHandDND(i);
        }
    }
    else
        menuUndo->set_enabled();

    if (valueDropped < size)
        registerHandDND(valueDropped, size - 1);

    // If the player has no more cards left (except of joker): Give him the reserve
    if (BuracoRules::pilesComplete(makeTable().piles[0])) {
        switch (BuracoRules::handStatus(hands[0].values(), !reserve[0].empty())) {
        case BuracoRules::HandStatus::TAKE_RESERVE:
            Glib::signal_idle().connect(bind_return(mem_fun(*this, &Buraco::addBuraco4HumanAndEnable), false));
            return true;

        case BuracoRules::HandStatus::GOING_OUT:
            points[0] += BuracoRules::GOING_OUT_BONUS;
            endGame();
            return true;

        case BuracoRules::HandStatus::PLAYING:
            break;
        }
    }
    Check3(aDNDHand.size() == hands[0].size());
    return true;
}

//-----------------------------------------------------------------------------
/// Adds the buraco to the human and re-enables his cards
//-----------------------------------------------------------------------------
void Buraco::addBuraco4HumanAndEnable() {
    disableHuman();
    addBuraco(0);
    enableHumanHand();
}

//-----------------------------------------------------------------------------
/// Supplies the content to drag when a hand-card's drag starts
/// \param cardPos Position of card in hand
/// \returns Glib::RefPtr<Gdk::ContentProvider> Content carrying cardPos
//-----------------------------------------------------------------------------
Glib::RefPtr<Gdk::ContentProvider> Buraco::prepareHandDrag(double, double, unsigned int cardPos) {
    Glib::Value<guint> v;
    v.init(G_TYPE_UINT);
    v.set(cardPos);
    return Gdk::ContentProvider::create(v);
}

//-----------------------------------------------------------------------------
/// Prepares the passed region of cards for drag'n'drop
/// \param start Number of first card to prepare for DND
/// \param end Number of last card to prepare for DND
/// \pre \c start < \c end; \c end <= Nr. ofcards
//-----------------------------------------------------------------------------
void Buraco::registerHandDND(unsigned int start, unsigned int end) {
    TRACE5("Buraco::registerHandDND(unsigned int, unsigned int) - [" << start << '-' << end << "] of " << activeCards.size());
    Check1(start <= end);
    Check1(end < hands[0].size());
    Check1(end < activeCards.size());

    for (; start <= end; ++start) {
        TRACE9("Buraco::registerHandDND(unsigned int, unsigned int) - Handling card " << start);

        activeCards[start].disconnect();
        activeCards[start] = hands[0][start]->signal_clicked().connect(bind(mem_fun(*this, &Buraco::cardSelected), start));

        unregisterHandDND(*hands[0][start]);
        registerHandDND(start);
    }
    TRACE9("Buraco::registerHandDND(unsigned int, unsigned int) - End");
}

//-----------------------------------------------------------------------------
/// Adds the buraco to the passed player.
/// \param player Player getting the reserve
//-----------------------------------------------------------------------------
void Buraco::addBuraco(unsigned int player) {
    TRACE3("Buraco::addBuraco(unsigned int) - " << player);
    Check1(player < NUM_PLAYERS);

    undo.pickUp = 1;
    unsigned int cJokers(hands[player].size());

    // Storing player picking up the buraco
    ((player & 1) ? gStatus.team2Buraco : gStatus.team1Buraco) = (player >> 1);

    // Add reserve
    std::ranges::sort(reserve[player & 1], compByNumberWithJokers);

    for (auto* card : std::views::reverse(reserve[player & 1]))
        hands[player].insert(*card, 0);
    reserve[player & 1].clear();

    Check3(actPlayers.size() > player);
    Check3(actPlayers[player]);

    // If the computer-player had jokers left, show them
    if (player && cJokers)
        Glib::signal_idle().connect(bind(sigc::ptr_fun(&Buraco::showJoker), &hands[player], cJokers, true));

    status.pop();
    Glib::ustring stat(_("%1 picked up the burraco"));
    stat.replace(stat.find("%1"), 2, actPlayers[player]->getName());
    status.push(stat);
    updateInfo();
}

//-----------------------------------------------------------------------------
/// Shows or hides the joker, which are displayed when picking up the buraco
/// \param pile Pile holding the jokers shown
/// \param cJokers Numer of jokers shown
/// \param show Flag if the jokers should be shown or hidden
/// \returns bool false
//-----------------------------------------------------------------------------
bool Buraco::showJoker(Card::IPile* pile, unsigned int cJokers, bool show) {
    TRACE9("Buraco::showJoker(Card::IPile*, unsigned int, bool) - " << cJokers);
    Check1(cJokers);
    Check1((cJokers + CARDS2DEAL) <= pile->size());

    Card::IPile::PileStyle opt(show ? Card::IPile::COMPRESSED : Card::IPile::VERY_COMPRESSED);
    for (Card::IPile::iterator i(pile->begin() + CARDS2DEAL); i != pile->end(); ++i) {
        show ? (*i)->showFace() : (*i)->showBack();
        pile->resize(**i, opt);
    }
    pile->resize(CARDS2DEAL - 1 + cJokers, Card::IPile::NORMAL);

    if (show)
        Glib::signal_timeout().connect(bind(sigc::ptr_fun(&Buraco::showJoker), pile, cJokers, false),
                                       Card::ComputerPlayer::TIMEOUT - 50);
    return false;
}

//-----------------------------------------------------------------------------
/// Makes a new pile for the passed team.
/// \param team Which team to make the pile for
/// \returns BuracoPile& New created pile
//-----------------------------------------------------------------------------
BuracoPile& Buraco::makeNewPile(unsigned int team) {
    TRACE8("Buraco::makeNewPile(unsigned int) - New pile for team " << team + 1);
    Check1(team < boxTeam.size());
    Check1(team < tablePiles.size());

    BuracoPile& pile(*tablePiles[team].emplace_back(std::make_unique<BuracoPile>()));
    pile.set_margin(5);
    if (!team)
        boxTeam[team].remove(newPile); // Keep the "new pile" widget last
    boxTeam[team].append(pile);
    if (!team)
        boxTeam[team].append(newPile);

    pile.show();
    return pile;
}

//-----------------------------------------------------------------------------
/// Removes a cerrado (a pile with 7 cards) from the table
/// \param player Player causing the remove of the pile
/// \param pile Pile holding the cerrado
//-----------------------------------------------------------------------------
void Buraco::removeCerrado(unsigned int player, BuracoPile& pile) {
    unsigned int team(player & 1);

    Check1(player < NUM_PLAYERS);
    Check1(std::ranges::find(tablePiles[team], &pile, getPtr) != tablePiles[team].end());
    TRACE8("Buraco::removeCerrado(unsigned int, BuracoPile&) - Pile "
           << (std::ranges::find(tablePiles[team], &pile, getPtr) - tablePiles[team].begin()) << " of team " << team);
    Check2(pile.size() == 7);

    pile.hide();
    Check3(static_cast<int>(pile.getPotentialPoints()) == pile.getPoints());
    points[team] += pile.getPoints();

    if (pile.getPoints() > 999)
        --unfinishedMonoPiles[team];
    updateInfo();
}

//-----------------------------------------------------------------------------
/// Actualizes the info-part of the statusbar
//-----------------------------------------------------------------------------
void Buraco::updateInfo() {
    Glib::ustring strInfo(_("Points [Buraco]: %1 [%2] / %3 [%4]"));
    strInfo.replace(strInfo.find("%1"), 2, YGP::ANumeric::toString(points[0]));
    strInfo.replace(strInfo.find("%2"), 2, (reserve[0].empty() ? _("N") : _("Y")));
    strInfo.replace(strInfo.find("%3"), 2, YGP::ANumeric::toString(points[1]));
    strInfo.replace(strInfo.find("%4"), 2, (reserve[1].empty() ? _("N") : _("Y")));

    info.set_text(strInfo);
}

//----------------------------------------------------------------------------
/// Sends a move-message to the connected machines
/// \param pile Number of pile involved
/// \param from Card to move
/// \param to Position card to move to
//----------------------------------------------------------------------------
void Buraco::sendMoveCard(unsigned int pile, unsigned int from, unsigned int to) const {
    TRACE8("Buraco::sendMoveCard(3x unsigned int) - Pile " << pile << ' ' << from << "->" << to);
    if (getConnectionMgr().getMode() != YGP::ConnectionMgr::NONE) {
        // Send played card to all clients (if any)
        std::ostringstream msg;
        msg << "Move=" << from << ";To=" << to << ";Pile=" << pile;

        sendMove(msg.str());
    }
}

//-----------------------------------------------------------------------------
/// Shows or hides the cards of the computer player
/// \param open Flag if cards should be shown or hidden
//-----------------------------------------------------------------------------
void Buraco::playOpen(bool open) {
    for (unsigned int i(1); i < NUM_PLAYERS; ++i) {
        hands[i].setShowOption(open ? Card::IPile::SHOWFACE : Card::IPile::SHOWBACK);
        hands[i].setStyle(open ? Card::IPile::COMPRESSED : Card::IPile::QUITE_COMPRESSED);
    }
}

//-----------------------------------------------------------------------------
/// Creates the combined team names from the players
/// \param names Array to receive groups
//-----------------------------------------------------------------------------
void Buraco::makeTeamNames(std::vector<Card::Player*>& names) const {
    Check1(actPlayers.size() >= NUM_PLAYERS);

    // First delete old names
    for (auto& name : names)
        delete name;
    names.clear();

    // ... then create it new with pair 0/2; 1/3
    for (unsigned int i(0); i < NUM_TEAMS; ++i) {
        Glib::ustring name(_("Team %1\n%2/%3"));
        name.replace(name.find("%1"), 2, 1, static_cast<char>('1' + i));
        name.replace(name.find("%2"), 2, actPlayers[i]->getName());
        name.replace(name.find("%3"), 2, actPlayers[i + 2]->getName());

        TRACE8("Buraco::makeTeamNames(std::vector<Card::Player*>) - Add: " << name);
        names.push_back(new Card::Human(name));
    }
}

//-----------------------------------------------------------------------------
/// Performs the steps to end the game
//-----------------------------------------------------------------------------
void Buraco::endGame() {
    TRACE8("Buraco::endGame() - " << currentPlayer());

    if (!currentPlayer())
        disableHuman();

    if (!pScoreDlg) {
        menuShowScoreDlg->set_enabled();
        pScoreDlg = Card::ScoreDlg::create(nameTeams);
        if (Gtk::Window* win = dynamic_cast<Gtk::Window*>(get_root()))
            pScoreDlg->set_transient_for(*win);
    }

    BuracoRules::RoundScore score(BuracoRules::roundScore(makeTable()));
    pScoreDlg->addPoints(score.bonus.data());
    pScoreDlg->addPoints(score.cards.data());
    points = score.cards;

    // Show all cards on the table
    for (const auto& team : tablePiles)
        for (const auto& p : team)
            p->show();
    pScoreDlg->show();

    Glib::ustring stat(_("Round ended"));
    unsigned int player;
    int maxPoints;
    pScoreDlg->getMaxPoints(maxPoints, player);
    TRACE7("Buraco::endGame() - Points: " << maxPoints);
    if (maxPoints >= static_cast<int>(ENDPOINTS)) {
        stat = _("Game ended; Team %1 won");
        stat.replace(stat.find("%1"), 2, 1, static_cast<char>('1' + player));
    }

    // Move cards of partners to first player and show them
    for (unsigned int i(1); i < NUM_PLAYERS; ++i) {
        hands[i].setStyle(Card::IPile::COMPRESSED);
        hands[i].setShowOption(Card::IPile::SHOWFACE);
    }

    status.pop();
    status.push(stat);

    setGameStatus(STOPPED);
    setNextPlayer(0); // enableHuman() checks for player == 0
    enableHuman();
    setNextPlayer(-1);
}

//-----------------------------------------------------------------------------
/// Compares the cards in the pile with regard of the number and with special
/// consideration of joker cards
/// \param a Card to compare
/// \param b Card to compare
/// \returns bool True, if a < b
//-----------------------------------------------------------------------------
bool Buraco::compByNumberWithJokers(const Card::Widget* a, const Card::Widget* b) {
    return BuracoRules::lessByNumberWithJokers(*a, *b);
}

//-----------------------------------------------------------------------------
/// Compares the cards in the pile with regard of the colour and with special
/// consideration of joker cards
/// \param a Card to compare
/// \param b Card to compare
/// \returns bool True, if a < b
//-----------------------------------------------------------------------------
bool Buraco::compByColourWithJokers(const Card::Widget* a, const Card::Widget* b) {
    return BuracoRules::lessByColourWithJokers(*a, *b);
}

//-----------------------------------------------------------------------------
/// Returns the actual state of the game (for the rules)
/// \returns BuracoRules::Table Actual game
//-----------------------------------------------------------------------------
BuracoRules::Table Buraco::makeTable() const {
    BuracoRules::Table table;
    for (unsigned int i(0); i < NUM_PLAYERS; ++i)
        table.hands[i] = hands[i].values();
    for (unsigned int i(0); i < NUM_TEAMS; ++i) {
        for (const auto& p : tablePiles[i])
            table.piles[i].push_back(p->values());
        table.reserve[i] = !reserve[i].empty();
        table.points[i] = points[i];
        table.unfinishedMonoPiles[i] = unfinishedMonoPiles[i];
    }
    table.buraco = {gStatus.team1Buraco, gStatus.team2Buraco};
    table.dumped = dumped.size();
    table.pickUpPlayed = gStatus.pickUpPlayed;
    table.startGame = gStatus.startGame;
    return table;
}

//-----------------------------------------------------------------------------
/// Re-arranges the cards in the hand of the player to the passed order
/// \param player Player whose cards to arrange
/// \param order New order of the cards (cards with the same ID are exchangeable)
//-----------------------------------------------------------------------------
void Buraco::arrangeHand(unsigned int player, const Card::Cards& order) {
    Check1(player < NUM_PLAYERS);
    Card::HPile& hand(hands[player]);
    Check1(hand.size() == order.size());

    for (unsigned int i(0); i < order.size(); ++i)
        if (hand[i]->id() != order[i].id()) {
            int pos(hand.find(order[i].id(), i + 1));
            Check3(pos > static_cast<int>(i));
            hand.move(i, pos);
        }
}

//-----------------------------------------------------------------------------
/// Shows the passed error of the human player
/// \param error Error to display
//-----------------------------------------------------------------------------
void Buraco::showInvalidMove(BuracoRules::PlayError error) {
    Gtk::MessageDialog dlg(_(BuracoRules::describe(error)), false, Gtk::MessageType::ERROR);
    dlg.set_title(_("Invalid move"));
    XGP::runModal(dlg);
}

//----------------------------------------------------------------------------
/// Changes the names of the playing people
/// \param newPlayer Array holding the new player
//----------------------------------------------------------------------------
void Buraco::changeNames(const std::vector<Card::Player*>& newPlayer) {
    Game::changeNames(newPlayer);
    makeTeamNames(nameTeams);

    for (unsigned int i(0); i < NUM_PLAYERS; ++i) {
        TRACE1("Buraco::changeNames(...) " << i << ": " << newPlayer[i]->getName());
        names[i].set_text(newPlayer[i]->getName());
    }

    if (pScoreDlg)
        pScoreDlg->update(nameTeams);
}

//----------------------------------------------------------------------------
/// Returns the pile from which the passed player plays cards to the passed
/// target
/// \param player Number of player
/// \param pile ID of the target (see handleMessage)
/// \returns Card::IPile* Pointer to pile to use or NULL, if the target is
///     invalid
//----------------------------------------------------------------------------
Card::IPile* Buraco::getPileOfPlayer(unsigned int player, unsigned int pile) {
    if (player >= NUM_PLAYERS)
        return nullptr;

    switch (pile) {
    case 1:
        return &hands[player];
    case 2:
        return &staple;
    case 3:
        return &dumped;
    default:
        // Cards played to a pile on the table (an existing or a new one)
        return ((pile >= 100) && (((pile - 100) >> 16) <= tablePiles[player & 1].size())) ? &hands[player] : nullptr;
    }
}

//----------------------------------------------------------------------------
/// Handles the messages the partners send for the Buraco cardgame. Those are
/// (additionally to the ones handled by Card::Game):
///   - <tt>Play=</tt><i>IDs of cards</i><tt>;Target=</tt><i>target</i>: The
///     player in turn plays the (blank-separated) cards to the target, which
///     is one of:
///       - 1: Dumps the (one) card from his hand; ending his turn.
///       - 2: Takes the top card of the staple into his hand.
///       - 3: Takes the top card of the dumped cards into his hand. If this is
///            not the first move of the game, he picks up the dumped cards:
///            He must play the taken card (with others) to a new pile; the
///            remaining dumped cards are added to his hand, as soon as that
///            pile holds (at least) 3 cards.
///       - (<i>pile</i> << 16) + <i>pos</i> + 100: Plays the cards from his
///            hand to the position of the pile (of his team). If \c pile is
///            the number of existing piles, a new pile is created.
///   - <tt>Move=</tt><i>from</i><tt>;To=</tt><i>to</i><tt>;Pile=</tt><i>pile</i>:
///     Moves a card (a joker) within the pile of the team of the player in
///     turn; followed by playing a card to that pile.
///   - <tt>Undo</tt>: Undoes the last card the (human) player in turn played
///     to the table.
///
/// The server passes the moves of a client on to all clients. Implicit
/// consequences of moves (removing a cerrado, getting the dumped cards or the
/// reserve, ending the game) are performed by every partner itself.
/// \param player ID of the player sending the message
/// \param message Message received from the partner
/// \returns bool True, if message has been processed completey
//----------------------------------------------------------------------------
bool Buraco::handleMessage([[maybe_unused]] unsigned int player, [[maybe_unused]] const std::string& message) {
    TRACE1("Buraco::handleMessage(unsigned int player, const std::string&) - " << message << " (" << player << ')');

#ifdef WITH_NETWORK
    const std::string_view cmd(Card::commandOf(message));
    if ((cmd == "Play") || (cmd == "Move") || (cmd == "Undo")) {
        if (gameStatus() == PLAYING)
            return (cmd == "Play") ? playRemoteCards(player, message)
                                   : ((cmd == "Move") ? moveRemoteCard(player, message) : undoRemoteMove(player));

        if (gameStatus() > INITIALIZING) {
            TRACE1("Buraco::handleMessage(unsigned int player, const std::string&) - Game stopped; ignoring " << message);
            return true;
        }
    }
#endif

    bool rc(Game::handleMessage(player, message));
#ifdef WITH_NETWORK
    // The client starts playing, after receiving the startplayer
    if ((cmd == "ActPlayer") && (getConnectionMgr().getMode() == YGP::ConnectionMgr::CLIENT) && (gameStatus() == PLAYING)) {
        TRACE1("Buraco::handleMessage(unsigned int player, const std::string&) - Start player: " << currentPlayer());
        startPlayer = currentPlayer();
        setStartPlayer();
    }
#endif
    return rc;
}

#ifdef WITH_NETWORK
//----------------------------------------------------------------------------
/// Returns the player in turn, after checking that he can make the received
/// move
/// \param sender ID of the player sending the message
/// \returns unsigned int Player in turn
/// \throw YGP::ParseError If the player in turn can't have sent the move
//----------------------------------------------------------------------------
unsigned int Buraco::getRemotePlayer(unsigned int sender) const {
    const unsigned int player(currentPlayer());

    // The own moves are never received; a server receives the moves of a
    // client only from that client
    if (!player || (player >= NUM_PLAYERS) ||
        ((getConnectionMgr().getMode() == YGP::ConnectionMgr::SERVER) && (sender != player)))
        throw YGP::ParseError(N_("Received a move of a player not in turn!"));
    return player;
}

//----------------------------------------------------------------------------
/// Executes the received move of a remote player (or a computer player of the
/// server): Plays the passed cards to the passed target
/// \param sender ID of the player sending the message
/// \param message Message describing the move: <tt>Play=</tt><i>IDs of
///     cards</i><tt>;Target=</tt><i>target</i> (see handleMessage)
/// \returns bool False, as the (animated) move is still pending; the game
///     continues, after it has been finished
/// \throw YGP::ParseError In case of an invalid move
//----------------------------------------------------------------------------
bool Buraco::playRemoteCards(unsigned int sender, const std::string& message) {
    TRACE3("Buraco::playRemoteCards(unsigned int, const std::string&) - " << message);

    const auto fields(Card::splitMessage(message));
    unsigned long dest(0);
    if ((fields.size() < 2) || (fields[1].key != "Target") || stringToNumber(dest, fields[1].value.c_str()) ||
        (dest != static_cast<unsigned int>(dest)))
        throw YGP::ParseError(N_("Invalid target!"));

    const unsigned int player(getRemotePlayer(sender));
    const unsigned int team(player & 1);
    Card::IPile* src(getPileOfPlayer(player, dest));
    if (!src)
        throw YGP::ParseError(N_("Invalid target!"));

    // Check the move
    Card::HPile& hand(hands[player]);
    const auto ids(Card::words(fields[0].value));
    unsigned int iPile(-1U), pos(0);
    if (src == &hand) {
        if (dest != 1) {
            iPile = (dest - 100) >> 16;
            pos = (dest - 100) & 0xffff;
            const BuracoPile* pile((iPile < tablePiles[team].size()) ? tablePiles[team][iPile].get() : nullptr);
            if (pile ? (!pile->get_visible() || (pos > pile->size()) || ((pile->size() + ids.size()) > 7))
                     : (pos || (ids.size() > 7)))
                throw YGP::ParseError(N_("Invalid target!"));
        }
        else if (ids.size() != 1)
            throw YGP::ParseError(N_("Invalid card specification!"));

        // Move the cards to play to the end of the hand (sets pos1Play/pos2Play)
        if (ids.empty() || (ids.size() > hand.size()))
            throw YGP::ParseError(N_("Card not found!"));
        flipCards2Play(hand, fields[0].value);
    }
    else {
        // The top card of the staple/the dumped cards is taken (the cards are
        // identified by their ID; and several cards might have the same)
        unsigned long id(0);
        if ((ids.size() != 1) || stringToNumber(id, ids[0].c_str()) || src->empty() || (src->getTopCard().id() != id))
            throw YGP::ParseError(N_("Card not found!"));
    }

    // Inform the other clients (the sender ignores the echo)
    if (getConnectionMgr().getMode() == YGP::ConnectionMgr::SERVER)
        broadcastMessage(message);

    const unsigned int first(pos1Play), last(pos2Play);
    pos1Play = pos2Play = -1U;
    switch (dest) {
    case 1:
        Check3(first == last);
        gStatus.startGame = 0;
        endTurn(player, first);
        break;

    case 2:
    case 3:
        startRemoteTurn(player);
        if (dest == 3) {
            if (!gStatus.startGame && (dumped.size() > 1))
                gStatus.pickUpPlayed = 1;
            dumped.getTopCard().show();
        }
        else if (gStatus.startGame && dumped.size()) // The first dumped card is not secret anymore
            dumped.getTopCard().show();
        gStatus.startGame = 0;

        animateCard(hand, *src, src->size() - 1).sigAnimation.connect(sigc::bind(mem_fun(*this, &Buraco::remoteMoveDone), -1U));
        break;

    default: {
        BuracoPile& pile((iPile < tablePiles[team].size()) ? *tablePiles[team][iPile] : makeNewPile(team));

        // Count the started piles of monos (like the players do it themselves)
        if (BuracoRules::isJoker(*hand[first]) &&
            (pile.empty() ? ((last > first) && std::all_of(hand.begin() + first, hand.end(),
                                                           [](const Card::Widget* c) { return BuracoRules::isJoker(*c); }))
                          : ((pile.size() == 1) && BuracoRules::isJoker(pile.getTopCard()))))
            ++unfinishedMonoPiles[team];

        if (first == last) {
            undo.assign(iPile, pos, first);
            if (movedPile == iPile)
                undo.monoPos = movedFrom;
        }
        animateCards(pile, pos, hand, first, last)
            .sigAnimation.connect(sigc::bind(mem_fun(*this, &Buraco::remoteMoveDone), iPile));
    }
    }

    movedPile = -1U;
    keepMessageLock();
    return false;
}

//----------------------------------------------------------------------------
/// Executes the received move of a card (a joker) within a pile on the table
/// \param sender ID of the player sending the message
/// \param message Message describing the move: <tt>Move=</tt><i>from</i>
///     <tt>;To=</tt><i>to</i><tt>;Pile=</tt><i>pile</i>
/// \returns bool True, as the move has been completely processed
/// \throw YGP::ParseError In case of an invalid move
//----------------------------------------------------------------------------
bool Buraco::moveRemoteCard(unsigned int sender, const std::string& message) {
    TRACE3("Buraco::moveRemoteCard(unsigned int, const std::string&) - " << message);

    unsigned long from(-1UL), to(-1UL), iPile(-1UL);
    for (const auto& field : Card::splitMessage(message)) {
        unsigned long* value((field.key == "Move") ? &from
                                                   : ((field.key == "To") ? &to : ((field.key == "Pile") ? &iPile : nullptr)));
        if (!value || stringToNumber(*value, field.value.c_str()))
            throw YGP::ParseError(N_("Invalid move!"));
    }

    const unsigned int player(getRemotePlayer(sender));
    if (iPile >= tablePiles[player & 1].size())
        throw YGP::ParseError(N_("Invalid pile!"));
    BuracoPile& pile(*tablePiles[player & 1][iPile]);
    if ((from >= pile.size()) || (to >= pile.size()))
        throw YGP::ParseError(N_("Invalid card!"));

    // Inform the other clients (the sender ignores the echo)
    if (getConnectionMgr().getMode() == YGP::ConnectionMgr::SERVER)
        broadcastMessage(message);

    pile.move(static_cast<unsigned int>(to), static_cast<unsigned int>(from));
    movedPile = static_cast<unsigned int>(iPile);
    movedFrom = static_cast<unsigned int>(from);
    return true;
}

//----------------------------------------------------------------------------
/// Undoes the last move to the table of the (remote) player in turn
/// \param sender ID of the player sending the message
/// \returns bool True, as the undo has been completely processed
/// \throw YGP::ParseError If there is nothing to undo
//----------------------------------------------------------------------------
bool Buraco::undoRemoteMove(unsigned int sender) {
    const unsigned int player(getRemotePlayer(sender));
    const auto& piles(tablePiles[player & 1]);
    if ((undo.destPile >= piles.size()) || (undo.destPos >= piles[undo.destPile]->size()) ||
        (undo.pickUp && (hands[player].size() < CARDS2DEAL)))
        throw YGP::ParseError(N_("Nothing to undo!"));

    // Inform the other clients (the sender ignores the echo)
    if (getConnectionMgr().getMode() == YGP::ConnectionMgr::SERVER)
        broadcastMessage("Undo");

    undoLast(player);
    return true;
}

//----------------------------------------------------------------------------
/// Performs the actions at the start of the turn of a remote player (like
/// playCards does it for a computer player)
/// \param player Player in turn
//----------------------------------------------------------------------------
void Buraco::startRemoteTurn(unsigned int player) {
    if (gStatus.startTurn) {
        TRACE5("Buraco::startRemoteTurn(unsigned int) - " << player);
        gStatus.startTurn = 0;

        if (((player & 1) ? gStatus.team2Buraco : gStatus.team1Buraco) == (player >> 1))
            ((player & 1) ? gStatus.team2Buraco : gStatus.team1Buraco) = 0x3;
    }
}

//----------------------------------------------------------------------------
/// Actions after a received move has been executed (animated): Performs the
/// implicit consequences of the move (like the player did it himself) and
/// continues the game.
/// \param pile Pile of the team of the player in turn the cards have been
///     played to; -1U if a card has been taken into the hand
//----------------------------------------------------------------------------
void Buraco::remoteMoveDone(unsigned int pile) {
    TRACE5("Buraco::remoteMoveDone(unsigned int) - " << pile);
    if (gameStatus() == STOPPED) // Game has been stopped meanwhile
        return;

    if ((pile != -1U) && (gameStatus() == PLAYING)) {
        const unsigned int player(currentPlayer());
        const unsigned int team(player & 1);
        Check3(pile < tablePiles[team].size());

        cleanCerrado(player);

        // The pile with the picked up card is complete: Take the dumped cards
        if (gStatus.pickUpPlayed && (tablePiles[team][pile]->size() > 2)) {
            gStatus.pickUpPlayed = 0;
            if (dumped.size()) {
                hands[player].getCards(dumped);
                hands[player].sort(compByNumberWithJokers);
            }
        }

        // If the player has no more cards left (except of jokers): Give him the
        // reserve or end the game
        if (std::ranges::all_of(tablePiles[team], [](const auto& p) { return p->size() > 2; })) {
            switch (BuracoRules::handStatus(hands[player].values(), !reserve[team].empty())) {
            case BuracoRules::HandStatus::TAKE_RESERVE:
                addBuraco(player);
                break;

            case BuracoRules::HandStatus::GOING_OUT:
                points[team] += BuracoRules::GOING_OUT_BONUS;
                endGame();
                return;

            case BuracoRules::HandStatus::PLAYING:
                break;
            }
        }
    }
    makeNextMoves();
}
#endif

//----------------------------------------------------------------------------
/// Returns the actual target, where flipCard2Play should position the cards to
/// \returns unsigned int ID of the target (in the format of the Play-message;
///     see handleMessage)
//----------------------------------------------------------------------------
unsigned int Buraco::getActTarget() const { return (target == -1U) ? 1 : (target + 100); }

//-----------------------------------------------------------------------------
/// Adds buraco-specific menus
/// \param menu Top-level menu to append the game's own submenu to
/// \param actions Action group ("game.*") to add the game's own actions to
//-----------------------------------------------------------------------------
void Buraco::addMenus(const Glib::RefPtr<Gio::Menu>& menu, const Glib::RefPtr<Gio::SimpleActionGroup>& actions) {
    TRACE1("Buraco::addMenus(const Glib::RefPtr<Gio::Menu>&, const Glib::RefPtr<Gio::SimpleActionGroup>&)");
    Check1(menu);
    Check1(actions);

    Glib::RefPtr<Gio::Menu> mb(Gio::Menu::create());

    menuUndo = actions->add_action("BuracoUndo", mem_fun(*this, &Buraco::undoMove));
    mb->append(_("_Undo"), "game.BuracoUndo");

    Glib::RefPtr<Gio::Menu> secSort(Gio::Menu::create());
    menuSort = actions->add_action("BuracoSort", mem_fun(*this, &Buraco::sortHand));
    secSort->append(_("_Sort cards (by number)"), "game.BuracoSort");
    menuSort2 = actions->add_action("BuracoSortCol", mem_fun(*this, &Buraco::sortHandByColour));
    secSort->append(_("Sort cards (by _colour)"), "game.BuracoSortCol");
    mb->append_section(secSort);

    Glib::RefPtr<Gio::Menu> secScore(Gio::Menu::create());
    menuShowScoreDlg = actions->add_action("showScoreDlg", bind(ptr_fun(&Card::ScoreDlg::display), &pScoreDlg));
    secScore->append(_("Show score dialog"), "game.showScoreDlg");
    mb->append_section(secScore);

    idxMenu = menu->get_n_items();
    menu->append_submenu(_("_Buraco"), mb);

    menuUndo->set_enabled(false);
    menuSort->set_enabled(false);
    menuSort2->set_enabled(false);
    menuShowScoreDlg->set_enabled(false);
}

//-----------------------------------------------------------------------------
/// Removes the buraco-specific menus
//-----------------------------------------------------------------------------
void Buraco::removeMenus(const Glib::RefPtr<Gio::Menu>& menu, const Glib::RefPtr<Gio::SimpleActionGroup>& actions) {
    Check1(menu);
    Check1(actions);

    if (idxMenu != -1) {
        menu->remove(idxMenu);
        idxMenu = -1;
    }
    actions->remove_action("BuracoUndo");
    actions->remove_action("BuracoSort");
    actions->remove_action("BuracoSortCol");
    actions->remove_action("showScoreDlg");
}

//-----------------------------------------------------------------------------
/// Undoes the last move of the human player
//-----------------------------------------------------------------------------
void Buraco::undoMove() {
    if (getConnectionMgr().getMode() != YGP::ConnectionMgr::NONE) {
        sendMove("Undo");
    }

    undoLast(0);
}

//-----------------------------------------------------------------------------
/// Undoes the last move
/// \param player Player whose turn to undo
//-----------------------------------------------------------------------------
void Buraco::undoLast(unsigned int player) {
    TRACE4("Buraco::undoLast(unsigned int) - " << player << ": " << undo.destPile << '-' << undo.destPos << "->" << undo.srcPos);
    if (!player)
        disableHuman();

    // Return the reserve (which has been inserted at the start of the hand)
    if (undo.pickUp) {
        Check3(reserve[player & 1].empty());
        Check3(hands[player].size() >= CARDS2DEAL);

        for (unsigned int i(0); i < CARDS2DEAL; ++i)
            reserve[player & 1].push_back(&hands[player].remove(0));
        updateInfo();
    }

    Check3(undo.destPile < tablePiles[player & 1].size());
    BuracoPile& src(*tablePiles[player & 1][undo.destPile]);
    Check3(undo.destPos < src.size());

    if (src.size() == 7) {
        src.show();
        Check3(static_cast<int>(src.getPotentialPoints()) == src.getPoints());
        points[player & 1] -= src.getPoints();
        updateInfo();

        if (src.getPosFirst() > 6)
            ++unfinishedMonoPiles[player & 1];
    }

    hands[player].insert(src.remove(undo.destPos), undo.srcPos);

    if (src.size() == 0) {
        boxTeam[player & 1].remove(src);
        tablePiles[player & 1].erase(tablePiles[player & 1].begin() + undo.destPile); // Deletes src
    }
    else if (undo.monoPos != 7)
        src.move(undo.monoPos, src.getPosJoker());

    menuUndo->set_enabled(false);
    if (!player) // The undo of a remote player just changes the table
        enableHumanHand();
}

//-----------------------------------------------------------------------------
/// Sorts the cards in the hand by number
//-----------------------------------------------------------------------------
void Buraco::sortHand() {
    disableHuman();
    hands[0].sort(compByNumberWithJokers);

    // Sorting directly after picking up the buraco disables undoing
    if (undo.pickUp)
        menuUndo->set_enabled(false);
    enableHumanHand();
}

//-----------------------------------------------------------------------------
/// Sorts the cards in the hand by colour
//-----------------------------------------------------------------------------
void Buraco::sortHandByColour() {
    disableHuman();
    hands[0].sort(compByColourWithJokers);

    // Sorting directly after picking up the buraco disables undoing
    if (undo.pickUp)
        menuUndo->set_enabled(false);
    enableHumanHand();
}

//-----------------------------------------------------------------------------
/// Actions to take when the cards are resized
/// \pre The cardsize must be set in Card::Images::WIDTH/HEIGHT
//-----------------------------------------------------------------------------
void Buraco::resizeCards() {
    staple.set_size_request(Card::Images::WIDTH, Card::Images::HEIGHT);
    dumped.set_size_request(Card::Images::WIDTH, Card::Images::HEIGHT);
    newPile.set_size_request(Card::Images::WIDTH, Card::Images::HEIGHT);
    for (auto& hand : hands)
        hand.set_size_request(-1, Card::Images::HEIGHT);

    for (auto& scrl : scrlTable)
        scrl.set_size_request(-1, Card::Images::HEIGHT + 5 * 15);
}
