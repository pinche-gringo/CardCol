//$Id$

//PROJECT     : Cardgames
//SUBSYSTEM   : Common
//REFERENCES  :
//TODO        :
//BUGS        :
//REVISION    : $Revision$
//AUTHOR      : Markus Schwab
//CREATED     : 03.04.2002
//COPYRIGHT   : Anticopyright (A) 2002

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

#include <Check.h>

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
           << " -> new size: " << cards.size () + 1);

   if ((style > NORMAL) && cards.size ()) {
      Check3 (cards[cards.size () - 1]);
      resize (getTopCard (), style);
   }

   if (showOpt < DONT_CHANGE)
      card.setVisible ((bool)showOpt);

   card.show ();
   cards.push_back (&card);
}

/*--------------------------------------------------------------------------*/
//Purpose   : Returns and removes the top card of the pile
//Returns   : CardWidget&: Reference to (removed) card
/*--------------------------------------------------------------------------*/
CardWidget& ICardPile::removeTopCard () {
   TRACE5 ("ICardPile::removeTopCard () - Size: " << cards.size ());

   Check3 (cards.size () > 0); Check3 (cards[cards.size () - 1]);
   CardWidget& card (getTopCard ());
   cards.pop_back ();

   TRACE5 ("ICardPile::removeTopCard (CardWidget&) - Card " << card
           << " -> new size: " << cards.size ());

   if (cards.size () && (style > NORMAL)) {
      TRACE9 ("ICardPile::removeTopCard () - Resizing");

      CardWidget& card (getTopCard ());
      resize (card, NORMAL);
   }

   card.set_sensitive (true);
   return card;
}


/*--------------------------------------------------------------------------*/
//Purpose   : Flips the topmost card of the pile
/*--------------------------------------------------------------------------*/
void ICardPile::flipTopCard () {
   Check3 (cards.size () > 0);

   getTopCard ().flip ();
}

/*--------------------------------------------------------------------------*/
//Purpose   : Sets the top card of the pile visible as indicated
//Parameters: visible: Flag if cardface should be shown or back
/*--------------------------------------------------------------------------*/
void ICardPile::setTopCardVisible (bool visible) {
   Check3 (cards.size () > 0);

   getTopCard ().setVisible (visible);
}

/*--------------------------------------------------------------------------*/
//Purpose   : Adds various cards to the pile
//Parameters: visible: Flag if cardface should be shown or back
/*--------------------------------------------------------------------------*/
void ICardPile::setTopCards (const vector<CardWidget*>& staple) {
   vector<CardWidget*>::const_iterator i;

   for (i = staple.begin (); i != staple.end (); ++i) {
      Check3 (*i);
      setTopCard (**i);
   }
}

/*--------------------------------------------------------------------------*/
//Purpose   : Adds various cards to the pile
//Parameters: visible: Flag if cardface should be shown or back
/*--------------------------------------------------------------------------*/
void ICardPile::setTopCards (const vector<CardWidget*>& staple, bool visible) {
   vector<CardWidget*>::const_iterator i;

   for (i = staple.begin (); i != staple.end (); ++i) {
      Check3 (*i);
      setTopCard (**i, visible);
   }
}

/*--------------------------------------------------------------------------*/
//Purpose   : Removes all cards from pile
/*--------------------------------------------------------------------------*/
void ICardPile::clear () {
   while (cards.size ())
      remove (getTopCard ());
}

/*--------------------------------------------------------------------------*/
//Purpose   : Returns the card with the passed ID
//Parameters: id: ID of card to return
//Returns   : CardWidget*: Pointer to card with passed ID (or NULL)
/*--------------------------------------------------------------------------*/
CardWidget* ICardPile::get (unsigned int id) const {
   vector<CardWidget*>::const_iterator i;

   for (i = cards.begin (); i != cards.end (); ++i) {
      Check3 (*i);
      if ((*i)->id () == id) {
         (*i)->set_sensitive (true);
         return *i;
      }
   }
   return NULL;
}

