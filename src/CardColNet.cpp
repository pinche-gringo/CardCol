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
#    include <cstdint>
#    include <ranges>
#    include <sstream>
#    include <string>
#    include <string_view>

#    include <glibmm/main.h>
#    include <gtkmm/messagedialog.h>

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
            Check3(aCommThreads.size() == 1);
            cmgr.changeMode(YGP::ConnectionMgr::NONE);
            aCommThreads[0]->cancel();
            delete aCommThreads[0];
            aCommThreads.clear();

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
    Check2(aCommThreads.empty());

    if (cmgr.getMode() == YGP::ConnectionMgr::CLIENT) {
        status.pop();
        status.push(_("Waiting for the server to start the game ..."));
        aCommThreads.push_back(THRDAPPL::create2(this, &CardgameCollection::waitForMessages,
                                                 reinterpret_cast<void*>(static_cast<std::intptr_t>(-1))));
        aCommThreads[0]->allowCancelation();
    }
    else {
        broadcastNames();

        for (unsigned int i(0); i < cmgr.getClients().size(); ++i) {
            aCommThreads.push_back(THRDAPPL::create2(this, &CardgameCollection::waitForMessages,
                                                     reinterpret_cast<void*>(static_cast<std::intptr_t>(i))));
            aCommThreads[i]->allowCancelation();
        }
    }

    apMenus[CHAT]->set_enabled(true);
}

