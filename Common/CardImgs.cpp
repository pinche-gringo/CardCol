//$Id$

//PROJECT     : Cardgames
//SUBSYSTEM   : Common
//REFERENCES  :
//TODO        :
//BUGS        :
//REVISION    : $Revision$
//AUTHOR      : Markus Schwab
//CREATED     : 29.03.2002
//COPYRIGHT   : Anticopyright (A) 2002

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


#define DEBUG 9
#include <Check.h>
#include <Trace_.h>

#include <File.h>
#include <ANumeric.h>

#include "CardSet.h"


/*--------------------------------------------------------------------------*/
//Purpose   : Destructor
/*--------------------------------------------------------------------------*/
CardSet::~CardSet () {
   TRACE9 ("CardSet::~CardSet ()");

   for (int i = CARDS; i > 0;) {
      delete cards[--i];
      cards[i] = NULL;
   }
}


/*--------------------------------------------------------------------------*/
//Purpose   : Retrieves the specified cardnumber
//Parameters: nr: Number of card to retrieve
/*--------------------------------------------------------------------------*/
CardWidget* CardSet::getCard (unsigned int nr) const {
   Check3 (nr < CARDS);

   return cards[nr];
}

/*--------------------------------------------------------------------------*/
//Purpose   : Constructor; adds all controls to the dialog
//Parameters: parent: Parent window
//            set: Specifier for type of cardset
/*--------------------------------------------------------------------------*/
void CardSet::load (const Gdk_Window& parent, CardSets set = NORMAL) throw (std::string) {
   std::string file (PKGDIR);
   if (file.empty ())
      file = ".";

   if (file[file.size () - 1] != File::DIRSEPERATOR)
      file += File::DIRSEPERATOR;

   std::string temp;
   ANumeric nr;
   for (int i = 0; i < CARDS; ++i) {
      nr = i + 1;
      temp = file + nr.toUnformatedString () + ".xpm";

      cards.push_back (new CardWidget (parent, temp));
   }
}
