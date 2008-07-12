//$Id$

//PROJECT     : Cardgames
//SUBSYSTEM   : Buraco
//REFERENCES  :
//TODO        :
//BUGS        :
//REVISION    : $Revision$
//AUTHOR      : Markus Schwab
//CREATED     : 27.09.2003
//COPYRIGHT   : Copyright (C) 2003 - 2005, 2007, 2008

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


#include <YGP/Check.h>
#include <YGP/Trace.h>

#include <CardWidget.h>

#include "Buraco.h"
#include "BuracoPile.h"


//-----------------------------------------------------------------------------
/// Constructor
//-----------------------------------------------------------------------------
BuracoPile::BuracoPile ()
    : CardVPile (COMPRESSED, SHOWFACE) {
   status.type = UNDEFINED;
   status.posFirst = status.posLast = status.posJoker = 7;
   status.points = 0;
}

//-----------------------------------------------------------------------------
/// Destructor
//-----------------------------------------------------------------------------
BuracoPile::~BuracoPile () {
}


//----------------------------------------------------------------------------
/// Sets a new top card of the pile.
/// \param newCard Card to set as uppermost card of the pile
/// \pre \c newCard must be a valid card
//----------------------------------------------------------------------------
void BuracoPile::setTopCard (CardWidget& newCard) {
   TRACE9 ("BuracoPile::setTopCard (CardWidget&) - " << newCard);
   CardVPile::setTopCard (newCard);
   analyzePile ();
}

//----------------------------------------------------------------------------
/// Inserts a card into the pile.
/// \param card Card to insert into the pile
/// \param pos Zero-based offset of where to insert the card
/// \pre \c newCard must be a valid card
/// \returns unsigned int Position where card was inserted
//----------------------------------------------------------------------------
unsigned int BuracoPile::insert (CardWidget& card, unsigned int pos) {
   TRACE9 ("BuracoPile::insert (CardWidget&, unsigned int) - " << card
           << " to " << pos);
   unsigned int rc (CardVPile::insert (card, pos));
   analyzePile ();
   return rc;
}


//----------------------------------------------------------------------------
/// Removes the passed card from the pile.
/// \param card Card to remove from the pile
/// \pre \c newCard must be a valid card
//----------------------------------------------------------------------------
CardWidget& BuracoPile::remove (CardWidget& card) {
   CardWidget& rcard (CardVPile::remove (card));
   analyzePile ();
   return rcard;
}

//----------------------------------------------------------------------------
/// Removes the passed card from the pile.
/// \param card Card to remove from the pile
/// \param pos Zero-based offset of where to insert the card
/// \pre \c newCard must be a valid card
//----------------------------------------------------------------------------
CardWidget& BuracoPile::remove (CardWidget& card, bool visible) {
   CardWidget& rcard (CardVPile::remove (card, visible));
   analyzePile ();
   return rcard;
}

//----------------------------------------------------------------------------
/// Removes the passed card from the pile.
/// \param pos Zero-based offset of card to remove
/// \pre \c newCard must be a valid card
//----------------------------------------------------------------------------
CardWidget& BuracoPile::remove (unsigned int pos) {
   TRACE9 ("BuracoPile::remove (unsigned int) - " << pos);
   CardWidget& card (CardVPile::remove (pos));
   analyzePile ();
   return card;
}

//----------------------------------------------------------------------------
/// Removes the passed card from the pile.
/// \param pos Zero-based offset of card to remove
/// \param visible Flag if the card face should be shown
/// \pre \c newCard must be a valid card
//----------------------------------------------------------------------------
CardWidget& BuracoPile::remove (unsigned int pos, bool visible) {
   CardWidget& card (CardVPile::remove (pos, visible));
   analyzePile ();
   return card;
}

