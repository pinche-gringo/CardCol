// PROJECT     : Cardgames
// SUBSYSTEM   : Jabberwocky
// REFERENCES  :
// TODO        :
// BUGS        :
// AUTHOR      : Markus Schwab
// CREATED     : 09.08.2006
// COPYRIGHT   : Copyright (C) 2006 - 2018, 2024, 2026

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

#include <memory>
#include <sstream>
#include <string_view>

#include <glibmm/main.h>

#include <gtkmm/adjustment.h>
#include <gtkmm/box.h>
#include <gtkmm/button.h>
#include <gtkmm/messagedialog.h>
#include <gtkmm/spinbutton.h>
#include <gtkmm/statusbar.h>
#include <gtkmm/window.h>

#include <giomm/menu.h>
#include <giomm/simpleaction.h>
#include <giomm/simpleactiongroup.h>

#include <YGP/ANumeric.h>
#include <YGP/Check.h>
#include <YGP/ConnMgr.h>
#include <YGP/Trace.h>

#include <XGP/XDialog.h>

#include <card/ComputerPlayer.h>
#include <card/Images.h>
#include <card/Message.h>
#include <card/Player.h>
#include <card/Random.h>
#include <card/RemotePlayer.h>
#include <card/ScoreDlg.h>
#include <card/Window.h>

#include "Jabberwocky.h"

Card::Widget::COLOURS Jabberwocky::trumpColour(Card::Widget::CLUBS);

//-----------------------------------------------------------------------------
/// Constructor
/// \param parent Parent widget to display the game in
/// \param statusbar Status bar widget to display information about the game
/// \param cardset Cardset to use
/// \param player Vector of player
/// \param posPlayer Position of player for the server
/// \param mxSerialize Mutex to serialize messages from the server
//-----------------------------------------------------------------------------
Jabberwocky::Jabberwocky(Gtk::Box& parent, Gtk::Statusbar& statusbar, Card::Set& cardset,
                         const std::vector<Card::Player*>& player, unsigned int posPlayer, Card::MessageLock& mxSerialize)
    : Game(parent, statusbar, cardset, player, posPlayer, mxSerialize, 15, 15),
      played(Card::IPile::COMPRESSED, Card::IPile::SHOWFACE), pTrump(nullptr), startPlayer(Card::randomNumber(NUM_PLAYERS)),
      turn(0), idxMenu(-1), pBidValue(), pBidCommit(), pScoreDlg(), menuSort(), menuSort2(), menuShowScoreDlg() {
    TRACE9("Jabberwocky::Jabberwocky(Box&, Statusbar&, CardSet&, ...)");

    // Show and attach card-piles
    changeNames(player);
    for (unsigned int i(0); i < NUM_PLAYERS; ++i) {
        players[i].name.set_hexpand();
        players[i].name.set_vexpand();
        players[i].name.set_margin_start(1);
        players[i].name.set_margin_end(1);
        attach(players[i].name, COLS_PLAYER[i], ROWS_PLAYER[i] + (i ? 1 : 3), 3, 1);

        TRACE9("Jabberwocky::Jabberwocky() - Name at: " << COLS_PLAYER[i] << '/' << ROWS_PLAYER[i] + ((i == 2) ? 3 : 1));

        players[i].won.set_margin_start(1);
        players[i].won.set_margin_end(1);
        attach(players[i].won, COLS_PLAYER[i], ROWS_PLAYER[i] + (i ? -2 : 2), 2, 1);
        TRACE9("Jabberwocky::Jabberwocky() - Won pile at: " << COLS_PLAYER[i] << '/' << ROWS_PLAYER[i] + ((i == 2) ? 2 : -2));

        players[i].hand.set_margin_start(1);
        players[i].hand.set_margin_end(1);
        attach(players[i].hand, COLS_PLAYER[i], ROWS_PLAYER[i], 3, 1);
        TRACE9("Jabberwocky::Jabberwocky() - Hand at: " << COLS_PLAYER[i] << '/' << ROWS_PLAYER[i]);

        players[i].won.setShowOption(Card::IPile::SHOWBACK);
        players[i].hand.setShowOption(i ? Card::IPile::SHOWBACK : Card::IPile::SHOWFACE);
        players[i].hand.setStyle(i ? Card::IPile::QUITE_COMPRESSED : Card::IPile::COMPRESSED);
        players[i].won.setStyle(Card::IPile::QUITE_COMPRESSED);
    }
    played.set_margin_top(5);
    played.set_margin_bottom(5);
    attach(played, 3, 6, 8, 3);

    resizeCards();
}

