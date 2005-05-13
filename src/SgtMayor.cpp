//$Id$

//PROJECT     : Cardgames
//SUBSYSTEM   : Sgt. Mayor
//REFERENCES  :
//TODO        :
//BUGS        :
//REVISION    : $Revision$
//AUTHOR      : Markus Schwab
//CREATED     : 11.4.2004
//COPYRIGHT   : Copyright (C) 2004, 2005

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


#include <iomanip>
#include <sstream>

#include <cardgames-cfg.h>

#include <gtkmm/menu.h>
#include <gtkmm/stock.h>
#include <gtkmm/statusbar.h>
#include <gtkmm/messagedialog.h>

#include <YGP/Check.h>
#include <YGP/Trace.h>
#include <YGP/ConnMgr.h>
#include <YGP/ANumeric.h>
#include <YGP/Tokenize.h>

#include <RemotePlayer.h>
#include <ComputerPlayer.h>

#include "SgtMayor.h"


const unsigned int SgtMayor::COLS_PLAYER[NUM_PLAYERS] = { 1, 4, 0 };
const unsigned int SgtMayor::ROWS_PLAYER[NUM_PLAYERS] = { 1, 6, 6 };


//-----------------------------------------------------------------------------
/// Constructor
/// \param parent: Parent widget to display the game in
/// \param statusbar: Status bar widget to display information about the game
/// \param cardset: Cardset to use
/// \param player: Vector of player
/// \param posPlayer: Position of player for the server
/// \param mxSerialize: Mutex to serialize messages from the server
//-----------------------------------------------------------------------------
SgtMayor::SgtMayor (Gtk::Box& parent, Gtk::Statusbar& statusbar, CardSet& cardset,
                    const std::vector<Player*>& player, unsigned int posPlayer,
                    YGP::Mutex& mxSerialize)
   : Game (parent, statusbar, cardset, player, posPlayer, mxSerialize, 10, 8)
     , played (ICardPile::COMPRESSED, ICardPile::SHOWFACE), pExchange (NULL)
     , pTrump (NULL), startPlayer (rand () % NUM_PLAYERS)
 {
   TRACE9 ("SgtMayor::SgtMayor (Box&, Statusbar&, CardSet&, ...)");

   int width (cards.getCard (0).getImageWidth ());
   int height (cards.getCard (0).getImageHeight ());

   // Show and attach card-piles
   changeNames (player);
   for (unsigned int i (0); i < NUM_PLAYERS; ++i) {
      players[i].name.show ();
      attach (players[i].name, COLS_PLAYER[i], COLS_PLAYER[i] + (i ? 3 : 5),
              ROWS_PLAYER[i] + 1, ROWS_PLAYER[i] + 2, Gtk::EXPAND,
              Gtk::SHRINK, 1, 2);

      players[i].neededTicks.show ();
      attach (players[i].neededTicks, COLS_PLAYER[i],
              COLS_PLAYER[i] + (i ? 3 : 5), ROWS_PLAYER[i] + 2,
              ROWS_PLAYER[i] + 3, Gtk::EXPAND, Gtk::SHRINK, 1, 5);

      players[i].won.show ();
      attach (players[i].won, COLS_PLAYER[i], COLS_PLAYER[i] + (i ? 3 : 5),
              ROWS_PLAYER[i] - 1, ROWS_PLAYER[i], Gtk::EXPAND,
              Gtk::SHRINK, 1, 5);

      players[i].hand.show ();
      attach (players[i].hand, COLS_PLAYER[i], COLS_PLAYER[i] + (i ? 3 : 5),
              ROWS_PLAYER[i], ROWS_PLAYER[i] + 1, Gtk::EXPAND, Gtk::EXPAND, 5);
      TRACE9 ("SgtMayor::SgtMayor () - Attach at: " << COLS_PLAYER[i] << '/'
              << COLS_PLAYER[i] + (i ? 3 : 5) << " - " << ROWS_PLAYER[i] << '/'
              << ROWS_PLAYER[i] + 1);

      players[i].hand.setStyle (i ? ICardPile::QUITE_COMPRESSED : ICardPile::COMPRESSED);
      players[i].hand.setShowOption (i ? ICardPile::SHOWBACK : ICardPile::SHOWFACE);
      players[i].hand.set_size_request (width + 17 * (i ? 7 : 18), height + 5);
      players[i].won.set_size_request (width + 17 * 7, height + 5);
      players[i].won.setStyle (ICardPile::QUITE_COMPRESSED);
      players[i].won.setShowOption (ICardPile::SHOWBACK);
   }

   // Show played area
   played.setStyle (ICardPile::COMPRESSED);
   played.show ();
   attach (played, 2, 5, 4, 5, Gtk::SHRINK, Gtk::SHRINK, 5);
   played.set_size_request (width + 150, height);

   diffTicks[0] = 0x7f;
}

//-----------------------------------------------------------------------------
/// Destructor
//-----------------------------------------------------------------------------
SgtMayor::~SgtMayor () {
   TRACE9 ("SgtMayor::~SgtMayor ()");
   clean ();
   TRACE9 ("SgtMayor::~SgtMayor () - Done");
}


//-----------------------------------------------------------------------------
/// Makes the move for the next player.
/// \param player: Actual player
/// \returns \c int: Next player or -1 if end of game
//-----------------------------------------------------------------------------
int SgtMayor::makeMove (unsigned int player) {
   if ((player + posServer) >= NUM_PLAYERS)
      player -= posServer;
   TRACE5 ("SgtMayor::makeMove () - Turn of player - " << player);
   Check1 (gameStatus () == PLAYING);
   Check3 (pos2Play == pos1Play);

   if (pos2Play == -1U) {
      pos2Play = pos1Play = findPos2Play (player);
      TRACE8 ("SgtMayor::makeMove (unsigned int) - Going to play card at pos " << pos2Play);
      flipCards2Play (players[player].hand, pos1Play, pos2Play);
   }
   else {
      TRACE9 ("SgtMayor::makeMove (unsigned int) - Playing card at pos " << pos2Play);

      player = playCard (player, pos1Play);
      pos1Play = pos2Play = -1U;
   }
   return convertPlayer (player);
}

