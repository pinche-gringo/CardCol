//$Id$

//PROJECT     : Cardgames
//SUBSYSTEM   : Network-code
//REFERENCES  :
//TODO        :
//BUGS        :
//REVISION    : $Revision$
//AUTHOR      : Markus Schwab
//CREATED     : 7.7.2005
//COPYRIGHT   : Copyright (C) 2005

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


#include <cardgames-cfg.h>

#ifdef HAVE_LIBPTHREAD

#include <string>

#include <gtkmm/messagedialog.h>

#include <YGP/Check.h>
#include <YGP/Trace.h>
#include <YGP/Socket.h>
#include <YGP/AttrParse.h>

#include <Game.h>
#include <Player.h>
#include <PlayerConnDlg.h>

#include "ChatDlg.h"

#include "CardCol.h"
#include "CardColAppl.h"


//-----------------------------------------------------------------------------
/// Stopps the client waiting for the server to start the game.
/// \returns true; if the client should wait for the server, or program runs as
///          server
//-----------------------------------------------------------------------------
bool CardgameCollection::stopClientWaiting () {
   if (cmgr.getMode () == YGP::ConnectionMgr::CLIENT) {
      Gtk::MessageDialog dlg (_("Stop waiting for the server to start the game and start a local one?"),
			      false, Gtk::MESSAGE_QUESTION, Gtk::BUTTONS_YES_NO);
      dlg.set_title (PACKAGE);
      if (dlg.run () == Gtk::RESPONSE_YES) {
	 Check3 (aCommThreads.size () == 1);
	 cmgr.changeMode (YGP::ConnectionMgr::NONE);
	 aCommThreads[0]->cancel ();
	 delete aCommThreads[0];
	 aCommThreads.clear ();
      }
      else
	 return true;
   }
   return false;
}

//-----------------------------------------------------------------------------
/// Opens a dialog allowing to connect to other computers
//-----------------------------------------------------------------------------
void CardgameCollection::connect () {
   playerPos = PlayerConnectDlg::perform (aPlayer, CardgameAppl::PORT, cmgr);
   TRACE1 ("CardgameCollection::connect () - Mode: " << cmgr.getMode ()
	   << "; Pos: " << playerPos);
   if (cmgr.getMode () != YGP::ConnectionMgr::NONE)
      initCommunication ();
}

//-----------------------------------------------------------------------------
/// Initializes the communication
//-----------------------------------------------------------------------------
void CardgameCollection::initCommunication () {
   Check2 (cmgr.getMode () != YGP::ConnectionMgr::NONE);
   Check2 (aCommThreads.empty ());

   if (cmgr.getMode () == YGP::ConnectionMgr::CLIENT) {
      status.pop ();
      status.push (_("Waiting for the server to start the game ..."));
      aCommThreads.push_back (THRDAPPL::create2 (this, &CardgameCollection::waitForMessages,
						 (void*)-1));
      aCommThreads[0]->allowCancelation ();
   }
   else
      for (unsigned int i (0); i < cmgr.getClients ().size (); ++i) {
         aCommThreads.push_back (THRDAPPL::create2 (this, &CardgameCollection::waitForMessages,
						    (void*)i));
         aCommThreads[i]->allowCancelation ();
      }

   apMenus[CHAT]->set_sensitive ();
}

