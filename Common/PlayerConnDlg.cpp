//$Id$

//PROJECT     : Cardgames
//SUBSYSTEM   : Common
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

#include <gtkmm/label.h>
#include <gtkmm/table.h>
#include <gtkmm/messagedialog.h>

#include <cardgames-cfg.h>

#define CHECK 9
#define TRACELEVEL 9
#include <Check.h>
#include <Trace_.h>
#include <Socket.h>
#include <ConnMgr.h>
#include <AttrParse.h>
#include <XAttribute.h>

#include <RemotePlayer.h>

#include "PlayerConnDlg.h"


//-----------------------------------------------------------------------------
/// Default constructor
/// \param player: The player
/// \param defPort: Default port to listen at/send to
/// \param connMgr: Connection manager; holding the connections to use
//-----------------------------------------------------------------------------
PlayerConnectDlg::PlayerConnectDlg (std::vector<Player*>& player,
                                    const Glib::ustring& port,
                                    ConnectionMgr& cmgr)
    : ConnectDlg (player.size (), port, cmgr), aPlayer (player)
      , connected (new Gtk::Label ())
      , lblConnected (new Gtk::Label (_("Connected:"))) {
   TRACE8 ("PlayerConnectDlg::PlayerConnectDlg (std::vector<Player*>&, "
           "const Glib::ustring&, ConnectionMgr&");

   pClient->resize (3, 4);
   pClient->attach (*lblConnected, 0, 1, 3, 4, Gtk::SHRINK, Gtk::FILL, 5, 3);
   pClient->attach (*connected, 1, 2, 3, 4, Gtk::FILL | Gtk::EXPAND,
                    Gtk::SHRINK, 5, 3);

   lblConnected->set_alignment (0, 0);
   connected->set_alignment (0, 0);
   lblConnected->show ();
   connected->show ();
}

//-----------------------------------------------------------------------------
/// Destructor
//-----------------------------------------------------------------------------
PlayerConnectDlg::~PlayerConnectDlg () {
}


//----------------------------------------------------------------------------
/// Performs the dialog (modal)
/// \param player: The player
/// \param defPort: Default port to listen at/send to
/// \param connMgr: Connection manager; holding the connections to use
//----------------------------------------------------------------------------
void PlayerConnectDlg::perform (std::vector<Player*>& player, unsigned int defPort,
                                ConnectionMgr& connMgr) {
   std::ostringstream port;
   port << defPort;
   return perform (player, port.str (), connMgr);
}

//----------------------------------------------------------------------------
/// Performs the dialog (modal)
/// \param player: The player
/// \param defPort: Default port to listen at/send to
/// \param connMgr: Connection manager; holding the connections to use
//----------------------------------------------------------------------------
void PlayerConnectDlg::perform (std::vector<Player*>& player, const Glib::ustring& defPort,
                                ConnectionMgr& connMgr) {
   PlayerConnectDlg* dlg (new PlayerConnectDlg (player, defPort, connMgr));
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

   Glib::ustring data ("Version=" PROTOCOLL ";Variant=" VARIANT ";Name=\""
                       + aPlayer[0]->getName () + '"');
   try {
      cmgr.getSocket ()->write (data);

      std::string input;
      cmgr.getSocket ()->read (input);
      TRACE8 ("PlayerConnectDlg::connect (const Glib::ustring&, unsigned int) - "
              "Received: " << input);
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

   Glib::ustring error;
   try {
      std::string input;
      sock->read (input);
      TRACE8 ("PlayerConnectDlg::addClient (int) - Received: " << input);

      Glib::ustring name;
      unsigned int protocoll, variant;
      AttributeParse ap;
      ATTRIBUTE (ap, Glib::ustring, name, "Name");
      ATTRIBUTE (ap, unsigned int, protocoll, "Version");
      ATTRIBUTE (ap, unsigned int, variant, "Variant");
      ap.assignValues (input);

      TRACE8 ("PlayerConnectDlg::addClient (int) - Connected: " << name);
      Check3 (aPlayer.size () > cmgr.getClients ().size ());
      delete aPlayer[cmgr.getClients ().size ()];
      aPlayer[cmgr.getClients ().size ()] = new RemotePlayer (sock, name);
      connected->set_text (connected->get_text () + name + '\n');

      Check3 (aPlayer.size () < 10);
      input = "";
      for (std::vector<Player*>::iterator i (aPlayer.begin ());
           i != aPlayer.end (); ++i)
         input += (std::string (1, static_cast<char> (i - aPlayer.begin () + '0'))
                   + "=\"" + (*i)->getName () + "\";");

      TRACE8 ("PlayerConnectDlg::addClient (int) - Sending players: " << input);
      sock->write (input);
   }
   catch (std::domain_error& err) {
      error = _("Error getting player name!\n\nReason: %1");
      error.replace (error.find ("%1"), 2, err.what ()); 
   }
   catch (std::string& err) {
      error = _("Error analyzing input from client!\n\nReason: %1");
      error.replace (error.find ("%1"), 2, err); 
   }
   if (error.size ()) {
      Gtk::MessageDialog dlg (error, Gtk::MESSAGE_ERROR, Gtk::BUTTONS_OK);
      dlg.set_title (PACKAGE);
      dlg.run ();
   }
   return sock;
}
