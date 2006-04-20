#ifndef GAMETYPES_H
#define GAMETYPES_H

//$Id$

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

#include <YGP/MetaEnum.h>


/**Class to store the different game-types; both as string and as (unique)
   numeric representation
*/
class GameTypes : public YGP::MetaEnum {
 public:
   enum {
#ifdef WITH_BURACO
	  BURACO,
#endif
#ifdef WITH_HEARTS
	  HEARTS,
#endif
#ifdef WITH_MACHIAVELLI
	  MACHIAVELLI,
#endif
#ifdef WITH_ROVHULT
	  ROVHULT,
#endif
#ifdef WITH_SGTMAYOR
	  SGTMAYOR,
#endif
#ifdef WITH_TWOPART
	  TWOPART,
#endif
	  LAST,
	  NONE = -1 };

   GameTypes ();
   ~GameTypes ();

 private:
   GameTypes (const GameTypes& other);

   const GameTypes& operator= (const GameTypes& other);
};

#endif
