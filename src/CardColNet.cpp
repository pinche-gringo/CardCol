// PROJECT     : Cardgames
// SUBSYSTEM   : Network-code
// REFERENCES  :
// TODO        :
// BUGS        :
// AUTHOR      : Markus Schwab
// CREATED     : 7.7.2005
// COPYRIGHT   : Copyright (C) 2005 - 2009, 2026

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

#ifdef WITH_NETWORK

#    include <algorithm>
#    include <cstdlib>
#    include <memory>
#    include <ranges>
#    include <sstream>
#    include <string>
#    include <string_view>

#    include <boost/asio/post.hpp>

#    include <glibmm/main.h>
#    include <gtkmm/messagedialog.h>

#    include <YGP/AssParse.h>
#    include <YGP/AttrParse.h>
#    include <YGP/Check.h>
#    include <YGP/Trace.h>

#    include <card/ComputerPlayer.h>
#    include <card/Game.h>
#    include <card/Message.h>
#    include <card/Player.h>
#    include <card/PlayerConnDlg.h>

#    include "ChatDlg.h"
#    include "GameTypes.h"

#    ifdef WITH_HEARTS
#        include "Hearts.h"
#    endif
#    ifdef WITH_BURACO
#        include "Buraco.h"
#        include "BuracoCards.h"
#    endif
#    ifdef WITH_ROVHULT
#        include "CardValue.h"
#        include "Rovhult.h"
#    endif
#    ifdef WITH_SGTMAYOR
#        include "SgtMayor.h"
#    endif

#    include "CardCol.h"
#    include "CardColAppl.h"

//-----------------------------------------------------------------------------
/// Stopps the client waiting for the server to start the game.
/// \returns true; if the client should wait for the server, or program runs as
///          server
//-----------------------------------------------------------------------------
bool CardgameCollection::stopClientWaiting() {
    if (cmgr.getMode() == YGP::ConnectionMgr::CLIENT) {
        Gtk::MessageDialog dlg(_("Stop waiting for the server to start the game and start a local one?"), false,
                               Gtk::MessageType::QUESTION, Gtk::ButtonsType::YES_NO);
        dlg.set_title(PACKAGE);
        if (XGP::runModal(dlg) == static_cast<int>(Gtk::ResponseType::YES)) {
            Check3(aCommPartners.size() == 1);
            stopCommunication();
            cmgr.changeMode(YGP::ConnectionMgr::NONE);

            apMenus[CHAT]->set_enabled(false);
        }
        else
            return false;
    }
    return true;
}

//-----------------------------------------------------------------------------
/// Opens a dialog allowing to connect to other computers
//-----------------------------------------------------------------------------
void CardgameCollection::connect() {
    // The dialog changes the connections, so they must not be used meanwhile
    stopCommunication();

    playerPos = Card::PlayerConnectDlg::perform(aPlayer, CardgameAppl::PORT, cmgr);
    TRACE1("CardgameCollection::connect() - Mode: " << cmgr.getMode() << "; Pos: " << playerPos);
    if (cmgr.getMode() != YGP::ConnectionMgr::NONE)
        initCommunication();
    else
        apMenus[CHAT]->set_enabled(false);
}

//-----------------------------------------------------------------------------
/// Initializes the communication
//-----------------------------------------------------------------------------
void CardgameCollection::initCommunication() {
    Check2(cmgr.getMode() != YGP::ConnectionMgr::NONE);
    Check2(aCommPartners.empty());
    Check2(!commThread.joinable());

    // An existing game must know the position of the player and the new players
    if (game) {
        game->setPlayerPosition(playerPos);
        game->changeNames(aPlayer);
    }

    if (cmgr.getMode() == YGP::ConnectionMgr::CLIENT) {
        status.pop();
        status.push(_("Waiting for the server to start the game ..."));
        Check3(cmgr.getServer());
        Check3(playerPos);
        aCommPartners.push_back(std::make_unique<CommPartner>(cmgr.getServer(), aPlayer.size() - playerPos, std::string()));
    }
    else {
        broadcastNames();

        // Client n sits at position n + 1 (disconnected clients are NULL)
        for (const auto& [i, client] : std::views::enumerate(cmgr.getClients()))
            if (client)
                aCommPartners.push_back(std::make_unique<CommPartner>(client.get(), i + 1, std::string()));
    }

    if (!aCommPartners.empty())
        commThread = std::jthread([this](std::stop_token stop) { waitForMessages(stop); });

    apMenus[CHAT]->set_enabled(true);
}

