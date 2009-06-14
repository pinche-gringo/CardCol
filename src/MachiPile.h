#ifndef MACHIPILE_H
#define MACHIPILE_H

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


#include <stdexcept>

#include <card/Pile.h>


/**Class for piles in the Machiavelli cardgame
 */
class MachiPile : public Card::HPile {
 public:
   /**Exception indicating an error in the pile
    */
   class PileError : public std::runtime_error {
   public:
      PileError (const std::string& what) : std::runtime_error (what) { }
   };


   typedef enum { UNDEFINED, NUMBER, COLOUR } TYPE;

   MachiPile ();
   virtual ~MachiPile ();

   virtual void setTopCard (Card::Widget& newCard);
   void setTopCard (Card::Widget& newCard, bool visible) {
      Card::HPile::setTopCard (newCard, visible); }

   virtual unsigned int insert (Card::Widget& card, unsigned int pos);

   virtual Card::Widget& remove (Card::Widget& card);
   Card::Widget& remove (Card::Widget& card, bool visible);
   virtual Card::Widget& remove (unsigned int pos);
   Card::Widget& remove (unsigned int pos, bool visible);

   unsigned int getPosition4Card (const Card::Widget& card) const;
   bool hasMatching3rd (std::vector<Card::Widget*>& pair, MachiPile::const_iterator& match,
			unsigned int& nr) const;

   TYPE getType () const { return type; }

   int getPosOfColour (Card::Widget::COLOURS colour) const;

   typedef enum { ACE, BOTH, ONE } ACEFLAG;
   static int cardDistance (const Card::Widget& a, const Card::Widget& b) {
       return cardDistance (a, b, BOTH); }
   static int cardDistance (const Card::Widget& a, const Card::Widget& b,
                            ACEFLAG aceIsOne);

   void checkIntegrity () throw (PileError);

 private:
   MachiPile (const MachiPile& other);
   const MachiPile& operator= (const MachiPile& other);

   void analyzePile ();

   TYPE type;
};

#endif