//-----------------------------------------------------------------------------
/// Starts the game by dealing the cards
//-----------------------------------------------------------------------------
void SgtMayor::start () {
   TRACE9 ("SgtMayor::start ()");
   Game::start ();

   Check2 (!played.size ());
   ICardPile pile;
   if (randomizeCardsToPile (pile)) {
      if (getConnectionMgr ().getMode () != YGP::ConnectionMgr::CLIENT) {
	 setNextPlayer (startPlayer = calcNextPlayer (startPlayer));
	 broadcastStartPlayer (startPlayer);
      }

      Check3 (cards.size () == 52);
      for (unsigned int i (0); i < NUM_PLAYERS; ++i)
         for (unsigned int j (0); j < (cards.size () / NUM_PLAYERS); ++j)
            players[(NUM_PLAYERS + i - posServer) % NUM_PLAYERS].hand.insertColourSorted (pile.removeTopCard ());

      Check3 (!pExchange); Check3 (pile.size ());
      pExchange = &pile.removeTopCard ();
      Check3 (pile.empty ());

      if ((diffTicks[0] > 5) || (diffTicks[1] > 5) || (diffTicks[2] > 5))
         memset (diffTicks, '\0', sizeof (diffTicks));

      for (unsigned int i (0); i < (sizeof (playedCards) / sizeof (playedCards[0])); ++i)
	 playedCards[i].reset ();
      bfColours = 0;

      if (getConnectionMgr ().getMode () != YGP::ConnectionMgr::CLIENT) {
	 showNeededTicks ();
	 makeExchange ();
      }
   }
}

//-----------------------------------------------------------------------------
/// Shows the ticks each player needs
//-----------------------------------------------------------------------------
void SgtMayor::showNeededTicks () {
   TRACE9 ("SgtMayor::showNeededTicks () - Startplayer " << startPlayer);
   // Separate this from dealing the cards, to give the client a chance to
   // receive and perform the ActPlayer-message (to set the start-player)
   for (unsigned int i (0); i < NUM_PLAYERS; ++i) {
      char neededTicks[NUM_PLAYERS] = { '9', '3', '5' };
      Glib::ustring needed (_("(needs %1 ticks)"));
      needed.replace (needed.find ("%1"), 2, 1, neededTicks[i]);
      players[(i + startPlayer) % NUM_PLAYERS].neededTicks.set_text (needed);
   }
}

//-----------------------------------------------------------------------------
/// Remove cards from everything which can hold them
//-----------------------------------------------------------------------------
void SgtMayor::clean () {
   TRACE9 ("SgtMayor::clean ()");
   for (unsigned int i (0); i < NUM_PLAYERS; ++i) {
      players[i].hand.clear ();

      for (ICardPile::iterator c (players[i].won.begin ());
           c != players[i].won.end (); ++c)
         (*c)->show ();
      players[i].won.clear ();
   }

   pExchange = NULL;

   if (pTrump) {
      remove (*pTrump);
      delete pTrump;
      pTrump = NULL;
   }

   played.clear ();
   disableHuman ();
   Game::clean ();
}

//-----------------------------------------------------------------------------
/// Shows or hides the cards of the computer player
/// \param open: Flag if cards should be shown or hidden
//-----------------------------------------------------------------------------
void SgtMayor::playOpen (bool open) {
   for (unsigned int i (1); i < NUM_PLAYERS; ++i) {
      players[i].hand.setShowOption (open ? ICardPile::SHOWFACE : ICardPile::SHOWBACK);
      players[i].hand.setStyle (open ? ICardPile::COMPRESSED : ICardPile::QUITE_COMPRESSED);
   }

   for (unsigned int i (0); i < NUM_PLAYERS; ++i) {
      players[i].won.setShowOption (open ? ICardPile::SHOWFACE : ICardPile::SHOWBACK);
      players[i].won.setStyle (open ? ICardPile::COMPRESSED : ICardPile::QUITE_COMPRESSED);

      for (CardVPile::iterator c (players[i].won.begin ());
           c != players[i].won.end (); ++c)
         if (((c - players[i].won.begin ()) % 3) != 2)
            open ? (*c)->show () : (*c)->hide ();
   }
}

//-----------------------------------------------------------------------------
/// Enables the cards of the human player
/// \returns \c Flag, if time should be continued
/// \remarks Depending of the status of the game (PLAYING2) also the top card
///     of the played pile is enabled
//-----------------------------------------------------------------------------
bool SgtMayor::enableHuman () {
   Check3 (activeCards.empty ());
   TRACE2 ("SgtMayor::enableHuman () - Human has " << players[0].hand.size () << " cards");

   for (int i (players[0].hand.size () - 1); i >= 0; --i)
      activeCards.push_back
         (players[0].hand[i]->signal_clicked ().connect
           (bind (mem_fun (*this, (&SgtMayor::cardSelected)), i)));

   return Game::enableHuman ();
}

//-----------------------------------------------------------------------------
/// Callback after clicking on a card in hand
/// \param iCard: Offset of card in hand
//-----------------------------------------------------------------------------
void SgtMayor::cardSelected (unsigned int iCard) {
   TRACE5 ("SgtMayor::cardSelected (unsigned int) - Position " << iCard);
   Check1 (iCard < players[0].hand.size ());
   Check2 (gameStatus () == PLAYING);
   Check2 (pos1Play == -1U);
   Check2 (pos2Play == -1U);

   CardWidget& selCard (*players[0].hand[iCard]);
   CardWidget::COLOURS playColour (selCard.colour ());

   if (played.size ()) {
      // The same colour must be played again (if available)
      CardWidget::COLOURS colour (played[0]->colour ());
      if (playColour != colour) {
	 if (players[0].hand.exists (colour)) {
	    Gtk::MessageDialog dlg (_("Play a card with an equal colour as "
				      "the first played one!"), Gtk::MESSAGE_ERROR);
	    dlg.set_title (PACKAGE " - SgtMayor");
	    dlg.run ();
	    return;
	 }
	 bfColours |= (1 << played[0]->colour ());
      }
   }

   if (getConnectionMgr ().getMode () != YGP::ConnectionMgr::NONE) {
      // Send played card to all clients (if any)
      std::ostringstream msg;
      msg << "Play=" << selCard.id () << ";Target=0";
      if (getConnectionMgr ().getMode () == YGP::ConnectionMgr::CLIENT)
         ignoreNextMsg = true;
      broadcastMessage (msg.str ());
   }

   unsigned int player (playCard (0, iCard));
   if (player != -1U) {
      setNextPlayer (convertPlayer (player));
      makeNextMoves ();
   }
   else
      disableHuman ();
}

