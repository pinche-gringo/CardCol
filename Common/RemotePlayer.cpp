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

#include <YGP/Check.h>
#include <YGP/Trace.h>

#include "Game.h"

#include "RemotePlayer.h"


//-----------------------------------------------------------------------------
/// Destructor
//-----------------------------------------------------------------------------
RemotePlayer::~RemotePlayer () {
}


//----------------------------------------------------------------------------
/// Reads the turn of a remote player.
/// \param game: Game played
/// \returns bool: Flag, if the method should be called again in the next turn.
//----------------------------------------------------------------------------
bool RemotePlayer::makeTurn (Game* game) {
   TRACE1 ("RemotePlayer::makeTurn (Game*) - " << name);
   Check1 (game);
   Check1 (sock);

   game->endTurn ();
   return false;
}
