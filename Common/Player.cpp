//$Id$

//PROJECT     : Cardgames
//SUBSYSTEM   : Common
//REFERENCES  :
//TODO        :
//BUGS        :
//REVISION    : $Revision$
//AUTHOR      : Markus Schwab
//CREATED     : 25.07.2003
//COPYRIGHT   : Copyright (C) 2002 - 2004

// This program is free software; you can redistribute it and/or modify
// it under the terms of the GNU General Public License as published by
// the Free Software Foundation; either version 2 of the License, or
// (at your option) any later version.

// This program is distributed in the hope that it will be useful,
// but WITHOUT ANY WARRANTY; without even the implied warranty of
// MERCHANTABILITY or FITNESS FOR A PARTICULAR PURPOSE.  See the
// GNU General Public License for more details.

// You should have received a copy of the GNU General Public License
// along with this program; if not, write to the Free Software
// Foundation, Inc., 59 Temple Place - Suite 330, Boston, MA 02111-1307, USA.


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