//-----------------------------------------------------------------------------
/// Callback after clicking on a card to select the special colour
/// \param iCard: Offset of card in hand
//-----------------------------------------------------------------------------
void SgtMayor::cardColourSelect (unsigned int iCard) {
   TRACE5 ("SgtMayor::cardColourSelect (unsigned int) - Position " << iCard);
   Check1 (iCard < players[0].hand.size ());
   Check2 (gameStatus () == PLAYING);

   showTrump (players[0].hand[iCard]->colour ());
   disableHuman ();
   makeNextMoves ();
}

//-----------------------------------------------------------------------------
/// Callback after clicking on a card to exchange bad cards with good ones
/// \param iCard: Offset of card in hand
//-----------------------------------------------------------------------------
void SgtMayor::cardExchange (unsigned int iCard) {
   TRACE5 ("SgtMayor::cardExchange (unsigned int) - Position " << iCard);
   Check3 (diffTicks[0]);

   for (unsigned int i (1); i < NUM_PLAYERS; ++i) {
      if (diffTicks[i] < 0) {
         exchangeCards (0, iCard, i);
         break;
      }
   }
   disableHuman ();
}

//-----------------------------------------------------------------------------
/// Exchanges the two of spades
/// \returns bool: False, if a human must exchange the two of spades
//-----------------------------------------------------------------------------
bool SgtMayor::exchangeSpade2 () {
   TRACE9 ("SgtMayor::exchangeSpade2 ()");

   unsigned int exchgPlayer (-1U);
   unsigned int exchgCard (0);

   for (unsigned int i (0); i < NUM_PLAYERS; ++i)
      if (exchgPlayer == -1U)
         for (exchgCard = 0; exchgCard < players[i].hand.size (); ++exchgCard) {
            if ((players[i].hand[exchgCard]->number () == CardWidget::TWO)
                && (players[i].hand[exchgCard]->colour () == CardWidget::SPADES)) {
               exchgPlayer = i;
               TRACE7 ("SgtMayor::exchangeSpade2 () - Player " << i << " exchanges "
                       << *players[exchgPlayer].hand[exchgCard] << " with " << *pExchange);
	       playedCards[CardWidget::SPADES].set (CardWidget::TWO);
               break;
            }
         }

   // Is a card to exchange?
   Glib::ustring stat;
   if (exchgPlayer != -1U) {
      Check3 (pExchange);
      CardWidget& exchg (players[exchgPlayer].hand.remove (exchgCard));
      unsigned int target (players[exchgPlayer].hand.insertColourSorted (*pExchange));
      pExchange = &exchg;
      pExchange->showBack ();

      // Mark the exchanged card, if the human received it
      if (!exchgPlayer) {
         players[0].hand[target]->mark ();
         Glib::signal_timeout ().connect
             (bind (mem_fun (*this, &SgtMayor::unmark), target), 1000);
      }

      stat = _("%1 exchanged the 2 of spades; ");
      stat.replace (stat.find ("%1"), 2, actPlayers[convertPlayer (exchgPlayer)]->getName ());
   }
   else
      stat = _("Nobodoy can exchange the 2 of spades; ");

   if (startPlayer) {
      unsigned int displayPlayer (convertPlayer (startPlayer));
      TRACE9 ("SgtMayor::exchangeSpade2 () - Startplayer: " << startPlayer);
      if (typeid (*actPlayers[displayPlayer]) == typeid (RemotePlayer)) {
	 status.pop ();
	 stat += _("Waiting for %1 to select the special colour ...");
	 stat.replace (stat.find ("%1"), 2,
		       actPlayers[displayPlayer]->getName ());
	 status.push (stat);
      }
      else
	 displayTurn (displayPlayer, stat);
   }
   else {
      status.pop ();
      stat += _("Select the special colour");
      status.push (stat);

      for (int i (players[0].hand.size () - 1); i >= 0; --i)
	 activeCards.push_back
	    (players[0].hand[i]->signal_clicked ().connect
	     (bind (mem_fun (*this, (&SgtMayor::cardColourSelect)), i)));
      return false;
   }
   return true;
}

//-----------------------------------------------------------------------------
/// Starts the playing phase of the game
//-----------------------------------------------------------------------------
void SgtMayor::startPlaying () {
   TRACE7 ("SgtMayor::startPlaying ()");
   Check3 (!diffTicks[0]); Check3 (!diffTicks[1]); Check3 (!diffTicks[2]);

   setNextPlayer (convertPlayer (startPlayer));
   if (exchangeSpade2 ()
       && (typeid (*actPlayers[currentPlayer ()]) == typeid (ComputerPlayer))) {
      // Find special colour
      ICardPile& pile (players[startPlayer].hand);
      int number[] = { 0, 0, 0, 0 };
      int points[] = { 0, 0, 0, 0 };

      for (unsigned int i (0); i < (pile.size () - 1); ++i) {
	 ++number[pile[i]->colour ()];
	 points[pile[i]->colour ()] += pile[i]->number () + 1;
      }

      unsigned int trumpColour (0);
      for (unsigned int i (1); i < 4; ++i) {
	 if ((number[i] > number[i - 1])
	     || ((number[i] == number[i - 1]) && (points[i] > points[i - 1])))
	    trumpColour = i;
      }
      showTrump ((CardWidget::COLOURS)trumpColour);
      makeNextMoves ();
   }
}

