//$Id$

//PROJECT     : Cardgames
//SUBSYSTEM   : Common
//REFERENCES  :
//TODO        :
//BUGS        :
//REVISION    : $Revision$
//AUTHOR      : Markus Schwab
//CREATED     : 03.04.2002
//COPYRIGHT   : Copyright (C) 2002 - 2005

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


#include "CardPile.h"

//-----------------------------------------------------------------------------
/// Constructor; adds all controls to the dialog
/// \param set: Specifier for type of cardset
/// \param show: Flag, if cards show their faces
//-----------------------------------------------------------------------------
ICardPile::ICardPile (PileStyle s, ShowOpt show)
   : style (s), showOpt (show) {
   TRACE3 ("ICardPile::ICardPile (PileStyle) - " << (int)style);
   Check3 (s < LAST);
}

//-----------------------------------------------------------------------------
/// Destructor
//-----------------------------------------------------------------------------
ICardPile::~ICardPile () {
   TRACE9 ("ICardPile::~ICardPile ()");
}

//-----------------------------------------------------------------------------
/// Sets the top card of the pile
/// \param newCard: New top-card
//-----------------------------------------------------------------------------
void ICardPile::setTopCard (CardWidget& card) {
   TRACE5 ("ICardPile::setTopCard (CardWidget&) - Card " << card
           << " -> new size: " << size () + 1);

   if ((style > NORMAL) && size ()) {
      Check3 (operator[] (size () - 1));
      resize (size () - 1, style);
   }

   if (showOpt < DONT_CHANGE)
      card.showFace ((bool)showOpt);

   push_back (&card);
   resize (size () - 1, NORMAL);
}

//-----------------------------------------------------------------------------
/// Returns and removes the top card of the pile
/// \returns \c CardWidget&: Reference to (removed) card
//-----------------------------------------------------------------------------
CardWidget& ICardPile::removeTopCard () {
   TRACE5 ("ICardPile::removeTopCard () - Size: " << size ());
   Check3 (size () > 0); Check3 (operator[] (size () - 1));

   CardWidget& card (getTopCard ());
   pop_back ();

   TRACE5 ("ICardPile::removeTopCard () - Card " << card
           << " -> new size: " << size ());

   if (style > NORMAL)
      resize (size () - 1, NORMAL);
   return card;
}


//-----------------------------------------------------------------------------
/// Flips the topmost card of the pile
//-----------------------------------------------------------------------------
void ICardPile::flipTopCard () {
   Check3 (size () > 0);

   getTopCard ().flip ();
}

//-----------------------------------------------------------------------------
/// Sets the top card of the pile visible as indicated
/// \param visible: Flag if cardface should be shown or back
//-----------------------------------------------------------------------------
void ICardPile::showTopCardFace (bool visible) {
   Check3 (size () > 0);

   getTopCard ().showFace (visible);
}

//-----------------------------------------------------------------------------
/// Adds various cards to the pile
/// \param visible: Flag if cardface should be shown or back
//-----------------------------------------------------------------------------
void ICardPile::setTopCards (const std::vector<CardWidget*>& staple) {
   std::vector<CardWidget*>::const_iterator i;

   for (i = staple.begin (); i != staple.end (); ++i) {
      Check3 (*i);
      setTopCard (**i);
   }
}

//-----------------------------------------------------------------------------
/// Adds various cards to the pile
/// \param visible: Flag if cardface should be shown or back
//-----------------------------------------------------------------------------
void ICardPile::setTopCards (const std::vector<CardWidget*>& staple, bool visible) {
   std::vector<CardWidget*>::const_iterator i;

   for (i = staple.begin (); i != staple.end (); ++i) {
      Check3 (*i);
      setTopCard (**i, visible);
   }
}

//-----------------------------------------------------------------------------
/// Removes all cards from pile
//-----------------------------------------------------------------------------
void ICardPile::clear () {
   while (size ())
      remove (getTopCard ());
}

