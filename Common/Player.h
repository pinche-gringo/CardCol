#ifndef PLAYER_H
#define PLAYER_H

//$Id$

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


#include <sigc++/trackable.h>
#include <glibmm/ustring.h>


class Game;


/**Abstract base class representing a player of the game.

   Every player can either be local human, remote human or computer player
*/
class Player : public sigc::trackable {
 public:
   Player (const Glib::ustring& playerName);
   virtual ~Player ();

   const Glib::ustring& getName () const { return name; }
   void setName (const Glib::ustring& playerName) { name = playerName; }

   /// Executes the turn of the player.
   /// \param game: Game playing.
   /// \returns bool: Flag, if the method should be called again in the next turn.
   virtual bool makeTurn (Game* game) = 0;

   virtual unsigned int timeout () const;

 protected:
   Glib::ustring name;

 private:
   Player (const Player& other);
   const Player& operator= (const Player& other);
};

#endif
