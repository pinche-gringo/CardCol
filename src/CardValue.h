#ifndef CARDVALUE_H
#define CARDVALUE_H

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
class CardValue : public YGP::MetaEnum {
 public:
   /// Creates a meta-enum of type CardValue
   /// \returns GamesTypes&: Instance of CardValue
   static const CardValue& get () {
      if (!instance)
	 instance = new CardValue;
      return *instance;
   }
   ~CardValue ();

 private:
   CardValue ();
   CardValue (const CardValue& other);
   const CardValue& operator= (const CardValue& other);

   static CardValue* instance;
};

#endif
