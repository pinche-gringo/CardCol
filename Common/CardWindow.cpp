//$Id$

//PROJECT     : Cardgames
//SUBSYSTEM   : libCard
//REFERENCES  :
//TODO        :
//BUGS        :
//REVISION    : $Revision$
//AUTHOR      : Markus Schwab
//CREATED     : 20.05.2007
//COPYRIGHT   : Copyright (C) 2007, 2008

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
// along with CardCol.  If not, see <http://www.gnu.org/licenses/>.


#include <YGP/Trace.h>

#include "CardPile.h"
#include "CardWidget.h"

#include "CardWindow.h"


//-----------------------------------------------------------------------------
/// Constructor
/// \param dest: Destination pile
/// \param posDest: Where to put the card in the destination
/// \param src: Source card
//-----------------------------------------------------------------------------
AnimatedCard::AnimatedCard (ICardPile& dest, unsigned int posDest, Gtk::Widget& src)
   : XGP::AnimatedWindow (src.get_window ()), dest (dest), posDest (posDest) {
   TRACE9 ("AnimatedCard::AnimatedCard (ICardPile&, unsigned int, Gtk::Widget&) - " << posDest);
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
      TRACE8 ("AnimatedCard::start () - Adding empty card");
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
   TRACE5 ("AnimatedCard::cleanup () - " << (int)posDest << '/' << dest.size ());
   if (posDest == -1U) {
      TRACE8 ("AnimatedCard::cleanup () - Removing empty card");
      Check1 (dest.size () == 1);
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
   CardWidget* widget ((posDest == -1U) ? dest.at (0)
		       : dest.at ((posDest >= dest.size ()) ? posDest - 1 : posDest));
   Check2 (widget); Check2 (widget->get_window ()); Check2 (win);

   widget->get_window ()->get_position (x, y);
   TRACE9 ("AnimatedCard::getEndPos (2x int&) - " << (int)posDest << " Dest: " << x << '/' << y);
}


//-----------------------------------------------------------------------------
/// Constructor
/// \param dest: Destination pile
/// \param posDest: Where to put the card in the destination
/// \param card: Card to show
/// \param card: Card to animate
//-----------------------------------------------------------------------------
CardWindow::CardWindow (ICardPile& dest, unsigned int posDest, ICardPile& src, unsigned int posSrc)
   : AnimatedCard (dest, posDest, *src.at (posSrc)), src (src), posSrc (posSrc) {
   TRACE9 ("CardWindow::CardWindow (2x(ICardPile&, unsigned int)) - " << posSrc << " -> " << posDest);
   Check1 (posSrc < src.size ());
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
/// \param src: Source card
/// \returns CardWindow*: Created window to animate
/// \pre The card must be shown (to get its position)
//-----------------------------------------------------------------------------
CardWindow* CardWindow::create (ICardPile& dest, unsigned int posDest, ICardPile& src, unsigned int posSrc) {
   Check1 (posSrc < src.size ());
   return new CardWindow (dest, posDest, src, posSrc);
}

//-----------------------------------------------------------------------------
/// Additional actions when starting the animation
//-----------------------------------------------------------------------------
void CardWindow::start () {
   TRACE8 ("CardWindow::start ()");
   AnimatedCard::start ();
   win->raise ();
   src.resize (posSrc, ICardPile::NORMAL);
}

//-----------------------------------------------------------------------------
/// Cleanup of the animation; moves the animated card to the distination pile
//-----------------------------------------------------------------------------
void CardWindow::cleanup () {
   TRACE8 ("CardWindow::cleanup () - " << posSrc << "->" << posDest);
   AnimatedCard::cleanup ();
   dest.insert (src.remove (posSrc), posDest);
   TRACE8 ("CardWindow::cleanup () - finish");
}


//-----------------------------------------------------------------------------
/// Constructor
/// \param win: Window
/// \param dest: Destination pile
/// \param posDest: Where to put the card in the destination
/// \param src: Source pile
/// \param start: First card to animate from source
/// \param end: Last card to animate from source
//-----------------------------------------------------------------------------
CardPileWindow::CardPileWindow (ICardPile& dest, unsigned int posDest,
				ICardPile& src, unsigned int start, unsigned int end)
   : CardWindow (dest, posDest, src, start), last (end) {
   TRACE8 ("CardPileWindow::CardPileWindow (...) - " << start << '/' << end);
   Check3 (src.size ()); Check3 (start <= end); Check3 (end < src.size ());
   Check1 (posDest <= dest.size ());
}

//-----------------------------------------------------------------------------
/// Destructor
//-----------------------------------------------------------------------------
CardPileWindow::~CardPileWindow () {
}


//-----------------------------------------------------------------------------
/// Moves to passed window to the passed coordinates
/// \param animWindow: Window to animate
/// \param x: X-coordinate
/// \param y: Y-coordinate
/// \returns bool: Always false
//-----------------------------------------------------------------------------
void CardPileWindow::getEndPos (int& x, int& y) {
   TRACE8 ("CardPileWindow::getEndPos (2x int&)");
   Check2 (src.size () > last);
   CardWindow::getEndPos (x, y);

   // Also move the remaining cards
   Glib::RefPtr<Gdk::Window> oldWin (win);
   for (unsigned int i (posSrc + 1); i <= last; ++i) {
      Check3 (src.at (i)->get_window ());
      win = src.at (i)->get_window (); Check3 (win);
      animateTo (x, y);
   }
   win = oldWin;
}

//-----------------------------------------------------------------------------
/// Additional actions when starting the animation
//-----------------------------------------------------------------------------
void CardPileWindow::start () {
   TRACE8 ("CardPileWindow::start ()");
   CardWindow::start ();
   for (unsigned int i (posSrc + 1); i <= last; ++i) {
      Check3 (src.at (i)->get_window ());
      src.at (i)->get_window ()->raise ();
   }
}

//-----------------------------------------------------------------------------
/// Cleanup of the animation; moves the animated cards to the distination pile
//-----------------------------------------------------------------------------
void CardPileWindow::cleanup () {
   TRACE8 ("CardPileWindow::cleanup () - " << posSrc << '/' << last << " -> " << (int)posDest);
   CardWindow::cleanup ();

   while (last-- > posSrc) {
      TRACE8 ("CardPileWindow::cleanup () - Move " << posSrc << '/' << last << " -> " << (int)posDest);
      dest.insert (src.remove (posSrc), ++posDest);
   }
   TRACE8 ("CardPileWindow::cleanup () - Finished");
}


//-----------------------------------------------------------------------------
/// Constructor
/// \param dest: Destination pile
/// \param posDest: Where to put the card in the destination
//-----------------------------------------------------------------------------
CardPileWindows::CardPileWindows (ICardPile& dest, unsigned int posDest,
				  ICardPile& src, unsigned int start, unsigned int end)
   : CardPileWindow (dest, posDest, src, start, end) {
   TRACE8 ("CardPileWindows::CardPileWindows (...) - " << start << '/' << end);
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
/// \param start: First card of source to animate
/// \param end: Last card of source to animate
/// \returns CardPileWindow*: Created window to animate
/// \pre: The first card must be shown somewhere (to get its position)
//-----------------------------------------------------------------------------
CardPileWindows* CardPileWindows::create (ICardPile& dest, unsigned int posDest,
					  ICardPile& src, unsigned int start, unsigned int end) {
   Check3 (src.size ()); Check3 (start <= end); Check3 (end < src.size ());
   Check1 (posDest <= dest.size ());
   return new CardPileWindows (dest, posDest, src, start, end);
}


//-----------------------------------------------------------------------------
/// Additional actions when starting the animation
//-----------------------------------------------------------------------------
void CardPileWindows::start () {
   TRACE8 ("CardPileWindow::start ()");
   CardPileWindow::start ();

   for (std::vector<AnimatedPile*>::iterator i (wins.begin ());
	i != wins.end (); ++i) {
      TRACE9 ("CardPileWindows::start () - Subwin: " << (i - wins.begin ()));
      Check3 (*i);
      (*i)->start ();
   }
}

//-----------------------------------------------------------------------------
/// Returns the position where to animate the card to
/// \param x: X-coordinate of destination
/// \param y: Y-coordinate of destination
//-----------------------------------------------------------------------------
void CardPileWindows::getEndPos (int& x, int& y) {
   CardPileWindow::getEndPos (x, y);

   for (std::vector<AnimatedPile*>::iterator i (wins.begin ());
	i != wins.end (); ++i) {
      TRACE9 ("CardPileWindows::getEndPos (2x int&) - Subwin: " << (i - wins.begin ()));
      Check3 (*i);
      (*i)->animateTo (x, y);
   }
}

//-----------------------------------------------------------------------------
/// Cleanup of the animation; moves the animated card to the distination pile
//-----------------------------------------------------------------------------
void CardPileWindows::cleanup () {
   TRACE5 ("CardPileWindows::cleanup ()");
   CardPileWindow::cleanup ();

   // Move the animated cards to their target; remove the animated widget
   for (std::vector<AnimatedPile*>::iterator i (wins.begin ());
	i != wins.end (); ++i) {
      Check3 ((*i)->posDest <= dest.size ());
      do
	 dest.insert ((*i)->source.remove ((*i)->first), (*i)->posDest++);
      while ((*i)->first < --(*i)->last);
   }
}

//-----------------------------------------------------------------------------
/// Adds a window to animate.
/// \param dest: Position in the destination
/// \param src: Source pile
/// \param start: First card of source to animate
/// \param end: Last card of source to animate
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
/// \param start: First card of source to animate
/// \param end: Last card of source to animate
//-----------------------------------------------------------------------------
void CardPileWindows::addWindow (ICardPile& src, unsigned int start, unsigned int end) {
   TRACE3 ("CardPileWindows::addWindow (ICardPile& src, 2x unsigned int) - " << start << '-' << end);
   Check1 (start <= end); Check1 (end < src.size ());

   AnimatedPile* win (new AnimatedPile (src, start, end)); Check3 (win);
   win->posDest = posDest;
   wins.push_back (win);
}


//-----------------------------------------------------------------------------
/// Default-ctr
/// \param src: Source pile
/// \param start: First card of source to animate
/// \param end: Last card of source to animate
//-----------------------------------------------------------------------------
CardPileWindows::AnimatedPile::AnimatedPile (ICardPile& src, unsigned int start, unsigned int end)
   : XGP::AnimatedWindow (src.at (start)->get_window ()),
     source (src), first (start), last (end), posDest (0) {
   TRACE9 ("CardPileWindows::AnimatedPile::AnimatedPile (ICardPile&, 2x unsigned int)");
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

//-----------------------------------------------------------------------------
/// Additional actions when starting the animation
//-----------------------------------------------------------------------------
void CardPileWindows::AnimatedPile::start () {
   TRACE1 ("CardPileWindows::AnimatedPile::start ()");
   for (unsigned int i (first + 1); i <= last; ++i) {
      Check3 (source.at (i)->get_window ());
      source.at (i)->get_window ()->raise ();
   }
}

//-----------------------------------------------------------------------------
/// Animates all specified cards in the source-pile to the passed coordinates
/// \param x: X-coordinate of destination
/// \param y: Y-coordinate of destination
//-----------------------------------------------------------------------------
void CardPileWindows::AnimatedPile::animateTo (int x, int y) {
   TRACE5 ("CardPileWindows::AnimatedPile::animateTo (2x int) - " << first << '/' << last
	   << " to " << x << '/' << y);
   XGP::AnimatedWindow::animateTo (x, y);

   // Also move the remaining cards
   Glib::RefPtr<Gdk::Window> oldWin (win);
   for (unsigned int i (first + 1); i <= last; ++i) {
      Check3 (source.at (i)->get_window ());
      win = source.at (i)->get_window (); Check3 (win);
      TRACE9 ("CardPileWindows::AnimatedPile::animateTo (2x int) - Visible " << win->is_visible ());
      if (win->is_visible ())
	 XGP::AnimatedWindow::animateTo (x, y);
   }
   win = oldWin;
}