//-----------------------------------------------------------------------------
/// Destructor
//-----------------------------------------------------------------------------
Jabberwocky::~Jabberwocky() { pScoreDlg.reset(); }

//-----------------------------------------------------------------------------
/// Starts the game
//-----------------------------------------------------------------------------
void Jabberwocky::start() {
    TRACE8("Jabberwocky::start()");
    Game::start();
    startPlayer = JabberwockyRules::nextStartPlayer(startPlayer);

    Card::IPile pile;
    if (randomiseCardsToPile(pile)) {
        // Show cards on the table: For all players put 3 cards in hand
        for (unsigned int i(0); i < NUM_PLAYERS; ++i) {
            players[i].bid.undefine();
            players[i].name.set_text(actPlayers[i]->getName());
            for (unsigned int j(0); j < JabberwockyRules::tricksOfRound(turn); ++j)
                players[(i - posServer) % NUM_PLAYERS].hand.setTopCard(pile.removeTopCard());
        }

        pos1Play = pos2Play = -1U;

        pTrump = &pile.removeShownTopCard();
        pTrump->set_margin(5);
        attach(*pTrump, 1, 2, 1, 1);
        pTrump->show();
        TRACE8("Jabberwocky::start() - Trump: " << *pTrump);

        // Set how to sort the colours
        trumpColour = pTrump->colour();

        // Sort cards according to trump
        for (unsigned int i(0); i < NUM_PLAYERS; ++i)
            players[(i - posServer) % NUM_PLAYERS].hand.sort(compByColourAccTrumps);

        // Resetting all status-information
        table.emplace(*pTrump, turn);

        if (!turn && pScoreDlg) {
            menuShowScoreDlg->set_enabled(false);
            pScoreDlg.reset();
        }

        if (getConnectionMgr().getMode() != YGP::ConnectionMgr::CLIENT) {
            setNextPlayer(startPlayer);
            broadcastStartPlayer(startPlayer);
            makeBids();
        }
        pile.clear();

        menuSort->set_enabled(false);
        menuSort2->set_enabled(false);
    }
}

//-----------------------------------------------------------------------------
/// Remove cards from everything which can hold them
//-----------------------------------------------------------------------------
void Jabberwocky::clean() {
    TRACE9("Jabberwocky::clean()");
    for (auto& player : players) { // Clear cards of players
        player.hand.clear();
        player.won.clear();
    }
    played.clear();

    // If a bid is still pending (game ended before the human committed it),
    // remove the leftover entry widgets from the grid
    if (pBidCommit) {
        remove(*pBidValue);
        remove(*pBidCommit);
        pBidValue.reset();
        pBidCommit.reset();
    }

    if (pTrump) {
        remove(*pTrump);
        pTrump = nullptr;
    }
    table.reset();

    menuSort->set_enabled(false);
    menuSort2->set_enabled(false);

    Game::clean();
}

//-----------------------------------------------------------------------------
/// Enables the cards of the passed player
/// \returns bool Flag, if timer should be continued; False
/// \remarks Depending of the status of the game (PLAYING2) also the top card
///     of the played pile is enabled
//-----------------------------------------------------------------------------
bool Jabberwocky::enableHuman() {
    TRACE2("Jabberwocky::enableHuman() - Has " << players[0].hand.size() << " cards");
    Check3(activeCards.empty());
    Check3(gameStatus() == PLAYING);

    for (int i(players[0].hand.size() - 1); i >= 0; --i)
        activeCards.push_back(
            players[0].hand[i]->signal_clicked().connect(bind(mem_fun(*this, (&Jabberwocky::cardSelected)), i)));

    return Game::enableHuman();
}

