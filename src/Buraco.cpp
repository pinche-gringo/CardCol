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
#include <bitset>
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
#include <card/Random.h>
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

    if (containsOnlyJoker(source)) {
        if (reserve[player & 1].size())
            addBuraco(player);
        else if (source.empty()) {
            Check3(points[player & 1] > 100);
            points[player & 1] += 100;
            endGame();
            return true;
        }
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
        if (gStatus.startGame
                ? (isJoker(dumpedCard) || playerPile.getFittingCard(dumpedCard, &cardDistance) != playerPile.end())
                : ((!isJoker(dumpedCard)) && pileHasFittingPair(playerPile, dumpedCard) &&
                   ((points[player & 1] > 100) || reserve[player & 1].size() || ((dumped.size() + playerPile.size()) > 4)))) {
            if (getConnectionMgr().getMode() != YGP::ConnectionMgr::NONE) {
                // Send played card to all clients (if any)
                std::ostringstream msg;
                msg << "Play=" << dumpedCard.id() << ";Target=3";

                sendMove(msg.str());
            }

            playerPile.insertSorted(dumped.removeTopCard(), compByNumberWithJokers);
            if (!gStatus.startGame) {
                if (dumped.size())
                    gStatus.pickUpPlayed = 1;

                std::map<unsigned int, unsigned int> aPos;
                std::vector<unsigned int> aOrder;
                unsigned int nrs(playerPile.getSeries(dumpedCard, aPos, aOrder, &cardDistance));
                TRACE8("Buraco::playCards() - Sizes: " << nrs << "<->" << aPos.size());
                Check3((nrs >= 3) || (aPos.size() >= 3));

                // Sanity checks: Don't dump more than 7 cards
                if (nrs > 7)
                    nrs = 7;

                // Don't dump all cards, if this would result in finishing the game
                // This can only be a numbered pile; as coloured piles return only 3 cards
                if ((points[player & 1] < 101) && ((dumped.size() + playerPile.size()) < 5))
                    --nrs;

                unsigned int pos1Play, pos2Play;
                unsigned int posTarget;
                if (nrs < aPos.size()) {
                    nrs = aPos.size();
                    pos1Play = playerPile.sortColourSerie(aPos, aOrder);
                    for (posTarget = pos1Play; posTarget < (pos1Play + nrs); ++posTarget)
                        if (playerPile[posTarget] == &dumpedCard)
                            break;
                    Check2(posTarget < (pos1Play + nrs));
                    posTarget -= pos1Play;
                    TRACE1("posTarget: " << posTarget);
                }
                else {
                    posTarget = nrs - 1;
                    pos1Play = playerPile.find(dumpedCard, compByNumberWithJokers);
                }
                pos2Play = pos1Play + nrs - 2;
                Check3((pos2Play - pos1Play) >= 1);

                // Put taken card back for animation
                dumped.setTopCard(playerPile.remove(dumpedCard));

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
/// \returns int ID for target (32 Bit: Pile << 16 + Position)
//-----------------------------------------------------------------------------
int Buraco::executeMove(unsigned int player, unsigned int& pos1Play, unsigned int& pos2Play) {
    TRACE6("Buraco::executeMove(player) - " << player);

    Card::IPile& playerPile(hands[player]);
    unsigned int target;

    // Check if any card can be added to an existing pile
    for (Card::IPile::const_iterator p(playerPile.begin()); p != playerPile.end(); ++p) {
        TRACE8("Buraco::executeMove(unsigned int) - Adding card " << **p << '?');
        target = cardFitsOnPlayedPile(player, p - playerPile.begin());
        if ((target != -1U) && canPlayCards(player, 1, target >> 16)) {
            pos1Play = pos2Play = p - playerPile.begin();
            return target;
        }
    }

    // Check for 3 cards belonging to a serie
    unsigned int i(0);
    for (; i < playerPile.size(); ++i) {
        TRACE8("Buraco::executeMove(unsigned int) - Analysing card " << *playerPile[i]);

        std::map<unsigned int, unsigned int> aPos; // diff, pos
        std::vector<unsigned int> aOrder;
        unsigned int nrs(playerPile.getSeries(*playerPile[i], aPos, aOrder, &cardDistance));

        // Play found cards (if any)
        //   - Play jokers if there are at least 5 and the team has still the
        //     reserve and the other team has no burraco and the reserve
        //   - Play the bigger of the found matching cards, if there are >= 3
        if (isJoker(*playerPile[i]) ? ((((nrs > 4) && reserve[player & 1].size()) || (nrs > 5)) &&
                                       ((points[(player + 1) & 1] < 101) || (nrs > 6) ||
                                        ((hands[(player + 2) % 3].size() > 5) && (hands[(player + 1) % 3].size() > 3))))
                                    : ((nrs > aPos.size()) ? (nrs > 2) : (aPos.size() > 2))) {
            unsigned int firstPos(i);
            if (nrs < aPos.size()) {
                firstPos = playerPile.sortColourSerie(aPos, aOrder);
                nrs = aPos.size();
            }
            if (nrs > 7)
                nrs = 7;

            bool canPlay(canPlayCards(player, nrs));
            if (canPlay || (nrs > 5)) {
                if (!canPlay)
                    nrs = 3;

                if (isJoker(*playerPile[firstPos]))
                    ++unfinishedMonoPiles[player & 1];

                // Create new pile with the found cards
                makeNewPile(player & 1);
                pos1Play = firstPos;
                pos2Play = firstPos + nrs - 1;
                return (tablePiles[player & 1].size() - 1) << 16;
            }
            else
                hands[player].sort(compByNumberWithJokers);
        }
    }

    // Check if all cards in the hand can (and should) be played
    TRACE8("Buraco::executeMove(unsigned int) - Playing all?");
    if (!unfinishedMonoPiles[player & 1] &&
        ((points[player & 1] > 100) || (reserve[player & 1].size() && canGetRidOfCards(player)))) {
        Card::IPile::const_iterator ci(playerPile.begin());
        // If pile still has normal cards (no joker)
        while (!((ci == playerPile.end()) || isJoker(**ci))) {
            Card::IPile::const_iterator next(playerPile.getFittingCard(**ci, ci + 1, &cardDistance));
            if ((next != playerPile.end()) && isJoker(*playerPile[playerPile.size() - 1])) {
                TRACE1("Buraco::executeMove(unsigned int) - Have two with joker: " << **ci << " and " << **next);
                int diff(cardDistance(**next, **ci));
                Check3(diff ? (*next)->colour() == (*ci)->colour() : true);
                if (diff < 0) {
                    Check3(diff >= -2);
                    playerPile.move(playerPile.size() - 2, next - playerPile.begin());
                    playerPile.move(playerPile.size() + ((diff == -2) ? -1 : -2), ci - playerPile.begin());
                }
                else {
                    Check3(diff <= 2);
                    playerPile.move(playerPile.size() - 1, next - playerPile.begin());
                    playerPile.move(playerPile.size() - 1 - diff, ci - playerPile.begin());
                }

                // Create a new pile with the found pair and a joker
                makeNewPile(player & 1);
                pos1Play = playerPile.size() - 3;
                pos2Play = playerPile.size() - 1;
                return (tablePiles[player & 1].size() - 1) << 16;
            }
            ++ci;
        }
    }

    // Play all jokers if team has a cerrado, or leave one, if the player has
    // >= 2 normal cards left.
    if (playerPile.size() && (points[player & 1] > 100)) {
        if ((isJoker(*playerPile[playerPile.size() - 1])) &&
            ((playerPile.size() <= 2) || ((!isJoker(*playerPile[1])) || isJoker(*playerPile[playerPile.size() - 2])))) {
            unsigned int bestPile(-1U);
            unsigned int size(0);
            for (auto p(tablePiles[player & 1].cbegin()); p != tablePiles[player & 1].cend(); ++p) {
                if ((((*p)->size() < 7) && ((*p)->getPosJoker() > 6)) &&
                    (((*p)->size() > size) ||
                     (((*p)->size() == size) &&
                      ((*p)->getPotentialPoints() > tablePiles[player & 1][bestPile]->getPotentialPoints())) ||
                     ((*p)->getPotentialPoints()) >= 1000)) {
                    bestPile = p - tablePiles[player & 1].begin();
                    size = (*p)->size();
                }
            }

            if (bestPile != -1U) {
                pos1Play = pos2Play = playerPile.size() - 1;
                unsigned int pos, move;

                Check3(bestPile < tablePiles[player & 1].size());
                tablePiles[player & 1][bestPile]->getPosition4Card(*playerPile[playerPile.size() - 1], pos, move);
                return (bestPile << 16) + pos;
            }
        }
    }

    // No more cards to put down: Find a card to dump
    TRACE8("Buraco::executeMove(unsigned int) - Searching for a card to dump");
    for (i = 0; i < playerPile.size() - 1; ++i) {
        Card::IPile::const_iterator p(playerPile.getFittingCard(*playerPile[i], playerPile.begin(), &cardDistance));
        if (static_cast<unsigned int>(p - playerPile.begin()) == i)
            p = playerPile.getFittingCard(*playerPile[i], ++p, &cardDistance);
        if (p == playerPile.end())
            break;
    }

    while (i && isJoker(*playerPile[i])) // Try to not dump jokers
        --i;

    Check3(i < playerPile.size());
    pos1Play = pos2Play = i;
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
            hands[(i - posServer) & 0x3].getCards(staple, staple.size() - CARDS2DEAL - 1, staple.size() - 1);

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
            startPlayer = Card::randomNumber(4);
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
    if (!humanPilesOK()) {
        Gtk::MessageDialog dlg(_("Every pile on the table must have at least 3 cards!"), false, Gtk::MessageType::ERROR);
        dlg.set_title(_("Invalid move"));
        XGP::runModal(dlg);
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
    unregisterHandDND(*hands[0][iCard]);
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
        if (!gStatus.startGame)
            try {
                if (isJoker(card))
                    throw _("You can't pick up monos!");

                if (!pileHasFittingPair(hands[0], card))
                    throw _("You need a fitting pair to pick up the pile of dumped cards!");

                // Don't allow picking up the pile, if that would force the game
                // to end without having neither buraco nor reserve
                if (((dumped.size() + hands[0].size()) < 5) && (points[0] < 200) && reserve[0].empty())
                    throw _("Picking up the staple would leave you without cards\n"
                            "and you can't end the game now!");
            }
            catch (Glib::ustring& e) {
                Gtk::MessageDialog dlg(e, false, Gtk::MessageType::ERROR);
                dlg.set_title(_("Invalid move"));
                XGP::runModal(dlg);
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
            Check3(pileHasFittingPair(hands[0], card));

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
/// Prepares the card for drag�n�drop
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
/// Stops the drag�n�drop abilities of the passed card
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
/// Prepares the passed region of cards for drag�n�drop
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
/// Prepares the card for drag�n�drop
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
/// Stops the drag�n�drop abilities of the passed card
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
/// Prepares the passed region of cards for drag�n�drop
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
/// Checks if the piles on the table are valid (have at least 3 cards)
/// \param except Pile which can be invalid
/// \returns bool True, if the piles are OK
//-----------------------------------------------------------------------------
bool Buraco::humanPilesOK(unsigned int except) const {
    for (unsigned int i(0); i < tablePiles[0].size(); ++i) {
        const BuracoPile& pile(*tablePiles[0][i]);
        Check3((pile.size() < 7) || !pile.get_visible());
        if (i == except)
            continue;

        if (pile.size() < 3)
            return false;
    }
    return true;
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

    try {
        // Check if all piles (except those to which card is dropped) are valid
        if (!humanPilesOK(iCard >> 8))
            throw Glib::ustring(_("You need to fill up other piles first!"));

        Card::Widget& moved(*hands[0][valueDropped]);
        TRACE4("Buraco::cardDroppedOnTable(...) - Card dropped: " << moved);

        // Move dropped card to a (new) pile on the table
        unsigned int iPile;
        BuracoPile* pile(nullptr);
        if (iCard == -1U) { // If card was dropped on the new pile: Create pile
            // Check validity of drop
            if (!(isJoker(moved) ? pileHasFittingPair(hands[0], &moved)
                                 : pileHasFittingPair(hands[0], moved, !gStatus.pickUpPlayed)))
                throw Glib::ustring(_("There are no cards to make a valid new pile!"));

            // Only allow dropping on new pile while having < 5 cards, if the game
            // can be ended, or there is still the reserve
            if (!canPlayCards(0, 3))
                throw Glib::ustring(_(unfinishedMonoPiles[0]
                                          ? N_("You can't end the game (a pile of monos is not finished)!")
                                          : ((hands[0].size() <= 5) ? N_("You can't end the game (there's no \"cerrado\")!")
                                                                    : N_("Not enough cards to make new pile!"))));

            iPile = tablePiles[0].size();
            pile = &makeNewPile(0);
            Check3(tablePiles[0].size());
            iCard = 0;
        }
        else {
            // Can't use the new pile (with the picked up card) with a joker
            if (isJoker(moved) && gStatus.pickUpPlayed)
                throw Glib::ustring(_("You may not start this new pile with a joker!"));

            // Else check pile to use
            Check1((iCard >> 8) < tablePiles[0].size());
            pile = tablePiles[0][iPile = (iCard >> 8)].get();

            if ((iCard = cardFitsOnPile(iPile, moved)) == -1U)
                throw Glib::ustring(_("This card does not fit on that pile!"));

            // Only allow dropping of last card, if the game can be ended, or there
            // is still the reserve
            if (!canPlayCards(0, 1, iPile))
                throw Glib::ustring(_(unfinishedMonoPiles[0] ? N_("You can't end the game (a pile of monos is not finished)!")
                                                             : N_("You can't end the game (there's no \"cerrado\")!")));

            if ((pile->size() == 1) && isJoker(pile->getTopCard()) && isJoker(moved))
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
        if (containsOnlyJoker(hands[0]) && humanPilesOK()) {
            if (!reserve[0].empty()) {
                Glib::signal_idle().connect(bind_return(mem_fun(*this, &Buraco::addBuraco4HumanAndEnable), false));
                return true;
            }
            else if (hands[0].empty()) {
                points[0] += 100;
                endGame();
                return true;
            }
        }
        Check3(aDNDHand.size() == hands[0].size());
    }
    catch (Glib::ustring& error) {
        Gtk::MessageDialog dlg(error, false, Gtk::MessageType::ERROR);
        dlg.set_title(_("Invalid move"));
        XGP::runModal(dlg);
        return false;
    }
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
/// Prepares the passed region of cards for drag�n�drop
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
/// Checks, if the passed pile contains no cards except jokers or 2s. This is
/// also true for empty piles.
/// \param pile Pile to inspect
/// \returns bool True, if there are only jokers (or pile is empty)
//-----------------------------------------------------------------------------
bool Buraco::containsOnlyJoker(const Card::IPile& pile) {
    TRACE8("Buraco::containsOnlyJoker(const Card::IPile&)");

    for (auto* i : pile) {
        Check3(i);
        if (!isJoker(*i))
            return false;
    }
    return true;
}

//-----------------------------------------------------------------------------
/// Checks, if the passed pile does not contain neither jokers nor 2s.
/// \param pile Pile to inspect
/// \returns bool True, if there are no jokers
//-----------------------------------------------------------------------------
bool Buraco::containsNoJoker(const Card::IPile& pile) {
    for (auto* i : pile) {
        Check3(i);
        if (isJoker(*i))
            return false;
    }
    return true;
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
/// Checks if the passed card can be put on one of the existing piles
/// \param player Player to inspect
/// \param iCard Card to inspect
/// \returns unsigned int Value describing the pile (and the offset of the card)
///     to play to; -1 if none
/// \remarks This method moves the card
//-----------------------------------------------------------------------------
unsigned int Buraco::cardFitsOnPlayedPile(unsigned int player, unsigned int iCard) {
    TRACE8("Buraco::cardFitsOnPlayedPile(unsigned int, unsigned int) - Card " << iCard << " of player " << player);
    Check1(player);
    Check1(player < NUM_PLAYERS);
    Check1(iCard < hands[player].size());
    Card::Widget& card(*hands[player][iCard]);
    TRACE3("Buraco::cardFitsOnPlayedPile(unsigned int, unsigned int) - Card " << card);

    unsigned int bestPile(-1U);
    unsigned int maxPoints(0);
    unsigned int size(0);
    for (auto p(tablePiles[player & 1].begin()); p != tablePiles[player & 1].end(); ++p) {
        TRACE5("Buraco::cardFitsOnPlayedPile(unsigned int, unsigned int) - Checking pile "
               << (p - tablePiles[player & 1].begin()));
        Check3(*p);
        if ((*p)->size() == 7) { // Skip finished piles
            Check3(!(*p)->get_visible());
            continue;
        }
        Check3((*p)->size() >= 3);
        Check3((*p)->size() < 7);

        // Play joker, if you can make a cerrado (7 in a row) - but only if the
        // one having picked up the reserve already played (the missing card
        // might be in there) and the oponent can't finish.
        int posPile((*p)->size());
        if (isJoker(card)
                ? (((((*p)->size() == 6) && ((*p)->getPosJoker() > 6)) && ((hands[player].size() - iCard) < 7) &&
                    (((reserve[player & 1].empty() &&
                       (((((player & 1) ? gStatus.team2Buraco : gStatus.team1Buraco) == 0x3)) || (hands[player].size() < 3))) ||
                      (points[player & 1] > 100)) ||
                     reserve[!(player & 1)].empty() || (points[!(player & 1)] > 100))) ||
                   ((*p)->getPosFirst() > 6))
                : ((posPile = cardFitsOnPile(p - tablePiles[player & 1].begin(), card)) != -1)) {
            // Always play on a joker pile (don't bother checking for a second one)
            if ((*p)->getPotentialPoints() >= 1000) {
                bestPile = p - tablePiles[player & 1].begin();
                break;
            }
            if ((size < (*p)->size()) || ((size == (*p)->size()) && (maxPoints < (*p)->getPotentialPoints()))) {
                size = (*p)->size();
                maxPoints = (*p)->getPotentialPoints();
                bestPile = p - tablePiles[player & 1].begin();
            }
        }
    }

    if (bestPile != -1U) {
        unsigned int pos(0), move(-1U);
        BuracoPile& pile(*tablePiles[player & 1][bestPile]);
        Check3(pile.getPosition4Card(card, pos, move));

        pile.getPosition4Card(card, pos, move);
        Check3(pos <= pile.size());
        if ((move != -1U) && canPlayCards(player, 1, bestPile)) {
            Check3(move <= pile.size());
            Check3(move != pile.getPosJoker());
            Check3(pile.getPosJoker() != 7);
            sendMoveCard(bestPile, pile.getPosJoker(), move);
            pile.move(move, pile.getPosJoker());
        }
        return (bestPile << 16) + pos;
    }
    return -1U;
}

//-----------------------------------------------------------------------------
/// Checks if the passed card is a joker
/// \param card Card to inspect
/// \returns bool True if card is a joker
//-----------------------------------------------------------------------------
bool Buraco::isJoker(const Card::Widget& card) {
    return ((card.number() == Card::Widget::TWO) || (card.number() > Card::Widget::ACE));
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

//-----------------------------------------------------------------------------
/// Checks if the passed card fits on the passed staple
/// \param iPile Pile to inspect
/// \param card Card to check
/// \returns Position where card can be played to, or -1 if card does not fit
//-----------------------------------------------------------------------------
int Buraco::cardFitsOnPile(unsigned int iPile, const Card::Widget& card) const {
    Check1(iPile < tablePiles[currentPlayer() & 1].size());
    BuracoPile& pile(*tablePiles[currentPlayer() & 1][iPile]);
    Check2(pile.size());
    Check2(pile.size() < 7);

    // Card played on a joker: Valid is:
    //   - A joker; if there are at least 3 jokers (on table + in hand)
    //   - Any card, which has a pair (if there's only one joker on the table)
    if (pile.getPosFirst() > 6) {
        if (pile.getPosJoker())
            return isJoker(card) ? 0 : -1;
        else {
            Card::IPile::const_iterator pCard(hands[currentPlayer()].getFittingCard(card, &cardDistance));
            if (*pCard == &card)
                pCard = hands[currentPlayer()].getFittingCard(card, ++pCard, &cardDistance);
            return pCard == hands[currentPlayer()].end() ? -1 : 0;
        }
    }

    unsigned int pos, move;
    if (pile.getPosition4Card(card, pos, move)) {
        Check3(pos <= pile.size());
        if ((pile.size() > 1) || isJoker(card))
            return pos;
        else {
            const Card::HPile& hand(hands[currentPlayer()]);
            if (!containsNoJoker(hand))
                return pos;

            Card::Widget& pileCard(*pile[pile.getPosFirst()]);
            int dist(cardDistance(pileCard, card));
            Check3((dist > -2) && (dist < 2));
            int cmp(0);

            Card::IPile::const_iterator pCard(hand.begin());
            do {
                pCard = hand.getFittingCard(pileCard, pCard, &cardDistance);
                if (*pCard == &card)
                    pCard = hand.getFittingCard(pileCard, ++pCard, &cardDistance);
                if (pCard == hand.end())
                    return -1;

                TRACE8("Buraco::cardFitsOnPile(unsigned int, const Card::Widget&) const -  Dist: "
                       << dist << "<->" << cardDistance(**pCard, card));

                cmp = dist - cardDistance(**pCard, card);
                ++pCard;
            }
            while ((cmp != -dist) && (cmp != (dist << 1)));
            return pos;
        }
    }
    return -1;
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

    points[0] += reserve[0].empty() ? 100 : -100;
    points[1] += reserve[1].empty() ? 100 : -100;
    pScoreDlg->addPoints(points.data());

    // Sum up all cards on the table
    for (unsigned int i(0); i < NUM_TEAMS; ++i) {
        int sum(0);
        int monoPile(0);

        for (const auto& p : tablePiles[i]) {
            Check3(p);
            Check3(p->size() > 2);

            p->show();
            // Substract 1000 points for every started cerrado of monos
            if (p->getPoints() < 0)
                monoPile += 1000;
            sum += p->getCardPoints();
        }

        TRACE5("Buraco::endGame() - Points of team " << i << " on table: " << sum << '/' << monoPile);
        points[i] = ((points[i] < (reserve[i].size() ? 100 : 300)) ? -sum : sum) - monoPile;
    }

    for (unsigned int i(0); i < NUM_PLAYERS; ++i)
        for (const auto* c : hands[i])
            points[i & 1] -= getPoints(*c);

    pScoreDlg->addPoints(points.data());
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
/// Returns the value of the passed card
/// \param card Card to inspect
/// \returns unsigned int Value of the card
//-----------------------------------------------------------------------------
unsigned int Buraco::getPoints(const Card::Widget& card) {
    // Card:                 2   3  4  5  6  7  8   9   10  J   Q   K   A   Joker
    static constexpr std::array<unsigned int, 14> values{25, 5, 5, 5, 5, 5, 10, 10, 10, 10, 10, 10, 20, 50};
    Check3(card.number() < static_cast<int>(values.size()));
    return values[card.number()];
}

//-----------------------------------------------------------------------------
/// Checks if the player can get rid of all cards in his hand except of the
/// jokers
/// \param player Player whose cards should be inspected
/// \returns bool True: if all cards can be played
/// \remarks This method does not check for triplets anymore!
//-----------------------------------------------------------------------------
bool Buraco::canGetRidOfCards(unsigned int player) const {
    TRACE5("Buraco::canGetRidOfCards(unsigned int) - Checking player " << player);
    std::bitset<160> used;
    Check3(hands[player].size() < used.size());
    const Card::HPile& pile(hands[player]);

    unsigned int cJokers(0);
    unsigned int piles(0);
    for (Card::IPile::const_iterator i(pile.begin()); i != pile.end(); ++i) {
        if (used[i - pile.begin()])
            continue;

        if (isJoker(**i)) {
            used.set(i - pile.begin());
            ++cJokers;
            continue;
        }

        // If there are equal cards (and the first card is not already marked as
        // used: Mark both card as used
        Card::IPile::const_iterator o(pile.getFittingCard(**i, i + 1, &cardDistance));
        if ((o != pile.end()) && !used[o - pile.begin()]) {
            ++piles;
            used.set(i - pile.begin());
            used.set(o - pile.begin());
        }
    }

    TRACE8("Buraco::canGetRidOfCards(unsigned int) -  "
           << used.count() << '/' << hands[player].size() << "; " << cJokers << " Joker for " << piles << " piles -> "
           << ((((used.count() + 1) >= hands[player].size()) && (piles <= cJokers)) ? 'Y' : 'N'));
    return (((used.count() + 1) >= hands[player].size()) && (piles <= cJokers));
}

//-----------------------------------------------------------------------------
/// Checks if the player can play the specified number of cards; a player can
/// only play all of his cards, if:
///   - The team has a cerrado
///   - The team still has the reserve
///   - The player can close a pile, with the cards to play
///   - After the turn there are no unfinished mono-piles
/// \param player Player to analyse
/// \param cards Number of cards player wants to play
/// \param pile Pile player is going to play its card to (or -1 for a new one)
/// \returns bool True, if card can be played
//-----------------------------------------------------------------------------
bool Buraco::canPlayCards(unsigned int player, unsigned int cards, unsigned int pile) const {
    TRACE7("Buraco::canPlayCards(3x unsigned int) - Player " << player << " playing " << cards << " cards to " << pile);
    Check1(player < NUM_PLAYERS);
    Check1((pile == -1U) || (tablePiles[player & 1].size() > pile));
    Check1((pile == -1U) || ((tablePiles[player & 1][pile]->size() + cards) <= 7));
    Check1(hands[player].size() >= cards);
    Check1(cards <= 7);

    unsigned int cCards(hands[player].size());
    if (gStatus.pickUpPlayed)
        cCards += dumped.size();

    if ((cCards <= (cards + 1)) && reserve[player & 1].empty()) {
        bool canPlay((points[player & 1] > 100) ||
                     ((pile != -1U) && (((tablePiles[player & 1][pile]->size() + cards) >= 7) ||
                                        (((tablePiles[player & 1][pile]->size() + cards) == 6) &&
                                         ((hands[player].size() - cards) == 1) && canClosePile(player, pile)))) ||
                     (cards >= 7));
        TRACE1("Buraco::canPlayCards(3x unsigned int) - Can play: " << (canPlay ? "Yes" : "No"));
        return ((!unfinishedMonoPiles[player & 1]) || ((unfinishedMonoPiles[player & 1] == 1) && (pile != -1U) &&
                                                       (tablePiles[player & 1][pile]->getPoints() < 0))
                    ? canPlay
                    : false);
    }
    return true;
}

//----------------------------------------------------------------------------
/// Checks if the player can with his two cards left close the passed pile
/// \param player Player to inspect
/// \param pile Pile to analyse
/// \return bool True, if the remaining cards of the player can make a
///        cerrado for this pile
/// \remarks - The player must have only two cards; the pile 5
//----------------------------------------------------------------------------
bool Buraco::canClosePile(unsigned int player, unsigned int pile) const {
    TRACE7("Buraco::canClosepile(2x unsigned int) - Player " << player << " closes pile " << pile);
    Check1(player < NUM_PLAYERS);
    Check1(tablePiles[player & 1].size() > pile);

    BuracoPile& orig(*tablePiles[player & 1][pile]);
    Check1(orig.size() == 5);
    Check1(hands[player].size() == 2);

    bool isOK(false);
    // Make a copy of the original pile. The copied cards are owned by cards;
    // which (being declared after copy) is destroyed first.
    BuracoPile copy;
    std::vector<std::unique_ptr<Card::Widget>> cards;
    auto dupCard([&cards](const Card::Widget& card) -> Card::Widget& {
        return *cards.emplace_back(std::make_unique<Card::Widget>(card));
    });
    for (auto* i : orig)
        copy.Card::IPile::append(dupCard(*i));

    unsigned int pos, move;
    for (unsigned int i(0); i < 2; ++i) {
        if (copy.getPosition4Card(*hands[player][i], pos, move)) {
            Check3(pos <= copy.size());
            if (move != -1U) {
                Check3(move <= copy.size());
                Check3(move != copy.getPosJoker());
                copy.move(move, copy.getPosJoker());
            }
            copy.insert(dupCard(*hands[player][i]), pos);

            if (copy.getPosition4Card(*hands[player][!i], pos, move)) {
                isOK = true;
                break;
            }
            else
                copy.remove(pos);
        }
    }
    return isOK;
}

//-----------------------------------------------------------------------------
/// Checks if the passed pile contains a pair matching the passed card
/// \param pile Pile to inspect
/// \param card Card where to find a pair to
/// \param withJokers Flag, if jokers should be inspected
/// \returns bool True, if the pile contains a matching pair
//-----------------------------------------------------------------------------
bool Buraco::pileHasFittingPair(const Card::IPile& pile, const Card::Widget& card, bool withJokers) {
    TRACE3("Buraco::pileHasFittingPair(const Card::IPile&, const Card::Widget&, bool) - " << card);

    if (withJokers) {
        Card::IPile::const_iterator i(pile.getFittingCard(card, pile.begin(), &cardDistance));
        if (*i == &card)
            i = pile.getFittingCard(card, ++i, &cardDistance);
        if ((i != pile.end()) && !containsNoJoker(pile))
            return true;
    }

    return pile.hasFittingPair(card, &cardDistance);
}

//-----------------------------------------------------------------------------
/// Checks if the passed pile contains a pair matching the passed card
/// \param pile Pile to inspect
/// \param exclude Card to not inspect (can be NULL)
/// \returns bool True, if the pile contains a matching pair
//-----------------------------------------------------------------------------
bool Buraco::pileHasFittingPair(const Card::IPile& pile, const Card::Widget* exclude) {
    TRACE3("Buraco::pileHasFittingPair(const Card::IPile&, const Card::Widget*)");

    for (auto p(pile.begin()); p != pile.end(); ++p)
        if (*p != exclude)
            if ((pile.getFittingCard(**p, pile.begin(), &cardDistance) != p) ||
                (pile.getFittingCard(**p, p + 1, &cardDistance) != pile.end()))
                return true;
    return false;
}

//-----------------------------------------------------------------------------
/// Compares the cards in the pile with regard of the number and with special
/// consideration of joker cards
/// \param a Card to compare
/// \param b Card to compare
/// \returns bool True, if a < b
//-----------------------------------------------------------------------------
bool Buraco::compByNumberWithJokers(const Card::Widget* a, const Card::Widget* b) {
    // Card:                 2   3  4  5  6  7  8  9  10 J  Q   K  A   Joker
    static constexpr std::array<unsigned char, 14> values{12, 0, 1, 2, 3, 4, 5, 6, 7, 8, 9, 10, 11, 13};
    Check3(a->number() < static_cast<int>(values.size()));
    Check3(b->number() < static_cast<int>(values.size()));

    return values[a->number()] < values[b->number()];
}

//-----------------------------------------------------------------------------
/// Compares the cards in the pile with regard of the colour and with special
/// consideration of joker cards
/// \param a Card to compare
/// \param b Card to compare
/// \returns bool True, if a < b
//-----------------------------------------------------------------------------
bool Buraco::compByColourWithJokers(const Card::Widget* a, const Card::Widget* b) {
    switch (a->number()) {
    case Card::Widget::TWO:
        return b->number() == Card::Widget::UNREACHABLE;
        break;

    case Card::Widget::UNREACHABLE:
        return false;
        break;

    default:
        return (isJoker(*b) ? true : ((a->colour() == b->colour()) ? a->number() < b->number() : a->colour() < b->colour()));
    } // endswitch
}

//----------------------------------------------------------------------------
/// Returns the distance between two cards. The ace also counts as one (if the
/// other card is a 3 or a 4) and 2's are equal to jokers.
/// \param a Card to compare
/// \param b Card to compare
/// \returns int Distance of the two passed cards (a - b)
//----------------------------------------------------------------------------
int Buraco::cardDistance(const Card::Widget& a, const Card::Widget& b) { return cardDistance(a, b, true); }

//----------------------------------------------------------------------------
/// Returns the distance between two cards. The ace also counts as one (if the
/// other card is a 3 or a 4) and 2's are equal to jokers.
/// \param a Card to compare
/// \param b Card to compare
/// \param aceIsOne Flag, if aces should (also) be treated as one
/// \returns int Distance of the two passed cards (a - b)
//----------------------------------------------------------------------------
int Buraco::cardDistance(const Card::Widget& a, const Card::Widget& b, bool aceIsOne) {
    TRACE9("Buraco::cardDistance(2x const Card::Widget&, bool) - " << a << "<->" << b);
    // Special handling of jokers
    bool aJoker(isJoker(a));
    bool bJoker(isJoker(b));
    if (aJoker || bJoker)
        return aJoker && bJoker ? 0 : 99;

    if (a.colour() != b.colour())
        return (a.number() == b.number()) ? 0 : 99;

    if (aceIsOne) { // Special handling of the ace like 1
        TRACE9("Buraco::cardDistance(2x const Card::Widget&, bool) - Ace");
        if ((a.number() == Card::Widget::ACE) && (b.number() < Card::Widget::EIGHT))
            return -static_cast<int>(b.number());
        else if ((b.number() == Card::Widget::ACE) && (a.number() < Card::Widget::EIGHT))
            return static_cast<int>(a.number());
    }

    TRACE9("Buraco::cardDistance(2x const Card::Widget&, bool) - Distance: " << a.number() - b.number());
    return a.number() - b.number();
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
        if (isJoker(*hand[first]) &&
            (pile.empty() ? ((last > first) &&
                             std::all_of(hand.begin() + first, hand.end(), [](const Card::Widget* c) { return isJoker(*c); }))
                          : ((pile.size() == 1) && isJoker(pile.getTopCard()))))
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
        if (containsOnlyJoker(hands[player]) &&
            std::ranges::all_of(tablePiles[team], [](const auto& p) { return p->size() > 2; })) {
            if (!reserve[team].empty())
                addBuraco(player);
            else if (hands[player].empty()) {
                points[team] += 100;
                endGame();
                return;
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