//-----------------------------------------------------------------------------
/// Searches for the card to play
/// \param player: Player to inspect
//-----------------------------------------------------------------------------
unsigned int SgtMayor::findPos2Play (unsigned int player) {
   Check1 (player < NUM_PLAYERS);
   TRACE8 ("SgtMayor::findPos2Play (unsigned int) - Player " << player);

   unsigned int pos (0);
   ICardPile& pile (players[player].hand);
   // Get the number of cards of each colour
   unsigned int cColours[] = { 0, 0, 0, 0 };
   unsigned int posColours[] = { -1U, -1U, -1U, -1U };
   for (unsigned int i (0); i < (pile.size () - 1); ++i) {
      ++cColours[pile[i]->colour ()];
      if (pile[i]->colour () != pile[i + 1]->colour ())
	 posColours[pile[i]->colour ()] = i;
   }
   posColours[pile[pile.size () - 1]->colour ()] = pile.size () - 1;
   ++cColours[pile[pile.size () - 1]->colour ()];
   TRACE8 ("SgtMayor::findPos2Play (unsigned int) - Nr: " << cColours[0] << '/'
	   << cColours[1] << '/' << cColours[2] << '/' << cColours[3]);
   TRACE9 ("SgtMayor::findPos2Play (unsigned int) - Pos: " << (int)posColours[0] << '/'
	   << (int)posColours[1] << '/' << (int)posColours[2] << '/' << (int)posColours[3]);
   TRACE9 ("SgtMayor::findPos2Play (unsigned int) - Out: " << std::hex << (int)bfColours << std::dec);

   switch (played.size ()) {
   case 0: {
      // - Play trumps?
      Check3 (pTrump);
      unsigned int trumpsLeft (13 - playedCards[pTrump->colour ()].count ());
      TRACE8 ("SgtMayor::findPos2Play (unsigned int) - Trumps: " << trumpsLeft
	      << '/' << playedCards[pTrump->colour ()].count ());
      Check3 (trumpsLeft <= 13);
      // If others have trumps left, but we have more; if player is not the
      // startplayer, assume, that the 3rd player has no more trumps left
      if ((trumpsLeft >  cColours[pTrump->colour ()])
	  && ((trumpsLeft / ((player == startPlayer) ? 3 : 2))
	      < cColours[pTrump->colour ()])) {
	 Check3 (pile.size () > posColours[pTrump->colour ()]);
	 pos = posColours[pTrump->colour ()];
	 if (!isHighest (*pile[posColours[pTrump->colour ()]]))
	    pos -= cColours[pTrump->colour ()] - 1;
	 break;
      }
      trumpsLeft -= cColours[pTrump->colour ()];

      // - Have dead cards?
      int maxDiff (0);
      CardWidget::COLOURS maxCards (CardWidget::HEARTS);
      for (unsigned int i (0); i < (sizeof (cColours) / sizeof (*cColours)); ++i) {
	 int diff (cColours[i] - (13 - playedCards[i].count ()) / 3);
	 if ((diff > maxDiff) && ((int)i != pTrump->colour ())) {
	    maxDiff = diff;
	    maxCards = (CardWidget::COLOURS)i;
	 }
      }
      TRACE8 ("SgtMayor::findPos2Play (unsigned int) - Dead cards: " << maxDiff
	      << ": " << (int)maxCards);
      if (maxDiff) {
	 Check3 (posColours[maxCards] != -1U);
	 pos = posColours[maxCards];
	 Check3 ((playedCards[maxCards].count () + cColours[maxCards]) <= 13);
	 if (!isHighest (*pile[posColours[maxCards]])
	     || (trumpsLeft
		 && (((playedCards[maxCards].count () + cColours[maxCards]) > 11)
		     || (bfColours & (0x111 << maxCards)))))
	    pos -= cColours[maxCards] - 1;
	 break;
      }

      // Try to find the highest card
      pos = 0;
      while (pos < pile.size ()) {
         pos = pile.findLastEqualColour (pos);
         if (isHighest (*pile[pos])
	     && (pile[pos]->colour () != pTrump->colour ())
	     && !(trumpsLeft && (bfColours & (0x110 << pile[pos]->colour ()))))
	    break;
         ++pos;
      }

      if (pos >= pile.size ())
         pos = pile.findLowestCard (pTrump->colour ());
      break; }

   case 1: {
      Check3 ((playedCards[played[0]->colour ()].count ()
	       + cColours[played[0]->colour ()]) <= 13);
      bool nextHasntColour ((bfColours & ((1 << played[0]->colour ())
					  << (calcNextPlayer (player) << 2)))
			    || ((playedCards[played[0]->colour ()].count ()
				 + cColours[played[0]->colour ()]) > 12));
      pos = posColours[played[0]->colour ()];
      TRACE9 ("SgtMayor::findPos2Play (unsigned int) - Play: " << (int)pos
	      << "; Next: " << nextHasntColour);

      if (pos == -1U) {
	 Check3 ((playedCards[pTrump->colour ()].count () + cColours[pTrump->colour ()]) <= 13);
	 bfColours |= ((1 << played[0]->colour ()) << (player << 2));
	 pos = posColours[pTrump->colour ()];
	 if ((pos == -1U)
	     || (nextHasntColour
		 && !isHighest (*pile[pos])
		 && ((cColours[pTrump->colour ()]
		      + playedCards[pTrump->colour ()].count ()) < 13)))
	    pos = pile.findLowestCard (pTrump->colour ());
	 else
	    pos -= cColours[pTrump->colour ()] - 1;
      }
      else
	 // If the next is know to not have the colour or player has not the
	 // highest left of this colour, play a low one
	 if (nextHasntColour
	     || !isHighest (*pile[pos])
	     || played[0]->number () > pile[pos]->number ())
	    pos -= cColours[played[0]->colour ()] - 1;
      break; }

   case 2:
      // The tick is taken by the second player with a trump. Either play a
      // small card or use a bigger trump.
      if ((played[0]->colour () != pTrump->colour ())
          && (played[1]->colour () == pTrump->colour ())) {
         pos = pile.find (played[0]->colour ());
         if (pos == -1U) {
            pos = pile.find1EqualOrBiggerByColour (*played[1]);
            if (pos == -1U) {
	       bfColours |= ((1 << played[0]->colour ()) << (player << 2));
               pos = pile.findLowestCard (pTrump->colour ());
	    }
         }
      }
      else {
         pos = pile.find1EqualOrBiggerByColour
	    (((played[0]->colour () != played[1]->colour ())
	      || (played[0]->number () > played[1]->number ()))
	     ? *played[0] : *played[1]);
         if ((pos == -1U)
             || (pile[pos]->colour () != played[0]->colour ()))
            pos = (pile.exists (played[0]->colour ())
                   ? pile.find (played[0]->colour ())
                   : (bfColours |= ((1 << played[0]->colour ()) << (player << 2)),
		      tryToGetTickWithTrump (pile)));
      }
      break;

   default:
      Check3 (0);
   }
   Check3 (pos < pile.size ());
   return pos;
}

//-----------------------------------------------------------------------------
/// Changes the names of the playing people
/// \param newPlayer: Array holding the new player
//-----------------------------------------------------------------------------
void SgtMayor::changeNames (const std::vector<Player*>& newPlayer) {
   Game::changeNames (newPlayer);

   for (unsigned int i (0); i < NUM_PLAYERS; ++i)
      players[i].name.set_text (actPlayers[convertPlayer (i)]->getName ());
}

//----------------------------------------------------------------------------
/// Converts a pile-number to the actual pile
/// \param player: Actual player
/// \param pile: ID of the pile to return
/// \returns ICardPile*: Pointer to pile to use or NULL
//----------------------------------------------------------------------------
ICardPile* SgtMayor::getPileOfPlayer (unsigned int player, unsigned int pile) {
   TRACE9 ("SgtMayor::getPileOfPlayer (2x unsigned int) - Player " << player << "; " << pile);
   if ((player + posServer) >= NUM_PLAYERS)
      player -= posServer;
   Check3 (player < NUM_PLAYERS);
   Check3 (!pile);
   return ((player >= NUM_PLAYERS) || pile) ? NULL : &players[player].hand;
}