//----------------------------------------------------------------------------
/// Waits (in the communication thread) for messages from all partners
/// \param stop Token signalling the thread to stop
//----------------------------------------------------------------------------
void CardgameCollection::waitForMessages(std::stop_token stop) {
    TRACE1("CardgameCollection::waitForMessages(std::stop_token)");
    boost::asio::io_context& ctx(cmgr.getContext());
    ctx.restart();
    for (auto& partner : aCommPartners)
        receiveNext(*partner, stop);
    ctx.run();

    TRACE7("CardgameCollection::waitForMessages(std::stop_token) - Finished");
}

//----------------------------------------------------------------------------
/// Reads the data available from the passed partner and passes the complete
/// messages to the GUI (one at a time); then waits for more data. Errors
/// (including too long messages) cause the partner to be removed.
/// \param partner Partner to receive the message from
/// \param stop Token signalling the communication thread to stop
/// \remarks The data is read without blocking, as the GUI thread might send
///     data over the same connection meanwhile (and the TLS stream must not be
///     used concurrently)
//----------------------------------------------------------------------------
void CardgameCollection::receiveNext(CommPartner& partner, std::stop_token stop) {
    if (stop.stop_requested())
        return;

    boost::system::error_code error;
    const bool drained(partner.conn->readAvailable(partner.buffer, Card::MAX_MESSAGE + 1, error));
    for (std::size_t end; (end = partner.buffer.find(Card::MESSAGE_END)) != std::string::npos;) {
        std::string message(partner.buffer, 0, end);
        partner.buffer.erase(0, end + 1);
        TRACE7("CardgameCollection::receiveNext(CommPartner&, std::stop_token) - `" << message << '\'');
        if (message.size()) {
            // Wait til the last message has been processed; the GUI returns the token after processing this one
            TRACE9("CardgameCollection::receiveNext(CommPartner&, std::stop_token) - Lock (thread)");
            if (!mxThreadCmd.lock(stop))
                return;

            TRACE9("CardgameCollection::receiveNext(CommPartner&, std::stop_token) - Perform cmd " << message);
            Glib::signal_idle().connect([this, player = partner.player, message] { return handleMessage(player, message); });
        }
    }
    if (!error && (partner.buffer.size() > Card::MAX_MESSAGE))
        error = make_error_code(YGP::ConnectionError::tooMuchData);

    if (error)
        Glib::signal_idle().connect_once([this, conn = partner.conn, error] { partnerLost(conn, error); });
    else if (!drained)
        // There is more data than fitting into the buffer; process it after the other partners
        boost::asio::post(cmgr.getContext(), [this, &partner, stop] { receiveNext(partner, stop); });
    else
        partner.conn->getSocket().async_wait(boost::asio::ip::tcp::socket::wait_read,
                                             [this, &partner, stop](const boost::system::error_code& error) {
                                                 if ((error == boost::asio::error::operation_aborted) || stop.stop_requested())
                                                     return;

                                                 if (error)
                                                     Glib::signal_idle().connect_once(
                                                         [this, conn = partner.conn, error] { partnerLost(conn, error); });
                                                 else
                                                     receiveNext(partner, stop);
                                             });
}

