// PROJECT     : Cardgames
// SUBSYSTEM   : Sgt. Mayor
// REFERENCES  :
// TODO        :
// BUGS        :
// AUTHOR      : Markus Schwab
// CREATED     : 11.4.2004
// COPYRIGHT   : Copyright (C) 2004 - 2009, 2026

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

#include <array>
#include <exception>
#include <iomanip>
#include <sstream>
#include <string>
#include <string_view>

#include <cardgames-cfg.h>

#include <gtkmm/messagedialog.h>
#include <gtkmm/statusbar.h>

#include <giomm/menu.h>
#include <giomm/simpleactiongroup.h>

#include <XGP/XDialog.h>

#include <YGP/ANumeric.h>
#include <YGP/Check.h>
#include <YGP/ConnMgr.h>
#include <YGP/Trace.h>

#include <card/ComputerPlayer.h>
#include <card/Images.h>
#include <card/Message.h>
#include <card/Random.h>
#include <card/RemotePlayer.h>
#include <card/ScoreDlg.h>
#include <card/Window.h>

#include "SgtMayor.h"

unsigned int SgtMayor::ENDTRICKS(10);

//-----------------------------------------------------------------------------
/// Constructor
/// \param parent Parent widget to display the game in
/// \param statusbar Status bar widget to display information about the game
/// \param cardset Cardset to use
/// \param player Vector of player
/// \param posPlayer Position of player for the server
/// \param mxSerialize Mutex to serialize messages from the server
//-----------------------------------------------------------------------------
SgtMayor::SgtMayor(Gtk::Box& parent, Gtk::Statusbar& statusbar, Card::Set& cardset, const std::vector<Card::Player*>& player,
                   unsigned int posPlayer, Card::MessageLock& mxSerialize)
    : Game(parent, statusbar, cardset, player, posPlayer, mxSerialize, 10, 8),
      played(Card::IPile::COMPRESSED, Card::IPile::SHOWFACE), pTrump(nullptr), startPlayer(Card::randomNumber(NUM_PLAYERS)),
      table(), menuSort(), menuSort2(), menuShowScoreDlg(), pScoreDlg(nullptr) {
    TRACE9("SgtMayor::SgtMayor(Box&, Statusbar&, Card::Set&, ...)");

    // Show and attach card-piles
    changeNames(player);
    for (unsigned int i(0); i < NUM_PLAYERS; ++i) {
        players[i].name.show();
        players[i].name.set_margin_start(1);
        players[i].name.set_margin_end(1);
        players[i].name.set_margin_top(2);
        players[i].name.set_margin_bottom(2);
        attach(players[i].name, COLS_PLAYER[i], ROWS_PLAYER[i] + (i ? 1 : 2), (i ? 3 : 5), 1);

        players[i].neededTricks.set_margin_top(5);
        players[i].neededTricks.set_margin_bottom(5);
        attach(players[i].neededTricks, COLS_PLAYER[i], ROWS_PLAYER[i] + (i ? 2 : 3), (i ? 3 : 5), 1);

        players[i].won.set_margin_start(1);
        players[i].won.set_margin_end(1);
        players[i].won.set_margin_top(5);
        players[i].won.set_margin_bottom(5);
        attach(players[i].won, COLS_PLAYER[i], ROWS_PLAYER[i] + (i ? -1 : +1), (i ? 3 : 5), 1);

        players[i].hand.set_margin_start(5);
        players[i].hand.set_margin_end(5);
        attach(players[i].hand, COLS_PLAYER[i], ROWS_PLAYER[i], (i ? 3 : 5), 1);
        TRACE9("SgtMayor::SgtMayor() - Attach at: " << COLS_PLAYER[i] << '/' << COLS_PLAYER[i] + (i ? 3 : 5) << " - "
                                                    << ROWS_PLAYER[i] << '/' << ROWS_PLAYER[i] + 1);

        players[i].hand.setStyle(i ? Card::IPile::QUITE_COMPRESSED : Card::IPile::COMPRESSED);
        players[i].hand.setShowOption(i ? Card::IPile::SHOWBACK : Card::IPile::SHOWFACE);
        players[i].won.setStyle(Card::IPile::QUITE_COMPRESSED);
        players[i].won.setShowOption(Card::IPile::SHOWBACK);
    }

    // Show played area
    played.setStyle(Card::IPile::COMPRESSED);
    played.set_margin_start(5);
    played.set_margin_end(5);
    attach(played, 2, 4, 3, 1);

    resizeCards();

    diffTricks.fill(0);
    points.fill(0);
}

//-----------------------------------------------------------------------------
/// Destructor
//-----------------------------------------------------------------------------
SgtMayor::~SgtMayor() {
    TRACE9("SgtMayor::~SgtMayor()");
    pScoreDlg.reset();
    clean();
}

//-----------------------------------------------------------------------------
/// Makes the move for the next player.
/// \param player Actual player
//-----------------------------------------------------------------------------
void SgtMayor::makeMove(unsigned int player) {
    player = localPlayer(player);
    TRACE5("SgtMayor::makeMove() - Turn of player - " << player);
    Check1(player < NUM_PLAYERS);
    Check1(gameStatus() == PLAYING);
    Check2(pTrump);

    Check3(table.trick.size() == played.size());

    Card::HPile& pile(players[player].hand);
    unsigned int pos(-1U);
    if (isShowingCardsToPlay()) { // Card of a remote player; already flipped
        pos = pos2Play;
        pos1Play = pos2Play = -1U;
        Check3(pos < pile.size());

        // Remember, if the player can't follow the colour (and has no trumps)
        table.noteNotFollowing(player, *pile[pos]);
    }
    else {
        pos = SgtMayorRules::selectCardToPlay(pile.values(), player, startPlayer, table);
        flipCards2Play(pile, pos, pos);
    }

    // Remember card as being played
    table.play(*pile[pos]);

    TRACE8("SgtMayor::makeMove(unsigned int) - Going to play card at pos " << pos);
    animateCard(played, pile, pos).sigAnimation.connect(bind(mem_fun(*this, &SgtMayor::playCardDelayed), player));
}

