#ifndef BURACOCARDS_H
#define BURACOCARDS_H

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


#include <YGP/MetaEnum.h>


/**Storing the possible values a card can have as text
 */
class BuracoCards : public YGP::MetaEnum {
 public:
   /// Creates a meta-enum of type BuracoCards
   /// \returns GamesTypes&: Instance of BuracoCards
   static const BuracoCards& get () {
      if (!instance)
	 instance = new BuracoCards;
      return *instance;
   }
   ~BuracoCards ();

 private:
   BuracoCards ();
   BuracoCards (const BuracoCards& other);
   const BuracoCards& operator= (const BuracoCards& other);

   static BuracoCards* instance;
};

#endif
