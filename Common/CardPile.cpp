//$Id$

//PROJECT     : Cardgames
//SUBSYSTEM   : Common
//REFERENCES  :
//TODO        :
//BUGS        :
//REVISION    : $Revision$
//AUTHOR      : Markus Schwab
//CREATED     : 03.04.2002
//COPYRIGHT   : Anticopyright (A) 2002, 2003

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


#include <Trace_.h>

#include "CardPile.h"


/*--------------------------------------------------------------------------*/
//Purpose   : Constructor; adds all controls to the dialog
//Parameters: set: Specifier for type of cardset
//            show: Flag, if cards show their faces
/*--------------------------------------------------------------------------*/
ICardPile::ICardPile (PileStyle s, ShowOpt show)
   : style (s), showOpt (show) {
   TRACE3 ("ICardPile::ICardPile (PileStyle) - " << (int)style);
   Check3 (s < LAST);
}

/*--------------------------------------------------------------------------*/
//Purpose   : Destructor
/*--------------------------------------------------------------------------*/
ICardPile::~ICardPile () {
   TRACE9 ("ICardPile::~ICardPile ()");
}

/*--------------------------------------------------------------------------*/
//Purpose   : Sets the top card of the pile
//Parameters: newCard: New top-card
/*--------------------------------------------------------------------------*/
void ICardPile::setTopCard (CardWidget& card) {
   TRACE5 ("ICardPile::setTopCard (CardWidget&) - Card " << card
           << " -> new size: " << size () + 1);

   if ((style > NORMAL) && size ()) {
      Check3 (operator [] (size () - 1));
      resize (size () - 1, style);
   }

   if (showOpt < DONT_CHANGE)
      card.showFace ((bool)showOpt);

   card.show ();
   push_back (&card);
   resize (size () - 1, NORMAL);
}

/*--------------------------------------------------------------------------*/
//Purpose   : Returns and removes the top card of the pile
//Returns   : CardWidget&: Reference to (removed) card
/*--------------------------------------------------------------------------*/
CardWidget& ICardPile::removeTopCard () {
   TRACE5 ("ICardPile::removeTopCard () - Size: " << size ());
   Check3 (size () > 0); Check3 (operator [] (size () - 1));

   CardWidget& card (getTopCard ());
   pop_back ();

   TRACE5 ("ICardPile::removeTopCard () - Card " << card
           << " -> new size: " << size ());

   if (style > NORMAL)
      resize (size () - 1, NORMAL);
   return card;
}


/*--------------------------------------------------------------------------*/
//Purpose   : Flips the topmost card of the pile
/*--------------------------------------------------------------------------*/
void ICardPile::flipTopCard () {
   Check3 (size () > 0);

   getTopCard ().flip ();
}

/*--------------------------------------------------------------------------*/
//Purpose   : Sets the top card of the pile visible as indicated
//Parameters: visible: Flag if cardface should be shown or back
/*--------------------------------------------------------------------------*/
void ICardPile::showTopCardFace (bool visible) {
   Check3 (size () > 0);

   getTopCard ().showFace (visible);
}

/*--------------------------------------------------------------------------*/
//Purpose   : Adds various cards to the pile
//Parameters: visible: Flag if cardface should be shown or back
/*--------------------------------------------------------------------------*/
void ICardPile::setTopCards (const std::vector<CardWidget*>& staple) {
   std::vector<CardWidget*>::const_iterator i;

   for (i = staple.begin (); i != staple.end (); ++i) {
      Check3 (*i);
      setTopCard (**i);
   }
}

/*--------------------------------------------------------------------------*/
//Purpose   : Adds various cards to the pile
//Parameters: visible: Flag if cardface should be shown or back
/*--------------------------------------------------------------------------*/
void ICardPile::setTopCards (const std::vector<CardWidget*>& staple, bool visible) {
   std::vector<CardWidget*>::const_iterator i;

   for (i = staple.begin (); i != staple.end (); ++i) {
      Check3 (*i);
      setTopCard (**i, visible);
   }
}