//-----------------------------------------------------------------------------
/// Delays playing the next card a while
//-----------------------------------------------------------------------------
void SgtMayor::playCardDelayed(unsigned int player) {
    player = playCard(player);
    if (player != -1U) {
        setNextPlayer(convertPlayer(player));
        makeNextMoves();
    }
}

//-----------------------------------------------------------------------------
/// Starts the game by dealing the cards
//-----------------------------------------------------------------------------
void SgtMayor::start() {
    TRACE9("SgtMayor::start()");
    Game::start();

    Check2(!played.size());
    Card::IPile pile;
    if (randomiseCardsToPile(pile)) {
        // Delete the score-dialog if the game has ended
        if (pScoreDlg) {
            if (SgtMayorRules::isGameOver(points, ENDTRICKS)) {
                menuShowScoreDlg->set_enabled(false);
                pScoreDlg.reset();

                diffTricks.fill(0);
            }
        }

        if (getConnectionMgr().getMode() != YGP::ConnectionMgr::CLIENT) {
            setNextPlayer(startPlayer = calcNextPlayer(startPlayer));
            broadcastStartPlayer(startPlayer);
        }

        Check3(cards.size() == Card::Value::CARDS_PER_DECK);
        for (unsigned int i(0); i < NUM_PLAYERS; ++i)
            for (unsigned int j(0); j < SgtMayorRules::CARDS_PER_PLAYER;)
                if (SgtMayorRules::isUsed(pile.getTopCard())) {
                    players[(NUM_PLAYERS + i - posServer) % NUM_PLAYERS].hand.insertColourSorted(pile.removeTopCard());
                    ++j;
                }
                else
                    pile.removeTopCard();
        // The unused card might be the last one
        if (pile.size()) {
            Check3(!SgtMayorRules::isUsed(pile.getTopCard()));
            pile.removeTopCard();
        }
        Check3(pile.empty());

        table.newRound();

        if (getConnectionMgr().getMode() != YGP::ConnectionMgr::CLIENT) {
            showNeededTricks();
            makeExchange();
        }
    }
}

//-----------------------------------------------------------------------------
/// Shows the tricks each player needs
//-----------------------------------------------------------------------------
void SgtMayor::showNeededTricks() {
    TRACE9("SgtMayor::showNeededTricks() - Startplayer " << startPlayer);
    // Separate this from dealing the cards, to give the client a chance to
    // receive and perform the ActPlayer-message (to set the start-player)
    for (unsigned int i(0); i < NUM_PLAYERS; ++i) {
        Glib::ustring needed(_("(needs %1 tricks)"));
        needed.replace(needed.find("%1"), 2, 1, static_cast<char>('0' + SgtMayorRules::neededTricks(i, startPlayer)));
        players[i].neededTricks.set_text(needed);
    }
}

//-----------------------------------------------------------------------------
/// Remove cards from everything which can hold them
//-----------------------------------------------------------------------------
void SgtMayor::clean() {
    TRACE9("SgtMayor::clean()");
    for (auto& player : players) {
        player.hand.clear();

        for (auto* card : player.won)
            card->show();
        player.won.clear();
    }

    if (pTrump) {
        remove(*pTrump);
        pTrump.reset();
    }

    menuSort->set_enabled(false);
    menuSort2->set_enabled(false);

#ifdef WITH_NETWORK
    // Drop a message deferred while exchanging; its (kept) token is released with the next status change
    deferredMsg.clear();
    exchanging = remoteExchange = false;
#endif

    played.clear();
    Game::clean();
}

//-----------------------------------------------------------------------------
/// Shows or hides the cards of the computer player
/// \param open Flag if cards should be shown or hidden
//-----------------------------------------------------------------------------
void SgtMayor::playOpen(bool open) {
    for (unsigned int i(1); i < NUM_PLAYERS; ++i) {
        players[i].hand.setShowOption(open ? Card::IPile::SHOWFACE : Card::IPile::SHOWBACK);
        players[i].hand.setStyle(open ? Card::IPile::COMPRESSED : Card::IPile::QUITE_COMPRESSED);
    }

    for (auto& player : players) {
        player.won.setShowOption(open ? Card::IPile::SHOWFACE : Card::IPile::SHOWBACK);
        player.won.setStyle(open ? Card::IPile::COMPRESSED : Card::IPile::QUITE_COMPRESSED);

        for (auto c(player.won.begin()); c != player.won.end(); ++c)
            if (((c - player.won.begin()) % 3) != 2)
                open ? (*c)->show() : (*c)->hide();
    }
}

//-----------------------------------------------------------------------------
/// Enables the cards of the human player
/// \returns \c Flag, if time should be continued
/// \remarks Depending of the status of the game (PLAYING2) also the top card
///     of the played pile is enabled
//-----------------------------------------------------------------------------
bool SgtMayor::enableHuman() {
    Check3(activeCards.empty());
    Check3(!currentPlayer());
    TRACE2("SgtMayor::enableHuman() - Human has " << players[0].hand.size() << " cards");

    for (int i(players[0].hand.size() - 1); i >= 0; --i)
        activeCards.push_back(players[0].hand[i]->signal_clicked().connect(bind(mem_fun(*this, (&SgtMayor::cardSelected)), i)));

    return Game::enableHuman();
}

