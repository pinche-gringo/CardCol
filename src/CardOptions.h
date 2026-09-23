#ifndef CARDOPTIONS_H
#define CARDOPTIONS_H

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

#include <string>

#include <cardgames-cfg.h>

#include "CardCol.h"

#include <YGP/Entity.h>

/**Options handling the cards within the CardCol application
 */
class CardOptions : public YGP::Entity {
    friend class Settings;
    friend class CardgameAppl;
    friend class CardgameCollection;

  public:
    CardOptions();
    virtual ~CardOptions();

  private:
    CardOptions(const CardOptions& other);
    const CardOptions& operator=(const CardOptions& other);

    std::string decks; // %attrib%; Front;   CARDDECKS_DIR CARDDECKS_FRONT
    std::string back;  // %attrib%; Back;    CARDDECKS_DIR CARDDECKS_BACK
};

#endif