//----------------------------------------------------------------------------
/// Handles (in the GUI thread) the lost connection to a partner
/// \param conn Connection to the partner
/// \param error Error, which caused the loss of the connection
//----------------------------------------------------------------------------
void CardgameCollection::partnerLost(const YGP::Connection* conn, boost::system::error_code error) {
    TRACE7("CardgameCollection::partnerLost(const YGP::Connection*, error_code) - " << error.message());

    // Ignore partners which have been removed meanwhile
    auto partner(std::ranges::find(aCommPartners, conn, [](const auto& p) { return p->conn; }));
    if (partner == aCommPartners.end())
        return;

    const unsigned int iPlayer((*partner)->player);
    aCommPartners.erase(partner);

    std::string msg;
    if (error == boost::asio::error::eof) {
        msg = _("Lost connection to %1!");
        Check3(iPlayer < aPlayer.size());
        msg.replace(msg.find("%1"), 2, aPlayer[iPlayer]->getName());
    }
    else {
        msg = _("Error receiving data!\n\nReason: %1");
        msg.replace(msg.find("%1"), 2, error.message());
    }

    // The server must change the disconnected remote to a computer controled player
    const bool server(cmgr.getMode() == YGP::ConnectionMgr::SERVER);
    if (server) {
        TRACE7("CardgameCollection::partnerLost(const YGP::Connection*, error_code) - Removing " << aPlayer[iPlayer]->getName());
        Card::Player* oldPlayer(aPlayer[iPlayer]);
        Check3(oldPlayer);
        aPlayer[iPlayer] = new Card::ComputerPlayer(oldPlayer->getName());
        delete oldPlayer;
    }
    // while the client changes all remote to computer controled player
    else {
        Check3(!aPlayer.empty());
        for (auto& player : aPlayer | std::views::drop(1)) {
            Card::Player* oldPlayer(player);
            Check3(oldPlayer);
            player = new Card::ComputerPlayer(oldPlayer->getName());
            delete oldPlayer;
        }
    }
    cmgr.disconnect(conn);

    if (aCommPartners.empty()) {
        stopCommunication();
        apMenus[CHAT]->set_enabled(false);
        cmgr.changeMode(YGP::ConnectionMgr::NONE);
    }

    // Let the computer players continue for the replaced ones
    if (game) {
        if (server)
            game->playerReplaced(iPlayer);
        else
            for (unsigned int i(1); i < aPlayer.size(); ++i)
                game->playerReplaced(i);
    }

    showMessage(msg);
}

