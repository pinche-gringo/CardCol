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
#include <card/Player.h>
#include <card/Random.h>
#include <card/RemotePlayer.h>
#include <card/ScoreDlg.h>
#include <card/Window.h>

#include "Jabberwocky.h"

std::array<char, 4> Jabberwocky::sortOrder{};

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
                         const std::vector<Card::Player*>& player, unsigned int posPlayer, YGP::Mutex& mxSerialize)
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
    startPlayer = (startPlayer + 1) % NUM_PLAYERS;

    Card::IPile pile;
    if (randomiseCardsToPile(pile)) {
        // Show cards on the table: For all players put 3 cards in hand
        for (unsigned int i(0); i < NUM_PLAYERS; ++i) {
            players[i].bid.undefine();
            players[i].name.set_text(actPlayers[i]->getName());
            for (unsigned int j(0); j < getTricks(turn); ++j)
                players[(i - posServer) % NUM_PLAYERS].hand.setTopCard(pile.removeTopCard());
        }

        pos1Play = pos2Play = -1U;

        pTrump = &pile.removeShownTopCard();
        pTrump->set_margin(5);
        attach(*pTrump, 1, 2, 1, 1);
        pTrump->show();
        TRACE8("Jabberwocky::start() - Trump: " << *pTrump);

        // Set how to sort the colours
        for (unsigned int i(0); i < 4; ++i)
            sortOrder[i] = (i - pTrump->colour() + 3) % NUM_PLAYERS;

        // Sort cards according to trump
        for (unsigned int i(0); i < NUM_PLAYERS; ++i)
            players[(i - posServer) % NUM_PLAYERS].hand.sort(compByColourAccTrumps);

        // Resetting all status-information
        for (auto& playedCard : playedCards)
            playedCard.reset();
        playedCards[pTrump->colour()].set(pTrump->number());
        outOfColour = {};

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

    try {
        Card::Widget& card(*players[0].hand[pos]);
        TRACE4("Jabberwocky::cardSelected(unsigned int) - Playing " << card);
        if (played.size()) {
            if ((card.colour() != played[0]->colour()) && players[0].hand.exists(played[0]->colour()))
                throw _("Play first cards with an equal colour as the first played one!");
        }
        else
            // One can start with a trump only if there hasn't been one played before
            // Note that also the cards shown as trump is counted as played, so
            // test accordingly
            if (((card.colour() == pTrump->colour()) && (playedCards[card.colour()].count() == 1) &&
                 ((players[0].hand)[0]->colour() != pTrump->colour())))
                throw _("You can't start with a trump,\nif they have not been played before!");

        if (getConnectionMgr().getMode() != YGP::ConnectionMgr::NONE) {
            // Send played card to all clients (if any)
            std::ostringstream msg;
            msg << "Play=" << card.id() << ";Target=0";
            if (getConnectionMgr().getMode() == YGP::ConnectionMgr::CLIENT)
                ignoreNextMsg = true;
            broadcastMessage(msg.str());
        }

        playedCards[players[0].hand[pos]->colour()].set(players[0].hand[pos]->number());

        animateCard(played, players[0].hand, pos).sigAnimation.connect(mem_fun(*this, &Jabberwocky::finishMove));
    }
    catch (Glib::ustring& error) {
        Gtk::MessageDialog dlg(error, false, Gtk::MessageType::ERROR);
        dlg.set_title(_("Jabberwocky"));
        XGP::runModal(dlg);
    }
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
                // does not place a bid which sums all bids up to the number of players
                players[actPlayer].bid = calcTricks(actPlayer);
                if ((start == NUM_PLAYERS) && (sumBids() == getTricks(turn)))
                    players[actPlayer].bid += Card::randomNumber(2) ? 1 : -1;
                showBid(actPlayer);

                if (getConnectionMgr().getMode() == YGP::ConnectionMgr::SERVER) {
                    std::ostringstream msg;
                    msg << "Bid=" << players[actPlayer].bid << ";Player=" << actPlayer << ';';
                    if (getConnectionMgr().getMode() == YGP::ConnectionMgr::CLIENT)
                        ignoreNextMsg = true;
                    broadcastMessage(msg.str());
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
            pBidValue = std::make_unique<Gtk::SpinButton>(Gtk::Adjustment::create(0, 0.0, getTricks(turn), 1, 2), 1, 0);
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
/// Returns the sum of all bids
/// \returns unsinged int Sum o fall bids
//-----------------------------------------------------------------------------
unsigned int Jabberwocky::sumBids() const {
    unsigned int sum(0);
    for (const auto& player : players)
        sum += static_cast<unsigned int>(player.bid);
    return sum;
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
    if ((start >= NUM_PLAYERS) && (sumBids() == getTricks(turn))) {
        Gtk::MessageDialog dlg(_("The sum of all bids must be different\nthan the number of possible tricks!"), false,
                               Gtk::MessageType::ERROR);
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
/// Calculates the bids for the passed player
/// \param player Number of player to calculate tricks to make for
/// \returns unsigned int Number of tricks player will win
//-----------------------------------------------------------------------------
unsigned int Jabberwocky::calcTricks(unsigned int player) const {
    TRACE5("Jabberwocky::calcTricks(unsigned int) - " << player);
    Check1(player < NUM_PLAYERS);
    Check3(pTrump);

    // Analyse cards to estimate tricks it will win
    unsigned int tricks(0);
    unsigned int left(cards.size() - 1 - NUM_PLAYERS * getTricks(turn));

    for (auto i : players[player].hand) {
        if (i->colour() == pTrump->colour()) {
            if ((i->number() > Card::Widget::EIGHT) || (left > 19))
                ++tricks;
            ++tricks;
        }
        else if (isHighEnough(*i))
            tricks += 2;
    }

    return tricks >> 1;
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
    return ((a->colour() == b->colour()) ? a->number() < b->number() : (sortOrder[a->colour()] < sortOrder[b->colour()]));
}

//-----------------------------------------------------------------------------
/// Shows the card to play
/// \param player Player to inspect
//-----------------------------------------------------------------------------
void Jabberwocky::showCards2Play(unsigned int player) {
    TRACE3("Jabberwocky::showCards2Play(unsigned int) - Player: " << player);
    Check1(player < NUM_PLAYERS);
    Check2(played.size() < NUM_PLAYERS);
    unsigned int pos2Play(-1U);

    Card::IPile& hand(players[player].hand);
    ColourPositions aPosColours;
    getPositionOfColours(hand, aPosColours);
    TRACE3("Jabberwocky::showCards2Play(unsigned int) - Missing tricks: "
           << (static_cast<int>(players[player].bid) - (players[player].won.size() / NUM_PLAYERS)));

    // Already cards played?
    if (played.size()) {
        unsigned int posWinner(check4Winner());
        TRACE4("Jabberwocky::showCards2Play(unsigned int) - Winning card: " << posWinner << " (" << *played[posWinner] << ')');

        // If the player still has bids to fullfill
        if (static_cast<unsigned int>(players[player].bid) > (players[player].won.size() / NUM_PLAYERS)) {
            // Can follow suit?
            if (aPosColours[played[0]->colour()] == -1) {
                TRACE7("Jabberwocky::showCards2Play(unsigned int) - Can't follow suit");
                outOfColour[player][played[0]->colour()] = true;

                unsigned int p(NUM_PLAYERS - played.size());
                Check3(p);
                while (--p) {
                    if (outOfColour[(player + p) % NUM_PLAYERS][played[0]->colour()]) {
                        TRACE9("Jabberwocky::showCards2Play(unsigned int) - Out of suit: " << p);
                        // Check if player can get the pile
                        pos2Play =
                            (((aPosColours[pTrump->colour()] != -1) && (isHighest(*hand[aPosColours[pTrump->colour()]]) ||
                                                                        (isHighEnough(*hand[aPosColours[pTrump->colour()]]))))
                                 ? aPosColours[pTrump->colour()]
                                 : findWorstCard(hand, aPosColours));
                        Check3(pos2Play != -1U);
                        break;
                    }
                }
                // Other players still seem to have the colour
                if (!p) {
                    // Try to get the card with a trump; else put worst card
                    if ((aPosColours[pTrump->colour()] == -1) ||
                        ((played[posWinner]->colour() == pTrump->colour())
                             ? ((pos2Play = findHigherCard(*played[posWinner], hand, aPosColours[played[posWinner]->colour()])) ==
                                -1U)
                             : ((pos2Play = hand.findFirstEqualColour(aPosColours[pTrump->colour()])) == -1U)))
                        pos2Play = findWorstCard(hand, aPosColours);
                    Check3(pos2Play != -1U);
                }
                TRACE5("Jabberwocky::showCards2Play(unsigned int) - Can't follow suit: " << pos2Play);
            }
            // Can follow suit
            else {
                TRACE5("Jabberwocky::showCards2Play(unsigned int) - Can follow suit: " << aPosColours[played[0]->colour()]);

                // If a trump has been played after non-trump, play something low
                if (played[posWinner]->colour() != played[0]->colour())
                    pos2Play = hand.findFirstEqualColour(aPosColours[played[0]->colour()]);
                else {
                    // Last in turn
                    if (played.size() == (NUM_PLAYERS - 1))
                        pos2Play = ((hand[aPosColours[played[0]->colour()]]->number() > played[posWinner]->number())
                                        ? findHigherCard(*played[posWinner], hand, aPosColours[played[0]->colour()])
                                        : hand.findFirstEqualColour(aPosColours[played[0]->colour()]));
                    else if ((hand[aPosColours[played[0]->colour()]]->number() > played[posWinner]->number()) &&
                             (isHighest(*hand[aPosColours[played[0]->colour()]]) ||
                              (isHighEnough(*hand[aPosColours[played[0]->colour()]]))))
                        pos2Play = aPosColours[played[0]->colour()];
                    else
                        pos2Play = hand.findFirstEqualColour(aPosColours[played[0]->colour()]);
                }
                TRACE5("Jabberwocky::showCards2Play(unsigned int) - Can follow suit - End: " << pos2Play);
            }
        }
        // Doesn't need any more tricks
        else if ((aPosColours[played[0]->colour()] != -1) && (played[0]->colour() == played[posWinner]->colour()))
            pos2Play = findLowerCard(*played[posWinner], hand, aPosColours[played[posWinner]->colour()]);
        else {
            unsigned int offset(0);
            for (unsigned int i(1); i < aPosColours.size(); ++i) {
                TRACE5("Jabberwocky::showCards2Play(unsigned int) - No more tricks; Colour: " << i);
                if ((static_cast<Card::Widget::COLOURS>(i) != pTrump->colour()) && (aPosColours[i] != -1) &&
                    ((aPosColours[offset] == -1) || ((hand[aPosColours[i]]->number() > hand[offset]->number()) ||
                                                     ((hand[aPosColours[i]]->number() == hand[offset]->number()) &&
                                                      (playedCards[hand[aPosColours[i]]->colour()].count() <
                                                       playedCards[hand[aPosColours[offset]]->colour()].count())))))
                    offset = i;
            }
            pos2Play = (aPosColours[offset] == -1) ? aPosColours[pTrump->colour()] : aPosColours[offset];
        }
    }
    // First card to play
    else {
        // If the player still has bids to fullfill
        if (static_cast<unsigned int>(players[player].bid) < (players[player].won.size() / NUM_PLAYERS)) {
            // Try to eliminate trumps
            if (playedCards[pTrump->colour()].count() && (aPosColours[pTrump->colour()] != -1) &&
                (isHighest(*hand[aPosColours[pTrump->colour()]]) || isHighEnough(*hand[aPosColours[pTrump->colour()]])))
                pos2Play = aPosColours[pTrump->colour()];
            else
                for (unsigned int i(0); i < 4; ++i)
                    if (static_cast<Card::Widget::COLOURS>(i) != pTrump->colour())
                        if ((aPosColours[static_cast<Card::Widget::COLOURS>(i)] != -1) &&
                            (isHighest(*hand[aPosColours[static_cast<Card::Widget::COLOURS>(i)]]) ||
                             isHighEnough(*hand[aPosColours[static_cast<Card::Widget::COLOURS>(i)]])))
                            pos2Play = aPosColours[static_cast<Card::Widget::COLOURS>(i)];
        }

        if (pos2Play == -1U)
            pos2Play = findWorstCard(hand, aPosColours);
    }

    // Finally play the card
    Check3(pos2Play < hand.size());
    TRACE1("Jabberwocky::showCards2Play(unsigned int) - Playing: " << pos2Play << " (" << *hand[pos2Play] << ')');
    playedCards[players[player].hand[pos2Play]->colour()].set(players[player].hand[pos2Play]->number());
    flipCards2Play(hand, pos2Play, pos2Play);

    animateCard(played, players[player].hand, pos2Play).sigAnimation.connect(mem_fun(*this, &Jabberwocky::finishMove));
}

//-----------------------------------------------------------------------------
/// Find the highest card lower than the passed one or the lowest card
/// of the colour of the passed card to match
/// \param cardCmp Card which should not be passed
/// \param pile Pile from which to play
/// \param aPosColour Position of last card in the pile with that colour
/// \returns unsigned int Position of card to play
//-----------------------------------------------------------------------------
unsigned int Jabberwocky::findLowerCard(const Card::Widget& cardCmp, const Card::IPile& pile, int aPosColour) const {
    TRACE9("Jabberwocky::findLowerCard(const Card::Widget&, const Card::IPile&, int) - " << aPosColour);
    Check1(static_cast<unsigned int>(aPosColour) < pile.size());
    Check2(pile[aPosColour]->colour() == cardCmp.colour());
    Check3(played.size());

    // Search for a lower card
    while (aPosColour >= 0 && (pile[aPosColour]->colour() == cardCmp.colour())) {
        if (pile[aPosColour]->number() < cardCmp.number()) {
            TRACE5("Jabberwocky::findLowerCard(const Card::Widget&, const Card::IPile&, int) - Playing card at "
                   << aPosColour << ": " << *pile[aPosColour]);
            return aPosColour;
        }
        --aPosColour;
    }

    TRACE5("Jabberwocky::findLowerCard(const Card::Widget&, const Card::IPile&, int) - Forced to play card at "
           << aPosColour + 1 << ": " << *pile[aPosColour + 1]);
    return aPosColour + 1;
}

//-----------------------------------------------------------------------------
/// Find the lowest card higher than the passed one
/// \param cardCmp Card which should not be passed
/// \param pile Pile from which to play
/// \param aPositions Array with positions of cards
/// \returns unsigned int Position of card to play
//-----------------------------------------------------------------------------
unsigned int Jabberwocky::findHigherCard(const Card::Widget& cardCmp, const Card::IPile& pile, unsigned int aPosColour) const {
    TRACE9("Jabberwocky::findHigherCard(const Card::Widget&, const Card::IPile&, unsigned int)");
    Check1(aPosColour < pile.size());
    Check2(pile[aPosColour]->colour() == cardCmp.colour());
    Check3(played.size());

    unsigned int pos(-1U);
    while (pile[aPosColour]->number() > cardCmp.number()) {
        pos = aPosColour;
        if (!aPosColour-- || (pile[aPosColour]->colour() != cardCmp.colour()))
            break;
    } // end-while

    TRACE5("Jabberwocky::findHigherCard(const Card::Widget&, const Card::IPile&, unsigned int) - Playing card at " << pos);
    return pos;
}

//-----------------------------------------------------------------------------
/// Checks who has played the highest card and would therefore win the played
/// pile
/// \returns \c ID of player with the highest card
//-----------------------------------------------------------------------------
unsigned int Jabberwocky::check4Winner() const {
    Check2(played.size());
    Check2(pTrump);
    Card::Widget::COLOURS colour(played[0]->colour());
    Card::Widget::NUMBERS highest(played[0]->number());
    unsigned int pos(0);
    for (unsigned int i(1); i < played.size(); ++i)
        if ((played[i]->colour() == colour) && (played[i]->number() > highest)) {
            pos = i;
            highest = played[i]->number();
            TRACE9("Jabberwocky::check4Winner(unsinged int, unsinged int) - "
                   "New high card at "
                   << i);
        }
        else {
            if (played[i]->colour() == pTrump->colour()) {
                pos = i;
                highest = played[i]->number();
                colour = pTrump->colour();
                TRACE9("Jabberwocky::check4Winner(unsinged int, unsinged int) - Trump wins at " << i);
            }
        }

    return pos;
}

//-----------------------------------------------------------------------------
/// Find the worst card to play (e.g. card not likely to win the trick)
/// \param pile Pile from which to play
/// \param aPositions Array with positions of cards
/// \returns unsigned int Position of card to play or -1
//-----------------------------------------------------------------------------
unsigned int Jabberwocky::findWorstCard(const Card::IPile& pile, const ColourPositions& aPositions) const {
    TRACE9("Jabberwocky::findWorstCard(const Card::IPile&, const int[4])");
    Check1(pile.size());

    // Special handling of a pile full of trumps
    if (pile[0]->colour() == pTrump->colour())
        return 0;

    // Find lowest card (ignoring trumps)
    unsigned int pos(0);
    for (unsigned i(0); i < 4; ++i)
        if ((aPositions[i] != -1) && (static_cast<unsigned int>(aPositions[i]) < (pile.size() - 1)) &&
            pile[aPositions[i] + 1]->colour() != pTrump->colour())
            if ((pile[aPositions[i] + 1]->number() < pile[pos]->number()) ||
                ((pile[aPositions[i] + 1]->number() == pile[pos]->number()) &&
                 (playedCards[pile[aPositions[i] + 1]->colour()].count() < playedCards[pile[pos]->colour()].count())))
                pos = aPositions[i] + 1;

    TRACE8("Jabberwocky::findWorstCard(const Card::IPile&, const int[4]) - " << pos);
    return pos;
}

//----------------------------------------------------------------------------
/// Checks if the passed card might be the highest, considering the unused and
/// played cards.
/// \param card Card to inspect
/// \return bool True, if card is likely highest unplayed one
//----------------------------------------------------------------------------
bool Jabberwocky::isHighEnough(const Card::Widget& card) const {
    TRACE8("Jabberwocky::isHighEnough(const Card::Widget&) - " << card);
    return (card.number() >= static_cast<Card::Widget::NUMBERS>(Card::Widget::TEN + ((getTricks(turn) - 3) >> 1)));
}

//----------------------------------------------------------------------------
/// Checks if the passed card is the highest card of its colour, which has
/// not been played.
/// \param card Card to inspect
/// \return bool True, if card is the highest unplayed one
//----------------------------------------------------------------------------
bool Jabberwocky::isHighest(const Card::Widget& card) const {
    TRACE8("Jabberwocky::isHighest(const Card::Widget&) - " << card);

    int nr(card.number());
    while (++nr <= Card::Widget::ACE) {
        if (!playedCards[card.colour()][nr])
            return false;
    }
    return true;
}
//-----------------------------------------------------------------------------
/// Stores the last position of each colour in the pile
/// \param pile Pile to inspect
/// \param result Array of position of last cards of earch colour
//-----------------------------------------------------------------------------
void Jabberwocky::getPositionOfColours(const Card::IPile& pile, ColourPositions& result) {
    result.fill(-1);
    for (unsigned int i(0); i < (pile.size() - 1); ++i)
        if (pile[i]->colour() != pile[i + 1]->colour())
            result[pile[i]->colour()] = i;
    result[pile[pile.size() - 1]->colour()] = pile.size() - 1;

    TRACE9("Jabberwocky::getPositionOfColours(const Card::IPile&, unsigned int) - Pos. of "
           "cards: "
           << result[0] << ", " << result[1] << ", " << result[2] << ", " << result[3]);
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
        auto i(played.begin());
        Card::Widget::NUMBERS nr((*i)->number());
        Card::Widget::COLOURS col((*i)->colour());
        unsigned int bestPlayer(0);

        Check3(pTrump);
        while (++i != played.end()) {
            TRACE8("Jabberwocky::playCard(unsigned int) - Comparing " << (*played[player]) << " - " << **i);

            if ((col != pTrump->colour()) && ((*i)->colour() == pTrump->colour())) {
                TRACE9("Jabberwocky::playCard(unsigned int) - Found trump ");
                col = pTrump->colour();
                nr = (*i)->number();
                bestPlayer = i - played.begin();
                continue;
            }

            if (((*i)->number() > nr) && ((*i)->colour() == col)) {
                TRACE9("Jabberwocky::playCard(unsigned int) - New best card " << **i);
                nr = (*i)->number();
                bestPlayer = i - played.begin();
            }
        }
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
        std::array<int, NUM_PLAYERS> points;
        for (unsigned int i(0); i < NUM_PLAYERS; ++i)
            points[i] = (static_cast<unsigned int>(players[i].bid) == ((players[i].won.size() / NUM_PLAYERS) + (player == i)));
        pScoreDlg->addPoints(points.data());
        pScoreDlg->display();

        // Stop after 13 rounds
        if (++turn == 13) {
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
#if 0
   Card::Tokenize command (message);
   std::string cmd (command.getNextNode ('='));

   if (cmd == "Bid") {
      TRACE5 ("Jabberwocky::handleMessage (unsigned int, const std::string&) - " << message);

      std::string value (command.getNextNode (';'));
      cmd = command.getNextNode ('=');
      unsigned long lPlayer (player);
      if ((cmd == "Player")
	  && !stringToNumber (lPlayer, command.getNextNode (';').c_str ())
	  && (lPlayer < NUM_PLAYERS)) {
	 lPlayer = (lPlayer - posServer) & 0x3;
	 unsigned long bid;
	 if ((!stringToNumber (bid, value.c_str ()))
	     || (bid > getTricks (turn)) || players[lPlayer].bid.isDefined ()) {
	    players[lPlayer].bid = bid;
	    showBid (lPlayer);
	    status.pop ();

	    // Continue with bidding; but first correct the player with whom to start
	    makeBids (((++lPlayer % NUM_PLAYERS) == startPlayer)
		      ? NUM_PLAYERS : ((lPlayer + NUM_PLAYERS - startPlayer) % NUM_PLAYERS));
	    return true;
	 }
      }
   }
#endif
    bool rc(Game::handleMessage(player, message));
#if 0
   if ((cmd == "ActPlayer")
       && (getConnectionMgr ().getMode () == YGP::ConnectionMgr::CLIENT)) {
      startPlayer = currentPlayer ();
      makeBids ();
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
