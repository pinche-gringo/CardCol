#ifndef PLAYERCONNECTDLG_H
#define PLAYERCONNECTDLG_H

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
#include <vector>

#include <boost/asio/ip/tcp.hpp>

#include <XGP/ConnectDlg.h>

namespace Card {
class Player;
}

namespace Card {

/**Implementation of the connect dialog for the players of the Cardgame
   collection.

   After establishing the connection it negotiates the player names.
*/
class PlayerConnectDlg : public XGP::ConnectDlg {
  public:
    PlayerConnectDlg(std::vector<Player*>& player, const Glib::ustring& port, YGP::ConnectionMgr& cmgr);
    ~PlayerConnectDlg() override;

    static unsigned int perform(std::vector<Player*>& player, const Glib::ustring& port, YGP::ConnectionMgr& cmgr);
    static unsigned int perform(std::vector<Player*>& player, unsigned int port, YGP::ConnectionMgr& cmgr);
    static unsigned int perform(std::vector<Player*>& player, YGP::ConnectionMgr& cmgr, const Glib::ustring& listenAt);
    static unsigned int perform(std::vector<Player*>& player, YGP::ConnectionMgr& cmgr, const Glib::ustring& host,
                                const Glib::ustring& hostPort);

  protected:
    /// \name Connection management
    void connect(const Glib::ustring& target, const Glib::ustring& port) override;
    boost::asio::ip::tcp::socket* addClient(std::unique_ptr<boost::asio::ip::tcp::socket> socket) override;
    static unsigned int connectToServer(std::vector<Player*>& aPlayer, YGP::ConnectionMgr& cmgr, const Glib::ustring& target,
                                        const Glib::ustring& port);

    Gtk::Label* connected;
    Gtk::Label* lblConnected;

  private:
    std::vector<Player*>& aPlayer;
    unsigned int posPlayer;

    PlayerConnectDlg(const PlayerConnectDlg& other) = delete;
    const PlayerConnectDlg& operator=(const PlayerConnectDlg& other) = delete;
};

} // namespace Card

#endif