//-----------------------------------------------------------------------------
/// Returns the card with the passed ID
/// \param id: ID of card to return
/// \returns \c CardWidget*: Pointer to card with passed ID (or NULL)
//-----------------------------------------------------------------------------
CardWidget* ICardPile::get (unsigned int id) const {
   std::vector<CardWidget*>::const_iterator i;

   for (i = begin (); i != end (); ++i) {
      Check3 (*i);
      if ((*i)->id () == id)
         return *i;
   }
   return NULL;
}

//-----------------------------------------------------------------------------
/// Inserts a card into the pile
/// \param card: Card to insert
/// \param pos: Position of new card
/// \returns unsigned int: Position where card was inserted
//-----------------------------------------------------------------------------
unsigned int ICardPile::insert (CardWidget& card, unsigned int pos) {
   TRACE5 ("ICardPile::insert (CardWidget, unsigned int&) - Card " << card
           << " at " << pos);
   Check3 (pos <= size ());

   if (showOpt < DONT_CHANGE)
      card.showFace ((bool)showOpt);

   std::vector<CardWidget*>::iterator i
      (std::vector<CardWidget*>::insert (begin () + pos, &card));

   if (style > NORMAL)                          // Cards to display compressed?
      resize ((pos == (size () - 1)) ? pos - 1 : pos, style);

   return i - begin ();
}

//-----------------------------------------------------------------------------
/// Inserts a card into the pile; sorted by the passed function
/// \param card: Card to insert
/// \param fnSort: Function, how to sort
/// \returns unsigned int: Position where card was inserted
//-----------------------------------------------------------------------------
unsigned int ICardPile::insertSorted (CardWidget& card, CMPFUNC fnSort) {
   TRACE5 ("ICardPile::insertSorted (CardWidget&, CMPFUNC) - Card " << card);
   return insert (card, (upper_bound (begin (), end (), &card, fnSort) - begin ()));
}

//-----------------------------------------------------------------------------
/// Removes the passed card from the collection
/// \param card: Card to remove
//-----------------------------------------------------------------------------
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

//-----------------------------------------------------------------------------
/// Removes the card at the passed position from the collection
/// \param card: Card(position) to remove
//-----------------------------------------------------------------------------
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

//-----------------------------------------------------------------------------
/// Changes the drawing-style of the collection
/// \param s: New style
//-----------------------------------------------------------------------------
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

//-----------------------------------------------------------------------------
/// Sorts the cards in the pile with regard of the colour
/// \param a: Card to compare
/// \param b: Card to compare
/// \returns \c bool: True, if a < b
//-----------------------------------------------------------------------------
bool ICardPile::compCards (const CardWidget* a, const CardWidget* b) {
   Check3 (a); Check3 (b);
   TRACE9 ("ICardPile::compCards (const CardWidget*, const CardWidget*) - "
           << *a << " < " << *b << " = "
           << ((a->colour () == b->colour ())
               ? a->number () < b->number () : a->colour () < b->colour ()));
   return ((a->colour () == b->colour ())
           ? a->number () < b->number () : a->colour () < b->colour ());
}

//-----------------------------------------------------------------------------
/// Sorts the cards in the pile without regard of the colour
/// \param a: Card to compare
/// \param b: Card to compare
/// \returns \c bool: True, if a < b
//-----------------------------------------------------------------------------
bool ICardPile::compCardsByNr (const CardWidget* a, const CardWidget* b) {
   Check3 (a); Check3 (b);
   TRACE9 ("ICardPile::compCardsByNr (const CardWidget*, const CardWidget*) - "
           << a->number () << " < " << b->number () << " == "
           << (a->number () < b->number ()));
   return a->number () < b->number ();
}

//-----------------------------------------------------------------------------
/// Sorts the cards in the pile without regard of the colour
/// \param a: Card to compare
/// \param b: Card to compare
/// \returns \c bool: True, if a < b
//-----------------------------------------------------------------------------
bool ICardPile::compCardsByID (const CardWidget* a, const CardWidget* b) {
   Check3 (a); Check3 (b);
   return a->id () < b->id ();
}

//-----------------------------------------------------------------------------
/// Sorts the cards in the pile according the past function
//-----------------------------------------------------------------------------
void ICardPile::sort (CMPFUNC fnSort) {
   std::sort (begin (), end (), fnSort);
}