//-----------------------------------------------------------------------------
/// Reads card- and playernumber from the next tokens
/// \param src: String to analyze
/// \param card: Filled with number of card
/// \param player: Filled with player number
/// \returns bool: True, if parsing was successfull
//-----------------------------------------------------------------------------
bool SgtMayor::readCardInfo (YGP::Tokenize& src, unsigned long& card, unsigned long& player) {
   std::string strCard (src.getNextNode (';'));
   std::string from (src.getNextNode ('='));
   std::string strPlayer (src.getNextNode (';'));
   if ((from == "From")
       && !stringToNumber (card, strCard.c_str ())
       && (card < 17)
       && !stringToNumber (player, strPlayer.c_str ())
       && (player < NUM_PLAYERS))
      return true;
   return false;
}

//----------------------------------------------------------------------------
/// Handles the messages the server might send for the Sgt.Mayor cardgame
/// \param player: ID of player sending the message
/// \param message: Message received from the server
/// \returns bool: True, if message has been completey processed
/// \throw std::string: In case of an error an describing text
//----------------------------------------------------------------------------
bool SgtMayor::handleMessage (unsigned int player, const std::string& message) throw (std::string) {
   TRACE1 ("SgtMayor::handleMessage (unsigned int player, const std::string&) - "
	   << message << " (" << player << ')');
   YGP::Tokenize command (message);
   std::string cmd (command.getNextNode ('='));

   if (cmd == "Exchange") {
      unsigned long card1, card2;
      unsigned long player1, player2;
      if (readCardInfo (command, card1, player1)
	  && (command.getNextNode ('=') == "With")
	  && (readCardInfo (command, card2, player2))) {
	 player1 = (NUM_PLAYERS + player1 - posServer) % NUM_PLAYERS;
	 player2 = (NUM_PLAYERS + player2 - posServer) % NUM_PLAYERS;

	 status.pop ();
	 if (getConnectionMgr ().getMode () == YGP::ConnectionMgr::CLIENT) {
	    doExchangeCards (player2, card2, player1, card1);
	 }
	 else
	    exchangeCards (player2, card2, player1, card1);
	 makeExchange ();
	 return true;
      }
   }
   else if (cmd == "Trump") {
      unsigned long trumpColour;
      if (!stringToNumber (trumpColour, command.getNextNode (';').c_str ())) {
	 if (getConnectionMgr ().getMode () == YGP::ConnectionMgr::CLIENT)
	    doShowTrump ((CardWidget::COLOURS)trumpColour);
	 else
	    showTrump ((CardWidget::COLOURS)trumpColour);
	 makeNextMoves ();
	 return true;
      }
   }
   bool rc (Game::handleMessage (player, message));
   if (cmd == "ActPlayer") {
      startPlayer = currentPlayer ();
      if ((startPlayer + posServer - 1) >= NUM_PLAYERS)
	 startPlayer -= 1;
      TRACE9 ("SgtMayor::handleMessage (unsigned int player, const std::string&) - Start with "
	      << startPlayer);
      showNeededTicks ();
      makeExchange ();
   }
   return rc;
}

//----------------------------------------------------------------------------
/// Shows the special colour on the board (the ace with that colour). Also
/// inform connected player about it
/// \param colour: The special colour to display
//----------------------------------------------------------------------------
void SgtMayor::showTrump (CardWidget::COLOURS colour) {
   TRACE9 ("SgtMayor::showTrump (CardWidget::COLOURS) - " << colour);
   if (getConnectionMgr ().getMode () != YGP::ConnectionMgr::NONE) {
      if (getConnectionMgr ().getMode () == YGP::ConnectionMgr::CLIENT)
	 ignoreNextMsg = true;

      std::ostringstream msg;
      msg << "Trump=" << colour;
      broadcastMessage (msg.str ());
   }

   doShowTrump (colour);
}

//----------------------------------------------------------------------------
/// Shows the special colour on the board (the ace with that colour)
/// \param colour: The special colour to display
//----------------------------------------------------------------------------
void SgtMayor::doShowTrump (CardWidget::COLOURS colour) {
   TRACE9 ("SgtMayor::showTrump (CardWidget::COLOURS) - " << colour);
   for (unsigned int i (0); i < cards.size (); ++i) {
      if ((cards.getCards ()[i]->number () == CardWidget::ACE)
          && (cards.getCards ()[i]->colour () == colour)) {
         Check3 (!pTrump);
         pTrump = new CardWidget (*cards.getCards ()[i]);
         pTrump->show ();
         pTrump->showFace ();
         attach (*pTrump, 0, 1, 0, 1, Gtk::SHRINK, Gtk::SHRINK, 5, 1);
	 displayTurn (convertPlayer (startPlayer));
         return;
      }
   }
   Check3 (0);
}

