#ifndef CARDOPTIONS_H
#define CARDOPTIONS_H

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

#include <cardgames-cfg.h>

#include "CardCol.h"

#include <YGP/Entity.h>


/**Options handling the cards within the CardCol application
 */
class CardOptions : public YGP::Entity {
   friend class Settings;
   friend class CardgameAppl;
   friend class CardgameCollection;

 public:
   CardOptions ();
   virtual ~CardOptions ();

 private:
   CardOptions (const CardOptions& other);
   const CardOptions& operator= (const CardOptions& other);

   std::string  decks;         // %attrib%; Front;   CARDDECKS_DIR "cards-default/"
   std::string  back;          // %attrib%; Back;    CARDDECKS_DIR "decks/deck1.png"
};

#endif
