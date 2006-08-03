//PROJECT     : Cardgames
//SUBSYSTEM   : BuracoCards
//REFERENCES  :
//TODO        :
//BUGS        :
//REVISION    : $Revision$
//AUTHOR      : Markus Schwab
//CREATED     : 03.08.2006
//COPYRIGHT   : Copyright (C) 2006

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


#include <cardgames-cfg.h>

#include "BuracoCards.h"


BuracoCards* BuracoCards::instance (NULL);


//-----------------------------------------------------------------------------
/// Default constructor; Fills the values
//-----------------------------------------------------------------------------
BuracoCards::BuracoCards () {
   insert (std::make_pair (11, "11"));
   insert (std::make_pair (13, "13"));
   insert (std::make_pair (15, "15"));
}

//-----------------------------------------------------------------------------
/// Destructor
//-----------------------------------------------------------------------------
BuracoCards::~BuracoCards () {
}
