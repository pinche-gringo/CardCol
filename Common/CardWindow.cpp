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
   : AnimatedWindow (), dest (dest), posDest (posDest), src (src), posSrc (posSrc) {
AnimatedCard::AnimatedCard (ICardPile& dest, unsigned int posDest)
   : AnimatedWindow (), dest (dest), pos (posDest) {

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
      card->show ();
      dest.setTopCard (*card);
      posDest = -1U;
   }
      pos = -1U;

//-----------------------------------------------------------------------------
/// Callback when the animated is finished
//-----------------------------------------------------------------------------
void AnimatedCard::finish () {
   TRACE5 ("AnimatedCard::finish () - " << posDest << '/' << dest.size ());
   if (posDest == -1U) {
   if (pos == -1U) {
   }
      pos = 0;
      (bind_return (mem_fun (*this, &AnimatedCard::emitSigAnimation), false));
//-----------------------------------------------------------------------------
/// Returns the position where to animate the card to
/// \param x: X-coordinate of destination
/// \param y: Y-coordinate of destination
//-----------------------------------------------------------------------------
void AnimatedCard::getEndPos (int& x, int& y) {
   Check2 (dest.size ());
   CardWidget* widget ((posDest == -1U) ? &dest.getTopCard ()
		       : dest.at ((posDest >= dest.size ()) ? posDest - 1 : posDest));
   CardWidget* widget ((pos == -1U) ? &dest.getTopCard () : dest.at ((pos >= dest.size ()) ? pos - 1 : pos));
   widget->get_window ()->get_origin (x, y);
   TRACE9 ("AnimatedCard::getEndPos (2x int&) - Dest: " << x << '/' << y);
}

//-----------------------------------------------------------------------------
/// Emits the signal; only used when emitting the signal is delayed after
/// Constructor
/// \param dest: Destination pile
/// \param posDest: Where to put the card in the destination
/// \param card: Card to show
/// \param src: Source pile
/// \param posSrc: Position of card in the source
			ICardPile& src, unsigned int posSrc)
CardWindow::CardWindow (ICardPile& dest, unsigned int posDest, CardWidget& card)
   : AnimatedCard (dest, posDest) {
   add (card);

   show ();
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
/// \param card: Card to show
//-----------------------------------------------------------------------------
CardWindow* CardWindow::create (ICardPile& dest, unsigned int posDest,
				ICardPile& src, unsigned int posSrc) {
CardWindow* CardWindow::create (ICardPile& dest, unsigned int posDest, CardWidget& card) {
   CardWindow* win (new CardWindow (dest, posDest, card));
   return win;
   move (x, y);
   card.get_window ()->get_origin (x, y);
   win->move (x, y);
   win->show ();

   return win;
//-----------------------------------------------------------------------------
/// Callback when the animated is finished

//-----------------------------------------------------------------------------
void CardWindow::finish () {
   TRACE8 ("CardWindow::finish ()");
   AnimatedCard::finish ();
   CardWidget& card (getCard ());
   Check2 (typeid (card) == typeid (CardWidget));
   remove ();
   dest.insert (card, posDest);
}
   dest.insert (card, pos);
   sigAnimation.emit ();

//-----------------------------------------------------------------------------
/// Constructor
/// \param dest: Destination pile
/// \param posDest: Where to put the card in the destination
/// \param src: Source pile
/// \param start: First card to animate from source
				unsigned int start, unsigned int end)
CardPileWindow::CardPileWindow (ICardPile& dest, unsigned int posDest)
   : AnimatedCard (dest, pos), pile (NULL) {
   show_all_children ();
   show ();
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
   Check3 (src.size ()); Check3 (start < end); Check3 (end < src.size ());
//-----------------------------------------------------------------------------
   ICardPile* pile;
   Gtk::Box*  box;
   if (typeid (dest) == typeid (CardHPile)) {
      box = tmp;
      tmp->set_size_request (CardImages::WIDTH + cards * tmp->getCompressedSize (), CardImages::HEIGHT);
   }
      CardVPile* tmp (new CardVPile);
      pile = tmp;
      box = tmp;
      tmp->set_size_request (CardImages::WIDTH, CardImages::HEIGHT + cards * tmp->getCompressedSize ());
   }
   pile->setStyle (dest.getStyle ());
   win->add (*box);
   return pile;
/// Callback when the animated is to be started
   CardPileWindow* win (new CardPileWindow (dest, posDest));
   win->add (*box);
   win->pile = pile;
   src.at (posSrc)->get_window ()->get_origin (x, y);
   move (x, y);
   src.at (start)->get_window ()->get_origin (x, y);
   win->move (x, y);
      pile->setTopCard (src.remove (posSrc));
   } while (posSrc < end--);
      pile->setTopCard (src.remove (start));
   } while (start < end--);

   return win;
//-----------------------------------------------------------------------------
/// Callback when the animated starts
//-----------------------------------------------------------------------------
void CardPileWindow::finish () {
   TRACE8 ("CardPileWindow::finish ()");
   AnimatedCard::finish ();
   Check2 (pile->size ());
   do {
      dest.insert (pile->remove (0), posDest++);
   } while (pile->size ());
      dest.insert (pile->remove (0), pos++);
}

   sigAnimation.emit ();


   TRACE1 ("CardPileWindows::AnimatedPile::getEndPos (2x int& x)");
   Check (0);
