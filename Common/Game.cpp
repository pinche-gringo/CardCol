//$Id$

//PROJECT     : Cardgames
//SUBSYSTEM   : Common/Game
//REFERENCES  :
//TODO        :
//BUGS        :
//REVISION    : $Revision$
//AUTHOR      : Markus Schwab
//CREATED     : 10.9.2002
//COPYRIGHT   : Anticopyright (A) 2002, 2003

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

#include <cerrno>
#include <cstdlib>

#include <sstream>

#include <glibmm/main.h>

#include <gtkmm/box.h>
#include <gtkmm/menu.h>
#include <gtkmm/statusbar.h>
#include <gtkmm/messagedialog.h>

#define CHECK 9
#define TRACELEVEL 9
#include <Check.h>
#include <Trace_.h>
#include <Socket.h>
#include <ConnMgr.h>
#include <AttrParse.h>

#include "Player.h"
#include "CardSet.h"
#include "CardPile.h"

#include "Game.h"


//-----------------------------------------------------------------------------
/// Constructor
/// \param parent: Parent of widget
/// \param statusbar: For messages
/// \param cardset: Cardset
/// \param player: Vector of player
/// \param rows: Number of rows needed by game
/// \param columns: Number of columns needed by game
//-----------------------------------------------------------------------------
Game::Game (Gtk::Box& parent, Gtk::Statusbar& statusbar, CardSet& cardset,
            const std::vector<Player*>& player, unsigned int rows,
            unsigned int columns)
   : Gtk::Table (rows, columns), statGame (NONE), status (statusbar)
     , cards (cardset), restart (false), pWonPile (NULL), pMenuPopSort (NULL)
     , actPlayers (player), data (NULL) {
   TRACE3 ("Game::Game (Gtk::Box&, Gtk::Statusbar&, Cardset&, std::vector<Player*>,"
           "unsinged int, unsigned int)");
   Check3 (cardset.size ());

   show ();
   set_col_spacings (2);
   set_row_spacings (2);

   parent.pack_start (*this, true, true, 5);
}

//-----------------------------------------------------------------------------
/// Destructor
//-----------------------------------------------------------------------------
Game::~Game () {
   TRACE9 ("Game::~Game ()");
   clean ();
   delete pMenuPopSort;
}


//-----------------------------------------------------------------------------
/// Starts the game
//-----------------------------------------------------------------------------
void Game::start () {
   TRACE9 ("Game::start ()");
   Check3 ((statGame <= INITIALIZING) || (statGame == STOPPED));
   if (statGame == STOPPED)
      clean ();

   setGameStatus (PLAYING);
   actPlayer = 0;

   Check3 (getConnectionMgr ());
   ConnectionMgr* cmgr (getConnectionMgr ());
   if (cmgr && cmgr->getClients ().size ()) {
      std::string msg ("Game=");
      msg += name ();
      TRACE9 ("Game::start () - Sending: " << msg);
      for (std::vector<Socket*>::const_iterator i (cmgr->getClients ().begin ());
           i != cmgr->getClients ().end (); ++i) {
         writeMessage (**i, msg);
      }
   }
}

//-----------------------------------------------------------------------------
/// Terminates the game and cleans the table
//-----------------------------------------------------------------------------
void Game::stop () {
   TRACE9 ("Game::stop ()");
   clean ();
   setGameStatus (STOPPED);
}

//-----------------------------------------------------------------------------
/// End the current game as soon as possible
/// \param startNew: Flag, if game should be restarted
//-----------------------------------------------------------------------------
void Game::end (bool startNew) {
   TRACE9 ("Game::end () - Restart: " << (startNew ? "Yes" : "No"));

   restart = startNew;
   if (canBeStopped ()) {
      setGameStatus (STOPPED);
      actPlayer = 0;
      disableHuman ();
   }
   else
      setGameStatus (TOSTOP);
}

