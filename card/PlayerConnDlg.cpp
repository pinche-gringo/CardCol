// PROJECT     : Cardgames
// SUBSYSTEM   : Common
// REFERENCES  :
// TODO        :
// BUGS        :
// AUTHOR      : Markus Schwab
// CREATED     : 24.07.2003
// COPYRIGHT   : Copyright (C) 2003 - 2006, 2008, 2009, 2024, 2026

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

#include <memory>
#include <ranges>
#include <sstream>
#include <string>
#include <string_view>

#include <gtkmm/entry.h>
#include <gtkmm/grid.h>
#include <gtkmm/label.h>
#include <gtkmm/messagedialog.h>
#include <gtkmm/passwordentry.h>

#include <cardgames-cfg.h>

#include <YGP/AssParse.h>
#include <YGP/AttrParse.h>
#include <YGP/Check.h>
#include <YGP/ConnMgr.h>
#include <YGP/Trace.h>

#include <XGP/XAttribute.h>

#include "Human.h"
#include "Message.h"
#include "RemotePlayer.h"

#include "PlayerConnDlg.h"

namespace Card {

//-----------------------------------------------------------------------------
/// Default constructor
/// \param player The player
/// \param defPort Default port to listen at/send to
/// \param connMgr Connection manager; holding the connections to use
//-----------------------------------------------------------------------------
PlayerConnectDlg::PlayerConnectDlg(std::vector<Player*>& player, const Glib::ustring& port, YGP::ConnectionMgr& cmgr)
    : XGP::ConnectDlg(player.size() - 1, port, cmgr), connected(Gtk::make_managed<Gtk::Label>()),
      lblConnected(Gtk::make_managed<Gtk::Label>(_("Connected:"))), aPlayer(player), posPlayer(0) {
    TRACE8("PlayerConnectDlg::PlayerConnectDlg(std::vector<Player*>&, const Glib::ustring&, ConnectionMgr&");

    lblConnected->set_margin_start(5);
    lblConnected->set_margin_end(5);
    lblConnected->set_margin_top(3);
    lblConnected->set_margin_bottom(3);
    pClient->attach(*lblConnected, 0, FIRST_FREE_ROW);

    connected->set_hexpand();
    connected->set_margin_start(5);
    connected->set_margin_end(5);
    connected->set_margin_top(3);
    connected->set_margin_bottom(3);
    pClient->attach(*connected, 1, FIRST_FREE_ROW);

    lblConnected->set_xalign(0);
    lblConnected->set_yalign(0);
    connected->set_xalign(0);
    connected->set_yalign(0);
    lblConnected->show();
    connected->show();
}

//-----------------------------------------------------------------------------
/// Destructor
//-----------------------------------------------------------------------------
PlayerConnectDlg::~PlayerConnectDlg() = default;

//----------------------------------------------------------------------------
/// Performs the dialog (modal)
/// \param player The player
/// \param defPort Default port to listen at/send to
/// \param connMgr Connection manager; holding the connections to use
/// \returns unsigned int Number player has for the server
//----------------------------------------------------------------------------
unsigned int PlayerConnectDlg::perform(std::vector<Player*>& player, unsigned int defPort, YGP::ConnectionMgr& connMgr) {
    std::ostringstream port;
    port << defPort;
    return perform(player, port.str(), connMgr);
}

//----------------------------------------------------------------------------
/// Performs the dialog (modal)
/// \param player The player
/// \param defPort Default port to listen at/send to
/// \param connMgr Connection manager; holding the connections to use
/// \returns unsigned int Number player has for the server
//----------------------------------------------------------------------------
unsigned int PlayerConnectDlg::perform(std::vector<Player*>& player, const Glib::ustring& defPort, YGP::ConnectionMgr& connMgr) {
    std::unique_ptr<PlayerConnectDlg> dlg(std::make_unique<PlayerConnectDlg>(player, defPort, connMgr));
    XGP::runModal(*dlg);
    dlg->hide(); // Destroying a still visible dialog confuses GTK's (Wayland) input method handling
    return dlg->posPlayer;
}

//----------------------------------------------------------------------------
/// Performs the dialog (modal)
/// \param player The player
/// \param connMgr Connection manager; holding the connections to use
/// \param listenAt Port the server should listen at
/// \param password Password the clients must know
/// \returns unsigned int The number the player has for the server
//----------------------------------------------------------------------------
unsigned int PlayerConnectDlg::perform(std::vector<Player*>& player, YGP::ConnectionMgr& cmgr, const Glib::ustring& listenAt,
                                       const Glib::ustring& password) {
    std::unique_ptr<PlayerConnectDlg> dlg(std::make_unique<PlayerConnectDlg>(player, "0", cmgr));
    Check3(dlg->pPort);
    Check3(dlg->pWait);
    dlg->pPort->set_text(listenAt);
    dlg->pPassword->set_text(password);
    dlg->command(WAIT); // Button::activate() would only emit clicked (delayed) after an animation
    XGP::runModal(*dlg);
    dlg->hide(); // Destroying a still visible dialog confuses GTK's (Wayland) input method handling
    return dlg->posPlayer;
}

//----------------------------------------------------------------------------
/// Performs the dialog (modal)
/// \param player The player
/// \param connMgr Connection manager; holding the connections to use
/// \param host Host the client should connect too
/// \param hostPort Port the host is listening at
/// \param password Password of the server
/// \returns unsigned int The number the player has for the server
//----------------------------------------------------------------------------
unsigned int PlayerConnectDlg::perform(std::vector<Player*>& player, YGP::ConnectionMgr& cmgr, const Glib::ustring& host,
                                       const Glib::ustring& hostPort, const Glib::ustring& password) {
    // Connect without user interaction; so don't create the dialog (destroying
    // it directly after showing it crashes GTK's Wayland input method handling)
    cmgr.setPassword(password.raw());
    return connectToServer(player, cmgr, host, hostPort);
}

//----------------------------------------------------------------------------
/// Connects this application to a server
/// \param target Name or IP address of the server
/// \param port Port the server is listening at
//----------------------------------------------------------------------------
void PlayerConnectDlg::connect(const Glib::ustring& target, const Glib::ustring& port) {
    posPlayer = connectToServer(aPlayer, cmgr, target, port);
}

//----------------------------------------------------------------------------
/// Connects to a server and negotiates the players with it. Errors are
/// displayed to the user.
/// \param aPlayer The players; replaced by the ones received from the server
/// \param cmgr Connection manager; holding the connection afterwards
/// \param target Name or IP address of the server
/// \param port Port the server is listening at
/// \returns unsigned int The number the player has for the server (0 in case
///     of an error; the connection manager is reset then)
//----------------------------------------------------------------------------
unsigned int PlayerConnectDlg::connectToServer(std::vector<Player*>& aPlayer, YGP::ConnectionMgr& cmgr,
                                               const Glib::ustring& target, const Glib::ustring& port) {
    TRACE3("PlayerConnectDlg::connectToServer(...) - " << target << ':' << port);
    unsigned int posPlayer(0);
    Glib::ustring error;
    try {
        cmgr.connectTo(target.raw(), port.raw());
    }
    catch (boost::system::system_error& err) {
        error = _("Can't connect to server!\n\nReason: %1");
        error.replace(error.find("%1"), 2, err.code().message());
    }

    if (error.empty()) {
        Check1(cmgr.getServer());
        YGP::Connection::Deadline deadline(*cmgr.getServer(), cmgr.getTimeout());
        try {
            std::string data("Version=" STRPROTOCOLL ";Variant=" STRVARIANT ";");
            data += YGP::AssignmentParse::makeAssignment("Name", sanitizeName(aPlayer[0]->getName()).raw());
            sendMessage(*cmgr.getServer(), data);

            std::string input(receiveMessage(*cmgr.getServer()));
            TRACE8("PlayerConnectDlg::connectToServer(...) - Received: " << input);

            Glib::ustring names;
            unsigned int rc(0);
            YGP::AttributeParse ap;
            ATTRIBUTE(ap, unsigned int, posPlayer, "Self");
            ATTRIBUTE(ap, Glib::ustring, names, "Names");
            ATTRIBUTE(ap, Glib::ustring, error, "Msg");
            ATTRIBUTE(ap, unsigned int, rc, "Error");
            ap.assignValues(input);

            if (rc)
                throw error.raw();
            if (!posPlayer)
                throw std::string(_("Position of this player is missing!"));

            // Check the received names before replacing the players
            std::vector<Glib::ustring> aNames;
            for (auto line : names.raw() | std::views::split('\n'))
                if (!line.empty())
                    aNames.push_back(sanitizeName(std::string(std::string_view(line))));
            TRACE9("PlayerConnectDlg::connectToServer(...) - Players: " << aNames.size());
            if ((aNames.size() < 2) || (aNames.size() > 9) || (posPlayer >= aNames.size()))
                throw std::string(_("Wrong number of players!"));

            // Clear the old players
            for (auto& i : aPlayer)
                delete i;
            aPlayer.clear();

            for (const auto& [c, name] : std::views::enumerate(aNames)) {
                TRACE9("PlayerConnectDlg::connectToServer(...) - Setting " << name);
                Player* pPlayer((static_cast<unsigned int>(c) == posPlayer)
                                    ? static_cast<Player*>(new Human(name))
                                    : static_cast<Player*>(new RemotePlayer(cmgr.getServer(), name)));

                // The players are rotated, so this player is the first one
                if (static_cast<unsigned int>(c) < posPlayer)
                    aPlayer.push_back(pPlayer);
                else
                    aPlayer.insert(aPlayer.begin() + (c - posPlayer), pPlayer);
            }
        }
        catch (boost::system::system_error& err) {
            error = _("Error exchanging the players with the server!\n\nReason: %1");
            error.replace(error.find("%1"), 2,
                          deadline.expired() ? make_error_code(YGP::ConnectionError::timeout).message() : err.code().message());
        }
        catch (YGP::ParseError& err) {
            error = _("Invalid response from server!\n\nReason: %1");
            error.replace(error.find("%1"), 2, err.what());
        }
        catch (std::string& err) {
            error = _("Invalid response from server!\n\nReason: %1");
            error.replace(error.find("%1"), 2, err);
        }
    }
    if (error.size()) {
        posPlayer = 0;
        cmgr.changeMode(YGP::ConnectionMgr::NONE);

        Gtk::MessageDialog dlg(error, false, Gtk::MessageType::ERROR, Gtk::ButtonsType::OK);
        dlg.set_title(PACKAGE);
        XGP::runModal(dlg);
    }
    return posPlayer;
}

//----------------------------------------------------------------------------
/// Negotiates the players with a newly connected client. Only clients
/// passing this are added to the connections; errors are displayed to the
/// user (and the connection to the client is closed).
/// \param connection Connection over which the clients communicates (NULL, if
///    the client could not connect)
/// \returns YGP::Connection* Added connection (or NULL)
//----------------------------------------------------------------------------
YGP::Connection* PlayerConnectDlg::addClient(std::unique_ptr<YGP::Connection> connection) {
    TRACE3("PlayerConnectDlg::addClient(std::unique_ptr<YGP::Connection>)");
    if (!connection)
        return nullptr;

    // Client n sits at position n (as seen from the server)
    const std::size_t pos(cmgr.getClients().size() + 1);
    Check3(aPlayer.size() < 10);
    if (pos >= aPlayer.size()) {
        TRACE1("PlayerConnectDlg::addClient(std::unique_ptr<YGP::Connection>) - No free seat; rejecting client");
        return nullptr;
    }

    Glib::ustring error, warning, name;
    {
        // Limit only the negotiation (and not the dialogs following) by the deadline
        YGP::Connection::Deadline deadline(*connection, cmgr.getTimeout());
        try {
            std::string input(receiveMessage(*connection));
            TRACE8("PlayerConnectDlg::addClient(std::unique_ptr<YGP::Connection>) - Received: " << input);

            unsigned int protocoll(0), variant(0);
            YGP::AttributeParse ap;
            ATTRIBUTE(ap, Glib::ustring, name, "Name");
            ATTRIBUTE(ap, unsigned int, protocoll, "Version");
            ATTRIBUTE(ap, unsigned int, variant, "Variant");
            try {
                ap.assignValues(input);
            }
            catch (YGP::ParseError& e) {
                throw std::string(e.what());
            }

            if (protocoll < PROTOCOLL) {
                error = _("Protocoll version %1 needed!");
                error.replace(error.find("%1"), 2, STRPROTOCOLL);
                throw error.raw();
            }
            else if ((protocoll == PROTOCOLL) && (variant < PROT_VARIANT)) {
                warning = _("Protocoll variant not sufficient - Version %1 needed!\n\n"
                            "Generally this means, that the partner does not support\n"
                            "all game types - Continue at your own risk!");
                warning.replace(warning.find("%1"), 2, STRVARIANT);
            }

            name = sanitizeName(name);
            if (name.empty())
                throw std::string(_("Name of the player is missing!"));
            TRACE8("PlayerConnectDlg::addClient(std::unique_ptr<YGP::Connection>) - Connected: " << name);

            input = "Self=";
            input += static_cast<char>('0' + pos);
            input += ";Names=";
            for (const auto& [i, player] : std::views::enumerate(aPlayer))
                input += sanitizeName((static_cast<std::size_t>(i) == pos) ? name : player->getName()) + std::string(1, '\n');

            TRACE8("PlayerConnectDlg::addClient(std::unique_ptr<YGP::Connection>) - Sending players: " << input);
            sendMessage(*connection, input);
        }
        catch (boost::system::system_error& err) {
            error = _("Error getting player name!\n\nReason: %1");
            error.replace(error.find("%1"), 2,
                          deadline.expired() ? make_error_code(YGP::ConnectionError::timeout).message() : err.code().message());
        }
        catch (std::string& err) {
            try {
                sendMessage(*connection, "Error=99;" + YGP::AssignmentParse::makeAssignment("Msg", err));
            }
            catch (boost::system::system_error&) {
            }
            error = _("Error analyzing input from client!\n\nReason: %1");
            error.replace(error.find("%1"), 2, err);
        }
    }

    YGP::Connection* conn(nullptr);
    if (error.empty()) {
        conn = connection.get();
        delete aPlayer[pos];
        aPlayer[pos] = new RemotePlayer(conn, name);
        connected->set_text(connected->get_text() + name + '\n');
        ConnectDlg::addClient(std::move(connection));
    }

    if (error.size() || warning.size()) {
        Gtk::MessageDialog dlg(error.size() ? error : warning, false, Gtk::MessageType::ERROR, Gtk::ButtonsType::OK);
        dlg.set_title(PACKAGE);
        XGP::runModal(dlg);
    }
    return conn;
}

} // namespace Card
