//$Id$

//PROJECT     : Cardgames
//SUBSYSTEM   : Common/Game
//REFERENCES  :
//TODO        :
//BUGS        :
//REVISION    : $Revision$
//AUTHOR      : Markus Schwab
//CREATED     : 10.9.2002
//COPYRIGHT   : Copyright (C) 2002 - 2007

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

#include <YGP/Check.h>
#include <YGP/Trace.h>
#include <YGP/Socket.h>
#include <YGP/ConnMgr.h>
#include <YGP/AttrParse.h>

#include "Player.h"
#include "CardSet.h"
#include "CardImgs.h"
#include "CardPile.h"
#include "ComputerPlayer.h"

#include "Game.h"


unsigned int Game::ANIMATE_STEPS (10);


//-----------------------------------------------------------------------------
/// Constructor
/// \param parent: Parent of widget
/// \param statusbar: For messages
/// \param cardset: Cardset
/// \param player: Vector of player
/// \param posPlayer: Position of the player for the server
/// \param rows: Number of rows needed by game
/// \param columns: Number of columns needed by game
//-----------------------------------------------------------------------------
Game::Game (Gtk::Box& parent, Gtk::Statusbar& statusbar, CardSet& cardset,
            const std::vector<Player*>& player, unsigned int posPlayer,
            YGP::Mutex& mxSerialize, unsigned int rows, unsigned int columns)
   : Gtk::Table (rows, columns), status (statusbar) , cards (cardset)
     , actPlayers (player), mxSerializeMsgs (mxSerialize), posServer (posPlayer)
     , pos2Play (-1U) , pos1Play (-1U), ignoreNextMsg (false), data (NULL)
     , statGame (NONE) , actPlayer (0), pWonPile (NULL), pMenuPopSort (NULL) {
   TRACE3 ("Game::Game (Gtk::Box&, Gtk::Statusbar&, Cardset&, std::vector<Player*>,"
           "unsinged int, unsigned int)");
   Check3 (cardset.size ());

   show ();
   set_col_spacings (2);
   set_row_spacings (2);

   parent.pack_start (*this, true, true, 5);

   stati.pendingTurn = stati.restart = 0;
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
   TRACE8 ("Game::start () - Act. status: " << statGame);
   Check3 ((statGame <= INITIALIZING) || (statGame == STOPPED));
   clean ();

   setGameStatus (PLAYING);

   if (getConnectionMgr ().getMode () == YGP::ConnectionMgr::SERVER) {
      std::string msg ("Game=");
      msg += name ();
      TRACE8 ("Game::start () - Sending: " << msg);
      broadcastMessage (msg);
   }
}

//-----------------------------------------------------------------------------
/// Terminates the game and cleans the table
//-----------------------------------------------------------------------------
void Game::stop () {
   TRACE8 ("Game::stop ()");
   if (getConnectionMgr ().getMode () == YGP::ConnectionMgr::SERVER)
      broadcastMessage ("End");

   clean ();
   setGameStatus (STOPPED);
}

