// PROJECT     : Cardgames
// SUBSYSTEM   : GameTypes
// REFERENCES  :
// TODO        :
// BUGS        :
// AUTHOR      : Markus Schwab
// CREATED     : 28.04.2005
// COPYRIGHT   : Copyright (C) 2005 - 2018, 2026

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

#include <cstring>

#include <string>

#include "GameTypes.h"

namespace {

/// Untranslated names of the games (in the order of the enum); used in the INI-file
const char* const KEYS[] = {
#ifdef WITH_BURACO
    N_("Buraco"),
#endif
#ifdef WITH_HEARTS
    N_("Hearts"),
#endif
#ifdef WITH_JABBERWOCKY
    N_("Jabberwocky"),
#endif
#ifdef WITH_MACHIAVELLI
    N_("Machiavelli"),
#endif
#ifdef WITH_ROVHULT
    /* For translations: Write the Rovhult as o-slash */
    N_("Rovhult"),
#endif
#ifdef WITH_SGTMAYOR
    N_("SgtMayor"),
#endif
#ifdef WITH_TWOPART
    N_("Twopart"),
#endif
};
static_assert(sizeof(KEYS) / sizeof(*KEYS) == GameTypes::LAST, "KEYS must match the enum");

} // namespace

//-----------------------------------------------------------------------------
/// Defaultconstructor
//-----------------------------------------------------------------------------
GameTypes::GameTypes() {
    for (int i(0); i < LAST; ++i) {
        std::string name(_(KEYS[i]));
#ifdef WITH_ROVHULT
        // Without translation use the correct spelling anyway
        if ((i == ROVHULT) && (name == KEYS[i]))
            name = "R\u00f8vhult";
#endif
        insert(std::make_pair(i, name));
    }
}

//-----------------------------------------------------------------------------
/// Destructor
//-----------------------------------------------------------------------------
GameTypes::~GameTypes() = default;

//-----------------------------------------------------------------------------
/// Returns the untranslated name (as stored in the INI-file) of a game
/// \param type Type of the game
/// \returns const char* Name of the game; "" if type is invalid
//-----------------------------------------------------------------------------
const char* GameTypes::getKey(int type) {
    return ((type >= 0) && (type < LAST)) ? KEYS[type] : "";
}

//-----------------------------------------------------------------------------
/// Returns the type of a game from its untranslated name
/// \param pKey Untranslated name of the game
/// \returns int Type of the game; NONE if not found
//-----------------------------------------------------------------------------
int GameTypes::fromKey(const char* pKey) {
    for (int i(0); i < LAST; ++i)
        if (!std::strcmp(pKey, KEYS[i]))
            return i;
    return NONE;
}
