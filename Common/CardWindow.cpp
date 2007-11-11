//$Id$

//PROJECT     : Cardgames
//SUBSYSTEM   : libCard
//REFERENCES  :
//TODO        :
//BUGS        :
//REVISION    : $Revision$
//AUTHOR      : Markus Schwab
//CREATED     : 20.05.2007
//COPYRIGHT   : Copyright (C) 2007

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


#include <YGP/Check.h>
#include <YGP/Trace.h>

#include "CardPile.h"
#include "CardWidget.h"

#include "CardWindow.h"


//-----------------------------------------------------------------------------
/// Constructor
/// \param dest: Destination pile
/// \param posDest: Where to put the card in the destination
/// \param src: Source pile
/// \param posSrc: Where to put the card in the destination
//-----------------------------------------------------------------------------
AnimatedCard::AnimatedCard (ICardPile& dest, unsigned int posDest, ICardPile& src, unsigned int posSrc)
   : AnimatedWindow (), dest (dest), posDest (posDest), src (src), posSrc (posSrc) {
}


//-----------------------------------------------------------------------------
/// Destructor
//-----------------------------------------------------------------------------
AnimatedCard::~AnimatedCard () {
}


//-----------------------------------------------------------------------------
/// Additional actions when starting the animation
//-----------------------------------------------------------------------------
void AnimatedCard::start () {
   TRACE5 ("AnimatedCard::start () - " << posDest << '/' << dest.size ());
   if (dest.empty ()) {
      CardWidget* card (CardWidget::getEmpty ());
      card->show ();
      dest.setTopCard (*card);
      posDest = -1U;
   }
}

//-----------------------------------------------------------------------------
/// Cleanup of the animation; moves the animated card to the distination pile
//-----------------------------------------------------------------------------
void AnimatedCard::cleanup () {
   TRACE5 ("AnimatedCard::cleanup () - " << posDest << '/' << dest.size ());
   if (posDest == -1U) {
      delete &dest.removeTopCard ();
      posDest = 0;
   }
}

//-----------------------------------------------------------------------------
/// Callback when the animation starts
//-----------------------------------------------------------------------------
void AnimatedCard::finish () {
   TRACE8 ("AnimatedCard::finish ()");
   sigAnimation.emit ();
}

//-----------------------------------------------------------------------------
/// Returns the position where to animate the card to
/// \param x: X-coordinate of destination
/// \param y: Y-coordinate of destination
//-----------------------------------------------------------------------------
void AnimatedCard::getEndPos (int& x, int& y) {
   Check2 (dest.size ());
   CardWidget* widget ((posDest == -1U) ? &dest.getTopCard ()
		       : dest.at ((posDest >= dest.size ()) ? posDest - 1 : posDest));
   Check2 (widget); Check2 (widget->get_window ());

   widget->get_window ()->get_origin (x, y);
   TRACE9 ("AnimatedCard::getEndPos (2x int&) - Dest: " << x << '/' << y);
}


//-----------------------------------------------------------------------------
/// Constructor
/// \param dest: Destination pile
/// \param posDest: Where to put the card in the destination
/// \param card: Card to show
/// \param src: Source pile
/// \param posSrc: Position of card in the source
//-----------------------------------------------------------------------------
CardWindow::CardWindow (ICardPile& dest, unsigned int posDest,
			ICardPile& src, unsigned int posSrc)
   : AnimatedCard (dest, posDest, src, posSrc) {
   show_all_children ();
}

//-----------------------------------------------------------------------------
/// Destructor
//-----------------------------------------------------------------------------
CardWindow::~CardWindow () {
}

//-----------------------------------------------------------------------------
/// Creates an CardWindow-object
/// \param dest: Destination pile
/// \param posDest: Where to put the card in the destination
/// \param src: Source pile
/// \param posSrc: Position of the card in the source
/// \returns CardWindow*: Created window to animate
/// \pre The card must be shown (to get its position)
//-----------------------------------------------------------------------------
CardWindow* CardWindow::create (ICardPile& dest, unsigned int posDest,
				ICardPile& src, unsigned int posSrc) {
   Check1 (posSrc < src.size ());
   Check1 (posDest <= dest.size ());

   CardWindow* win (new CardWindow (dest, posDest, src, posSrc));
   return win;
}


//-----------------------------------------------------------------------------
/// Callback when the animated is to be started
//-----------------------------------------------------------------------------
void CardWindow::start () {
   TRACE8 ("CardWindow::start ()");
   AnimatedCard::start ();

   int x, y;
   src.at (posSrc)->get_window ()->get_origin (x, y);
   move (x, y);
   add (src.remove (posSrc));
   show ();
}

//-----------------------------------------------------------------------------
/// Cleanup of the animation; moves the animated card to the distination pile
//-----------------------------------------------------------------------------
void CardWindow::cleanup () {
   TRACE8 ("CardWindow::cleanup ()");
   AnimatedCard::cleanup ();

   CardWidget& card (getCard ());
   Check2 (typeid (card) == typeid (CardWidget));
   remove ();
   dest.insert (card, posDest);
}


