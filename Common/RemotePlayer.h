#ifndef REMOTEHUMAN_H
#define REMOTEHUMAN_H

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
// along with CardCol.  If not, see <http://www.gnu.org/licenses/>.


#include "Player.h"


// Forward declarations
namespace YGP {
   class Socket;
}


/**Implementation of a player on a (remote) computer, communicating over a
   network.
*/
class RemotePlayer : public Player {
 public:
   /// Constructor; taking the socket for the communication and a name of the
   /// player
   RemotePlayer (YGP::Socket* socket, const Glib::ustring& name) : Player (name)
       , sock (socket) { }
   virtual ~RemotePlayer ();

   virtual bool makeTurn (Game* game);

 protected:
   YGP::Socket* sock;

 private:
   RemotePlayer ();
   RemotePlayer (const RemotePlayer& other);
   const RemotePlayer& operator= (const RemotePlayer& other);
};

#endif
