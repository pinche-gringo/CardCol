#ifndef OPTIONS_H
#define OPTIONS_H

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
