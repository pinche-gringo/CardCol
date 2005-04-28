//$Id$

//PROJECT     : Cardgames
//SUBSYSTEM   : GameTypes
//REFERENCES  :
//TODO        :
//BUGS        :
//REVISION    : $Revision$
//AUTHOR      : Markus Schwab
//CREATED     : 28.04.2005
//COPYRIGHT   : Copyright (C) 2005

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


#include "GameTypes.h"


//-----------------------------------------------------------------------------
/// Default constructor
//-----------------------------------------------------------------------------
GameTypes::GameTypes () {
   insert (std::make_pair ((int)ROVHULT, "Røvhult"));
   insert (std::make_pair ((int)TWOPART, "Twopart"));
   insert (std::make_pair ((int)HEARTS, "Hearts"));
   insert (std::make_pair ((int)BURACO, "Buraco"));
   insert (std::make_pair ((int)MACHIAVELLI, "Machiavelli"));
   insert (std::make_pair ((int)SGTMAYOR, "SgtMayor"));
}

//-----------------------------------------------------------------------------
/// Destructor
//-----------------------------------------------------------------------------
GameTypes::~GameTypes () {
}
