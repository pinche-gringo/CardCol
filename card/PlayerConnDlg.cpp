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
#include <sstream>

#include <gtkmm/entry.h>
#include <gtkmm/grid.h>
#include <gtkmm/label.h>
#include <gtkmm/messagedialog.h>

#include <cardgames-cfg.h>

#include <YGP/AttrParse.h>
#include <YGP/Check.h>
#include <YGP/ConnMgr.h>
#include <YGP/Socket.h>
#include <YGP/Trace.h>

#include <XGP/XAttribute.h>

#include "Human.h"
#include "RemotePlayer.h"
#include "Tokenize.h"

#include "PlayerConnDlg.h"

namespace Card {

//-----------------------------------------------------------------------------
/// Default constructor
/// \param player The player
/// \param defPort Default port to listen at/send to
/// \param connMgr Connection manager; holding the connections to use
//-----------------------------------------------------------------------------
PlayerConnectDlg::PlayerConnectDlg(std::vector<Player*>& player, const Glib::ustring& port, YGP::ConnectionMgr& cmgr)
    : XGP::ConnectDlg(player.size(), port, cmgr), connected(Gtk::make_managed<Gtk::Label>()),
      lblConnected(Gtk::make_managed<Gtk::Label>(_("Connected:"))), aPlayer(player), posPlayer(0) {
    TRACE8("PlayerConnectDlg::PlayerConnectDlg(std::vector<Player*>&, const Glib::ustring&, ConnectionMgr&");

    lblConnected->set_margin_start(5);
    lblConnected->set_margin_end(5);
    lblConnected->set_margin_top(3);
    lblConnected->set_margin_bottom(3);
    pClient->attach(*lblConnected, 0, 3);

    connected->set_hexpand();
    connected->set_margin_start(5);
    connected->set_margin_end(5);
    connected->set_margin_top(3);
    connected->set_margin_bottom(3);
    pClient->attach(*connected, 1, 3);

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
    return dlg->posPlayer;
}

//----------------------------------------------------------------------------
/// Performs the dialog (modal)
/// \param player The player
/// \param connMgr Connection manager; holding the connections to use
/// \param listenAt Port the server should listen at
/// \returns unsigned int The number the player has for the server
//----------------------------------------------------------------------------
unsigned int PlayerConnectDlg::perform(std::vector<Player*>& player, YGP::ConnectionMgr& cmgr, const Glib::ustring& listenAt) {
    std::unique_ptr<PlayerConnectDlg> dlg(std::make_unique<PlayerConnectDlg>(player, "0", cmgr));
    Check3(dlg->pPort);
    Check3(dlg->pWait);
    dlg->pPort->set_text(listenAt);
    dlg->pWait->activate();
    XGP::runModal(*dlg);
    return dlg->posPlayer;
}

//----------------------------------------------------------------------------
/// Performs the dialog (modal)
/// \param player The player
/// \param connMgr Connection manager; holding the connections to use
/// \param host Host the client should connect too
/// \param hostPort Port the host is listening at
/// \returns unsigned int The number the player has for the server
//----------------------------------------------------------------------------
unsigned int PlayerConnectDlg::perform(std::vector<Player*>& player, YGP::ConnectionMgr& cmgr, const Glib::ustring& host,
                                       const Glib::ustring& hostPort) {
    std::unique_ptr<PlayerConnectDlg> dlg(std::make_unique<PlayerConnectDlg>(player, "0", cmgr));
    Check3(dlg->pPort);
    Check3(dlg->pConnect);
    Check3(dlg->pTarget);
    dlg->pTarget->set_text(host);
    dlg->pPort->set_text(hostPort);
    dlg->pConnect->activate();
    return dlg->posPlayer;
}

//----------------------------------------------------------------------------
/// Connects this application to a server
/// \param target Name or IP address of the server
/// \param port Port the server is listening at
//----------------------------------------------------------------------------
void PlayerConnectDlg::connect(const Glib::ustring& target, unsigned int port) {
    TRACE3("PlayerConnectDlg::connect(const Glib::ustring&, unsigned int) - " << target << ':' << port);
    Glib::ustring error;
    try {
        ConnectDlg::connect(target, port);
        Check1(cmgr.getSocket());

        Glib::ustring data("Version=" STRPROTOCOLL ";Variant=" STRVARIANT ";Name=\"" + aPlayer[0]->getName() + '"');
        cmgr.getSocket()->write(data);

        std::string input;
        cmgr.getSocket()->read(input);
        TRACE8("PlayerConnectDlg::connect(const Glib::ustring&, unsigned int) - Received: " << input);

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

        // Clear the old players
        for (auto& i : aPlayer)
            delete i;
        aPlayer.clear();

        Tokenize split(names.raw());
        unsigned int c(0);
        for (Glib::ustring name; !(name = split.getNextNode('\n')).empty();) {
            TRACE9("PlayerConnectDlg::connect(const Glib::ustring&, unsigned int) - Setting " << name);

            Player* pPlayer((c == posPlayer) ? static_cast<Player*>(new Human(name))
                                             : static_cast<Player*>(new RemotePlayer(cmgr.getSocket(), name)));

            if (c < posPlayer)
                aPlayer.push_back(pPlayer);
            else {
                Check3(aPlayer.size() > (c - posPlayer));
                aPlayer.insert(aPlayer.begin() + c - posPlayer, pPlayer);
            }
            c++;
        }
        TRACE9("PlayerConnectDlg::connect(const Glib::ustring&, unsigned int) - Players: " << c << "<->" << aPlayer.size());
        if ((c != aPlayer.size()) || (posPlayer >= aPlayer.size()))
            throw std::string(_("Wrong number of players!"));
    }
    catch (YGP::CommError& err) {
        error = _("Error sending player name!\n\nReason: %1");
        error.replace(error.find("%1"), 2, err.what());
    }
    catch (std::string& err) {
        error = _("Invalid response from server!\n\nReason: %1");
        error.replace(error.find("%1"), 2, err);
    }
    if (error.size()) {
        Gtk::MessageDialog dlg(error, false, Gtk::MessageType::ERROR, Gtk::ButtonsType::OK);
        dlg.set_title(PACKAGE);
        XGP::runModal(dlg);
    }
}

//----------------------------------------------------------------------------
/// Updates the name of the player with the data send from the client
/// \param socket Socket over which the clients communicates
//----------------------------------------------------------------------------
YGP::Socket* PlayerConnectDlg::addClient(int socket) {
    TRACE3("PlayerConnectDlg::addClient(int)");
    YGP::Socket* sock(ConnectDlg::addClient(socket));
    Check3(sock);

    Glib::ustring error;
    try {
        std::string input;
        sock->read(input);
        TRACE8("PlayerConnectDlg::addClient(int) - Received: " << input);

        Glib::ustring name;
        unsigned int protocoll(0), variant(0);
        YGP::AttributeParse ap;
        ATTRIBUTE(ap, Glib::ustring, name, "Name");
        ATTRIBUTE(ap, unsigned int, protocoll, "Version");
        ATTRIBUTE(ap, unsigned int, variant, "Variant");
        ap.assignValues(input);

        if (protocoll < PROTOCOLL) {
            error = _("Protocoll version %1 needed!");
            error.replace(error.find("%1"), 2, STRPROTOCOLL);
            throw error.raw();
        }
        else if ((protocoll == PROTOCOLL) && (variant < PROT_VARIANT)) {
            error = _("Protocoll variant not sufficient - Version %1 needed!\n\n"
                      "Generally this means, that the partner does not support\n"
                      "all game types - Continue at your own risk!");
            error.replace(error.find("%1"), 2, STRVARIANT);
        }

        TRACE8("PlayerConnectDlg::addClient(int) - Connected: " << name);
        Check3(aPlayer.size() > cmgr.getClients().size());
        delete aPlayer[cmgr.getClients().size()];
        aPlayer[cmgr.getClients().size()] = new RemotePlayer(sock, name);
        connected->set_text(connected->get_text() + name + '\n');

        Check3(aPlayer.size() < 10);
        input = "Self=";
        input += static_cast<char>('0' + cmgr.getClients().size());
        input += ";Names=";
        for (auto& i : aPlayer)
            input += i->getName() + std::string(1, '\n');

        TRACE8("PlayerConnectDlg::addClient(int) - Sending players: " << input);
        sock->write(input);
    }
    catch (YGP::CommError& err) {
        error = _("Error getting player name!\n\nReason: %1");
        error.replace(error.find("%1"), 2, err.what());
    }
    catch (std::string& err) {
        sock->write("Error=99;Msg=\"");
        sock->write(err);
        sock->write("\"");
        error = _("Error analyzing input from client!\n\nReason: %1");
        error.replace(error.find("%1"), 2, err);
    }
    if (error.size()) {
        Gtk::MessageDialog dlg(error, false, Gtk::MessageType::ERROR, Gtk::ButtonsType::OK);
        dlg.set_title(PACKAGE);
        XGP::runModal(dlg);
    }
    return sock;
}

} // namespace Card