//-----------------------------------------------------------------------------
/// Makes the move for the next player.
/// \param player Actual player
//-----------------------------------------------------------------------------
void Jabberwocky::makeMove(unsigned int player) {
    TRACE5("Jabberwocky::makeMove() - Turn of player " << player);
    Check3(gameStatus() == PLAYING);

    if (isShowingCardsToPlay()) { // Card of a remote player; already flipped
        const unsigned int pos(pos2Play);
        pos1Play = pos2Play = -1U;

        Card::IPile& hand(players[player].hand);
        table->play(*hand[pos]);
        animateCard(played, hand, pos).sigAnimation.connect(mem_fun(*this, &Jabberwocky::finishMove));
    }
    else
        showCards2Play(player);
}

//-----------------------------------------------------------------------------
/// Finishes the move; calculates the next player and - if necessary -
/// moves the won cards to the winner.
//-----------------------------------------------------------------------------
void Jabberwocky::finishMove() {
    TRACE9("Jabberwocky::finishMove() - ");

    unsigned int next(playCard(currentPlayer()));
    if (played.size() == NUM_PLAYERS)
        Glib::signal_timeout().connect(bind_return(bind(mem_fun(*this, &Jabberwocky::takeWonCards), next), false),
                                       Card::ComputerPlayer::TIMEOUT - 50);

    if (players[next].hand.size()) {
        setNextPlayer(next);
        makeNextMoves();
    }
}

//-----------------------------------------------------------------------------
/// Picks up the won cards
/// \param player Player taking won cards
//-----------------------------------------------------------------------------
void Jabberwocky::takeWonCards(unsigned int player) {
    TRACE9("Jabberwocky::takeWonCards(unsigned int) - " << player);
    Check1(player < NUM_PLAYERS);
    if (played.size() == NUM_PLAYERS) {
        players[player].won.getCards(played, 0, NUM_PLAYERS - 1);

        if (!player) {
            enableWonCards(players[0].won);
            menuSort->set_enabled();
            menuSort2->set_enabled();
        }
    }
}

//-----------------------------------------------------------------------------
/// Shows or hides the cards of the computer player
/// \param open Flag if cards should be shown or hidden
//-----------------------------------------------------------------------------
void Jabberwocky::playOpen(bool open) {
    Card::IPile::ShowOpt show(open ? Card::IPile::SHOWFACE : Card::IPile::SHOWBACK);

    for (unsigned int i(0); i < NUM_PLAYERS; ++i) {
        players[i].won.setShowOption(show);
        players[i].won.setStyle(open ? Card::IPile::COMPRESSED : Card::IPile::VERY_COMPRESSED);
        if (i)
            players[i].hand.setStyle(open ? Card::IPile::COMPRESSED : Card::IPile::VERY_COMPRESSED);
        players[i].hand.setShowOption(i ? show : Card::IPile::SHOWFACE);
    }
}

//-----------------------------------------------------------------------------
/// Changes the names of the playing people
/// \param newPlayer Array holding the new player
//-----------------------------------------------------------------------------
void Jabberwocky::changeNames(const std::vector<Card::Player*>& newPlayer) {
    Game::changeNames(newPlayer);

    std::vector<Card::Player*> player;
    for (unsigned int i(0); i < NUM_PLAYERS; ++i) {
        player.push_back(actPlayers[(i + posServer) % NUM_PLAYERS]);
        players[i].name.set_text(actPlayers[i]->getName());
    }

    if (pScoreDlg)
        pScoreDlg->update(player);
}

//----------------------------------------------------------------------------
/// Converts a pile-number to the actual pile
/// \param newPlayer Array holding the new player
/// \param pile ID of the pile to return
/// \returns Card::IPile* Pile corresponding to the passed number or NULL
//----------------------------------------------------------------------------
Card::IPile* Jabberwocky::getPileOfPlayer(unsigned int player, unsigned int pile) {
    TRACE8("Jabberwocky::getPileOfPlayer(unsigned int, unsigned int) - Player " << player << "; Pile " << pile);
    if ((player >= NUM_PLAYERS) || pile)
        return nullptr;

    return &players[player].hand;
}