//----------------------------------------------------------------------------
/// Handles received global messages: Those are:
///   - Error messages (to display error messages):
///      <pre>  <b>Error</b>=<tt>Number</tt>;<b>Msg</b>="<tt>message</tt>"</pre>
///   - Game messages (to restart a game):
///      <pre>  <b>Game</b>=<tt>Name</tt>;
///   - Chat messages (to communicate with other):
///      <pre>  <b>Msg</b>=<tt>message</tt>;<b>Sender</b>=<tt>who</tt>;
///   - Name messages (to change the name of the players):
///      <pre>  <b>ChgNames</b>=<tt>lines-with-names</tt>;
/// \param player Player sending the message
/// \param msg Received message to handle
/// \returns int True: Message was a supported message and has been processed;
///     -1 if Message was handled, but not fully processed yet; else false
//----------------------------------------------------------------------------
int CardgameCollection::handleGlobalMessage(unsigned int player, const std::string& msg) {
    TRACE5("CardgameCollection::handleGlobalMessage(unsigned int, char*) - " << msg);

    // Message has the form cmd=param;remaining
    std::string_view remaining(msg);
    auto nextNode([&remaining](char separator) {
        const auto pos(remaining.find(separator));
        std::string node(remaining.substr(0, pos));
        remaining.remove_prefix((pos == std::string_view::npos) ? remaining.size() : (pos + 1));
        return node;
    });
    std::string cmd(nextNode('='));
    std::string param(nextNode(';'));
    TRACE3("CardgameCollection::handleGlobalMessage(unsigned int, char*) - " << cmd);

    if (cmd == "Game") {
        // Only the server decides which game is played
        if (cmgr.getMode() != YGP::ConnectionMgr::CLIENT) {
            cmd = _("Unexpected command `%1'");
            cmd.replace(cmd.find("%1"), 2, "Game");
            throw YGP::ParseError(cmd);
        }

        int type(CardgameAppl::convertToGameType(param.c_str()));
        if (type == GameTypes::NONE) {
            cmd = _("Invalid game type: `%1'");
            cmd.replace(cmd.find("%1"), 2, param);
            throw YGP::ParseError(cmd);
        }

        changeGame(type);
        if (game) {
            restart = true;
            if (restartGame())
                mxThreadCmd.unlock();
        }
        else {
            startGame();
            mxThreadCmd.unlock();
        }
        Card::sendMessage(*cmgr.getServer(), "Error=0");
        return -1;
    }
    else if (cmd == "Msg") {
        cmd.clear();
        param.clear();

        YGP::AttributeParse ap;
        ATTRIBUTE(ap, std::string, cmd, "Msg");
        ATTRIBUTE(ap, std::string, param, "Sender");

        try {
            ap.assignValues(msg);
        }
        catch (YGP::ParseError&) {
            cmd = _("Invalid message received!");
        }
        param = Card::sanitizeName(param);
        if (cmd.size() && param.size()) {
            showChatDlg();
            dlgChat->addMessage(param, cmd);

            if (cmgr.getMode() == YGP::ConnectionMgr::SERVER)
                broadcastMsg(msg, player - 1);
            return true;
        }
    }
    else if (cmd == "ChgNames") {
        if (cmgr.getMode() == YGP::ConnectionMgr::SERVER) {
            const Glib::ustring name(Card::sanitizeName(param));
            if (name.size())
                aPlayer[player]->setName(name);
            broadcastNames();
        }
        else {
            unsigned int pos(-playerPos);
            for (auto line : param | std::views::split('\n')) {
                if (line.empty())
                    continue;

                Glib::ustring name(Card::sanitizeName(std::string(std::string_view{line})));
                TRACE9("CardgameCollection::handleGlobalMessage(unsigned int, const std::string&) - Setting " << name);
                aPlayer[pos++ % aPlayer.size()]->setName(name);
            }
        }
        return true;
    }
    else if (cmd == "Settings") {
        // Parse in an own scope, so the Rovhult settings are written back before sending them
        {
#    ifdef WITH_ROVHULT
            EnumAsUInt nuke(Rovhult::cardNuke), reverse(Rovhult::cardReverse), skip(Rovhult::cardSkip);
#    endif
            YGP::AttributeParse ap;
#    if !defined(WITH_BURACO) || !defined(WITH_HEARTS) || !defined(WITH_ROVHULT) || !defined(WITH_SGTMAYOR)
            unsigned int temp;
#    endif
#    ifdef WITH_BURACO
            MEATTRIBUTE(ap, BuracoCards::get(), Buraco::CARDS2DEAL, "BCards");
            ATTRIBUTE(ap, unsigned int, Buraco::ENDPOINTS, "BPoints");
#    else
            ATTRIBUTE(ap, unsigned int, temp, "BCards");
            ATTRIBUTE(ap, unsigned int, temp, "BPoints");
#    endif
#    ifdef WITH_HEARTS
            ATTRIBUTE(ap, unsigned int, Hearts::ENDPOINTS, "HPoints");
#    else
            ATTRIBUTE(ap, unsigned int, temp, "HPoints");
#    endif
#    ifdef WITH_ROVHULT
            MEATTRIBUTE(ap, CardValue::get(), nuke, "RCardNuke");
            MEATTRIBUTE(ap, CardValue::get(), reverse, "RCardReverse");
            MEATTRIBUTE(ap, CardValue::get(), skip, "RCardSkip");
#    else
            ATTRIBUTE(ap, unsigned int, temp, "RCardNuke");
            ATTRIBUTE(ap, unsigned int, temp, "RCardReverse");
            ATTRIBUTE(ap, unsigned int, temp, "RCardSkip");
#    endif
#    ifdef WITH_SGTMAYOR
            ATTRIBUTE(ap, unsigned int, SgtMayor::ENDTRICKS, "STricks");
#    else
            ATTRIBUTE(ap, unsigned int, temp, "STricks");
#    endif

            try {
                TRACE1("Param: " << msg.substr(msg.find('=') + 1));
                ap.assignValues(msg.substr(msg.find('=') + 1));
            }
            catch (YGP::ParseError& e) {
                cmd = _("Invalid settings: `%1'!\n\n%2");
                cmd.replace(cmd.find("%1"), 2, param);
                cmd.replace(cmd.find("%2"), 2, e.what());
                throw YGP::ParseError(cmd);
            }
        }

        if (cmgr.getMode() == YGP::ConnectionMgr::SERVER)
            sendSettings();
        return true;
    }
    else if (cmd == "Error") {
        if (param != "0") {
            cmd.clear();
            YGP::AttributeParse ap;
            ATTRIBUTE(ap, std::string, cmd, "Msg");

            try {
                ap.assignValues(std::string(remaining));
                if (cmd.empty())
                    cmd = static_cast<std::string>(_("Unspecified error"));
            }
            catch (YGP::ParseError&) {
                cmd = _("Invalid message received!");
            }

            Glib::ustring err(_("%1 send error %2\n\n%3"));
            err.replace(err.find("%1"), 2,
                        (cmgr.getMode() == YGP::ConnectionMgr::CLIENT ? _("The server") : aPlayer[player]->getName()));
            err.replace(err.find("%2"), 2, param);
            err.replace(err.find("%3"), 2, _(cmd.c_str()));
            showMessage(err);
        }
        return true;
    }
    return false;
}

