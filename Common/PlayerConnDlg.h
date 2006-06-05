#ifndef PLAYERCONNECTDLG_H
#define PLAYERCONNECTDLG_H

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


#include <vector>

#include <XGP/ConnectDlg.h>

class Player;

/**Implementation of the connect dialog for the players of the Cardgame
   collection.

   After establishing the connection it negotiates the player names.
*/
class PlayerConnectDlg : public XGP::ConnectDlg {
 public:
   PlayerConnectDlg (std::vector<Player*>& player, const Glib::ustring& port,
                     YGP::ConnectionMgr& cmgr);
   virtual ~PlayerConnectDlg ();

   static unsigned int perform (std::vector<Player*>& player, const Glib::ustring& port,
                                YGP::ConnectionMgr& cmgr);
   static unsigned int perform (std::vector<Player*>& player, unsigned int port,
                                YGP::ConnectionMgr& cmgr);
   static unsigned int perform (std::vector<Player*>& player, YGP::ConnectionMgr& cmgr,
                                const Glib::ustring& listenAt);
   static unsigned int perform (std::vector<Player*>& player, YGP::ConnectionMgr& cmgr,
                                const Glib::ustring& host, const Glib::ustring& hostPort);

 protected:
   /// \name Connection management
   virtual void connect (const Glib::ustring& target, unsigned int port) throw (YGP::CommError);
   virtual YGP::Socket* addClient (int socket);

   Gtk::Label* connected;
   Gtk::Label* lblConnected;

 private:
   std::vector<Player*>& aPlayer;
   unsigned int posPlayer;

   PlayerConnectDlg (const PlayerConnectDlg& other);
   const PlayerConnectDlg& operator= (const PlayerConnectDlg& other);
};

#endif
