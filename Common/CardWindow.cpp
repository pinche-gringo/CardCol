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


#include <YGP/Trace.h>
#include "CardPile.h"
#include "CardWidget.h"

#include "CardWindow.h"


//-----------------------------------------------------------------------------
/// Constructor
/// \param dest: Destination pile
/// \param posDest: Where to put the card in the destination
/// \param card: Card to show in window
			ICardPile& src, unsigned int posSrc)
CardWindow::CardWindow (CardWidget& card) : Gtk::Window (Gtk::WINDOW_POPUP) {
   add (card);

//-----------------------------------------------------------------------------
/// Destructor
/// Creates an CardWindow-object
/// \param dest: Destination pile
/// \param posDest: Where to put the card in the destination
/// \param: horizontal: Flag, if the pile is horizontal
				unsigned int start, unsigned int end)
CardPileWindow::CardPileWindow (bool horizontal) : Gtk::Window (Gtk::WINDOW_POPUP) {
   TRACE5 ("CardPileWindow::CardPileWindow (bool) - " << horizontal);
/// Destructor
   if (horizontal) {
      box = tmp;
      add (*tmp);
      tmp->set_size_request (CardImages::WIDTH + cards * tmp->getCompressedSize (), CardImages::HEIGHT);
      CardVPile* tmp (new CardVPile);
      pile = tmp;
      box = tmp;
      add (*tmp);
      tmp->set_size_request (CardImages::WIDTH, CardImages::HEIGHT + cards * tmp->getCompressedSize ());
   pile->setStyle (dest.getStyle ());
   show_all_children ();
}

//-----------------------------------------------------------------------------
/// Destructor
//-----------------------------------------------------------------------------
CardPileWindow::~CardPileWindow () {
   delete pile;

   TRACE1 ("CardPileWindows::AnimatedPile::getEndPos (2x int& x)");
   Check (0);