//-----------------------------------------------------------------------------
/// Adds game-specific menus
/// \param menu Top-level menu to add the game's submenu to
/// \param actions Action-group ("win"-scoped) to add the game's actions to
//-----------------------------------------------------------------------------
void Jabberwocky::addMenus(const Glib::RefPtr<Gio::Menu>& menu, const Glib::RefPtr<Gio::SimpleActionGroup>& actions) {
    Check1(menu);
    Check1(actions);

    Glib::RefPtr<Gio::Menu> sub(Gio::Menu::create());
    Glib::RefPtr<Gio::Menu> secSort(Gio::Menu::create());
    menuSort = actions->add_action("JabberwockySort", mem_fun(*this, &Jabberwocky::sortWonByNumber));
    secSort->append(_("_Sort won cards (by number)"), "game.JabberwockySort");
    menuSort2 = actions->add_action("JabberwockySortCol", mem_fun(*this, &Jabberwocky::sortWonByColour));
    secSort->append(_("Sort won cards (by _colour)"), "game.JabberwockySortCol");
    sub->append_section(secSort);

    Glib::RefPtr<Gio::Menu> secScore(Gio::Menu::create());
    menuShowScoreDlg = actions->add_action("showScoreDlg", [this]() {
        if (pScoreDlg)
            pScoreDlg->display();
    });
    secScore->append(_("Show score dialog"), "game.showScoreDlg");
    sub->append_section(secScore);

    idxMenu = menu->get_n_items();
    menu->append_submenu(_("_Jabberwocky"), sub);

    menuShowScoreDlg->set_enabled(false);
}

//-----------------------------------------------------------------------------
/// Removes the game-specific menus
/// \param menu Top-level menu to remove the game's submenu from
/// \param actions Action-group to remove the game's actions from
//-----------------------------------------------------------------------------
void Jabberwocky::removeMenus(const Glib::RefPtr<Gio::Menu>& menu, const Glib::RefPtr<Gio::SimpleActionGroup>& actions) {
    Check1(menu);
    Check1(actions);
    if (idxMenu != -1) {
        menu->remove(idxMenu);
        idxMenu = -1;
    }
    actions->remove_action("JabberwockySort");
    actions->remove_action("JabberwockySortCol");
    actions->remove_action("showScoreDlg");
}

//-----------------------------------------------------------------------------
/// Callback after clicking on a card in hand
/// \param pos Offset of card in hand
//-----------------------------------------------------------------------------
void Jabberwocky::cardSelected(unsigned int pos) {
    TRACE5("Jabberwocky::cardSelected(unsigned int) - Position " << pos);
    Check3(pos < players[0].hand.size());
    Check3(gameStatus() == PLAYING);
    Check2(pTrump);

    // Pick up won pile
    if (played.size() == NUM_PLAYERS)
        takeWonCards(0);

    Card::Widget& card(*players[0].hand[pos]);
    TRACE4("Jabberwocky::cardSelected(unsigned int) - Playing " << card);
    const JabberwockyRules::PlayError error(JabberwockyRules::checkPlay(players[0].hand.values(), pos, currentTable()));
    if (error != JabberwockyRules::PlayError::NONE) {
        Gtk::MessageDialog dlg(_(JabberwockyRules::describe(error)), false, Gtk::MessageType::ERROR);
        dlg.set_title(_("Jabberwocky"));
        XGP::runModal(dlg);
        return;
    }

    if (getConnectionMgr().getMode() != YGP::ConnectionMgr::NONE) {
        // Send played card to all clients (if any)
        std::ostringstream msg;
        msg << "Play=" << card.id() << ";Target=0";
        sendMove(msg.str());
    }

    table->play(card);
    animateCard(played, players[0].hand, pos).sigAnimation.connect(mem_fun(*this, &Jabberwocky::finishMove));
}