//----------------------------------------------------------------------------
/// Handles received messages
/// \param player Player sending the message (relative to server)
/// \param msg Received message to handle
/// \returns bool False
//----------------------------------------------------------------------------
bool CardgameCollection::handleMessage(unsigned int player, const std::string& msg) {
    TRACE5("CardgameCollection::handleMessage(unsigned int, char*) - " << msg);

    bool unlock(true);
    try {
        int rc(handleGlobalMessage(player, msg));
        if ((rc == -1) || (!rc && (game && !game->ignoreMessage(msg) && !game->handleMessage(player, msg))))
            unlock = false;
    }
    catch (std::exception& error) {
        TRACE9("CardgameCollection::handleMessage(unsigned int, const std::string)"
               " - Error "
               << error.what());
        broadcastMsg("Error=99;" + YGP::AssignmentParse::makeAssignment("Msg", error.what()));

        Glib::ustring message(_("Error processing command `%1'!\n\n%2"));
        message.replace(message.find("%1"), 2, msg);
        message.replace(message.find("%2"), 2, _(error.what()));
        showMessage(message);
    }

    TRACE9("CardgameCollection::handleMessages(unsigned int, char*) - Unlock (main): " << unlock);
    if (unlock)
        mxThreadCmd.unlock();

    return false;
}

//-----------------------------------------------------------------------------
/// Opens a dialog to chat with the connected persons
//-----------------------------------------------------------------------------
void CardgameCollection::showChatDlg() {
    if (dlgChat)
        dlgChat->present();
    else {
        dlgChat = ChatDlg::create(*this);
        dlgChat->signal_response().connect(mem_fun(*this, &CardgameCollection::closeChat));
        dlgChat->signalSend.connect(mem_fun(*this, &CardgameCollection::sendMessage));
    }
}

//-----------------------------------------------------------------------------
/// Sends the passes message to the partners
/// \param msg Message to send
//-----------------------------------------------------------------------------
void CardgameCollection::sendMessage(const Glib::ustring& msg) {
    TRACE9("CardgameCollection::sendMessage(const Glib::ustring&) - " << msg);
    Check2(dlgChat);

    dlgChat->addMessage(aPlayer[0]->getName(), msg);

    broadcastMsg(YGP::AssignmentParse::makeAssignment("Msg", msg.raw()) +
                 YGP::AssignmentParse::makeAssignment("Sender", Card::sanitizeName(aPlayer[0]->getName()).raw()));
}

//-----------------------------------------------------------------------------
/// Broadcast a message to all partners
/// \param msg Message to broadcast
/// \param exclude Client to exclude (Zero-based; -1: None)
//-----------------------------------------------------------------------------
void CardgameCollection::broadcastMsg(const std::string& msg, unsigned int exclude) {
    TRACE9("CardgameCollection::broadcastMsg(const Glib::ustring&, unsigned int) - " << msg << "; Exclude: " << exclude);
    try {
        if (cmgr.getMode() == YGP::ConnectionMgr::SERVER) {
            for (const auto& [i, client] : std::views::enumerate(cmgr.getClients()))
                if (client && (exclude != static_cast<unsigned int>(i)))
                    Card::sendMessage(*client, msg);
        }
        else {
            Check3(cmgr.getServer());
            Card::sendMessage(*cmgr.getServer(), msg);
        }
    }
    catch (std::exception&) {
    }
}