//-----------------------------------------------------------------------------
/// Callback after clicking on a card in hand
/// \param iCard Offset of card in hand
//-----------------------------------------------------------------------------
void SgtMayor::cardSelected(unsigned int iCard) {
    TRACE5("SgtMayor::cardSelected(unsigned int) - Position " << iCard);
    Check1(iCard < players[0].hand.size());
    Check2(gameStatus() == PLAYING);

    Check3(table.trick.size() == played.size());

    Card::HPile& pile(players[0].hand);
    Card::Widget& selCard(*pile[iCard]);

    const SgtMayorRules::PlayError error(SgtMayorRules::checkPlay(pile.values(), iCard, table));
    if (error != SgtMayorRules::PlayError::NONE) {
        Gtk::MessageDialog dlg(_(SgtMayorRules::describe(error)), false, Gtk::MessageType::ERROR);
        dlg.set_title(PACKAGE " - SgtMayor");
        XGP::runModal(dlg);
        return;
    }

    Check2(pTrump);
    table.noteNotFollowing(0, selCard);
    table.play(selCard);

    if (getConnectionMgr().getMode() != YGP::ConnectionMgr::NONE) {
        // Send played card to all clients (if any)
        std::ostringstream msg;
        msg << "Play=" << selCard.id() << ";Target=0";
        sendMove(msg.str());
    }

    animateCard(played, pile, iCard).sigAnimation.connect(bind(mem_fun(*this, &SgtMayor::playCardDelayed), 0));
    disableHuman();
}

//-----------------------------------------------------------------------------
/// Callback after clicking on a card to select the special colour
/// \param iCard Offset of card in hand
//-----------------------------------------------------------------------------
void SgtMayor::cardColourSelect(unsigned int iCard) {
    TRACE5("SgtMayor::cardColourSelect(unsigned int) - Position " << iCard);
    Check1(iCard < players[0].hand.size());
    Check2(gameStatus() == PLAYING);

    showTrump(players[0].hand[iCard]->colour());
    disableHuman();
    makeNextMoves();
}

//-----------------------------------------------------------------------------
/// Callback after clicking on a card to exchange bad cards with good ones
/// \param iCard Offset of card in hand
//-----------------------------------------------------------------------------
void SgtMayor::cardExchange(unsigned int iCard) {
    TRACE5("SgtMayor::cardExchange(unsigned int) - Position " << iCard);
    Check3(diffTricks[0]);

    disableHuman();
    const auto exchange(SgtMayorRules::nextExchange(diffTricks, startPlayer));
    Check3(exchange && !exchange->playerBad);
    exchangeCards(0, iCard, exchange->playerGood);
}

//-----------------------------------------------------------------------------
/// Selects the trump
/// \returns bool False, if a human must select the trump colour
//-----------------------------------------------------------------------------
bool SgtMayor::selectTrump() {
    TRACE9("SgtMayor::selectTrump()");

    Glib::ustring stat;
    unsigned int trumpPlayer(SgtMayorRules::trumpPlayer(startPlayer));
    if (trumpPlayer) {
        unsigned int displayPlayer(convertPlayer(startPlayer));
        const Card::Player& player(*actPlayers[convertPlayer(trumpPlayer)]);
        TRACE9("SgtMayor::selectTrump() - Trumpplayer: " << trumpPlayer);
        if (typeid(player) == typeid(Card::RemotePlayer)) {
            // Wait for the Trump-message (see handleMessage)
            status.pop();
            stat = _("Waiting for %1 to select the special colour ...");
            stat.replace(stat.find("%1"), 2, player.getName());
            status.push(stat);
            return false;
        }
        else {
            Check3(typeid(player) == typeid(Card::ComputerPlayer));
            showTrump(SgtMayorRules::selectTrump(players[trumpPlayer].hand.values()));
            displayTurn(displayPlayer, stat);
        }
    }
    else {
        status.pop();
        stat = _("Select the special colour");
        status.push(stat);

        for (int i(players[0].hand.size() - 1); i >= 0; --i)
            activeCards.push_back(
                players[0].hand[i]->signal_clicked().connect(bind(mem_fun(*this, (&SgtMayor::cardColourSelect)), i)));
        return false;
    }
    return true;
}

//-----------------------------------------------------------------------------
/// Starts the playing phase of the game
//-----------------------------------------------------------------------------
void SgtMayor::startPlaying() {
    TRACE7("SgtMayor::startPlaying()");
    Check3(!diffTricks[0]);
    Check3(!diffTricks[1]);
    Check3(!diffTricks[2]);

    setNextPlayer(convertPlayer(startPlayer));
    if (selectTrump())
        makeNextMoves();
}

//-----------------------------------------------------------------------------
/// Changes the names of the playing people
/// \param newPlayer Array holding the new player
//-----------------------------------------------------------------------------
void SgtMayor::changeNames(const std::vector<Card::Player*>& newPlayer) {
    Game::changeNames(newPlayer);

    std::vector<Card::Player*> player;
    for (unsigned int i(0); i < NUM_PLAYERS; ++i) {
        player.push_back(actPlayers[convertPlayer(i)]);
        players[i].name.set_text(player.back()->getName());
    }

    if (pScoreDlg)
        pScoreDlg->update(player);
}

