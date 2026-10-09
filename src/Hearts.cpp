// PROJECT     : Cardgames
// SUBSYSTEM   : Hearts
// REFERENCES  :
// TODO        :
// BUGS        :
// AUTHOR      : Markus Schwab
// CREATED     : 24.12.2002
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

#include <algorithm>
#include <iterator>
#include <memory>
#include <sstream>

#include <cardgames-cfg.h>

#include <glibmm/main.h>

#include <gtkmm/messagedialog.h>
#include <gtkmm/statusbar.h>

#include <giomm/menu.h>
#include <giomm/simpleactiongroup.h>

#include <XGP/XDialog.h>

#include <YGP/Check.h>
#include <YGP/ConnMgr.h>
#include <YGP/Trace.h>

#include <card/ComputerPlayer.h>
#include <card/Images.h>
#include <card/Message.h>
#include <card/ScoreDlg.h>
#include <card/Window.h>

#include "Hearts.h"

unsigned int Hearts::ENDPOINTS(100);

namespace {

/// Delay (in ms) before animating cards, which have just been added or moved;
/// so that their new position has been layouted
constexpr unsigned int LAYOUT_DELAY(50);

//-----------------------------------------------------------------------------
/// Creates a pile of the passed type and stores it in the passed owner
/// \param owner Smart pointer taking over the ownership of the created pile
/// \returns T& Reference to the created pile
//-----------------------------------------------------------------------------
template <class T> T& createPile(std::unique_ptr<Card::IPile>& owner) {
    auto pile(std::make_unique<T>());
    T& result(*pile);
    owner = std::move(pile);
    return result;
}

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
Hearts::Hearts(Gtk::Box& parent, Gtk::Statusbar& statusbar, Card::Set& cardset, const std::vector<Card::Player*>& player,
               unsigned int posPlayer, Card::MessageLock& mxSerialize)
    : Game(parent, statusbar, cardset, player, posPlayer, mxSerialize, 18, 12), playedSQ(false), player2Exchange(3),
      played(Card::IPile::COMPRESSED, Card::IPile::SHOWFACE), pScoreDlg(nullptr), menuSort(), menuSort2(), menuShowScoreDlg() {
    TRACE9("Hearts::Hearts(Box&, Statusbar&, Card::Set&, ...");
    Card::HPile& hand0(createPile<Card::HPile>(players[0].hand));
    Card::HPile& won0(createPile<Card::HPile>(players[0].won));
    Card::HPile& hand2(createPile<Card::HPile>(players[2].hand));
    Card::HPile& won2(createPile<Card::HPile>(players[2].won));

    Card::VPile& hand1(createPile<Card::VPile>(players[1].hand));
    Card::VPile& won1(createPile<Card::VPile>(players[1].won));
    Card::VPile& hand3(createPile<Card::VPile>(players[3].hand));
    Card::VPile& won3(createPile<Card::VPile>(players[3].won));

    attach(won0, COLS_PLAYER[0], ROWS_PLAYER[0] + 4, 3, 1);
    attach(hand0, COLS_PLAYER[0], ROWS_PLAYER[0], 3, 1);

    attach(won1, COLS_PLAYER[1] + 2, ROWS_PLAYER[1], 1, 1);
    attach(hand1, COLS_PLAYER[1], ROWS_PLAYER[1], 1, 1);

    attach(won2, COLS_PLAYER[2], ROWS_PLAYER[2] - 2, 3, 1);
    attach(hand2, COLS_PLAYER[2], ROWS_PLAYER[2], 3, 1);

    attach(won3, COLS_PLAYER[3] - 2, ROWS_PLAYER[3], 1, 1);
    attach(hand3, COLS_PLAYER[3], ROWS_PLAYER[3], 1, 1);

    // Show and attach card-piles
    changeNames(player);
    for (unsigned int i(0); i < NUM_PLAYERS; ++i) {
        players[i].name.set_margin(1);
        attach(players[i].name, COLS_PLAYER[i], ROWS_PLAYER[i] + 2, (i & 1) ? 1 : 3, 1);

        players[i].won->setShowOption(Card::IPile::SHOWBACK);
        players[i].hand->setShowOption(i ? Card::IPile::SHOWBACK : Card::IPile::SHOWFACE);

        players[i].hand->setStyle((i & 1) ? Card::IPile::QUITE_COMPRESSED : Card::IPile::COMPRESSED);
        players[i].won->setStyle(Card::IPile::VERY_COMPRESSED);
    }

    // Show played area
    played.setStyle(Card::IPile::COMPRESSED);
    played.set_margin(5);
    played.set_halign(Gtk::Align::START);
    played.set_valign(Gtk::Align::START);
    attach(played, 6, 7, 1, 1);

    resizeCards();
}

//-----------------------------------------------------------------------------
/// Destructor
//-----------------------------------------------------------------------------
Hearts::~Hearts() {
    TRACE9("Hearts::~Hearts()");
    pScoreDlg.reset();
    clean();
}

//-----------------------------------------------------------------------------
/// Makes the move for the next player.
/// \param player Actual player
//-----------------------------------------------------------------------------
void Hearts::makeMove(unsigned int player) {
    TRACE5("Hearts::makeMove() - Turn of player " << player);
    Check1(gameStatus() == PLAYING);

    Card::IPile& pile(*players[player].hand);
    unsigned int pos(-1U);
    if (isShowingCardsToPlay()) { // Card of a remote player; already flipped
        pos = pos2Play;
        pos1Play = pos2Play = -1U;
    }
    else {
        pos = HeartsRules::selectCardToPlay(pile.values(), currentTable());
        flipCards2Play(pile, pos, pos);
    }
    TRACE8("Hearts::makeMove(unsigned int) - Going to play card at pos " << pos);
    Check3(pos < pile.size());

    aPlayed[pile[pos]->colour()]++;
    if (pile[pos]->is(Card::Widget::SPADES, Card::Widget::QUEEN))
        playedSQ = true;

    animateCard(played, pile, pos).sigAnimation.connect(mem_fun(*this, &Hearts::finishMove));
}

//-----------------------------------------------------------------------------
/// Finishes the move; calculates the next player and - if necessary -
/// moves the won cards to the winner.
//-----------------------------------------------------------------------------
void Hearts::finishMove() {
    unsigned int next(calcNextPlayer(currentPlayer()));

    // Show a complete trick for a moment, before the winner takes it (and continues)
    if (played.size() == NUM_PLAYERS)
        Glib::signal_timeout().connect(bind_return(bind(mem_fun(*this, &Hearts::takeWonCards), next), false),
                                       Card::ComputerPlayer::TIMEOUT - 50);
    else
        nextMove(next);
}

//-----------------------------------------------------------------------------
/// Lets the passed player make his move (if he has cards left)
/// \param player Player in turn
//-----------------------------------------------------------------------------
void Hearts::nextMove(unsigned int player) {
    if (players[player].hand->size()) {
        setNextPlayer(player);
        makeNextMoves();
    }
}

//-----------------------------------------------------------------------------
/// Moves the won cards to the passed player
/// \param player Player taking won cards
//-----------------------------------------------------------------------------
void Hearts::takeWonCards(unsigned int player) {
    TRACE9("Hearts::takeWonCards(unsigned int) - " << player);
    Check1(player < NUM_PLAYERS);

    // Remark: Do nothing, if the game has been ended meanwhile
    if (played.size() == NUM_PLAYERS)
        animateCards(*players[player].won, played, 0, NUM_PLAYERS - 1, 0)
            .sigAnimation.connect(bind(mem_fun(*this, &Hearts::trickTaken), player));
}

//-----------------------------------------------------------------------------
/// Actions after the won cards have been moved to the passed player; he
/// continues the game
/// \param player Player who took the won cards
//-----------------------------------------------------------------------------
void Hearts::trickTaken(unsigned int player) {
    TRACE9("Hearts::trickTaken(unsigned int) - " << player);
    if (!player) {
        enableWonCards(*players[0].won);
        menuSort->set_enabled();
        menuSort2->set_enabled();
    }
    nextMove(player);
}

//-----------------------------------------------------------------------------
/// Starts the game by dealing the cards
//-----------------------------------------------------------------------------
void Hearts::start() {
    TRACE9("Hearts::start()");
    Game::start();

    // Hide won pile again (if not in debug-mode)
#if TRACELEVEL > 0
    if (players[1].won->getShowOption() == Card::IPile::SHOWBACK)
#endif
        showWonCards(false);

    Check2(!played.size());
    Card::IPile pile;
    if (randomiseCardsToPile(pile)) {
        for (unsigned int i(0); i < NUM_PLAYERS; ++i) {
            Card::IPile* actPile(players[(i - posServer) & 0x3].hand.get());
            actPile->getCards(pile, 0, cards.size() / NUM_PLAYERS - 1);
            actPile->sortByColour();
        }

        if (pScoreDlg) {
            unsigned int player;
            int points;
            pScoreDlg->getMaxPoints(points, player);
            if (static_cast<unsigned int>(points) >= ENDPOINTS) {
                menuShowScoreDlg->set_enabled(false);
                pScoreDlg.reset();
            }
        }

        if (player2Exchange) {
            Glib::ustring stat(_("Select 3 cards to exchange with %1"));
            Check3(actPlayers.size() > player2Exchange);
            Check3(actPlayers[player2Exchange & 0x3]);
            stat.replace(stat.find("%1"), 2, actPlayers[player2Exchange]->getName());
            status.pop();
            status.push(stat);
            setGameStatus(EXCHANGE);
            setNextPlayer(0);
            enableHuman();
        }
        else
            startPlaying();
    }
}

//-----------------------------------------------------------------------------
/// Remove cards from everything which can hold them
//-----------------------------------------------------------------------------
void Hearts::clean() {
    TRACE9("Hearts::clean()");
    for (auto& player : players) {
        player.hand->clear();
        player.won->clear();
    }

    played.clear();
    for (auto& exchange : aExchange)
        exchange.clear();
    Game::clean();

    menuSort->set_enabled(false);
    menuSort2->set_enabled(false);
}

//-----------------------------------------------------------------------------
/// Shows or hides the cards of the computer player
/// \param open Flag if cards should be shown or hidden
//-----------------------------------------------------------------------------
void Hearts::playOpen(bool open) {
    for (unsigned int i(1); i < NUM_PLAYERS; ++i) {
        players[i].hand->setShowOption(open ? Card::IPile::SHOWFACE : Card::IPile::SHOWBACK);
        players[i].hand->setStyle(open ? Card::IPile::COMPRESSED : Card::IPile::QUITE_COMPRESSED);
        players[i].won->setShowOption(open ? Card::IPile::SHOWFACE : Card::IPile::SHOWBACK);
        players[i].won->setStyle(open ? Card::IPile::COMPRESSED : Card::IPile::VERY_COMPRESSED);
    }
    players[0].won->setShowOption(open ? Card::IPile::SHOWFACE : Card::IPile::SHOWBACK);
    players[0].won->setStyle(open ? Card::IPile::COMPRESSED : Card::IPile::VERY_COMPRESSED);
}

//-----------------------------------------------------------------------------
/// Enables the cards of the human player
/// \returns Flag, if time should be continued
/// \remarks Depending of the status of the game (PLAYING2) also the top card
///     of the played pile is enabled
//-----------------------------------------------------------------------------
bool Hearts::enableHuman() {
    Check3(activeCards.empty());
    Check3((gameStatus() == PLAYING) || (gameStatus() == EXCHANGE));
    TRACE2("Hearts::enableHuman() - Human has " << players[0].hand->size() << " cards");

    for (int i(players[0].hand->size() - 1); i >= 0; --i)
        activeCards.push_back((*players[0].hand)[i]->signal_clicked().connect(bind(mem_fun(*this, &Hearts::cardSelected), i)));

    if (gameStatus() == EXCHANGE)
        for (int i(played.size() - 1); i >= 0; --i)
            activeCards.push_back(played[i]->signal_clicked().connect(bind(mem_fun(*this, &Hearts::takeCard), i)));

    return Game::enableHuman();
}

//-----------------------------------------------------------------------------
/// Callback after clicking on a card in the played field
/// \param iCard Offset of card in hand
//-----------------------------------------------------------------------------
void Hearts::takeCard(unsigned int iCard) {
    TRACE9("Hearts::takeCard(unsigned int) - Picking up card " << iCard);
    Check1(iCard < played.size());
    Check1(gameStatus() == EXCHANGE);

    disableHuman();
    animateCard(*players[0].hand, played, iCard).sigAnimation.connect(mem_fun(*this, &Hearts::cardTaken));
}

//-----------------------------------------------------------------------------
/// Action after animation of taken card is finished
//-----------------------------------------------------------------------------
void Hearts::cardTaken() {
    TRACE9("Hearts::cardTaken()");
    players[0].hand->sortByColour();
    makeNextMoves();
}

//-----------------------------------------------------------------------------
/// Callback after clicking on a card in hand
/// \param iCard Offset of card in hand
//-----------------------------------------------------------------------------
void Hearts::cardSelected(unsigned int iCard) {
    TRACE5("Hearts::cardSelected(unsigned int) - Position " << iCard);
    Check1(iCard < players[0].hand->size());
    Check3((gameStatus() == PLAYING) || (gameStatus() == EXCHANGE));

    // Hide won pile again (if not in debug-mode)
#if TRACELEVEL > 0
    if (players[1].won->getShowOption() == Card::IPile::SHOWBACK)
#endif
        showWonCards(false);

    if (moveSelectedCardToPlayed(0, iCard)) {
        if (gameStatus() == PLAYING) {
            if (getConnectionMgr().getMode() != YGP::ConnectionMgr::NONE) {
                // Send played card to all clients (if any)
                std::ostringstream msg;
                msg << "Play=" << played[played.size() - 1]->id() << ";Target=0";
                sendMove(msg.str());
            }
        }
        else {
            Check3(gameStatus() == EXCHANGE);
            if (played.size() == 3) {
                // Exchange the cards in pre-play
                TRACE7("Hearts::cardSelected(unsigned int) - Finished exchange");

                if (getConnectionMgr().getMode() == YGP::ConnectionMgr::NONE) {
                    exchangeCards();
                    return;
                }
                else {
                    std::ostringstream msg;
                    msg << "Exchange=" << played[0]->id() << ' ' << played[1]->id() << ' ' << played[2]->id()
                        << ";Player=" << posServer;
                    broadcastMessage(msg.str());

                    if (allCardsExchanged()) {
                        exchangeCards();
                        return;
                    }
                    else {
                        status.pop();
                        status.push(_("Waiting for other player to exchange their cards ..."));
                    }
                }
                disableHuman();
                return;
            }
        }
    }
}

//-----------------------------------------------------------------------------
/// Starts the playing phase of the game
//-----------------------------------------------------------------------------
void Hearts::startPlaying() {
    TRACE7("Hearts::startPlaying()");

    // Clear variables for a new game
    aPlayed.fill(0);
    playedSQ = false;
    setGameStatus(PLAYING);

    // Search for startplayer
    std::array<Card::Cards, NUM_PLAYERS> hands;
    for (unsigned int i(0); i < NUM_PLAYERS; ++i)
        hands[i] = players[i].hand->values();
    unsigned int nextPlayer(HeartsRules::startPlayer(hands));
    TRACE7("Hearts::startPlaying() - Start with player " << nextPlayer);
    Check3(nextPlayer < NUM_PLAYERS);
    player2Exchange = (player2Exchange - 1) & 0x3;

    setNextPlayer(nextPlayer);
    displayTurn(nextPlayer);

    YGP::ConnectionMgr& cmgr(getConnectionMgr());
    if (((cmgr.getMode() == YGP::ConnectionMgr::NONE) && nextPlayer) ||
        ((cmgr.getMode() == YGP::ConnectionMgr::SERVER) && (nextPlayer > getConnectionMgr().getClients().size()))) {
        unsigned int pos2Play(0);
        flipCards2Play(*players[nextPlayer].hand, pos2Play, pos2Play);
        aPlayed[(*players[nextPlayer].hand)[pos2Play]->colour()]++;
        animateCard(played, *players[nextPlayer].hand, pos2Play).sigAnimation.connect(mem_fun(*this, &Hearts::finishMove));
    }
    else
        makeNextMoves();
}

//-----------------------------------------------------------------------------
/// Checks if the round is at end and gives the cards to winner if so
/// \param player ID of player who did the last turn
/// \returns unsigned int Next player
//-----------------------------------------------------------------------------
unsigned int Hearts::calcNextPlayer(unsigned int player) {
    TRACE9("Hearts::calcNextPlayer(unsigned int) - " << player);
    Check1(player < NUM_PLAYERS);
    Check2(played.size() <= NUM_PLAYERS);

    if (played.size() == NUM_PLAYERS)
        player = (player + HeartsRules::trickWinner(played.values()) - NUM_PLAYERS + 1) & 0x3;
    else
        player = ((player + 1) & 0x3);

    if (!players[player].hand->size()) {
        setGameStatus(STOPPED);
        if (!pScoreDlg) {
            pScoreDlg.reset(Card::ScoreDlg::create(actPlayers));
            Gtk::Window* win(dynamic_cast<Gtk::Window*>(get_root()));
            if (win)
                pScoreDlg->set_transient_for(*win);
            menuShowScoreDlg->set_enabled();
        }

        std::array<unsigned int, NUM_PLAYERS> aPoints{};
        aPoints[player] = HeartsRules::pointsOf(played.values()); // Adds points still on table
        for (unsigned int i(0); i < NUM_PLAYERS; ++i)
            aPoints[i] += HeartsRules::pointsOf(players[i].won->values());
        std::array<int, NUM_PLAYERS> aScore(HeartsRules::roundScore(aPoints));

        pScoreDlg->addPoints(aScore.data());
        pScoreDlg->display();

        Glib::ustring stat(_("Round ended"));
        unsigned int player;
        int points;
        pScoreDlg->getMaxPoints(points, player);
        if (static_cast<unsigned int>(points) >= ENDPOINTS) {
            stat = _("Game ended; %1 won");
            pScoreDlg->getMinPoints(points, player);

            Check3(actPlayers.size() > player);
            Check3(actPlayers[player]);
            stat.replace(stat.find("%1"), 2, actPlayers[player]->getName());
        }

        status.pop();
        status.push(stat);
    }
    else
        displayTurn(player);

    TRACE4("Hearts::calcNextPlayer(unsinged int) - Continuing with player " << player);
    return player;
}

//-----------------------------------------------------------------------------
/// Moves the selected card to the played pile
/// \param player ID of player
/// \param card Offset of card to play
/// \returns bool Status of move; true: Card could be moved; false else
//-----------------------------------------------------------------------------
bool Hearts::moveSelectedCardToPlayed(unsigned int player, unsigned int card) {
    TRACE5("Hearts::moveSelectedCardToPlayed(unsigned int, unsigned int) - Player " << player << "; Pos.  " << card);
    Check1(player < NUM_PLAYERS);
    Check1(card < players[player].hand->size());
    Check1((gameStatus() == PLAYING) || (gameStatus() == EXCHANGE));

    if (gameStatus() == PLAYING) {
        const HeartsRules::PlayError error(HeartsRules::checkPlay(players[player].hand->values(), card, currentTable()));
        if (error != HeartsRules::PlayError::NONE) {
            Gtk::MessageDialog dlg(_(HeartsRules::describe(error)), false, Gtk::MessageType::ERROR);
            dlg.set_title(_("Hearts"));
            XGP::runModal(dlg);
            return false;
        }

        Card::Widget& actCard(*(*players[player].hand)[card]);
        Card::Widget::COLOURS playColour(actCard.colour());
        Check3(static_cast<unsigned int>(playColour) < aPlayed.size());
        aPlayed[playColour]++;
        if (actCard.is(Card::Widget::SPADES, Card::Widget::QUEEN))
            playedSQ = true;

        disableHuman();
        Card::Window& win(animateCard(played, *players[player].hand, card));
        win.sigAnimation.connect(mem_fun(*this, &Hearts::finishMove));
    }
    else {
        // If there are already two cards exchanged (and thus the 3rd is going
        // to be exchanged) start exchanging of cards for the computer players
        disableHuman();
        Card::Window& win(animateCard(played, *players[player].hand, card));
        win.sigAnimation.connect((played.size() != 2) ? mem_fun(*this, &Hearts::makeNextMoves)
                                                      : mem_fun(*this, &Hearts::exchangeCards));
    }
    return true;
}

//-----------------------------------------------------------------------------
/// Exchanges the cards of the computer players. Get rid of cards according the
/// following algorithm:
///    - If nr. of clubs or diamonds are < 3 -> Use them
///    - If nr. of spades < 5 get rid of high spades (especially the queen)
///    - Get rid of high hearts
///    - Get rid of high cards
//-----------------------------------------------------------------------------
void Hearts::exchangeCards() {
    TRACE8("Hearts::exchangeCards() - with " << player2Exchange);
    Check1(player2Exchange);
    Check1(player2Exchange < NUM_PLAYERS);
    Check3(played.size() == 3);

    disableHuman();
    if (getConnectionMgr().getMode() != YGP::ConnectionMgr::CLIENT) {
        for (unsigned int i(getConnectionMgr().getClients().size() + 1); i < NUM_PLAYERS; ++i) {
            TRACE8("Hearts::exchangeCards() - Player " << i);

            aExchange[i].clear();
            for (const auto& card : HeartsRules::selectCardsToExchange(players[i].hand->values()))
                aExchange[i].push_back(card.id());
            Check3(aExchange[i].size() == 3);

            if (getConnectionMgr().getMode() == YGP::ConnectionMgr::SERVER) {
                std::ostringstream msg;
                msg << "Exchange=" << aExchange[i][0] << ' ' << aExchange[i][1] << ' ' << aExchange[i][2] << ";Player=" << i
                    << ';';
                broadcastMessage(msg.str());
            }
        }
    }
    exchangeStep(0);
}

//-----------------------------------------------------------------------------
/// Animates the cards the passed player gives away to the receiving player
/// (the human receives them onto the played pile, to see them); afterwards
/// the next player gives his cards
/// \param giver Player giving away his cards; the human gives the cards on
///     the played pile
//-----------------------------------------------------------------------------
void Hearts::exchangeStep(unsigned int giver) {
    TRACE8("Hearts::exchangeStep(unsigned int) - " << giver);
    if (gameStatus() != EXCHANGE) // Game ended meanwhile
        return;

    if (giver == NUM_PLAYERS) {
        Glib::signal_timeout().connect(bind_return(mem_fun(*this, &Hearts::takeExchangedCards), false),
                                       Card::ComputerPlayer::TIMEOUT);
        return;
    }

    // Move the cards to give away to the end of the hand (to animate them together)
    Card::IPile* src(&played);
    if (giver) {
        src = players[giver].hand.get();
        Check3(aExchange[giver].size() == 3);
        for (auto id : aExchange[giver]) {
            const int pos(src->find(id));
            Check3(pos != -1);
            src->move(src->size() - 1, pos);
        }
    }
    Check3(src->size() >= 3);

    const unsigned int receiver((giver + player2Exchange) & 0x3);
    animateCards(receiver ? *players[receiver].hand : played, *src, src->size() - 3, src->size() - 1, LAYOUT_DELAY)
        .sigAnimation.connect(bind(mem_fun(*this, &Hearts::exchangeStep), giver + 1));
}

//-----------------------------------------------------------------------------
/// Moves the cards the human received into his hand
//-----------------------------------------------------------------------------
void Hearts::takeExchangedCards() {
    TRACE9("Hearts::takeExchangedCards()");
    if (gameStatus() != EXCHANGE) // Game ended meanwhile
        return;

    Check3(played.size() == 3);
    animateCards(*players[0].hand, played, 0, played.size() - 1, 0)
        .sigAnimation.connect(mem_fun(*this, &Hearts::exchangeFinished));
}

//-----------------------------------------------------------------------------
/// Finishes exchanging the cards and starts the game
//-----------------------------------------------------------------------------
void Hearts::exchangeFinished() {
    TRACE9("Hearts::exchangeFinished()");
    for (unsigned int i(0); i < NUM_PLAYERS; ++i) {
        aExchange[i].clear();
        players[i].hand->sortByColour();
        Check3(players[i].hand->size() == (cards.size() / NUM_PLAYERS));
    }

    startPlaying();
}

//-----------------------------------------------------------------------------
/// Returns the state of the actual round, as needed by the rules
/// \returns HeartsRules::Table State of the round
//-----------------------------------------------------------------------------
HeartsRules::Table Hearts::currentTable() const {
    HeartsRules::Table table;
    table.trick = played.values();
    table.played = aPlayed;
    table.playedSQ = playedSQ;
    for (const auto& player : players)
        table.cardsWon += player.won->size();
    table.cardsInGame = cards.size();
    return table;
}

//-----------------------------------------------------------------------------
/// Changes the names of the playing people
/// \param newPlayer Array holding the new player
//-----------------------------------------------------------------------------
void Hearts::changeNames(const std::vector<Card::Player*>& newPlayer) {
    Game::changeNames(newPlayer);

    std::vector<Card::Player*> player;
    for (unsigned int i(0); i < NUM_PLAYERS; ++i) {
        player.push_back(actPlayers[(i + posServer) & 0x3]);
        players[i].name.set_text(actPlayers[i]->getName());
    }

    if (pScoreDlg)
        pScoreDlg->update(player);
}

//----------------------------------------------------------------------------
/// Converts a pile-number to the actual pile
/// \param newPlayer Array holding the new player
/// \param pile ID of the pile to return
/// \returns Card::IPile* Pointer to pile to use or NULL
//----------------------------------------------------------------------------
Card::IPile* Hearts::getPileOfPlayer(unsigned int player, unsigned int pile) {
    return ((player >= NUM_PLAYERS) || pile) ? nullptr : players[player].hand.get();
}

//----------------------------------------------------------------------------
/// Handles the messages the server might send for the hearts cardgame
/// \param player ID of player sending the message
/// \param message Message received from the server
/// \returns bool True, if message has been completey processed
//----------------------------------------------------------------------------
bool Hearts::handleMessage(unsigned int player, const std::string& message) {
#ifdef WITH_NETWORK
    // Exchange=<IDs of cards>;Player=<position of player (as seen from the server)>
    if ((gameStatus() == EXCHANGE) && (Card::commandOf(message) == "Exchange")) {
        TRACE1("Hearts::handleMessage(unsigned int player, const std::string&) - " << message << " (" << player << ')');

        const auto fields(Card::splitMessage(message));
        unsigned long lPlayer(player);
        if ((fields.size() >= 2) && (fields[1].key == "Player") && !stringToNumber(lPlayer, fields[1].value.c_str()) &&
            (lPlayer < NUM_PLAYERS)) {
            // The server receives the exchanges of the clients directly; a client all over the server
            Check3((getConnectionMgr().getMode() != YGP::ConnectionMgr::SERVER) || (lPlayer == player));

            const unsigned int sender(lPlayer);
            lPlayer = (lPlayer - posServer) & 0x3;

            // Don't exchange already exchanged cards (the own ones, echoed by the server)
            if (sender != posServer) {
                for (const auto& id : Card::words(fields[0].value)) {
                    unsigned long card(0);
                    if (stringToNumber(card, id.c_str()))
                        throw YGP::ParseError(N_("Invalid card specification!"));

                    TRACE9("Hearts::handleMessage(unsigned int, const std::string&) - " << lPlayer << ": " << card);
                    if (players[lPlayer].hand->find(static_cast<unsigned int>(card)) == -1)
                        throw YGP::ParseError(N_("Card not found!"));
                    aExchange[lPlayer].push_back(static_cast<unsigned int>(card));
                }
            }

            TRACE2("Hearts::handleMessage(unsigned int player, const std::string&) - Exchanged: " << aExchange[lPlayer].size()
                                                                                                  << " cards");
            if (aExchange[lPlayer].size() == 3) {
                // Inform other clients
                if (getConnectionMgr().getMode() == YGP::ConnectionMgr::SERVER)
                    broadcastMessage(message);
                if (allCardsExchanged())
                    exchangeCards();
            }
            return true;
        }
    }
#endif
    return Game::handleMessage(player, message);
}

//----------------------------------------------------------------------------
/// Checks if the number of exchanged cards is equal to the passed value.
/// \param cards Number of cards to exchange
/// \returns bool True, if all cards have been exchanged
/// \pre Game must be in EXCHANGE state
//----------------------------------------------------------------------------
bool Hearts::cardsExchanged(unsigned int cards) {
    Check1(gameStatus() == EXCHANGE);

    for (const auto& i : aExchange)
        cards -= i.size();

    TRACE9("Hearts::cardsExchanged(unsigned int) - Remaining: " << cards);
    return !(cards - played.size());
}

//----------------------------------------------------------------------------
/// Checks if all players, which exchange their cards interactively, did so.
/// The server waits for its own and the remote players (its computer players
/// exchange their cards afterwards), while a client waits for all players.
/// \returns bool True, if all cards have been exchanged
/// \pre Game must be in EXCHANGE state
//----------------------------------------------------------------------------
bool Hearts::allCardsExchanged() {
    const YGP::ConnectionMgr& cmgr(getConnectionMgr());
    return cardsExchanged(((cmgr.getMode() == YGP::ConnectionMgr::SERVER) ? (cmgr.getClients().size() + 1) : NUM_PLAYERS) * 3);
}

//-----------------------------------------------------------------------------
/// Adds game-specific menus
/// \param menu Menu to add game-specific entries to
/// \param actions Action group to add game-specific actions to
//-----------------------------------------------------------------------------
void Hearts::addMenus(const Glib::RefPtr<Gio::Menu>& menu, const Glib::RefPtr<Gio::SimpleActionGroup>& actions) {
    Check1(menu);
    Check1(actions);

    Glib::RefPtr<Gio::Menu> sub(Gio::Menu::create());

    menuSort = actions->add_action("HeartSort", mem_fun(*this, &Hearts::sortWonByNumber));
    sub->append(_("_Sort won cards (by number)"), "game.HeartSort");

    menuSort2 = actions->add_action("HeartSortCol", mem_fun(*this, &Hearts::sortWonByColour));
    sub->append(_("Sort won cards (by _colour)"), "game.HeartSortCol");

    Glib::RefPtr<Gio::Menu> sec(Gio::Menu::create());
    menuShowScoreDlg = actions->add_action("showScoreDlg", [this]() {
        if (pScoreDlg)
            pScoreDlg->display();
    });
    sec->append(_("Show score dialog"), "game.showScoreDlg");
    sub->append_section(sec);

    menu->append_submenu(_("H_earts"), sub);

    menuShowScoreDlg->set_enabled(false);
}

//-----------------------------------------------------------------------------
/// Removes the game-specific menus
/// \param menu Menu to remove game-specific entries from
/// \param actions Action group to remove game-specific actions from
//-----------------------------------------------------------------------------
void Hearts::removeMenus(const Glib::RefPtr<Gio::Menu>& menu, const Glib::RefPtr<Gio::SimpleActionGroup>& actions) {
    Check1(menu);
    Check1(actions);

    menu->remove(menu->get_n_items() - 1);
    actions->remove_action("HeartSort");
    actions->remove_action("HeartSortCol");
    actions->remove_action("showScoreDlg");
}

//-----------------------------------------------------------------------------
/// Actions to take when the cards are resized
/// \pre The cardsize must be set in Card::Images::WIDTH/HEIGHT
//-----------------------------------------------------------------------------
void Hearts::resizeCards() {
    for (unsigned int i(0); i < NUM_PLAYERS; ++i) {
        Gtk::Box* hand(dynamic_cast<Gtk::Box*>(players[i].hand.get()));
        Check3(hand);
        Gtk::Box* won(dynamic_cast<Gtk::Box*>(players[i].won.get()));
        Check3(won);

        if (i & 1) {
            hand->set_size_request(Card::Images::WIDTH + 5, Card::Images::HEIGHT + 12 * 7);
            won->set_size_request(Card::Images::WIDTH + 5, Card::Images::HEIGHT + 12 * 7);
        }
        else {
            hand->set_size_request(Card::Images::WIDTH + 12 * 18, Card::Images::HEIGHT + 5);
            won->set_size_request(Card::Images::WIDTH + 12 * 7, Card::Images::HEIGHT + 5);
        }
    }
    // played holds at most NUM_PLAYERS cards at COMPRESSED pitch (18px)
    played.set_size_request(Card::Images::WIDTH + (NUM_PLAYERS - 1) * 18, Card::Images::HEIGHT);
}
