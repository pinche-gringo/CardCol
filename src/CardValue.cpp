//$Id$

//PROJECT     : Cardgames
//SUBSYSTEM   : libCard
//REFERENCES  :
//TODO        :
//BUGS        :
//REVISION    : $Revision$
//AUTHOR      : Markus Schwab
//CREATED     : 02.08.2006
//COPYRIGHT   : Copyright (C) 2006, 2007

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
// along with libYGP.  If not, see <http://www.gnu.org/licenses/>.


#include <cardgames-cfg.h>

#include "CardWidget.h"

#include "CardValue.h"


CardValue* CardValue::instance (NULL);


//-----------------------------------------------------------------------------
/// Default constructor; Fills the values
//-----------------------------------------------------------------------------
CardValue::CardValue () {
   for (unsigned nr (CardWidget::THREE); nr < CardWidget::TEN; ++nr)
      insert (std::make_pair (nr, std::string (1, CardWidget::strNumber (static_cast<CardWidget::NUMBERS> (nr)))));
   insert (std::make_pair (CardWidget::TEN, _("10")));
   insert (std::make_pair (CardWidget::JACK, _("Jack")));
   insert (std::make_pair (CardWidget::QUEEN, _("Queen")));
   insert (std::make_pair (CardWidget::KING, _("King")));
   insert (std::make_pair (CardWidget::ACE, _("Ace")));
}

//-----------------------------------------------------------------------------
/// Destructor
//-----------------------------------------------------------------------------
CardValue::~CardValue () {
}