//-----------------------------------------------------------------------------
/// End the current game as soon as possible
/// \param startNew: Flag, if game should be restarted
//-----------------------------------------------------------------------------
void Game::end (bool startNew) {
   TRACE8 ("Game::end () - Restart: " << (startNew ? "Yes" : "No"));
   stati.restart = startNew;
   if (canBeStopped ()) {
      stop ();
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
   YGP::ConnectionMgr& cmgr (getConnectionMgr ());

   if (cmgr.getMode () == YGP::ConnectionMgr::CLIENT) {
      Check3 (data && *data);
      std::string input (data);

      YGP::AttributeParse ap;
      ATTRIBUTE (ap, std::string, input, "Cards");
      try {
         ap.assignValues (input);

         YGP::Tokenize positions (input);
         TRACE8 ("Game::randomizeCardsToPile (ICardPile&) - Cards: " << cards.size ());
         for (unsigned int i (0); i < (cards.size () - 1); ++i) {
            unsigned long pos (0);
            std::string token;
            char* pTail (NULL);
            errno = 0;

            // Read next token; the value must be a number
            if ((token = positions.getNextNode (' ')).empty ()
                || stringToNumber (pos, token.c_str ())
                || (pos >= cards.size ())
                || (errno || (pTail && *pTail))) {
               std::string error (N_("Invalid card specification!"));
               throw YGP::CommError (error);
            }

            TRACE9 ("Game::randomizeCardsToPile (ICardPile&) const - [" << i
                    << "] = " << pos);
            cards.set (i, pos);
         }
         writeOK (*cmgr.getSocket ());
      }
      catch (YGP::CommError& error) {
         writeError (*cmgr.getSocket (), 99, error.what ());
         Glib::ustring err (_("Received invalid input from the server!\n\nReason: %1"));
         err.replace (err.find ("%1"), 2, _(error.what ()));
         Gtk::MessageDialog dlg (err, false, Gtk::MESSAGE_ERROR, Gtk::BUTTONS_OK);
         dlg.set_title (PACKAGE);
         dlg.run ();
         return false;
      }
   }
   else {
      cards.shuffle ();
      std::ostringstream msg;
      msg << "Cards=";
      for (unsigned int i (0); i < cards.size (); ++i)
         msg << cards.getCard (i).id () << ' ';

      const_cast<Game*> (this)->cardOrder = msg.str ();

      if (cmgr.getMode () == YGP::ConnectionMgr::SERVER)
         broadcastMessage (cardOrder);
   }

   pile.setTopCards (cards.getCards ());
   return true;
}

//-----------------------------------------------------------------------------
/// Moves cards from one pile to another
/// \param dest: Destination pile
/// \param source: Source pile
/// \param start: First card to move
/// \param end: Last card to move; -1: Move til end
//-----------------------------------------------------------------------------
void Game::movePile (ICardPile& dest, ICardPile& source, unsigned int start, int end) {
   TRACE3 ("Game::movePile (ICardPile&, ICardPile&, unsigned int, int) - "
           "moving from pos " << start << " to " << end);
   Check3 (source.size ());
   Check3 (start < source.size ());

   if (end == -1)
      end = source.size () - 1;
   Check1 (end < static_cast<int> (source.size ()));
   Check1 (static_cast<int> (start) <= end);

   do {
      dest.append (source.remove (start));
   } while ((unsigned int)end-- > start);
}

//-----------------------------------------------------------------------------
/// Cleans the table
//-----------------------------------------------------------------------------
void Game::clean () {
   TRACE8 ("Game::clean ()");
   disableWonCards ();
}

//-----------------------------------------------------------------------------
/// Activates the next player
//-----------------------------------------------------------------------------
void Game::makeNextMoves () {
   if (actPlayer >= 0) {
      TRACE8 ("Game::makeNextMoves () - " << actPlayer);
      Check3 (actPlayer < static_cast<int> (actPlayers.size ()));
      Check3 (!stati.pendingTurn);
      unsigned int timeout (actPlayers[actPlayer]->timeout ());
      if (timeout) {
         Glib::signal_timeout ().connect
             (bind (mem_fun (*actPlayers[actPlayer], &Player::makeTurn), this), timeout);
         stati.pendingTurn = 1;
      }
      else
          Glib::signal_idle ().connect
              (bind (mem_fun (*actPlayers[actPlayer], &Player::makeTurn), this));
      disableHuman ();
   }
}


//-----------------------------------------------------------------------------
/// Ends the move of the passed remote player. This contains of executing the
/// move and re-enable receiving of messages
/// \param player: ID of remote player
/// \returns bool: False
//-----------------------------------------------------------------------------
bool Game::endRemoteMove (unsigned int player) {
   TRACE8 ("Game::endRemoteMove () - " << player);
   makeMove (player);
   mxSerializeMsgs.unlock ();
   stati.pendingTurn = 0;
   makeNextMoves ();
   return false;
}

//-----------------------------------------------------------------------------
/// Enables the cards of the human player
//-----------------------------------------------------------------------------
bool Game::enableHuman () {
   Check3 (!actPlayer);
   TRACE8 ("Game::enableHuman () - enabling " << actPlayers[0]->getName ());
   return false;
}

//-----------------------------------------------------------------------------
/// Makes the move for the next computer player.
/// \returns \c int: Flag for timer, if it should continue (0: no; else: yes)
//-----------------------------------------------------------------------------
bool Game::makeComputerMove () {
   TRACE5 ("Game::makeComputerMove () - Turn of player " << actPlayer);
   stati.pendingTurn = 0;
   if (statGame == TOSTOP)
      stop ();
   else
      makeMove (actPlayer);
   return false;
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
/// \param cards: String containing the (comma-separated) IDs of the cards to flip
/// \throw YGP::ParseError: If invalid numbers for cards are found
//-----------------------------------------------------------------------------
void Game::flipCards2Play (ICardPile& pile, const std::string& cards) throw (YGP::ParseError) {
   TRACE2 ("Game::flipCards2Play (ICardPile&, const std::string&) - Cards " << cards);
   Check1 (cards.size ());

   YGP::Tokenize tokCards (cards);
   unsigned long card (0);
   unsigned int cCards (0);
   bool bFollow (false);
   while (tokCards.getNextNode (' ').size ()) {
      if (stringToNumber (card, tokCards.getActNode ().c_str ())) {
         std::string error (N_("Invalid card specification!"));
         throw YGP::ParseError (error);
      }

      card = pile.find (static_cast <unsigned int> (card));
      if ((card != -1U) && (card < (pile.size () - cCards))) {
         Check3 (card < pile.size ());
         TRACE9 ("Game::flipCards2Play (ICardPile&, const std::string&) - Found "
                 << card << " = " << *pile[card]);
         ++cCards;

         CardWidget& cardWg (*pile[card]);
         pile.move (pile.size () - 1, card);
         cardWg.showFace ();

         if ((pile.getStyle () != ICardPile::NORMAL) && bFollow) {
            Check3 (pile.size () > 1);
            pile.resize (pile.size () - 2, ICardPile::COMPRESSED);
         }
         bFollow = true;
      }
      else {
         TRACE1 ("Game::flipCards2Play (ICardPile&, const std::string&) - "
                 "Card " << tokCards.getActNode () << " not found in "
                 << pile.size () << " cards");
         std::string error ("Card not found!");
         throw YGP::ParseError (error);
      }
   } // end-while string has data
   pos2Play = pile.size () - 1;
   pos1Play = pos2Play - cCards + 1;
   Check3 (pos1Play <= pos2Play);
   TRACE8 ("Game::flipCards2Play (ICardPile&, const std::string&) - "
           "New positions " << pos1Play << " and " << pos2Play);
}

//----------------------------------------------------------------------------
/// Returns the actual target, where flipCard2Play should position the cards to
/// \returns unsigned int: ID of the target
//----------------------------------------------------------------------------
unsigned int Game::getActTarget () const {
   return 0;
}

//-----------------------------------------------------------------------------
/// Flips the cards the user is about to play
/// \param pile: Pile to manipulate
/// \param start: Position of first card to play; update to reflect moving
/// \param start: Position of last card to play; update to reflect moving
//-----------------------------------------------------------------------------
void Game::flipCards2Play (ICardPile& pile, unsigned int& start, unsigned int& end) {
   TRACE2 ("Game::flipCards2Play (ICardPile&, unsigned int, unsigned int) - "
           "Cards " << start << " - " << end);
   Check3 (end < pile.size ());
   Check3 (start <= end);

   unsigned int e (end);
   bool bFollow (false);

   // Inform clients about cards to play
   if (getConnectionMgr ().getMode () == YGP::ConnectionMgr::SERVER) {
      std::ostringstream msg;
      msg << "Play=";
      for (unsigned int i (start); i < end; ++i)
         msg << pile[i]->id () << ' ';
      msg << pile[end]->id () << ";Target=" << getActTarget ();

      broadcastMessage (msg.str ());
   }

   do {
      CardWidget& card (*pile[start]);
      pile.move (pile.size () - 1, start);
      card.showFace ();

      if ((pile.getStyle () != ICardPile::NORMAL) && bFollow) {
         Check3 (pile.size () > 1);
         pile.resize (pile.size () - 2, pile.getStyle ());
      }
      bFollow = true;
   } while (e-- && (start <= e));

   start = pile.size () - 1 - (end - start);
   end =  pile.size () - 1;
   TRACE8 ("Game::flipCards2Play (ICardPile&, unsigned int, unsigned int) - "
           "New positions " << start << " and " << end);
}

//-----------------------------------------------------------------------------
/// Shows or hides the won cards
/// \param show: Flag if to show or to hide the cards
/// \param style: Style how pile should be displayed; must be a value understood
///     by ICardPile::setStyle
//-----------------------------------------------------------------------------
void Game::showWonCards (bool show, unsigned int style) {
   if (pWonPile) {
      if (style == -1U)
	 style = show ? ICardPile::COMPRESSED : ICardPile::VERY_COMPRESSED;
      Check3 (style < ICardPile::LAST);

      pWonPile->setShowOption (show ? ICardPile::SHOWFACE : ICardPile::SHOWBACK);
      pWonPile->setStyle ((ICardPile::PileStyle)style);
      Glib::signal_idle ().connect (mem_fun (*this, &Game::enableActWonCards));
      disableWonCards ();
   }
}

//-----------------------------------------------------------------------------
/// Callback for any event for the top of the won cards
/// \param event: Caused event
//-----------------------------------------------------------------------------
bool Game::wonCardsSelected (GdkEvent* event) {
   TRACE9 ("Game::wonCardsSelected (GdkEvent*) - " << event->type);

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
                                              (_("Sort by _number"),
                                               mem_fun (*this, &Game::sortWonByNumber)));
            pMenuPopSort->items ().push_back (Gtk::Menu_Helpers::MenuElem
                                              (_("Sort by _colour"),
                                               mem_fun (*this, &Game::sortWonByColour)));
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
   if (pWonPile) {
      pWonPile->sortByNumber ();
      showWonCards ();
      Glib::signal_idle ().connect (mem_fun (*this, &Game::enableActWonCards));
      disableWonCards ();
   }
}

//-----------------------------------------------------------------------------
/// Shows and sorts the won cards by colour
//-----------------------------------------------------------------------------
void Game::sortWonByColour () {
   TRACE8 ("Game::sortWonByColour ()");
   if (pWonPile) {
      pWonPile->sortByColour ();
      showWonCards ();
      Glib::signal_idle ().connect (mem_fun (*this, &Game::enableActWonCards));
      disableWonCards ();
   }
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
          (mem_fun (*this, (&Game::wonCardsSelected))));
   return false;
}

