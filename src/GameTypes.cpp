//PROJECT     : Cardgames
//SUBSYSTEM   : GameTypes
//REFERENCES  :
//TODO        :
//BUGS        :
//AUTHOR      : Markus Schwab
//CREATED     : 28.04.2005
//COPYRIGHT   : Copyright (C) 2005 - 2018

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

#include <glibmm/convert.h>
#include <glibmm/ustring.h>

#include "GameTypes.h"


GameTypes* GameTypes::instance(NULL);


//-----------------------------------------------------------------------------
/// Default constructor
//-----------------------------------------------------------------------------
GameTypes::GameTypes() {
#ifdef WITH_ROVHULT
   Glib::ustring rovhult(_("Rovhult"));
   if (rovhult == "Rovhult") {
      try {
	 rovhult = Glib::locale_to_utf8("Røvhult");
      }
      catch (Glib::Error&) {}
   }

   insert(std::make_pair((int)ROVHULT, rovhult));
#endif
#ifdef WITH_TWOPART
   insert(std::make_pair((int)TWOPART, _("Twopart")));
#endif
#ifdef WITH_HEARTS
   insert(std::make_pair((int)HEARTS, _("Hearts")));
#endif
#ifdef WITH_JABBERWOCKY
   insert(std::make_pair((int)JABBERWOCKY, _("Jabberwocky")));
#endif
#ifdef WITH_BURACO
   insert(std::make_pair((int)BURACO, _("Buraco")));
#endif
#ifdef WITH_MACHIAVELLI
   insert(std::make_pair((int)MACHIAVELLI, _("Machiavelli")));
#endif
#ifdef WITH_SGTMAYOR
   insert(std::make_pair((int)SGTMAYOR, _("SgtMayor")));
#endif
}

//-----------------------------------------------------------------------------
/// Destructor
//-----------------------------------------------------------------------------
GameTypes::~GameTypes() {
}
