//PROJECT     : Cardgames
//SUBSYSTEM   : Machiavelli
//REFERENCES  :
//TODO        :
//BUGS        :
//AUTHOR      : Markus Schwab
//CREATED     : 05.11.2003
//COPYRIGHT   : Copyright (C) 2003 - 2006, 2008, 2009, 2024, 2026

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


#include <cardgames-cfg.h>

#include <YGP/Check.h>
#include <YGP/Trace.h>
#include <YGP/ANumeric.h>

#include "MachiPile.h"


//-----------------------------------------------------------------------------
/// Default constructor
//-----------------------------------------------------------------------------
MachiPile::MachiPile () : Card::HPile (COMPRESSED, SHOWFACE), type (UNDEFINED) {
}

//-----------------------------------------------------------------------------
/// Destructor
//-----------------------------------------------------------------------------
MachiPile::~MachiPile () {
}


//----------------------------------------------------------------------------
/// Sets a new top card of the pile.
/// \param newCard Card to set as uppermost card of the pile
/// \pre \c newCard must be a valid card
//----------------------------------------------------------------------------
void MachiPile::setTopCard (Card::Widget& newCard) {
   TRACE9 ("MachiPile::setTopCard (Card::Widget&) - " << newCard);
   Card::HPile::setTopCard (newCard);
   analysePile ();
}

//----------------------------------------------------------------------------
/// Inserts a card into the pile.
/// \param card Card to insert into the pile
/// \param pos Zero-based offset of where to insert the card
/// \pre \c newCard must be a valid card
/// \returns unsigned int Position where card was inserted
//----------------------------------------------------------------------------
unsigned int MachiPile::insert (Card::Widget& card, unsigned int pos) {
   TRACE9 ("MachiPile::insert (Card::Widget&, unsigned int) - " << card
           << " to " << pos);
   unsigned int rc (Card::HPile::insert (card, pos));
   analysePile ();
   return rc;
}


//----------------------------------------------------------------------------
/// Removes the passed card from the pile.
/// \param card Card to remove from the pile
/// \param pos Zero-based offset of where to insert the card
/// \pre \c newCard must be a valid card
//----------------------------------------------------------------------------
Card::Widget& MachiPile::remove (Card::Widget& card) {
   Card::Widget& rcard (Card::HPile::remove (card));
   analysePile ();
   return rcard;
}

//----------------------------------------------------------------------------
/// Removes the passed card from the pile.
/// \param card Card to remove from the pile
/// \param pos Zero-based offset of where to insert the card
/// \pre \c newCard must be a valid card
//----------------------------------------------------------------------------
Card::Widget& MachiPile::remove (Card::Widget& card, bool visible) {
   Card::Widget& rcard (Card::HPile::remove (card, visible));
   analysePile ();
   return rcard;
}

//----------------------------------------------------------------------------
/// Removes the passed card from the pile.
/// \param card Card to remove from the pile
/// \param pos Zero-based offset of where to insert the card
/// \pre \c newCard must be a valid card
//----------------------------------------------------------------------------
Card::Widget& MachiPile::remove (unsigned int pos) {
   TRACE9 ("MachiPile::remove (unsigned int) - " << pos);
   Card::Widget& card (Card::HPile::remove (pos));
   analysePile ();
   return card;
}

//----------------------------------------------------------------------------
/// Removes the passed card from the pile.
/// \param card Card to remove from the pile
/// \param pos Zero-based offset of where to insert the card
/// \pre \c newCard must be a valid card
//----------------------------------------------------------------------------
Card::Widget& MachiPile::remove (unsigned int pos, bool visible) {
   Card::Widget& card (Card::HPile::remove (pos, visible));
   analysePile ();
   return card;
}

//----------------------------------------------------------------------------
/// Returns the position in the pile where the card can be played to.
/// - Card played on an empty pile -> Valid
/// - Check if the card "fits": Either the same number as the other (first and
///   last) card, or the same colour and the number in serie.
/// \param card Card to inspect.
/// \returns unsigned int Position of card in pile or -1U
/// \pre Coloured piles must be sorted strict ascending
//----------------------------------------------------------------------------
unsigned int MachiPile::getPosition4Card (const Card::Widget& card) const {
   TRACE1 ("MachiPile::getPosition4Card (const Card::Widget&) const - " << card);

   if (empty ())
      return 0;

   // First test numbered piles (btw. undefined piles)
   if (type != COLOUR) {
      if ((operator[] (0)->number () == card.number ())
          && (find (card.id ()) == -1))
         return size ();
      else if (type == NUMBER)
         return -1U;
   }

   // Now check for matching colour
   TRACE9 ("MachiPile::getPosition4Card (const Card::Widget&) const - Checking colour");
   if (operator[] (0)->colour () == card.colour ()) {
      Card::Widget* cmp (operator[] (0));
      if (cardDistance (card, *cmp,
                        ((cmp->number () == Card::Widget::ACE)
                         && (size () > 1) ? ONE : BOTH)) == -1)
         return 0;

      cmp = operator[] (size () - 1);
      if (cardDistance (card, *cmp,
                        ((cmp->number () == Card::Widget::ACE)
                         && (size () > 1) ? ACE : BOTH)) == 1)
         return size ();
   }

   return -1U;
}

