// PROJECT     : Cardgames
// SUBSYSTEM   : Machiavelli
// REFERENCES  :
// TODO        : Rewrite reorderTableToFit() to only iterate once over the piles
// BUGS        :
// AUTHOR      : Markus Schwab
// CREATED     : 05.11.2003
// COPYRIGHT   : Copyright (C) 2003 - 2009, 2011, 2012, 2024, 2026

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
#include <string_view>

#include <gtk/gtk.h>

#include <glibmm/main.h>
#include <glibmm/value.h>

#include <gtkmm/messagedialog.h>
#include <gtkmm/statusbar.h>

#include <giomm/menu.h>
#include <giomm/simpleactiongroup.h>

#include <gdkmm/contentprovider.h>

#include <YGP/ANumeric.h>
#include <YGP/AttrParse.h>
#include <YGP/Check.h>
#include <YGP/ConnMgr.h>
#include <YGP/StatusObj.h>
#include <YGP/Trace.h>

#include <XGP/MessageDlg.h>
#include <XGP/XDialog.h>

#include <card/ComputerPlayer.h>
#include <card/Images.h>
#include <card/Message.h>
#include <card/Player.h>
#include <card/Random.h>
#include <card/Set.h>
#include <card/Widget.h>
#include <card/Window.h>

#include "Machiavelli.h"

// Some Windows-header seems to define ERROR
#ifdef ERROR
#    undef ERROR
#endif

// Discriminant encoded into the high byte of the drag-and-drop payload (see
// registerHandDND()/registerTableDND()) to tell apart cards dragged from the
// hand from cards dragged from a pile on the table - GTK4 no longer provides
// a separate "info" alongside the dropped data, so both must travel together
// in a single int.
enum { HAND, TABLE };