//-----------------------------------------------------------------------------
/// Disables the cards the human player can select
//-----------------------------------------------------------------------------
void Game::disableHuman () {
   TRACE2 ("Game::disableHuman () - " << activeCards.size () << " cards");

   for (int i (activeCards.size ()); i > 0;)
      activeCards[--i].disconnect ();
   
   activeCards.clear ();
}

//-----------------------------------------------------------------------------
/// Shuffles (Randomizes) the cards onto the staple
/// \param pile: Pile to which the cards should be shuffeled to
/// \returns bool: Flag, if method completed successfully
//-----------------------------------------------------------------------------
bool Game::randomizeCardsToPile (ICardPile& pile) const {
   // Randomize and put cards onto staple
   ConnectionMgr* cmgr (getConnectionMgr ());
   if (cmgr && (cmgr->getMode () == ConnectionMgr::CLIENT)) {
      Check3 (data);
      std::string input (data);

      AttributeParse ap;
      ATTRIBUTE (ap, std::string, input, "Cards");
      try {
         ap.assignValues (input);

         Tokenize positions (input);
         TRACE9 ("Game::randomizeCardsToPile (ICardPile&) - Cards: " << cards.size ());
         for (unsigned int i (0); i < (cards.size () - 1); ++i) {
            unsigned long pos (0);
            std::string token;
            char* pTail (NULL);
            errno = 0;

            // Read next token; the value must be a number
            if ((token = positions.getNextNode (' ')).empty ()
                || ((pos = strtol (token.c_str (), &pTail, 10)) > cards.size ())
                || (errno || (pTail && *pTail))) {
               std::string error (_("Not a number: `%1'"));
               error.replace (error.find ("%1"), 2, positions.getActNode ());
               throw error;
            }

            TRACE9 ("Game::randomizeCardsToPile (ICardPile&) const - [" << i
                    << "] = " << pos);
            cards.set (i, pos);
         }
         writeOK (*cmgr->getSocket ());
      }
      catch (std::string& error) {
         writeError (*cmgr->getSocket (), 99, error);
         std::string err (_("Received invalid input from the server!\n\nReason: %1"));
         err.replace (err.find ("%1"), 2, error);
         Gtk::MessageDialog dlg (err, Gtk::MESSAGE_ERROR, Gtk::BUTTONS_OK);
         dlg.set_title (PACKAGE);
         dlg.run ();
         return false;
      }
   }
   else {
      cards.shuffle ();
      if (cmgr && cmgr->getClients ().size ()) {
         std::ostringstream msg;
         msg << "Cards=";
         for (unsigned int i (0); i < cards.size (); ++i)
            msg << cards.getCard (i).id () << ' ';
         TRACE9 ("Game::randomizeCardsToPile (ICardPile&) - Sending: " << msg.str ());

         for (std::vector<Socket*>::const_iterator i (cmgr->getClients ().begin ());
              i != cmgr->getClients ().end (); ++i) {
            writeMessage (**i, msg.str ());
         }
      }
   }

   pile.setTopCards (cards.getCards ());
   return true;
}

//-----------------------------------------------------------------------------
/// Moves cards from one pile to another
/// \param dest: Destination pile
/// \param source: Source pile
//-----------------------------------------------------------------------------
void Game::movePile (ICardPile& dest, ICardPile& source, unsigned int start,
                     int end) {
   TRACE3 ("Game::movePile (ICardPile&, ICardPile&, unsigned int, int) - "
           "moving from pos " << start << " to " << end);
   Check3 (source.size ());
   Check3 (start < source.size ());
   
   if (end == -1)
      end = source.size () - 1;
   Check1 (end < source.size ()); Check1 (start <= end);

   do {
      dest.append (source.remove (start));
   } while ((unsigned int)end-- > start);
}

//-----------------------------------------------------------------------------
/// Cleans the table
//-----------------------------------------------------------------------------
void Game::clean () {
   TRACE9 ("Game::clean ()");
   disableWonCards ();
}