//----------------------------------------------------------------------------
/// Converts a pile-number to the actual pile
/// \param player Actual player
/// \param pile ID of the pile to return
/// \returns Card::IPile* Pointer to pile to use or NULL
//----------------------------------------------------------------------------
Card::IPile* SgtMayor::getPileOfPlayer(unsigned int player, unsigned int pile) {
    TRACE9("SgtMayor::getPileOfPlayer(2x unsigned int) - Player " << player << "; " << pile);
    player = localPlayer(player);
    Check3(player < NUM_PLAYERS);
    Check3(!pile);
    return ((player >= NUM_PLAYERS) || pile) ? nullptr : &players[player].hand;
}

#ifdef WITH_NETWORK
//-----------------------------------------------------------------------------
/// Reads card- and playernumber from the passed fields of a message
/// \param card Field holding the ID of the card (key is ignored)
/// \param from Field holding the (absolute) position of the player owning the card
/// \param idCard Filled with ID of card
/// \param player Filled with player number (as seen from the server)
/// \returns bool True, if parsing was successfull
//-----------------------------------------------------------------------------
bool SgtMayor::readCardInfo(const Card::MessageField& card, const Card::MessageField& from, unsigned long& idCard,
                            unsigned long& player) {
    return (from.key == "From") && !stringToNumber(idCard, card.value.c_str()) && (idCard < 52) &&
           !stringToNumber(player, from.value.c_str()) && (player < NUM_PLAYERS);
}

//----------------------------------------------------------------------------
/// Handles the messages the partners might send for the Sgt.Mayor cardgame:
///   - <tt>Exchange=<i>ID of good card</i>;From=<i>player</i>;With=<i>ID of bad
///     card</i>;From=<i>player</i></tt>: A player exchanges a bad card with a
///     good card of another player (positions are as seen from the server)
///   - <tt>Trump=<i>colour</i></tt>: The special colour has been selected
///   - <tt>ActPlayer=<i>player</i></tt> (only clients): Sets the start player
///     and starts the exchange of the cards
///   - Everything else is handled by Card::Game::handleMessage
/// \param player ID of player sending the message
/// \param message Message received from the server
/// \returns bool True, if message has been completey processed
/// \throw YGP::ParseError In case of an error an describing text
//----------------------------------------------------------------------------
bool SgtMayor::handleMessage(unsigned int player, const std::string& message) {
    TRACE1("SgtMayor::handleMessage(unsigned int player, const std::string&) - " << message << " (" << player << ')');

    // While cards are exchanged (which takes a while, due to the animation) the
    // next message must wait
    if (exchanging) {
        TRACE5("SgtMayor::handleMessage(unsigned int player, const std::string&) - Deferring " << message);
        Check3(deferredMsg.empty());
        deferredMsg = message;
        deferredSender = player;
        keepMessageLock();
        return false;
    }

    const std::string_view cmd(Card::commandOf(message));
    if (cmd == "Exchange") {
        const auto fields(Card::splitMessage(message));
        unsigned long card1(0), card2(0);
        unsigned long player1(0), player2(0);
        if ((fields.size() < 4) || !readCardInfo(fields[0], fields[1], card1, player1) || (fields[2].key != "With") ||
            !readCardInfo(fields[2], fields[3], card2, player2))
            throw YGP::ParseError(N_("Invalid exchange of cards!"));

        // Convert the positions to the own view
        player1 = (NUM_PLAYERS + player1 - posServer) % NUM_PLAYERS;
        player2 = (NUM_PLAYERS + player2 - posServer) % NUM_PLAYERS;
        if (pTrump || (player1 == player2) || (diffTricks[player1] >= 0) || (diffTricks[player2] <= 0))
            throw YGP::ParseError(N_("Invalid exchange of cards!"));

        const int pos1(players[player1].hand.find(static_cast<unsigned int>(card1)));
        const int pos2(players[player2].hand.find(static_cast<unsigned int>(card2)));
        if ((pos1 == -1) || (pos2 == -1))
            throw YGP::ParseError(N_("Card not found!"));

        // The server informs the (other) clients about the exchange
        if (getConnectionMgr().getMode() == YGP::ConnectionMgr::CLIENT)
            doExchangeCards(player2, pos2, player1, pos1);
        else
            exchangeCards(player2, pos2, player1, pos1);

        // Wait with the next message til the exchange is finished (exchgNext)
        remoteExchange = true;
        keepMessageLock();
        return false;
    }
    else if (cmd == "Trump") {
        const auto fields(Card::splitMessage(message));
        unsigned long trumpColour(0);
        if (stringToNumber(trumpColour, fields.at(0).value.c_str()) || (trumpColour > Card::Widget::HEARTS) || pTrump ||
            diffTricks[0] || diffTricks[1] || diffTricks[2])
            throw YGP::ParseError(N_("Invalid special colour!"));

        // The server informs the (other) clients about the special colour
        if (getConnectionMgr().getMode() == YGP::ConnectionMgr::CLIENT)
            doShowTrump(static_cast<Card::Widget::COLOURS>(trumpColour));
        else
            showTrump(static_cast<Card::Widget::COLOURS>(trumpColour));
        makeNextMoves();
        return true;
    }

    bool rc(Game::handleMessage(player, message));

    // The client starts exchanging the cards, after receiving the startplayer
    if ((cmd == "ActPlayer") && (getConnectionMgr().getMode() == YGP::ConnectionMgr::CLIENT)) {
        startPlayer = localPlayer(currentPlayer());
        TRACE9("SgtMayor::handleMessage(unsigned int player, const std::string&) - Start with " << startPlayer);
        showNeededTricks();
        makeExchange();
    }
    return rc;
}

