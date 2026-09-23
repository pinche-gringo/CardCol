#ifndef PLAYER_H
#define PLAYER_H

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

#include <glibmm/ustring.h>
#include <sigc++/trackable.h>

namespace Card {
class Game;
}

namespace Card {

/**Abstract base class representing a player of the game.

   Every player can either be local human, remote human or computer player
*/
class Player : public sigc::trackable {
  public:
    explicit Player(Glib::ustring playerName);
    virtual ~Player();

    /// Returns the name of the player
    /// \returns Glib::ustring Name of the player
    const Glib::ustring& getName() const { return name; }
    /// Sets/changes the name of this player
    /// \param playerName New name of this player
    void setName(const Glib::ustring& playerName) { name = playerName; }

    /// Executes the turn of the player.
    /// \param game Game playing.
    /// \returns bool Flag, if the method should be called again in the next turn.
    virtual bool makeTurn(Game* game) = 0;

    virtual unsigned int timeout() const;

  protected:
    Glib::ustring name;

  private:
    Player(const Player& other) = delete;
    const Player& operator=(const Player& other) = delete;
};

} // namespace Card

#endif