//-----------------------------------------------------------------------------
/// Makes (or waits) for the bids of the users
/// \param start Number of first player to make its bid
//-----------------------------------------------------------------------------
void Jabberwocky::makeBids(unsigned int start) {
    TRACE8("Jabberwocky::makeBids(unsigned int) - Start: " << start);

    // Make the remaining bids
    while (start < NUM_PLAYERS) {
        unsigned int actPlayer((startPlayer + start) % NUM_PLAYERS);
        TRACE8("Jabberwocky::makeBids(unsigned int) - PlayerID: " << actPlayer);

        if (actPlayer) {
            if (typeid(*actPlayers[actPlayer]) == typeid(Card::ComputerPlayer)) {
                // Estimate the tricks for the player; take care the last player
                // does not place a bid which sums all bids up to the number of tricks
                Check3(pTrump);
                players[actPlayer].bid =
                    JabberwockyRules::selectBid(players[actPlayer].hand.values(), pTrump->colour(), turn,
                                                start == (NUM_PLAYERS - 1), sumOtherBids(actPlayer), cards.size());
                showBid(actPlayer);

                if (getConnectionMgr().getMode() == YGP::ConnectionMgr::SERVER) {
                    std::ostringstream msg;
                    msg << "Bid=" << players[actPlayer].bid << ";Player=" << actPlayer << ';';
                    sendMove(msg.str());
                }
            }
            else {
                Check3(typeid(*actPlayers[actPlayer]) == typeid(Card::RemotePlayer));
                Glib::ustring msg(_("Waiting for %1 to bid ..."));
                msg.replace(msg.find("%1"), 2, actPlayers[actPlayer]->getName());
                status.push(msg);
                return;
            }
        }
        else {
            status.pop();
            status.push(_("Make your bid for the number of tricks you are going to make!"));

            // Remark: Gtk::Statusbar can no longer host arbitrary child widgets
            // under GTK4 (it derives from Gtk::Widget, not Gtk::Box, and offers
            // no packing API), so the bid-entry widgets are attached to this
            // game's own grid instead.
            pBidCommit = std::make_unique<Gtk::Button>(_("_Bid"), true);
            pBidValue = std::make_unique<Gtk::SpinButton>(
                Gtk::Adjustment::create(0, 0.0, JabberwockyRules::tricksOfRound(turn), 1, 2), 1, 0);
            Gtk::Button* bid(pBidCommit.get());
            Gtk::SpinButton* value(pBidValue.get());
            bid->show();
            value->show();

            value->set_margin(5);
            bid->set_margin(5);
            attach(*value, 0, 0);
            attach(*bid, 1, 0);

            bid->signal_clicked().connect(bind(mem_fun(*this, &Jabberwocky::placedBid), value, bid, start + 1));
            return;
        }
        ++start;
    }

    Check2(players[0].bid.isDefined());
    Check2(players[1].bid.isDefined());
    Check2(players[2].bid.isDefined());
    Check2(players[3].bid.isDefined());
    startGame();
}

//-----------------------------------------------------------------------------
/// Returns the sum of the (already placed) bids of the other players
/// \param player Player whose bid should be ignored
/// \returns unsigned int Sum of the bids
//-----------------------------------------------------------------------------
unsigned int Jabberwocky::sumOtherBids(unsigned int player) const {
    unsigned int sum(0);
    for (unsigned int i(0); i < NUM_PLAYERS; ++i)
        if ((i != player) && players[i].bid.isDefined())
            sum += static_cast<unsigned int>(players[i].bid);
    return sum;
}

//-----------------------------------------------------------------------------
/// Returns the state of the actual round (with the played cards of the
/// actual trick)
/// \returns JabberwockyRules::Table& State of the round
//-----------------------------------------------------------------------------
JabberwockyRules::Table& Jabberwocky::currentTable() {
    Check3(table);
    table->trick = played.values();
    return *table;
}

//-----------------------------------------------------------------------------
/// After the initial bidding phase: Start the actual game
//-----------------------------------------------------------------------------
void Jabberwocky::startGame() {
    TRACE2("Jabberwocky::startGame() - " << startPlayer);
    displayTurn(currentPlayer());
    makeNextMoves();
}

