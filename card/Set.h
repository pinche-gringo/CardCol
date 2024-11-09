#ifndef CARDSET_H
#define CARDSET_H

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


namespace Card {

class Widget;
class Images;


/**Class to hold the cards used in a game
*/
class Set {
 public:
   Set() : cards_() { }
   Set(const Images& decks): cards_() { addPacket(decks); }
   ~Set();

   Widget& getCard(unsigned int nrCard) const;
   const std::vector<Widget*>& getCards() const { return cards_; }
   unsigned int size() const { return cards_.size(); }

   void shuffle();
   void set(unsigned int pos, unsigned int nrCard);

   void clear();
   void addPacket(const Images& decks);
   void update() const;

 private:
   std::vector<Widget*> cards_;
};

}

#endif
