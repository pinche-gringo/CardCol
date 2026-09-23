// PROJECT     : Cardgames
// SUBSYSTEM   : Settings
// REFERENCES  :
// TODO        :
// BUGS        :
// AUTHOR      : Markus Schwab
// CREATED     : 02.02.2007
// COPYRIGHT   : Copyright (C) 2007, 2008, 2026

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

#include "CardSizes.h"

//-----------------------------------------------------------------------------
/// Default constructor
//-----------------------------------------------------------------------------
CardSizes::CardSizes() {
    insert(std::make_pair(static_cast<int>(TINY), _("Tiny")));
    insert(std::make_pair(static_cast<int>(SMALL), _("Small")));
    insert(std::make_pair(static_cast<int>(NORMAL), _("Normal")));
    insert(std::make_pair(static_cast<int>(BIG), _("Big")));
}

//-----------------------------------------------------------------------------
/// Destructor
//-----------------------------------------------------------------------------
CardSizes::~CardSizes() = default;