/*--------------------------------------------------------------------------*/
//Purpose   : Removes all cards from pile
/*--------------------------------------------------------------------------*/
void ICardPile::clear () {
   while (size ())
      remove (getTopCard ());
}

/*--------------------------------------------------------------------------*/
//Purpose   : Returns the card with the passed ID
//Parameters: id: ID of card to return
//Returns   : CardWidget*: Pointer to card with passed ID (or NULL)
/*--------------------------------------------------------------------------*/
CardWidget* ICardPile::get (unsigned int id) const {
   std::vector<CardWidget*>::const_iterator i;

   for (i = begin (); i != end (); ++i) {
      Check3 (*i);
      if ((*i)->id () == id)
         return *i;
   }
   return NULL;
}

/*--------------------------------------------------------------------------*/
//Purpose   : Inserts a card into the pile
//Parameters: card: Card to insert
//            pos: Position of new card
/*--------------------------------------------------------------------------*/
void ICardPile::insert (CardWidget& card, unsigned int pos) {
   TRACE5 ("ICardPile::insert (CardWidget, unsigned int&) - Card " << card
           << " at " << pos);
   Check3 (pos <= size ());

   card.show ();

   if (showOpt < DONT_CHANGE)
      card.showFace ((bool)showOpt);

   std::vector<CardWidget*>::insert (begin () + pos, &card);

   if (style > NORMAL)                          // Cards to display compressed?
      resize ((pos == (size () - 1)) ? pos - 1 : pos, style);
}

/*--------------------------------------------------------------------------*/
//Purpose   : Inserts a card into the pile; sorted by number
//Parameters: card: Card to insert
/*--------------------------------------------------------------------------*/
void ICardPile::insertSorted (CardWidget& card) {
   TRACE5 ("ICardPile::insertSorted (CardWidget&) - Card " << card);
   
   insert (card, (upper_bound (begin (), end (), &card, compCardsByNr) - begin ()));
}

/*--------------------------------------------------------------------------*/
//Purpose   : Inserts a card into the pile; sorted by colour and number
//Parameters: card: Card to insert
/*--------------------------------------------------------------------------*/
void ICardPile::insertColourSorted (CardWidget& card) {
   TRACE5 ("ICardPile::insertSorted (CardWidget&) - Card " << card);
   
   insert (card, (upper_bound (begin (), end (), &card, compCards) - begin ()));
}

/*--------------------------------------------------------------------------*/
//Purpose   : Removes the passed card from the collection
//Parameters: card: Card to remove
/*--------------------------------------------------------------------------*/
CardWidget& ICardPile::remove (CardWidget& card) {
   TRACE8 ("ICardPile::remove (CardWidget&) - " << card);
   Check3 (size () > 0);

   // Search for card and remove it
   std::vector<CardWidget*>::iterator i (std::find (begin (), end (), &card));
   Check3 (i != end ());
   i = erase (i);

   // Check if we have to resize a card
   if (style > NORMAL)
      // If last card was removed: Resize new last card (if any)
      (i == end ()) ? resize (i - begin () - 1, NORMAL) : resize (card, NORMAL);

   return card;
}

/*--------------------------------------------------------------------------*/
//Purpose   : Removes the card at the passed position from the collection
//Parameters: card: Card(position) to remove
/*--------------------------------------------------------------------------*/
CardWidget& ICardPile::remove (unsigned int pos) {
   Check1 (size () > pos);
   TRACE8 ("ICardPile::remove (unsigned int) - Card at pos " << pos << " ("
           << *operator[] (pos) << ')');

   // Remove card on passed position
   std::vector<CardWidget*>::iterator i (begin () + pos);
   CardWidget* pTemp (*i); Check3 (pTemp);
   i = erase (i);

   // Check if we have to resize a card
   if (style > NORMAL)
      // If last card was removed: Resize new last card (if any)
      (i == end ()) ? resize (size () - 1, NORMAL) : resize (*pTemp, NORMAL);

   return *pTemp;
}

