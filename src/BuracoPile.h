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

class BuracoPile : public CardVPile {
 public:
   BuracoPile (PileStyle style = NORMAL, ShowOpt show = DONT_CHANGE);
   virtual ~BuracoPile ();

   virtual void setTopCard (CardWidget& newCard);
   void setTopCard (CardWidget& newCard, bool visible) {
      CardVPile::setTopCard (newCard, visible); }

   virtual void insert (CardWidget& card, unsigned int pos);

   unsigned int getCardPoints () const;
   unsigned int getPotentialPoints () const { return points; }
   unsigned int getPoints () const {
      return (size () == 7 ? points : points >= 1000 ? -1000 : 0); }

   unsigned int getPosJoker () const { return status.posJoker; }
   unsigned int getPosFirst () const { return status.posFirst; }
   unsigned int getPosLast () const { return status.posLast; }

   bool getPosition4Card (const CardWidget& card, unsigned int& pos,
                          int& move) const;

 protected:
   bool isValid (const CardWidget& card) const {
      unsigned int pos;
      int move;
      return getPosition4Card (card, pos, move); }
   void analyzePile ();

 private:
   BuracoPile (const BuracoPile& other);
   const BuracoPile& operator= (const BuracoPile& other);

   enum { UNDEFINED, NUMBER, COLOUR };

   unsigned int points;
   struct {
      unsigned int posFirst : 3;
      unsigned int posLast  : 3;
      unsigned int posJoker : 3;
      unsigned int type     : 2;
   } status;
};

#endif
