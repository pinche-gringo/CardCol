#ifndef CARDSIZES_H
#define CARDSIZES_H

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


/**Enumeration holding the supported card-sizes
 */
class CardSizes : public YGP::MetaEnum {
 public:
   typedef enum { TINY = 0, SMALL, NORMAL, BIG } SIZES;

   /// Creates a meta-enum of type CardSizes
   /// \returns CardSizes&: Instance of CardSizes
   static const CardSizes& get () {
      if (!instance)
         instance = new CardSizes;
      return *instance;
   };
   ~CardSizes ();

   static SIZES getSize (unsigned int width, unsigned int height);
   static unsigned int getWidth (SIZES size);
   static unsigned int getHeight (SIZES size);

   static const unsigned int WIDTH_TINY;
   static const unsigned int HEIGHT_TINY;
   static const unsigned int WIDTH_SMALL;
   static const unsigned int HEIGHT_SMALL;
   static const unsigned int WIDTH_NORMAL;
   static const unsigned int HEIGHT_NORMAL;
   static const unsigned int WIDTH_BIG;
   static const unsigned int HEIGHT_BIG;

 private:
   CardSizes ();
   CardSizes (const CardSizes& other);

   const CardSizes& operator= (const CardSizes& other);

   static CardSizes* instance;
};

#endif
