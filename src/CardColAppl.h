#ifndef CARDCOLAPPL_H
#define CARDCOLAPPL_H

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

#include "Options.h"

#include <YGP/IVIOAppl.h>

/**Helper to access an enumeration as unsigned int& (as expected by
 * YGP::MetaEnumAttribute), without type-punning the enum.
 *
 * The value of the enumeration is copied on construction and written back
 * on destruction, so objects of this class must live as long as the
 * attribute referencing them is used (for reading).
 */
template <typename E> class EnumAsUInt {
  public:
    /// Constructor
    /// \param value Enumeration value to access
    explicit EnumAsUInt(E& value) : ref(value), val(static_cast<unsigned int>(value)) {}
    /// Destructor; writes the (possibly changed) value back to the enumeration
    ~EnumAsUInt() { ref = static_cast<E>(val); }

    /// Returns the value as reference to unsigned int
    operator unsigned int&() { return val; }

  private:
    EnumAsUInt(const EnumAsUInt&) = delete;
    EnumAsUInt& operator=(const EnumAsUInt&) = delete;

    E& ref;
    unsigned int val;
};

/**Console part of the Cardgames; cares about reading the INI-file and
 * processing the options
 */
class CardgameAppl : public YGP::IVIOApplication {
  public:
    CardgameAppl(const int argc, const char* argv[]) : IVIOApplication(argc, argv, lo), options() {}
    ~CardgameAppl() override = default;

    static int convertToGameType(const char* pText);

#ifdef WITH_NETWORK
    static constexpr unsigned int PORT{31338}; ///< Default port for network games
#endif

  protected:
    void readINIFile(const char* pFile) override;
    bool handleOption(const char option) override;

    // Program-handling
    bool shallShowInfo() const override { return false; }
    int perform(int argc, const char* argv[]) override;
    const char* name() const override { return PACKAGE_NAME; }
    const char* description() const override;

    // Help-handling
    void showHelp() const override;
    void showGames() const;

  private:
    // Prohobited manager functions
    CardgameAppl() = delete;
    CardgameAppl(const CardgameAppl&) = delete;
    const CardgameAppl& operator=(const CardgameAppl&) = delete;

    Options options;

    static const longOptions lo[];
};

#endif
