#ifndef REMOTEHUMAN_H
#define REMOTEHUMAN_H

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

#include "Player.h"

namespace YGP {
class Connection;
} // namespace YGP

namespace Card {

/**Implementation of a player on a (remote) computer, communicating over a
   network.
*/
class RemotePlayer : public Player {
  public:
    /// Constructor; taking the connection for the communication and a name of
    /// the player
    /// \param connection Connection used for the communication
    /// \param name Name of the (remote) player
    RemotePlayer(YGP::Connection* connection, const Glib::ustring& name) : Player(name), conn(connection) {}
    ~RemotePlayer() override;

    bool makeTurn(Game* game) override;

  protected:
    YGP::Connection* conn;

  private:
    RemotePlayer() = delete;
    RemotePlayer(const RemotePlayer& other) = delete;
    const RemotePlayer& operator=(const RemotePlayer& other) = delete;
};

} // namespace Card

#endif