//----------------------------------------------------------------------------
/// Returns the position in the pile where the card can be played to.
/// - Card played on an empty pile -> Valid (but check for fitting tripple or
///   pair with joker)
/// - Joker played on a pile without joker -> Valid (but check for fitting
///    card, if the pile has only one card)
/// - Ordinary card: Check if the card "fits": Either the same number as the
///   other (first and last) card, or the same colour and the number in serie.
/// \param card Card to inspect.
/// \param pos Position of card to play, or -1U if can't be played
/// \param move Position to move joker to or -1U
/// \returns bool True, if card fits on pile
/// \pre Coloured piles must be sorted strict ascending
//----------------------------------------------------------------------------
bool BuracoPile::getPosition4Card (const CardWidget& card, unsigned int& pos,
                                   unsigned int& move) const {
   TRACE6 ("BuracoPile::getPosition4Card (const CardWidget&, 2x unsigned int&) - "
           << card);
   Check2 ((status.posJoker < size () || (status.posJoker > 6)));
   move = -1U;

   // Card played on an empty pile or a pile with only one joker -> Valid
   if ((status.posFirst > 6) && ((status.posJoker < 1) || (status.posJoker > 6))) {
      Check3 (status.type == UNDEFINED);
      pos = 0;
      return true;
   }

   // Joker played on a pile without joker: Valid
   if (Buraco::isJoker (card) && ((status.posFirst > 6) || (status.posJoker > 6))) {
      pos = ((status.type == NUMBER)
	     ? 1
	     : ((((status.posFirst < 7))
		&& (operator[] (status.posFirst)->number () == CardWidget::ACE))
		? (status.posLast + 1) : 0));
      return true;
   }
   Check2 (status.posFirst <= status.posLast);
   Check2 (status.posLast < size ());

   // Else check if the pile is a numberd or a coloured one
   TRACE5 ("BuracoPile::getPosition4Card (const CardWidget&, 2x unsigned int&) "
           "- Cards: " << *at (status.posFirst) << " and " << *at (status.posLast));
   if (operator[] (status.posFirst)->number () == card.number ()) {
       if (status.type != COLOUR) {
          pos = status.posJoker ? size () : 0;
          return true;
       }
   }
   else
      if ((operator[] (status.posFirst)->colour () == card.colour ())
          && (status.type != NUMBER)) {
         // This code assumes that the (potentially) coloured pile is sorted
         // from lower card to higher cards (strict ascending).
         // First check, if a joker in between other cards can be replaced
         if ((status.posJoker < 7)
             && (Buraco::cardDistance (card, *operator[] (status.posFirst))
                 == static_cast<int> (status.posJoker))) {
            if ((status.posJoker && (status.posJoker < status.posLast))
		|| (card.number () == CardWidget::ACE))
               move = ((((status.posJoker > status.posFirst)
                         ? operator[] (status.posFirst)->number ()
                         : card.number ()) == CardWidget::ACE)
                       ? status.posLast : 0);
            pos = status.posJoker;
            if ((move != -1U) && (move < pos))
               ++pos;
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
                                < CardWidget::FIVE));
            TRACE9 ("BuracoPile::getPosition4Card (const CardWidget&, 2x unsigned "
                    "int&) - Diff: " << diff << "; max: " << maxDiff);
            Check3 (diff);

            if (diff && (diff <= maxDiff)) {
               pos = status.posFirst - diff + 1;
	       if ((status.posJoker < 7)) {
		  if ((diff == 2)
		      ? ((status.posJoker > status.posFirst)
			 && ((operator[] (status.posFirst)->number () != CardWidget::ACE)
			     || (size () < 3)))
		      : ((card.number () == CardWidget::ACE)
			 ? (!status.posJoker && (pos == 1))
			 : ((card.number () == CardWidget::KING)
			    && (!pos && status.posJoker)))) {
		     move = ((((status.posJoker > status.posFirst)
			       ? operator[] (status.posFirst)->number ()
			       : card.number ()) == CardWidget::ACE)
			     ? status.posLast : 0);
		     move ? --pos : ++pos;
		  }
	       }
               return true;
            }
         }

         // Test if card fits at other end
         unsigned int diff (Buraco::cardDistance (card, *operator[] (status.posLast),
                                                  status.posFirst == status.posLast));
         TRACE9 ("BuracoPile::getPosition4Card (const CardWidget&, 2x unsigned "
                 "int&) - Diff (end): " << diff << "; max: " << maxDiff);
         if (diff && (diff <= maxDiff)) {
            pos = status.posLast + diff;
            if ((diff == 2) && (status.posJoker < status.posLast)) {
               Check3 (status.posFirst > status.posJoker);

               move = status.posLast;
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
   status.posFirst = status.posLast = status.posJoker = 7;
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

   status.type = ((status.posFirst != status.posLast)
		  ? ((operator[] (status.posFirst)->number ()
                      == operator[] (status.posLast)->number ())
                     ? NUMBER : COLOUR)
		  : UNDEFINED);

   TRACE9 ("BuracoPile::analyzePile () - " << status.posJoker << '/'
           << status.posFirst << '/' << status.posLast << ' '
           << status.type);

   // If there is no first "normal" card, it must be a pile of monos
   if (status.posFirst > 6) {
      if (size () > 1)        // Only set type, if there are at least two cards
	 status.type = NUMBER;
      Check2 (size () ? (status.posJoker < 7) : 1);
      status.points = 2000;

      // Correct points if it is "sucio"
      for (const_iterator i (begin ()); i != end (); ++i)
         if ((*i)->number () == CardWidget::UNREACHABLE) {
            status.points = 1000;
            break;
         }
   }
   else {
      status.points = (((status.type == NUMBER)
                        && (operator[] (status.posFirst)->number () == CardWidget::ACE))
                       ? 300 : 200);
      if (status.posJoker > 6)
         status.points += 200;
   }

   TRACE8 ("BuracoPile::analyzePile () - " << status.posFirst << '/'
	   << status.posLast << '/' << status.posJoker);
   TRACE9 ("BuracoPile::analyzePile () - Points: " << status.points
	   << ((status.type == NUMBER) ? " (NUMBER)" : " (COLOUR)"));
}

//----------------------------------------------------------------------------
/// Returns the sum of the points of the cards of the pile.
/// \returns unsigned int Points of the cards
//----------------------------------------------------------------------------
unsigned int BuracoPile::getCardPoints () const {
   unsigned int sum (0);
   for (const_iterator c (begin ()); c != end (); ++c)
      sum += Buraco::getPoints (**c);

   TRACE9 ("BuracoPile::getCardPoints () const - " << sum);
   return sum;
}
