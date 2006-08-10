//$Id$

//PROJECT     : Cardgames
//SUBSYSTEM   : Jabberwocky
//REFERENCES  :
//TODO        :
//BUGS        :
//REVISION    : $Revision$
//AUTHOR      : Markus Schwab
//CREATED     : 09.08.2006
//COPYRIGHT   : Copyright (C) 2006

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

#include <gtkmm/box.h>
#include <gtkmm/stock.h>
#include <gtkmm/button.h>
#include <gtkmm/statusbar.h>
#include <gtkmm/adjustment.h>
#include <gtkmm/spinbutton.h>

#define CHECK 9
#define TRACELEVEL 9
#include <YGP/Check.h>
#include <YGP/Trace.h>
#include <YGP/ConnMgr.h>
#include <YGP/ANumeric.h>

#include <Player.h>
#include <ScoreDlg.h>

#include "Jabberwocky.h"


const unsigned int Jabberwocky::COLS_PLAYER[NUM_PLAYERS] = { 7, 1, 7, 13 };
const unsigned int Jabberwocky::ROWS_PLAYER[NUM_PLAYERS] = { 4, 8, 10, 8 };


//-----------------------------------------------------------------------------
/// Constructor
/// \param parent: Parent widget to display the game in
/// \param statusbar: Status bar widget to display information about the game
/// \param cardset: Cardset to use
/// \param player: Vector of player
/// \param posPlayer: Position of player for the server
/// \param mxSerialize: Mutex to serialize messages from the server
//-----------------------------------------------------------------------------
Jabberwocky::Jabberwocky (Gtk::Box& parent, Gtk::Statusbar& statusbar, CardSet& cardset,
			  const std::vector<Player*>& player, unsigned int posPlayer,
			  YGP::Mutex& mxSerialize)
   : Game (parent, statusbar, cardset, player, posPlayer, mxSerialize, 15, 15),
     played (ICardPile::COMPRESSED, ICardPile::SHOWFACE),
     pTrump (NULL), startPlayer (rand () % NUM_PLAYERS), actTricks (3),
     pScoreDlg (NULL)
 {
   TRACE9 ("Jabberwocky::Jabberwocky (Box&, Statusbar&, CardSet&, ...)");

   int width (cards.getCard (0).getImageWidth ());
   int height (cards.getCard (0).getImageHeight ());

   // Show and attach card-piles
   changeNames (player);
   for (unsigned int i (0); i < NUM_PLAYERS; ++i) {
      attach (players[i].name, COLS_PLAYER[i], COLS_PLAYER[i] + 1,
              ROWS_PLAYER[i] + ((i == 2) ? 3 : 1),
              ROWS_PLAYER[i] + ((i == 2) ? 4 : 2),
              Gtk::EXPAND, Gtk::EXPAND, 1);

      attach (players[i].neededTricks, COLS_PLAYER[i] + 1, COLS_PLAYER[i] + 3,
              ROWS_PLAYER[i] + ((i == 2) ? 3 : 1),
              ROWS_PLAYER[i] + ((i == 2) ? 4 : 2),
 	      Gtk::EXPAND, Gtk::SHRINK, 1, 5);

      TRACE9 ("Jabberwocky::Jabberwocky () - Name at: " << COLS_PLAYER[i] << '/'
              << ROWS_PLAYER[i] + ((i == 2) ? 3 : 1));

      attach (players[i].won, COLS_PLAYER[i] + 1,
              COLS_PLAYER[i] + 2,
              ROWS_PLAYER[i] + ((i == 2) ? 2 : -2),
              ROWS_PLAYER[i] + ((i == 2) ? 3 : -1),
              Gtk::SHRINK, Gtk::SHRINK, 1);
      TRACE9 ("Jabberwocky::Jabberwocky () - Won pile at: "
              << COLS_PLAYER[i] + 1 << '/' << ROWS_PLAYER[i] + ((i == 2) ? 2 : -2));

      attach (players[i].hand, COLS_PLAYER[i],
              COLS_PLAYER[i] + 3, ROWS_PLAYER[i],
              ROWS_PLAYER[i] + 1, Gtk::SHRINK, Gtk::SHRINK, 1);
      TRACE9 ("Jabberwocky::Jabberwocky () - Hand at: "
              << COLS_PLAYER[i] << '/' << ROWS_PLAYER[i]);

      players[i].won.setShowOption (ICardPile::SHOWBACK);
      players[i].hand.setShowOption (i ? ICardPile::SHOWBACK : ICardPile::SHOWFACE);

      players[i].won.set_size_request (width + 20, height + 5);
      players[i].hand.set_size_request (width * 3, height + 5);
   }

   for (unsigned int i (1); i < NUM_PLAYERS; ++i) {
      players[i].hand.setStyle (ICardPile::QUITE_COMPRESSED);
      players[i].won.setStyle (ICardPile::VERY_COMPRESSED);
   }
   players[0].hand.setStyle (ICardPile::COMPRESSED);
   players[0].won.setStyle (ICardPile::QUITE_COMPRESSED);

   attach (played, 3, 11, 6, 9, Gtk::SHRINK, Gtk::SHRINK, 0, 5);
   played.set_size_request (width + 150, height);

   show_all_children ();
}