//-----------------------------------------------------------------------------
/// Disables the won cards
//-----------------------------------------------------------------------------
void Game::disableWonCards () {
   TRACE8 ("Game::disableWonCards () - Disabling " << wonCards.size () << " cards");
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

//----------------------------------------------------------------------------
/// Callback to inform a controller about status changes
/// \param status: New status of the game
//----------------------------------------------------------------------------
void Game::control (unsigned int status) const {
}

//----------------------------------------------------------------------------
/// Writes a message to all partners
/// \param msg: Message to write
//----------------------------------------------------------------------------
void Game::broadcastMessage (const std::string& msg) const {
   TRACE3 ("Game::broadcastMessage (const std::string&) - " << msg);

   const YGP::ConnectionMgr& cmgr (getConnectionMgr ());
   if (getConnectionMgr ().getMode () == YGP::ConnectionMgr::SERVER)
      for (std::vector<YGP::Socket*>::const_iterator i (cmgr.getClients ().begin ());
           i != cmgr.getClients ().end (); ++i)
         writeMessage (**i, msg);
   else
       writeMessage (*cmgr.getSocket (), msg);
}

//----------------------------------------------------------------------------
/// Writes a set-startplayer message to all clients
/// \param startplayer: Startplayer
//----------------------------------------------------------------------------
void Game::broadcastStartPlayer (unsigned int startplayer) {
   // Send startplayer to the clients
   const YGP::ConnectionMgr& cmgr (getConnectionMgr ());
   if (cmgr.getMode () == YGP::ConnectionMgr::SERVER) {
      TRACE3 ("Game::broadcastStartPlayer (unsigned int) - " << startplayer);

      const std::vector<YGP::Socket*>& clients (cmgr.getClients ());
      unsigned int player ((startplayer - 1) & 0x3);
      for (std::vector<YGP::Socket*>::const_iterator i (clients.begin ());
	   i != clients.end (); ++i) {
	 std::ostringstream msg;
	 msg << "ActPlayer=" << player;
	 writeMessage (**i, msg.str ());
	 player = (player - 1) & 0x3;
      }
   }
}

//----------------------------------------------------------------------------
/// Writes a message to the partner
/// \param socket: Socket to write message to
/// \param msg: Message to write
//----------------------------------------------------------------------------
void Game::writeMessage (YGP::Socket& socket, const std::string& msg) {
   try {
      socket.write (msg);
      socket.write ("\0", 1);
   }
   catch (YGP::CommError& error) {
      std::string err (_("Can't write message!\n\nReason: %1"));
      err.replace (err.find ("%1"), 2, error.what ());
      Gtk::MessageDialog dlg (msg, false, Gtk::MESSAGE_ERROR, Gtk::BUTTONS_OK);
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
void Game::writeError (YGP::Socket& socket, unsigned int rc, const std::string& msg) {
   std::ostringstream error;
   error << "Error=" << rc << ";Msg=\"" + msg << "\"\0";
   writeMessage (socket, error.str ());
}

//----------------------------------------------------------------------------
/// Handles a message send from the server
/// \param player: ID of player sending the message
/// \param msg: Message to handle
/// \returns bool: True, if the message has been processed completely; else
///    (if message is still pending) false
/// \throw YGP::ParseError, YGP::CommError: In case of an error an describing string
//----------------------------------------------------------------------------
bool Game::handleMessage (unsigned int player, const std::string& msg) throw (YGP::ParseError, YGP::CommError) {
   TRACE1 ("Game::handleMessage (unsigned int player, const std::string&) - " << msg
           << " (" << player << ')');
   Check1 (msg.size ());
   Check2 (!data);

   bool rc (true);
   Check2 (!ignoreNextMsg);
   switch (statGame) {
   case NONE:
      statGame = INITIALIZING;
      data = msg.c_str ();
      start ();
      data = NULL;
      break;

   case INITIALIZING:
      break;

   default:                          // Playing (and game specific stati)
      rc = performCommand (player, msg);
   }
   return rc;
}

//----------------------------------------------------------------------------
/// Sets the player performing the next turn
/// \param player: Number identifying player (starting with 0)
//----------------------------------------------------------------------------
void Game::setNextPlayer (unsigned int player) {
   TRACE8 ("Game::setNextPlayer (unsigned int) - " << player);
   actPlayer = player;
}

//----------------------------------------------------------------------------
/// Handles a command the server sent in playing mode
/// \param player: ID of player sending the message
/// \param msg: Command to perform
/// \returns bool: Flag, if command has been performed completely
/// \throws YGP::ParseError, YGP::CommError: Describing the error
//----------------------------------------------------------------------------
bool Game::performCommand (unsigned int player, const std::string& msg) throw (YGP::ParseError, YGP::CommError) {
   TRACE8 ("Game::performCommand (unsigned int player, const std::string&) - "
           << msg << " (" << player << ')');
   Check1 (msg.size ());

   YGP::Tokenize command (msg);
   std::string cmd (command.getNextNode ('='));
   TRACE2 ("Game::performCommand (unsigned int player, const std::string&) - " << cmd);

   if (cmd == "Play") {
      cmd = command.getNextNode (';');
      std::string playTo (command.getNextNode ('='));
      std::string strTarget (command.getNextNode (';'));

      unsigned long target (-1UL);
      if (stringToNumber (target, strTarget.c_str ())
          || (playTo != "Target"))
         throw YGP::ParseError (N_("Invalid target!"));

      Check3 (actPlayer >= 0);
      ICardPile* pile (getPileOfPlayer (actPlayer, target));
      if (!pile)
         throw YGP::ParseError (N_("Invalid target!"));
      flipCards2Play (*pile, cmd);

      // Inform clients about cards to play
      if (getConnectionMgr ().getMode () == YGP::ConnectionMgr::SERVER)
          broadcastMessage (msg);

      if (executeRemoteMove (*pile, target)) {
         Glib::signal_timeout ().connect
             (bind (mem_fun (*this, &Game::endRemoteMove), actPlayer),
              ComputerPlayer::TIMEOUT);
         stati.pendingTurn = 1;
         return false;
      }
      else
         makeNextMoves ();
   }
   else if (cmd == "ActPlayer") {
      cmd = command.getNextNode (';');
      TRACE8 ("Game::performCommand (unsigned int player, const std::string&) - "
              "Next player: " << cmd);
      unsigned long player;
      if (stringToNumber (player, cmd.c_str ()))
         throw YGP::ParseError (N_("Invalid number"));

      actPlayer = player;
      if (statGame == PLAYING)
         displayTurn (player);
   }
   else if (cmd == "End") {
      end (false);
      if (getConnectionMgr ().getMode () == YGP::ConnectionMgr::SERVER)
         broadcastMessage ("End");
   }
   else
      throw YGP::ParseError (N_("Unknown command!"));
   return true;
}

//----------------------------------------------------------------------------
/// Converts a string into a number
/// \param number: Target of conversion
/// \param text: String to convert
/// \returns bool: False, if conversion succeeded (\c text contained a number)
//----------------------------------------------------------------------------
bool Game::stringToNumber (unsigned long& number, const char* text) {
   Check1 (text);
   char* pTail = NULL;
   errno = 0;
   number = strtoul (text, &pTail, 0);
   return (errno || (pTail && *pTail));
}

//----------------------------------------------------------------------------
/// Executes the remote move locally
/// \param pile: Pile to move to/from
/// \param card: ID of target, where to play the card
/// \returns bool: True, if the timer to execute the move should be set
/// \throw YGP::ParseError: In case of an invalid value
//----------------------------------------------------------------------------
bool Game::executeRemoteMove (ICardPile& pile, unsigned int card) throw (YGP::ParseError) {
   TRACE8 ("Game::executeRemoteMove (ICardPile&, unsigned int) - " << pos2Play);
   return true;
}

//----------------------------------------------------------------------------
/// Returns if the game can be stopped imediately. This is true, if there is no
/// timer activated.
/// \returns bool: True, if the game can be stopped imediately
//----------------------------------------------------------------------------
bool Game::canBeStopped () const {
   return !(actPlayer && stati.pendingTurn);
}

//----------------------------------------------------------------------------
/// Checks if the game should ignore a message. If so, the count of messages
/// to ignore is reduced by 1.
/// \returns bool: True, if a message should be ignored
//----------------------------------------------------------------------------
bool Game::ignoreMessage () {
   if (ignoreNextMsg) {
      TRACE9 ("Game::ignoreMessage () - Ignoring " << ignoreNextMsg);
      --ignoreNextMsg;
      return true;
   }
   return false;
}

//-----------------------------------------------------------------------------
/// Adds game-specific menus
/// \param mgrUI: UIManager to add to
//-----------------------------------------------------------------------------
void Game::addMenus (Glib::RefPtr<Gtk::UIManager> mgrUI) {
}

//-----------------------------------------------------------------------------
/// Removes the game-specific menus
/// \param mgrUI: UIManager to add to
//-----------------------------------------------------------------------------
void Game::removeMenus (Glib::RefPtr<Gtk::UIManager> mgrUI) {
}

//-----------------------------------------------------------------------------
/// Actions to take when the cards are resized
/// \pre The cardsize must be set in CardImages::WIDTH/HEIGHT
//-----------------------------------------------------------------------------
void Game::resizeCards () {
}

//-----------------------------------------------------------------------------
/// Animates the given card to the position in the passed pile
/// \param dest: Destination pile
/// \param source: Source pile
/// \param pos: Card of source to move
/// \pre: The card must be shown somewhere (to get its position)
//-----------------------------------------------------------------------------
void Game::animateCard (ICardPile& dest, ICardPile& src, unsigned int pos) {
   TRACE3 ("Game::animate (ICardPile&, ICardPile&, unsigned int) - " << ANIMATE_STEPS);
   Check1 (pos <= src.size ());

   if (ANIMATE_STEPS) {
      // Get the position of the source card
      CardWidget& card (*src.at (pos)); Check2 (card.get_window ());
      int x, y;
      card.get_window ()->get_origin (x, y);
      TRACE9 ("Game::animate (ICardPile&, ICardPile&, unsigned int) - Origin: " << x << '/' << y);
      src.remove (pos);

      // Create the animated window
      Gtk::Window* win (new Gtk::Window (Gtk::WINDOW_POPUP));
      win->add (*manage (new CardWidget (card)));
      win->move (x, y);
      win->show_all_children ();
      win->show ();

      // Start the animation
      if (dest.size ())
	 startAnimation (win, &dest, &card);
      else {
	 dest.setTopCard (card);
	 card.set_size_request (1, 1);
	 Glib::signal_idle ().connect
	    (bind (mem_fun (*this, &Game::startAnimation), win, &dest, &card));
      }
   }
   else {
      movePile (dest, src, pos, pos);

      sigAnimation.emit ();
      cbAnimation.disconnect ();
   }
}

//-----------------------------------------------------------------------------
/// Calculates the movements for the animation
/// \param win: Window to animate (move)
/// \param dest: Pile where to animate the window to
/// \param card: Card to insert at target
/// \returns bool: Always false
//-----------------------------------------------------------------------------
bool Game::startAnimation (Gtk::Window* win, ICardPile* dest, CardWidget* card) {
   TRACE6 ("Game::startAnimation (Gtk::Window*, ICardPile*, CardWidget&)");
   Check1 (win); Check1 (dest); Check1 (card);
   Check2 (dest->size ());

   CardWidget& widget (dest->getTopCard ()); Check2 (widget.get_window ());
   int x, y, x2, y2;
   win->get_position (x, y);
   widget.get_window ()->get_origin (x2, y2);
   TRACE9 ("Game::startAnimation (Gtk::Window*, ICardPile*, CardWidget&) - Dest: " << x2 << '/' << y2);

   x2 -= x;
   y2 -= y;
   x2 /= (int)ANIMATE_STEPS;
   y2 /= (int)ANIMATE_STEPS;

   Glib::signal_idle ().connect
      (bind (mem_fun (*this, &Game::doAnimation), win, x2, y2, dest, card, ANIMATE_STEPS));
   return false;
}

//-----------------------------------------------------------------------------
/// Moves the animation-window to the next position
/// \param win: Window to animate (move)
/// \param x: Vertical distance to move the animation
/// \param y: Horizontal distance to move the animation
/// \param dest: Pile where to animate the window to
/// \param card: Card to insert at target
/// \param steps: Remaining steps
/// \returns false; To stop animation
//-----------------------------------------------------------------------------
bool Game::doAnimation (Gtk::Window* win, int x, int y, ICardPile* dest,
			CardWidget* card, unsigned int steps) {
   TRACE9 ("Game::doAnimation (...) - Steps: " << steps);
   Check1 (win); Check1 (dest); Check1 (card);

   if (steps) {
      int actX, actY;
      win->get_position (actX, actY);
      win->move (actX + x, actY + y);
      Glib::signal_idle ().connect
	 (bind (mem_fun (*this, &Game::doAnimation), win, x, y, dest, card, steps - 1));
   }
   else {
      if (&dest->getTopCard () == card)
	 card->set_size_request (card->getImageWidth (), card->getImageHeight ());
      else
	 dest->setTopCard (*card);
      delete win;

      // Start the callback
      sigAnimation.emit ();
      cbAnimation.disconnect ();
   }
   return false;
}
