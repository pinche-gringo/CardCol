#ifndef CARDWINDOW_H
#define CARDWINDOW_H

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


#include <vector>

#include "CardPile.h"

#include <YGP/Check.h>

#include <XGP/AnimWindow.h>


class CardWidget;


/**Baseclass for animated windows in the cardgame collection
 */
class AnimatedCard : public XGP::AnimatedWindow {
 public:
   ~AnimatedCard ();

   /// Signal emitted, when the animation is finished
   sigc::signal<void> sigAnimation;

   void getEndPos (int& x, int& y);
   void start ();
   void cleanup ();
   void finish ();

 protected:
   AnimatedCard (ICardPile& dest, unsigned int posDest, Gtk::Widget& src);

   ICardPile& dest;
   unsigned int posDest;

 private:
   AnimatedCard ();
   AnimatedCard (const AnimatedCard&);
   AnimatedCard& operator= (const AnimatedCard&);
};


/**Window holding exactly one card. This window can be used to animate a
 * card or fully show it if its put in a pile.
 */
class CardWindow : public AnimatedCard {
 public:
   ~CardWindow ();

   static CardWindow* create (ICardPile& dest, unsigned int posDest, ICardPile& src, unsigned int posSrc);

   void start ();
   void cleanup ();

 protected:
   CardWindow (ICardPile& dest, unsigned int posDest, ICardPile& src, unsigned int posSrc);

   ICardPile& src;
   unsigned int posSrc;

 private:
   CardWindow ();
   CardWindow (const CardWindow&);
   CardWindow& operator= (const CardWindow&);
};


/**Window holding a pile of cards, which can be used for animation.
 */
class CardPileWindow : public CardWindow {
 public:
   ~CardPileWindow ();

   /// Creates a CardPileWindow object
   /// \param dest: Destination pile
   /// \param posDest: Where to put the card in the destination
   /// \param src: Source pile; should be a CardPile<T>
   /// \param start: First card of source to move
   /// \param end: Last card of source to move
   /// \returns CardPileWindow*: Created window to animate
   /// \pre: The first card must be shown somewhere (to get its position)
   static CardPileWindow* create (ICardPile& dest, unsigned int posDest,
				  ICardPile& src, unsigned int start, unsigned int end) {
      Check1 (src.getWidget ());
      Check1 (dynamic_cast<Gtk::Box*> (src.getWidget ()));
      return new CardPileWindow (dest, posDest, src, start, end);
   }

   void cleanup ();
   void getEndPos (int& x, int& y);

 protected:
   CardPileWindow (ICardPile& dest, unsigned int posDest, ICardPile& src,
		   unsigned int start, unsigned int end);

   unsigned int last;

 private:
   CardPileWindow ();
   CardPileWindow (const CardPileWindow&);
   CardPileWindow& operator= (const CardPileWindow&);
};


/**Window holding a pile of cards, which can be used for animation.
 */
class CardPileWindows : public CardPileWindow {
 public:
   ~CardPileWindows ();

   static CardPileWindows* create (ICardPile& dest, unsigned int posDest,
				   ICardPile& src, unsigned int start, unsigned int end);

   void getEndPos (int& x, int& y);
   void cleanup ();

   void addWindow (ICardPile& src, unsigned int start, unsigned int end);
   void addWindow (unsigned int posDest, ICardPile& src, unsigned int start, unsigned int end);

 protected:
   CardPileWindows (ICardPile& dest, unsigned int posDest,
		    ICardPile& src, unsigned int start, unsigned int end);

 private:
   CardPileWindows (const CardPileWindows&);
   CardPileWindows& operator= (const CardPileWindows&);

   struct AnimatedPile : public XGP::AnimatedWindow {
      AnimatedPile (ICardPile& src, unsigned int start, unsigned int end);
      ~AnimatedPile () { }

      ICardPile&    source;
      unsigned int  start, end;
      unsigned int  posDest;                ///< Target position in destination

      void getEndPos (int& x, int& y);
      void animateTo (int x, int y);
   };

   std::vector<AnimatedPile*> wins;
};


#endif
