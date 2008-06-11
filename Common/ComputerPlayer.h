#ifndef COMPUTERPLAYER_H
#define COMPUTERPLAYER_H

//$Id$

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


#include "Player.h"


/**Implementation of a computer player.
*/
class ComputerPlayer : public Player {
 public:
   /// Constructor from a name
   ComputerPlayer (const Glib::ustring& name) : Player (name) { }
   virtual ~ComputerPlayer ();

   virtual bool makeTurn (Game* game);
   virtual unsigned int timeout () const;

   static unsigned int TIMEOUT;

 private:
   ComputerPlayer ();
   ComputerPlayer (const ComputerPlayer& other);
   const ComputerPlayer& operator= (const ComputerPlayer& other);
};

#endif