/*--------------------------------------------------------------------------*/
//Purpose   : Inserts a card into the pile
//Parameters: card: Card to insert
//            pos: Position of new card
/*--------------------------------------------------------------------------*/
void ICardPile::insert (CardWidget& card, unsigned int pos) {
   TRACE5 ("ICardPile::insertCard (CardWidget, unsigned int&) - Card " << card
           << " at " << pos);
   Check3 (pos <= cards.size ());

   card.show ();

   if ((style > NORMAL) && cards.size ()) {     // Cards to display compressed?
      CardWidget* compressCard (pos == cards.size ()
                                ? cards[cards.size () - 1] : &card);
      Check3 (compressCard);
      resize (*compressCard, COMPRESSED);
   }

   if (showOpt < DONT_CHANGE)
      card.setVisible ((bool)showOpt);

   cards.insert (cards.begin () + pos, &card);
}

/*--------------------------------------------------------------------------*/
//Purpose   : Inserts a card into the pile
//Parameters: card: Card to insert
/*--------------------------------------------------------------------------*/
void ICardPile::insertSorted (CardWidget& card) {
   TRACE5 ("ICardPile::insertSorted (CardWidget&) - Card " << card);
   
   insert (card, (upper_bound (cards.begin (), cards.end (),
                              &card, compCardsByNr)
                  - cards.begin ()));
}

/*--------------------------------------------------------------------------*/
//Purpose   : Removes the passed card from the collection
//Parameters: card: Card to remove
/*--------------------------------------------------------------------------*/
CardWidget& ICardPile::remove (CardWidget& card) {
   Check3 (cards.size () > 0);

   // Search for card and remove it
   vector<CardWidget*>::iterator i (find (cards.begin (), cards.end (), &card));
   Check3 (i != cards.end ());

   // Check if we have to resize a card
   if (style > NORMAL) {
      // If last card was removed: Resize new last card (if any)
      if (((i + 1) == cards.end ()) && i != cards.begin ())
         resize (**(i - 1), NORMAL);
      else
         resize (card, NORMAL);                    // Else enlarge removed card
   }

   cards.erase (i--);

   return card;
}

/*--------------------------------------------------------------------------*/
//Purpose   : Removes the card at the passed position from the collection
//Parameters: card: Card(position) to remove
/*--------------------------------------------------------------------------*/
CardWidget& ICardPile::remove (unsigned int pos) {
   Check3 (cards.size () > pos);

   // Remove card on passed position it
   vector<CardWidget*>::iterator i (cards.begin () + pos);
   CardWidget* pTemp (*i); Check3 (pTemp);

   // Check if we have to resize a card
   if (style > NORMAL) {
      // If last card was removed: Resize new last card (if any)
      if (((i + 1) == cards.end ()) && i != cards.begin ())
         resize (**(i - 1), NORMAL);
      else
         resize (*pTemp, NORMAL);                  // Else enlarge removed card
   }

   cards.erase (i--);
   return *pTemp;
}

/*--------------------------------------------------------------------------*/
//Purpose   : Changes the drawing-style of the collection
//Parameters: s: New style
/*--------------------------------------------------------------------------*/
void ICardPile::setStyle (PileStyle s) {
   TRACE5 ("ICardPile::setStyle (PileStyle) - Size = " << cards.size ());

   if (s == style)
      return;

   style = s;
   for (int i (0); i < static_cast <int> (cards.size () - 1); ++i) {
      Check3 (cards[i]);
      resize (*cards[i], style);
   }
}

/*--------------------------------------------------------------------------*/
//Purpose   : Sorts the cards in the pile with regard of the color
//Parameters: a: Card to compare
//            b: Card to compare
//Returns   : bool: True, if a < b
/*--------------------------------------------------------------------------*/
bool ICardPile::compCards (const CardWidget* a, const CardWidget* b) {
   Check3 (a); Check3 (b);
   bool diff (a->color () < b->color ());
   if (!diff)
      diff = a->number () < b->number ();
   return diff;
}

/*--------------------------------------------------------------------------*/
//Purpose   : Sorts the cards in the pile without regard of the color
//Parameters: a: Card to compare
//            b: Card to compare
//Returns   : bool: True, if a < b
/*--------------------------------------------------------------------------*/
bool ICardPile::compCardsByNr (const CardWidget* a, const CardWidget* b) {
   Check3 (a); Check3 (b);
   TRACE9 ("ICardPile::compCardsByNr (const CardWidget*, const CardWidget*) - "
           << a->number () << " <-> " << b->number ());
   return a->number () < b->number ();
}

