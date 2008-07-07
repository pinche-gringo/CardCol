//$Id$

//PROJECT     : Cardgames
//SUBSYSTEM   : Common
//REFERENCES  :
//TODO        :
//BUGS        :
//REVISION    : $Revision$
//AUTHOR      : Markus Schwab
//CREATED     : 24.07.2003
//COPYRIGHT   : Copyright (C) 2003 - 2006, 2008

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


#include <sstream>

#include <gtkmm/entry.h>
#include <gtkmm/label.h>
#include <gtkmm/table.h>
#include <gtkmm/messagedialog.h>

#include <cardgames-cfg.h>

#include <YGP/Check.h>
#include <YGP/Trace.h>
#include <YGP/Socket.h>
#include <YGP/ConnMgr.h>
#include <YGP/AttrParse.h>

#include <XGP/XAttribute.h>

#include "Human.h"
#include "RemotePlayer.h"

#include "PlayerConnDlg.h"


//-----------------------------------------------------------------------------
/// Default constructor
/// \param player: The player
/// \param defPort: Default port to listen at/send to
/// \param connMgr: Connection manager; holding the connections to use
//-----------------------------------------------------------------------------
PlayerConnectDlg::PlayerConnectDlg (std::vector<Player*>& player,
                                    const Glib::ustring& port,
                                    YGP::ConnectionMgr& cmgr)
    : XGP::ConnectDlg (player.size (), port, cmgr)
      , connected (new Gtk::Label ())
      , lblConnected (new Gtk::Label (_("Connected:")))
      , aPlayer (player), posPlayer (0) {
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
/// \returns <tt>unsigned int</tt>: Number player has for the server
//----------------------------------------------------------------------------
unsigned int PlayerConnectDlg::perform (std::vector<Player*>& player, unsigned int defPort,
                                        YGP::ConnectionMgr& connMgr) {
   std::ostringstream port;
   port << defPort;
   return perform (player, port.str (), connMgr);
}

//----------------------------------------------------------------------------
/// Performs the dialog (modal)
/// \param player: The player
/// \param defPort: Default port to listen at/send to
/// \param connMgr: Connection manager; holding the connections to use
/// \returns <tt>unsigned int</tt>: Number player has for the server
//----------------------------------------------------------------------------
unsigned int PlayerConnectDlg::perform (std::vector<Player*>& player, const Glib::ustring& defPort,
                                        YGP::ConnectionMgr& connMgr) {
   unsigned int pos (0);
   PlayerConnectDlg* dlg (new PlayerConnectDlg (player, defPort, connMgr));
   dlg->run ();
   pos = dlg->posPlayer;
   delete dlg;
   return pos;
}

//----------------------------------------------------------------------------
/// Performs the dialog (modal)
/// \param player: The player
/// \param connMgr: Connection manager; holding the connections to use
/// \param listenAt: Port the server should listen at
/// \returns <tt>unsigned int</tt>: The number the player has for the server
//----------------------------------------------------------------------------
unsigned int PlayerConnectDlg::perform (std::vector<Player*>& player,
                                        YGP::ConnectionMgr& cmgr, const Glib::ustring& listenAt) {
   unsigned int pos (0);
   PlayerConnectDlg* dlg (new PlayerConnectDlg (player, "0", cmgr));
   Check3 (dlg->pPort); Check3 (dlg->pWait);
   dlg->pPort->set_text (listenAt);
   dlg->pWait->clicked ();
   dlg->run ();
   pos = dlg->posPlayer;
   delete dlg;
   return pos;
}

//----------------------------------------------------------------------------
/// Performs the dialog (modal)
/// \param player: The player
/// \param connMgr: Connection manager; holding the connections to use
/// \param host: Host the client should connect too
/// \param hostPort: Port the host is listening at
/// \returns <tt>unsigned int</tt>: The number the player has for the server
//----------------------------------------------------------------------------
unsigned int PlayerConnectDlg::perform (std::vector<Player*>& player,
                                        YGP::ConnectionMgr& cmgr, const Glib::ustring& host,
                                        const Glib::ustring& hostPort) {
   unsigned int pos (0);
   PlayerConnectDlg* dlg (new PlayerConnectDlg (player, "0", cmgr));
   Check3 (dlg->pPort); Check3 (dlg->pConnect); Check3 (dlg->pTarget);
   dlg->pTarget->set_text (host);
   dlg->pPort->set_text (hostPort);
   dlg->pConnect->clicked ();
   pos = dlg->posPlayer;
   delete dlg;
   return pos;
}

//----------------------------------------------------------------------------
/// Connects this application to a server
/// \param target: Name or IP address of the server
/// \param port: Port the server is listening at
/// \throw YGP::CommError: Does not throw;
//----------------------------------------------------------------------------
void PlayerConnectDlg::connect (const Glib::ustring& target, unsigned int port)
    throw (YGP::CommError) {
   TRACE3 ("PlayerConnectDlg::connect (const Glib::ustring&, unsigned int) - "
           << target << ':' << port);
   Glib::ustring error;
   try {
      ConnectDlg::connect (target, port);
      Check1 (cmgr.getSocket ());

      Glib::ustring data ("Version=" STRPROTOCOLL ";Variant=" STRVARIANT ";Name=\""
                          + aPlayer[0]->getName () + '"');
      cmgr.getSocket ()->write (data);

      std::string input;
      cmgr.getSocket ()->read (input);
      TRACE8 ("PlayerConnectDlg::connect (const Glib::ustring&, unsigned int) - "
              "Received: " << input);

      Glib::ustring names;
      unsigned int rc (0);
      YGP::AttributeParse ap;
      ATTRIBUTE (ap, unsigned int, posPlayer, "Self");
      ATTRIBUTE (ap, Glib::ustring, names, "Names");
      ATTRIBUTE (ap, Glib::ustring, error, "Msg");
      ATTRIBUTE (ap, unsigned int, rc, "Error");
      ap.assignValues (input);

      if (rc)
         throw error;
      if (!posPlayer)
         throw std::string (_("Position of this player is missing!"));

      // Clear the old players
      for (std::vector<Player*>::iterator i (aPlayer.begin ());
           i != aPlayer.end (); ++i)
          delete *i;
      aPlayer.clear ();

      YGP::Tokenize split (names);
      unsigned int c (0);
      while (split.getNextNode ('\n').size ()) {
         TRACE9 ("PlayerConnectDlg::connect (const Glib::ustring&, unsigned int)"
                 "- Setting " << split.getActNode ());

         Player* pPlayer ((c == posPlayer)
                          ? static_cast<Player*> (new Human (split.getActNode ()))
                          : static_cast<Player*> (new RemotePlayer (cmgr.getSocket (), split.getActNode ())));

         if (c < posPlayer)
            aPlayer.push_back (pPlayer);
         else {
            Check3 (aPlayer.size () > (c - posPlayer));
            aPlayer.insert (aPlayer.begin () + c - posPlayer, pPlayer);
         }
         c++;
      }
      TRACE9 ("PlayerConnectDlg::connect (const Glib::ustring&, unsigned int) - Players: "
              << c << "<->" << aPlayer.size ());
      if ((c != aPlayer.size ()) || (posPlayer >= aPlayer.size ()))
         throw std::string (_("Wrong number of players!"));
   }
   catch (YGP::CommError& err) {
      error = _("Error sending player name!\n\nReason: %1");
      error.replace (error.find ("%1"), 2, err.what ());
   }
   catch (std::string& err) {
      error = _("Invalid response from server!\n\nReason: %1");
      error.replace (error.find ("%1"), 2, err);
   }
   if (error.size ()) {
      Gtk::MessageDialog dlg (error, false, Gtk::MESSAGE_ERROR, Gtk::BUTTONS_OK);
      dlg.set_title (PACKAGE);
      dlg.run ();
   }
}

//----------------------------------------------------------------------------
/// Updates the name of the player with the data send from the client
/// \param socket: Socket over which the clients communicates
//----------------------------------------------------------------------------
YGP::Socket* PlayerConnectDlg::addClient (int socket) {
   TRACE3 ("PlayerConnectDlg::addClient (int)");
   YGP::Socket* sock (ConnectDlg::addClient (socket));
   Check3 (sock);

   Glib::ustring error;
   try {
      std::string input;
      sock->read (input);
      TRACE8 ("PlayerConnectDlg::addClient (int) - Received: " << input);

      Glib::ustring name;
      unsigned int protocoll (0), variant (0);
      YGP::AttributeParse ap;
      ATTRIBUTE (ap, Glib::ustring, name, "Name");
      ATTRIBUTE (ap, unsigned int, protocoll, "Version");
      ATTRIBUTE (ap, unsigned int, variant, "Variant");
      ap.assignValues (input);

      if (protocoll < PROTOCOLL) {
         error = _("Protocoll version %1 needed!");
         error.replace (error.find ("%1"), 2, STRPROTOCOLL);
         throw error;
      }
      else if ((protocoll == PROTOCOLL) && (variant < PROT_VARIANT)) {
         error = _("Protocoll variant not sufficient - Version %1 needed!\n\n"
                   "Generally this means, that the partner does not support\n"
                   "all game types - Continue at your own risk!");
         error.replace (error.find ("%1"), 2, STRVARIANT);
      }

      TRACE8 ("PlayerConnectDlg::addClient (int) - Connected: " << name);
      Check3 (aPlayer.size () > cmgr.getClients ().size ());
      delete aPlayer[cmgr.getClients ().size ()];
      aPlayer[cmgr.getClients ().size ()] = new RemotePlayer (sock, name);
      connected->set_text (connected->get_text () + name + '\n');

      Check3 (aPlayer.size () < 10);
      input = "Self=";
      input += ('0' + cmgr.getClients ().size ());
      input += ";Names=";
      for (std::vector<Player*>::iterator i (aPlayer.begin ());
           i != aPlayer.end (); ++i)
         input += (*i)->getName () + std::string (1, '\n');

      TRACE8 ("PlayerConnectDlg::addClient (int) - Sending players: " << input);
      sock->write (input);
   }
   catch (YGP::CommError& err) {
      error = _("Error getting player name!\n\nReason: %1");
      error.replace (error.find ("%1"), 2, err.what ());
   }
   catch (std::string& err) {
      sock->write ("Error=99;Msg=\"");
      sock->write (err);
      sock->write ("\"");
      error = _("Error analyzing input from client!\n\nReason: %1");
      error.replace (error.find ("%1"), 2, err);
   }
   if (error.size ()) {
      Gtk::MessageDialog dlg (error, false, Gtk::MESSAGE_ERROR, Gtk::BUTTONS_OK);
      dlg.set_title (PACKAGE);
      dlg.run ();
   }
   return sock;
}
