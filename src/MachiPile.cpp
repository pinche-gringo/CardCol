//$Id$

//PROJECT     : Cardgames
//SUBSYSTEM   : MachiavelliPile
//REFERENCES  :
//TODO        :
//BUGS        :
//REVISION    : $Revision$
//AUTHOR      : Markus Schwab
//CREATED     : 05.11.2003
//COPYRIGHT   : Anticopyright (A) 2003

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


#include <cardgames-cfg.h>

#include <YGP/Check.h>
#include <YGP/Trace.h>
#include <YGP/ANumeric.h>

#include "MachiPile.h"


//-----------------------------------------------------------------------------
/// Default constructor
//-----------------------------------------------------------------------------
MachiPile::MachiPile () : CardHPile (COMPRESSED, SHOWFACE) {
   type = UNDEFINED;
}

//-----------------------------------------------------------------------------
/// Destructor
//-----------------------------------------------------------------------------
MachiPile::~MachiPile () {
}


//----------------------------------------------------------------------------
/// Sets a new top card of the pile.
/// \param newCard: Card to set as uppermost card of the pile
/// \pre \c newCard must be a valid card
//----------------------------------------------------------------------------
void MachiPile::setTopCard (CardWidget& newCard) {
   TRACE9 ("MachiPile::setTopCard (CardWidget&) - " << newCard);
   CardHPile::setTopCard (newCard);
   analyzePile ();
}

//----------------------------------------------------------------------------
/// Inserts a card into the pile.
/// \param card: Card to insert into the pile
/// \param pos: Zero-based offset of where to insert the card
/// \pre \c newCard must be a valid card
//----------------------------------------------------------------------------
void MachiPile::insert (CardWidget& card, unsigned int pos) {
   TRACE9 ("MachiPile::insert (CardWidget&, unsigned int) - " << card
           << " to " << pos);
   CardHPile::insert (card, pos);
   analyzePile ();
}


//----------------------------------------------------------------------------
/// Removes the passed card from the pile.
/// \param card: Card to remove from the pile
/// \param pos: Zero-based offset of where to insert the card
/// \pre \c newCard must be a valid card
//----------------------------------------------------------------------------
CardWidget& MachiPile::remove (CardWidget& card) {
   CardWidget& rcard (CardHPile::remove (card));
   analyzePile ();
   return rcard;
}

//----------------------------------------------------------------------------
/// Removes the passed card from the pile.
/// \param card: Card to remove from the pile
/// \param pos: Zero-based offset of where to insert the card
/// \pre \c newCard must be a valid card
//----------------------------------------------------------------------------
CardWidget& MachiPile::remove (CardWidget& card, bool visible) {
   CardWidget& rcard (CardHPile::remove (card, visible));
   analyzePile ();
   return rcard;
}

//----------------------------------------------------------------------------
/// Removes the passed card from the pile.
/// \param card: Card to remove from the pile
/// \param pos: Zero-based offset of where to insert the card
/// \pre \c newCard must be a valid card
//----------------------------------------------------------------------------
CardWidget& MachiPile::remove (unsigned int pos) {
   TRACE9 ("MachiPile::remove (unsigned int) - " << pos);
   CardWidget& card (CardHPile::remove (pos));
   analyzePile ();
   return card;
}

//----------------------------------------------------------------------------
/// Removes the passed card from the pile.
/// \param card: Card to remove from the pile
/// \param pos: Zero-based offset of where to insert the card
/// \pre \c newCard must be a valid card
//----------------------------------------------------------------------------
CardWidget& MachiPile::remove (unsigned int pos, bool visible) {
   CardWidget& card (CardHPile::remove (pos, visible));
   analyzePile ();
   return card;
}

//----------------------------------------------------------------------------
/// Returns the position in the pile where the card can be played to.
/// - Card played on an empty pile -> Valid
/// - Check if the card "fits": Either the same number as the other (first and
///   last) card, or the same colour and the number in serie.
/// \param card: Card to inspect. 
/// \returns unsigned int: Position of card in pile or -1U
/// \pre Coloured piles must be sorted strict ascending
//----------------------------------------------------------------------------
unsigned int MachiPile::getPosition4Card (const CardWidget& card) const {
   TRACE1 ("MachiPile::getPosition4Card (const CardWidget&) const - " << card);

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
   TRACE9 ("MachiPile::getPosition4Card (const CardWidget&) const - Checking colour");
   if (operator[] (0)->colour () == card.colour ()) {
      CardWidget* cmp (operator[] (0));
      if (cardDistance (card, *cmp,
                        ((cmp->number () == CardWidget::ACE)
                         && (size () > 1) ? ONE : BOTH)) == -1)
         return 0;
      
      cmp = operator[] (size () - 1);
      if (cardDistance (card, *cmp,
                        ((cmp->number () == CardWidget::ACE)
                         && (size () > 1) ? ACE : BOTH)) == 1)
         return size ();
   }

   return -1U;
}

