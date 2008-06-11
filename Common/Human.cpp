//$Id$

//PROJECT     : Cardgames
//SUBSYSTEM   : Common
//REFERENCES  :
//TODO        :
//BUGS        :
//REVISION    : $Revision$
//AUTHOR      : Markus Schwab
//CREATED     : 25.07.2003
//COPYRIGHT   : Copyright (C) 2003 - 2005, 2008

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


#include <YGP/Check.h>
#include <YGP/Trace.h>

#include "Game.h"
#include "Human.h"


//-----------------------------------------------------------------------------
/// Destructor
//-----------------------------------------------------------------------------
Human::~Human () {
}

//----------------------------------------------------------------------------
/// Allows the human to execute its turn by enabling its cards
/// \param game: Game played
/// \returns bool: Flag, if the method should be called again in the next turn.
//----------------------------------------------------------------------------
bool Human::makeTurn (Game* game) {
   TRACE1 ("Human::makeTurn (Game*) - " << name);
   Check1 (game);
   game->enableHuman ();
   return false;
}