//-----------------------------------------------------------------------------
/// Destructor
//-----------------------------------------------------------------------------
Jabberwocky::~Jabberwocky () {
}


//-----------------------------------------------------------------------------
/// Starts the game
//-----------------------------------------------------------------------------
void Jabberwocky::start () {
   TRACE8 ("Jabberwocky::start ()");
   Game::start ();
   ICardPile pile;
   if (randomizeCardsToPile (pile)) {
      // Show cards on the table: For all players put 3 cards in hand
      for (unsigned int i (0); i < NUM_PLAYERS; ++i)
         for (unsigned int j (0); j < actTricks; ++j)
            players[(i - posServer) & 0x3].hand.insertSorted (pile.removeTopCard ());

      pos1Play = pos2Play = -1U;

      makeBets ();
   }
}

//-----------------------------------------------------------------------------
/// Remove cards from everything which can hold them
//-----------------------------------------------------------------------------
void Jabberwocky::clean () {
   for (unsigned int i (0); i < NUM_PLAYERS; ++i) {   // Clear cards of players
      players[i].hand.clear ();
      players[i].won.clear ();

      players[i].neededTricks.set_text ("");
   }
   played.clear ();

   disableHuman ();
   if (pTrump) {
      remove (*pTrump);
      delete pTrump;
      pTrump = NULL;
   }
   Game::clean ();
}

//-----------------------------------------------------------------------------
/// Enables the cards of the passed player
/// \returns bool: Flag, if timer should be continued; False
/// \remarks Depending of the status of the game (PLAYING2) also the top card
///     of the played pile is enabled
//-----------------------------------------------------------------------------
bool Jabberwocky::enableHuman () {
   Check3 (activeCards.empty ());
   Check3 (gameStatus () == PLAYING);

   TRACE2 ("Jabberwocky::enableHuman () - Has " << players[0].hand.size ()
           << " cards");

   for (int i (players[0].hand.size () - 1); i >= 0; --i)
      activeCards.push_back
         (players[0].hand[i]->signal_clicked ().connect
           (bind (mem_fun (*this, (&Jabberwocky::cardSelected)), i)));

   return Game::enableHuman ();
}

//-----------------------------------------------------------------------------
/// Makes the move for the next player.
/// \param player: Actual player
/// \returns int: Next player or -1 if end of game
//-----------------------------------------------------------------------------
int Jabberwocky::makeMove (unsigned int player) {
   TRACE5 ("Jabberwocky::makeMove () - Turn of player " << player);
   Check3 (gameStatus () == PLAYING);

#if 0
   if (pos2Play == -1U) {
      if (findPos2Play (player, pos1Play, pos2Play) != -1) {
         // Flip card(s) to play
         flipCards2Play (players[player].hand, pos1Play, pos2Play);
         return player;
      }
      else {
         if ((gameStatus () == PLAYING2)
             && (getConnectionMgr ().getMode () == YGP::ConnectionMgr::SERVER)) {
            std::ostringstream msg;
            Check3 (startPos[offPos - 1] < played.size ());
            msg << "Play=" << played[startPos[offPos - 1]]->id () << ";Target=1";
            broadcastMessage (msg.str ());
         }
         player = pickUpPlayedPile (player);
      }
   }
   else {
      player = executeMove (player, pos1Play, pos2Play);
      pos1Play = pos2Play = -1U;
   }
#else
   player = (player + 1) % NUM_PLAYERS;
#endif
   return player;
}

