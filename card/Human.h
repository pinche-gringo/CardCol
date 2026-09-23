#ifndef HUMAN_H
#define HUMAN_H

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

#include <card/Player.h>

namespace Card {

/**Implementation of a human player.
 */
class Human : public Player {
  public:
    /// Constructor from a name
    /// \param name Name of the human player
    Human(const Glib::ustring& name) : Player(name) {}
    virtual ~Human();

    virtual bool makeTurn(Game* game);

  private:
    Human();
    Human(const Human& other);
    const Human& operator=(const Human& other);
};

} // namespace Card

#endif
