#ifndef BURACOPILE_H
#define BURACOPILE_H

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

/**Class for piles in the Buraco cardgame
 */
class BuracoPile : public CardVPile {
 public:
   BuracoPile ();
   virtual ~BuracoPile ();

   virtual void setTopCard (CardWidget& newCard);
   void setTopCard (CardWidget& newCard, bool visible) {
      CardVPile::setTopCard (newCard, visible); }

   virtual void insert (CardWidget& card, unsigned int pos);

   virtual CardWidget& remove (CardWidget& card);
   CardWidget& remove (CardWidget& card, bool visible);
   virtual CardWidget& remove (unsigned int pos);
   CardWidget& remove (unsigned int pos, bool visible);

   unsigned int getCardPoints () const;
   unsigned int getPotentialPoints () const { return status.points; }
   int getPoints () const {
      return ((size () == 7) ? status.points : ((status.points >= 1000)
                                                ? -1000 : 0)); }

   unsigned int getPosJoker () const { return status.posJoker; }
   unsigned int getPosFirst () const { return status.posFirst; }
   unsigned int getPosLast () const { return status.posLast; }

   bool getPosition4Card (const CardWidget& card, unsigned int& pos,
                          unsigned int& move) const;

 protected:
   bool isValid (const CardWidget& card) const {
      unsigned int pos, move;
      return getPosition4Card (card, pos, move); }
   void analyzePile ();

 private:
   BuracoPile (const BuracoPile& other);
   const BuracoPile& operator= (const BuracoPile& other);

   enum { UNDEFINED, NUMBER, COLOUR };

   struct {
      unsigned int posFirst : 3;
      unsigned int posLast  : 3;
      unsigned int posJoker : 3;
      unsigned int type     : 2;
      unsigned int points   : 11;
   } status;
};

#endif