//-----------------------------------------------------------------------------
/// Shows or hides the cards of the computer player
/// \param open: Flag if cards should be shown or hidden
//-----------------------------------------------------------------------------
void Jabberwocky::playOpen (bool open) {
   ICardPile::ShowOpt show (open ? ICardPile::SHOWFACE : ICardPile::SHOWBACK);

   for (unsigned int i (0); i < NUM_PLAYERS; ++i) {
      players[i].won.setShowOption (show);
      players[i].won.setStyle (open ? ICardPile::COMPRESSED : ICardPile::VERY_COMPRESSED);
      if (i)
         players[i].hand.setStyle (open ? ICardPile::COMPRESSED : ICardPile::VERY_COMPRESSED);
      players[i].hand.setShowOption (i ? show : ICardPile::SHOWFACE);
   }
}

//-----------------------------------------------------------------------------
/// Changes the names of the playing people
/// \param newPlayer: Array holding the new player
//-----------------------------------------------------------------------------
void Jabberwocky::changeNames (const std::vector<Player*>& newPlayer) {
   Game::changeNames (newPlayer);

   std::vector<Player*> player;
   for (unsigned int i (0); i < NUM_PLAYERS; ++i) {
      player.push_back (actPlayers[(i + posServer) & 0x3]);
      players[i].name.set_text (actPlayers[i]->getName ());
   }

   if (pScoreDlg)
      pScoreDlg->update (player);
}

//----------------------------------------------------------------------------
/// Converts a pile-number to the actual pile
/// \param newPlayer: Array holding the new player
/// \param pile: ID of the pile to return
/// \returns ICardPile*: Pile corresponding to the passed number or NULL
//----------------------------------------------------------------------------
ICardPile* Jabberwocky::getPileOfPlayer (unsigned int player, unsigned int pile) {
   TRACE8 ("Jabberwocky::getPileOfPlayer (unsigned int, unsigned int) - Player "
           << player << "; Pile " << pile);
   if ((player >= NUM_PLAYERS) || (pile > 1))
      return NULL;

   return &(pile ? played : players[player].hand);
}

//-----------------------------------------------------------------------------
/// Adds game-specific menus
/// \param mgrUI: UIManager to add to
//-----------------------------------------------------------------------------
void Jabberwocky::addMenus (Glib::RefPtr<Gtk::UIManager> mgrUI) {
   Check1 (mgrUI);
   Glib::ustring ui ("<menubar name='Menu'>"
		     "  <placeholder name='GameMenu'>"
		     "    <menu action='MB'>"
		     "      <menuitem action='JabberwockySort'/>"
		     "      <menuitem action='JabberwockySortCol'/>"
		     "    </menu></placeholder></menubar>");

   Glib::RefPtr<Gtk::ActionGroup> grpAction (Gtk::ActionGroup::create ());
   grpAction->add (Gtk::Action::create ("MB", _("_Jabberwocky")));
   grpAction->add (Gtk::Action::create ("JabberwockySort", Gtk::Stock::SORT_ASCENDING,
					_("_Sort won cards (by number)")),
		   Gtk::AccelKey ("<shft>S"),
		   mem_fun (*this, &Jabberwocky::sortWonByNumber));
   grpAction->add (Gtk::Action::create ("JabberwockySortCol", Gtk::Stock::SORT_ASCENDING,
					_("Sort won cards (by _colour)")),
		   Gtk::AccelKey ("S"),
		   mem_fun (*this, &Jabberwocky::sortWonByColour));

   mgrUI->insert_action_group (grpAction);
   idMrg = mgrUI->add_ui_from_string (ui);
}

