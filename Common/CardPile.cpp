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

#define DEBUG 0
#include <Check.h>

#include "CardPile.h"


static const int SIZE_COMPRESSED = 17;


/*--------------------------------------------------------------------------*/
//Purpose   : Constructor; adds all controls to the dialog
//Parameters: set: Specifier for type of cardset
//            access: Pile (topmost card) can be accessed
/*--------------------------------------------------------------------------*/
ICardPile::ICardPile (Style s, bool access) : style (s), accessable (access) {
   TRACE3 ("ICardPile::ICardPile (Style) - " << (int)style);
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
   TRACE5 ("ICardPile::setTopCard (CardWidget&) - New size: " << cards.size () + 1);

   if ((style > NORMAL) && cards.size ()) {
      Check3 (cards[cards.size () - 1]);
      resize (getTopCard (), style);
   }

   card.set_sensitive (accessable);
   card.show ();
   cards.push_back (&card);
}

/*--------------------------------------------------------------------------*/
//Purpose   : Returns and removes the top card of the pile
//Returns   : CardWidget&: Reference to (removed) card
/*--------------------------------------------------------------------------*/
CardWidget& ICardPile::removeTopCard () {
   TRACE5 ("ICardPile::removeTopCard () - New size: " << cards.size () - 1);

   Check3 (cards.size () > 0); Check3 (cards[cards.size () - 1]);
   CardWidget& card (getTopCard ());
   cards.pop_back ();

   if (cards.size () && (style > NORMAL)) {
      TRACE9 ("ICardPile::removeTopCard () - Resizing");

      CardWidget& card (getTopCard ());
      resize (card, NORMAL);
      card.set_sensitive (accessable);
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
//Purpose   : Returns the card with the passed ID
//Parameters: id: ID of card to return
//Returns   : CardWidget*: Pointer to card with passed ID (or NULL)
/*--------------------------------------------------------------------------*/
void ICardPile::setAccessable (bool access) {
   accessable = access;
   if (cards.size ())
      getTopCard ().set_sensitive (access);
}

/*--------------------------------------------------------------------------*/
//Purpose   : Inserts a card into the pile
//Parameters: card: Card to insert
/*--------------------------------------------------------------------------*/
void ICardPile::insert (CardWidget& card, unsigned int pos) {
   TRACE5 ("ICardPile::insertCard (CardWidget&) - " << pos);
   Check3 (pos <= cards.size ());

   card.show ();

   if ((style > NORMAL) && cards.size ()) {     // Cards to display compressed?
      CardWidget* compressCard (pos == cards.size ()
                                ? cards[cards.size () - 1] : &card);
      Check3 (compressCard);
      resize (*compressCard, COMPRESSED);
   }
   cards.insert (cards.begin () + pos, &card);
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
void ICardPile::setStyle (Style s) {
   TRACE5 ("ICardPile::setStyle (Style) - Size = " << cards.size ());

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

#if DEBUG > 0
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

#if DEBUG > 0
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
//Purpose   : Checks if the passed card (internal number!) exists
//Parameters: nr: Number of card (2, 3, 4, ... Ace) to search for
//Returns   : bool: True if found
/*--------------------------------------------------------------------------*/
bool ICardPile::exists (CardWidget::NUMBERS nr) const {
   unsigned int first (0), last (cards.size ());
   unsigned int middle;

   while ((last - first) > 0 ) {
      middle = first + ((last - first) >> 1);

      TRACE5 ("ICardPile::exists (CardWidget::NUMBERS) - Data = [" << first << "-("
              << middle << ")-" << last << ')');

      Check3 (cards[first]); Check3 (cards[middle]);
      Check3 (cards[first]->number () <= cards[middle]->number ());
      Check3 ((last == cards.size ()) ? 1
              : cards[last] && (cards[middle]->number () <= cards[last]->number ()));

      if (compNr (cards[middle], nr))
         first = middle + 1;
      else
         last = middle;

      // Perform sanity-checks; don't wory if DEBUG is not defined or less then
      // 3 this produces no code
      Check3 (middle >= 0); Check3 (middle <= cards.size ());
      Check3 (first >= 0); Check3 (last <= cards.size ());
      Check3 (first <= last); Check3 (middle <= last);
   }
   return (first != last) && !compNr (cards[first], nr);
}

/*--------------------------------------------------------------------------*/
//Purpose   : Checks if the passed card has the specified value
//Parameters: nr: Number of card (2, 3, 4, ... Ace) to search for
//            card: Pointer to card to analyze
//Returns   : bool: True if if is smaller
/*--------------------------------------------------------------------------*/
bool ICardPile::compNr (const CardWidget* card, CardWidget::NUMBERS nr) {
   Check3 (card);
   TRACE5 ("ICardPile::compNr (const CardWidget*, CardWidget::NUMBERS) - "
           << card->number () << " <-> " << nr);
   return card->number () < nr;
}
