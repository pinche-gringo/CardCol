#ifndef GAMETYPES_H
#define GAMETYPES_H

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

#include <YGP/MetaEnum.h>

/**Class to store the different game-types; both as string and as (unique)
   numeric representation
*/
class GameTypes : public YGP::MetaEnum {
  public:
    enum {
#ifdef WITH_BURACO
        BURACO,
#endif
#ifdef WITH_HEARTS
        HEARTS,
#endif
#ifdef WITH_JABBERWOCKY
        JABBERWOCKY,
#endif
#ifdef WITH_MACHIAVELLI
        MACHIAVELLI,
#endif
#ifdef WITH_ROVHULT
        ROVHULT,
#endif
#ifdef WITH_SGTMAYOR
        SGTMAYOR,
#endif
#ifdef WITH_TWOPART
        TWOPART,
#endif
        LAST,
        NONE = -1
    };

    /// Creates a meta-enum of type GameTypes
    /// \returns GamesTypes& Instance of GameTypes
    static const GameTypes& get() {
        static const GameTypes instance;
        return instance;
    }

    ~GameTypes() override;

    static const char* getKey(int type);
    static int fromKey(const char* pKey);

  private:
    GameTypes();
    GameTypes(const GameTypes& other) = delete;

    const GameTypes& operator=(const GameTypes& other) = delete;
};

#endif
