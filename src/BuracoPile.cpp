//$Id$

//PROJECT     : Cardgames
//SUBSYSTEM   : Buraco
//REFERENCES  :
//TODO        :
//BUGS        :
//REVISION    : $Revision$
//AUTHOR      : Markus Schwab
//CREATED     : 27.09.2003
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


#define CHECK 9
#define TRACELEVEL 9
#include <Check.h>
#include <Trace_.h>

#include <CardWidget.h>

#include "Buraco.h"
#include "BuracoPile.h"


//-----------------------------------------------------------------------------
/// Constructor
/// @param style: Display style of pile
/// @param show: Flag, if cards should display its face or its back
//-----------------------------------------------------------------------------
BuracoPile::BuracoPile (PileStyle style, ShowOpt show)
    : CardVPile (style, show), points (0) {
   status.type = UNDEFINED;
   status.posFirst = status.posLast = status.posJoker = -1U;
}

//-----------------------------------------------------------------------------
/// Destructor
//-----------------------------------------------------------------------------
BuracoPile::~BuracoPile () {
}


//----------------------------------------------------------------------------
/// Sets a new top card of the pile.
/// \param newCard: Card to set as uppermost card of the pile
/// \pre \c newCard must be a valid card
//----------------------------------------------------------------------------
void BuracoPile::setTopCard (CardWidget& newCard) {
   Check1 (isValid (newCard));
   CardVPile::setTopCard (newCard);
   analyzePile ();
}

//----------------------------------------------------------------------------
/// Inserts a card into the pile.
/// \param card: Card to insert into the pile
/// \param pos: Zero-based offset of where to insert the card
/// \pre \c newCard must be a valid card
//----------------------------------------------------------------------------
void BuracoPile::insert (CardWidget& card, unsigned int pos) {
   Check1 (isValid (card));
   CardVPile::insert (card, pos);
   analyzePile ();
}

//----------------------------------------------------------------------------
/// Returns the position in the pile where the card can be played to.
/// - Card played on an empty pile -> Valid (but check for fitting tripple or
///   pair with joker

/// \param card: Card to inspect. 
/// \param pos: Position of card to play, or -1U if can't be played
/// \param move: Card which needs to be moved or -1U
/// \returns bool: True, if card fits on pile
//----------------------------------------------------------------------------
bool BuracoPile::getPosition4Card (const CardWidget& card, unsigned int& pos,
                                   int& move) const {
   Check2 ((status.posJoker < size () || (status.posJoker > 6)));
   move = 0;

   // Card played on an empty pile -> Valid
   if (status.posFirst > 6) {
      pos = 0;
      return true;
   }

   Check2 (status.posFirst <= status.posLast);
   Check2 (status.posLast < size ());

   // Joker played on a pile without joker: Valid
   if ((status.posJoker > 6) && Buraco::isJoker (card)) {
      unsigned int nr (operator[] (status.posFirst)->number ());
      if (nr == operator[] (status.posLast)->number ()) {
         pos = 1;
         return true;
      }

      for (pos = status.posFirst; pos < status.posLast; ++pos)
         if (nr++ != pos)
            break;
      return true;
   }

   // Else check if the pile is a numberd or a coloured one
   TRACE5 ("BuracoPile::getPosition4Card (const CardWidget&, unsigned int&, "
           "int&) - Cards: " << *at (status.posFirst) << " and " << *at (status.posLast));
   if (operator[] (status.posFirst)->number () == card.number ()) {
       if (status.type != COLOUR) {
          pos = size ();
          return true;
       }
   }
   else
      if ((operator[] (status.posFirst)->colour () == card.colour ())
          && (status.type != NUMBER)) {
         // This code assumes that the (potentially) coloured pile is sorted
         // from lower card to higher cards (strict ascending).
         // First check, if a joker can be replaced
         if ((status.posJoker < 7)
             && (Buraco::cardDistance (card, *operator[] (status.posFirst))
                 == status.posJoker)) {
            move = ~status.posJoker;
            pos = status.posJoker + 1;
            return true;
         }

         // Possible difference the card can have: 1 or two if joker at one end
         unsigned int maxDiff ((status.posJoker > 6) ? 1
                               : (((status.posJoker < status.posFirst)
                                   || (status.posJoker > status.posLast))
                                  ? 2 : 1));
         if ((status.posFirst == status.posLast)
             || (operator[] (status.posFirst)->number () != CardWidget::ACE)) {
            unsigned int diff (Buraco::cardDistance
                               (*operator[] (status.posFirst), card,
                                operator[] (status.posFirst)->number ()
                                <= CardWidget::FOUR));
            TRACE9 ("BuracoPile::getPosition4Card (const CardWidget&, unsigned "
                    "int&, int&) - Diff: " << diff << "; max: " << maxDiff);
            Check3 (diff);

            if (diff && (diff <= maxDiff)) {
               pos = status.posFirst - diff + 1;
               if ((diff == 2) && (status.posJoker > status.posFirst)) {
                  Check3 (!status.posFirst);

                  move = ~status.posJoker;
                  ++pos;
               }
               return true;
            }
         }

         // Test if card fits at other end
         unsigned int diff (Buraco::cardDistance (card, *operator[] (status.posLast),
                                                  status.posFirst == status.posLast));
         TRACE9 ("BuracoPile::getPosition4Card (const CardWidget&, unsigned "
                 "int&, int&) - Diff (end):" << diff << "; max: " << maxDiff);
         if (diff && (diff <= maxDiff)) {
            pos = status.posLast + diff;
            if ((diff == 2) && (status.posJoker < status.posLast)) {
               Check3 (status.posFirst > status.posJoker);

               move = status.posJoker;
               --pos;
            }
            return true;
         }
      }
   return false;
}

//----------------------------------------------------------------------------
/// Analyzes the pile and stores its characteristics.
//----------------------------------------------------------------------------
void BuracoPile::analyzePile () {
   status.posFirst = status.posLast = status.posJoker = -1U;
   if (size ()) {
      for (const_iterator i (begin ()); i != end (); ++i)
         if (Buraco::isJoker (**i)) {
            Check3 ((status.posJoker > 6) || (status.posFirst > 6));
            status.posJoker = i - begin ();
         }
         else
            ((status.posFirst > 6) ? status.posFirst : status.posLast) =
                i - begin ();
   }
   if (status.posLast > 6)
      status.posLast = status.posFirst;


   if (status.posFirst != status.posLast)
      status.type = ((operator[] (status.posFirst)->number ()
                      == operator[] (status.posLast)->number ())
                     ? NUMBER : COLOUR);

   TRACE9 ("BuracoPile::analyzePile () - " << status.posJoker << '/'
           << status.posFirst << '/' << status.posLast << ' '
           << status.type);
}

//----------------------------------------------------------------------------
/// Returns the sum of the points of the cards of the pile.
/// \returns unsigned int: Points of the cards
//----------------------------------------------------------------------------
unsigned int BuracoPile::getCardPoints () const {
   unsigned int sum (0);
   for (const_iterator c (begin ()); c != end (); ++c)
      sum += Buraco::getPoints (**c);
   return sum;
}