/*--------------------------------------------------------------------------*/
//Purpose   : Sorts the cards in the pile without regard of the number
/*--------------------------------------------------------------------------*/
void ICardPile::sortByNumber () {
   sort (cards.begin (), cards.end (), compCardsByNr);

#if CHECK > 0
   vector<CardWidget*>::const_iterator i (cards.begin ());
   if (i != cards.end ())
      Check (*i);

   for (++i; i < cards.end (); ++i) {
      Check (*i); Check ((*i)->number () >= i[-1]->number ());
   }
#endif
}

/*--------------------------------------------------------------------------*/
//Purpose   : Sorts the cards in the pile without regard of the number
/*--------------------------------------------------------------------------*/
void ICardPile::sortByColor () {
   sort (cards.begin (), cards.end (), compCards);

#if CHECK > 0
   vector<CardWidget*>::const_iterator i (cards.begin ());
   if (i != cards.end ())
      Check (*i);

   for (++i; i < cards.end (); ++i) {
      Check (*i);
      Check (((*i)->color () >= i[-1]->color ())
             || (((*i)->color () == i[-1]->color ())
                 && ((*i)->number () >= i[-1]->number ())));
   }
#endif
}

/*--------------------------------------------------------------------------*/
//Purpose   : Finds the first card being equal or bigger than the past one
//Parameters: nr: Number of card (2, 3, 4, ... Ace) to search for
//Returns   : int: Position of card in pile (or -1, if none found)
//Requires  : Cards must be sorted (as the search is binary)
/*--------------------------------------------------------------------------*/
int ICardPile::findFirstEqualOrBigger (CardWidget::NUMBERS nr) const {
   unsigned int first (0), last (cards.size ());
   unsigned int middle;

   TRACE8 ("ICardPile::findFirstEqualOrBigger (CardWidget::NUMBERS) - Searching for "
           << nr << " in " << cards.size () << " cards");

   while ((last - first) > 0 ) {
      middle = first + ((last - first) >> 1);

      TRACE5 ("ICardPile::findFirstEqualOrBigger (CardWidget::NUMBERS) - Data = ["
              << first << "-(" << middle << ")-" << last << ") = "
              << *cards[middle]);

      Check3 (cards[first]); Check3 (cards[middle]);
      Check3 (cards[first]->number () <= cards[middle]->number ());
      Check3 ((last == cards.size ()) ? 1
              : cards[last] && (cards[middle]->number () <= cards[last]->number ()));

      if (cards[middle]->number () < nr)
         first = middle + 1;
      else
         last = middle;

      Check3 (middle >= 0); Check3 (middle <= cards.size ());
      Check3 (first >= 0); Check3 (last <= cards.size ());
      Check3 (first <= last); Check3 (middle <= last);
   }

#if TRACELEVEL > 4
   TRACE ("ICardPile::findFirstEqualOrBigger (CardWidget::NUMBERS) - End = ["
          << first << "-(" << middle << ")-" << last << ')');
   if (first < cards.size ()) {
      TRACE ("\t-> " << *cards[first]);
   }
   else
      TRACE ("\t-> Not found");
#endif

   return ((first < cards.size ()) && ((cards[first]->number () >= nr))
           ? static_cast<int> (first) : -1);
}

/*--------------------------------------------------------------------------*/
//Purpose   : Finds the last card having an equal number as the passed card
//Parameters: pos: Card whose (equal) number has to be found
//Returns   : int: Position of card in pile
//Requires  : Cards must be sorted (as the search is binary)
/*--------------------------------------------------------------------------*/
int ICardPile::findLastEqual (unsigned int pos) const {
   Check3 (pos < cards.size ());

   CardWidget::NUMBERS nr (cards[pos]->number ());
   while (++pos < cards.size ()) {
      if (cards[pos]->number () != nr)
         break;
   }

   TRACE5 ("ICardPile::findLastEqual (CardWidget::NUMBERS) - Card "
           << *cards[pos - 1] << " at position " << pos - 1);
   return pos - 1;
}

/*--------------------------------------------------------------------------*/
//Purpose   : Makes the staple display either back or faces of the cards
//Parameters: show: Option of how to display the cards
/*--------------------------------------------------------------------------*/
void ICardPile::setShowOption (ShowOpt show) {
   showOpt = show;

   if (showOpt < DONT_CHANGE) {
      vector<CardWidget*>::iterator i;

      for (i = cards.begin (); i != cards.end (); ++i) {
         Check3 (*i);
         (*i)->setVisible (showOpt);
      }
   }
}