//-----------------------------------------------------------------------------
/// Constructor
/// \param dest: Destination pile
/// \param posDest: Where to put the card in the destination
/// \param src: Source pile
/// \param start: First card to animate from source
/// \param end: Last card to animate from source
//-----------------------------------------------------------------------------
CardPileWindow::CardPileWindow (ICardPile& dest, unsigned int posDest, ICardPile& src,
				unsigned int start, unsigned int end)
   : AnimatedCard (dest, posDest, src, start), pile (NULL), end (end) {
   TRACE8 ("CardPileWindow::CardPileWindow (...) - " << posSrc << '/' << end);
}

//-----------------------------------------------------------------------------
/// Destructor
//-----------------------------------------------------------------------------
CardPileWindow::~CardPileWindow () {
}


//-----------------------------------------------------------------------------
/// Creates a CardPileWindow object
/// \param dest: Destination pile
/// \param posDest: Where to put the card in the destination
/// \param src: Source pile
/// \param start: First card of source to move
/// \param end: Last card of source to move
/// \returns CardPileWindow*: Created window to animate
/// \pre: The first card must be shown somewhere (to get its position)
//-----------------------------------------------------------------------------
CardPileWindow* CardPileWindow::create (ICardPile& dest, unsigned int posDest,
					ICardPile& src, unsigned int start, unsigned int end) {
   TRACE8 ("CardPileWindow::create (...) - " << start << '/' << end);
   Check3 (src.size ()); Check3 (start <= end); Check3 (end < src.size ());
   Check1 (posDest <= dest.size ());

   CardPileWindow* win (new CardPileWindow (dest, posDest, src, start, end));
   win->pile = setPile (win, dest, end - start);
   return win;
}


//-----------------------------------------------------------------------------
/// Sets the pile to be animated
/// \param win: Window to animate
/// \param dest: Destination pile
/// \param cards: Number of cards in pile
/// \returns ICardPile: CardPile to be add to a window
//-----------------------------------------------------------------------------
ICardPile* CardPileWindow::setPile (Gtk::Window* win, const ICardPile& dest, unsigned int cards) {
   ICardPile* pile (NULL);
   Gtk::Box*  box (NULL);
   if (dynamic_cast<const CardHPile*> (&dest)) {
      CardHPile* tmp (new CardHPile);
      pile = tmp;
      box = tmp;
      tmp->set_size_request (CardImages::WIDTH + cards * tmp->getCompressedSize (), CardImages::HEIGHT);
   }
   else {
      CardVPile* tmp (new CardVPile);
      pile = tmp;
      box = tmp;
      tmp->set_size_request (CardImages::WIDTH, CardImages::HEIGHT + cards * tmp->getCompressedSize ());
   }
   box->show ();
   pile->setStyle (dest.getStyle ());
   pile->setShowOption (dest.getShowOption ());
   win->add (*box);
   return pile;
}

//-----------------------------------------------------------------------------
/// Callback when the animated is to be started
//-----------------------------------------------------------------------------
void CardPileWindow::start () {
   TRACE8 ("CardPileWindow::start () - " << posSrc << '/' << end);
   AnimatedCard::start ();

   int x, y;
   src.at (posSrc)->get_window ()->get_origin (x, y);
   move (x, y);

   do {
      pile->setTopCard (src.remove (posSrc));
   } while (posSrc < end--);

   pile->getSize (x, y);
   set_size_request (x, y);
   TRACE5 ("CardPileWindow::start () - Sizes: " << x << '/' << y);
   show ();
}

//-----------------------------------------------------------------------------
/// Cleanup of the animation; moves the animated cards to the distination pile
//-----------------------------------------------------------------------------
void CardPileWindow::cleanup () {
   TRACE8 ("CardPileWindow::cleanup ()");
   AnimatedCard::cleanup ();

   Check2 (pile->size ());
   do {
      dest.insert (pile->remove (0), posDest++);
   } while (pile->size ());
   delete pile;
}


//-----------------------------------------------------------------------------
/// Constructor
/// \param dest: Destination pile
/// \param posDest: Where to put the card in the destination
//-----------------------------------------------------------------------------
CardPileWindows::CardPileWindows (ICardPile& dest, unsigned int posDest,
				  ICardPile& src, unsigned int start, unsigned int end)
   : CardPileWindow (dest, posDest, src, start, end) {
}

//-----------------------------------------------------------------------------
/// Destructor
//-----------------------------------------------------------------------------
CardPileWindows::~CardPileWindows () {
   for (std::vector<AnimatedPile*>::iterator i (wins.begin ());
	i != wins.end (); ++i)
      delete *i;
}


