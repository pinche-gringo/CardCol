#ifndef MACHIPILE_H
#define MACHIPILE_H

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


#include <CardPile.h>


/**Class for piles in the Machiavelli cardgame
 */
class MachiPile : public CardHPile {
 public:
   MachiPile ();
   virtual ~MachiPile ();

   virtual void setTopCard (CardWidget& newCard);
   void setTopCard (CardWidget& newCard, bool visible) {
      CardHPile::setTopCard (newCard, visible); }

   virtual void insert (CardWidget& card, unsigned int pos);

   virtual CardWidget& remove (CardWidget& card);
   CardWidget& remove (CardWidget& card, bool visible);
   virtual CardWidget& remove (unsigned int pos);
   CardWidget& remove (unsigned int pos, bool visible);

   unsigned int getPosition4Card (const CardWidget& card) const;

   typedef enum { ACE, BOTH, ONE } ACEFLAG;
   static int cardDistance (const CardWidget& a, const CardWidget& b) {
       return cardDistance (a, b, BOTH); }
   static int cardDistance (const CardWidget& a, const CardWidget& b,
                            ACEFLAG aceIsOne);

   void checkIntegrity () throw (Glib::ustring);

 private:
   MachiPile (const MachiPile& other);
   const MachiPile& operator= (const MachiPile& other);

   void analyzePile ();

   enum { UNDEFINED, NUMBER, COLOUR } type;
};

#endif
