//$Id$

//PROJECT     : Cardgames
//SUBSYSTEM   : <FILLIN>
//REFERENCES  :
//TODO        :
//BUGS        :
//REVISION    : $Revision$
//AUTHOR      : Markus Schwab
//CREATED     : 24.07.2003
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


#include <sstream>

#include <gtkmm/messagedialog.h>

#include <cardgames-cfg.h>

#define CHECK 9
#define TRACELEVEL 9
#include <Check.h>
#include <Trace_.h>
#include <Socket.h>
#include <ConnMgr.h>

#include "PlayerConnDlg.h"


//-----------------------------------------------------------------------------
/// Default constructor
/// \param names: Name of the players
/// \param defPort: Default port to listen at/send to
/// \param connMgr: Connection manager; holding the connections to use
//-----------------------------------------------------------------------------
PlayerConnectDlg::PlayerConnectDlg (std::vector<Glib::ustring>& names,
                                    const Glib::ustring& port,
                                    ConnectionMgr& cmgr)
    : ConnectDlg (names.size (), port, cmgr), players (names) {
   TRACE8 ("PlayerConnectDlg::PlayerConnectDlg (std::vector<Glib::ustring>&, "
           "const Glib::ustring&, ConnectionMgr&");
}

//-----------------------------------------------------------------------------
/// Destructor
//-----------------------------------------------------------------------------
PlayerConnectDlg::~PlayerConnectDlg () {
}


//----------------------------------------------------------------------------
/// Performs the dialog (modal)
/// \param cMaxConnections: Maximal number of connections the dialog (in
///    server mode) will wait for.
/// \returns ConnectDlg::modeConnect: Mode of the connection
/// \remarks This method cares about freeing the dialog afterwards
//----------------------------------------------------------------------------
void PlayerConnectDlg::perform (std::vector<Glib::ustring>& names, unsigned int defPort,
                                ConnectionMgr& connMgr) {
   std::ostringstream port;
   port << defPort;
   return perform (names, port.str (), connMgr);
}

//----------------------------------------------------------------------------
/// Performs the dialog (modal)
/// \param cMaxConnections: Maximal number of connections the dialog (in
///    server mode) will wait for.
/// \returns ConnectDlg::modeConnect: Mode of the connection
/// \remarks This method cares about freeing the dialog afterwards
//----------------------------------------------------------------------------
void PlayerConnectDlg::perform (std::vector<Glib::ustring>& names, const Glib::ustring& defPort,
                                ConnectionMgr& connMgr) {
   PlayerConnectDlg* dlg (new PlayerConnectDlg (names, defPort, connMgr));
   dlg->run ();
   delete dlg;
}

//----------------------------------------------------------------------------
/// Connects this application to the server
/// \param target: Name or IP address of the target
/// \param port: Port the server is listening at
//----------------------------------------------------------------------------
void PlayerConnectDlg::connect (const Glib::ustring& target, unsigned int port) {
   TRACE3 ("PlayerConnectDlg::connect (const Glib::ustring&, unsigned int) - "
           << target << ':' << port);
   ConnectDlg::connect (target, port);
   Check1 (cmgr.getSocket ());

   std::string data ("Name=\"" + players[0] + '"');
   try {
      cmgr.getSocket ()->write (data);
   }
   catch (std::domain_error& err) {
      Glib::ustring error (_("Error sending player name!\n\nReason: %1"));
      error.replace (error.find ("%1"), 2, err.what ()); 
      Gtk::MessageDialog dlg (error, Gtk::MESSAGE_ERROR, Gtk::BUTTONS_OK);
      dlg.set_title (PACKAGE);
      dlg.run ();
   }
}

//----------------------------------------------------------------------------
/// Updates the name of the player with the data send from the client
/// \param socket: Socket over which the clients communicates
//----------------------------------------------------------------------------
Socket* PlayerConnectDlg::addClient (int socket) {
   TRACE3 ("PlayerConnectDlg::addClient (int)");
   Socket* sock (ConnectDlg::addClient (socket));
   Check3 (sock);

   try {
      std::string input;
      sock->read (input);
      TRACE1 ("PlayerConnectDlg::addClient (int) - Received: " << input);
   }
   catch (std::domain_error& err) {
      Glib::ustring error (_("Error getting player name!\n\nReason: %1"));
      error.replace (error.find ("%1"), 2, err.what ()); 
      Gtk::MessageDialog dlg (error, Gtk::MESSAGE_ERROR, Gtk::BUTTONS_OK);
      dlg.set_title (PACKAGE);
      dlg.run ();
   }
   return sock;
}
