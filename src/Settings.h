#ifndef SETTINGS_H
#define SETTINGS_H

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

#include <string>

#include <gtkmm/entry.h>

#include <XGP/EnumEntry.h>
#include <XGP/XAttrEntry.h>
#include <XGP/XAttrSpin.h>

#include "GameTypes.h"

#include <XGP/XDialog.h>

// Forward declarations
class Options;

namespace Gtk {
class Grid;
}

class Settings : public XGP::XDialog {
  public:
    virtual ~Settings();

    static Settings* create(Gtk::Window& parent, Options& options);

    /// Signal emitted, when OK clicked
    sigc::signal<void()> sigCommit;
    sigc::signal<void()> sigCardResize;

  protected:
    Settings(Options& options);

  private:
    // Prohibited manager functions
    Settings(const Settings& other);
    const Settings& operator=(const Settings& other);

    virtual void okEvent();

    XGP::EnumEntry gameType;
    XGP::XAttributeSpinEntry<unsigned int> timeout;
    XGP::EnumEntry cardSize;
#ifdef WITH_BURACO
    XGP::XAttributeSpinEntry<unsigned int> maxBuracoPoints;
    XGP::EnumEntry numBuracoCards;
#endif
#ifdef WITH_HEARTS
    XGP::XAttributeSpinEntry<unsigned int> maxHeartsPoints;
#endif
#ifdef WITH_ROVHULT
    XGP::EnumEntry cardNuke;
    XGP::EnumEntry cardReverse;
    XGP::EnumEntry cardSkip;

    void chgValueRovhult(unsigned int which);
#endif
#ifdef WITH_SGTMAYOR
    XGP::XAttributeSpinEntry<unsigned int> tricksSgtMayor;
#endif

    static XGP::XAttributeSpinEntry<unsigned int> Settings::* intFields[];
    static Settings* instance;

    unsigned int& startGame;
};

#endif