//----------------------------------------------------------------------------
/// Handles the message received while exchanging cards
//----------------------------------------------------------------------------
void SgtMayor::handleDeferredMessage() {
    Check3(deferredMsg.size());
    Check3(!exchanging);

    std::string msg;
    msg.swap(deferredMsg);
    TRACE5("SgtMayor::handleDeferredMessage() - " << msg);
    try {
        // The token is still kept for the message; so release it if processed
        if (handleMessage(deferredSender, msg))
            releaseMessageLock();
    }
    catch (std::exception& error) {
        releaseMessageLock();

        Glib::ustring err(_("Error processing command `%1'!\n\n%2"));
        err.replace(err.find("%1"), 2, msg);
        err.replace(err.find("%2"), 2, _(error.what()));
        Gtk::MessageDialog dlg(err, false, Gtk::MessageType::ERROR, Gtk::ButtonsType::OK);
        dlg.set_title(PACKAGE);
        XGP::runModal(dlg);
    }
}
#endif

//----------------------------------------------------------------------------
/// Shows the special colour on the board (the ace with that colour). Also
/// inform connected player about it
/// \param colour The special colour to display
//----------------------------------------------------------------------------
void SgtMayor::showTrump(Card::Widget::COLOURS colour) {
    TRACE9("SgtMayor::showTrump(Card::Widget::COLOURS) - " << colour);
    if (getConnectionMgr().getMode() != YGP::ConnectionMgr::NONE) {
        std::ostringstream msg;
        msg << "Trump=" << colour;
        sendMove(msg.str());
    }

    doShowTrump(colour);
}

//----------------------------------------------------------------------------
/// Shows the special colour on the board (the ace with that colour)
/// \param colour The special colour to display
//----------------------------------------------------------------------------
void SgtMayor::doShowTrump(Card::Widget::COLOURS colour) {
    TRACE9("SgtMayor::doShowTrump(Card::Widget::COLOURS) - " << colour);
    for (unsigned int i(0); i < cards.size(); ++i) {
        if ((cards.getCards()[i]->number() == Card::Widget::ACE) && (cards.getCards()[i]->colour() == colour)) {
            Check3(!pTrump);
            table.trump = colour;
            pTrump = std::make_unique<Card::Widget>(*cards.getCards()[i]);
            pTrump->show();
            pTrump->showFace();
            pTrump->set_margin_start(5);
            pTrump->set_margin_end(5);
            pTrump->set_margin_top(1);
            pTrump->set_margin_bottom(1);
            attach(*pTrump, 0, 7, 1, 1);
            displayTurn(convertPlayer(startPlayer));
            return;
        }
    }
    Check3(0);
}

//----------------------------------------------------------------------------
/// Plays the passed card; find winner and give him the cards at the end of a
/// turn.
/// \param player Player on turn
/// \returns unsigned int Next player; or -1U, if end of game
//----------------------------------------------------------------------------
unsigned int SgtMayor::playCard(unsigned int player) {
    TRACE3("SgtMayor::playCard(unsigned int, unsigned int) - Player " << player);

    Check3(table.trick.size() == played.size());
    if (played.size() == NUM_PLAYERS) {
        // Find the winner
        Check3(pTrump);
        unsigned int bestPlayer(SgtMayorRules::trickWinner(table.trick, table.trump));
        table.clearTrick();
        TRACE9("SgtMayor::playCard(unsigned int) - Calc. winner from " << player << " and " << bestPlayer);
        player = (player + 1 + bestPlayer) % NUM_PLAYERS;
        TRACE9("SgtMayor::playCard(unsigned int) - Winner " << player);

        // Move the cards to his won pile (but show only one of them)
        while (played.size() > 1) {
            Card::Widget& card(played.remove(0));
            card.hide();
            players[player].won.Card::IPile::append(card);
        }
        players[player].won.Card::IPile::append(played.removeTopCard());
        Check3(played.empty());

        if (!player) {
            enableWonCards(players[0].won);
            menuSort->set_enabled();
            menuSort2->set_enabled();
        }
    }
    else
        player = calcNextPlayer(player);

    if (players[player].hand.size()) {
        Check3(convertPlayer(player) < actPlayers.size());
        displayTurn(convertPlayer(player));
        return player;
    }

    Glib::ustring stat(_("Game ended; %1 has %2 %7, %3 %4 and %5 %6 %8"));
    std::array<unsigned int, NUM_PLAYERS> tricks{};
    for (unsigned int i(0); i < NUM_PLAYERS; ++i)
        tricks[i] = players[i].won.size() / NUM_PLAYERS;
    diffTricks = SgtMayorRules::roundScore(tricks, startPlayer);
    Check(!(diffTricks[0] + diffTricks[1] + diffTricks[2]));

    // Create score-dialog
    if (!pScoreDlg) {
        // Resort player for score dialogue
        std::vector<Card::Player*> player;
        for (unsigned int i(0); i < NUM_PLAYERS; ++i)
            player.push_back(actPlayers[convertPlayer(i)]);

        pScoreDlg.reset(Card::ScoreDlg::create(player));
        points.fill(0);
        Gtk::Window* win(dynamic_cast<Gtk::Window*>(get_root()));
        if (win)
            pScoreDlg->set_transient_for(*win);
        menuShowScoreDlg->set_enabled();
    }

    pScoreDlg->addPoints(diffTricks.data());
    pScoreDlg->display();
    for (unsigned int i(0); i < NUM_PLAYERS; ++i)
        points[i] += diffTricks[i];

    if (SgtMayorRules::isGameOver(points, ENDTRICKS)) {
        Glib::ustring won(_("; %1 won"));
        won.replace(won.find("%1"), 2, actPlayers[convertPlayer(SgtMayorRules::winner(points))]->getName());
        stat += won;
    }

    stat.replace(stat.find("%1"), 2, actPlayers[0]->getName());
    stat.replace(stat.find("%3"), 2, actPlayers[convertPlayer(1)]->getName());
    stat.replace(stat.find("%5"), 2, actPlayers[convertPlayer(2)]->getName());

    stat.replace(stat.find("%2"), 2, formatNumber(diffTricks[0]));
    stat.replace(stat.find("%4"), 2, formatNumber(diffTricks[1]));
    stat.replace(stat.find("%6"), 2, formatNumber(diffTricks[2]));

    stat.replace(stat.find("%7"), 2, (ngettext("trick", "tricks", (diffTricks[0] < 0) ? -diffTricks[0] : diffTricks[0])));
    stat.replace(stat.find("%8"), 2, (ngettext("trick", "tricks", (diffTricks[2] < 0) ? -diffTricks[2] : diffTricks[2])));

    status.pop();
    status.push(stat);
    setGameStatus(STOPPED);
    return -1U;
}