//----------------------------------------------------------------------------
/// Returns the distance between two cards. The ace also counts as one (if the
/// other card is a 2 or a 3)
/// \param a: Card to compare
/// \param b: Card to compare
/// \param aceIsOne: Flag, if aces should (also) be treated as one
/// \returns \c int: Distance of the two passed cards (a - b)
//----------------------------------------------------------------------------
int MachiPile::cardDistance (const CardWidget& a, const CardWidget& b,
                             ACEFLAG aceIsOne) {
   TRACE9 ("MachiPile::cardDistance (const CardWidget&, const CardWidget&, ACEFLAG) - "
           << a << "<->" << b);
   
   if (aceIsOne != ACE) {                 // Special handling of the ace like 1
      TRACE9 ("MachiPile::cardDistance (const CardWidget&, const CardWidget&, ACEFLAG) - "
               "Checking for Ace");
     if (a.number () == CardWidget::ACE) {
        if ((aceIsOne == ONE) || (b.number () < CardWidget::FOUR))
           return -static_cast<int> (b.number ()) - 1;
     }
     else if (b.number () == CardWidget::ACE)
        if ((aceIsOne == ONE) || (a.number () < CardWidget::FOUR))
           return static_cast<int> (a.number ()) + 1;
   }

   TRACE9 ("MachiPile::cardDistance (const CardWidget&, const CardWidget&, ACEFLAG) - "
           "Distance: " << a.number () - b.number ());
   return a.number () - b.number ();
}

//----------------------------------------------------------------------------
/// Analyzes the pile and stores its characteristics
//----------------------------------------------------------------------------
void MachiPile::analyzePile () {
   if (size () == 2)
      type = ((operator[] (0)->number () == operator[] (1)->number ())
              ? NUMBER : COLOUR);
   else if (size () < 2)
      type = UNDEFINED;
}

//----------------------------------------------------------------------------
/// Checks the integrity of the object
/// \throw std::string describing the error 
//----------------------------------------------------------------------------
void MachiPile::checkIntegrity () throw (Glib::ustring) {
   if (size () < 3)
      throw _("Not enough cards (must be at least 3)!");

   if (type == UNDEFINED)
      throw _("Invalid type!");

   for (const_iterator i (begin ()); (i + 1) != end (); ++i)
      if ((type == COLOUR)
          ? (((*i)->colour () != (*(i + 1))->colour ())
             || (cardDistance (**(i + 1), **i) != 1))
          : ((*i)->number () != (*(i + 1))->number ())) {
         Glib::ustring error (_("Card %1 does not fit!"));
         error.replace (error.find ("%1"), 2,
                        YGP::ANumeric::toString ((i + 1) - begin ()));
         throw error;
      }
}

//----------------------------------------------------------------------------
/// Checks if this has a card matching to the ones passed in pair
/// \param pair: Pile holding the pair to match
/// \param match: Set to
///    - Position of the card which matches the pair (if this card can be
///      played directly)
///    - Position where the pile has to split, so that the matching card can be
///      played
/// \param nr: Number of cards which has to be moved (1, if the card can be
///    played directly, else the number of cards to move to "free" the matching
///    one)
/// \return bool: True, if a matching card can be found
/// \remarks \c match and \c nr might be changed, even if no matching card is
///     found!
//----------------------------------------------------------------------------
bool MachiPile::hasMatching3rd (ICardPile& pair, MachiPile::const_iterator& match,
                               unsigned int& nr) const {
   Check1 (pair.size () == 2);
   CardWidget *card (operator[] (0));
   int diff (cardDistance (*pair[1], *pair[0]));
   int diffTable (cardDistance (*pair[0], *card,
                                (diff < 0) ? ONE
                                : ((pair[0]->number () == CardWidget::ACE)
                                   && (pair[1]->number () < 5)) ? ACE : BOTH));
   TRACE1 ("MachiPile::hasMatching3rd (ICardPile&) - Differences: "
           << diff << '/' << diffTable);

   if (diff < 0) {
      diff = -diff;
      diffTable = --diffTable;
   }

   nr = 1;
   match = end ();
   switch (diff) {
   case 0:                                                     // Equal numbers
      if (diffTable) {
         if ((diffTable > 2)
             && (getType () == COLOUR)
             && (((int)(size () - 4) > diffTable)
                 || ((int)(size () - 1) == diffTable))
             && (Check3 (diffTable < (int)size ()),
                 pair.find (operator[] (diffTable)->id ()) == -1)) {
            match = begin () + diffTable;
            nr = end () - match;
            if ((int)(size () - 4) > diffTable)
               pair.clear ();
         }
      }
      else {
         if (getType () == NUMBER) {
            match = begin ();
            while (match != end ()) {
               if (pair.find ((*match)->id ()) == -1)
                  break;
               ++match;
            }
         }
         else
            if (pair.find (card->id ()) == -1)
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