//-----------------------------------------------------------------------------
/// Activates the next player
//-----------------------------------------------------------------------------
void Game::makeNextMoves () {
   if (actPlayer >= 0) {
      Check1 (actPlayer < actPlayers.size ());
      unsigned int timeout (actPlayers[actPlayer]->timeout ());
      if (timeout)
         Glib::signal_timeout ().connect
             (bind (slot (*actPlayers[actPlayer], &Player::makeTurn), this), timeout);
      else
          Glib::signal_idle ().connect
              (bind (slot (*actPlayers[actPlayer], &Player::makeTurn), this));
      disableHuman ();
   }
}

//-----------------------------------------------------------------------------
/// Enables the cards of the human player
//-----------------------------------------------------------------------------
bool Game::enableHuman () {
   TRACE9 ("Game::enableHuman () - enabling player " << actPlayer);
   return false;
}

//-----------------------------------------------------------------------------
/// Makes the move for the next player.
/// \returns \c int: Flag for timer, if it should continue (0: no; else: yes)
//-----------------------------------------------------------------------------
bool Game::makeComputerMove () {
   TRACE5 ("Game::makeComputerMove () - Turn of player " << actPlayer);

   if (statGame == TOSTOP) {
      TRACE8 ("Game::makeComputerMove () - End game; Restart: "
              << (restart ? "Yes" : "No"));
      setGameStatus (STOPPED);
      if (restart)
         // Restart the game, when idle (means: *after* this signalhandler
         // terminates)
         Glib::signal_idle ().connect
             (bind_return (slot (*this, &Game::start), false));
      return false;
   }

   actPlayer = makeMove (actPlayer);
   TRACE9 ("Game::makeComputerMove () - Next player: " << actPlayer);
   if (!actPlayer)
      enableHuman ();
   return actPlayer > 0;
}

//-----------------------------------------------------------------------------
/// Displays information about whose turn it is
/// \param player: Player in turn
//-----------------------------------------------------------------------------
void Game::displayTurn (unsigned int player) {
   Check1 (player < actPlayers.size ());
   status.pop ();
   Glib::ustring stat (_("Turn of %1"));
   stat.replace (stat.find ("%1"), 2, actPlayers[player]->getName ());
   status.push (stat);
}

//-----------------------------------------------------------------------------
/// Displays information about whose turn it is
/// \param player: Player in turn
//-----------------------------------------------------------------------------
void Game::displayTurn (unsigned int player, const Glib::ustring& preText) {
   status.pop ();
   Glib::ustring stat (_("Turn of %1"));
   stat.replace (stat.find ("%1"), 2, actPlayers[player]->getName ());
   status.push (preText + stat);
}

//-----------------------------------------------------------------------------
/// Changes the game-status
/// \param newStatus: Status to set
//-----------------------------------------------------------------------------
void Game::setGameStatus (unsigned int newStatus) {
   statGame = newStatus;
   control (statGame);
}

//-----------------------------------------------------------------------------
/// Flips the cards the user is about to play
/// \param pile: Pile to manipulate
/// \param start: Position of first card to play; update to reflect moving
/// \param start: Position of last card to play; update to reflect moving
/// \returns \c unsigned int: Changed position to play
//-----------------------------------------------------------------------------
void Game::flipCards2Play (ICardPile& pile, unsigned int& start, unsigned int& end) {
   TRACE2 ("Game::flipCards2Play (ICardPile&, unsigned int, unsigned int) - "
           "Cards from " << start << " to " << end);
   Check3 (end < pile.size ());
   Check3 (start <= end);

   unsigned int s (start);
   unsigned int e (end);
   bool bFollow (false);
   do {
      CardWidget& card (*pile[s]);
      pile.move (pile.size () - 1, s);
      card.showFace ();

      if ((pile.getStyle () != ICardPile::NORMAL)
          && bFollow) {
         Check3 (pile.size () > 1);
         pile.resize (pile.size () - 2, ICardPile::COMPRESSED);
      }
      bFollow = true;
   } while (e-- && (s <= e));

   start = pile.size () - 1 - (end - start);
   end =  pile.size () - 1;
   TRACE8 ("Game::flipCards2Play (ICardPile&, unsigned int, unsigned int) - "
           "New positions " << start << " and " << end);
}

