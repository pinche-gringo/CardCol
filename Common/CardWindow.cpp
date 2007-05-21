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
/// \param card: Card to show
   : AnimatedWindow (), dest (dest), posDest (posDest), src (src), posSrc (posSrc) {
AnimData::AnimData (ICardPile& dest, unsigned int posDest) :
   dest (dest), posDest (posDest) {
   Check1 (posDest <= dest.size ());
/// Additional actions when starting the animation
      card->show ();
      dest.setTopCard (*card);
      posDest = -1U;
   }
      posDest = -1U;

//-----------------------------------------------------------------------------
/// Callback when the animated is finished

//-----------------------------------------------------------------------------
void AnimatedCard::finish () {
   TRACE5 ("AnimatedCard::finish () - " << posDest << '/' << dest.size ());
void AnimData::finish () {
   if (posDest == -1U) {
   }
      posDest = 0;
      (bind_return (mem_fun (*this, &AnimatedCard::emitSigAnimation), false));
//-----------------------------------------------------------------------------
/// Returns the position where to animate the card to

/// \param x: X-coordinate of destination
/// \param y: Y-coordinate of destination
//-----------------------------------------------------------------------------
void AnimatedCard::getEndPos (int& x, int& y) {
   Check2 (dest.size ());
void AnimData::getEndPos (int& x, int& y) {
   CardWidget* widget ((posDest == -1U) ? &dest.getTopCard () : dest.at ((posDest >= dest.size ()) ? posDest - 1 : posDest));
   widget->get_window ()->get_origin (x, y);
   TRACE9 ("AnimatedCard::getEndPos (2x int&) - Dest: " << x << '/' << y);
}
   TRACE9 ("AnimData::getEndPos (2x int&) - Dest: " << x << '/' << y);
//-----------------------------------------------------------------------------
/// Emits the signal; only used when emitting the signal is delayed after
/// Constructor
/// \param dest: Destination pile
/// \param posDest: Where to put the card in the destination
/// \param: obj: Animated object
			ICardPile& src, unsigned int posSrc)
CardWindow::CardWindow (AnimCard* obj) : AnimatedWindow (obj) {
   Check1 (obj);
   add (obj->getCard ());

//-----------------------------------------------------------------------------
/// Destructor
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
   AnimCard* obj (new AnimCard (dest, posDest, card));
   CardWindow* win (new CardWindow (obj));
   return win;
   move (x, y);
   card.get_window ()->get_origin (x, y);
   win->move (x, y);
   win->show ();

   return win;
//-----------------------------------------------------------------------------
/// Callback when the animated is finished

//-----------------------------------------------------------------------------
/// \param posDest: Where to put the card in the destination
/// \param: obj: Animated object
				unsigned int start, unsigned int end)
CardPileWindow::CardPileWindow (AnimPile* obj) : AnimatedWindow (obj), pile (NULL) {
   show_all_children ();
//-----------------------------------------------------------------------------
/// Destructor
//-----------------------------------------------------------------------------
CardPileWindow::~CardPileWindow () {
}

   delete pile;

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
   AnimPile* obj (new AnimPile (dest, posDest, *pile));
   CardPileWindow* win (new CardPileWindow (obj));
   win->add (*box);
   win->pile = pile;
   src.at (posSrc)->get_window ()->get_origin (x, y);
   move (x, y);
   src.at (start)->get_window ()->get_origin (x, y);
   win->move (x, y);
      pile->setTopCard (src.remove (posSrc));
   while (start <= end--)
      pile->setTopCard (src.remove (start));

   win->show ();
   return win;
//-----------------------------------------------------------------------------
/// Callback when the animated starts

//-----------------------------------------------------------------------------
/// Constructor
/// \param dest: Destination pile
/// \param posDest: Where to put the card in the destination
/// \param src: Source pile
   TRACE8 ("CardPileWindow::finish ()");
CardPileWindow::AnimPile::AnimPile (ICardPile& dest, unsigned int posDest, ICardPile& src)
   : AnimData (dest, posDest), src (src) {
}
   do {

//-----------------------------------------------------------------------------
/// Callback when the animated starts
//-----------------------------------------------------------------------------
void CardPileWindow::AnimPile::start () {
   AnimData::start ();
}

//-----------------------------------------------------------------------------
/// Callback when the animated is finished
//-----------------------------------------------------------------------------
void CardPileWindow::AnimPile::finish () {
   AnimData::finish ();

   TRACE1 ("CardPileWindows::AnimatedPile::getEndPos (2x int& x)");
   Check (0);
