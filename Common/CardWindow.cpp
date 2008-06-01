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


#define CHECK 9
#define TRACELEVEL 9
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
   src.resize (posSrc, ICardPile::NORMAL);
}

//-----------------------------------------------------------------------------
/// Cleanup of the animation; moves the animated card to the distination pile
//-----------------------------------------------------------------------------
void CardWindow::cleanup () {
   TRACE8 ("CardWindow::cleanup ()");
   AnimatedCard::cleanup ();

   dest.insert (src.remove (posSrc), posDest);
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
   : AnimatedCard (dest, posDest, *src.getWidget ()), animPile (),
     src (src), first (start), last (end) {
   TRACE8 ("CardPileWindow::CardPileWindow (...) - " << start << '/' << end);
   Check3 (src.size ()); Check3 (start <= end); Check3 (end < src.size ());
   Check1 (posDest <= dest.size ());

   animPile.set_size_request (CardImages::WIDTH + (end - start) * 10,
			      CardImages::HEIGHT);

   Check3 (src.getWidget ());
   src.getWidget ()->pack_start (animPile);
   animPile.show ();
   Check3 (animPile.get_window ());
   Check3 (animPile.get_window () != src.getWidget ()->get_window ());
   // moveCards (animPile, src, start, end);
}

//-----------------------------------------------------------------------------
/// Destructor
//-----------------------------------------------------------------------------
CardPileWindow::~CardPileWindow () {
}


//-----------------------------------------------------------------------------
/// Move the cards from its source to the animated pile
/// \param src: Source pile
/// \param start: First card to animate from source
/// \param end: Last card to animate from source
//-----------------------------------------------------------------------------
void CardPileWindow::moveCards (CardHPile& animPile, ICardPile& src,
				unsigned int first, unsigned int last) {
   TRACE8 ("CardPileWindow::moveCards () - " << first << '/' << last << " of " << src.size ());
   Check2 (first <= last); Check2 (last < src.size ());
   Check2 (src.at (first)->get_window ());

   int x, y;
   src.at (first)->get_window ()->get_origin (x, y);

   animPile.get_window ()->move (x, y);

   do
      animPile.setTopCard (src.remove (first));
   while (first < last--);

   TRACE5 ("CardPileWindow::moveCards () - Position: " << x << '/' << y);
}

//-----------------------------------------------------------------------------
/// Additional actions when starting the animation
//-----------------------------------------------------------------------------
void CardPileWindow::start () {
   TRACE8 ("CardPileWindow::start ()");
   Check1 (animPile.get_window ());
   win = animPile.get_window ();           // Set the window to animate (again)
   AnimatedCard::start ();

   Check3 (animPile.get_window ());
   Glib::RefPtr<Gdk::Pixmap> pixmap (Gdk::Pixmap::create (animPile.get_window (),
							  CardImages::WIDTH + (last - first) * 10,
							  CardImages::HEIGHT));

   Glib::RefPtr<Gdk::GC> gc (Gdk::GC::create (pixmap)); Check3 (gc);
   Glib::RefPtr<Gdk::Pixbuf> cardImg;

   unsigned int x (0);
   for (unsigned int i (first); i < last; ++i) {
      CardWidget& card (*src.at (i));
      cardImg = card.getImage (); Check3 (cardImg);
      pixmap->draw_pixbuf (gc, cardImg, 0, 0, x, 0, -1, 10, Gdk::RGB_DITHER_NONE, 0, 0);
      x += 10;
      card.hide ();
   }
   Check3 (src.at (last)); Check3 (src.at (last)->getImage ());
   pixmap->draw_pixbuf (gc, src.at (last)->getImage (), 0, 0, x, 0, -1, -1,
			Gdk::RGB_DITHER_NONE, 0, 0);
   Gtk::Image img (pixmap, Glib::RefPtr<Gdk::Bitmap> ());
   img.show ();
   animPile.add (img);
}

//-----------------------------------------------------------------------------
/// Cleanup of the animation; moves the animated cards to the distination pile
//-----------------------------------------------------------------------------
void CardPileWindow::cleanup () {
   TRACE8 ("CardPileWindow::cleanup ()");
   AnimatedCard::cleanup ();

   do {
      CardWidget& card (src.remove (first));
      card.show ();
      dest.insert (card, posDest++);
   } while (first < last--);
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
   return new CardPileWindows (dest, posDest, src, start, end);
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
/// Cleanup of the animation; moves the animated card to the distination pile
//-----------------------------------------------------------------------------
void CardPileWindows::cleanup () {
   TRACE5 ("CardPileWindows::cleanup ()");
   CardPileWindow::cleanup ();

   for (std::vector<AnimatedPile*>::iterator i (wins.begin ());
	i != wins.end (); ++i) {
      Check3 ((*i)->posDest <= dest.size ());
      do
	 dest.insert ((*i)->remove (0), (*i)->posDest++);
      while ((*i)->size ());
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
   win->posDest = posDest;
   wins.push_back (win);

   int x, y;
   src.at (start)->get_window ()->get_origin (x, y);
   win->move (x, y);

   TRACE8 ("CardPileWindows::addWindow () - Pile: " << wins.size ());
   moveCards (*win, src, start, end);
   win->show ();
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