//-----------------------------------------------------------------------------
/// Commits the bid of the user and continues bidding
/// \param value Entryfield where user entered his bid
/// \param commit Button commiting the bid
/// \param start Number of first player to make its bid
//-----------------------------------------------------------------------------
void Jabberwocky::placedBid(Gtk::SpinButton* value, Gtk::Button* commit, unsigned int start) {
    TRACE4("Jabberwocky::placedBid(Gtk::SpinButton*, Gtk::Button*, unsigned int) - " << start);
    Check1(commit);
    Check1(value);

    commit->grab_focus();
    players[0].bid = YGP::ANumeric(value->get_text());
    const JabberwockyRules::BidError error(
        JabberwockyRules::checkBid(static_cast<unsigned int>(players[0].bid), sumOtherBids(0), start >= NUM_PLAYERS, turn));
    if (error != JabberwockyRules::BidError::NONE) {
        Gtk::MessageDialog dlg(_(JabberwockyRules::describe(error)), false, Gtk::MessageType::ERROR);
        dlg.set_title(_("Jabberwocky"));
        XGP::runModal(dlg);
    }
    else {
        if (getConnectionMgr().getMode() != YGP::ConnectionMgr::NONE) {
            std::ostringstream msg;
            msg << "Bid=" << players[0].bid << ";Player=" << posServer << ';';
            broadcastMessage(msg.str());
        }

        Check3(value == pBidValue.get());
        Check3(commit == pBidCommit.get());
        remove(*value);
        remove(*commit);
        pBidValue.reset();
        pBidCommit.reset();

        status.pop();
        showBid(0);
        makeBids(start);
    }
}

//-----------------------------------------------------------------------------
/// Shows the bid the passed player has set
/// \param player Player whose bid shall be shown
//-----------------------------------------------------------------------------
void Jabberwocky::showBid(unsigned int player) {
    if (players[player].bid.isDefined()) {
        Glib::ustring tricks(_("; bids %1 tricks"));
        tricks.replace(tricks.find("%1"), 2, players[player].bid.toString());
        players[player].name.set_text(actPlayers[player]->getName() + tricks);
    }
}

//-----------------------------------------------------------------------------
/// Compares the cards in the pile with regard of the colour and with special
/// consideration of trumps
/// \param a Card to compare
/// \param b Card to compare
/// \returns bool True, if a < b
//-----------------------------------------------------------------------------
bool Jabberwocky::compByColourAccTrumps(const Card::Widget* a, const Card::Widget* b) {
    Check3(a);
    Check3(b);
    return JabberwockyRules::lessByColourAccTrump(*a, *b, trumpColour);
}

//-----------------------------------------------------------------------------
/// Shows the card to play
/// \param player Player to inspect
//-----------------------------------------------------------------------------
void Jabberwocky::showCards2Play(unsigned int player) {
    TRACE3("Jabberwocky::showCards2Play(unsigned int) - Player: " << player);
    Check1(player < NUM_PLAYERS);
    Check2(played.size() < NUM_PLAYERS);

    Card::IPile& hand(players[player].hand);
    unsigned int pos2Play(JabberwockyRules::selectCardToPlay(hand.values(), player,
                                                             static_cast<unsigned int>(players[player].bid),
                                                             players[player].won.size() / NUM_PLAYERS, currentTable()));

    // Finally play the card
    Check3(pos2Play < hand.size());
    table->play(*hand[pos2Play]);
    flipCards2Play(hand, pos2Play, pos2Play);

    animateCard(played, hand, pos2Play).sigAnimation.connect(mem_fun(*this, &Jabberwocky::finishMove));
}

//-----------------------------------------------------------------------------
/// Plays a card out of a hand
/// \param player ID of player
/// \returns int Next player or -1 at end
//-----------------------------------------------------------------------------
int Jabberwocky::playCard(unsigned int player) {
    TRACE5("Jabberwocky::playCard(unsigned int) - Player: " << player);
    Check3(player < NUM_PLAYERS);

    // All players have placed their cards
    if (played.size() == NUM_PLAYERS) {
        // Find the winner
        Check3(pTrump);
        const unsigned int bestPlayer(JabberwockyRules::trickWinner(played.values(), pTrump->colour()));
        player = (player - NUM_PLAYERS + bestPlayer + 1) % NUM_PLAYERS;
        TRACE6("Jabberwocky::playCard(unsigned int) - Winner: " << player);
    }
    else
        player = (player + 1) % NUM_PLAYERS;

    // Still cards left: Continue playing
    if (players[player].hand.size())
        displayTurn(player);
    else {
        Glib::ustring stat(_("Game ended"));
        // Create score-dialog
        if (!pScoreDlg) {
            menuShowScoreDlg->set_enabled();
            pScoreDlg.reset(Card::ScoreDlg::create(actPlayers));
            if (Gtk::Window* win = dynamic_cast<Gtk::Window*>(get_root()))
                pScoreDlg->set_transient_for(*win);
        }
        // Add points, if bid has been met; take care of the cards won in the
        // last round
        std::array<unsigned int, NUM_PLAYERS> bids, tricks;
        for (unsigned int i(0); i < NUM_PLAYERS; ++i) {
            bids[i] = static_cast<unsigned int>(players[i].bid);
            tricks[i] = (players[i].won.size() / NUM_PLAYERS) + (player == i);
        }
        std::array<int, NUM_PLAYERS> points(JabberwockyRules::roundScore(bids, tricks));
        pScoreDlg->addPoints(points.data());
        pScoreDlg->display();

        // Stop after 13 rounds
        if (++turn == JabberwockyRules::NUM_ROUNDS) {
            turn = 0;

            int points;
            pScoreDlg->getMaxPoints(points, player);
            Check3(points >= 0);
            Glib::ustring won(_("; %1 won"));
            won.replace(won.find("%1"), 2, actPlayers[player]->getName());
            stat += won;
        }

        status.pop();
        status.push(stat);
        setGameStatus(STOPPED);
    }
    return player;
}

