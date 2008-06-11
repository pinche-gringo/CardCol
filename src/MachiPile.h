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
// along with libYGP.  If not, see <http://www.gnu.org/licenses/>.


#include <stdexcept>

#include <CardPile.h>


/**Class for piles in the Machiavelli cardgame
 */
class MachiPile : public CardHPile {
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

   virtual void setTopCard (CardWidget& newCard);
   void setTopCard (CardWidget& newCard, bool visible) {
      CardHPile::setTopCard (newCard, visible); }

   virtual unsigned int insert (CardWidget& card, unsigned int pos);

   virtual CardWidget& remove (CardWidget& card);
   CardWidget& remove (CardWidget& card, bool visible);
   virtual CardWidget& remove (unsigned int pos);
   CardWidget& remove (unsigned int pos, bool visible);

   unsigned int getPosition4Card (const CardWidget& card) const;
   bool hasMatching3rd (ICardPile& pair, MachiPile::const_iterator& match,
                        unsigned int& nr) const;

   TYPE getType () const { return type; }

   int getPosOfColour (CardWidget::COLOURS colour) const;

   typedef enum { ACE, BOTH, ONE } ACEFLAG;
   static int cardDistance (const CardWidget& a, const CardWidget& b) {
       return cardDistance (a, b, BOTH); }
   static int cardDistance (const CardWidget& a, const CardWidget& b,
                            ACEFLAG aceIsOne);

   void checkIntegrity () throw (PileError);

 private:
   MachiPile (const MachiPile& other);
   const MachiPile& operator= (const MachiPile& other);

   void analyzePile ();

   TYPE type;
};

#endif
