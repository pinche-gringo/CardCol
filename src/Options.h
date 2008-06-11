#ifndef OPTIONS_H
#define OPTIONS_H

//$Id$

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


#include <string>
#include <vector>

#include "CardOptions.h"

#include <YGP/Entity.h>


/**General options of the CardCol application
 */
class Options : public YGP::Entity {
   friend class Settings;
   friend class CardgameAppl;
   friend class CardgameCollection;

 public:
   Options ();
   virtual ~Options ();

 private:
   Options (const Options& other);
   const Options& operator= (const Options& other);

   std::string strType;        // %attrib%; Type;        "Rovhult"
   unsigned int type;
   std::string  browser;       // %attrib%; Helpbrowser; "galeon"
   std::string  helpPath;      // %attrib%; Helpdir;     DOCUDIR
   std::string  target;
   std::string  port;

   CardOptions  co;

#ifdef SAVE_GAME
   bool load;
   std::string gameFile;
#endif

   std::vector<Glib::ustring> names;
   const char*    pNameINIFile;
};

#endif