namespace {
constexpr int makeDNDPayload(unsigned int info, unsigned int pos) { return static_cast<int>((info << 24) | (pos & 0xffffffU)); }
constexpr unsigned int dndInfo(int payload) { return static_cast<unsigned int>(payload) >> 24; }
constexpr unsigned int dndPos(int payload) { return static_cast<unsigned int>(payload) & 0xffffffU; }
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
Machiavelli::Machiavelli(Gtk::Box& parent, Gtk::Statusbar& statusbar, Card::Set& cardset,
                         const std::vector<Card::Player*>& player, unsigned int posPlayer, Card::MessageLock& mxSerialize)
    : Game(parent, statusbar, cardset, player, posPlayer, mxSerialize, 3, 10), piles(), tablePiles(), startPlayer(-1U),
      newPile(_("New pile")), dstNewPile(), staple(Card::IPile::TOTALLY_COMPRESSED, Card::IPile::SHOWBACK),
      nextTurn(_("_End turn"), true), aDNDHand(), aDNDTable(), target(-1U), posPiles(), undo(), undoDlg(nullptr), undo1(),
      undoAll(), nxtTurn() {
    TRACE9("Machiavelli::Machiavelli(Box&, Statusbar&, CardSet&, const std::vector<Glib::ustring>&)");

    TRACE9("Machiavelli::Machiavelli(Box&, Statusbar&, CardSet&, const std::vector<Glib::ustring>&) - Init common staples");
    for (unsigned int i(1); i < NUM_PLAYERS; ++i) {
        hands[i].set_hexpand();
        hands[i].set_margin_start(5);
        hands[i].set_margin_end(5);
        hands[i].set_margin_top(5);
        hands[i].set_margin_bottom(5);
        attach(hands[i], ((NUM_PLAYERS - i) << 2) - 4, 0, 4, 1);
        names[i].set_hexpand();
        attach(names[i], ((NUM_PLAYERS - i) << 2) - 4, 1, 4, 1);
        hands[i].setStyle(Card::IPile::QUITE_COMPRESSED);
        hands[i].setShowOption(Card::IPile::SHOWBACK);
    }
    hands[0].setStyle(Card::IPile::COMPRESSED);
    hands[0].setShowOption(Card::IPile::SHOWFACE);

    TRACE9("Machiavelli::Machiavelli(Box&, Statusbar&, CardSet&, const std::vector<Glib::ustring>&) - Attach widgets");
    hands[0].set_hexpand();
    hands[0].set_margin_start(1);
    hands[0].set_margin_end(1);
    hands[0].set_margin_top(5);
    hands[0].set_margin_bottom(5);
    attach(hands[0], 3, 4, 9, 1);
    names[0].set_hexpand();
    names[0].set_margin_start(1);
    names[0].set_margin_end(1);
    names[0].set_margin_top(5);
    names[0].set_margin_bottom(5);
    attach(names[0], 3, 5, 9, 1);
    staple.set_margin_start(5);
    staple.set_margin_end(5);
    attach(staple, 0, 4, 1, 1);
    nextTurn.set_margin_start(5);
    nextTurn.set_margin_end(5);
    attach(nextTurn, 0, 5, 1, 1);
    newPile.set_hexpand();
    newPile.set_margin_top(5);
    newPile.set_margin_bottom(5);
    attach(newPile, 0, 3, 12, 1);
    piles.set_hexpand();
    piles.set_vexpand();
    piles.set_margin_top(5);
    piles.set_margin_bottom(5);
    attach(piles, 0, 2, 12, 1);

    TRACE9("Machiavelli::Machiavelli(Box&, Statusbar&, CardSet&, const std::vector<Glib::ustring>&) - Show widgets");

    changeNames(player);

    // Simplification vs. GTK3: no set_can_default()/grab_default() equivalent
    // is wired up here anymore, since that requires the top-level Gtk::Window,
    // which isn't reliably reachable via get_root() until this widget has been
    // added to the widget tree by the caller.
    nextTurn.set_receives_default();
    nextTurn.set_sensitive(false);
    nextTurn.signal_clicked().connect(mem_fun(*this, (&Machiavelli::endTurn)));

    resizeCards();
}

//-----------------------------------------------------------------------------
/// Destructor
//-----------------------------------------------------------------------------
Machiavelli::~Machiavelli() {
    TRACE9("Machiavelli::~Machiavelli()");
    clean();
}

//-----------------------------------------------------------------------------
/// Starts the game by dealing the cards
//-----------------------------------------------------------------------------
void Machiavelli::start() {
    TRACE6("Machiavelli::start()");
    Game::start();

    pos1Play = pos2Play = -1U;
    target = -1U;
    posPiles.clear();

    if (randomiseCardsToPile(staple)) {
        for (unsigned int i(0); i < NUM_PLAYERS; ++i)
            hands[(i - posServer) & 0x3].getCards(staple, staple.size() - MachiavelliRules::CARDS_PER_PLAYER, staple.size() - 1);

        hands[0].sortByColour();
        for (unsigned int i(1); i < NUM_PLAYERS; ++i) {
            hands[i].setStyle(Card::IPile::QUITE_COMPRESSED);
            hands[i].setShowOption(Card::IPile::SHOWBACK);
            hands[i].sortByNumber();
        }

        status.pop();
        status.push(_("You can sort the cards in your hand with drag and drop or put"
                      " them on the table - click the staple to end turn"));

        if (getConnectionMgr().getMode() != YGP::ConnectionMgr::CLIENT) {
            // Set random startplayer (if not already set)
            if (startPlayer == -1U)
                startPlayer = Card::randomNumber(4);
            setStartPlayer();
        }
    }
}

//-----------------------------------------------------------------------------
/// Remove cards from everything which can hold them
//-----------------------------------------------------------------------------
void Machiavelli::clean() {
    TRACE6("Machiavelli::clean()");
    Game::clean();

    for (auto& hand : hands)
        hand.clear();

    staple.clear();

    for (const auto& tablePile : tablePiles) {
        tablePile->clear();
        piles.remove(*tablePile);
    }
    tablePiles.clear(); // Deletes the piles
}

//-----------------------------------------------------------------------------
/// Shows or hides the cards of the computer player
/// \param open Flag if cards should be shown or hidden
//-----------------------------------------------------------------------------
void Machiavelli::playOpen(bool open) {
    for (unsigned int i(1); i < NUM_PLAYERS; ++i) {
        hands[i].setShowOption(open ? Card::IPile::SHOWFACE : Card::IPile::SHOWBACK);
        hands[i].setStyle(open ? Card::IPile::COMPRESSED : Card::IPile::QUITE_COMPRESSED);
    }
}

//-----------------------------------------------------------------------------
/// Makes the move for the next player.
/// \param player Actual player
/// \remarks For a remote player (whose cards to play have already been
///     flipped) this method expects the target pile to play to in the
///     target-member and the positions of the cards in pos1Play and pos2Play;
///     else the move of the computer player is calculated
//-----------------------------------------------------------------------------
void Machiavelli::makeMove(unsigned int player) {
    TRACE5("Machiavelli::makeMove(unsigned int) - Turn of player " << player << "; Target: " << std::hex << target << std::dec);
    Check1(player);
    Check1(player < NUM_PLAYERS);

#ifdef WITH_NETWORK
    // Cards of a remote player; already received and flipped
    if (isShowingCardsToPlay()) {
        playRemoteCards(player);
        return;
    }
#endif
    Check1(gameStatus() == PLAYING);

    // No more cards found: Continue with next player
    if (showCardsToPlay(player)) {
        if (getConnectionMgr().getMode() != YGP::ConnectionMgr::NONE) {
            // Send end of turn to all clients (if any)
            sendMove("EndTurn");
        }

        unsigned int nextPlayer(findNextPlayer(player));
        if (MachiavelliRules::isGameOver(handSizes(), nextPlayer)) {
            endGame(nextPlayer);
            return;
        }
        displayTurn(player = nextPlayer);
        dealCard(player);
        setNextPlayer(player);
    }
}

//-----------------------------------------------------------------------------
/// Enables the cards the human can pick up.
/// \returns \c 0
//-----------------------------------------------------------------------------
bool Machiavelli::enableHuman() {
    TRACE4("Machiavelli::enableHuman()");
    Check3(staple.size());
    Check3(activeCards.empty());

    if (staple.size())
        activeCards.push_back(staple.getTopCard().signal_clicked().connect(mem_fun(*this, (&Machiavelli::endTurn))));

    for (unsigned int i(0); i < hands[0].size(); ++i)
        registerHandDND(i);
    Check3(aDNDHand.size() == hands[0].size());

    dstNewPile = Gtk::DropTarget::create(G_TYPE_INT, Gdk::DragAction::MOVE);
    dstNewPile->signal_drop().connect(
        [this](const Glib::ValueBase& value, double, double) -> bool { return cardDroppedOnTable(value, -1U); }, false);
    newPile.add_controller(dstNewPile);

    for (unsigned int i(0); i < tablePiles.size(); ++i) {
        MachiPile& pile(*tablePiles[i]);
        unsigned int value(i << 8);
        for (auto& j : pile)
            registerTableDND(*j, value++);
    }

    nxtTurn->set_enabled();
    nextTurn.set_sensitive();

    return Game::enableHuman();
}

//-----------------------------------------------------------------------------
/// Disables the cards the human player can select
//-----------------------------------------------------------------------------
void Machiavelli::disableHuman() {
    TRACE2("Machiavelli::disableHuman() - DND: " << aDNDHand.size() << "; " << aDNDTable.size());
    Game::disableHuman();

    if (dstNewPile) {
        newPile.remove_controller(dstNewPile);
        dstNewPile.reset();
    }

    if (aDNDHand.size())
        for (auto& i : hands[0])
            unregisterHandDND(*i);
    TRACE9("Machiavelli::disableHuman() - Remaining cards: " << aDNDHand.size());
    Check3(aDNDHand.empty());

    unregisterTableDND();
    nextTurn.set_sensitive(false);
    nxtTurn->set_enabled(false);

    if (undo.empty()) {
        undo1->set_enabled(false);
        undoAll->set_enabled(false);
    }
}

//----------------------------------------------------------------------------
/// Changes the names of the playing people
/// \param newPlayer Array holding the new player
//----------------------------------------------------------------------------
void Machiavelli::changeNames(const std::vector<Card::Player*>& newPlayer) {
    Game::changeNames(newPlayer);

    for (unsigned int i(0); i < NUM_PLAYERS; ++i) {
        TRACE1("Machiavelli::changeNames() " << i << ": " << newPlayer[i]->getName());
        names[i].set_text(newPlayer[i]->getName());
    }
}

//----------------------------------------------------------------------------
/// Sets the startplayer (stored in startPlayer); including showing it in the
/// status bar and dealing him a card. Afterwards startPlayer holds the player
/// to start the next game.
/// \returns bool True, if a card is dealt (the game continues after its animation)
//----------------------------------------------------------------------------
bool Machiavelli::setStartPlayer() {
    if (getConnectionMgr().getMode() != YGP::ConnectionMgr::CLIENT) {
        setNextPlayer(startPlayer);
        broadcastStartPlayer(startPlayer);
    }

    const bool dealt(dealCard(startPlayer));
    displayTurn(startPlayer++);
    startPlayer &= 0x3;
    return dealt;
}

//-----------------------------------------------------------------------------
/// Callback to end a turn. Checks if the piles are OK
//-----------------------------------------------------------------------------
void Machiavelli::endTurn() {
    TRACE5("Machiavelli::endTurn()");
    Check1(gameStatus() == PLAYING);
    Check3(staple.size());
    Check3(activeCards.size());

    // Show error, if any
    YGP::StatusObject obj;
    checkPiles(obj, true);
    if (obj.getType() != YGP::StatusObject::UNDEFINED) {
        obj.generalize(_("Can't end turn: The piles are not valid!"));
        undoDlg = std::make_unique<XGP::MessageDlg>(obj);
        undoDlg->set_title(PACKAGE);
        if (Gtk::Root* root = get_root())
            if (Gtk::Window* win = dynamic_cast<Gtk::Window*>(root))
                undoDlg->set_transient_for(*win);
        undoDlg->signal_response().connect(mem_fun(*this, &Machiavelli::removeUndoDlg));

        // Add undo-buttons
        Gtk::Button* undoAll(Gtk::make_managed<Gtk::Button>(_("_Undo all"), true));
        Gtk::Button* undoLast(Gtk::make_managed<Gtk::Button>(_("Undo _last"), true));

        undoDlg->add_action_widget(*undoLast, 0);
        undoDlg->add_action_widget(*undoAll, 0);

        undoAll->signal_clicked().connect(bind(mem_fun(*this, &Machiavelli::undoMove), -1U));
        undoLast->signal_clicked().connect(bind(mem_fun(*this, &Machiavelli::undoMove), 1));
    }
    else
        doEndTurn();
}

//-----------------------------------------------------------------------------
/// Ends a turn without checking if the piles are OK
//-----------------------------------------------------------------------------
void Machiavelli::doEndTurn() {
    TRACE5("Machiavelli::doEndTurn()");
    Check1(gameStatus() == PLAYING);
    Check3(staple.size());
    Check3(activeCards.size());
    while (undo.size())
        undo.pop();

    if (getConnectionMgr().getMode() != YGP::ConnectionMgr::NONE) {
        // Send played card to all clients (if any)
        sendMove("EndTurn");
    }

    disableHuman();

    unsigned int nextPlayer(findNextPlayer(currentPlayer()));
    setNextPlayer(nextPlayer);
    if (MachiavelliRules::isGameOver(handSizes(), nextPlayer))
        endGame(nextPlayer);
    else {
        displayTurn(nextPlayer);
        dealCard(nextPlayer);
    }
}

//-----------------------------------------------------------------------------
/// Prepares the passed region of cards for drag'n'drop
/// \param start Number of first card to prepare for DND
/// \param end Number of last card to prepare for DND
/// \pre \c start < \c end; \c end <= Nr. ofcards
//-----------------------------------------------------------------------------
void Machiavelli::registerHandDND(unsigned int start, unsigned int end) {
    TRACE9("Machiavelli::registerHandDND(unsigned int, unsigned int) - [" << start << '-' << end << ']');
    Check1(start <= end);
    Check1(end < hands[0].size());

    for (; start <= end; ++start) {
        unregisterHandDND(*hands[0][start]);
        registerHandDND(start);
    }
}

//-----------------------------------------------------------------------------
/// Prepares the card for drag'n'drop
/// \param iCard Number of card in hand
//-----------------------------------------------------------------------------
void Machiavelli::registerHandDND(unsigned int iCard) {
    Check1(iCard < hands[0].size());
    TRACE9("Machiavelli::registerHandDND(unsigned int) - Card: " << iCard << " (" << *hands[0][iCard] << ')');

    Card::Widget& card(*hands[0][iCard]);
    Check3(aDNDHand.find(&card) == aDNDHand.end());

    CONNECTIONS conn;

    // Card accepts drops from hand (re-ordering within the hand) ...
    conn.dst = Gtk::DropTarget::create(G_TYPE_INT, Gdk::DragAction::MOVE);
    conn.dst->signal_drop().connect(
        [this, iCard](const Glib::ValueBase& value, double, double) -> bool { return cardDropped(value, iCard); }, false);
    card.add_controller(conn.dst);

    // ... and can be dragged (within the hand)
    conn.src = Gtk::DragSource::create();
    conn.src->set_actions(Gdk::DragAction::MOVE);
    conn.src->signal_prepare().connect(
        [iCard](double, double) -> Glib::RefPtr<Gdk::ContentProvider> {
            Glib::Value<int> v;
            v.init(Glib::Value<int>::value_type());
            v.set(makeDNDPayload(HAND, iCard));
            return Gdk::ContentProvider::create(v);
        },
        false);
    card.add_controller(conn.src);

    aDNDHand[&card] = conn;
}

//-----------------------------------------------------------------------------
/// Stops the drag'n'drop abilities of the passed card
/// \param card Card to unregister of dnd
//-----------------------------------------------------------------------------
void Machiavelli::unregisterHandDND(Card::Widget& card) {
    TRACE9("Machiavelli::unregisterHandDND(Card::Widget&) - Card: " << card);
    Check1(aDNDHand.size());

    auto i(aDNDHand.find(&card));
    Check1(i != aDNDHand.end());

    card.remove_controller(i->second.dst);
    card.remove_controller(i->second.src);
    aDNDHand.erase(i);
}

//-----------------------------------------------------------------------------
/// Prepares the passed region of cards for drag'n'drop
/// \param pile Pile whose cards should be registered. This value is calcualated
///     like (row << 4) + column
/// \param start Number of first card to prepare for DND
/// \param end Number of last card to prepare for DND
/// \pre \c start < \c end; \c end <= Number of cards
//-----------------------------------------------------------------------------
void Machiavelli::registerTableDND(unsigned int pile, unsigned int start, unsigned int end) {
    TRACE9("Machiavelli::registerTableDND(unsigned int, unsigned int, unsigned int)" << " - " << pile << '[' << start << '-'
                                                                                     << end << ']');
    Check1(pile < tablePiles.size());
    Check1(start <= end);
    Check1(end < tablePiles[pile]->size());

    Card::IPile& tmp(*tablePiles[pile]);

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
void Machiavelli::registerTableDND(Card::Widget& card, unsigned int nr) {
    TRACE9("Machiavelli::registerTableDND(Card::Widget&, unsigned int) - " << card << " = " << std::hex << nr << std::dec);

    CONNECTIONS conn;

    // Card accepts drops from hand and from the table ...
    conn.dst = Gtk::DropTarget::create(G_TYPE_INT, Gdk::DragAction::MOVE);
    conn.dst->signal_drop().connect(
        [this, nr](const Glib::ValueBase& value, double, double) -> bool { return cardDroppedOnTable(value, nr); }, false);
    card.add_controller(conn.dst);

    // ... and can be dragged from the table
    conn.src = Gtk::DragSource::create();
    conn.src->set_actions(Gdk::DragAction::MOVE);
    conn.src->signal_prepare().connect(
        [nr](double, double) -> Glib::RefPtr<Gdk::ContentProvider> {
            Glib::Value<int> v;
            v.init(Glib::Value<int>::value_type());
            v.set(makeDNDPayload(TABLE, nr));
            return Gdk::ContentProvider::create(v);
        },
        false);
    card.add_controller(conn.src);

    aDNDTable[&card] = conn;
}

//-----------------------------------------------------------------------------
/// Stops the drag'n'drop abilities of the passed card
/// \param card Card to de-register
//-----------------------------------------------------------------------------
void Machiavelli::unregisterTableDND(Card::Widget& card) {
    TRACE9("Machiavelli::unregisterTableDND(unsigned int) - Card: " << card << " - " << &card);
    Check1(aDNDTable.size());

    auto i(aDNDTable.find(&card));
    Check1(i != aDNDTable.end());

    card.remove_controller(i->second.dst);
    card.remove_controller(i->second.src);
    aDNDTable.erase(i);
}

//-----------------------------------------------------------------------------
/// Stops the drag'n'drop abilities of all cards on the table
//-----------------------------------------------------------------------------
void Machiavelli::unregisterTableDND() {
    for (auto& i : aDNDTable) {
        i.first->remove_controller(i.second.dst);
        i.first->remove_controller(i.second.src);
    }

    aDNDTable.clear();
}

//-----------------------------------------------------------------------------
/// Callback after dropping a card (within the hand)
/// \param value Drag payload (see makeDNDPayload())
/// \param card Number of card where something was dropped at
/// \returns bool True, if the drop was handled
//-----------------------------------------------------------------------------
bool Machiavelli::cardDropped(const Glib::ValueBase& value, unsigned int card) {
    Check3(card < hands[0].size());

    Glib::Value<int> v;
    v.init(value.gobj());
    Check3(dndInfo(v.get()) == HAND);
    unsigned int srcPos(dndPos(v.get()));
    Check3(srcPos < hands[0].size());
    TRACE1("Machiavelli::cardDropped(...) - Inserting card " << srcPos << " at pos " << card);

    Card::Widget& cardMoved(hands[0].remove(srcPos));
    hands[0].insert(cardMoved, card); // Insert moved card

    // Adapt dnd-settings
    if (srcPos < card) {
        unsigned int temp(card);
        card = srcPos;
        srcPos = temp;
    }

    Glib::signal_idle().connect(bind(mem_fun(*this, &Machiavelli::doRegisterHand), card, srcPos));
    return true;
}

//-----------------------------------------------------------------------------
/// Checks if the piles on the table are valid (have at least 3 cards)
/// \param except Pile which can be invalid
/// \returns bool True, if the piles are OK
//-----------------------------------------------------------------------------
bool Machiavelli::doRegisterHand(unsigned int first, unsigned int last) {
    TRACE9("Buraco::doRegisterHand(unsigned int, unsigned int) - [" << first << '-' << last);
    Check1(last < hands[0].size());
    Check1(first <= last);

    registerHandDND(first, last);
    Check3(aDNDHand.size() == hands[0].size());
    return false;
}

//-----------------------------------------------------------------------------
/// Callback after dropping a card on the table
/// \param value Drag payload (see makeDNDPayload())
/// \param iCard Combination of card and pile on which card was dropped
///     (or -1U if dropped on the "new pile" label)
/// \returns bool True, if the drop was handled
//-----------------------------------------------------------------------------
bool Machiavelli::cardDroppedOnTable(const Glib::ValueBase& value, unsigned int iCard) {
    Glib::Value<int> v;
    v.init(value.gobj());
    unsigned int info(dndInfo(v.get()));
    unsigned int pos(dndPos(v.get()));
    TRACE1("Machiavelli::cardDroppedOnTable(...) - Card dropped on " << std::hex << iCard << std::dec << "; " << info);
    Check1((info == HAND) || (info == TABLE));

    unsigned int nrpile(pos >> 8);
    unsigned int off(pos & 0xff);
    Check2((info == HAND) ? (off < hands[0].size()) : (nrpile < tablePiles.size() && (off < tablePiles[nrpile]->size())));

    // Move dropped card to a (new) pile on the table
    unsigned int iPile((iCard == -1U) ? tablePiles.size() : (iCard >> 8));
    Check1(iPile <= tablePiles.size());

    // Ignore dnd from a pile to itself
    if ((info == TABLE) && (iPile == nrpile))
        return true;

    Card::IPile& src((info == HAND) ? hands[0] : *tablePiles[nrpile]);
    TRACE4("Machiavelli::cardDroppedOnTable(...) - Card dropped: " << *src[off]);

    MachiavelliRules::Move move;
    const MachiavelliRules::MoveError error(MachiavelliRules::checkMove(
        hands[0].values(), currentTable(), (info == HAND) ? MachiavelliRules::HAND : nrpile, off, iPile, move));
    if (error != MachiavelliRules::MoveError::NONE) {
        Gtk::MessageDialog dlg(_(MachiavelliRules::describe(error)), false, Gtk::MessageType::ERROR);
        dlg.set_title(_("Invalid move"));
        XGP::runModal(dlg);
        return false;
    }

    const MachiavelliRules::Transfer& transfer(move.transfers[0]);
    off = transfer.first;
    unsigned int nr(transfer.number());
    iCard = transfer.destPos;
    MachiPile* pile((iPile == tablePiles.size()) ? &makeNewPile() : tablePiles[iPile].get());
    Check3(pile);

    // Store undo-info 4 Bytes: Target-pile, target-card, source-pile,
    // source-card; if played from hand, set source-pile to 0xff
    undoValue val(iPile, iCard, (info == HAND) ? 0xff : nrpile, off, nr);

    // Send move
    if (getConnectionMgr().getMode() != YGP::ConnectionMgr::NONE) {
        std::ostringstream msg;
        if (info == HAND)
            msg << "Play=" << src[off]->id();
        else
            msg << "Reorder=" << ((nr << 16) + (nrpile << 8) + off);

        msg << ";Target=" << (iPile << 16) + iCard;

        sendMove(msg.str());
    }

    TRACE8("Machiavelli::cardDroppedOnTable(...) - Moving " << nr << " cards from " << off);
    while (nr--) {
        TRACE8("Machiavelli::cardDroppedOnTable(...) - Insert to: " << iPile << "; Pos: " << iCard);
        Check3(iCard != -1U);

        // Unregister old card
        Card::Widget* moved(src[off]);
        src.remove(off);
        (info == HAND) ? unregisterHandDND(*moved) : unregisterTableDND(*moved);

        // Insert card into pile and register it for DND
        Check3(iCard <= pile->size());
        pile->insert(*moved, iCard);
        registerTableDND(*moved, (iPile << 8) + iCard);
        if (iCard < (pile->size() - 1))
            registerTableDND(iPile, iCard + 1, pile->size() - 1);

        iCard++;
    }

    // Check if human got rid of all cards
    if (hands[0].empty()) {
        YGP::StatusObject obj;
        checkPiles(obj, true);
        if (obj.getType() == YGP::StatusObject::UNDEFINED) {
            doEndTurn();
            return true;
        }
    }

    if (info == TABLE) {
        if (src.empty()) { // Pile moved completely?
            Check3(tablePiles[nrpile].get() == &src);
            removePile(nrpile);
            val.create = true;

            if (nrpile > iPile)
                nrpile = iPile;

            // Re-register the following piles
            while (nrpile < tablePiles.size()) {
                registerTableDND(nrpile, 0, tablePiles[nrpile]->size() - 1);
                ++nrpile;
            }
        }
        else {
            MachiPile& srcPile(*tablePiles[nrpile]);
            for (unsigned int i(0); i < srcPile.size(); ++i) {
                unregisterTableDND(*srcPile[i]);
                registerTableDND(*srcPile[i], (nrpile << 8) + i);
            }
            srcPile.markValidity();
        }
    }

    // Re-register the cards in the hand of the human for DND
    if ((info == HAND) && pos < hands[0].size())
        registerHandDND(pos, hands[0].size() - 1);
    Check3(aDNDHand.size() == hands[0].size());

    pile->markValidity();

    undo.push(val);
    undo1->set_enabled(true);
    undoAll->set_enabled(true);
    return true;
}

//----------------------------------------------------------------------------
/// Finds the next player still having cards
/// \param player Player to find next player to
/// \return unsigned int Next player having cards
/// \remarks We assume (without really checking), that there's a next player.
//----------------------------------------------------------------------------
unsigned int Machiavelli::findNextPlayer(unsigned int player) const {
    player = MachiavelliRules::findNextPlayer(handSizes(), player);
    TRACE8("Machiavelli::findNextPlayer(unsigned int) const - Player: " << player);
    return player;
}

//----------------------------------------------------------------------------
/// Returns the number of cards in the hands of the players
/// \returns std::array<unsigned int, NUM_PLAYERS> Number of cards per player
//----------------------------------------------------------------------------
std::array<unsigned int, Machiavelli::NUM_PLAYERS> Machiavelli::handSizes() const {
    std::array<unsigned int, NUM_PLAYERS> sizes{};
    for (unsigned int i(0); i < NUM_PLAYERS; ++i)
        sizes[i] = hands[i].size();
    return sizes;
}

//----------------------------------------------------------------------------
/// Returns the piles on the table (without display)
/// \returns MachiavelliRules::Table Piles on the table
//----------------------------------------------------------------------------
MachiavelliRules::Table Machiavelli::currentTable() const {
    MachiavelliRules::Table table;
    table.reserve(tablePiles.size());
    for (const auto& pile : tablePiles)
        table.push_back(pile->rules());
    return table;
}

//-----------------------------------------------------------------------------
/// Makes a new pile.
/// \returns MachiPile& New created pile
//-----------------------------------------------------------------------------
MachiPile& Machiavelli::makeNewPile() {
    TRACE8("Machiavelli::makeNewPile()");

    MachiPile& pile(*tablePiles.emplace_back(std::make_unique<MachiPile>()));
    pile.show();
    piles.add(pile);
    TRACE8("Machiavelli::makeNewPile() - Pile " << tablePiles.size());
    return pile;
}

//-----------------------------------------------------------------------------
/// Makes a new pile in a certain position
/// \param pos Position of pile on the table
/// \returns MachiPile& New created pile
//-----------------------------------------------------------------------------
MachiPile& Machiavelli::makeNewPile(unsigned int pos) {
    TRACE8("Machiavelli::makeNewPile()");

    Check3(pos <= tablePiles.size());
    MachiPile& pile(**tablePiles.insert(tablePiles.begin() + pos, std::make_unique<MachiPile>()));
    pile.show();
    piles.insert(pile, pos);
    return pile;
}

//-----------------------------------------------------------------------------
/// Searches for cards to play (see MachiavelliRules::selectMove) and shows them
/// in the hand of the actual player. Afterwards they are animated to the
/// target pile.
/// \param player Player to inspect
/// \returns bool True, if no card can be played
//-----------------------------------------------------------------------------
bool Machiavelli::showCardsToPlay(unsigned int player) {
    TRACE2("Machiavelli::showCardsToPlay(unsigned int) - Player " << player);
    Check3(posPiles.empty());

    Card::IPile& playerPile(hands[player]);
    Card::Cards hand(playerPile.values());
    const MachiavelliRules::Move move(MachiavelliRules::selectMove(hand, currentTable()));
    reorderHand(playerPile, hand); // The computer player might have re-ordered his cards

    const bool played(!move.empty());
    if (played)
        executeMove(playerPile, move);

#ifdef WITH_NETWORK
    // Inform the clients about the cards moved on the table (the cards played
    // from the hand have already been sent while flipping them)
    if (getConnectionMgr().getMode() == YGP::ConnectionMgr::SERVER)
        for (const auto& [src, dest] : posPiles) {
            std::ostringstream msg;
            msg << "Reorder=" << src << ";Target=" << dest;
            broadcastMessage(msg.str());
        }
#endif
    posPiles.clear();
    return !played;
}

//-----------------------------------------------------------------------------
/// Re-orders the cards in the passed pile, so that they are in the passed
/// order
/// \param pile Pile to re-order
/// \param order Cards of the pile in the new order
/// \pre The cards must be a permutation of the ones of the pile
//-----------------------------------------------------------------------------
void Machiavelli::reorderHand(Card::IPile& pile, const Card::Cards& order) {
    Check1(pile.size() == order.size());
    for (unsigned int i(0); i < order.size(); ++i)
        if (pile[i]->id() != order[i].id()) {
            int pos(pile.find(order[i].id(), i + 1));
            Check3(pos > 0);
            pile.move(i, pos);
        }
}

//-----------------------------------------------------------------------------
/// Executes the move of a computer player: The cards are animated to their
/// destination; cards moved on the table are marked til the end of the
/// animation
/// \param hand Cards of the player
/// \param move Move to execute
//-----------------------------------------------------------------------------
void Machiavelli::executeMove(Card::IPile& hand, const MachiavelliRules::Move& move) {
    TRACE5("Machiavelli::executeMove(Card::IPile&, const Move&) - To pile " << move.dest);
    Check1(move.transfers.size());
    Check1(move.dest <= tablePiles.size());

    bool fromTable(false);
    for (const auto& transfer : move.transfers)
        if (transfer.pile != MachiavelliRules::HAND) {
            Check3(transfer.pile < tablePiles.size());
            fromTable = true;
            Card::IPile& src(*tablePiles[transfer.pile]);
            for (unsigned int i(transfer.first); i <= transfer.last; ++i)
                src[i]->mark();
            addTableMove(transfer.pile, transfer.first, transfer.number(), move.dest, transfer.destPos);
        }

    // The cards of the first transfer are animated, the others follow them
    MachiavelliRules::Transfer first(move.transfers[0]);
    Card::IPile* src(&hand);
    if (first.pile == MachiavelliRules::HAND) {
        target = (move.dest << 16) + first.destPos;
        flipCards2Play(hand, first.first, first.last);
    }
    else
        src = tablePiles[first.pile].get();

    MachiPile& dest((move.dest == tablePiles.size()) ? makeNewPile() : *tablePiles[move.dest]);
    Card::Window* win(nullptr);
    if (move.transfers.size() > 1) {
        Card::PileWindows& wins(animateCards2(dest, first.destPos, *src, first.first, first.last));
        for (const auto& transfer : move.transfers | std::views::drop(1))
            wins.addWindow(transfer.destPos, *tablePiles[transfer.pile], transfer.first, transfer.last);
        win = &wins;
    }
    else if (first.first == first.last)
        win = &animateCard(dest, first.destPos, *src, first.first);
    else
        win = &animateCards(dest, first.destPos, *src, first.first, first.last);

    if (fromTable)
        win->sigAnimation.connect(bind(mem_fun(*this, &Machiavelli::unmarkAndEnd), &dest));
    else
        win->sigAnimation.connect(mem_fun(*this, &Machiavelli::endComputerMove));
}

//-----------------------------------------------------------------------------
/// Stores cards the computer player moves from one pile on the table to
/// another one (to inform the clients about them)
/// \param pile Pile to take the cards from
/// \param first Position of the first card to take
/// \param nr Number of cards to take
/// \param destPile Pile to move the cards to (might be a new one)
/// \param destPos Position in the destination (after moving the previous cards)
//-----------------------------------------------------------------------------
void Machiavelli::addTableMove([[maybe_unused]] unsigned int pile, [[maybe_unused]] unsigned int first,
                               [[maybe_unused]] unsigned int nr, [[maybe_unused]] unsigned int destPile,
                               [[maybe_unused]] unsigned int destPos) {
    TRACE8("Machiavelli::addTableMove(5x unsigned int) - " << nr << " cards from " << pile << '/' << first << " to " << destPile
                                                           << '/' << destPos);
    Check1(nr);
    Check1(pile < tablePiles.size());
    Check1((first + nr) <= tablePiles[pile]->size());
#ifdef WITH_NETWORK
    posPiles.emplace_back((nr << 16) + (pile << 8) + first, (destPile << 16) + destPos);
#endif
}

//----------------------------------------------------------------------------
/// Deals a card to the passed player; after the animation the game continues
/// with the next move (of the current player)
/// \param player Player to give a card to
/// \returns bool True, if a card is dealt; false if the staple is empty
//----------------------------------------------------------------------------
bool Machiavelli::dealCard(unsigned int player) {
    TRACE5("Machiavelli::dealCard(unsigned int) - " << player);
    if (staple.size() == 1) {
        Gtk::MessageDialog dlg(_("Taking last card! Solve the game (somehow) ..."), false, Gtk::MessageType::ERROR);
        dlg.set_title(_("Game over"));
        XGP::runModal(dlg);
    }

    if (staple.size()) {
        const unsigned int pos(player ? MachiavelliRules::dealPosition(hands[player].values(), staple.getTopCard())
                                      : hands[0].size());
        animateCard(hands[player], pos, staple, staple.size() - 1)
            .sigAnimation.connect(mem_fun(*this, &Machiavelli::endComputerMove));
        return true;
    }
    return false;
}

//----------------------------------------------------------------------------
/// Checks, if all the piles on the table are valid
/// \param obj Object collecting all errors
/// \param mark Flag if invalid piles should be marked
//----------------------------------------------------------------------------
void Machiavelli::checkPiles(YGP::StatusObject& obj, bool /*mark*/) const {
    for (auto i(tablePiles.cbegin()); i != tablePiles.cend(); ++i) {
        try {
            (*i)->checkIntegrity();
            (*i)->unmark();
        }
        catch (MachiPile::PileError& error) {
            (*i)->mark();
            TRACE8("Machiavelli::checkPiles() const - Pile " << (i - tablePiles.begin()) << ": " << error.what());
            Glib::ustring msg(_("Pile %1: %2\n"));
            msg.replace(msg.find("%1"), 2, YGP::ANumeric::toString(i - tablePiles.begin() + 1));
            msg.replace(msg.find("%2"), 2, error.what());
            obj.setMessage(YGP::StatusObject::ERROR, msg);
        }
    }
}

//----------------------------------------------------------------------------
/// Undoes the passed number of moves (starting from the last)
/// \param number Number of moves to undo
//----------------------------------------------------------------------------
void Machiavelli::undoMove(unsigned int number) {
    TRACE3("Machiavelli::undoMove(unsigned int) - Undo " << number);
    Check2(number);
    Check2(undo.size());

    if (number > undo.size())
        number = undo.size();
    TRACE8("Machiavelli::undoMove(unsigned int) - Undo (avail): " << number);

    disableHuman();

    while (number--) {
        undoValue move(undo.top());
        undo.pop();

        if (move.create)
            makeNewPile(move.srcPile);

        TRACE8("Machiavelli::undoMove(unsigned int) - Undo " << move.number << "; " << move.destPile << '/' << move.destPos
                                                             << "-> " << move.srcPile << '/' << move.srcPos);
        Card::IPile& dest((move.srcPile == 0xff) ? hands[currentPlayer()] : *tablePiles[move.srcPile]);
        Check3(move.srcPos <= dest.size());

        Check3(move.destPile < tablePiles.size());
        MachiPile& src(*tablePiles[move.destPile]);
        Check3(move.destPos < src.size());
        Check3(move.number);
        Check3(static_cast<unsigned int>(move.number + move.destPos) <= src.size());

        // Inform clients about cards to play
        if (getConnectionMgr().getMode() != YGP::ConnectionMgr::NONE) {
            std::ostringstream msg;
            msg << "Move=" << move.destPile << ";From=" << move.destPos << ";To=" << (move.destPos + move.number - 1)
                << ";Target=" << move.srcPile << ";At=" << move.srcPos;
            if (move.create)
                msg << ";Create=1";
            sendMove(msg.str());
        }

        do {
            dest.insert(src.remove(move.destPos), move.srcPos++);
        }
        while (--move.number);

        if (src.empty())
            removePile(move.destPile);
    }

    enableHuman();

    if (undo.empty()) {
        undoDlg.reset();

        undo1->set_enabled(false);
        undoAll->set_enabled(false);
    }
    else if (undoDlg) {
        YGP::StatusObject obj;
        checkPiles(obj, true);
        if (obj.getType() != YGP::StatusObject::UNDEFINED)
            obj.generalize(_("Can't end turn: The piles are not valid!"));
        else
            obj.setMessage(YGP::StatusObject::INFO, _("Could end turn: The piles are OK!"));

        undoDlg->update(obj);
    }
}

//----------------------------------------------------------------------------
/// Removes the passed pile from the table and internally
/// \param pile Offset of pile to remove
//----------------------------------------------------------------------------
void Machiavelli::removePile(unsigned int pile) {
    TRACE8("Machiavelli::removePile(unsigned int) - " << pile);
    Check1(pile < tablePiles.size());

    Check3(tablePiles[pile]->empty());

    piles.remove(*tablePiles[pile]);
    tablePiles.erase(tablePiles.begin() + pile); // Deletes the pile
}

//----------------------------------------------------------------------------
/// Ends the game
/// \param looser Number of player having lost the game
//----------------------------------------------------------------------------
void Machiavelli::endGame(unsigned int looser) {
    Check1(looser < NUM_PLAYERS);

    status.pop();
    Glib::ustring stat(_("%1 lost"));
    stat.replace(stat.find("%1"), 2, actPlayers[looser]->getName());
    status.push(stat);
    setGameStatus(STOPPED);
}

//----------------------------------------------------------------------------
/// Converts the pile-number to the actual pile; the target of the cards to
/// play is stored to be used in makeMove()
/// \param player Number of player
/// \param pile ID of the pile to return; the target ((pile << 16) + position)
///     of the cards to play (pile might be the number of piles, to create a new
///     one)
/// \returns Card::IPile* Pile corresponding to the passed number or nullptr
//----------------------------------------------------------------------------
Card::IPile* Machiavelli::getPileOfPlayer(unsigned int player, unsigned int pile) {
    const unsigned int nrPile(pile >> 16);
    const unsigned int pos(pile & 0xffff);
    if (!player || (player >= NUM_PLAYERS) || (nrPile > tablePiles.size()) ||
        (pos > ((nrPile == tablePiles.size()) ? 0 : tablePiles[nrPile]->size())))
        return nullptr;

    target = pile;
    return &hands[player];
}

//----------------------------------------------------------------------------
/// Handles the messages the server might send for the Machiavelli cardgame
/// \param player ID of the player sending the message
/// \param message Message received from the server
/// \returns bool True, if message has been processed completey
//----------------------------------------------------------------------------
bool Machiavelli::handleMessage([[maybe_unused]] unsigned int player, const std::string& message) {
    TRACE1("Machiavelli::handleMessage(unsigned int player, const std::string&) - " << message << " (" << player << ')');

#ifdef WITH_NETWORK
    const std::string_view cmd(Card::commandOf(message));
    const bool server(getConnectionMgr().getMode() == YGP::ConnectionMgr::SERVER);

    if (gameStatus() == PLAYING) {
        // The server accepts moves only from the player in turn
        if (server && ((cmd == "Play") || (cmd == "Reorder") || (cmd == "Move") || (cmd == "EndTurn")) &&
            (player != currentPlayer()))
            throw YGP::ParseError(N_("Move of a player not in turn!"));

        if (cmd == "EndTurn") {
            // EndTurn
            if (server)
                broadcastMessage(message);

            unsigned int nextPlayer(findNextPlayer(currentPlayer()));
            setNextPlayer(nextPlayer);
            if (MachiavelliRules::isGameOver(handSizes(), nextPlayer)) {
                endGame(nextPlayer);
                return true;
            }

            displayTurn(nextPlayer);

#    if CHECK > 2
            YGP::StatusObject obj;
            checkPiles(obj);
            if (obj.getType() != YGP::StatusObject::UNDEFINED) {
                TRACE1("Machiavelli::handleMessage(unsigned int, const std::string&) - Invalid piles!\n" << obj.getMessage());
                Check(obj.getType() == YGP::StatusObject::UNDEFINED);
            }
#    endif

            // Keep the message token til the card is dealt (and the next move starts)
            if (dealCard(nextPlayer)) {
                keepMessageLock();
                return false;
            }
            return true;
        }
        else if (cmd == "Reorder")
            return reorderRemote(message);
        else if (cmd == "Move") {
            moveRemote(message);
            return true;
        }
    }
#endif

    bool rc(Game::handleMessage(player, message));
#ifdef WITH_NETWORK
    // The client starts playing, after receiving the startplayer
    if ((cmd == "ActPlayer") && (getConnectionMgr().getMode() == YGP::ConnectionMgr::CLIENT)) {
        TRACE1("Machiavelli::handleMessage(unsigned int player, const std::string&) - Next player: " << currentPlayer());

        // Keep the message token til the card is dealt (and the first move starts)
        startPlayer = currentPlayer();
        if (setStartPlayer()) {
            keepMessageLock();
            rc = false;
        }
    }
#endif
    return rc;
}

#ifdef WITH_NETWORK
//----------------------------------------------------------------------------
/// Plays the cards a remote player played from his hand (flipped by
/// flipCards2Play and positioned in pos1Play-pos2Play) to the target
/// (target-member: (pile << 16) + position); the game continues after the
/// animation
/// \param player Player playing the cards
//----------------------------------------------------------------------------
void Machiavelli::playRemoteCards(unsigned int player) {
    TRACE5("Machiavelli::playRemoteCards(unsigned int) - Player " << player << ": " << pos1Play << '-' << pos2Play << " to "
                                                                  << std::hex << target << std::dec);
    Check1(player < NUM_PLAYERS);
    Check1(pos1Play <= pos2Play);
    Check1(pos2Play < hands[player].size());

    const unsigned int start(pos1Play), end(pos2Play);
    const unsigned int nrPile(target >> 16), pos(target & 0xffff);
    pos1Play = pos2Play = target = -1U;

    if (gameStatus() == TOSTOP) {
        stop();
        return;
    }
    Check1(gameStatus() == PLAYING);

    Check3(nrPile <= tablePiles.size());
    MachiPile& pile((nrPile == tablePiles.size()) ? makeNewPile() : *tablePiles[nrPile]);
    Check3(pos <= pile.size());
    animateCards(pile, pos, hands[player], start, end).sigAnimation.connect([this] { makeNextMoves(); });
}

//----------------------------------------------------------------------------
/// Handles a received Reorder-message: Moves cards from one pile on the table
/// to another one:
///   <pre>  <b>Reorder</b>=<tt>(number << 16) + (source pile << 8) + first card</tt>;<b>Target</b>=<tt>(pile << 16) +
///   position</tt></pre>
/// (a target pile equal to the number of piles creates a new pile; a source
/// pile getting empty is removed). A trailing <tt>Now=1</tt> is accepted and
/// ignored.
/// \param message Received message
/// \returns bool True, if message has been processed completely; else it is
///     finished after the animation
/// \throw YGP::ParseError In case of an invalid message
//----------------------------------------------------------------------------
bool Machiavelli::reorderRemote(const std::string& message) {
    TRACE5("Machiavelli::reorderRemote(const std::string&) - " << message);

    const auto fields(Card::splitMessage(message));
    const auto values(Card::words(fields.at(0).value));
    unsigned long source(0), dest(-1UL);
    if ((values.size() != 1) || stringToNumber(source, values[0].c_str()) || (source > 0xffffffffUL))
        throw YGP::ParseError(N_("Invalid cards!"));
    for (const auto& field : fields | std::views::drop(1))
        if (field.key == "Target") {
            if (stringToNumber(dest, field.value.c_str()))
                throw YGP::ParseError(N_("Invalid destination pile!"));
        }
        else if (field.key != "Now")
            throw YGP::ParseError(N_("Invalid message!"));

    const unsigned int nrSrc((source >> 8) & 0xff);
    const unsigned int nr(source >> 16);
    const unsigned int posSrc(source & 0xff);
    const unsigned int nrDest(dest >> 16);
    const unsigned int posDest(dest & 0xffff);
    TRACE8("Machiavelli::reorderRemote(const std::string&) - Move " << nr << " cards from " << nrSrc << '/' << posSrc << " to "
                                                                    << nrDest << '/' << posDest);

    if (nrSrc >= tablePiles.size())
        throw YGP::ParseError(N_("Invalid source pile!"));
    if (!nr || ((posSrc + nr) > tablePiles[nrSrc]->size()))
        throw YGP::ParseError(N_("Invalid cards!"));
    if ((dest > 0xffffffffUL) || (nrDest > tablePiles.size()) || (nrDest == nrSrc) ||
        (posDest > ((nrDest == tablePiles.size()) ? 0 : tablePiles[nrDest]->size())))
        throw YGP::ParseError(N_("Invalid destination pile!"));

    // Inform the (other) clients
    if (getConnectionMgr().getMode() == YGP::ConnectionMgr::SERVER)
        broadcastMessage(message);

    MachiPile& src(*tablePiles[nrSrc]);
    MachiPile& pile((nrDest == tablePiles.size()) ? makeNewPile() : *tablePiles[nrDest]);
    animateCards(pile, posDest, src, posSrc, posSrc + nr - 1)
        .sigAnimation.connect(bind(mem_fun(*this, &Machiavelli::endRemoteReorder), &src));

    // Keep the message token til the animation has finished
    keepMessageLock();
    return false;
}

//----------------------------------------------------------------------------
/// Callback after animating the cards of a received Reorder-message: Removes
/// the source pile, if it got empty, and continues with the game
/// \param src Pile the cards have been taken from
//----------------------------------------------------------------------------
void Machiavelli::endRemoteReorder(MachiPile* src) {
    TRACE8("Machiavelli::endRemoteReorder(MachiPile*)");
    Check1(src);

    if (src->empty()) {
        auto pile(std::ranges::find_if(tablePiles, [src](const auto& p) { return p.get() == src; }));
        Check3(pile != tablePiles.end());
        removePile(pile - tablePiles.begin());
    }
    makeNextMoves();
}

//----------------------------------------------------------------------------
/// Handles a received Move-message (sent when undoing moves): Moves cards from
/// a pile on the table back to another pile or the hand of the current player:
///   <pre>  <b>Move</b>=<tt>source pile</tt>;<b>From</b>=<tt>first card</tt>;<b>To</b>=<tt>last card</tt>;
///   <b>Target</b>=<tt>destination pile (255: hand)</tt>;<b>At</b>=<tt>position</tt>[;<b>Create</b>=1]</pre>
/// With Create=1 the destination pile is created (at the passed position in
/// the piles) first; a source pile getting empty is removed.
/// \param message Received message
/// \throw YGP::ParseError In case of an invalid message
//----------------------------------------------------------------------------
void Machiavelli::moveRemote(const std::string& message) {
    TRACE5("Machiavelli::moveRemote(const std::string&) - " << message);

    YGP::AttributeParse ap;
    unsigned int card1(-1U), card2(-1U), dest(-1U), src(-1U), destPos(0), create(0);
    ATTRIBUTE(ap, unsigned int, src, "Move");
    ATTRIBUTE(ap, unsigned int, card1, "From");
    ATTRIBUTE(ap, unsigned int, card2, "To");
    ATTRIBUTE(ap, unsigned int, dest, "Target");
    ATTRIBUTE(ap, unsigned int, destPos, "At");
    ATTRIBUTE(ap, unsigned int, create, "Create");
    ap.assignValues(message);

    if (create ? (dest > tablePiles.size()) : ((dest >= tablePiles.size()) && (dest != 255)))
        throw YGP::ParseError(N_("Invalid destination pile!"));
    if (create)
        makeNewPile(dest);

    Card::IPile& pile((dest == 255) ? static_cast<Card::IPile&>(hands[currentPlayer()]) : *tablePiles[dest]);
    MachiPile* srcPile(nullptr);
    try {
        if (destPos > pile.size())
            throw YGP::ParseError(N_("Invalid position in destination pile!"));

        if ((src >= tablePiles.size()) || (src == dest))
            throw YGP::ParseError(N_("Invalid source pile!"));
        srcPile = tablePiles[src].get();
        if ((card2 < card1) || (card2 >= srcPile->size()))
            throw YGP::ParseError(N_("Invalid cards!"));
    }
    catch (...) {
        if (create)
            removePile(dest);
        throw;
    }

    // Inform the (other) clients
    if (getConnectionMgr().getMode() == YGP::ConnectionMgr::SERVER)
        broadcastMessage(message);

    Check3(srcPile);
    do
        pile.insert(srcPile->remove(card1), destPos++);
    while (card1 < card2--);

    if (srcPile->empty())
        removePile(src);
}
#endif

//----------------------------------------------------------------------------
/// Returns the actual target, where flipCard2Play should position the cards to
/// \returns unsigned int ID of the target
//----------------------------------------------------------------------------
unsigned int Machiavelli::getActTarget() const {
    Check3((target >> 16) <= tablePiles.size());
    return target;
}

//-----------------------------------------------------------------------------
/// Adds machiavelli-specific menus
/// \param menu Menu (placeholder for the game's own entries) to add to
/// \param actions Action group the game's actions get registered into
/// \remarks GTK4 moved keyboard accelerators to the Gtk::Application level
///     (Gtk::Application::set_accel_for_action); registering them for these
///     actions ("game.MachiUndo" etc.) is left to the caller.
//-----------------------------------------------------------------------------
void Machiavelli::addMenus(const Glib::RefPtr<Gio::Menu>& menu, const Glib::RefPtr<Gio::SimpleActionGroup>& actions) {
    Check1(menu);
    Check1(actions);

    Glib::RefPtr<Gio::Menu> menuMachi(Gio::Menu::create());

    Glib::RefPtr<Gio::Menu> secUndo(Gio::Menu::create());
    undo1 = actions->add_action("MachiUndo", bind(mem_fun(*this, &Machiavelli::undoMove), 1));
    secUndo->append(_("_Undo"), "game.MachiUndo");
    undoAll = actions->add_action("MachiUndoAll", bind(mem_fun(*this, &Machiavelli::undoMove), -1U));
    secUndo->append(_("Undo _all"), "game.MachiUndoAll");
    menuMachi->append_section(secUndo);

    Glib::RefPtr<Gio::Menu> secSort(Gio::Menu::create());
    actions->add_action("MachiSort", mem_fun(*this, &Machiavelli::sortHand));
    secSort->append(_("_Sort cards (by number)"), "game.MachiSort");
    actions->add_action("MachiSortCol", mem_fun(*this, &Machiavelli::sortHandByColour));
    secSort->append(_("Sort cards (by _colour)"), "game.MachiSortCol");
    menuMachi->append_section(secSort);

    Glib::RefPtr<Gio::Menu> secEnd(Gio::Menu::create());
    nxtTurn = actions->add_action("MachiEndTurn", mem_fun(*this, (&Machiavelli::endTurn)));
    secEnd->append(_("_End turn"), "game.MachiEndTurn");
    menuMachi->append_section(secEnd);

    menu->append_submenu(_("_Machiavelli"), menuMachi);

    undo1->set_enabled(false);
    undoAll->set_enabled(false);
}

//-----------------------------------------------------------------------------
/// Removes the machiavelli-specific menus
/// \param menu Menu to remove the game's entries from
/// \param actions Action group to remove the game's actions from
//-----------------------------------------------------------------------------
void Machiavelli::removeMenus(const Glib::RefPtr<Gio::Menu>& menu, const Glib::RefPtr<Gio::SimpleActionGroup>& actions) {
    Check1(menu);
    Check1(actions);

    menu->remove_all();
    actions->remove_action("MachiUndo");
    actions->remove_action("MachiUndoAll");
    actions->remove_action("MachiSort");
    actions->remove_action("MachiSortCol");
    actions->remove_action("MachiEndTurn");
}

//-----------------------------------------------------------------------------
/// Sorts the cards in the hand by number
//-----------------------------------------------------------------------------
void Machiavelli::sortHand() {
    bool enabled(activeCards.size());
    if (enabled)
        disableHuman();
    hands[0].sort(Card::IPile::compCardsByNr);
    if (enabled)
        enableHuman();
}

//-----------------------------------------------------------------------------
/// Sorts the cards in the hand by colour
//-----------------------------------------------------------------------------
void Machiavelli::sortHandByColour() {
    bool enabled(activeCards.size());
    if (enabled)
        disableHuman();
    hands[0].sort(Card::IPile::compCards);
    if (enabled)
        enableHuman();
}

//-----------------------------------------------------------------------------
/// Callback after removing the undo-dialog. Sets the undoDlg variable to NULL
//-----------------------------------------------------------------------------
void Machiavelli::removeUndoDlg(int) {
    TRACE1("Machiavelli::removeUndoDlg(int)");
    undoDlg.reset();
}

//-----------------------------------------------------------------------------
/// Actions to take when the cards are resized
/// \pre The cardsize must be set in CardImages::WIDTH/HEIGHT
//-----------------------------------------------------------------------------
void Machiavelli::resizeCards() {
    staple.set_size_request(Card::Images::WIDTH, Card::Images::HEIGHT);
    for (auto& hand : hands)
        hand.set_size_request(-1, Card::Images::HEIGHT);
}

//-----------------------------------------------------------------------------
/// Callback after animating marked cards; all cards in the passed pile are
/// unmarked
/// \param pile Pile to unmark
//-----------------------------------------------------------------------------
void Machiavelli::unmarkAndEnd(MachiPile* pile) {
    Check1(pile);
    pile->unmark();
    endComputerMove();
}

//-----------------------------------------------------------------------------
/// Finishes the turn and starts the next one
//-----------------------------------------------------------------------------
void Machiavelli::endComputerMove() {
    TRACE9("Machiavelli::endComputerMove()");
#if CHECK > 0
    YGP::StatusObject obj;
    checkPiles(obj);
    Check(obj.getType() == YGP::StatusObject::UNDEFINED);
#endif

    makeNextMoves();
}
