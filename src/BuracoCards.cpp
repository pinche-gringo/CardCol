// PROJECT     : Cardgames
// SUBSYSTEM   : BuracoCards
// REFERENCES  :
// TODO        :
// BUGS        :
// AUTHOR      : Markus Schwab
// CREATED     : 03.08.2006
// COPYRIGHT   : Copyright (C) 2006, 2008, 2026

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

#include "BuracoCards.h"

//-----------------------------------------------------------------------------
/// Default constructor; Fills the values
//-----------------------------------------------------------------------------
BuracoCards::BuracoCards() {
    insert(std::make_pair(11, "11"));
    insert(std::make_pair(13, "13"));
    insert(std::make_pair(15, "15"));
}

//-----------------------------------------------------------------------------
/// Destructor
//-----------------------------------------------------------------------------
BuracoCards::~BuracoCards() = default;
