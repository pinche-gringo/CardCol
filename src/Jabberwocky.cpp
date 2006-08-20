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
#include <gtkmm/messagedialog.h>

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

char Jabberwocky::sortOrder[4];


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
     pTrump (NULL), startPlayer (rand () % NUM_PLAYERS), turn (0),
     pScoreDlg (NULL)
 {
   TRACE9 ("Jabberwocky::Jabberwocky (Box&, Statusbar&, CardSet&, ...)");

   int width (cards.getCard (0).getImageWidth ());
   int height (cards.getCard (0).getImageHeight ());

   // Show and attach card-piles
   changeNames (player);
   for (unsigned int i (0); i < NUM_PLAYERS; ++i) {
      attach (players[i].name, COLS_PLAYER[i], COLS_PLAYER[i] + 3,
              ROWS_PLAYER[i] + ((i == 2) ? 3 : 1),
              ROWS_PLAYER[i] + ((i == 2) ? 4 : 2),
              Gtk::EXPAND, Gtk::EXPAND, 1);

      TRACE9 ("Jabberwocky::Jabberwocky () - Name at: " << COLS_PLAYER[i] << '/'
              << ROWS_PLAYER[i] + ((i == 2) ? 3 : 1));

      attach (players[i].won, COLS_PLAYER[i], COLS_PLAYER[i] + 2,
              ROWS_PLAYER[i] + ((i == 2) ? 2 : -2),
              ROWS_PLAYER[i] + ((i == 2) ? 3 : -1),
              Gtk::SHRINK, Gtk::SHRINK, 1);
      TRACE9 ("Jabberwocky::Jabberwocky () - Won pile at: "
              << COLS_PLAYER[i] << '/' << ROWS_PLAYER[i] + ((i == 2) ? 2 : -2));

      attach (players[i].hand, COLS_PLAYER[i],
              COLS_PLAYER[i] + 3, ROWS_PLAYER[i],
              ROWS_PLAYER[i] + 1, Gtk::SHRINK, Gtk::SHRINK, 1);
      TRACE9 ("Jabberwocky::Jabberwocky () - Hand at: " << COLS_PLAYER[i] << '/' << ROWS_PLAYER[i]);

      players[i].won.setShowOption (ICardPile::SHOWBACK);
      players[i].hand.setShowOption (i ? ICardPile::SHOWBACK : ICardPile::SHOWFACE);

      players[i].won.set_size_request (width + 20, height + 5);
      players[i].hand.set_size_request ((i & 1) ? width + 8 * 5: width * 3, height + 5);
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
   startPlayer = (startPlayer + 1) % NUM_PLAYERS;

   ICardPile pile;
   if (randomizeCardsToPile (pile)) {
      // Show cards on the table: For all players put 3 cards in hand
      for (unsigned int i (0); i < NUM_PLAYERS; ++i) {
	 players[i].bet.undefine ();
	 players[i].name.set_text (actPlayers[i]->getName ());
         for (unsigned int j (0); j < getTricks (turn); ++j)
            players[(i - posServer) % NUM_PLAYERS].hand.setTopCard (pile.removeTopCard ());
      }

      pos1Play = pos2Play = -1U;

      pTrump = &pile.removeShownTopCard ();
      attach (*pTrump, 1, 2, 2, 3, Gtk::SHRINK, Gtk::SHRINK, 5, 5);
      pTrump->show ();
      TRACE8 ("Jabberwocky::start () - Trump: " << *pTrump);

      // Set how to sort the colours
      for (unsigned int i (0); i < 4; ++i)
	 sortOrder[i] = (i - pTrump->colour () + 3) % NUM_PLAYERS;

      // Sort cards according to trump
      for (unsigned int i (0); i < NUM_PLAYERS; ++i)
	 players[(i - posServer) % NUM_PLAYERS].hand.sort (compByColourAccTrumps);

      // Resetting all status-information
      for (unsigned int i (0); i < (sizeof (playedCards) / sizeof (playedCards[0])); ++i)
	 playedCards[i].reset ();
      playedCards[pTrump->colour ()].set (pTrump->number ());
      memset (outOfColour, 0, sizeof (outOfColour));

      if (!turn && pScoreDlg) {
	 delete pScoreDlg;
	 pScoreDlg = NULL;
      }

      makeBets ();
      pile.clear ();
   }
}

//-----------------------------------------------------------------------------
/// Remove cards from everything which can hold them
//-----------------------------------------------------------------------------
void Jabberwocky::clean () {
   TRACE9 ("Jabberwocky::clean ()");
   for (unsigned int i (0); i < NUM_PLAYERS; ++i) {   // Clear cards of players
      players[i].hand.clear ();
      players[i].won.clear ();
   }
   played.clear ();

   while (status.children ().size () > 1)
      status.children ().remove (status.children ()[1]);

   disableHuman ();
   if (pTrump) {
      remove (*pTrump);
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
   TRACE2 ("Jabberwocky::enableHuman () - Has " << players[0].hand.size ()
           << " cards");
   Check3 (activeCards.empty ());
   Check3 (gameStatus () == PLAYING);

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

   if (pos2Play == -1U)
      showCards2Play (player);
   else {
      player = playCard (player, pos2Play);
      pos2Play = -1U;
   }
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
      player.push_back (actPlayers[(i + posServer) % NUM_PLAYERS]);
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
   TRACE5 ("Jabberwocky::cardSelected (unsigned int) - Position " << pos);
   Check3 (pos < players[0].hand.size ());
   Check3 (gameStatus () == PLAYING);
   Check2 (pTrump);

   try {
      CardWidget& card (*players[0].hand[pos]);
      TRACE4 ("Jabberwocky::cardSelected (unsigned int) - Playing " << card);
      if (played.size ()) {
	 if ((card.colour () != played[0]->colour ()) && players[0].hand.exists (played[0]->colour ()))
	    throw _("Play first cards with an equal colour as the first played one!");
      }
      else
	 // One can start with a trump only if there hasn't been one played before
	 // Note that also the cards shown as trump is counted as played, so
	 // test accordingly
	 if (((card.colour () == pTrump->colour ())
	      && (playedCards[card.colour ()].count () == 1)
	      && ((players[0].hand)[0]->colour () != pTrump->colour ())))
	    throw _("You can't start with a trump, if they have not been played before!");

      if ((pos = playCard (0, pos)) != -1U) {
	 setNextPlayer (pos);
	 makeNextMoves ();
      }
   }
   catch (Glib::ustring& error) {
      Gtk::MessageDialog dlg (error, Gtk::MESSAGE_ERROR);
      dlg.set_title (_("Jabberwocky"));
      dlg.run ();
   }
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
	 // Estimate the tricks for the player; take care the last player
	 // does not place a bet which sums all bets up to the number of players
	 players[actPlayer].bet = calcTricks (actPlayer);
	 if ((start == end) && (sumBets () == getTricks (turn)))
	    players[actPlayer].bet += (rand () & 1) ? 1 : -1;
	 showBet (actPlayer);
      }
      else {
	 status.pop ();
	 status.push (_("Make your bid for the number of tricks you are going to make!"));

	 Gtk::Button* bet (new Gtk::Button (_("_Bet"), true));
	 Gtk::Adjustment* adj (new Gtk::Adjustment (0, 0.0, getTricks (turn), 1, 2));
	 Gtk::SpinButton* value (new Gtk::SpinButton (*manage (adj), 1, 0));
	 bet->show ();
	 value->show ();

	 status.pack_start (*value, Gtk::PACK_SHRINK, 5);
	 status.pack_start (*bet, Gtk::PACK_SHRINK, 5);

	 bet->signal_clicked ().connect (bind (mem_fun (*this, &Jabberwocky::placedBet), value, bet,  start, end));
	 return;
      }
   }

   startGame ();
}