//----------------------------------------------------------------------------
/// Wait for messages
/// \param thread Thread executing this method; its argument is the ID of the
///     player (-1 for server; 0 .. n for clients)
/// \returns \c void* NULL
//----------------------------------------------------------------------------
void* CardgameCollection::waitForMessages(void* thread) {
    TRACE1("CardgameCollection::waitForMessage(void*)");
    Check2(cmgr.getMode() != YGP::ConnectionMgr::NONE);

    long iPlayer(static_cast<long>(reinterpret_cast<std::intptr_t>(static_cast<THRDAPPL*>(thread)->getArgs())));
    Check2((cmgr.getMode() == YGP::ConnectionMgr::CLIENT) ? (iPlayer == -1)
                                                          : (iPlayer < static_cast<long>(cmgr.getClients().size())));

    boost::asio::ip::tcp::socket* sock((iPlayer == -1) ? cmgr.getSocket() : cmgr.getClients()[iPlayer].get());
    Check3(sock);
    Check3((iPlayer == -1) ? playerPos : true);
    iPlayer = (iPlayer == -1) ? (aPlayer.size() - playerPos) : (iPlayer + 1);
    try {
        while (true) {
            std::string message(Card::receiveMessage(*sock));
            TRACE7("CardgameCollection::waitForMessage(void*) - `" << message << '\'');
            if (message.empty())
                continue;

            TRACE9("CardgameCollection::waitForMessages(void*) - Lock (thread)");
            mxThreadCmd.lock(); // Wait til last message has been processed
            TRACE9("CardgameCollection::waitForMessages(void*) - Perform cmd " << message);

            Glib::signal_idle().connect([this, iPlayer, message] { return handleMessage(iPlayer, message); });
            mxGuiCmd.lock();
            mxThreadCmd.unlock();
            mxGuiCmd.unlock();
            TRACE9("CardgameCollection::waitForMessages(void*) - Handled msg");
        }
    }
    catch (boost::system::system_error& error) {
        std::string msg;
        if (error.code() == boost::asio::error::eof) {
            msg = _("Lost connection to %1!");
            Check3(static_cast<unsigned int>(iPlayer) < aPlayer.size());
            msg.replace(msg.find("%1"), 2, aPlayer[iPlayer]->getName());
        }
        else {
            msg = _("Error receiving data!\n\nReason: %1");
            msg.replace(msg.find("%1"), 2, error.code().message());
        }
        Glib::signal_idle().connect([this, msg] { return showMessage(msg); });
    }

    // The server must change the disconnected remote to a computer controled player
    if (cmgr.getMode() == YGP::ConnectionMgr::SERVER) {
        TRACE7("CardgameCollection::waitForMessages(void*) - Removing " << aPlayer[iPlayer]->getName());
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
    cmgr.disconnect(sock);

    aCommThreads.erase(std::ranges::find(aCommThreads, thread));
    if (aCommThreads.empty()) {
        apMenus[CHAT]->set_enabled(false);
        cmgr.changeMode(YGP::ConnectionMgr::NONE);
    }

    TRACE7("CardgameCollection::waitForMessages(void*) - Finished");
    return nullptr;
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
        int type(CardgameAppl::convertToGameType(param.c_str()));
        if (type == GameTypes::NONE) {
            cmd = _("Invalid game type: `%1'");
            cmd.replace(cmd.find("%1"), 2, param);
            throw YGP::ParseError(cmd);
        }

        actGame = type;
        if (game) {
            restart = true;
            if (restartGame())
                mxThreadCmd.unlock();
        }
        else {
            startGame();
            mxThreadCmd.unlock();
        }
        Card::sendMessage(*cmgr.getSocket(), "Error=0");
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
            aPlayer[player]->setName(param);
            broadcastNames();
        }
        else {
            unsigned int pos(-playerPos);
            for (auto line : param | std::views::split('\n')) {
                if (line.empty())
                    continue;

                std::string name(std::string_view{line});
                TRACE9("CardgameCollection::handleGlobalMessage(unsigned int, const std::string&) - Setting " << name);
                aPlayer[pos++ % aPlayer.size()]->setName(name);
            }
        }
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
            TRACE1("New points: " << Buraco::ENDPOINTS);

            try {
                TRACE1("Param: " << std::string(msg, msg.find('=')));
                ap.assignValues(std::string(msg, msg.find('=')));
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

    mxGuiCmd.unlock();
    mxThreadCmd.lock(); // Block message processing
    mxGuiCmd.lock();

    bool unlock(true);
    try {
        int rc(handleGlobalMessage(player, msg));
        if ((rc == -1) || (!rc && (game && !game->ignoreMessage() && !game->handleMessage(player, msg))))
            unlock = false;
    }
    catch (std::exception& error) {
        TRACE9("CardgameCollection::handleMessage(unsigned int, const std::string)"
               " - Error "
               << error.what());
        std::string msg("Error=99;Msg=\"");
        msg += error.what();
        msg += '"';
        broadcastMsg(msg);

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

    std::string sendString("Msg=\"");
    sendString += msg;
    sendString += "\";Sender=\"";
    sendString += aPlayer[0]->getName();
    sendString += '"';

    broadcastMsg(sendString);
}

//-----------------------------------------------------------------------------
/// Broadcast a message to all partners
/// \param msg Message to broadcast
/// \param exclude Partner to exclude (Zero-based; -1: None)
//-----------------------------------------------------------------------------
void CardgameCollection::broadcastMsg(const std::string& msg, unsigned int exclude) {
    TRACE9("CardgameCollection::broadcastMsg(const Glib::ustring&, unsigned int) - " << msg << "; Exclude: " << exclude);
    try {
        if (cmgr.getMode() == YGP::ConnectionMgr::SERVER) {
            for (const auto& [i, client] : std::views::enumerate(cmgr.getClients()))
                if (exclude != static_cast<unsigned int>(i)) {
                    Check(client);
                    Card::sendMessage(*client, msg);
                }
        }
        else {
            Check3(cmgr.getSocket());
            Card::sendMessage(*cmgr.getSocket(), msg);
        }
    }
    catch (std::exception&) {
    }
}

//-----------------------------------------------------------------------------
/// Starts the connections, if passed as options
/// \param options Options of program; containing host/port
//-----------------------------------------------------------------------------
void CardgameCollection::autoConnect(const Options& options) {
    if (options.port.size()) {
        TRACE9("CardgameCollection::autoConnect(const Options&) - Connect: " << options.target << '-' << options.port);
        if (options.target.size())
            playerPos = Card::PlayerConnectDlg::perform(aPlayer, cmgr, options.target, options.port);
        else
            playerPos = Card::PlayerConnectDlg::perform(aPlayer, cmgr, options.port);

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
/// Removes the threads for communication
//-----------------------------------------------------------------------------
void CardgameCollection::removeCommThreads() {
    for (auto& aCommThread : aCommThreads) {
        aCommThread->cancel();
        delete aCommThread;
    }
    aCommThreads.clear();
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
            msg += i->getName() + '\n';
    else
        msg += aPlayer[0]->getName();

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