//-----------------------------------------------------------------------------
/// Finds the first card being equal or bigger than the past one
/// \param nr: Number of card (2, 3, 4, ... Ace) to search for
/// \returns \c int: Position of card in pile (or -1, if none found)
/// \pre Cards must be sorted (as the search is binary)
//-----------------------------------------------------------------------------
int ICardPile::findFirstEqualOrBigger (CardWidget::NUMBERS nr) const {
   unsigned int first (0), last (size ());
   unsigned int middle (0);

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

      Check3 (middle <= size ());
      Check3 (last <= size ());
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

//-----------------------------------------------------------------------------
/// Finds the first card being equal or bigger than the past one
/// \param nr: Colour of card (club, diamond, ...) to search for
/// \returns \c int: Position of card in pile (or -1, if none found)
/// \pre Cards must be sorted (as the search is binary)
//-----------------------------------------------------------------------------
int ICardPile::findFirstEqualOrBiggerColour (CardWidget::COLOURS col) const {
   unsigned int first (0), last (size ());
   unsigned int middle (0);

   TRACE8 ("ICardPile::findFirstEqualOrBiggerColour (CardWidget::COLOURS) - Searching for "
           << col << " in " << size () << " cards");

   while ((last - first) > 0 ) {
      middle = first + ((last - first) >> 1);

      TRACE5 ("ICardPile::findFirstEqualOrBiggerColour (CardWidget::COLOURS) - Data = ["
              << first << "-(" << middle << ")-" << last << ") = "
              << *operator[] (middle));

      Check3 (operator[] (first)); Check3 (operator[] (middle));
      Check3 (operator[] (first)->colour () <= operator[] (middle)->colour ());
      Check3 ((last == size ()) ? 1
              : operator[] (last) && (operator[] (middle)->colour () <= operator[] (last)->colour ()));

      if (operator[] (middle)->colour () < col)
         first = middle + 1;
      else
         last = middle;

      Check3 (middle <= size ());
      Check3 (last <= size ());
      Check3 (first <= last); Check3 (middle <= last);
   }

#if TRACELEVEL > 4
   TRACE ("ICardPile::findFirstEqualOrBigger (CardWidget::COLOURS) - End = ["
          << first << "-(" << middle << ")-" << last << ')');
   if (first < size ())
      TRACE ("\t-> " << *operator[] (first))
   else
      TRACE ("\t-> Not found");
#endif

   return ((first < size ()) && ((operator[] (first)->colour () >= col))
           ? static_cast<int> (first) : -1);
}

//-----------------------------------------------------------------------------
/// Finds the last card having an equal number as the passed card
/// \param pos: Card whose (equal) number has to be found
/// \returns \c int: Position of card in pile
/// \pre Cards must be sorted
//-----------------------------------------------------------------------------
int ICardPile::findLastEqual (unsigned int pos) const {
   Check3 (pos < size ());

   CardWidget::NUMBERS nr (operator[] (pos)->number ());
   while (++pos < size ()) {
      if (operator[] (pos)->number () != nr)
         break;
   }

   TRACE5 ("ICardPile::findLastEqual (unsigned int) - Card "
           << *operator[] (pos - 1) << " at position " << pos - 1);
   return pos - 1;
}

//-----------------------------------------------------------------------------
/// Finds the last card having an equal colour as the passed card
/// \param pos: Card whose (equal) colour has to be found
/// \returns \c int: Position of card in pile
/// \pre Cards must be sorted
//-----------------------------------------------------------------------------
int ICardPile::findLastEqualColour (unsigned int pos) const {
   Check3 (pos < size ());

   CardWidget::COLOURS col (operator[] (pos)->colour ());
   while (++pos < size ()) {
      if (operator[] (pos)->colour () != col)
         break;
   }

   TRACE5 ("ICardPile::findLastEqualColour (unsigned int) - Card "
           << *operator[] (pos - 1) << " at position " << pos - 1);
   return pos - 1;
}

//-----------------------------------------------------------------------------
/// Finds the first card having an equal number as the passed card
/// \param pos: Card whose (equal) number has to be found
/// \returns \c int: Position of card in pile
/// \pre Cards must be sorted
//-----------------------------------------------------------------------------
int ICardPile::findFirstEqual (unsigned int pos) const {
   TRACE5 ("ICardPile::findFirstEqual (unsigned int) - Checking card " << pos);
   Check1 (pos < size ());
   Check3 (operator[] (pos));

   CardWidget::NUMBERS nr (operator[] (pos)->number ());
   while (pos--) {
      TRACE9 ("ICardPile::findFirstEqual (unsigned int) - Checking card"
              << " at " << pos << " = " << *operator[] (pos));
      if (operator[] (pos)->number () != nr)
         break;
   }
   TRACE5 ("ICardPile::findFirstEqual (unsigned int) - Card at position "
           << (pos + 1) << " = " << *operator[] (pos + 1));
   return pos + 1;
}

//-----------------------------------------------------------------------------
/// Finds the first card having an equal colour as the passed card
/// \param pos: Card whose (equal) colour has to be found
/// \returns \c int: Position of card in pile
/// \pre Cards must be sorted
//-----------------------------------------------------------------------------
int ICardPile::findFirstEqualColour (unsigned int pos) const {
   TRACE5 ("ICardPile::findFirstEqualColour (unsigned int) - Checking card " << pos);
   Check1 (pos < size ());
   Check3 (operator[] (pos));

   CardWidget::COLOURS col (operator[] (pos)->colour ());
   while (pos--) {
      TRACE9 ("ICardPile::findFirstEqualColour (unsigned int) - Checking card"
              << " at " << pos << " = " << *operator[] (pos));
      if (operator[] (pos)->colour () != col)
         break;
   }
   TRACE5 ("ICardPile::findFirstEqualColour (unsigned int) - Card at position "
           << (pos + 1) << " = " << *operator[] (pos + 1));
   return pos + 1;
}

//-----------------------------------------------------------------------------
/// Makes the staple display either back or faces of the cards
/// \param show: Option of how to display the cards
//-----------------------------------------------------------------------------
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

//-----------------------------------------------------------------------------
/// Moves the card at pos source to pos dest
/// \param source: Position of card to move
/// \param dest: New position of card
//-----------------------------------------------------------------------------
void ICardPile::move (unsigned int dest, unsigned int source) {
   TRACE5 ("ICardPile::move (unsigned int, unsigned int) - Card from pos "
           << source << " to " << dest);

   insert (remove (source), dest);
}

//-----------------------------------------------------------------------------
/// Searches for the first card having the passed number
/// \param nr: Number to search for
/// \param start: Position of start of search
/// \returns \c int: Offset of found card or -1
//-----------------------------------------------------------------------------
int ICardPile::find (CardWidget::NUMBERS nr, unsigned int start) const {
   for (; start < size (); ++start)
      if (operator[] (start)->number () == nr)
         return start;

   return -1;
}

//-----------------------------------------------------------------------------
/// Searches for the first card having the passed colour
/// \param colour: Colour to search for
/// \param start: Position of start of search
/// \returns \c int: Offset of found card or -1
//-----------------------------------------------------------------------------
int ICardPile::find (CardWidget::COLOURS colour, unsigned int start) const {
   for (; start < size (); ++start)
      if (operator[] (start)->colour () == colour)
         return start;

   return -1;
}

//-----------------------------------------------------------------------------
/// Searches for the first card having the passed id
/// \param id: ID of card to search for
/// \param id: ID of card to search for
/// \returns \c int: Offset of found card or -1
//-----------------------------------------------------------------------------
int ICardPile::find (unsigned int id, unsigned int start) const {
   Check1 (start < size ());
   for (const_iterator i (begin () + start); i != end (); ++i)
      if ((*i)->id () == id)
         return i - begin ();
   return -1;
}

//-----------------------------------------------------------------------------
/// Resizing of a card in the pile
/// \param CardWidget&: Card to resize
/// \param PileStyle: Style of pile
//-----------------------------------------------------------------------------
void ICardPile::resize (CardWidget&, PileStyle) {
}


//-----------------------------------------------------------------------------
/// Checks if the passed pile contains a pair matching the passed card
/// \param card: Card where to find a pair to
/// \param cmp: Method to compare two cards. This method gets the two cards to
///        compare as input as must return an integer describing their
///        difference (0: Equal). A pair can have a difference of at most
///        [-2 - 2]
/// \param doubles: True, if the same card (id) can be included more than once
/// \returns \c True, if the pile contains a matching pair
//-----------------------------------------------------------------------------
bool ICardPile::hasFittingPair (const CardWidget& card, CMPFUNC2 cmp,
                                bool doubles) const {
   TRACE3 ("ICardPile::pileHasFittingPair (const CardWidget*, CMPFUNC2, bool) - " << card);

   unsigned int nrs (0);
   unsigned int bCols (0);

   std::vector<unsigned int> foundCards (4);
   if (!doubles)
      foundCards.push_back (card.id ());

   for (const_iterator p (begin ());
        (p = getFittingCard (card, p, cmp)) != end (); ++p) {
      if (*p == &card)                      // Skip card if its the passed one
         continue;

      int diff (cmp (**p, card));
      if (diff) {
         diff = (diff < 0) ? (diff + 2) : (diff + 1);
         Check3 (diff < 4);
         TRACE1 ("ICardPile::pileHasFittingPair (const " "CardWidget*, "
                 "CMPFUNC2, bool) - " << **p << " diff: " << diff);
         if ((((unsigned int)diff) < 4) && !(bCols & (1 << diff))) {
            // The card is valid, if either a card bordering the one the
            // inspect and this one has been found. Note that for aces the
            // bordering card must be in the same direction as the card to
            // to inspect (e.g. K-A-2 is not valid; only Q-K-A!)
            if ((card.number () == CardWidget::ACE)
                ? (bCols & (0x1 << (diff ^ 0x1)))
                : (bCols & (diff ? (0x5 << (diff - 1)) : 0x1)))
               return true;
            bCols |= (1 << diff);
         }
      }
      else {
         // Filter out doubles (if specififed)
        if (!doubles) {
           std::vector<unsigned int>::const_iterator i (foundCards.begin ());
           do {
              TRACE1 ("ICardPile::pileHasFittingPar (const CardWidget*, CMPFUNC2, bool)) - "
                      << *i << '-' << (*p)->id ());
              if (*i == (*p)->id ())
                 break;
           } while (++i != foundCards.end ());
           if (i != foundCards.end ())
              continue;

           foundCards.push_back ((*p)->id ());
           TRACE1 ("ICardPile::pileHasFittingPair (const " "CardWidget*, "
                   "CMPFUNC2, bool) - Adding non-double " << **p);
         }
         if (++nrs == 2)
            return true;
      }
   }
   TRACE7 ("CardPile::pileHasFittingPair (const CardWidget*, CMPFUNC2, bool) - "
           << card << " matches " << nrs << '/' << std::hex << bCols << std::dec);
   return false;
}

//-----------------------------------------------------------------------------
/// Returns a card fitting to the passed on
/// \param card: Card where to find a fitting one to
/// \param start: Position where to start the search
/// \param cmp: Method to compare two cards. This method gets the two cards to
///        compare as input as must return an integer describing their
///        difference (0: Equal). A pair can have a difference of at most
///        [-2 - 2]
/// \returns \c Position of matching card or pile.end ()
/// \pre start must be a valid iterator in pile
//-----------------------------------------------------------------------------
ICardPile::iterator ICardPile::getFittingCard (const CardWidget& card,
                                               const_iterator start,
                                               CMPFUNC2 cmp) const {
   TRACE9 ("ICardPile::getFittingCard (const CardWidget*, const_iterator, "
           "CMPFUNC2) - " << card);

   while (start != end ()) {
      if ((static_cast<unsigned int> (cmp (card, **start) + 2)) < 5)
	 break;
      ++start;
   }

#if TRACELEVEL > 8
   if (start == end ()) {
      TRACE ("ICardPile::getFittingCard (const CardWidget*, const_iterator, "
             "CMPFUNC2) - End");
   }
   else {
      TRACE ("ICardPile::getFittingCard (const CardWidget*, const_iterator, "
             "CMPFUNC2) - Found " << **start);
   }
#endif
   return (const_cast<ICardPile*> (this)->begin ()
           + (start - (const_iterator)begin ()));
}

//----------------------------------------------------------------------------
/// Sorts a series of matching cards to the end of the pile
/// \param aPos: Map holding the positions of the cards in the pile
/// \param aOrder: Sorted order of the cards
/// \returns unsigned int: Position of start of sorted serie
//----------------------------------------------------------------------------
unsigned int ICardPile::sortColourSerie (std::map<unsigned int, unsigned int>& aPos,
                                         std::vector<unsigned int>& aOrder) {
   unsigned int pos (1);
   for (std::vector<unsigned int>::reverse_iterator p (aOrder.rbegin ());
        p != aOrder.rend (); ++p) {
      std::map<unsigned int, unsigned int>::const_iterator v;
      if ((v = aPos.find (*p)) != aPos.end ()) {
          TRACE9 ("ICardPile::sortColourSeries (...) - Moving " << v->second
                  << " to end " << ((*p < 2) ? *p : 0));
          Check3 ((p - aOrder.rbegin ()) >= 0);
          // Move the card to the end of the staple; If it belongs before the
          // first card move it before the other cards.
          move (size () - pos, v->second);
          unsigned int oldOrder (*p);
          if (((p + 1) != aOrder.rend ()) && (*(p + 1) < oldOrder))
             ++pos;
      }
   }
   TRACE9 ("ICardPile::sortColourSeries (...) - Moved "
           << size () - aPos.size () << " cards");
   return size () - aPos.size ();
}

//----------------------------------------------------------------------------
/// Gets a series of matching cards
/// \param card: Card to compare
/// \param aPos: Map holding the positions of the cards in the pile
/// \param aOrder: Sorted order of the cards
/// \param cmp: Method to compare two cards. This method gets the two cards to
///        compare as input as must return an integer describing their
///        difference (0: Equal). A pair can have a difference of at most
///        [-2 - 2]
/// \returns unsigned int: The number of matching cards in a row
//----------------------------------------------------------------------------
unsigned int ICardPile::getSeries (CardWidget& card,
                                   std::map<unsigned int, unsigned int>& aPos,
                                   std::vector<unsigned int>& aOrder, CMPFUNC2 cmp,
                                   bool doubles) {
   TRACE1 ("ICardPile::getSeries (...) for " << card);
   unsigned int nrs (0);
   unsigned int bCols (0x4);

   std::vector<unsigned int> foundCards (4);
   unsigned int cDoubles (0);
   if (!doubles)
      foundCards.push_back (card.id ());

   for (const_iterator p (begin ()); ((p = getFittingCard (card, p, cmp)) != end ()); ++p) {
      if (*p == &card) {
         aPos[2] = p - begin ();
         aOrder.push_back (2);
         ++nrs;
      }
      else {
         int diff (cmp (**p, card));
         TRACE1 ("ICardPile::getSeries (...) - " << **p << " differs " << diff);
         Check3 (static_cast<unsigned int> (diff + 2) < 5);
         if (diff) {
            diff += 2;
            Check3 (diff <= 4);
            if (!(bCols & (1 << diff))) {
               Check3 (aPos.find (diff) == aPos.end ());
               bCols |= (1 << diff);
               aPos[diff] = p - begin ();
               aOrder.push_back (diff);
               if (aPos.size () == 7)
                   break;
            }
         }
         else {
            // Filter out doubles (if specififed)
            if (!doubles) {
               std::vector<unsigned int>::const_iterator i (foundCards.begin ());
               do {
                  TRACE1 ("ICardPile::getSeries (...) - " << *i << '-' << (*p)->id ());
                  if (*i == (*p)->id ())
                     break;
               } while (++i != foundCards.end ());
               if (i != foundCards.end ()) {
                  ++cDoubles;
                  continue;
               }

               foundCards.push_back ((*p)->id ());
               TRACE1 ("ICardPile::getSeries (...) - Adding non-double " << **p);

               if (cDoubles)
                  move (p - begin () - cDoubles, p - begin ());
            }
            ++nrs;
         }
      }
   }

   // Check if the series of colors is a valid one
   TRACE1 ("ICardPile::getSeries (...) - Serie: " << std::hex << bCols << std::dec);
   Check3 (bCols & 0x4);

   // Delete cards having no direct access to the analyzed one
   if ((bCols & 0x3) == 0x1) {
      Check3 (aPos.find (0) != aPos.end ());
      Check3 (aPos.find (1) == aPos.end ());
      aPos.erase (aPos.find (0));
   }
   if ((bCols & 0x18) == 0x10) {
      Check3 (aPos.find (4) != aPos.end ());
      Check3 (aPos.find (3) == aPos.end ());
      aPos.erase (aPos.find (4));
   }

   // Special handling of series of colours for an ace, to avoid the problem
   // with 3-K-A of one colour.
   if (card.number () == CardWidget::ACE) {
       if ((bCols & 0xa) == 0xa) {
          std::map<unsigned int, unsigned int>::iterator i;
          if ((bCols & 0x18) == 0x18) {
             if ((i = aPos.find (0)) != aPos.end ())
                aPos.erase (i);
             if ((i = aPos.find (1)) != aPos.end ())
                aPos.erase (i);
          }
          else {
             if ((i = aPos.find (3)) != aPos.end ())
                aPos.erase (i);
             if ((i = aPos.find (4)) != aPos.end ())
                aPos.erase (i);
          }
       }
   }

   return nrs;
}

//----------------------------------------------------------------------------
/// Find the worst (lowest) card in the pile
/// \return unsigned int: Position of the lowest card in the pile
//----------------------------------------------------------------------------
unsigned int ICardPile::findLowestCard () const {
   TRACE9 ("ICardPile::findLowestCard () const - Analyzing " << size () << " cards");
   Check3 (size ());

   unsigned int pos (0);
   for (ICardPile::const_iterator i (begin () + 1); i != end (); ++i)
      if ((*i)->number () < operator[] (pos)->number ())
         pos = i - begin ();

   TRACE8 ("ICardPile::findLowestCard () const - Lowest card: "
           << *operator[] (pos) << " at " << pos);
   return pos;
}

//----------------------------------------------------------------------------
/// Find the worst (lowest) card in the pile
/// \param excludeColour: Special colour, which is not included in the search
/// \return unsigned int: Position of the lowest card in the pile
//----------------------------------------------------------------------------
unsigned int ICardPile::findLowestCard (CardWidget::COLOURS excludeColour) const {
   TRACE9 ("ICardPile::findLowestCard (CardWidget::COLOURS) const - Analyzing "
           << size () << " cards")
   Check3 (size ());

   unsigned int pos (0);
   for (ICardPile::const_iterator i (begin () + 1); i != end (); ++i)
      if (((*i)->colour () != excludeColour)
          && ((*i)->number () < operator[] (pos)->number ()))
         pos = i - begin ();

   TRACE8 ("ICardPile::findLowestCard (CardWidget::COLOURS) const - Lowest card: "
           << *operator[] (pos) << " at " << pos);
   return pos;
}

//----------------------------------------------------------------------------
/// Finds the last card with the passed colour in the pile.
/// \param col: Colour to search for
/// \return int: Position of found card; -1 if there's no bigger card
/// \pre The pile must be sorted by colour
//----------------------------------------------------------------------------
int ICardPile::findLastEqualOrBiggerColour (CardWidget::COLOURS col) const {
   TRACE9 ("ICardPile::findLastEqualOrBiggerColour (CardWidget::COLOURS)");
   int pos (findFirstEqualOrBiggerColour (col));
   return ((pos == -1) ? - 1 : ((operator[] (pos)->colour () == col)
                                ? findLastEqualColour (pos) : -1));
}