//-----------------------------------------------------------------------------
/// Starts the connections, if passed as options. The password is taken from
/// the environment variable CARDCOL_PASSWORD (not from an option, as those
/// are visible to other users of the computer).
/// \param options Options of program; containing host/port
//-----------------------------------------------------------------------------
void CardgameCollection::autoConnect(const Options& options) {
    if (options.port.size()) {
        TRACE9("CardgameCollection::autoConnect(const Options&) - Connect: " << options.target << '-' << options.port);
        const char* envPassword(std::getenv("CARDCOL_PASSWORD"));
        const Glib::ustring password(envPassword ? envPassword : "");
        if (options.target.size())
            playerPos = Card::PlayerConnectDlg::perform(aPlayer, cmgr, options.target, options.port, password);
        else
            playerPos = Card::PlayerConnectDlg::perform(aPlayer, cmgr, options.port, password);

        TRACE1("CardgameCollection::autoConnect(const Options&) - " << cmgr.getMode() << "; Pos: " << playerPos);
        if (cmgr.getMode() != YGP::ConnectionMgr::NONE)
            initCommunication();
    }
}

//-----------------------------------------------------------------------------
/// Resets the dlgChat-member
//-----------------------------------------------------------------------------
void CardgameCollection::closeChat(int) {
    TRACE9("CardgameCollection::closeChat(int)");
    dlgChat = nullptr;
}

//-----------------------------------------------------------------------------
/// Stops the communication with the partners (without closing the connections)
//-----------------------------------------------------------------------------
void CardgameCollection::stopCommunication() {
    TRACE9("CardgameCollection::stopCommunication()");
    if (commThread.joinable()) {
        commThread.request_stop(); // Aborts waiting for the message token
        cmgr.getContext().stop();
        commThread.join();
    }

    // Abort the outstanding receive-operations and let them finish, while their partners still exist
    if (!aCommPartners.empty()) {
        for (auto& partner : aCommPartners) {
            boost::system::error_code ignore;
            partner->conn->getSocket().cancel(ignore);
        }
        cmgr.getContext().restart();
        cmgr.getContext().poll();
        aCommPartners.clear();
    }
}

//-----------------------------------------------------------------------------
/// Informs the partners about the names of the players
//-----------------------------------------------------------------------------
void CardgameCollection::broadcastNames() {
    TRACE9("CardgameCollection::broadcastNames()");
    Check2(cmgr.getMode() != YGP::ConnectionMgr::NONE);

    std::string msg("ChgNames=");
    if (cmgr.getMode() == YGP::ConnectionMgr::SERVER)
        for (auto& i : aPlayer)
            msg += Card::sanitizeName(i->getName()) + '\n';
    else
        msg += Card::sanitizeName(aPlayer[0]->getName());

    broadcastMsg(msg);
}

//-----------------------------------------------------------------------------
/// Sends the changed settings to the clients
//-----------------------------------------------------------------------------
void CardgameCollection::sendSettings() {
    if (cmgr.getMode() != YGP::ConnectionMgr::NONE) {
        std::ostringstream msg;
        msg << "Settings=";

#    ifdef WITH_BURACO
        msg << "BCards=" << BuracoCards::get()[Buraco::CARDS2DEAL] << ";BPoints=" << Buraco::ENDPOINTS << ';';
#    endif

#    ifdef WITH_HEARTS
        msg << "HPoints=" << Hearts::ENDPOINTS << ';';
#    endif

#    ifdef WITH_ROVHULT
        msg << "RCardNuke=" << CardValue::get()[Rovhult::cardNuke] << ";RCardReverse=" << CardValue::get()[Rovhult::cardReverse]
            << ";RCardSkip=" << CardValue::get()[Rovhult::cardSkip] << ';';
#    endif
#    ifdef WITH_SGTMAYOR
        msg << "STricks=" << SgtMayor::ENDTRICKS << ';';
#    endif

        broadcastMsg(msg.str());
    }
}

#endif
