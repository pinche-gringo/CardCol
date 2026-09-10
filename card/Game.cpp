//PROJECT     : Cardgames
//SUBSYSTEM   : Common/Game
//REFERENCES  :
//TODO        :
//BUGS        :
//AUTHOR      : Markus Schwab
//CREATED     : 10.9.2002
//COPYRIGHT   : Copyright (C) 2002 - 2018, 2024

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


#include <cardgames-cfg.h>

#include <cerrno>
#include <cstdlib>

#include <sstream>

#include <boost/tokenizer.hpp>

#include <glibmm/main.h>

#include <gtkmm/box.h>
#include <gtkmm/statusbar.h>
#include <gtkmm/messagedialog.h>
#include <gtkmm/popovermenu.h>

#include <giomm/menu.h>
#include <giomm/simpleactiongroup.h>

#include <gdkmm/rectangle.h>

#include <XGP/XDialog.h>

#include <YGP/Check.h>
#include <YGP/Trace.h>
#include <YGP/Socket.h>
#include <YGP/ConnMgr.h>
#include <YGP/AttrParse.h>

#include "Set.h"
#include "Pile.h"
#include "Player.h"
#include "Images.h"
#include "Window.h"
#include "ComputerPlayer.h"

#include "Game.h"

namespace Card {


//-----------------------------------------------------------------------------
/// Constructor
/// \param parent Parent of widget
/// \param statusbar For messages
/// \param cardset Cardset
/// \param player Vector of player
/// \param posPlayer Position of the player for the server
/// \param rows Number of rows needed by game
/// \param columns Number of columns needed by game
//-----------------------------------------------------------------------------
Game::Game(Gtk::Box& parent, Gtk::Statusbar& statusbar, Set& cardset,
           const std::vector<Player*>& player, unsigned int posPlayer,
           YGP::Mutex& mxSerialize, unsigned int, unsigned int)
   : Gtk::Grid(), status(statusbar) , cards(cardset),
     activeCards(), actPlayers(player), mxSerializeMsgs(mxSerialize),
     posServer(posPlayer), pos2Play(-1U) , pos1Play(-1U), ignoreNextMsg(false),
     data(NULL), statGame(NONE) , actPlayer(0), stati(), wonCards(),
     pWonPile(NULL), pMenuPopSort(NULL), cardOrder() {
   TRACE3("Game::Game(Gtk::Box&, Gtk::Statusbar&, set&, std::vector<Player*>,"
          "unsinged int, unsigned int)");
   Check3(cardset.size());

   show();
   set_column_spacing(2);
   set_row_spacing(2);

   set_hexpand(); set_vexpand();
   set_margin(5);
   insert_before(parent, statusbar);

   stati.pendingTurn = stati.restart = 0;
}

//-----------------------------------------------------------------------------
/// Destructor
//-----------------------------------------------------------------------------
Game::~Game() {
   TRACE9("Game::~Game()");
   clean();
   if (pMenuPopSort) {
      pMenuPopSort->unparent();
      delete pMenuPopSort;
   }
}


//-----------------------------------------------------------------------------
/// Starts the game
//-----------------------------------------------------------------------------
void Game::start() {
   TRACE8("Game::start() - Act. status: " << statGame);
   Check3((statGame <= INITIALIZING) ||(statGame == STOPPED));
   clean();

   setGameStatus(PLAYING);

   if (getConnectionMgr().getMode() == YGP::ConnectionMgr::SERVER) {
      std::string msg("Game=");
      msg += name();
      TRACE8("Game::start() - Sending: " << msg);
      broadcastMessage(msg);
   }
}

//-----------------------------------------------------------------------------
/// Terminates the game and cleans the table
//-----------------------------------------------------------------------------
void Game::stop() {
   TRACE8("Game::stop()");
   if (getConnectionMgr().getMode() == YGP::ConnectionMgr::SERVER)
      broadcastMessage("End");

   clean();
   setGameStatus(STOPPED);
}

//-----------------------------------------------------------------------------
/// End the current game as soon as possible
/// \param startNew Flag, if game should be restarted
//-----------------------------------------------------------------------------
void Game::end(bool startNew) {
   TRACE8("Game::end() - Restart: " << (startNew ? "Yes" : "No"));
   stati.restart = startNew;
   if(canBeStopped()) {
      stop();
      actPlayer = 0;
      disableHuman();
   }
   else
      setGameStatus(TOSTOP);
}

//-----------------------------------------------------------------------------
/// Disables the cards the human player can select
/// This method must not assume that cards are activated
//-----------------------------------------------------------------------------
void Game::disableHuman() {
   TRACE2("Game::disableHuman() - " << activeCards.size() << " cards");

   for (int i(activeCards.size()); i > 0;)
      activeCards[--i].disconnect();

   activeCards.clear();
}

//-----------------------------------------------------------------------------
/// Shuffles (Randomizes) the cards onto the staple
/// \param pile Pile to which the cards should be shuffeled to
/// \returns bool Flag, if method completed successfully
//-----------------------------------------------------------------------------
bool Game::randomiseCardsToPile(IPile& pile) const {
   // Randomize and put cards onto staple
#ifdef WITH_NETWORK
  YGP::ConnectionMgr& cmgr(getConnectionMgr());

   if (cmgr.getMode() == YGP::ConnectionMgr::CLIENT) {
      Check3(data && *data);
      std::string input(data);

      YGP::AttributeParse ap;
      ATTRIBUTE(ap, std::string, input, "Cards");
      try {
         ap.assignValues(input);

	 boost::tokenizer<> positions(input);
	 boost::tokenizer<>::iterator act(positions.begin ());
         TRACE8("Game::randomiseCardsToPile (IPile&) - Cards: " << cards.size ());
         for (unsigned int i(0); i < (cards.size() - 1); ++i) {
            unsigned long pos(0);
            std::string token;
            char* pTail(NULL);
            errno = 0;

            // Read next token; the value must be a number
            if ((act == positions.end())
                || stringToNumber(pos, act->c_str())
                || (pos >= cards.size())
                || (errno || (pTail && *pTail))) {
               std::string error(N_("Invalid card specification!"));
               throw YGP::CommError(error);
            }

            TRACE9("Game::randomiseCardsToPile(IPile&) const - [" << i << "] = " << pos);
            cards.set(i, pos);
         }
         writeOK(*cmgr.getSocket());
      }
      catch (YGP::CommError& error) {
         writeError(*cmgr.getSocket(), 99, error.what());
         Glib::ustring err(_("Received invalid input from the server!\n\nReason: %1"));
         err.replace(err.find("%1"), 2, _(error.what()));
         Gtk::MessageDialog dlg(err, false, Gtk::MessageType::ERROR, Gtk::ButtonsType::OK);
         dlg.set_title(PACKAGE);
         XGP::runModal (dlg);
         return false;
      }
   }
   else {
#endif
      cards.shuffle();
      std::ostringstream msg;
      msg << "Cards=";
      for (unsigned int i(0); i < cards.size(); ++i)
         msg << cards.getCard(i).id() << ' ';

      const_cast<Game*>(this)->cardOrder = msg.str();

#ifdef WITH_NETWORK
      if (cmgr.getMode() == YGP::ConnectionMgr::SERVER)
         broadcastMessage(cardOrder);
   }
#endif

   pile.setTopCards(cards.getCards());
   return true;
}

//-----------------------------------------------------------------------------
/// Cleans the table
//-----------------------------------------------------------------------------
void Game::clean() {
   TRACE8("Game::clean()");
   disableHuman();
   disableWonCards();
}

//-----------------------------------------------------------------------------
/// Activates the next player
//-----------------------------------------------------------------------------
void Game::makeNextMoves() {
   if (actPlayer >= 0) {
      TRACE8("Game::makeNextMoves() - " << actPlayer);
      Check3(actPlayer < static_cast<int>(actPlayers.size()));
      Check3(!stati.pendingTurn);

      disableHuman();
      unsigned int timeout(actPlayers[actPlayer]->timeout());
      if (timeout) {
         Glib::signal_timeout().connect
             (bind(mem_fun(*actPlayers[actPlayer], &Player::makeTurn), this), timeout);
         stati.pendingTurn = 1;
      }
      else
          Glib::signal_idle().connect
              (bind(mem_fun(*actPlayers[actPlayer], &Player::makeTurn), this));
   }
}


//-----------------------------------------------------------------------------
/// Ends the move of the passed remote player. This contains of executing the
/// move and re-enable receiving of messages
/// \param player ID of remote player
/// \returns bool False
//-----------------------------------------------------------------------------
bool Game::endRemoteMove(unsigned int player) {
   TRACE8("Game::endRemoteMove() - " << player);
   makeMove(player);
   mxSerializeMsgs.unlock();
   stati.pendingTurn = 0;
   makeNextMoves();
   return false;
}

//-----------------------------------------------------------------------------
/// Enables the cards of the human player
/// \returns bool False
//-----------------------------------------------------------------------------
bool Game::enableHuman() {
   Check3(!actPlayer ||(actPlayer == -1));
   TRACE8("Game::enableHuman() - enabling " << actPlayers[0]->getName());
   return false;
}

//-----------------------------------------------------------------------------
/// Makes the move for the next computer player.
/// \returns bool Flag for timer, if it should continue (false: no; else: yes)
//-----------------------------------------------------------------------------
bool Game::makeComputerMove() {
   TRACE5("Game::makeComputerMove() - Turn of player " << actPlayer);
   stati.pendingTurn = 0;
   if (statGame == TOSTOP)
      stop();
   else
      makeMove(actPlayer);
   return false;
}

//-----------------------------------------------------------------------------
/// Displays information about whose turn it is
/// \param player Player in turn
//-----------------------------------------------------------------------------
void Game::displayTurn(unsigned int player) {
   Check1(player < actPlayers.size());
   status.pop();
   Glib::ustring stat(_("Turn of %1"));
   stat.replace(stat.find("%1"), 2, actPlayers[player]->getName());
   status.push(stat);
}

//-----------------------------------------------------------------------------
/// Displays information about whose turn it is
/// \param player Player in turn
//-----------------------------------------------------------------------------
void Game::displayTurn(unsigned int player, const Glib::ustring& preText) {
   status.pop();
   Glib::ustring stat(_("Turn of %1"));
   stat.replace(stat.find("%1"), 2, actPlayers[player]->getName());
   status.push(preText + stat);
}

//-----------------------------------------------------------------------------
/// Changes the game-status
/// \param newStatus Status to set
//-----------------------------------------------------------------------------
void Game::setGameStatus(unsigned int newStatus) {
   statGame = newStatus;
   control(statGame);
}

//-----------------------------------------------------------------------------
/// Flips the cards the user is about to play
/// \param pile Pile to manipulate
/// \param cards String containing the (comma-separated) IDs of the cards to flip
//-----------------------------------------------------------------------------
void Game::flipCards2Play(IPile& pile, const std::string& cards) {
   TRACE2("Game::flipCards2Play(IPile&, const std::string&) - Cards " << cards);
   Check1(cards.size());

   boost::tokenizer<> tokCards(cards);
   unsigned long card(0);
   unsigned int cCards(0);
   bool bFollow(false);
   for (boost::tokenizer<>::iterator act(tokCards.begin()); act != tokCards.end(); ++act) {
      if (stringToNumber(card, act->c_str())) {
         std::string error(N_("Invalid card specification!"));
         throw YGP::ParseError(error);
      }

      card = pile.find(static_cast <unsigned int>(card));
      if ((card != -1U) && (card < (pile.size() - cCards))) {
         Check3(card < pile.size());
         TRACE9("Game::flipCards2Play(IPile&, const std::string&) - Found " << card << " = " << *pile[card]);
         ++cCards;

	 Card::Widget& cardWg(*pile[card]);
         pile.move(pile.size() - 1, card);
         cardWg.showFace();

         if ((pile.getStyle() != IPile::NORMAL) && bFollow) {
            Check3(pile.size() > 1);
            pile.resize(pile.size() - 2, IPile::COMPRESSED);
         }
         bFollow = true;
      }
      else {
         TRACE1("Game::flipCards2Play(IPile&, const std::string&) - Card " << *act << " not found in " << pile.size() << " cards");
         std::string error("Card not found!");
         throw YGP::ParseError(error);
      }
   } // end-while string has data
   pos2Play = pile.size() - 1;
   pos1Play = pos2Play - cCards + 1;
   Check3(pos1Play <= pos2Play);
   TRACE8("Game::flipCards2Play(IPile&, const std::string&) - New positions " << pos1Play << " and " << pos2Play);
}

//----------------------------------------------------------------------------
/// Returns the actual target, where flipCard2Play should position the cards to
/// \returns unsigned int ID of the target
//----------------------------------------------------------------------------
unsigned int Game::getActTarget() const {
   return 0;
}

//-----------------------------------------------------------------------------
/// Flips the cards the user is about to play
/// \param pile Pile to manipulate
/// \param start Position of first card to play; update to reflect moving
/// \param end Position of last card to play; update to reflect moving
/// \remarks It is safe to pass the same variable as start and end
//-----------------------------------------------------------------------------
void Game::flipCards2Play(IPile& pile, unsigned int& start, unsigned int& end) {
   TRACE2("Game::flipCards2Play(IPile&, unsigned int, unsigned int) - Cards " << start << " - " << end);
   Check3(end < pile.size());
   Check3(start <= end);

   unsigned int e(end);
   bool bFollow(false);

   // Inform clients about cards to play
   if (getConnectionMgr().getMode() == YGP::ConnectionMgr::SERVER) {
      std::ostringstream msg;
      msg << "Play=";
      for (unsigned int i(start); i < end; ++i)
         msg << pile[i]->id() << ' ';
      msg << pile[end]->id() << ";Target=" << getActTarget();

      broadcastMessage(msg.str());
   }

   do {
      Card::Widget& card(*pile[start]);
      pile.move(pile.size() - 1, start);
      card.showFace();

      if ((pile.getStyle() != IPile::NORMAL) && bFollow) {
         Check3(pile.size() > 1);
         pile.resize(pile.size() - 2, pile.getStyle());
      }
      bFollow = true;
   } while (e-- && (start <= e));

   start = pile.size() - 1 - (end - start);
   end =  pile.size() - 1;
   TRACE8("Game::flipCards2Play(IPile&, unsigned int, unsigned int) - New positions " << start << " and " << end);
}

//-----------------------------------------------------------------------------
/// Shows or hides the won cards
/// \param show Flag if to show or to hide the cards
/// \param style Style how pile should be displayed; must be a value understood
///     by IPile::setStyle
//-----------------------------------------------------------------------------
void Game::showWonCards(bool show, unsigned int style) {
   if (pWonPile) {
      if (style == -1U)
	 style = show ? IPile::COMPRESSED : IPile::VERY_COMPRESSED;
      Check3(style < IPile::LAST);

      pWonPile->setShowOption(show ? IPile::SHOWFACE : IPile::SHOWBACK);
      pWonPile->setStyle((IPile::PileStyle)style);
      Glib::signal_idle().connect(mem_fun(*this, &Game::enableActWonCards));
      disableWonCards();
   }
}

//-----------------------------------------------------------------------------
/// Callback for a left click onto the top of the won cards
//-----------------------------------------------------------------------------
void Game::wonCardsSelectedLeft() {
   TRACE9("Game::wonCardsSelectedLeft()");
   Check3(pWonPile);
   showWonCards(pWonPile->getShowOption() == IPile::SHOWBACK);
}

//-----------------------------------------------------------------------------
/// Callback for a right click onto the top of the won cards; shows a popup
/// menu to sort the won cards
/// \param x X-coordinate (card-relative) of the click
/// \param y Y-coordinate (card-relative) of the click
/// \param card Card which was clicked
//-----------------------------------------------------------------------------
void Game::wonCardsSelectedRight(double x, double y, Card::Widget& card) {
   TRACE9("Game::wonCardsSelectedRight(2x double, Widget&) - " << x << '/' << y);

   if (pMenuPopSort) {
      pMenuPopSort->unparent();
      delete pMenuPopSort;
      pMenuPopSort = NULL;
   }

   Glib::RefPtr<Gio::SimpleActionGroup> actions(Gio::SimpleActionGroup::create());
   Glib::RefPtr<Gio::Menu> menu(Gio::Menu::create());

   actions->add_action("sortnumber", mem_fun(*this, &Game::sortWonByNumber));
   menu->append(_("Sort by _number"), "wonsort.sortnumber");

   actions->add_action("sortcolour", mem_fun(*this, &Game::sortWonByColour));
   menu->append(_("Sort by _colour"), "wonsort.sortcolour");

   insert_action_group("wonsort", actions);

   pMenuPopSort = new Gtk::PopoverMenu(menu);
   pMenuPopSort->set_parent(*this);
   pMenuPopSort->set_has_arrow(false);

   double gx(x), gy(y);
   card.translate_coordinates(*this, x, y, gx, gy);
   Gdk::Rectangle rect(static_cast<int>(gx), static_cast<int>(gy), 1, 1);
   pMenuPopSort->set_pointing_to(rect);
   pMenuPopSort->popup();
}

//-----------------------------------------------------------------------------
/// Shows and sorts the won cards by number
//-----------------------------------------------------------------------------
void Game::sortWonByNumber() {
   TRACE8("Game::sortWonByNumber()");
   if (pWonPile) {
      pWonPile->sortByNumber();
      showWonCards();
      Glib::signal_idle().connect(mem_fun(*this, &Game::enableActWonCards));
      disableWonCards();
   }
}

//-----------------------------------------------------------------------------
/// Shows and sorts the won cards by colour
//-----------------------------------------------------------------------------
void Game::sortWonByColour() {
   TRACE8("Game::sortWonByColour()");
   if (pWonPile) {
      pWonPile->sortByColour();
      showWonCards();
      Glib::signal_idle().connect(mem_fun(*this, &Game::enableActWonCards));
      disableWonCards();
   }
}

//-----------------------------------------------------------------------------
/// Enables the actual won cards
//-----------------------------------------------------------------------------
bool Game::enableActWonCards() {
   disableWonCards();

   Check3(pWonPile);
   TRACE9("Game::enableActWonCards() - Enabling " << pWonPile->size() << " cards");
   for (int i(pWonPile->size()); i;) {
      Card::Widget& card(*(*pWonPile)[--i]);
      wonCards.push_back(card.signal_clicked().connect(mem_fun(*this, &Game::wonCardsSelectedLeft)));
      wonCards.push_back(card.signal_right_clicked().connect
                         (sigc::bind(mem_fun(*this, &Game::wonCardsSelectedRight), std::ref(card))));
   }
   return false;
}

//-----------------------------------------------------------------------------
/// Disables the won cards
//-----------------------------------------------------------------------------
void Game::disableWonCards() {
   TRACE8("Game::disableWonCards() - Disabling " << wonCards.size() << " cards");
   for (int i(wonCards.size()); i > 0;)
      wonCards[--i].disconnect();

   wonCards.clear();
}

//-----------------------------------------------------------------------------
/// Changes the names of the playing people
/// \param newNames Array holding the new names of the players
//-----------------------------------------------------------------------------
void Game::changeNames(const std::vector<Player*>& newPlayer) {
   const_cast<std::vector<Player*>&>(actPlayers) = newPlayer;
}

//----------------------------------------------------------------------------
/// Callback to inform a controller about status changes
/// \param status New status of the game
//----------------------------------------------------------------------------
void Game::control(unsigned int status) const {
}

//----------------------------------------------------------------------------
/// Writes a message to all partners
/// \param msg Message to write
//----------------------------------------------------------------------------
void Game::broadcastMessage(const std::string& msg) const {
   TRACE3("Game::broadcastMessage(const std::string&) - " << msg);

   const YGP::ConnectionMgr& cmgr(getConnectionMgr());
   if (getConnectionMgr().getMode() == YGP::ConnectionMgr::SERVER)
      for (std::vector<YGP::Socket*>::const_iterator i(cmgr.getClients().begin());
           i != cmgr.getClients().end(); ++i)
         writeMessage(**i, msg);
   else
       writeMessage(*cmgr.getSocket(), msg);
}

//----------------------------------------------------------------------------
/// Writes a set-startplayer message to all clients
/// \param startplayer Startplayer
//----------------------------------------------------------------------------
void Game::broadcastStartPlayer(unsigned int startplayer) {
   // Send startplayer to the clients
   const YGP::ConnectionMgr& cmgr(getConnectionMgr());
   if (cmgr.getMode() == YGP::ConnectionMgr::SERVER) {
      TRACE3("Game::broadcastStartPlayer(unsigned int) - " << startplayer);

      const std::vector<YGP::Socket*>& clients(cmgr.getClients());
      unsigned int player((startplayer - 1) & 0x3);
      for (std::vector<YGP::Socket*>::const_iterator i(clients.begin()); i != clients.end(); ++i) {
	 std::ostringstream msg;
	 msg << "ActPlayer=" << player;
	 writeMessage(**i, msg.str());
	 player =(player - 1) & 0x3;
      }
   }
}

//----------------------------------------------------------------------------
/// Writes a message to the partner
/// \param socket Socket to write message to
/// \param msg Message to write
//----------------------------------------------------------------------------
void Game::writeMessage(YGP::Socket& socket, const std::string& msg) {
   try {
      socket.write(msg);
      socket.write("\0", 1);
   }
   catch (YGP::CommError& error) {
      std::string err(_("Can't write message!\n\nReason: %1"));
      err.replace(err.find("%1"), 2, error.what());
      Gtk::MessageDialog dlg(msg, false, Gtk::MessageType::ERROR, Gtk::ButtonsType::OK);
      dlg.set_title(PACKAGE);
      XGP::runModal (dlg);
   }
}

//----------------------------------------------------------------------------
/// Writes a status message to the partner
/// \param socket Socket to write message to
/// \param rc Error code to send
/// \param msg Message to write
//----------------------------------------------------------------------------
void Game::writeError(YGP::Socket& socket, unsigned int rc, const std::string& msg) {
   std::ostringstream error;
   error << "Error=" << rc << ";Msg=\"" + msg << "\"\0";
   writeMessage(socket, error.str());
}

//----------------------------------------------------------------------------
/// Handles a message send from the server
/// \param player ID of player sending the message
/// \param msg Message to handle
/// \returns bool True, if the message has been processed completely; else
///    (if message is still pending) false
//----------------------------------------------------------------------------
bool Game::handleMessage(unsigned int player, const std::string& msg) {
   TRACE1("Game::handleMessage(unsigned int player, const std::string&) - " << msg << "(" << player << ')');
   Check1(msg.size());
   Check2(!data);

   bool rc(true);
   Check2(!ignoreNextMsg);
   switch (statGame) {
   case NONE:
      statGame = INITIALIZING;
      data = msg.c_str();
      start();
      data = NULL;
      break;

   case INITIALIZING:
      break;

   default:                          // Playing (and game specific stati)
      rc = performCommand(player, msg);
   }
   return rc;
}

//----------------------------------------------------------------------------
/// Sets the player performing the next turn
/// \param player Number identifying player (starting with 0)
//----------------------------------------------------------------------------
void Game::setNextPlayer(unsigned int player) {
   TRACE8("Game::setNextPlayer(unsigned int) - " << player);
   actPlayer = player;
}

//----------------------------------------------------------------------------
/// Handles a command the server sent in playing mode
/// \param player ID of player sending the message
/// \param msg Command to perform
/// \returns bool Flag, if command has been performed completely
//----------------------------------------------------------------------------
bool Game::performCommand(unsigned int player, const std::string& msg) {
   TRACE8("Game::performCommand(unsigned int player, const std::string&) - " << msg << " (" << player << ')');
   Check1(msg.size());

#if 0 // Only needed for network functionality
   YGP::Tokenize command(msg);
   std::string cmd(command.getNextNode('='));
   TRACE2("Game::performCommand(unsigned int player, const std::string&) - " << cmd);

   if (cmd == "Play") {
      cmd = command.getNextNode(';');
      std::string playTo(command.getNextNode('='));
      std::string strTarget(command.getNextNode(';'));

      unsigned long target(-1UL);
      if (stringToNumber(target, strTarget.c_str()) || (playTo != "Target"))
         throw YGP::ParseError(N_("Invalid target!"));

      Check3(actPlayer >= 0);
      IPile* pile(getPileOfPlayer(actPlayer, target));
      if (!pile)
         throw YGP::ParseError(N_("Invalid target!"));
      flipCards2Play(*pile, cmd);

      // Inform clients about cards to play
      if (getConnectionMgr().getMode() == YGP::ConnectionMgr::SERVER)
          broadcastMessage(msg);

      if (executeRemoteMove(*pile, target)) {
         Glib::signal_timeout().connect(bind(mem_fun(*this, &Game::endRemoteMove), actPlayer),
                                        ComputerPlayer::TIMEOUT);
         stati.pendingTurn = 1;
         return false;
      }
      else
         makeNextMoves();
   }
   else if (cmd == "ActPlayer") {
      cmd = command.getNextNode(';');
      TRACE8("Game::performCommand(unsigned int player, const std::string&) - Next player: " << cmd);
      unsigned long player;
      if (stringToNumber(player, cmd.c_str()))
         throw YGP::ParseError(N_("Invalid number"));

      actPlayer = player;
      if (statGame == PLAYING)
         displayTurn(player);
   }
   else if (cmd == "End") {
      end(false);
      if (getConnectionMgr().getMode() == YGP::ConnectionMgr::SERVER)
         broadcastMessage("End");
   }
   else
      throw YGP::ParseError(N_("Unknown command!"));
#endif
   return true;
}

//----------------------------------------------------------------------------
/// Converts a string into a number
/// \param number Target of conversion
/// \param text String to convert
/// \returns bool False, if conversion succeeded (\c text contained a number)
//----------------------------------------------------------------------------
bool Game::stringToNumber(unsigned long& number, const char* text) {
   Check1(text);
   char* pTail = NULL;
   errno = 0;
   number = strtoul(text, &pTail, 0);
   return (errno || (pTail && *pTail));
}

//----------------------------------------------------------------------------
/// Executes the remote move locally
/// \param pile Pile to move to/from
/// \param card ID of target, where to play the card
/// \returns bool True, if the timer to execute the move should be set
//----------------------------------------------------------------------------
bool Game::executeRemoteMove(IPile& pile, unsigned int card) {
   TRACE8("Game::executeRemoteMove(IPile&, unsigned int) - " << pos2Play);
   return true;
}

//----------------------------------------------------------------------------
/// Returns if the game can be stopped imediately. This is true, if there is no
/// timer activated.
/// \returns bool True, if the game can be stopped imediately
//----------------------------------------------------------------------------
bool Game::canBeStopped() const {
   return !(actPlayer && stati.pendingTurn);
}

//----------------------------------------------------------------------------
/// Checks if the game should ignore a message. If so, the count of messages
/// to ignore is reduced by 1.
/// \returns bool True, if a message should be ignored
//----------------------------------------------------------------------------
bool Game::ignoreMessage() {
   if (ignoreNextMsg) {
      TRACE9("Game::ignoreMessage() - Ignoring " << ignoreNextMsg);
      --ignoreNextMsg;
      return true;
   }
   return false;
}

//-----------------------------------------------------------------------------
/// Adds game-specific menus
/// \param menu Menu to add game-specific entries to
/// \param actions Action group to add game-specific actions to
//-----------------------------------------------------------------------------
void Game::addMenus(const Glib::RefPtr<Gio::Menu>&, const Glib::RefPtr<Gio::SimpleActionGroup>&) {
}

//-----------------------------------------------------------------------------
/// Removes the game-specific menus
/// \param menu Menu to remove game-specific entries from
/// \param actions Action group to remove game-specific actions from
//-----------------------------------------------------------------------------
void Game::removeMenus(const Glib::RefPtr<Gio::Menu>&, const Glib::RefPtr<Gio::SimpleActionGroup>&) {
}

//-----------------------------------------------------------------------------
/// Actions to take when the cards are resized
/// \pre The cardsize must be set in Images::WIDTH/HEIGHT
//-----------------------------------------------------------------------------
void Game::resizeCards() {
}


//-----------------------------------------------------------------------------
/// Animates the given cards to the given position of the passed pile
/// \param dest Destination pile
/// \param posDest Where to put the card in the destination
/// \param source Source pile
/// \param pos First card of source to move
/// \pre The card must be shown somewhere (to get its position)
//-----------------------------------------------------------------------------
Window& Game::animateCard(IPile& dest, unsigned int posDest, IPile& src, unsigned int pos) {
   TRACE3("Game::animateCard(...) - " << pos);
   Check1(pos < src.size());
   Check1(posDest <= dest.size());

   Window& win(*Window::create(dest, posDest, src, pos));
   unsigned int timeout(actPlayers[actPlayer]->timeout());
   if (timeout)
      Glib::signal_timeout().connect(bind_return(mem_fun(win, &Window::animate), false), timeout);
   else
      Glib::signal_idle().connect(bind_return(mem_fun(win, &Window::animate), false));
   return win;
}

//-----------------------------------------------------------------------------
/// Animates the given cards to the given position of the passed pile
/// \param dest Destination pile
/// \param posDest Where to put the card in the destination
/// \param source Source pile
/// \param start First card of source to move
/// \param end Last card of source to move
/// \pre The card must be shown somewhere (to get its position)
//-----------------------------------------------------------------------------
PileWindow& Game::animateCards(IPile& dest, unsigned int posDest, IPile& src,
                               unsigned int start, unsigned int end) {
   TRACE3("Game::animateCards(...) - " << start << '/' << end);
   Check1(end < src.size()); Check1(start <= end);
   Check1(posDest <= dest.size());

   PileWindow& win(*PileWindow::create(dest, posDest, src, start, end));
   unsigned int timeout(actPlayers[actPlayer]->timeout());
   if (timeout)
      Glib::signal_timeout().connect(bind_return(mem_fun(win, &PileWindow::animate), false), timeout);
   else
      Glib::signal_idle().connect(bind_return(mem_fun(win, &PileWindow::animate), false));
   return win;
}

//-----------------------------------------------------------------------------
/// Animates the given cards to the given position of the passed pile. Further
/// cards to be animated to the same destination can be added.
/// \param dest Destination pile
/// \param posDest Where to put the card in the destination
/// \param source Source pile
/// \param start First card of source to move
/// \param end Last card of source to move
/// \pre The card must be shown somewhere (to get its position)
//-----------------------------------------------------------------------------
PileWindows& Game::animateCards2(IPile& dest, unsigned int posDest, IPile& src,
				  unsigned int start, unsigned int end) {
   TRACE3("Game::animateCards2(...) - " << start << '/' << end);
   Check1(end < src.size()); Check1(start <= end);
   Check1(posDest <= dest.size());

   PileWindows& win(*PileWindows::create(dest, posDest, src, start, end));
   unsigned int timeout(actPlayers[actPlayer]->timeout());
   if (timeout)
      Glib::signal_timeout().connect(bind_return(mem_fun(win, &PileWindow::animate), false), timeout);
   else
      Glib::signal_idle().connect(bind_return(mem_fun(win, &PileWindow::animate), false));
   return win;
}

}
