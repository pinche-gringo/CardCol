//$Id$

//PROJECT     : Cardgames
//SUBSYSTEM   : Common
//REFERENCES  :
//TODO        :
//BUGS        :
//REVISION    : $Revision$
//AUTHOR      : Markus Schwab
//CREATED     : 25.07.2003
//COPYRIGHT   : Anticopyright (A) 2003

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
#include "ComputerPlayer.h"


unsigned int ComputerPlayer::TIMEOUT (700);


//-----------------------------------------------------------------------------
/// Destructor
//-----------------------------------------------------------------------------
ComputerPlayer::~ComputerPlayer () {
}


//----------------------------------------------------------------------------
/// Executes the turn of a computer player
/// \param game: Game played
//----------------------------------------------------------------------------
bool ComputerPlayer::makeTurn (Game* game) {
   TRACE1 ("ComputerPlayer::makeTurn (Game*) - " << name);
   Check1 (game);
   return game->makeComputerMove ();
}

//----------------------------------------------------------------------------
/// Time in between two turns of the player.
/// \returns unsigned int: Time to delay: 0 -> execute immediately
//----------------------------------------------------------------------------
unsigned int ComputerPlayer::timeout () const {
   return TIMEOUT;
}