//-----------------------------------------------------------------------------
/// Returns the sum of all bets
/// \returns unsinged int: Sum o fall bets
//-----------------------------------------------------------------------------
unsigned int Jabberwocky::sumBets () const {
   unsigned int sum (0);
   for (unsigned int i (0); i < NUM_PLAYERS; ++i)
      sum += (unsigned int)players[i].bet;
   return sum;
}

//-----------------------------------------------------------------------------
/// After the initial betting phase: Start the actual game
//-----------------------------------------------------------------------------
void Jabberwocky::startGame () {
   TRACE2 ("Jabberwocky::startGame () - " << startPlayer);
   if (getConnectionMgr ().getMode () != YGP::ConnectionMgr::CLIENT) {
      setNextPlayer (startPlayer);
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
   TRACE4 ("Jabberwocky::placedBet (Gtk::SpinButton*, Gtk::Button*, 2x unsigned int) - [" << start << '-' << end << ']');
   Check1 (end < NUM_PLAYERS);
   Check1 (commit); Check1 (value);

   commit->grab_focus ();
   players[0].bet = YGP::ANumeric (value->get_text ());
   if ((start == end) && (sumBets () == getTricks (turn))) {
      Gtk::MessageDialog dlg (_("The sum of all bids must be different\nthan the number of players!"), Gtk::MESSAGE_ERROR);
      dlg.set_title (_("Jabberwocky"));
      dlg.run ();
   }
   else {
      status.pop ();

      status.remove (*value);
      status.remove (*commit);

      delete value;
      delete commit;

      showBet (0);
      makeBets (start, end);
   }
}

//-----------------------------------------------------------------------------
/// Shows the bet the passed player has set
/// \param player: Player whose bet shall be shown
//-----------------------------------------------------------------------------
void Jabberwocky::showBet (unsigned int player) {
   if (players[player].bet.isDefined ()) {
      Glib::ustring tricks (_("; bets %1 tricks"));
      tricks.replace (tricks.find ("%1"), 2, players[player].bet.toString ());
      players[player].name.set_text (actPlayers[player]->getName () + tricks);
   }
}


//-----------------------------------------------------------------------------
/// Calculates the bets for the passed player
/// \param player: Number of player to calculate tricks to make for
/// \returns unsigned int: Number of tricks player will win
//-----------------------------------------------------------------------------
unsigned int Jabberwocky::calcTricks (unsigned int player) const {
   TRACE5 ("Jabberwocky::calcTricks (unsigned int) - " << player);
   Check1 (player < NUM_PLAYERS); Check3 (pTrump);

   // Analyse cards to estimate tricks it will win
   unsigned int tricks (0);
   unsigned int left (cards.size () - 1 - NUM_PLAYERS * getTricks (turn));

   for (ICardPile::const_iterator i (players[player].hand.begin ());
	i != players[player].hand.end (); ++i) {
      if ((*i)->colour () == pTrump->colour ()) {
	 if (((*i)->number () > CardWidget::EIGHT) || (left > 19))
	    ++tricks;
      }
      else
	 if (isHighEnough (**i))
	    ++tricks;
   }

   return tricks;
}

//-----------------------------------------------------------------------------
/// Compares the cards in the pile with regard of the colour and with special
/// consideration of trumps
/// \param a: Card to compare
/// \param b: Card to compare
/// \returns bool: True, if a < b
//-----------------------------------------------------------------------------
bool Jabberwocky::compByColourAccTrumps (const CardWidget* a, const CardWidget* b) {
   Check3 (a); Check3 (b);

   return ((a->colour () == b->colour ())
           ? a->number () < b->number ()
           : (sortOrder[a->colour ()] < sortOrder[b->colour ()]));
}

//-----------------------------------------------------------------------------
/// Shows the card to play
/// \param player: Player to inspect
//-----------------------------------------------------------------------------
void Jabberwocky::showCards2Play (unsigned int player) {
   TRACE3 ("Jabberwocky::showCards2Play (unsigned int) - Player: " << player);
   Check1 (player < NUM_PLAYERS); Check3 (pos2Play == -1U);
   Check2 (played.size () < NUM_PLAYERS);

   ICardPile& hand (players[player].hand);
   int aPosColours[4];
   getPositionOfColours (hand, aPosColours);
   TRACE3 ("Jabberwocky::showCards2Play (unsigned int) - Missing tricks: " << ((int)players[player].bet - (players[player].won.size () / NUM_PLAYERS)));

   // Already cards played?
   if (played.size ()) {
      unsigned int posWinner (check4Winner ());
      TRACE4 ("Jabberwocky::showCards2Play (unsigned int) - Winning card: " << posWinner << " (" << *played[posWinner] << ')');

      // If the player still has bets to fullfill
      if ((unsigned int)players[player].bet > (players[player].won.size () / NUM_PLAYERS)) {
	 // Can follow suit?
	 if (aPosColours[played[0]->colour ()] == -1) {
	    TRACE7 ("Jabberwocky::showCards2Play (unsigned int) - Can't follow suit");
	    outOfColour[player][played[0]->colour ()] = true;

	    unsigned int p (NUM_PLAYERS - played.size ()); Check3 (p);
	    while (--p) {
	       if (outOfColour[(player + p) % NUM_PLAYERS][played[0]->colour ()]) {
		  TRACE9 ("Jabberwocky::showCards2Play (unsigned int) - Out of suit: " << p);
		  // Check if player can get the pile
		  pos2Play = (((aPosColours[pTrump->colour ()] != -1)
			       && (isHighest (*hand[aPosColours[pTrump->colour ()]])
				   || (isHighEnough (*hand[aPosColours[pTrump->colour ()]]))))
			      ? aPosColours[pTrump->colour ()]
			      : findWorstCard (hand, aPosColours)); Check3 (pos2Play != -1U);
		  break;
	       }
	    }
	    // Other players still seem to have the colour
	    if (!p) {
	       // Try to get the card with a trump; else put worst card
	       if ((aPosColours[pTrump->colour ()] == -1)
		   || ((played[posWinner]->colour () == pTrump->colour ())
		       ? ((pos2Play = findHigherCard (*played[posWinner], hand, aPosColours[played[posWinner]->colour ()])) == -1U)
		       : ((pos2Play = hand.findFirstEqualColour (aPosColours[pTrump->colour ()])) == -1U)))
		  pos2Play = findWorstCard (hand, aPosColours);
	       Check3 (pos2Play != -1U);
	    }
	    TRACE5 ("Jabberwocky::showCards2Play (unsigned int) - Can't follow suit: " << pos2Play);
	 }
	 // Can follow suit
	 else {
	    TRACE5 ("Jabberwocky::showCards2Play (unsigned int) - Can follow suit: " << aPosColours[played[0]->colour ()]);

	    // If a trump has been played after non-trump, play something low
	    if (played[posWinner]->colour () != played[0]->colour ())
	       pos2Play = hand.findFirstEqualColour (aPosColours[played[0]->colour ()]);
	    else {
	       // TODO: Check if following people can't possibly take trick
	       // Last in turn
	       if (played.size () == (NUM_PLAYERS - 1))
		  pos2Play = ((hand[aPosColours[played[0]->colour ()]]->number () > played[posWinner]->number ())
			      ? findHigherCard (*played[posWinner], hand, aPosColours[played[0]->colour ()])
			      : hand.findFirstEqualColour (aPosColours[played[0]->colour ()]));
	       else
		  if ((hand[aPosColours[played[0]->colour ()]]->number () > played[posWinner]->number ())
		      && (isHighest (*hand[aPosColours[played[0]->colour ()]])
			  || (isHighEnough (*hand[aPosColours[played[0]->colour ()]]))))
		     pos2Play = aPosColours[played[0]->colour ()];
		  else
		     pos2Play = hand.findFirstEqualColour (aPosColours[played[0]->colour ()]);
	    }
	    TRACE5 ("Jabberwocky::showCards2Play (unsigned int) - Can follow suit - End: " << pos2Play);
	 }
      }
      // Doesn't need any more tricks
      else
	 if ((aPosColours[played[0]->colour ()] != -1)
	     && (played[0]->colour () == played[posWinner]->colour ()))
	    pos2Play = findLowerCard (*played[posWinner], hand, aPosColours[played[posWinner]->colour ()]);
	 else {
	    unsigned int offset (0);
	    for (unsigned int i (1); i < (sizeof (aPosColours) / sizeof (aPosColours[0])); ++i) {
	       TRACE5 ("Jabberwocky::showCards2Play (unsigned int) - No more tricks; Colour: " << i);
	       if ((CardWidget::COLOURS (i) != pTrump->colour ())
		   && (aPosColours[i] != -1)
		   && ((aPosColours[offset] == -1)
		       || ((hand[aPosColours[i]]->number () > hand[offset]->number ())
			   || ((hand[aPosColours[i]]->number () == hand[offset]->number ())
			       && (playedCards[hand[aPosColours[i]]->colour ()].count ()
				   < playedCards[hand[aPosColours[offset]]->colour ()].count ())))))
		  offset = i;
	    }
	    pos2Play = (aPosColours[offset] == -1) ? aPosColours[pTrump->colour ()] : aPosColours[offset];
	 }
   }
   // First card to play
   else {
      // If the player still has bets to fullfill
      if ((unsigned int)players[player].bet < (players[player].won.size () / NUM_PLAYERS))
	 for (unsigned int i (0); i < 4; ++i)
	    if (CardWidget::COLOURS (i) != pTrump->colour ())
	       if ((aPosColours[CardWidget::COLOURS (i)] != -1)
		   && (isHighest (*hand[aPosColours[CardWidget::COLOURS (i)]])
		       || isHighEnough (*hand[aPosColours[CardWidget::COLOURS (i)]])))
		    pos2Play = aPosColours[CardWidget::COLOURS (i)];

      if (pos2Play == -1U)
	 pos2Play = findWorstCard (hand, aPosColours);
   }


   Check3 (pos2Play < hand.size ());
   TRACE1 ("Jabberwocky::showCards2Play (unsigned int) - Playing: " << pos2Play << " (" << *hand[pos2Play] << ')');
   flipCards2Play (hand, pos2Play, pos1Play = pos2Play);
}

//-----------------------------------------------------------------------------
/// Find the highest card lower than the passed one or the lowest card
/// of the colour of the passed card to match
/// \param cardCmp: Card which should not be passed
/// \param pile: Pile from which to play
/// \param aPosColour: Position of last card in the pile with that colour
/// \returns \c Position of card to play
//-----------------------------------------------------------------------------
unsigned int Jabberwocky::findLowerCard (const CardWidget& cardCmp, const ICardPile& pile, int aPosColour) const {
   TRACE9 ("Jabberwocky::findLowerCard (const CardWidget&, const ICardPile&, int) - " << aPosColour);
   Check1 ((unsigned int)aPosColour < pile.size ());
   Check2 (pile[aPosColour]->colour () == cardCmp.colour ());
   Check3 (played.size ());

   // Search for a lower card
   while (aPosColour >= 0 && (pile[aPosColour]->colour () == cardCmp.colour ())) {
      if (pile[aPosColour]->number () < cardCmp.number ()) {
	 TRACE5 ("Jabberwocky::findLowerCard (const CardWidget&, const ICardPile&, int) - "
		 "Playing card at " << aPosColour  << ": " << *pile[aPosColour]);
	 return aPosColour;
      }
      --aPosColour;
   }

   TRACE5 ("Jabberwocky::findLowerCard (const CardWidget&, const ICardPile&, int) - "
	   "Forced to play card at " << aPosColour + 1 << ": " << *pile[aPosColour + 1]);
   return aPosColour + 1;
}

//-----------------------------------------------------------------------------
/// Find the lowest card higher than the passed one
/// \param cardCmp: Card which should not be passed
/// \param pile: Pile from which to play
/// \param aPositions: Array with positions of cards
/// \returns \c Position of card to play
//-----------------------------------------------------------------------------
unsigned int Jabberwocky::findHigherCard (const CardWidget& cardCmp, const ICardPile& pile, unsigned int aPosColour) const {
   TRACE9 ("Jabberwocky::findHigherCard (const CardWidget&, const ICardPile&, unsigned int)");
   Check1 (aPosColour < pile.size ());
   Check2 (pile[aPosColour]->colour () == cardCmp.colour ());
   Check3 (played.size ());

   unsigned int pos (-1U);
   while (pile[aPosColour]->number () > cardCmp.number ()) {
      pos = aPosColour;
      if (!aPosColour-- || (pile[aPosColour]->colour () != cardCmp.colour ()))
	 break;
   } // end-while

   TRACE5 ("Jabberwocky::findHigherCard (const CardWidget&, const ICardPile&, unsigned int) - Playing card at " << pos);
   return pos;
}

//-----------------------------------------------------------------------------
/// Checks who has played the highest card and would therefore win the played
/// pile
/// \returns \c ID of player with the highest card
//-----------------------------------------------------------------------------
unsigned int Jabberwocky::check4Winner () const {
   Check2 (played.size ()); Check2 (pTrump);
   CardWidget::COLOURS colour (played[0]->colour ());
   CardWidget::NUMBERS highest (played[0]->number ());
   unsigned int pos (0);
   for (unsigned int i (1); i < played.size (); ++i)
      if ((played[i]->colour () == colour)
          && (played[i]->number () > highest)) {
         pos = i;
         highest = played[i]->number ();
         TRACE9 ("Jabberwocky::check4Winner (unsinged int, unsinged int) - "
                 "New high card at " << i);
      }
      else {
	 if (played[i]->colour () == pTrump->colour ()) {
	    pos = i;
	    highest = played[i]->number ();
	    colour = pTrump->colour ();
	    TRACE9 ("Jabberwocky::check4Winner (unsinged int, unsinged int) - "
                 "Trump wins at " << i);
	 }
      }

   return pos;
}

//-----------------------------------------------------------------------------
/// Find the worst card to play (e.g. card not likely to win the trick)
/// \param pile: Pile from which to play
/// \param aPositions: Array with positions of cards
/// \returns \c Position of card to play or -1
//-----------------------------------------------------------------------------
unsigned int Jabberwocky::findWorstCard (const ICardPile& pile, const int aPositions[4]) const {
   TRACE9 ("Jabberwocky::findWorstCard (const ICardPile&, const int[4])");
   Check1 (pile.size ());

   // Special handling of a pile full of trumps
   if (pile[0]->colour () == pTrump->colour ())
      return 0;

   // Find lowest card (ignoring trumps)
   unsigned int pos (0);
   for (unsigned i (0); i < 4; ++i)
      if ((aPositions[i] != -1) && ((unsigned int)aPositions[i] < (pile.size () - 1))
	  && pile[aPositions[i] + 1]->colour () != pTrump->colour ())
	 if ((pile[aPositions[i] + 1]->number () < pile[pos]->number ())
	     || ((pile[aPositions[i] + 1]->number () == pile[pos]->number ())
		 && (playedCards[pile[aPositions[i] + 1]->colour ()].count ()
		     < playedCards[pile[pos]->colour ()].count ())))
	    pos = aPositions[i] + 1;

   TRACE8 ("Jabberwocky::findWorstCard (const ICardPile&, const int[4]) - " << pos);
   return pos;
}

//----------------------------------------------------------------------------
/// Checks if the passed card might be the highest, considering the unused and
/// played cards.
/// \param card: Card to inspect
/// \return bool: True, if card is likely highest unplayed one
//----------------------------------------------------------------------------
bool Jabberwocky::isHighEnough (const CardWidget& card) const {
   TRACE8 ("Jabberwocky::isHighEnough (const CardWidget&) - " << card);
   return (card.number () >= CardWidget::NUMBERS (CardWidget::TEN + ((getTricks (turn) - 3) >> 1)));
}

//----------------------------------------------------------------------------
/// Checks if the passed card is the highest card of its colour, which has
/// not been played.
/// \param card: Card to inspect
/// \return bool: True, if card is the highest unplayed one
//----------------------------------------------------------------------------
bool Jabberwocky::isHighest (const CardWidget& card) const {
   TRACE8 ("Jabberwocky::isHighest (const CardWidget&) - " << card);

   int nr (card.number ());
   while (++nr <= CardWidget::ACE) {
      if (!playedCards[card.colour ()][nr])
         return false;
   }
   return true;
}
//-----------------------------------------------------------------------------
/// Stores the last position of each colour in the pile
/// \param pile: Pile to inspect
/// \param result: Array of position of last cards of earch colour
//-----------------------------------------------------------------------------
void Jabberwocky::getPositionOfColours (const ICardPile& pile, int result[4]) {
   memset (result, (char)-1, sizeof (int[4]));
   for (unsigned int i (0); i < (pile.size () - 1); ++i)
      if (pile[i]->colour () != pile[i + 1]->colour ())
         result[pile[i]->colour ()] = i;
   result[pile[pile.size () - 1]->colour ()] = pile.size () - 1;

   TRACE9 ("Jabberwocky::getPositionOfColours (const ICardPile&, unsigned int) - Pos. of "
           "cards: " << result[0] << ", " << result[1]
           << ", " << result[2] << ", " << result[3]);
}

//-----------------------------------------------------------------------------
/// Plays a card out of a hand
/// \param player: ID of player
/// \param card: Offset of card to play
/// \returns int: Next player or -1 at end
//-----------------------------------------------------------------------------
int Jabberwocky::playCard (unsigned int player, unsigned int card) {
   TRACE5 ("Jabberwocky::playCard (2x unsigned int) - Player: "
           << player << " at position " << card);
   Check3 (player < NUM_PLAYERS);
   Check3 (card < players[player].hand.size ());

   playedCards[players[player].hand[card]->colour ()].set (players[player].hand[card]->number ());
   movePile (played, players[player].hand, card, card);

   // All players have placed their cards
   if (played.size () == NUM_PLAYERS) {
      // Find the winner
      ICardPile::const_iterator i (played.begin ());
      CardWidget::NUMBERS nr ((*i)->number ());
      CardWidget::COLOURS col ((*i)->colour ());
      unsigned int bestPlayer (0);

      Check3 (pTrump);
      while (++i != played.end ()) {
         TRACE8 ("Jabberwocky::playCard (2x unsigned int) - Comparing " << (*played[player]) << " - " << **i);

         if ((col != pTrump->colour ())
             && ((*i)->colour () == pTrump->colour ())) {
            TRACE9 ("Jabberwocky::playCard (2x unsigned int) - Found trump ");
            col = pTrump->colour ();
            nr = (*i)->number ();
            bestPlayer = i - played.begin ();
            continue;
         }

         if (((*i)->number () > nr) && ((*i)->colour () == col)) {
            TRACE9 ("Jabberwocky::playCard (2x unsigned int) - New best card " << **i);
            nr = (*i)->number ();
            bestPlayer = i - played.begin ();
         }
      }
      player = (player - NUM_PLAYERS + bestPlayer + 1) % NUM_PLAYERS;
      TRACE6 ("Jabberwocky::playCard (2x unsigned int) - Winner: " << player);

      // Move the cards to his won pile
      movePile (players[player].won, played, 0, played.size () - 1);
      if (!player)
	 enableWonCards (players[0].won);
   }
   else
      player = (player + 1) % NUM_PLAYERS;

   // Still cards left: Continue playing
   if (players[player].hand.size ()) {
      displayTurn (player);
      return player;
   }

   Glib::ustring stat (_("Game ended"));
   // Create score-dialog
   if (!pScoreDlg) {
      pScoreDlg = ScoreDlg::create (actPlayers);
      pScoreDlg->get_window ()->set_transient_for (get_window ());
   }
   // Add points, if bet has been met
   int points[NUM_PLAYERS];
   for (unsigned int i (0); i < NUM_PLAYERS; ++i)
      points[i] = (unsigned int)players[i].bet == (players[i].won.size () / NUM_PLAYERS);
   pScoreDlg->addPoints (points);
   pScoreDlg->show ();

   // Stop after 13 rounds
   if (++turn == 13) {
      turn = 0;

      int points;
      pScoreDlg->getMaxPoints (points, player); Check3 (points >= 0);
      Glib::ustring won (_("; %1 won"));
      won.replace (won.find ("%1"), 2, actPlayers[player]->getName ());
      stat += won;
   }

   status.pop ();
   status.push (stat);
   setGameStatus (STOPPED);
   return -1;
}