//-----------------------------------------------------------------------------
/// Shows or hides the won cards
/// \param show: Flag if to show or to hide the cards
//-----------------------------------------------------------------------------
void Game::showWonCards (bool show) {
   if (pWonPile) {
      pWonPile->setShowOption (show ? ICardPile::SHOWFACE : ICardPile::SHOWBACK);
      pWonPile->setStyle (show ? ICardPile::COMPRESSED : ICardPile::VERY_COMPRESSED);
      Glib::signal_timeout ().connect
         (slot (*this, &Game::enableActWonCards), 50);
      disableWonCards ();
   }
}

//-----------------------------------------------------------------------------
/// Callback for any event for the top of the won cards
/// \param event: Caused event
//-----------------------------------------------------------------------------
bool Game::wonCardsSelected (GdkEvent* event) {
   TRACE2 ("Game::wonCardsSelected (GdkEvent*) - " << event->type);

   if (event->type == GDK_BUTTON_PRESS) {
      GdkEventButton* bev ((GdkEventButton*)(event));
      switch (bev->button) {
      case 1:
         Check3 (pWonPile);
         showWonCards (pWonPile->getShowOption () == ICardPile::SHOWBACK);
         break;

      case 3: {
         if (!pMenuPopSort) {
            TRACE9 ("Game::wonCardsSelected (GdkEvent*) - Creating menu");
            pMenuPopSort = new Gtk::Menu;
            pMenuPopSort->items ().push_back (Gtk::Menu_Helpers::MenuElem
                                              (_("Sort by number"),
                                               slot (*this, &Game::sortWonByNumber)));
            pMenuPopSort->items ().push_back (Gtk::Menu_Helpers::MenuElem
                                              (_("Sort by colour"),
                                               slot (*this, &Game::sortWonByColour)));
         }
         pMenuPopSort->popup (bev->button, bev->time);
         break; }
      }
      return true;
   }

   return false;
}

//-----------------------------------------------------------------------------
/// Shows and sorts the won cards by number
//-----------------------------------------------------------------------------
void Game::sortWonByNumber () {
   TRACE8 ("Game::sortWonByNumber ()");
   Check3 (pWonPile);
   pWonPile->sortByNumber ();
   showWonCards ();
   Glib::signal_timeout ().connect (slot (*this, &Game::enableActWonCards), 50);
   disableWonCards ();
}

//-----------------------------------------------------------------------------
/// Shows and sorts the won cards by colour
//-----------------------------------------------------------------------------
void Game::sortWonByColour () {
   TRACE8 ("Game::sortWonByColour ()");
   Check3 (pWonPile);
   pWonPile->sortByColour ();
   showWonCards ();
   Glib::signal_timeout ().connect (slot (*this, &Game::enableActWonCards), 50);
   disableWonCards ();
}

//-----------------------------------------------------------------------------
/// Enables the actual won cards
//-----------------------------------------------------------------------------
bool Game::enableActWonCards () {
   disableWonCards ();

   Check3 (pWonPile);
   TRACE9 ("Game::enableActWonCards () - Enabling " << pWonPile->size ()
           << " cards");
   for (int i (pWonPile->size ()); i;)
      wonCards.push_back
         ((*pWonPile)[--i]->signal_event ().connect
          (slot (*this, (&Game::wonCardsSelected))));
   return false;
}

//-----------------------------------------------------------------------------
/// Disables the won cards
//-----------------------------------------------------------------------------
void Game::disableWonCards () {
   TRACE9 ("Game::disableWonCards () - Disabling " << wonCards.size () << " cards");
   for (int i (wonCards.size ()); i > 0;)
      wonCards[--i].disconnect ();
   
   wonCards.clear ();
}