/*--------------------------------------------------------------------------*/
//Purpose   : Changes the drawing-style of the collection
//Parameters: s: New style
/*--------------------------------------------------------------------------*/
void ICardPile::setStyle (PileStyle s) {
   TRACE5 ("ICardPile::setStyle (PileStyle) - Size = " << size ());

   if (s == style)
      return;

   style = s;
   for (int i (0); i < static_cast <int> (size () - 1); ++i) {
      Check3 (operator[] (i));
      resize (i, style);
   }
   if (size ())
      resize (size () - 1, NORMAL);
}

/*--------------------------------------------------------------------------*/
//Purpose   : Sorts the cards in the pile with regard of the colour
//Parameters: a: Card to compare
//            b: Card to compare
//Returns   : bool: True, if a < b
/*--------------------------------------------------------------------------*/
bool ICardPile::compCards (const CardWidget* a, const CardWidget* b) {
   Check3 (a); Check3 (b);
   TRACE9 ("ICardPile::compCards (const CardWidget*, const CardWidget*) - "
           << *a << " < " << *b << " = "
           << ((a->colour () == b->colour ())
               ? a->number () < b->number () : a->colour () < b->colour ()));
   return ((a->colour () == b->colour ())
           ? a->number () < b->number () : a->colour () < b->colour ());
}

/*--------------------------------------------------------------------------*/
//Purpose   : Sorts the cards in the pile without regard of the colour
//Parameters: a: Card to compare
//            b: Card to compare
//Returns   : bool: True, if a < b
/*--------------------------------------------------------------------------*/
bool ICardPile::compCardsByNr (const CardWidget* a, const CardWidget* b) {
   Check3 (a); Check3 (b);
   TRACE9 ("ICardPile::compCardsByNr (const CardWidget*, const CardWidget*) - "
           << a->number () << " < " << b->number () << " == "
           << (a->number () < b->number ()));
   return a->number () < b->number ();
}

/*--------------------------------------------------------------------------*/
//Purpose   : Sorts the cards in the pile according the past function
/*--------------------------------------------------------------------------*/
void ICardPile::sort (CMPFUNC fnSort) {
   std::sort (begin (), end (), fnSort);
}

/*--------------------------------------------------------------------------*/
//Purpose   : Finds the first card being equal or bigger than the past one
//Parameters: nr: Number of card (2, 3, 4, ... Ace) to search for
//Returns   : int: Position of card in pile (or -1, if none found)
//Requires  : Cards must be sorted (as the search is binary)
/*--------------------------------------------------------------------------*/
int ICardPile::findFirstEqualOrBigger (CardWidget::NUMBERS nr) const {
   unsigned int first (0), last (size ());
   unsigned int middle;

   TRACE8 ("ICardPile::findFirstEqualOrBigger (CardWidget::NUMBERS) - Searching for "
           << nr << " in " << size () << " cards");

   while ((last - first) > 0 ) {
      middle = first + ((last - first) >> 1);

      TRACE5 ("ICardPile::findFirstEqualOrBigger (CardWidget::NUMBERS) - Data = ["
              << first << "-(" << middle << ")-" << last << ") = "
              << *operator[] (middle));

      Check3 (operator[] (first)); Check3 (operator[] (middle));
      Check3 (operator[] (first)->number () <= operator[] (middle)->number ());
      Check3 ((last == size ()) ? 1
              : operator[] (last) && (operator[] (middle)->number () <= operator[] (last)->number ()));

      if (operator[] (middle)->number () < nr)
         first = middle + 1;
      else
         last = middle;

      Check3 (middle >= 0); Check3 (middle <= size ());
      Check3 (first >= 0); Check3 (last <= size ());
      Check3 (first <= last); Check3 (middle <= last);
   }

#if TRACELEVEL > 4
   TRACE ("ICardPile::findFirstEqualOrBigger (CardWidget::NUMBERS) - End = ["
          << first << "-(" << middle << ")-" << last << ')');
   if (first < size ())
      TRACE ("\t-> " << *operator[] (first))
   else
      TRACE ("\t-> Not found");
#endif

   return ((first < size ()) && ((operator[] (first)->number () >= nr))
           ? static_cast<int> (first) : -1);
}

