#ifndef CARDVALUE_H
#define CARDVALUE_H

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


#include <YGP/MetaEnum.h>


/**Storing the possible values a card can have as text
 */
class CardValue : public YGP::MetaEnum {
 public:
   /// Creates a meta-enum of type CardValue
   /// \returns const CardValue&: Instance of CardValue
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