//-----------------------------------------------------------------------------
/// Changes the names of the playing people
/// \param newNames: Array holding the new names of the players
//-----------------------------------------------------------------------------
void Game::changeNames (const std::vector<Player*>& newPlayer) {
   const_cast<std::vector<Player*>&> (actPlayers) = newPlayer;
}

//-----------------------------------------------------------------------------
/// Reads the turn of a remote player
/// \param socket: Socket from which to read the turn
/// \returns bool: Flag, if player continues its turn
//-----------------------------------------------------------------------------
bool Game::readTurn (Socket& socket) {
   return false;
}

//-----------------------------------------------------------------------------
/// Sends the performed turn to a clients
/// \param socket: Socket from which to read the turn
/// \param start: First card of the player to play
/// \param end: Last card of the player to play
/// \returns bool: Flag, if message could be sent
//-----------------------------------------------------------------------------
bool Game::writeTurn (Socket& socket, unsigned int start, unsigned int end) {
   std::ostringstream msg;
   msg << "Start=" << start << ";End=" << end;
   writeMessage (socket, msg.str ());

}

//----------------------------------------------------------------------------
/// Callback to inform a controller about status changes
/// \param status: New status of the game
//----------------------------------------------------------------------------
void Game::control (unsigned int status) const {
}

//----------------------------------------------------------------------------
/// Checks, if the game is running in server mode
/// \returns ConnectionMgr*: Pointer to connection manager or NULL
//----------------------------------------------------------------------------
ConnectionMgr* Game::getConnectionMgr () const {
   return NULL;
}

//----------------------------------------------------------------------------
/// Writes a message to the partner
/// \param socket: Socket to write message to
/// \param msg: Message to write
//----------------------------------------------------------------------------
void Game::writeMessage (Socket& socket, const std::string& msg) {
   try {
      socket.write (msg);
   }
   catch (std::domain_error& error) {
      std::string err (_("Can't write message!\n\nReason: %1"));
      err.replace (err.find ("%1"), 2, error.what ());
      Gtk::MessageDialog dlg (msg, Gtk::MESSAGE_ERROR, Gtk::BUTTONS_OK);
      dlg.set_title (PACKAGE);
      dlg.run ();
   }
}

//----------------------------------------------------------------------------
/// Writes a status message to the partner
/// \param socket: Socket to write message to
/// \param rc: Error code to send
/// \param msg: Message to write
//----------------------------------------------------------------------------
void Game::writeError (Socket& socket, unsigned int rc, const std::string& msg) {
   std::string error ("Error=98;Msg=\"" + msg);
   error += '"';
   writeMessage (socket, error);
}

//----------------------------------------------------------------------------
/// Reads the response from the partner.
///
/// The response must be in the format:
/// <pre>  <b>Error</b>=<tt>Number</tt>;<b>Msg</b>="<tt>message</tt>"</pre>
/// \param socket: Socket to read response from
//----------------------------------------------------------------------------
void Game::checkResponse (const char* msg) {
   std::string message;
   try {
      unsigned int rc (0);
      AttributeParse ap;
      ATTRIBUTE (ap, std::string, message, "Msg");
      ATTRIBUTE (ap, unsigned int, rc, "Error");

      ap.assignValues (msg);
      if (!rc)
         msg = "";
   }
   catch (std::string& error) {
      message = _("Invalid response!\n\nReason: %1");
      message.replace (message.find ("%1"), 2, error);
      Gtk::MessageDialog dlg (message, Gtk::MESSAGE_ERROR, Gtk::BUTTONS_OK);
      dlg.set_title (PACKAGE);
      dlg.run ();
   }
}

//----------------------------------------------------------------------------
/// Handles a message send from the server
/// \param msg: Message to handle
//----------------------------------------------------------------------------
void Game::handleMessage (const char* msg) {
   TRACE1 ("Game::handleMessage (const char*) - " << msg);
   Check1 (data == NULL);

   switch (statGame) {
   case NONE:
      statGame = INITIALIZING;
      data = msg;
      start ();
      break;

   case INITIALIZING:
      checkResponse (msg);
      break;
   }

   data = NULL;
}