//-----------------------------------------------------------------------------
/// Formats a number with sign character always shown
/// \param nr Number to format
/// \returns std::string Formatted number
/// \remarks Shows the sign always (e.g. also the plus sign (+)
//-----------------------------------------------------------------------------
std::string SgtMayor::formatNumber(int nr) {
    std::ostringstream msg;
    msg << std::showpos << nr;
    return msg.str();
}

//----------------------------------------------------------------------------
/// Exchanges cards between players having too much/too less tricks in the last
/// round.
//----------------------------------------------------------------------------
void SgtMayor::makeExchange() {
    TRACE5("SgtMayor::makeExchange() - Exchanging cards: " << (diffTricks[0] > 0 ? diffTricks[0] : 0) +
                                                                  (diffTricks[1] > 0 ? diffTricks[1] : 0) +
                                                                  (diffTricks[2] > 0 ? diffTricks[2] : 0));

    if (const auto exchange(SgtMayorRules::nextExchange(diffTricks, startPlayer)); exchange) {
        const unsigned int playerBad(exchange->playerBad);
        TRACE9("SgtMayor::makeExchange() - " << playerBad << " with " << exchange->playerGood);
        if (playerBad) {
            if ((getConnectionMgr().getMode() == YGP::ConnectionMgr::CLIENT) ||
                (typeid(*actPlayers[convertPlayer(playerBad)]) == typeid(Card::RemotePlayer))) {
                // Wait for the Exchange-message (see handleMessage)
                Glib::ustring msg(_("Waiting for %1 to exchange cards ..."));
                msg.replace(msg.find("%1"), 2, actPlayers[convertPlayer(playerBad)]->getName());
                status.pop();
                status.push(msg);
            }
            else
                exchangeCards(playerBad, exchange->playerGood);
        }
        else {
            Check3(diffTricks[0] > 0);
            displayExchangeStatus();

            for (int i(players[0].hand.size() - 1); i >= 0; --i)
                activeCards.push_back(
                    players[0].hand[i]->signal_clicked().connect(bind(mem_fun(*this, (&SgtMayor::cardExchange)), i)));
        }
        return;
    }

    // Glib::signal_timeout ().connect (bind_return (mem_fun (*this, &SgtMayor::makeExchange), false), 400);
    startPlaying();
}

//-----------------------------------------------------------------------------
/// Displays the number of cards the human can exchange and with whom
//-----------------------------------------------------------------------------
void SgtMayor::displayExchangeStatus() {
    Glib::ustring msg;
    unsigned int exchg(1);

    if ((diffTricks[1] < 0) && (diffTricks[2] < 0)) {
        msg = _("You can exchange %1 %2; %4 with %3 (and than %5 with %6)!");

        msg.replace(msg.find("%4"), 2, 1, static_cast<char>((diffTricks[1] < 0) ? ('0' - diffTricks[1]) : ('0' + diffTricks[1])));
        msg.replace(msg.find("%5"), 2, 1, static_cast<char>((diffTricks[2] < 0) ? ('0' - diffTricks[2]) : ('0' + diffTricks[2])));
        msg.replace(msg.find("%6"), 2, actPlayers[convertPlayer(2)]->getName());
    }
    else {
        msg = _("You can exchange %1 %2 with %3!");
        if (diffTricks[2] < 0)
            exchg = 2;
    }
    msg.replace(msg.find("%1"), 2, 1, static_cast<char>((diffTricks[0] < 0) ? ('0' - diffTricks[0]) : ('0' + diffTricks[0])));
    msg.replace(msg.find("%2"), 2, (ngettext("bad card", "bad cards", (diffTricks[0] < 0) ? -diffTricks[0] : diffTricks[0])));
    msg.replace(msg.find("%3"), 2, actPlayers[convertPlayer(exchg)]->getName());

    status.pop();
    status.push(msg);
}

//----------------------------------------------------------------------------
/// Exchanges a good card from playerGood with a bad card from player bad
/// \param playerBad Player giving away a bad card
/// \param playerGood Player giving away a good card
//----------------------------------------------------------------------------
void SgtMayor::exchangeCards(unsigned int playerBad, unsigned int playerGood) {
    TRACE7("SgtMayor::exchangeCards(unsigned int, unsigned int) - Players " << playerBad << " and " << playerGood);
    Check1(playerBad < NUM_PLAYERS);
    Check1(playerGood < NUM_PLAYERS);

    unsigned int posBad(SgtMayorRules::selectBadCard(players[playerBad].hand.values()));
    Check3(posBad < players[playerBad].hand.size());
    exchangeCards(playerBad, posBad, playerGood);
}