//----------------------------------------------------------------------------
/// Plays the passed card; find winner and give him the cards at the end of a
/// turn.
/// \param player: Player on turn
/// \param card: Card to play
/// \returns unsigned int: Next player; or -1U, if end of game
//----------------------------------------------------------------------------
unsigned int SgtMayor::playCard (unsigned int player, unsigned int card) {
   TRACE3 ("SgtMayor::playCard (unsigned int, unsigned int) - Player " << player);

   playedCards[players[player].hand[card]->colour ()].set (players[player].hand[card]->number ());
   movePile (played, players[player].hand, card, card);

   if (played.size () == NUM_PLAYERS) {
      // Find the winner
      CardVPile::const_iterator i (played.begin ());
      CardWidget::NUMBERS nr ((*i)->number ());
      CardWidget::COLOURS col ((*i)->colour ());
      unsigned int bestPlayer (0);

      Check3 (pTrump);
      while (++i != played.end ()) {
         TRACE8 ("SgtMayor::playCard (unsigned int, unsigned int) - Comparing "
                 << (**(i - 1)) << " - " << **i);

         if ((col != pTrump->colour ())
             && ((*i)->colour () == pTrump->colour ())) {
            TRACE9 ("SgtMayor::playCard (unsigned int, unsigned int) - Found trump ");
            col = pTrump->colour ();
            nr = (*i)->number ();
            bestPlayer = i - played.begin ();
            continue;
         }

         if (((*i)->number () > nr) && ((*i)->colour () == col)) {
            TRACE9 ("SgtMayor::playCard (unsigned int, unsigned int) - New best card " << **i);
            nr = (*i)->number ();
            bestPlayer = i - played.begin ();
         }
      }
      TRACE9 ("SgtMayor::playCard (unsigned int, unsigned int) - Calc. winner from "
              << player << " and " << bestPlayer);
      player = (player + 1 + bestPlayer) % NUM_PLAYERS;
      TRACE9 ("SgtMayor::playCard (unsigned int, unsigned int) - Winner " << player);

      // Move the cards to his won pile (but show only one of them)
      while (played.size () > 1) {
         CardWidget& card (played.remove (0));
         card.hide ();
         players[player].won.append (card);
      }
      players[player].won.append (played.removeTopCard ());
      Check3 (played.empty ());

      if (!player)
	 enableWonCards (players[0].won);
   }
   else
      player = calcNextPlayer (player);

   if (players[player].hand.size ()) {
      Check3 ((posServer + player) < actPlayers.size ());
      displayTurn (convertPlayer (player));
      return player;
   }

   Glib::ustring stat (_("Game ended; %1 has %2 %7, %3 %4 and %5 %6 %8"));
   unsigned int neededTicks[NUM_PLAYERS] = { 9, 3, 5 };
   player = startPlayer;
   for (unsigned int i (0); i < NUM_PLAYERS; ++i) {
      int madeTicks (players[player].won.size () / 3);
      diffTicks[player] = madeTicks - neededTicks[i];
      TRACE9 ("SgtMayor::playCard (unsigned int, unsigned int) - Player "
              << player << " made " << madeTicks << " = " << (int)diffTicks[player]);

      if (diffTicks[player] > 5) {
	 Glib::ustring won (_("; %1 won"));
	 won.replace (won.find ("%1"), 2, actPlayers[convertPlayer (player)]->getName ());
	 stat += won;
      }
      player = calcNextPlayer (player);
   }

   stat.replace (stat.find ("%1"), 2, actPlayers[0]->getName ());
   stat.replace (stat.find ("%3"), 2, actPlayers[convertPlayer (1)]->getName ());
   stat.replace (stat.find ("%5"), 2, actPlayers[convertPlayer (2)]->getName ());

   stat.replace (stat.find ("%2"), 2, formatNumber (*diffTicks));
   stat.replace (stat.find ("%4"), 2, formatNumber (diffTicks[1]));
   stat.replace (stat.find ("%6"), 2, formatNumber (diffTicks[2]));

   stat.replace (stat.find ("%7"), 2,
		 (ngettext ("tick", "ticks", (*diffTicks < 0) ? -*diffTicks : *diffTicks)));
   stat.replace (stat.find ("%8"), 2,
		 (ngettext ("tick", "ticks", (diffTicks[2] < 0) ? -diffTicks[2] : diffTicks[2])));

   status.pop ();
   status.push (stat);
   setGameStatus (STOPPED);
   return -1U;
}

//-----------------------------------------------------------------------------
/// Formats a number with sign character always shown
/// \param nr: Number to format
/// \returns std::string: Formatted number
/// \remarks Shows the sign always (e.g. also the plus sign (+)
//-----------------------------------------------------------------------------
std::string SgtMayor::formatNumber (int nr) {
   std::ostringstream msg;
   msg << std::showpos << nr;
   return msg.str ();
}

//----------------------------------------------------------------------------
/// Checks if the passed card is the highest card of its colour, which has
/// not been played.
/// \param card: Card to inspect
/// \return bool: True, if card is the highest unplayed one
//----------------------------------------------------------------------------
bool SgtMayor::isHighest (const CardWidget& card) const {
   TRACE8 ("SgtMayor::isHighest (const CardWidget&) - " << card);

   int nr (card.number ());
   while (++nr <= CardWidget::ACE) {
      if (!playedCards[card.colour ()][nr])
         return false;
   }
   return true;
}

//----------------------------------------------------------------------------
/// Tries to get the tick with a trump card; returns a bad card, if there's no
/// trump.
/// \param pile: Pile to play from
/// \return unsigned int: Position of card to play
//----------------------------------------------------------------------------
unsigned int SgtMayor::tryToGetTickWithTrump (const ICardPile& pile) const {
   TRACE8 ("SgtMayor::tryToGetTickWithTrump (const ICardPile&)  - Size " << pile.size ());
   Check3 (pile.size ());

   unsigned int pos (pile.find (pTrump->colour ()));
   if (pos == -1U)
      pos = pile.findLowestCard (pTrump->colour ());
   return pos;
}

//----------------------------------------------------------------------------
/// Exchanges cards between players having too much/too less ticks in the last
/// round.
//----------------------------------------------------------------------------
void SgtMayor::makeExchange () {
   TRACE5 ("SgtMayor::makeExchange () - Exchanging cards: " << (*diffTicks > 0 ? (int)*diffTicks : 0) + (diffTicks[1] > 0 ? (int)diffTicks[1] : 0) + (diffTicks[2] > 0 ? (int)diffTicks[2] : 0));
   Check3 (*diffTicks + diffTicks[1] == -diffTicks[2]);

   for (unsigned int i (startPlayer); (i - startPlayer) < NUM_PLAYERS; ++i) {
      TRACE9 ("SgtMayor::makeExchange () - " << i % NUM_PLAYERS << "'s ticks: " << (int)diffTicks[i % NUM_PLAYERS]);

      while (diffTicks[i % NUM_PLAYERS] > 0) {
	 Check3 ((diffTicks[(i + 1) % NUM_PLAYERS] < 0)
		 || (diffTicks[(i + 2) % NUM_PLAYERS] < 0));

	 for (unsigned int j (1); j < NUM_PLAYERS; ++j) {
	    TRACE9 ("SgtMayor::makeExchange () - With " << (i + j) % NUM_PLAYERS << "'s ticks: " << (int)diffTicks[(i + j) % NUM_PLAYERS]);
	    if (diffTicks[(j + i) % NUM_PLAYERS] < 0) {
	       if (i % NUM_PLAYERS) {
		  if ((getConnectionMgr ().getMode () == YGP::ConnectionMgr::CLIENT)
		      || (typeid (*actPlayers[convertPlayer (i % NUM_PLAYERS)])
			  == typeid (RemotePlayer))) {
		     Glib::ustring msg (_("Waiting for %1 to exchange cards ..."));
		     msg.replace (msg.find ("%1"), 2, actPlayers[convertPlayer (i % NUM_PLAYERS)]->getName ());
		     status.push (msg);
		     return;
		  }
		  else
		     exchangeCards (i % NUM_PLAYERS, (i + j) % NUM_PLAYERS);
	       }
	       else {
		  Check3 (diffTicks[0] > 0);
		  displayExchangeStatus ();

		  for (int i (players[0].hand.size () - 1); i >= 0; --i)
		     activeCards.push_back
			(players[0].hand[i]->signal_clicked ().connect
			 (bind (mem_fun (*this, (&SgtMayor::cardExchange)), i)));
		  return;
	       }
	    }
	 }
      }
   }
   startPlaying ();
}

