//$Id$

//PROJECT     : Cardgames
//SUBSYSTEM   : Common
//REFERENCES  :
//TODO        :
//BUGS        :
//REVISION    : $Revision$
//AUTHOR      : Markus Schwab
//CREATED     : 25.07.2003
//COPYRIGHT   : Copyright (C) 2002 - 2004, 2008

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
// along with libYGP.  If not, see <http://www.gnu.org/licenses/>.


#include "Player.h"

//-----------------------------------------------------------------------------
/// Constructor; from a name
/// \param playerName: Name of the player
//-----------------------------------------------------------------------------
Player::Player (const Glib::ustring& playerName) : name (playerName) {
}

//-----------------------------------------------------------------------------
/// Destructor
//-----------------------------------------------------------------------------
Player::~Player () {
}


//----------------------------------------------------------------------------
/// Time in between two turns of the player.
/// \returns unsigned int: Time to delay: 0 -> execute immediately
//----------------------------------------------------------------------------
unsigned int Player::timeout () const {
   return 0;
}