//----------------------------------------------------------------------------
/// Returns the distance between two cards. The ace also counts as one (if the
/// other card is a 2 or a 3)
/// \param a Card to compare
/// \param b Card to compare
/// \param aceIsOne Flag, if aces should (also) be treated as one
/// \returns int Distance of the two passed cards (a - b)
//----------------------------------------------------------------------------
int MachiPile::cardDistance (const Card::Widget& a, const Card::Widget& b,
                             ACEFLAG aceIsOne) {
   TRACE9 ("MachiPile::cardDistance (2x const Card::Widget&, ACEFLAG) - "
           << a << "<->" << b);

   if (a.colour () != b.colour ())
      return (a.number () == b.number ()) ? 0 : 99;

   if (aceIsOne != ACE) {                 // Special handling of the ace like 1
      TRACE9 ("MachiPile::cardDistance (2x const Card::Widget&, ACEFLAG) - "
               "Checking for Ace");
     if (a.number () == Card::Widget::ACE) {
        if ((aceIsOne == ONE)
            || ((b.number () < Card::Widget::FOUR)
                && (b.number () != Card::Widget::ACE)))
           return -static_cast<int> (b.number ()) - 1;
     }
     else if (b.number () == Card::Widget::ACE)
        if ((aceIsOne == ONE)
            || ((a.number () < Card::Widget::FOUR)
                && (a.number () != Card::Widget::ACE)))
           return static_cast<int> (a.number ()) + 1;
   }

   TRACE9 ("MachiPile::cardDistance (2x const Card::Widget&, ACEFLAG) - "
           "Distance: " << a.number () - b.number ());
   return a.number () - b.number ();
}

//----------------------------------------------------------------------------
/// Analyses the pile and stores its characteristics
//----------------------------------------------------------------------------
void MachiPile::analysePile () {
   if (size () == 2)
      type = ((operator[] (0)->number () == operator[] (1)->number ())
              ? NUMBER : COLOUR);
   else if (size () < 2)
      type = UNDEFINED;
}

//----------------------------------------------------------------------------
/// Checks the integrity of the object
//----------------------------------------------------------------------------
void MachiPile::checkIntegrity () const {
   if (size () < 3)
      throw PileError (_("Not enough cards (must be at least 3)!"));

   if (type == UNDEFINED)
      throw PileError (_("Invalid type!"));

   for (const_iterator i (begin ()); (i + 1) != end (); ++i)
      if ((type == COLOUR)
          ? (((*i)->colour () != (*(i + 1))->colour ())
             || (cardDistance (**(i + 1), **i,
                               (i == begin ()) ? ONE : ACE) != 1))
          : ((*i)->number () != (*(i + 1))->number ())) {
         Glib::ustring error (_("Card %1 does not fit!"));
         error.replace (error.find ("%1"), 2,
                        YGP::ANumeric::toString ((i + 1) - begin ()));
         throw PileError (error);
      }
}

//-----------------------------------------------------------------------------
/// Returns the position of the first card having the passed colour
/// \param colour Colour to find
/// \returns int Position of card or -1
//-----------------------------------------------------------------------------
int MachiPile::getPosOfColour (Card::Widget::COLOURS colour) const {
   TRACE9 ("MachiPile::getPosOfColour (Card::Widget::COLOURS) - " << colour);
   Check3 (size ());

   if (type == NUMBER) {
      for (const_iterator i (begin ()); i != end (); ++i)
	 if ((*i)->colour () == colour)
	    return i - begin ();
      return -1;
   }
   else
      return (operator[] (0)->colour () == colour) ? 0 : -1;
}

