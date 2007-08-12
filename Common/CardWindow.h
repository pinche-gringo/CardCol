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


#include <vector>

#include "CardPile.h"

#include "AnimWindow.h"


class CardWidget;


/**Baseclass for animated windows in the cardgame collection
 */
class AnimatedCard : public AnimatedWindow {
 public:
   ~AnimatedCard ();

   /// Signal emitted, when the animation is finished
   sigc::signal<void> sigAnimation;

   void getEndPos (int& x, int& y);
   void start ();
   void finish ();

 protected:
   AnimatedCard (ICardPile& dest, unsigned int posDest, ICardPile& src, unsigned int posSrc);

   ICardPile& dest;
   unsigned int posDest;

   ICardPile& src;
   unsigned int posSrc;

 private:
   AnimatedCard ();
   AnimatedCard (const AnimatedCard&);
   AnimatedCard& operator= (const AnimatedCard&);

   /// Emits the signal; only used when emitting the signal is delayed after
   /// update of the GUI
   void emitSigAnimation () const;
};


/**Window holding exactly one card. This window can be used to animate a
 * card or fully show it if its put in a pile.
 */
class CardWindow : public AnimatedCard {
 public:
   ~CardWindow ();

   static CardWindow* create (ICardPile& dest, unsigned int posDest, ICardPile& src, unsigned int posSrc);

   /// Returns the card to be animated
   /// \returns CardWidget&: Card to be animated
   CardWidget& getCard () const { return *(CardWidget*)*get_children ().begin (); }

   virtual void start ();
   virtual void finish ();

 protected:
   CardWindow (ICardPile& dest, unsigned int posDest, ICardPile& src, unsigned int posSrc);

 private:
   CardWindow ();
   CardWindow (const CardWindow&);
   CardWindow& operator= (const CardWindow&);
};


/**Window holding a pile of cards, which can be used for animation.
 */
class CardPileWindow : public AnimatedCard {
 public:
   ~CardPileWindow ();

   static CardPileWindow* create (ICardPile& dest, unsigned int posDest,
				  ICardPile& src, unsigned int start, unsigned int end);

   /// Returns the pile of card to be animated
   /// \returns ICardPile&: Pile of cards
   ICardPile& getPile () const { return *pile; }

   virtual void start ();
   virtual void finish ();

 protected:
   CardPileWindow (ICardPile& dest, unsigned int posDest, ICardPile& src,
		   unsigned int start, unsigned int end);

   static ICardPile* setPile (Gtk::Window* win, const ICardPile& dest, unsigned int cards);

   ICardPile* pile;
   unsigned int end;

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
   virtual void start ();
   virtual void finish ();

   void addWindow (ICardPile& src, unsigned int start, unsigned int end);
   void addWindow (unsigned int posDest, ICardPile& src, unsigned int start, unsigned int end);

 protected:
   CardPileWindows (ICardPile& dest, unsigned int posDest,
		    ICardPile& src, unsigned int start, unsigned int end);

 private:
   CardPileWindows (const CardPileWindows&);
   CardPileWindows& operator= (const CardPileWindows&);

   struct AnimatedPile : public AnimatedWindow {
      AnimatedPile () : AnimatedWindow (), pile (NULL), src (NULL), start (0),
	   end (0), posDest (0) { }
      ~AnimatedPile () { delete pile; }

      ICardPile* pile;
      ICardPile* src;
      unsigned int start;
      unsigned int end;
      unsigned int posDest;

      void animateTo (int x, int y) { AnimatedWindow::animateTo (x, y); }
      void getEndPos (int& x, int& y);
   };

   std::vector<AnimatedPile*> wins;
};


#endif
