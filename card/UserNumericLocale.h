#ifndef CARD_USERNUMERICLOCALE_H
#define CARD_USERNUMERICLOCALE_H

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

#include <clocale>

namespace Card {

//-----------------------------------------------------------------------------
/// Uses (for the current thread) the numeric formatting of the user's locale
/// while an object of this class exists.
/// \remarks The global locale uses the classic numeric formatting (see
///          CardgameAppl::perform), as numbers are exchanged with the network
///          partners
//-----------------------------------------------------------------------------
class UserNumericLocale {
  public:
    UserNumericLocale() : prev(userLocale() ? uselocale(userLocale()) : nullptr) {}
    ~UserNumericLocale() {
        if (prev)
            uselocale(prev);
    }

    UserNumericLocale(const UserNumericLocale&) = delete;
    UserNumericLocale& operator=(const UserNumericLocale&) = delete;

  private:
    static locale_t userLocale() {
        static const locale_t loc(newlocale(LC_NUMERIC_MASK, "", duplocale(LC_GLOBAL_LOCALE)));
        return loc;
    }

    locale_t prev;
};

} // namespace Card

#endif