//-----------------------------------------------------------------------------
/// Animated exchange of a card; after the exchange, the receiver gives back a card.
/// \param playerBad Player giving away a bad card
/// \param posBad Position of bad card to give away
/// \param playerGood Player giving away a good card
/// \param posGood Position of good card to give away
//-----------------------------------------------------------------------------
void SgtMayor::exchange(unsigned int playerBad, unsigned int posBad, unsigned int playerGood, unsigned int posGood) {
    Check1(playerBad < NUM_PLAYERS);
    Check1(playerGood < NUM_PLAYERS);
    Check2(playerBad != playerGood);
    Check2(posBad < players[playerBad].hand.size());
    Check2(posGood < players[playerGood].hand.size());

    Card::HPile& pileBad(players[playerBad].hand);
    Card::HPile& pileGood(players[playerGood].hand);
    TRACE9("SgtMayor::exchange(4x unsigned int ) - Player " << playerBad << " and " << playerGood << " exchange "
                                                            << *pileBad[posBad] << " and " << *pileGood[posGood]);

    // Show the human the card he gets
    if (!playerGood)
        showExchangedCard(pileBad, posBad);
    animateCard(pileGood, pileBad, posBad)
        .sigAnimation.connect(bind(mem_fun(*this, &SgtMayor::exchgBack), playerBad, playerGood, posGood));
}

//-----------------------------------------------------------------------------
/// Animated exchange of a card; after the exchange further cards might be exchanged
/// \param playerBad Player giving away a bad card
/// \param playerGood Player giving away a good card
/// \param posGood Position of good card to give away
//-----------------------------------------------------------------------------
void SgtMayor::exchgBack(unsigned int playerBad, unsigned int playerGood, unsigned int posGood) {
    Card::HPile& pileGood(players[playerGood].hand);
    Card::HPile& pileBad(players[playerBad].hand);

    if (!playerBad)
        showExchangedCard(pileGood, posGood);
    animateCard(pileBad, pileGood, posGood).sigAnimation.connect(bind(mem_fun(*this, &SgtMayor::exchgNext), &pileGood, &pileBad));
}

//-----------------------------------------------------------------------------
/// Shows the face of a card, which is given to the human while exchanging cards
/// \param pile Pile holding the card
/// \param pos Position of the card in the pile; updated if the card is moved
//-----------------------------------------------------------------------------
void SgtMayor::showExchangedCard(Card::IPile& pile, unsigned int& pos) {
    // The server must not use flipCards2Play, as that informs the clients about playing the card
    if (getConnectionMgr().getMode() == YGP::ConnectionMgr::SERVER)
        pile[pos]->showFace();
    else
        flipCards2Play(pile, pos, pos);
}

//-----------------------------------------------------------------------------
/// Sorts the passed pile and exchanges the next cards (or starts the game)
/// \param pileGood Pile to sort
/// \param pileBad Pile to sort
//-----------------------------------------------------------------------------
void SgtMayor::exchgNext(Card::HPile* pileGood, Card::HPile* pileBad) {
    Check1(pileGood);
    Check1(pileBad);
    pileBad->sortByColour();
    pileGood->sortByColour();

#ifdef WITH_NETWORK
    exchanging = false;
#endif
    makeExchange();

#ifdef WITH_NETWORK
    // A received exchange has been finished; continue with the next message
    if (remoteExchange) {
        remoteExchange = false;
        releaseMessageLock();
    }
    if (deferredMsg.size())
        handleDeferredMessage();
#endif
}

//----------------------------------------------------------------------------
/// Exchanges a good card from playerGood with a bad card from player bad
/// \param playerBad Player giving away a bad card
/// \param posBad Position of bad card to give away
/// \param playerGood Player giving away a good card
//----------------------------------------------------------------------------
void SgtMayor::exchangeCards(unsigned int playerBad, unsigned int posBad, unsigned int playerGood) {
    TRACE7("SgtMayor::exchangeCards(3x unsigned int int) - Players " << playerBad << " and " << playerGood);
    Check1(playerBad < NUM_PLAYERS);
    Check1(playerGood < NUM_PLAYERS);
    Check1(posBad < players[playerBad].hand.size());

    unsigned int posGood(SgtMayorRules::selectGoodCard(players[playerGood].hand.values(), *players[playerBad].hand[posBad]));
    TRACE9("SgtMayor::exchangeCards(3x unsigned int) - Player " << playerBad << ", card " << posBad << " with " << playerGood
                                                                << "'s " << posGood);
    Check3(posGood < players[playerGood].hand.size());

    exchangeCards(playerBad, posBad, playerGood, posGood);
}

