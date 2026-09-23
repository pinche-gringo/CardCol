#ifndef CARDSIZES_H
#define CARDSIZES_H

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

/**Enumeration holding the supported card-sizes
 */
class CardSizes : public YGP::MetaEnum {
  public:
    enum SIZES { TINY = 0, SMALL, NORMAL, BIG };

    /// Creates a meta-enum of type CardSizes
    /// \returns CardSizes& Instance of CardSizes
    static const CardSizes& get() {
        static const CardSizes instance;
        return instance;
    }
    ~CardSizes() override;

    static constexpr SIZES getSize(unsigned int width, unsigned int height);
    static constexpr unsigned int getWidth(SIZES size);
    static constexpr unsigned int getHeight(SIZES size);

    static constexpr unsigned int WIDTH_TINY{66};
    static constexpr unsigned int HEIGHT_TINY{88};
    static constexpr unsigned int WIDTH_SMALL{69};
    static constexpr unsigned int HEIGHT_SMALL{92};
    static constexpr unsigned int WIDTH_NORMAL{72};
    static constexpr unsigned int HEIGHT_NORMAL{96};
    static constexpr unsigned int WIDTH_BIG{75};
    static constexpr unsigned int HEIGHT_BIG{100};

  private:
    CardSizes();
    CardSizes(const CardSizes& other) = delete;

    const CardSizes& operator=(const CardSizes& other) = delete;
};

//-----------------------------------------------------------------------------
/// Returns the size (as enum-value) which corresponds to the passes values
/// \param width Width to check
/// \param height Height to check
/// \returns CardSizes::SIZES Corresponding size
//-----------------------------------------------------------------------------
constexpr CardSizes::SIZES CardSizes::getSize(unsigned int width, unsigned int height) {
    width += height;
    if (width <= (WIDTH_TINY + HEIGHT_TINY))
        return TINY;
    else if (width <= (WIDTH_SMALL + HEIGHT_SMALL))
        return SMALL;
    else if (width <= (WIDTH_NORMAL + HEIGHT_NORMAL))
        return NORMAL;
    return BIG;
}

//-----------------------------------------------------------------------------
/// Returns the width corresponding to the passed size
/// \param size Size
/// \returns unsigned int Width corresponding to size
//-----------------------------------------------------------------------------
constexpr unsigned int CardSizes::getWidth(SIZES size) { return WIDTH_TINY + size * 3; }

//-----------------------------------------------------------------------------
/// Returns the height corresponding to the passed size
/// \param size Size
/// \returns unsigned int Height corresponding to size
//-----------------------------------------------------------------------------
constexpr unsigned int CardSizes::getHeight(SIZES size) { return HEIGHT_TINY + (size << 2); }

#endif