//-----------------------------------------------------------------------------
/// Creates a CardPileWindow object
/// \param dest: Destination pile
/// \param posDest: Where to put the card in the destination
/// \param src: Source pile
/// \param start: First card of source to move
/// \param end: Last card of source to move
/// \returns CardPileWindow*: Created window to animate
/// \pre: The first card must be shown somewhere (to get its position)
//-----------------------------------------------------------------------------
CardPileWindows* CardPileWindows::create (ICardPile& dest, unsigned int posDest,
					  ICardPile& src, unsigned int start, unsigned int end) {
   Check3 (src.size ()); Check3 (start <= end); Check3 (end < src.size ());
   Check1 (posDest <= dest.size ());

   CardPileWindows* win (new CardPileWindows (dest, posDest, src, start, end));
   win->pile = setPile (win, dest, end - start);
   return win;
}


//-----------------------------------------------------------------------------
/// Returns the position where to animate the card to
/// \param x: X-coordinate of destination
/// \param y: Y-coordinate of destination
//-----------------------------------------------------------------------------
void CardPileWindows::getEndPos (int& x, int& y) {
   Check2 (dest.size ());
   CardWidget* widget ((posDest == -1U) ? &dest.getTopCard () : dest.at ((posDest >= dest.size ())
									 ? posDest - 1 : posDest));
   Check2 (widget); Check2 (widget->get_window ());

   widget->get_window ()->get_origin (x, y);
   TRACE9 ("CardPileWindows::getEndPos (2x int&) - Dest: " << x << '/' << y);

   for (std::vector<AnimatedPile*>::iterator i (wins.begin ());
	i != wins.end (); ++i) {
      TRACE9 ("CardPileWindows::getEndPos (2x int&) - Subwin: " << (i - wins.begin ()));
      Check3 (*i);
      (*i)->animateTo (x, y);
   }
}

//-----------------------------------------------------------------------------
/// Callback when the animation starts
//-----------------------------------------------------------------------------
void CardPileWindows::start () {
   TRACE5 ("CardPileWindows::start ()");
   CardPileWindow::start ();

   int x, y;
   for (std::vector<AnimatedPile*>::iterator i (wins.begin ());
	i != wins.end (); ++i) {
      (*i)->src->at ((*i)->start)->get_window ()->get_origin (x, y);
      (*i)->move (x, y);

      ICardPile* pile ((*i)->pile);
      pile->getSize (x, y);
      (*i)->set_size_request (x, y);
      TRACE5 ("CardPileWindow::start () - Sizes: " << x << '/' << y);

      do {
	 TRACE8 ("CardPileWindows::start () - Pile: " << (i - wins.begin ()));
	 pile->setTopCard ((*i)->src->remove ((*i)->start));
	 TRACE5 ("CardPileWindows::start () - Sizes: " << x << '/' << y);
      } while ((*i)->start < (*i)->end--);
      (*i)->show ();
   }
}

//-----------------------------------------------------------------------------
/// Cleanup of the animation; moves the animated card to the distination pile
//-----------------------------------------------------------------------------
void CardPileWindows::cleanup () {
   TRACE5 ("CardPileWindows::cleanup ()");
   CardPileWindow::cleanup ();

   for (std::vector<AnimatedPile*>::iterator i (wins.begin ());
	i != wins.end (); ++i) {
      ICardPile* pile ((*i)->pile);
      Check3 ((*i)->posDest <= dest.size ());
      do {
	 dest.insert (pile->remove (0), (*i)->posDest++);
      } while (pile->size ());
   }
}

//-----------------------------------------------------------------------------
/// Adds a window to animate.
/// \param dest: Position in the destination
/// \param src: Source pile
/// \param start: First card of source to move
/// \param end: Last card of source to move
//-----------------------------------------------------------------------------
void CardPileWindows::addWindow (unsigned int dest, ICardPile& src, unsigned int start, unsigned int end) {
   TRACE3 ("CardPileWindows::addWindow (unsigned int, ICardPile& src, 2x unsigned int)");
   Check1 (start <= end);
   Check1 (end < src.size ());
   addWindow (src, start, end);
   wins.back ()->posDest = dest;
}

//-----------------------------------------------------------------------------
/// Adds a window to animate.
/// \param src: Source pile
/// \param start: First card of source to move
/// \param end: Last card of source to move
//-----------------------------------------------------------------------------
void CardPileWindows::addWindow (ICardPile& src, unsigned int start, unsigned int end) {
   TRACE3 ("CardPileWindows::addWindow (ICardPile& src, 2x unsigned int) - " << start << '-' << end);
   Check1 (start <= end);
   Check1 (end < src.size ());
   AnimatedPile* win (new AnimatedPile);
   win->pile = setPile (win, dest, end - start);
   win->src = &src;
   win->start = start;
   win->posDest = posDest;
   wins.push_back (win);
}

//-----------------------------------------------------------------------------
/// Returns the position where to animate the card to
/// \param x: X-coordinate of destination
/// \param y: Y-coordinate of destination
//-----------------------------------------------------------------------------
void CardPileWindows::AnimatedPile::getEndPos (int& x, int& y) {
   TRACE1 ("CardPileWindows::AnimatedPile::getEndPos (2x int& x)");
   Check (0);
   x = y = 0;
}
