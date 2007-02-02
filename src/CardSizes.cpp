//$Id$

//PROJECT     : Cardgames
//SUBSYSTEM   : Settings
//REFERENCES  :
//TODO        :
//BUGS        :
//REVISION    : $Revision$
//AUTHOR      : Markus Schwab
//CREATED     : 02.02.2007
//COPYRIGHT   : Copyright (C) 2007

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

#include "CardSizes.h"


CardSizes* CardSizes::instance (NULL);


const unsigned int CardSizes::WIDTH_TINY (66);
const unsigned int CardSizes::HEIGHT_TINY (88);
const unsigned int CardSizes::WIDTH_SMALL (69);
const unsigned int CardSizes::HEIGHT_SMALL (92);
const unsigned int CardSizes::WIDTH_NORMAL (72);
const unsigned int CardSizes::HEIGHT_NORMAL (96);
const unsigned int CardSizes::WIDTH_BIG (75);
const unsigned int CardSizes::HEIGHT_BIG (100);


//-----------------------------------------------------------------------------
/// Default constructor
//-----------------------------------------------------------------------------
CardSizes::CardSizes () {
   insert (std::make_pair ((int)TINY, _("Tiny")));
   insert (std::make_pair ((int)SMALL, _("Small")));
   insert (std::make_pair ((int)NORMAL, _("Normal")));
   insert (std::make_pair ((int)BIG, _("Big")));
}

//-----------------------------------------------------------------------------
/// Destructor
//-----------------------------------------------------------------------------
CardSizes::~CardSizes () {
}


//-----------------------------------------------------------------------------
/// Returns the size (as enum-value) which corresponds to the passes values
/// \param width: Width to check
/// \param height: Height to check
/// \returns SIZE: Corresponding size
//-----------------------------------------------------------------------------
CardSizes::SIZES CardSizes::getSize (unsigned int width, unsigned int height) {
   width += height;
   if (width <= (WIDTH_TINY + HEIGHT_TINY))
      return TINY;
   else if (width <= (WIDTH_SMALL + HEIGHT_SMALL))
      return SMALL;
   else if (width <= (WIDTH_NORMAL + HEIGHT_NORMAL))
      return NORMAL;
   return BIG;
}

//-----------------------------------------------------------------------------
/// Returns the width corresponding to the passed size
/// \param size: Size
/// \returns unsigned int: Width corresponding to size
//-----------------------------------------------------------------------------
unsigned int CardSizes::getWidth (SIZES size) {
   return WIDTH_TINY + size * 3;
}

//-----------------------------------------------------------------------------
/// Returns the height corresponding to the passed size
/// \param size: Size
/// \returns unsigned int: Height corresponding to size
//-----------------------------------------------------------------------------
unsigned int CardSizes::getHeight (SIZES size) {
   return HEIGHT_TINY + (size << 2);
}