/*--------------------------------------------------------------------------*/
//Purpose   : Finds the last card having an equal number as the passed card
//Parameters: pos: Card whose (equal) number has to be found
//Returns   : int: Position of card in pile
//Requires  : Cards must be sorted
/*--------------------------------------------------------------------------*/
int ICardPile::findLastEqual (unsigned int pos) const {
   Check3 (pos < size ());

   CardWidget::NUMBERS nr (operator[] (pos)->number ());
   while (++pos < size ()) {
      if (operator[] (pos)->number () != nr)
         break;
   }

   TRACE5 ("ICardPile::findLastEqual (CardWidget::NUMBERS) - Card "
           << *operator[] (pos - 1) << " at position " << pos - 1);
   return pos - 1;
}

/*--------------------------------------------------------------------------*/
//Purpose   : Finds the first card having an equal number as the passed card
//Parameters: pos: Card whose (equal) number has to be found
//Returns   : int: Position of card in pile
//Requires  : Cards must be sorted
/*--------------------------------------------------------------------------*/
int ICardPile::findFirstEqual (unsigned int pos) const {
   TRACE5 ("ICardPile::findFirstEqual (CardWidget::NUMBERS) - Checking card " << pos);
   Check1 (pos < size ());
   Check3 (operator[] (pos));

   CardWidget::NUMBERS nr (operator[] (pos)->number ());
   while (pos--) {
      TRACE9 ("ICardPile::findFirstEqual (CardWidget::NUMBERS) - Checking card"
              << " at " << pos << " = " << *operator[] (pos));
      if (operator[] (pos)->number () != nr)
         break;
   }
   TRACE5 ("ICardPile::findFirstEqual (CardWidget::NUMBERS) - Card at position "
           << (pos + 1) << " = " << *operator[] (pos + 1));
   return pos + 1;
}

/*--------------------------------------------------------------------------*/
//Purpose   : Makes the staple display either back or faces of the cards
//Parameters: show: Option of how to display the cards
/*--------------------------------------------------------------------------*/
void ICardPile::setShowOption (ShowOpt show) {
   showOpt = show;

   if (showOpt < DONT_CHANGE) {
      std::vector<CardWidget*>::iterator i;

      for (i = begin (); i != end (); ++i) {
         Check3 (*i);
         (*i)->showFace (showOpt);
      }
   }
}

/*--------------------------------------------------------------------------*/
//Purpose   : Moves the card at pos source to pos dest
//Parameters: source: Position of card to move
//            dest: New position of card
/*--------------------------------------------------------------------------*/
CardWidget& ICardPile::move (unsigned int dest, unsigned int source) {
   TRACE5 ("ICardPile::move (unsigned int, unsigned int) - Card from pos "
           << source << " to " << dest);

   insert (remove (source), dest);
}

/*--------------------------------------------------------------------------*/
//Purpose   : Searches for the first card having the passed number
//Parameters: nr: Number to search for
//            start: Position of start of search
//Returns   : int: Offset of found card or -1
/*--------------------------------------------------------------------------*/
int ICardPile::find (CardWidget::NUMBERS nr, unsigned int start) const {
   for (; start < size (); ++start)
      if (operator[] (start)->number () == nr)
         return start;

   return -1;
}

/*--------------------------------------------------------------------------*/
//Purpose   : Searches for the first card having the passed colour
//Parameters: colour: Colour to search for
//            start: Position of start of search
//Returns   : int: Offset of found card or -1
/*--------------------------------------------------------------------------*/
int ICardPile::find (CardWidget::COLOURS colour, unsigned int start) const {
   for (; start < size (); ++start)
      if (operator[] (start)->colour () == colour)
         return start;

   return -1;
}

/*--------------------------------------------------------------------------*/
//Purpose   : Resizing of a card in the pile
//Parameters: CardWidget&: Card to resize
//            PileStyle: Style of pile
/*--------------------------------------------------------------------------*/
void ICardPile::resize (CardWidget&, PileStyle) {
}