//----------------------------------------------------------------------------
/// Wait for messages
/// \param player: ID of player (-1 for server; 0 .. n for clients)
/// \returns \c void*: NULL
//----------------------------------------------------------------------------
void* CardgameCollection::waitForMessages (void* thread) {
   TRACE1 ("CardgameCollection::waitForMessage (void*)");
   Check2 (cmgr.getMode () != YGP::ConnectionMgr::NONE);

   int iPlayer ((int)((YGP::Thread*)thread)->getArgs ());
   Check2 ((cmgr.getMode () == YGP::ConnectionMgr::CLIENT)
           ? (iPlayer == -1) : (iPlayer < (int)cmgr.getClients ().size ()));

   std::string input;
   YGP::Socket* sock ((iPlayer == -1) ? cmgr.getSocket () : cmgr.getClients ()[iPlayer]);
   Check3 ((iPlayer == -1) ? playerPos : true);
   iPlayer = (iPlayer == -1) ? (aPlayer.size () - playerPos) : (iPlayer + 1);
   unsigned int cont (true);
   try {
      while (cont) {
         sock->read (input);

         TRACE7 ("CardgameCollection::waitForMessage (void*) - `" << input << '\'');
         if (input.empty ()) {
            std::string msg (_("Lost connection to %1!"));
            Check3 (static_cast<unsigned int>(iPlayer) < aPlayer.size ());
            msg.replace (msg.find ("%1"), 2, aPlayer[iPlayer]->getName ());
            cont = false;
            throw msg;
         }

         YGP::Tokenize messages (input);
         std::string message;
         while ((message = messages.getNextNode ('\0')).size ()) {
            TRACE9 ("CardgameCollection::waitForMessages (void*) - Lock (thread)");
            mxThreadCmd.lock ();    // Wait til last message has been processed
            TRACE9 ("CardgameCollection::waitForMessages (void*) - Perform cmd " << message);

            Glib::signal_idle ().connect
               (bind (mem_fun (*this, &CardgameCollection::handleMessage),
                      iPlayer, message));
            mxGuiCmd.lock ();
            mxThreadCmd.unlock ();
            mxGuiCmd.unlock ();
            TRACE9 ("CardgameCollection::waitForMessages (void*) - Handled msg");
         }
      }
   }
   catch (std::string& error) {
      std::string msg (_("Error receiving data!\n\nReason: %1"));
      msg.replace (msg.find ("%1"), 2, error);

      Glib::signal_idle ().connect
          (bind (mem_fun (*this, &CardgameCollection::showMessage), msg));
   }
   catch (std::domain_error& error) {
      std::string msg (_("Lost connection to %1!"));
      Check3 (static_cast<unsigned int> (iPlayer) < aPlayer.size ());
      msg.replace (msg.find ("%1"), 2, aPlayer[iPlayer]->getName ());
      Glib::signal_idle ().connect
          (bind (mem_fun (*this, &CardgameCollection::showMessage), msg));
   }

   aCommThreads.erase (find (aCommThreads.begin (), aCommThreads.end (), thread));
   return NULL;
}

//----------------------------------------------------------------------------
/// Handles received global messages: Those are:
///   - Error messages (to display error messages):
///      <pre>  <b>Error</b>=<tt>Number</tt>;<b>Msg</b>="<tt>message</tt>"</pre>
///   - Game messages (to restart a game):
///      <pre>  <b>Game</b>=<tt>Name</tt>;
///   - ActPlayer messages (to set the next player; handled here to determine
///       from where to read data from):
///      <pre>  <b>ActPlayer</b>=<tt>player</tt>;
/// \param player: Player sending the message
/// \param msg: Received message to handle
/// \returns int: True: Message was a supported message and has been processed;
///     -1 if Message was handled, but not fully processed yet; else false
//----------------------------------------------------------------------------
int CardgameCollection::handleGlobalMessage (unsigned int player,
                                             const std::string& msg) throw (std::string) {
   TRACE5 ("CardgameCollection::handleGlobalMessage (unsigned int, char*) - " << msg);

   YGP::Tokenize message (msg);
   std::string cmd (message.getNextNode ('='));
   std::string param (message.getNextNode (';'));
   TRACE3 ("CardgameCollection::handleGlobalMessage (unsigned int, char*) - " << cmd);

   if (cmd == "Game") {
      int type (CardgameAppl::convertToGameType (param.c_str ()));
      if (type == GameTypes::NONE) {
         std::string msg (_("Invalid game type: `%1'"));
         msg.replace (msg.find ("%1"), 2, param);
         throw msg;
      }

      actGame = type;
      if (game) {
         restart = true;
         if (restartGame ())
            mxThreadCmd.unlock ();
      }
      else {
          startGame ();
          mxThreadCmd.unlock ();
      }
      cmgr.getSocket ()->write ("Error=0\0");
      return -1U;
   }
   else if (cmd == "Msg") {
      cmd.clear ();
      param.clear ();

      YGP::AttributeParse ap;
      ATTRIBUTE (ap, std::string, cmd, "Msg");
      ATTRIBUTE (ap, std::string, param, "Sender");

      try {
	 ap.assignValues (msg);
      }
      catch (std::string& e) {
	 cmd = _("Invalid message received!");
      }
      if (cmd.size () && param.size ()) {
	 showChatDlg ();
	 dlgChat->addMessage (param, cmd);

	 if (cmgr.getMode () == YGP::ConnectionMgr::SERVER)
	    broadcastMsg (msg, player);

	 return true;
      }
   }
   else if (cmd == "Error") {
      if (param != "0") {
         cmd.clear ();
         YGP::AttributeParse ap;
         ATTRIBUTE (ap, std::string, cmd, "Msg");

	 try {
	    ap.assignValues (message.getNextNode ('\0').c_str ());
	    if (cmd.empty ())
	       cmd = static_cast<std::string> (_("Unspecified error"));
	 }
	 catch (std::string& e) {
	    cmd = _("Invalid message received!");
	 }

         Glib::ustring err (_("%1 send error %2\n\n%3"));
         err.replace (err.find ("%1"), 2,
                      (cmgr.getMode () == YGP::ConnectionMgr::CLIENT
                       ? _("The server")
                       : aPlayer[player]->getName ()));
         err.replace (err.find ("%2"), 2, param);
         err.replace (err.find ("%3"), 2, _(cmd.c_str ()));
	 showMessage (err);
      }
      return true;
   }
   return false;
}