//-----------------------------------------------------------------------------
/// Displays the number of cards the human can exchange and with whom
//-----------------------------------------------------------------------------
void SgtMayor::displayExchangeStatus () {
   Glib::ustring msg;
   unsigned int exchg (1);

   if ((diffTicks[1] < 0) && (diffTicks[2] < 0)) {
      msg = _("You can exchange %1 %2; %4 with %3 (and than %5 with %6)!");

      msg.replace (msg.find ("%4"), 2, 1, (char)((diffTicks[1] < 0) ? ('0' - diffTicks[1]) : ('0' + diffTicks[1])));
      msg.replace (msg.find ("%5"), 2, 1,  (char)((diffTicks[2] < 0) ? ('0' - diffTicks[2]) : ('0' + diffTicks[2])));
      msg.replace (msg.find ("%6"), 2, actPlayers[convertPlayer (2)]->getName ());
   }
   else {
      msg = _("You can exchange %1 %2 with %3!");
      if (diffTicks[2])
	 exchg = 2;
   }
   msg.replace (msg.find ("%1"), 2, 1, (char)((diffTicks[0] < 0) ? ('0' - diffTicks[0]) : ('0' + diffTicks[0])));
   msg.replace (msg.find ("%2"), 2, (ngettext ("bad card", "bad cards", (diffTicks[0] < 0) ? -diffTicks[0] : diffTicks[0])));
   msg.replace (msg.find ("%3"), 2, actPlayers[convertPlayer (exchg)]->getName ());

   status.pop ();
   status.push (msg);
}

//----------------------------------------------------------------------------
/// Exchanges a good card from playerGood with a bad card from player bad
/// \param playerBad: Player giving away a bad card
/// \param playerGood: Player giving away a good card
//----------------------------------------------------------------------------
void SgtMayor::exchangeCards (unsigned int playerBad, unsigned int playerGood) {
   TRACE7 ("SgtMayor::exchangeCards (unsigned int, unsigned int) - Players "
           << playerBad << " and " << playerGood);
   Check1 (playerBad < NUM_PLAYERS);
   Check1 (playerGood < NUM_PLAYERS);

   unsigned int posBad (players[playerBad].hand.findLowestCard ());
   Check3 (posBad < players[playerBad].hand.size ());
   exchangeCards (playerBad, posBad, playerGood);
}

//-----------------------------------------------------------------------------
/// Exchanges cards directly; e.g. between computer players
/// \param playerBad: Player giving away a bad card
/// \param posBad: Position of bad card to give away
/// \param playerGood: Player giving away a good card
/// \param posGood: Position of good card to give away
//-----------------------------------------------------------------------------
void SgtMayor::directExchange (unsigned int playerBad, unsigned int posBad,
			       unsigned int playerGood, unsigned int posGood) {
   CardWidget& bad (players[playerBad].hand.remove (posBad));
   CardWidget& good (players[playerGood].hand.remove (posGood));
   players[playerBad].hand.insertColourSorted (good);
   players[playerGood].hand.insertColourSorted (bad);
   TRACE9 ("SgtMayor::directExchange (4x unsigned int ) - Player "
           << playerBad << " and " << playerGood << " exchange " << bad
           << " and " << good);
}

//-----------------------------------------------------------------------------
/// Exchanges cards directly; e.g. between computer players
/// \param playerBad: Player giving away a bad card
/// \param posBad: Position of bad card to give away
/// \param playerGood: Player giving away a good card
/// \param posGood: Position of good card to give away
//-----------------------------------------------------------------------------
void SgtMayor::delayedExchange (unsigned int playerBad, unsigned int posBad,
				unsigned int playerGood, unsigned int posGood) {
   Check2 (!(playerBad && playerGood));
   Check2 (posBad < players[playerBad].hand.size ());
   Check2 (posGood < players[playerGood].hand.size ());

   // For easier handling: playerBad is human
   if (playerBad) {
      std::swap (playerGood, playerBad);
      std::swap (posGood, posBad);
   }
   Check3 (playerGood); Check3 (!playerBad);

   CardWidget& bad (*players[0].hand.at (posBad));
   TRACE9 ("SgtMayor::delayedExchange (4x unsigned int ) - Player "
           << playerBad << " and " << playerGood << " exchanges "
	   << bad << " and " << *players[playerGood].hand.at (posGood));
   played.append (players[playerGood].hand.remove (posGood));
   bad.mark ();

   Glib::signal_timeout ().connect
      (bind (mem_fun (*this, &SgtMayor::exchangeMarked), &bad, playerGood), 1000);
}

//----------------------------------------------------------------------------
/// Exchanges a good card from playerGood with a bad card from player bad
/// \param playerBad: Player giving away a bad card
/// \param posBad: Position of bad card to give away
/// \param playerGood: Player giving away a good card
//----------------------------------------------------------------------------
void SgtMayor::exchangeCards (unsigned int playerBad, unsigned int posBad,
                              unsigned int playerGood) {
   TRACE7 ("SgtMayor::exchangeCards (3x unsigned int int) - Players "
           << playerBad << " and " << playerGood);
   Check1 (playerBad < NUM_PLAYERS);
   Check1 (playerGood < NUM_PLAYERS);
   Check1 (posBad < players[playerBad].hand.size ());

   unsigned int posGood (players[playerGood].hand.findLastEqualOrBiggerColour
                         (players[playerBad].hand[posBad]->colour ()));
   if (posGood == -1U)
      posGood = players[playerGood].hand.findLowestCard ();
   TRACE9 ("SgtMayor::exchangeCards (3x unsigned int) - Player "
           << playerBad << ", card " << posBad << " with "
           << playerGood << "'s " << posGood);
   Check3 (posGood < players[playerGood].hand.size ());

   exchangeCards (playerBad, posBad, playerGood, posGood);
}