//-----------------------------------------------------------------------------
/// Removes the game-specific menus
/// \param mgrUI: UIManager to remove from
//-----------------------------------------------------------------------------
void Jabberwocky::removeMenus (Glib::RefPtr<Gtk::UIManager> mgrUI) {
   Check1 (mgrUI);
   mgrUI->remove_ui (idMrg);
}

//-----------------------------------------------------------------------------
/// Callback after clicking on a card in hand
/// \param pos: Offset of card in hand
//-----------------------------------------------------------------------------
void Jabberwocky::cardSelected (unsigned int pos) {
   TRACE5 ("Twopart::cardSelected (unsigned int) - Position " << pos);
   Check3 (pos < players[0].hand.size ());
   Check3 (gameStatus () == PLAYING);
}

//-----------------------------------------------------------------------------
/// Makes (or waits) for the bets of the users
/// \param start: Number of first player to make its bet
/// \param end: Number of last player to make its bet
//-----------------------------------------------------------------------------
void Jabberwocky::makeBets (unsigned int start, unsigned int end) {
   TRACE8 ("Jabberwocky::makeBets () - [" << start << '-' << end << ']');
   Check1 (end < NUM_PLAYERS);

   // Make the remaining bets
   while (start <= end) {
      unsigned int actPlayer ((startPlayer + start++) % NUM_PLAYERS);
      if (actPlayer) {
	 players[actPlayer].bet = 1;
	 showBet (actPlayer);
      }
      else {
	 status.pop ();
	 status.push (_("Make your bet for the number of ticks you are going to make!"));

	 Gtk::Button* bet (new Gtk::Button (_("_Bet"), true));
	 Gtk::Adjustment* adj (new Gtk::Adjustment (0, 0.0, actTricks, 1, 2));
	 Gtk::SpinButton* value (new Gtk::SpinButton (*manage (adj), 1, 0));
	 bet->show ();
	 value->show ();

	 attach (*value, COLS_PLAYER[0] + 1, COLS_PLAYER[0] + 2,
		 ROWS_PLAYER[0] + 1, ROWS_PLAYER[0] + 2,
		 Gtk::EXPAND, Gtk::SHRINK, 0, 5);
	 attach (*bet,   COLS_PLAYER[0] + 2, COLS_PLAYER[0] + 3,
		 ROWS_PLAYER[0] + 1, ROWS_PLAYER[0] + 2,
		 Gtk::SHRINK, Gtk::SHRINK, 0, 5);

	 bet->signal_clicked ().connect (bind (mem_fun (*this, &Jabberwocky::placedBet), value, bet,  start, end));
	 return;
      }
   }

   startGame ();
}

//-----------------------------------------------------------------------------
/// After the initial betting phase: Start the actual game
//-----------------------------------------------------------------------------
void Jabberwocky::startGame () {
   TRACE2 ("Jabberwocky::startGame () - " << startPlayer);
   if (getConnectionMgr ().getMode () != YGP::ConnectionMgr::CLIENT) {
      setNextPlayer (startPlayer = (startPlayer + 1) % NUM_PLAYERS);
      broadcastStartPlayer (startPlayer);
   }

   displayTurn (currentPlayer ());
   makeNextMoves ();
}

//-----------------------------------------------------------------------------
/// Commits the bet of the user and continues betting
/// \param value: Entryfield where user entered his bet
/// \param commit: Button commiting the bet
/// \param start: Number of first player to make its bet
/// \param end: Number of last player to make its bet
//-----------------------------------------------------------------------------
void Jabberwocky::placedBet (Gtk::SpinButton* value, Gtk::Button* commit,
			     unsigned int start, unsigned int end) {
   status.pop ();

   players[0].bet = YGP::ANumeric (value->get_text ());

   remove (*value);
   remove (*commit);

   delete value;
   delete commit;

   showBet (0);
   makeBets (start, end);
}

//-----------------------------------------------------------------------------
/// Shows the bet the passed player has set
/// \param player: Player whose bet shall be shown
//-----------------------------------------------------------------------------
void Jabberwocky::showBet (unsigned int player) {
   Glib::ustring tricks (_("Bets %1 tricks"));
   tricks.replace (tricks.find ("%1"), 2, YGP::ANumeric::toString (players[player].bet));
   players[player].neededTricks.set_text (tricks);
}