//----------------------------------------------------------------------------
/// Handles received messages
/// \param player: Player sending the message (relative to server)
/// \param msg: Received message to handle
/// \returns bool: False
//----------------------------------------------------------------------------
bool CardgameCollection::handleMessage (unsigned int player, const std::string msg) {
   TRACE5 ("CardgameCollection::handleMessage (unsigned int, char*) - " << msg);

   mxGuiCmd.unlock ();
   mxThreadCmd.lock ();                             // Block message processing
   mxGuiCmd.lock ();

   bool unlock (true);
   try {
      int rc (handleGlobalMessage (player, msg));
      if ((rc == -1)
          || (!rc && (game && !game->ignoreMessage ()
                      && !game->handleMessage (player, msg))))
         unlock = false;
   }
   catch (std::string& error) {
      TRACE9 ("CardgameCollection::handleMessage (unsigned int, const std::string)"
              " - Error " << error);
      std::string msg ("Error=99;Msg=\"");
      msg += error;
      msg += "\"\0";
      broadcastMsg (msg);

      Glib::ustring message (_("Error processing command `%1'!\n\n%2"));
      message.replace (message.find ("%1"), 2, msg);
      message.replace (message.find ("%2"), 2, _(error.c_str ()));
      showMessage (message);
   }

   TRACE9 ("CardgameCollection::handleMessages (unsigned int, char*) - Unlock (main): " << int(unlock));
   if (unlock)
      mxThreadCmd.unlock ();

   return false;
}

//-----------------------------------------------------------------------------
/// Opens a dialog to chat with the connected persons
//-----------------------------------------------------------------------------
void CardgameCollection::showChatDlg () {
   if (dlgChat)
      ; // TODO: Activate existing dialog
   else {
      dlgChat = ChatDlg::create (get_window ());
      dlgChat->signalSend.connect (mem_fun (*this, &CardgameCollection::sendMessage));
   }
}

//-----------------------------------------------------------------------------
/// Sends the passes message to the partners
/// \param msg: Message to send
//-----------------------------------------------------------------------------
void CardgameCollection::sendMessage (const Glib::ustring& msg) {
   TRACE9 ("CardgameCollection::sendMessage (const Glib::ustring&) - " << msg);
   Check2 (dlgChat);

   if (cmgr.getMode () == YGP::ConnectionMgr::SERVER)
      dlgChat->addMessage (aPlayer[0]->getName (), msg);

   std::string sendString ("Msg=\"");
   sendString += Glib::locale_from_utf8 (msg);
   sendString += "\";Sender=\"";
   sendString += aPlayer[0]->getName ();
   sendString+= "\"\0";

   broadcastMsg (sendString);
}

//-----------------------------------------------------------------------------
/// Broadcast a message to all partners
/// \param msg: Message to broadcast
/// \param exclude: Partner to exclude (-1: None
//-----------------------------------------------------------------------------
void CardgameCollection::broadcastMsg (const Glib::ustring& msg, unsigned int exclude) {
   TRACE9 ("CardgameCollection::broadcastMsg (const Glib::ustring&, unsigned int) - " << msg);
   try {
      if (cmgr.getMode () == YGP::ConnectionMgr::SERVER) {
	 for (std::vector<YGP::Socket*>::const_iterator i (cmgr.getClients ().begin ());
	      i != cmgr.getClients ().end (); ++i)
	    if (exclude != (unsigned int)(i - cmgr.getClients ().begin ())) {
	       Check (*i);
	       (*i)->write (msg);
	    }
      }
      else {
	 Check3 (cmgr.getSocket ());
	 cmgr.getSocket ()->write (msg);
      }
   }
   catch (std::string& e) {
   }
}

//-----------------------------------------------------------------------------
/// Starts the connections, if passed as options
/// \param options: Options of program; containing host/port
//-----------------------------------------------------------------------------
void CardgameCollection::autoConnect (const Options& options) {
   if (options.port.size ()) {
      TRACE9 ("CardgameCollection::autoConnect (const Options&) - Connect: "
              << options.target << '-' << options.port);
      if (options.target.size ())
         playerPos = PlayerConnectDlg::perform (aPlayer, cmgr, options.target,
                                                options.port);
      else
          playerPos = PlayerConnectDlg::perform (aPlayer, cmgr, options.port);

      TRACE1 ("CardgameCollection::autoConnect (const Options&) - "
              << cmgr.getMode () << "; Pos: " << playerPos);
      if (cmgr.getMode () != YGP::ConnectionMgr::NONE)
         initCommunication ();
   }
}

#endif
