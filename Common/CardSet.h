#ifndef CARDSET_H
#define CARDSET_H

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
// along with CardCol.  If not, see <http://www.gnu.org/licenses/>.


#include <string>
#include <vector>


class CardWidget;
class CardImages;


/**Class to hold the cards used in a game
*/
class CardSet {
 public:
   CardSet () : cards_ () { }
   CardSet (const CardImages& decks)  : cards_ () { addPacket (decks); }
   ~CardSet ();

   CardWidget& getCard (unsigned int nrCard) const;
   const std::vector<CardWidget*>& getCards () const { return cards_; }
   unsigned int size () const { return cards_.size (); }

   void shuffle ();
   void set (unsigned int pos, unsigned int nrCard);

   void clear ();
   void addPacket (const CardImages& decks);
   void update () const;

 private:
   std::vector<CardWidget*> cards_;
};

#endif