//----------------------------------------------------------------------------
/// Exchanges a good card from playerGood with a bad card from player bad
/// and informs the connected partners about it
/// \param playerBad Player giving away a bad card
/// \param posBad Position of bad card to give away
/// \param playerGood Player giving away a good card
//----------------------------------------------------------------------------
void SgtMayor::exchangeCards(unsigned int playerBad, unsigned int posBad, unsigned int playerGood, unsigned int posGood) {
    TRACE7("SgtMayor::exchangeCards(4x unsigned int int) - Players " << playerBad << " and " << playerGood);
    Check1(playerBad < NUM_PLAYERS);
    Check1(playerGood < NUM_PLAYERS);
    Check1(posBad < players[playerBad].hand.size());
    Check1(posGood < players[playerGood].hand.size());

    // Broadcast exchange-info to others
    if (getConnectionMgr().getMode() != YGP::ConnectionMgr::NONE) {
        std::ostringstream msg;
        msg << "Exchange=" << players[playerGood].hand[posGood]->id() << ";From=" << (playerGood + posServer) % NUM_PLAYERS
            << ";With=" << players[playerBad].hand[posBad]->id() << ";From=" << (playerBad + posServer) % NUM_PLAYERS;

        sendMove(msg.str());
    }
    doExchangeCards(playerBad, posBad, playerGood, posGood);
}

//----------------------------------------------------------------------------
/// Exchanges a good card from playerGood with a bad card from player bad
/// \param playerBad Player giving away a bad card
/// \param posBad Position of bad card to give away
/// \param playerGood Player giving away a good card
/// \param posGood Position of good card to give away
//----------------------------------------------------------------------------
void SgtMayor::doExchangeCards(unsigned int playerBad, unsigned int posBad, unsigned int playerGood, unsigned int posGood) {
    TRACE7("SgtMayor::doExchangeCards(4x unsigned int int) - Players " << playerBad << " and " << playerGood);
    Check1(playerBad < NUM_PLAYERS);
    Check1(playerGood < NUM_PLAYERS);
    Check1(posBad < players[playerBad].hand.size());
    Check1(posGood < players[playerGood].hand.size());
    TRACE9("SgtMayor::doExchangeCards(4x unsigned int ) - Player " << playerBad << " and " << playerGood << " exchange "
                                                                   << *players[playerBad].hand[posBad] << " and "
                                                                   << *players[playerGood].hand[posGood]);
    Check2(SgtMayorRules::isValidExchange(players[playerGood].hand.values(), posGood, *players[playerBad].hand[posBad]));

    SgtMayorRules::applyExchange(diffTricks, {playerBad, playerGood});

#ifdef WITH_NETWORK
    exchanging = true;
#endif
    exchange(playerBad, posBad, playerGood, posGood);
}

//-----------------------------------------------------------------------------
/// Adds game-specific menus
/// \param menu Menu to add game-specific entries to
/// \param actions Action group to add game-specific actions to
//-----------------------------------------------------------------------------
void SgtMayor::addMenus(const Glib::RefPtr<Gio::Menu>& menu, const Glib::RefPtr<Gio::SimpleActionGroup>& actions) {
    Check1(menu);
    Check1(actions);

    Glib::RefPtr<Gio::Menu> sub(Gio::Menu::create());

    menuSort = actions->add_action("SgMayorSort", mem_fun(*this, &SgtMayor::sortWonByNumber));
    sub->append(_("_Sort won cards (by number)"), "game.SgMayorSort");

    menuSort2 = actions->add_action("SgMayorSortCol", mem_fun(*this, &SgtMayor::sortWonByColour));
    sub->append(_("Sort won cards (by _colour)"), "game.SgMayorSortCol");

    Glib::RefPtr<Gio::Menu> sec(Gio::Menu::create());
    menuShowScoreDlg = actions->add_action("showScoreDlg", [this]() {
        if (pScoreDlg)
            pScoreDlg->display();
    });
    sec->append(_("Show score dialog"), "game.showScoreDlg");
    sub->append_section(sec);

    menu->append_submenu(_("_Sgt. Mayor"), sub);

    menuShowScoreDlg->set_enabled(false);
}

//-----------------------------------------------------------------------------
/// Removes the game-specific menus
/// \param menu Menu to remove game-specific entries from
/// \param actions Action group to remove game-specific actions from
//-----------------------------------------------------------------------------
void SgtMayor::removeMenus(const Glib::RefPtr<Gio::Menu>& menu, const Glib::RefPtr<Gio::SimpleActionGroup>& actions) {
    Check1(menu);
    Check1(actions);

    menu->remove(menu->get_n_items() - 1);
    actions->remove_action("SgMayorSort");
    actions->remove_action("SgMayorSortCol");
    actions->remove_action("showScoreDlg");
}

//-----------------------------------------------------------------------------
/// Shows or hides the won cards
/// \param show Flag if to show or to hide the cards
/// \param style Style of the won pile (ignored; only traced)
//-----------------------------------------------------------------------------
void SgtMayor::showWonCards(bool show, [[maybe_unused]] unsigned int style) {
    TRACE9("SgtMayor::showWonCards(bool, unsigned int) - " << show << '/' << style);

    for (auto c(players[0].won.begin()); c != players[0].won.end(); ++c)
        if (show)
            (*c)->show();
        else if (((c - players[0].won.begin()) % 3) != 2)
            (*c)->hide();

    if (show)
        Game::showWonCards(true);
    else
        Game::showWonCards(false, Card::IPile::QUITE_COMPRESSED);
}

//-----------------------------------------------------------------------------
/// Actions to take when the cards are resized
/// \pre The cardsize must be set in CardImages::WIDTH/HEIGHT
//-----------------------------------------------------------------------------
void SgtMayor::resizeCards() {
    played.set_size_request(Card::Images::WIDTH + 150, Card::Images::HEIGHT);
    for (unsigned int i(0); i < NUM_PLAYERS; ++i) {
        players[i].hand.set_size_request(Card::Images::WIDTH + 17 * (i ? 7 : 18), Card::Images::HEIGHT + 5);
        players[i].won.set_size_request(Card::Images::WIDTH + 17 * 7, Card::Images::HEIGHT + 5);
    }
}