//----------------------------------------------------------------------------
/// Exchanges a good card from playerGood with a bad card from player bad
/// and informs the connected partners about it
/// \param playerBad: Player giving away a bad card
/// \param posBad: Position of bad card to give away
/// \param playerGood: Player giving away a good card
//----------------------------------------------------------------------------
void SgtMayor::exchangeCards (unsigned int playerBad, unsigned int posBad,
                              unsigned int playerGood, unsigned int posGood) {
   TRACE7 ("SgtMayor::exchangeCards (4x unsigned int int) - Players "
           << playerBad << " and " << playerGood);
   Check1 (playerBad < NUM_PLAYERS);
   Check1 (playerGood < NUM_PLAYERS);
   Check1 (posBad < players[playerBad].hand.size ());
   Check1 (posGood < players[playerGood].hand.size ());

   // Broadcast exchange-info to others
   if (getConnectionMgr ().getMode () != YGP::ConnectionMgr::NONE) {
      std::ostringstream msg;
      msg << "Exchange=" << posGood << ";From=" << (playerGood + posServer) % NUM_PLAYERS
          << ";With=" << posBad << ";From=" << (playerBad + posServer) % NUM_PLAYERS;

      if (getConnectionMgr ().getMode () == YGP::ConnectionMgr::CLIENT)
         ignoreNextMsg = true;
      broadcastMessage (msg.str ());
   }
   doExchangeCards (playerBad, posBad, playerGood, posGood);
}

//----------------------------------------------------------------------------
/// Exchanges a good card from playerGood with a bad card from player bad
/// \param playerBad: Player giving away a bad card
/// \param posBad: Position of bad card to give away
/// \param playerGood: Player giving away a good card
//----------------------------------------------------------------------------
void SgtMayor::doExchangeCards (unsigned int playerBad, unsigned int posBad,
				unsigned int playerGood, unsigned int posGood) {
   TRACE7 ("SgtMayor::doExchangeCards (4x unsigned int int) - Players "
           << playerBad << " and " << playerGood);
   Check1 (playerBad < NUM_PLAYERS);
   Check1 (playerGood < NUM_PLAYERS);
   Check1 (posBad < players[playerBad].hand.size ());
   Check1 (posGood < players[playerGood].hand.size ());

   --diffTicks[playerBad];
   ++diffTicks[playerGood];

   if (playerBad && playerGood)
      directExchange (playerBad, posBad, playerGood, posGood);
   else
      delayedExchange (playerBad, posBad, playerGood, posGood);
}

//----------------------------------------------------------------------------
/// Exchanges the passed card with the first one in the played area (if any)
/// \param card: Pointer to card to exchange
/// \param destPlayer: Player to get the card
/// \return bool: Always false to end the timer
//----------------------------------------------------------------------------
bool SgtMayor::exchangeMarked (CardWidget* card, unsigned int destPlayer) {
   Check1 (card);
   TRACE9 ("SgtMayor::exchangeMarked (CardWidget*, unsigned int) - Moving " << *card
	   << " to player " << destPlayer);
   Check2 ((unsigned int)players[0].hand.findByColour (*card) < players[0].hand.size ());
   Check2 (played.size ()); Check1 ((destPlayer - 1) < (NUM_PLAYERS - 1));

   CardWidget& hand (players[0].hand.remove ((unsigned int)players[0].hand.findByColour (*card)));
   hand.unmark ();

   players[0].hand.insertColourSorted (played.remove (0));
   players[destPlayer].hand.insertColourSorted (hand);

   makeExchange ();
   return false;
}

//----------------------------------------------------------------------------
/// Unmarks the passed card
/// \param card: Position of card to unmark
/// \return bool: Always false to end the timer
//----------------------------------------------------------------------------
bool SgtMayor::unmark (unsigned int card) {
   Check2 (card < players[0].hand.size ());
   TRACE9 ("SgtMayor::unmark (unsigned int) - Unmarking " << card);

   players[0].hand.at (card)->unmark ();
   return false;
}

//-----------------------------------------------------------------------------
/// Adds game-specific menus
/// \param mgrUI: UIManager to add to
//-----------------------------------------------------------------------------
void SgtMayor::addMenus (Glib::RefPtr<Gtk::UIManager> mgrUI) {
   Check1 (mgrUI);
   Glib::ustring ui ("<menubar name='Menu'>"
		     "  <placeholder name='GameMenu'>"
		     "    <menu action='MB'>"
		     "      <menuitem action='Sort'/>"
		     "      <menuitem action='SortCol'/>"
		     "    </menu></placeholder></menubar>");

   Glib::RefPtr<Gtk::ActionGroup> grpAction (Gtk::ActionGroup::create ());
   grpAction->add (Gtk::Action::create ("MB", _("_Sgt. Mayor")));
   grpAction->add (Gtk::Action::create ("Sort", Gtk::Stock::SORT_ASCENDING,
					_("_Sort won cards (by number)")),
		   Gtk::AccelKey ("<ctl><alt>S"),
		   mem_fun (*this, &SgtMayor::sortWonByNumber));
   grpAction->add (Gtk::Action::create ("SortCol", Gtk::Stock::SORT_ASCENDING,
					_("Sort won cards (by _colour)")),
		   Gtk::AccelKey ("<shft><ctl>S"),
		   mem_fun (*this, &SgtMayor::sortWonByColour));

   mgrUI->insert_action_group (grpAction);
   idMrg = mgrUI->add_ui_from_string (ui);
}

//-----------------------------------------------------------------------------
/// Removes the game-specific menus
/// \param mgrUI: UIManager to remove from
//-----------------------------------------------------------------------------
void SgtMayor::removeMenus (Glib::RefPtr<Gtk::UIManager> mgrUI) {
   Check1 (mgrUI);
   mgrUI->remove_ui (idMrg);
}

//-----------------------------------------------------------------------------
/// Shows or hides the won cards
/// \param show: Flag if to show or to hide the cards
//-----------------------------------------------------------------------------
void SgtMayor::showWonCards (bool show, unsigned int style) {
   TRACE9 ("SgtMayor::showWonCards (bool, unsigned int) - " << show << '/' << style);

   for (CardVPile::iterator c (players[0].won.begin ());
	c != players[0].won.end (); ++c)
      if (show)
	 (*c)->show ();
      else
	 if (((c - players[0].won.begin ()) % 3) != 2)
	    (*c)->hide ();

   if (show) {
      Game::showWonCards (true);
   }
   else
      Game::showWonCards (false, ICardPile::QUITE_COMPRESSED);
}
