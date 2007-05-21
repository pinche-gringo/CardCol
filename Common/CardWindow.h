#ifndef CARDWINDOW_H
#define CARDWINDOW_H

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


#include "AnimWindow.h"


class ICardPile;
class CardWidget;


/**Information about the actual object to be animated
 */
class AnimData : public AnimatedObject {
 public:
   AnimData (ICardPile& dest, unsigned int posDest);

   virtual void getEndPos (int& x, int& y);
   virtual void finish ();

 private:
   ICardPile& dest;
   unsigned int posDest;
};



/**Window holding exactly one card. This window can be used to animate a
 * card or fully show it if its put in a pile.
 */
class CardWindow : public AnimatedWindow {
 public:
   ~CardWindow () { }

   static CardWindow* create (ICardPile& dest, unsigned int posDest, CardWidget& card);

   CardWidget& getCard () const { return *(CardWidget*)*get_children ().begin (); }

 protected:
   /**Information about the actual object to be animated
    */
   class AnimCard : public AnimData {
    public:
      /// Constructor
      /// \param dest: Destination pile
      /// \param posDest: Where to put the card in the destination
      /// \param card: Card to show
      AnimCard (ICardPile& dest, unsigned int posDest, CardWidget& card)
	 : AnimData (dest, posDest), card (card) { }

      CardWidget& getCard () const { return card; }

    private:
      CardWidget& card;
   };

   CardWindow (AnimCard* obj);

 private:
   CardWindow ();
   CardWindow (const CardWindow&);
   CardWindow& operator= (const CardWindow&);
};


/**Window holding a pile of cards, which can be used for animation.
 */
class CardPileWindow : public AnimatedWindow {
 public:
   ~CardPileWindow ();

   static CardPileWindow* create (ICardPile& dest, unsigned int posDest,
				  ICardPile& src, unsigned int start, unsigned int end);

   ICardPile& getPile () const { return *pile; }

 protected:
   /**Information about the actual object to be animated
    */
   class AnimPile : public AnimData {
    public:
      AnimPile (ICardPile& dest, unsigned int posDest, ICardPile& src);

      void start ();
      void finish ();

    private:
      ICardPile& src;
   };

   CardPileWindow (AnimPile* obj);

 private:
   CardPileWindow ();
   CardPileWindow (const CardPileWindow&);
   CardPileWindow& operator= (const CardPileWindow&);

   ICardPile* pile;
};


#endif