//----------------------------------------------------------------------------
/// Handles the messages the server might send for the Jabberwocky cardgame
/// \param player ID of player sending the message
/// \param message Message received from the server
/// \returns bool True, if message has been completey processed
//----------------------------------------------------------------------------
bool Jabberwocky::handleMessage(unsigned int player, const std::string& message) {
#ifdef WITH_NETWORK
    const std::string_view cmd(Card::commandOf(message));

    // Bid=<number of tricks>;Player=<position of player (as seen from the server)>
    if (cmd == "Bid") {
        TRACE5("Jabberwocky::handleMessage(unsigned int, const std::string&) - " << message);

        const auto fields(Card::splitMessage(message));
        unsigned long sender(0), bid(0);
        if ((fields.size() < 2) || (fields[1].key != "Player") || stringToNumber(sender, fields[1].value.c_str()) ||
            (sender >= NUM_PLAYERS) || stringToNumber(bid, fields[0].value.c_str()) ||
            (bid > JabberwockyRules::tricksOfRound(turn)))
            throw YGP::ParseError(N_("Invalid bid!"));

        // Inform the other clients
        if (getConnectionMgr().getMode() == YGP::ConnectionMgr::SERVER)
            broadcastMessage(message);

        // Ignore the own bid (echoed by the server)
        if (sender != posServer) {
            unsigned int lPlayer((sender - posServer) & 0x3);
            if (players[lPlayer].bid.isDefined())
                throw YGP::ParseError(N_("Invalid bid!"));

            players[lPlayer].bid = bid;
            showBid(lPlayer);
            status.pop();

            // Continue with bidding; but first correct the player with whom to start
            makeBids((((++lPlayer) % NUM_PLAYERS) == startPlayer) ? NUM_PLAYERS
                                                                  : ((lPlayer + NUM_PLAYERS - startPlayer) % NUM_PLAYERS));
        }
        return true;
    }
#endif

    bool rc(Game::handleMessage(player, message));
#ifdef WITH_NETWORK
    // The client starts bidding, after receiving the startplayer
    if ((cmd == "ActPlayer") && (getConnectionMgr().getMode() == YGP::ConnectionMgr::CLIENT)) {
        startPlayer = currentPlayer();
        makeBids();
    }
#endif
    return rc;
}

//-----------------------------------------------------------------------------
/// Actions to take when the cards are resized
/// \pre The cardsize must be set in CardImages::WIDTH/HEIGHT
//-----------------------------------------------------------------------------
void Jabberwocky::resizeCards() {
    played.set_size_request(Card::Images::WIDTH + 150, Card::Images::HEIGHT);
    for (unsigned int i(0); i < NUM_PLAYERS; ++i) {
        players[i].won.set_size_request(Card::Images::WIDTH + 20, Card::Images::HEIGHT + 5);
        players[i].hand.set_size_request((i & 1) ? Card::Images::WIDTH + 8 * 5 : Card::Images::WIDTH * 3,
                                         Card::Images::HEIGHT + 5);
    }
}