//----------------------------------------------------------------------------
/// Checks if this has a card matching to the ones passed in pair
/// \param pair Vector holding the pair to match
/// \param match Set to
///    - Position of the card which matches the pair (if this card can be
///      played directly)
///    - Position where the pile has to split, so that the matching card can be
///      played
/// \param nr Number of cards which have to be moved (1, if the card can be
///    played directly, else the number of cards to move to "free" the matching
///    one)
/// \return bool True, if a matching card can be found
/// \remarks \c match and \c nr might be changed, even if no matching card is
///     found!
//----------------------------------------------------------------------------
bool MachiPile::hasMatching3rd (std::vector<Card::Widget*>& pair, MachiPile::const_iterator& match,
				unsigned int& nr) const {
   Check1 (pair.size () == 2);
   Card::Widget *card (operator[] (0));

   // For numbered pile find right colour (if pair has the same colour)
   if ((type == NUMBER) && (pair[0]->colour () == pair[1]->colour ())) {
      int pos (getPosOfColour (pair[1]->colour ()));
      if (pos >= 0)
	 card = operator[] (pos);
   }

   int diff (cardDistance (*pair[1], *pair[0]));
   int diffTable (cardDistance (*pair[0], *card,
                                (diff < 0) ? ONE
                                : ((pair[0]->number () == Card::Widget::ACE)
                                   && (pair[1]->number () < 5)) ? ACE : BOTH));
   TRACE1 ("MachiPile::hasMatching3rd (ICardPile&) - Differences: "
           << diff << '/' << diffTable);

   if (diff < 0) {
      diff = -diff;
      --diffTable;
   }

   nr = 1;
   match = end ();
   switch (diff) {
   case 0:                                                     // Equal numbers
      if (diffTable) {
         if ((diffTable > 2)
             && (type == COLOUR)
             && (((int)(size () - 4) > diffTable)
                 || ((int)(size () - 1) == diffTable))
	     && (pair[0]->id () != operator[] (diffTable)->id ())
	     && (pair[1]->id () != operator[] (diffTable)->id ())) {
            match = begin () + diffTable;
            nr = end () - match;
            if (((int)(size () - 4) > diffTable)
		&& ((int)(size () - 1) != diffTable))
               pair.clear ();
         }
      }
      else {
         if (getType () == NUMBER) {
            match = begin ();
            while (match != end ()) {
	       if ((pair[0]->id () != (*match)->id ())
		   && (pair[1]->id () != (*match)->id ()))
                  break;
               ++match;
            }
         }
         else
	    if ((pair[0]->id () != card->id ())
		&& (pair[1]->id () != card->id ()))
               match = begin ();
      }
      break;

   case 1:
      TRACE9 ("MachiPile::hasMatching3rd (ICardPile&) - Checking " << *pair[0]
              << '-' << *pair[1] << '-' << *card);
      if ((pair[0]->colour () == card->colour ())
          && (getType () == COLOUR)) {
         Check3 (pair[1]->colour () == card->colour ());
         if ((diffTable == -2) || (diffTable == 1))
            match = begin ();
         else if (((size () - diffTable) == 3) || (diffTable == (int)size ()))
            match = end () - 1;
         else {
            bool bot (diffTable > 3);
            bool top (diffTable < (int)(size () - 7));
            if ((size () > 6) && bot & top) {
               pair.clear ();
               match = begin () + (bot ? diffTable : (diffTable + 3));
               nr = end () - match;
            }
         }
      }
      break;

   case 2:
      if ((pair[0]->colour () == card->colour ()) && (diffTable < 0)
          && (getType () == COLOUR)) {
         Check3 (pair[1]->colour () == card->colour ());
         if ((diffTable == 1)
             || ((size () - 1) == (unsigned int)diffTable))
            match = begin () + diffTable - 1;
         else {
            bool bot (diffTable > 3);
            bool top (diffTable < (int)(size () - 7));
            if ((size () > 6) && bot & top) {
               pair.clear ();
               match = begin () + (bot ? diffTable : (diffTable + 3));
               nr = end () - match;
            }
         }
      }
      break;

   default:
      Check3 (0);
   } // end-switch

   TRACE5 ("MachiPile::hasMatching3rd (ICardPile&) - Match "
           << ((match != end ()) ? 'Y' : 'N') << "; " << nr);
   return match != end ();
}

//----------------------------------------------------------------------------
/// Marks all cards in the pile
//----------------------------------------------------------------------------
void MachiPile::mark () const {
   for (MachiPile::const_iterator i (begin ()); i != end (); ++i)
      (*i)->mark ();
}

//----------------------------------------------------------------------------
/// Unmarks all cards in the pile
//----------------------------------------------------------------------------
void MachiPile::unmark () const {
   for (MachiPile::const_iterator i (begin ()); i != end (); ++i)
      (*i)->unmark ();
}

//----------------------------------------------------------------------------
/// Unmarks all cards in the pile
//----------------------------------------------------------------------------
void MachiPile::markValidity () const {
   try {
      checkIntegrity ();
      unmark ();
   }
   catch (PileError&) {
      mark ();
   }
}
