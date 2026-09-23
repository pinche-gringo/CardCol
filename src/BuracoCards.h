#ifndef BURACOCARDS_H
#define BURACOCARDS_H

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

#include <YGP/MetaEnum.h>

/**Storing the possible values a card can have as text
 */
class BuracoCards : public YGP::MetaEnum {
  public:
    /// Creates a meta-enum of type BuracoCards
    /// \returns BuracoCards& Instance of BuracoCards
    static const BuracoCards& get() {
        if (!instance)
            instance = new BuracoCards;
        return *instance;
    }
    ~BuracoCards();

  private:
    BuracoCards();
    BuracoCards(const BuracoCards& other);
    const BuracoCards& operator=(const BuracoCards& other);

    static BuracoCards* instance;
};

#endif
