#ifndef CARDSET_H
#define CARDSET_H

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
#include <vector.h>

#include <CardWidget.h>

// Class to display a card on the screen
class CardSet {
 public:
   typedef enum { NORMAL } CardSets;

   CardSet () { }
   ~CardSet ();

   CardWidget* getCard (unsigned int nr) const;

   void load (const Gdk_Window& parent, CardSets set = NORMAL) throw (std::string);

   unsigned int getCardNumber () const { return CARDS; }

 private:
   vector<CardWidget*> cards;

   static const unsigned int CARDS = 52;  // Number of cards in traditional set
};

#endif
