#ifndef REMOTEHUMAN_H
#define REMOTEHUMAN_H

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


#include "Player.h"


// Forward declarations
class Socket;


/**Implementation of a player on a (remote) computer, communicating over a
   network.
*/
class RemotePlayer : public Player {
 public:
   /// Constructor; taking the socket for the communication and a name of the
   /// player
   RemotePlayer (Socket* socket, const Glib::ustring& name) : Player (name)
       , sock (socket) { }
   virtual ~RemotePlayer ();

   virtual bool makeTurn (Game* game);

 protected:
   Socket* sock;

 private:
   RemotePlayer ();
   RemotePlayer (const RemotePlayer& other);
   const RemotePlayer& operator= (const RemotePlayer& other);
};

#endif
