//$Id$

//PROJECT     : Cardgames
//SUBSYSTEM   : Hearts
//REFERENCES  :
//TODO        :
//BUGS        :
//REVISION    : $Revision$
//AUTHOR      : Markus Schwab
//CREATED     : 24.12.2002
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


#include <sstream>

#include <cardgames-cfg.h>

#include <Check.h>
#include <Trace_.h>
#include <ConnMgr.h>
#include <Tokenize.h>

#include <gtkmm/menu.h>
#include <gtkmm/statusbar.h>
#include <gtkmm/messagedialog.h>

#include <Player.h>
#include <ScoreDlg.h>

#include "Hearts.h"


const unsigned int Hearts::COLS_PLAYER[NUM_PLAYERS] = { 3, 9, 3, 1 };
const unsigned int Hearts::ROWS_PLAYER[NUM_PLAYERS] = { 3, 7, 9, 7 };


//-----------------------------------------------------------------------------
/// Constructor
/// \param parent: Parent widget to display the game in
/// \param statusbar: Status bar widget to display information about the game
/// \param cardset: Cardset to use
/// \param player: Vector of player
/// \param posPlayer: Position of player for the server
/// \param mxSerialize: Mutex to serialize messages from the server
//-----------------------------------------------------------------------------
Hearts::Hearts (Gtk::Box& parent, Gtk::Statusbar& statusbar, CardSet& cardset,
                const std::vector<Player*>& player, unsigned int posPlayer,
                Mutex& mxSerialize)
   : Game (parent, statusbar, cardset, player, posPlayer, mxSerialize, 14, 10)
     , played (ICardPile::COMPRESSED, ICardPile::SHOWFACE)
     , playedSQ (false), pScoreDlg (NULL)
     , player2Exchange (3) {
   TRACE9 ("Hearts::Hearts (Box&, Statusbar&, CardSet&, const std::vector<Glib::ustring>&)");

   int width (cards.getCard (0).getImageWidth ());
   int height (cards.getCard (0).getImageHeight ());

   // Show and attach card-piles
   changeNames (player);
   for (int i (0); i < NUM_PLAYERS; ++i) {
      players[i].name.show ();
      attach (players[i].name, COLS_PLAYER[i], COLS_PLAYER[i] + ((i & 1) ? 1 : 5),
              ROWS_PLAYER[i] + ((i == 2) ? 3 : 1),
              ROWS_PLAYER[i] + ((i == 2) ? 4 : 2),
              Gtk::EXPAND, Gtk::EXPAND, 1);

      players[i].won.show ();
      attach (players[i].won, COLS_PLAYER[i],
              COLS_PLAYER[i] + ((i & 1) ? 1 : 5),
              ROWS_PLAYER[i] + ((i == 2) ? 2 : -2),
              ROWS_PLAYER[i] + ((i == 2) ? 2 : -2) + 1, Gtk::EXPAND);

      TRACE9 ("Hearts::Hearts () - Set at: "
              << COLS_PLAYER[i] << '/' << ROWS_PLAYER[i] + ((i == 2) ? 2 : -2));

      players[i].hand.show ();
      attach (players[i].hand, COLS_PLAYER[i],
              COLS_PLAYER[i] + ((i & 1) ? 1 : 5), ROWS_PLAYER[i],
              ROWS_PLAYER[i] + 1, Gtk::EXPAND);
      TRACE9 ("Hearts::Hearts () - 2nd set at: "
              << COLS_PLAYER[i] << '/' << ROWS_PLAYER[i]);

      players[i].won.setShowOption (ICardPile::SHOWBACK);
      players[i].hand.setShowOption (i ? ICardPile::SHOWBACK : ICardPile::SHOWFACE);

      players[i].won.set_size_request (width + 12 * 7, height + 5);
      players[i].hand.set_size_request (width + 12 * 18, height + 5);

      players[i].hand.setStyle (i ? ICardPile::QUITE_COMPRESSED : ICardPile::COMPRESSED);
      players[i].won.setStyle (ICardPile::VERY_COMPRESSED);
   }

   // Show played area
   played.setStyle (ICardPile::COMPRESSED);
   played.show ();
   attach (played, 3, 4, 5, 8, Gtk::SHRINK, Gtk::SHRINK, 5);
   played.set_size_request (width + 150, height);
}

//-----------------------------------------------------------------------------
/// Destructor
//-----------------------------------------------------------------------------
Hearts::~Hearts () {
   TRACE9 ("Hearts::~Hearts ()");
   delete pScoreDlg;
}


//-----------------------------------------------------------------------------
/// Makes the move for the next player.
/// \param player: Actual player
/// \returns \c int: Next player or -1 if end of game
//-----------------------------------------------------------------------------
int Hearts::makeMove (unsigned int player) {
   TRACE5 ("Hearts::makeMove () - Turn of player " << player);
   Check1 (gameStatus () == PLAYING);
   Check3 (pos2Play == pos1Play);

   if (pos2Play == -1U) {
      pos2Play = pos1Play = findPos2Play (player);
      TRACE8 ("Hearts::makeMove (unsigned int) - Going to play card at pos " << pos2Play);
      flipCards2Play (players[player].hand, pos1Play, pos2Play);
   }
   else {
      TRACE9 ("Hearts::makeMove (unsigned int) - Playing card at pos " << pos2Play);
      ICardPile& pile (players[player].hand);
      Check3 (pos2Play < pile.size ());
      aPlayed[pile[pos2Play]->colour ()]++;
      if ((pile[pos2Play]->colour () == CardWidget::SPADES)
          && (pile[pos2Play]->number () == CardWidget::QUEEN))
          playedSQ = true;

      movePile (played, pile, pos1Play, pos2Play);
      pos1Play = pos2Play = -1U;
      player = calcNextPlayer (player);
   }
   return player;
}

//-----------------------------------------------------------------------------
/// Starts the game by dealing the cards
//-----------------------------------------------------------------------------
void Hearts::start () {
   TRACE9 ("Hearts::start ()");
   Game::start ();

   // Hide won pile again (if not in debug-mode)
#if TRACELEVEL > 0
   if (players[1].won.getShowOption () == ICardPile::SHOWBACK)
#endif
      showWonCards (false);

   Check2 (!played.size ());
   ICardPile pile;
   if (randomizeCardsToPile (pile)) {
      for (unsigned int i (0); i < NUM_PLAYERS; ++i)
         for (unsigned int j (0); j < (cards.size () / NUM_PLAYERS); ++j)
            players[(i - posServer) & 0x3].hand.insertColourSorted (pile.removeTopCard ());

      if (pScoreDlg) {
         unsigned int player;
         int points;
         pScoreDlg->getMaxPoints (points, player);
         if (points >= 100) {
             delete pScoreDlg;
             pScoreDlg = NULL;
         }
      }

      if (player2Exchange) {
         Glib::ustring stat (_("Select 3 cards to exchange with %1"));
         Check3 (actPlayers.size () > player2Exchange);
         Check3 (actPlayers[player2Exchange & 0x3]);
         stat.replace (stat.find ("%1"), 2,
                       actPlayers[player2Exchange]->getName ());
         status.pop ();
         status.push (stat);
         setGameStatus (EXCHANGE);
         setNextPlayer (0);
         enableHuman ();
      }
      else
         startPlaying ();
   }
}

//-----------------------------------------------------------------------------
/// Remove cards from everything which can hold them
//-----------------------------------------------------------------------------
void Hearts::clean () {
   TRACE9 ("Hearts::clean ()");
   for (unsigned int i (0); i < NUM_PLAYERS; ++i) {
      players[i].hand.clear ();
      players[i].won.clear ();
   }

   played.clear ();
   disableHuman ();
   Game::clean ();
}

//-----------------------------------------------------------------------------
/// Shows or hides the cards of the computer player
/// \param open: Flag if cards should be shown or hidden
//-----------------------------------------------------------------------------
void Hearts::playOpen (bool open) {
   for (int i (1); i < NUM_PLAYERS; ++i) {
      players[i].hand.setShowOption (open ? ICardPile::SHOWFACE : ICardPile::SHOWBACK);
      players[i].hand.setStyle (open ? ICardPile::COMPRESSED : ICardPile::QUITE_COMPRESSED);
      players[i].won.setShowOption (open ? ICardPile::SHOWFACE : ICardPile::SHOWBACK);
      players[i].won.setStyle (open ? ICardPile::COMPRESSED : ICardPile::VERY_COMPRESSED);
   }
   players[0].won.setShowOption (open ? ICardPile::SHOWFACE : ICardPile::SHOWBACK);
   players[0].won.setStyle (open ? ICardPile::COMPRESSED : ICardPile::VERY_COMPRESSED);
}

//-----------------------------------------------------------------------------
/// Enables the cards of the human player
/// \returns \c Flag, if time should be continued
/// \remarks Depending of the status of the game (PLAYING2) also the top card
///     of the played pile is enabled
//-----------------------------------------------------------------------------
bool Hearts::enableHuman () {
   Check3 (activeCards.empty ());
   Check3 ((gameStatus () == PLAYING) || (gameStatus () == EXCHANGE));

   TRACE2 ("Hearts::enableHuman () - Human has " << players[0].hand.size () << " cards");

   for (int i (players[0].hand.size ()); i;)
      activeCards.push_back
         (players[0].hand[--i]->signal_clicked ().connect
           (bind (slot (*this, (&Hearts::cardSelected)), i)));

   if (gameStatus () == EXCHANGE)
      for (int i (played.size ()); i;)
         activeCards.push_back
            (played[--i]->signal_clicked ().connect
             (bind (slot (*this, (&Hearts::takeCard)), i)));

   return Game::enableHuman ();
}

//-----------------------------------------------------------------------------
/// Callback after clicking on a card in the played field
/// \param iCard: Offset of card in hand
//-----------------------------------------------------------------------------
void Hearts::takeCard (unsigned int iCard) {
   TRACE9 ("Hearts::takeCard (unsigned int) - Picking up card " << iCard);
   Check1 (iCard < played.size ());
   Check1 (gameStatus () == EXCHANGE);

   movePile (players[0].hand, played, iCard, iCard);
   players[0].hand.sortByColour ();
   makeNextMoves ();
}

//-----------------------------------------------------------------------------
/// Callback after clicking on a card in hand
/// \param iCard: Offset of card in hand
//-----------------------------------------------------------------------------
void Hearts::cardSelected (unsigned int iCard) {
   TRACE5 ("Hearts::cardSelected (unsigned int) - Position " << iCard);
   Check1 (iCard < players[0].hand.size ());
   Check3 ((gameStatus () == PLAYING) || (gameStatus () == EXCHANGE));
   Check3 (pos1Play == -1U);
   Check3 (pos2Play == -1U);

   // Hide won pile again (if not in debug-mode)
#if TRACELEVEL > 0
   if (players[1].won.getShowOption () == ICardPile::SHOWBACK)
#endif
      showWonCards (false);

   if (moveSelectedCardToPlayed (0, iCard)) {
      if (gameStatus () == PLAYING) {
         if (getConnectionMgr ().getMode () != ConnectionMgr::NONE) {
            // Send played card to all clients (if any)
            std::ostringstream msg;
            msg << "Play=" << played[played.size () - 1]->id () << ";Target=0";
            if (getConnectionMgr ().getMode () == ConnectionMgr::CLIENT)
               ignoreNextMsg = true;
            broadcastMessage (msg.str ());
         }

         setNextPlayer (calcNextPlayer (currentPlayer ()));
      }
      else {
         Check3 (gameStatus () == EXCHANGE);
         if (played.size () == 3) {
            // Exchange the cards in pre-play
            TRACE7 ("Hearts::cardSelected (unsigned int) - Finished exchange");

            if (getConnectionMgr ().getMode () == ConnectionMgr::NONE) {
               exchangeCards ();
               startPlaying ();
            }
            else {
               std::ostringstream msg;
               msg << "Exchange=" << played[0]->id () << ' ' << played[1]->id ()
                   << ' ' << played[2]->id () << ";Player=" << posServer;
               broadcastMessage (msg.str ());

               ConnectionMgr& cmgr (getConnectionMgr ());
               if ((cmgr.getMode () == ConnectionMgr::SERVER)
                   && cardsExchanged ((cmgr.getClients ().size () + 1) * 3)) {
                  exchangeCards ();
                  startPlaying ();
               }
               else {
                  status.pop ();
                  status.push (_("Waiting for other player to exchange their cards ..."));
               }
            }
            disableHuman ();
            return;
         }
      }
      makeNextMoves ();
   }
}

//-----------------------------------------------------------------------------
/// Starts the playing phase of the game
//-----------------------------------------------------------------------------
void Hearts::startPlaying () {
   TRACE7 ("Hearts::startPlaying ()");

   // Clear variables for a new game
   memset (aPlayed, 0, sizeof (aPlayed));
   playedSQ = false;
   setGameStatus (PLAYING);

   // Search for startplayer
   unsigned int nextPlayer (0);
   for (unsigned int i (1); i < NUM_PLAYERS; ++i)
       if ((players[i].hand[0]->number () == CardWidget::TWO)
           && (players[i].hand[0]->colour () == CardWidget::CLUBS)) {
           TRACE7 ("Hearts::startPlaying () - Start with player " << i);
           nextPlayer = i;
           break;
       }
   Check3 (nextPlayer < NUM_PLAYERS);

   setNextPlayer (nextPlayer);
   ConnectionMgr& cmgr (getConnectionMgr ());
   if (((cmgr.getMode () == ConnectionMgr::NONE)
        && nextPlayer)
       || ((cmgr.getMode () == ConnectionMgr::SERVER)
           && (nextPlayer > getConnectionMgr ().getClients ().size ())))
      flipCards2Play (players[nextPlayer].hand, pos1Play = 0, pos2Play = 0);

   player2Exchange = (player2Exchange - 1) & 0x3;

   displayTurn (currentPlayer ());
   makeNextMoves ();
}


//-----------------------------------------------------------------------------
/// Checks who has played the highest card and would therefore win the played
/// pile
/// \returns \c ID of player with the highest card
//-----------------------------------------------------------------------------
unsigned int  Hearts::check4Winner () {
   CardWidget::COLOURS colour (played[0]->colour ());
   CardWidget::NUMBERS highest (CardWidget::TWO);
   unsigned int pos (0);
   for (unsigned int i (0); i < played.size (); ++i)
      if ((played[i]->colour () == colour)
          && (played[i]->number () > highest)) {
         pos = i;
         highest = played[i]->number ();
         TRACE9 ("Hearts::check4Winner (unsinged int, unsinged int) - "
                 "New high card at " << i);
      }

   return pos;
}

//-----------------------------------------------------------------------------
/// Checks if the round is at end and gives the cards to winner if so
/// \param player: ID of player who did the last turn
/// \returns \c Next player
//-----------------------------------------------------------------------------
unsigned int Hearts::calcNextPlayer (unsigned int player) {
   if (played.size () == NUM_PLAYERS) {
      // Everyone played its card: Search for winner of played pile;
      // clear it and continue with winner
      player = (player - NUM_PLAYERS + check4Winner () + 1) & 0x3;
      movePile (players[player].won, played);
   }
   else
      player = ((player + 1) & 0x3);

   if (!players[player].hand.size ()) {
      player = -1U;
      setGameStatus (STOPPED);
      if (!pScoreDlg) {
         // Resort player for score dialogue
         std::vector<Player*> player;
         for (unsigned int i (0); i < NUM_PLAYERS; ++i)
            player.push_back (actPlayers[i]);
         
         pScoreDlg = ScoreDlg::create (player);
         pScoreDlg->get_window ()->set_transient_for (get_window ());
      }

      int aScore[NUM_PLAYERS];
      for (unsigned int i (0); i < NUM_PLAYERS; ++i) {
         aScore[i] = pointsOfPile (players[i].won);
         if (aScore[i] == 26) {
            aScore[0] = aScore[1] = aScore[2] = aScore[3] = 26;
            aScore[i] = 0;
            break;
         }
      }

      pScoreDlg->addPoints (aScore);
      pScoreDlg->show ();

      Glib::ustring stat (_("Round ended"));
      unsigned int player;
      int points;
      pScoreDlg->getMaxPoints (points, player);
      if (points >= 100) {
         stat = _("Game ended; %1 won");
         pScoreDlg->getMinPoints (points, player);

         Check3 (actPlayers.size () > player);
         Check3 (actPlayers[player]);
         stat.replace (stat.find ("%1"), 2, actPlayers[player]->getName ());
      }

      status.pop ();
      status.push (stat);
   }
   else
      displayTurn (player);

   if (!player)
      enableWonCards (players[0].won);

   TRACE4 ("Hearts::calcNextPlayer (unsinged int) - Continuing with player "
           << player);
   return player;
}

//-----------------------------------------------------------------------------
/// Moves the selected card to the played pile
/// \param player: ID of player
/// \param card: Offset of card to play
/// \returns \c Status of moving; true: Card could be moved; false else
//-----------------------------------------------------------------------------
bool Hearts::moveSelectedCardToPlayed (unsigned int player, unsigned int card) {
   TRACE5 ("Hearts::moveSelectedCardToPlayed (unsigned int, unsigned int) - Player "
           << player << "; Pos.  " << card);
   Check1 (player < NUM_PLAYERS);
   Check1 (card < players[player].hand.size ());
   Check1 ((gameStatus () == PLAYING) || (gameStatus () == EXCHANGE));

   if (gameStatus () == PLAYING) {
      CardWidget& card (*players[player].hand[card]);
      CardWidget::COLOURS playColour (card.colour ());
      unsigned int cardsPlayed (0);
      for (unsigned int i (0); i < NUM_PLAYERS; ++i)
         cardsPlayed += players[i].won.size ();

      if (played.size ()) {
         // The same colour must be played again (if available)
         CardWidget::COLOURS colour (played[0]->colour ());
         if ((playColour != colour) && players[player].hand.exists (colour)) {
            Gtk::MessageDialog dlg (_("Play first cards with an equal colour as "
                                      "the first played one!"), Gtk::MESSAGE_ERROR);
            dlg.set_title (PACKAGE " - Hearts");
            dlg.run ();
            return false;
         }
      }
      else {
         // The game must be started with the two of clubs
         if (!cardsPlayed) {
            if ((card.colour () != CardWidget::CLUBS)
                || (card.number () != CardWidget::TWO)) {
               Gtk::MessageDialog dlg (_("The game must be started with the two of clubs!"),
                                         Gtk::MESSAGE_ERROR);
               dlg.set_title (PACKAGE " - Hearts");
               dlg.run ();
               return false;
            }
         }

         // One can start with a heart only if there has been one played before
         if (((playColour == CardWidget::HEARTS) && !aPlayed[CardWidget::HEARTS])
             && (players[player].hand[0]->colour () != CardWidget::HEARTS)) {
            Gtk::MessageDialog dlg (_("You can't start with a heart, if they have"
                                      " not been played before!"),
                                    Gtk::MESSAGE_ERROR);
            dlg.set_title (PACKAGE " - Hearts");
            dlg.run ();
            return false;
         }
      }

      // The queen of spades can't be played in the first round
      if (!cardsPlayed) {
         if ((card.colour () == CardWidget::SPADES)
             && (card.number () == CardWidget::QUEEN)) {
            Gtk::MessageDialog dlg (_("The queen of spades can't be played in the first"
                                      " round!"), Gtk::MESSAGE_ERROR);
            dlg.set_title (PACKAGE " - Hearts");
            dlg.run ();
            return false;
         }

         if (((playColour == CardWidget::HEARTS) && !aPlayed[CardWidget::HEARTS])
              && (players[player].hand[0]->colour () != CardWidget::HEARTS)) {
               Gtk::MessageDialog dlg (_("Hearts can't be played in the first round!"),
                                       Gtk::MESSAGE_ERROR);
               dlg.set_title (PACKAGE " - Hearts");
               dlg.run ();
            return false;
         }
      }

      Check3 (playColour < (sizeof (aPlayed) / sizeof (aPlayed[0])));
      aPlayed[playColour]++;
   }

   movePile (played, players[player].hand, card, card);
   return true;
}

//-----------------------------------------------------------------------------
/// Exchanges the cards of the computer players. Get rid of cards according the
/// following algorithm:
///    - If nr. of clubs or diamonds are < 3 -> Use them
///    - If nr. of spades < 5 get rid of high spades (especially the queen)
///    - Get rid of high hearts
///    - Get rid of high cards
//-----------------------------------------------------------------------------
void Hearts::exchangeCards () {
   TRACE8 ("Hearts::exchangeCards () - with " << player2Exchange);
   Check3 (played.size () == 3);

   movePile (aExchange[0], played); Check9 (aExchange[0].size () == 3);

   if (getConnectionMgr ().getMode () != ConnectionMgr::CLIENT) {
      for (unsigned int i (getConnectionMgr ().getClients ().size () + 1);
           i < NUM_PLAYERS; ++i) {
         TRACE8 ("Hearts::exchangeCards () - Player " << i);

         int posColours[4];
         ICardPile& source (players[i].hand);
         getPositionOfColours (source, posColours);

         unsigned int moved (0);
         unsigned int cCards (numberOfCards (posColours, CardWidget::CLUBS));
         // If nr. of clubs or diamonds are < 3 -> Use them
         if (cCards && (cCards < 3)) {
            TRACE3 ("Hearts::exchangeCards () - Getting rid of all clubs: 0 - "
                    << cCards - 1 << "; " << cCards << " cards");
            movePile (aExchange[i], source, 0, posColours[CardWidget::CLUBS]);
            moved = cCards;
         }
         cCards = numberOfCards (posColours, CardWidget::DIAMONDS);
         if (cCards && (cCards) < (3 - moved)) {
            TRACE3 ("Hearts::exchangeCards () - Getting rid of all diamonds: "
                    << posColours[CardWidget::DIAMONDS] - moved  - cCards + 1
                    << " - " << posColours[CardWidget::DIAMONDS] - moved << "; "
                    << cCards << " cards");
            movePile (aExchange[i], source,
                      posColours[CardWidget::DIAMONDS] - moved - cCards + 1,
                      posColours[CardWidget::DIAMONDS] - moved);
            moved += cCards;
         }

         // If nr. of spades < 5 get rid of high spades (especially the queen)
         cCards = numberOfCards (posColours, CardWidget::SPADES);
         if (cCards && (cCards < 5)) {
            // Search for the queen of spades and get rid of cards equal or
            // bigger
            unsigned int start (posColours[CardWidget::SPADES] - moved - cCards + 1);
            while (start <= (posColours[CardWidget::SPADES] - moved - 1)
                   && (source[start]->number () < CardWidget::QUEEN)) {
                TRACE9 ("Hearts::exchangeCards () - Checking spades at " << start);
                Check3 (source[start]->colour () == CardWidget::SPADES);
                ++start;
            }

            if (source[start]->number () == CardWidget::QUEEN) {
               TRACE3 ("Hearts::exchangeCards () - Getting rid of queen of spades at "
                       << start);
               movePile (aExchange[i], source, start, start);
               moved++;
            }

            if ((moved < 3)
                && (start < (posColours[CardWidget::SPADES] - moved))) {
                start = posColours[CardWidget::SPADES] - moved;
               cCards = 2 - moved;
               TRACE3 ("Hearts::exchangeCards () - Getting rid of all high spades: "
                       << start -  cCards << " - " << start << "; " << (cCards + 1)
                       << " cards");
               movePile (aExchange[i], source, start - cCards, start);
               moved += cCards + 1;
            }
         }

         // Get rid of high cards
         int cardPos;
         for (unsigned int nr (CardWidget::ACE); moved < 3; --nr) {
            Check3 (nr > CardWidget::TWO);
            TRACE8 ("Hearts::exchangeCards () - Getting rid of high cards");
            if ((cardPos = source.find (CardWidget::NUMBERS (nr))) != -1) {
               TRACE3 ("Hearts::exchangeCards () - Getting rid of high card at "
                       << cardPos);
               movePile (aExchange[i], source, cardPos, cardPos);
               moved++;
            }
         }

         if (getConnectionMgr ().getMode () == ConnectionMgr::SERVER) {
            std::ostringstream msg;
            msg << "Exchange=" << aExchange[i][0]->id () << ' '
                << aExchange[i][1]->id () << ' ' << aExchange[i][2]->id ()
                << ";Player=" << i << ';';
            broadcastMessage (msg.str ());
         }
      }
   }

   for (unsigned int i (0); i < NUM_PLAYERS; ++i) {
      TRACE9 ("Hearts::exchangeCards () - " << i << " gives to "
              << ((i + player2Exchange) & 0x3));
      Check3 (aExchange[i].size () == 3);
      ICardPile& target (players[(i + player2Exchange) & 0x3].hand);
      movePile (target, aExchange[i]);

      target.sortByColour ();
      Check3 (target.size () == (cards.size () / NUM_PLAYERS));
      Check3 (aExchange[i].empty ());
   }
}

//-----------------------------------------------------------------------------
/// Retrieve the last position of each colour in the pile
/// \param pile: Pile to inspect
/// \param result: Array of position of last cards of earch colour
//-----------------------------------------------------------------------------
void Hearts::getPositionOfColours (ICardPile& pile, int result[4]) {
   memset (result, (char)-1, sizeof (int[4]));
   for (unsigned int i (0); i < (pile.size () - 1); ++i)
      if (pile[i]->colour () != pile[i + 1]->colour ())
         result[pile[i]->colour ()] = i;
   result[pile[pile.size () - 1]->colour ()] = pile.size () - 1;

   TRACE9 ("Hearts::getPositionOfColours (ICardPile&, unsigned int) - Pos. of "
           "cards: " << result[0] << ", " << result[1]
           << ", " << result[2] << ", " << result[3]);
}

//-----------------------------------------------------------------------------
/// Calculate the number of cards out of the positions
/// \param aPositions: Array of positions
/// \param colour: Colour whose number should be calculated
/// \returns \c int: Number of cards for colour
//-----------------------------------------------------------------------------
unsigned int Hearts::numberOfCards (const int aPositions[4], CardWidget::COLOURS colour) {
   Check1 (colour <= CardWidget::HEARTS);
   unsigned int nr (0);
   if (aPositions[colour] != -1) {
      nr = aPositions[colour] + 1;
      while (colour)
         if (aPositions[--(unsigned int)colour] != -1) {
            nr -= aPositions[colour] + 1;
            break;
         }
   }

   TRACE9 ("Hearts::size (int, CardWidget::COLOURS) - Cards: " << nr);
   return nr;
}

//-----------------------------------------------------------------------------
/// Calculate the number of cards for each colour
/// \param pile: Pile to inspect
/// \param result: Array of number of cards for earch colour
//-----------------------------------------------------------------------------
unsigned int Hearts::findPos2Play (unsigned int player) {
   Check1 (player < NUM_PLAYERS);
   TRACE8 ("Hearts::findPos2Play (unsigned int)");

   ICardPile& pile (players[player].hand);
   int aPos[4];
   getPositionOfColours (pile, aPos);

   if (played.size ()) {
      // Check if cards of the same colour are available
      return (aPos[played[0]->colour ()] == -1) 
         ? findWorstCard (pile, aPos) : findLowerCard (pile, aPos);
   }
   else {
      // Player starts the round: If he has loads of spades: Play them
      unsigned int nrSpades (numberOfCards (aPos, CardWidget::SPADES));
      unsigned int missingSpades (cards.size () - nrSpades
                                  - aPlayed[CardWidget::SPADES]);
      // If there are still spades left (with other players) and either the
      // player has no high spades or loads of spades: Play them
      if (missingSpades && (!playedSQ)
          && (((aPos[CardWidget::SPADES] != -1)
               && (pile[aPos[CardWidget::SPADES]]->number ()
                   < CardWidget::QUEEN))
              || (((missingSpades / 3) + 1) < nrSpades))) {
         unsigned int pos ((aPos[1] >= 0)
                           ? aPos[1] + 1
                           : ((aPos[0] >= 0) ? aPos[0] + 1: 0));
         TRACE5 ("Hearts::findPos2Play (unsigned int) - Starting with spade at "
                 << pos << " (" << *pile[pos] << ')');
         return pos;
      }

      // Else: Search for a low card
      int pos (0);
      for (unsigned int card (CardWidget::TWO); card <= CardWidget::ACE;
           ++card) {
         pos = 0;
         while ((pos = pile.find (CardWidget::NUMBERS (card), pos)) != -1) {
            CardWidget::COLOURS colour (pile[pos]->colour ());
            // Play the lowest card, if there are still cards of that colour
            // owned by other players and - if it is a heart - there are
            // already played hearts.
            TRACE9 ("Hearts::findPos2Play (unsigned int) - Analyzing "
                    << *pile[pos] << "; Played: " << aPlayed[colour]
                    << "; I have: " << numberOfCards (aPos, colour));
            if ((aPlayed[colour] + numberOfCards (aPos, colour))
                < (cards.size () / NUM_PLAYERS)) {
               TRACE8 ("Hearts::findPos2Play (unsigned int) - Considering to "
                       "play " << *pile[pos] << "; Played: " << aPlayed[colour]);

                if ((colour != CardWidget::HEARTS)
                    || aPlayed[CardWidget::HEARTS]) {
                   TRACE5 ("Hearts::findPos2Play (unsigned int) - Starting with "
                           << *pile[pos]);
                   return pos;
                }
            }
            ++pos;
         }
      }
      TRACE1 ("Hearts::findPos2Play (unsigned int) - All cards for player " << player);
   }
   return 0;
}

//-----------------------------------------------------------------------------
/// Find a lower card than the previously played ones
/// \param pile: Pile from which to play
/// \param aPositions: Array with positions of cards
/// \returns \c Position of card to play
//-----------------------------------------------------------------------------
unsigned int Hearts::findLowerCard (const ICardPile& pile, const int aPositions[4]) {
   TRACE9 ("Hearts::findLowerCard (const ICardPile&, const int[4]");
   CardWidget::COLOURS colour (played[0]->colour ());
   unsigned int posWinner (check4Winner ());

   // Play queen of spades, if there's already a higher card in the pile
   if ((colour == CardWidget::SPADES)
       && (played[posWinner]->number () > CardWidget::QUEEN))
      return ((pile[aPositions[CardWidget::SPADES]]->number () == CardWidget::QUEEN)
              || (numberOfCards (aPositions, CardWidget::SPADES) == 1)
              || (pile[aPositions[CardWidget::SPADES] - 1]->number ()
                  != CardWidget::QUEEN)
              ? aPositions[CardWidget::SPADES] : aPositions[CardWidget::SPADES] - 1);

   // Play high card of the colour, if last player and pile contains no
   // counting card, except if that would mean to play the queen of spades.
   if ((played.size () == (NUM_PLAYERS - 1))
       && !pointsOfPile (played))
      return ((colour != CardWidget::SPADES)
              || (pile[aPositions[CardWidget::SPADES]]->number ()
                  != CardWidget::QUEEN)
              || numberOfCards (aPositions, CardWidget::SPADES) == 1)
         ? aPositions[colour] : aPositions[colour] - 1;
   else {
      // Play highest card lower than the previously played ones
      int card (0);
      Check3 (played.size ());
      CardWidget::NUMBERS highest (played[posWinner]->number ());
      TRACE9 ("Hearts::findLowerCard (const ICardPile&, unsigned int[4]) - Try to"
              " be below " << *played[posWinner]);

      // Search for a lower card
      unsigned int nrCards (numberOfCards (aPositions, colour));
      card = aPositions[colour] + 1; Check3 (card >= 1);
      do {
         Check3 (pile[card - 1]->colour () == colour);
         if (pile[--card]->number () < highest) {
            // Found a lower card; test if the highest is the ace of
            // spades and you have the queen and are about to play the king
            if ((colour == CardWidget::SPADES)
                && (highest == CardWidget::ACE)
                && card
                && (pile[card - 1]->number () == CardWidget::QUEEN)
                && (pile[card - 1]->colour () == CardWidget::SPADES))
               --card;

            TRACE5 ("Hearts::findLowerCard (const ICardPile&, unsigned int[4]) - "
                    "Playing card at " << card  << ": " << *pile[card]);
            return card;
         }
      } while (--nrCards);

      // Try to not play the queen of spades, if possible
      if ((colour == CardWidget::SPADES)
          && (card < aPositions[CardWidget::SPADES])
          && (pile[card]->number () == CardWidget::QUEEN)) {
         Check3 (pile[card + 1]->colour () == CardWidget::SPADES);
         ++card;
      }

      if (played.size () == (NUM_PLAYERS - 1))
         card = aPositions[colour];
      TRACE5 ("Hearts::findLowerCard (const ICardPile&, unsigned int[4]) - "
              "Forced to play card at " << card << ": " << *pile[card]);
      return card;
   }
}

//-----------------------------------------------------------------------------
/// Find the worst card to play (defined as having the highest number of bad
/// points (like the queen of spades with 13 points and every heart with 1
/// point) or the highest numbered card).
/// \param pile: Pile from which to play
/// \param aPositions: Array with positions of cards
/// \returns \c Position of card to play
//-----------------------------------------------------------------------------
unsigned int Hearts::findWorstCard (const ICardPile& pile, const int aPositions[4]) {
   TRACE9 ("Hearts::findWorstCard (const ICardPile&, const int[4]");

   // Search for queen of spades or any heart or a high card
   unsigned int cardsPlayed (0);
   for (unsigned int i (0); i < NUM_PLAYERS; ++i)
      cardsPlayed += players[i].won.size ();

   if (cardsPlayed) {
      TRACE5 ("Hearts::findWorstCard (const ICardPile&, const int[4]) - Searching"
              " for SQ");
      if (aPositions[2] > 0)
         for (unsigned int pos ((aPositions[1] >= 0)
                                ? aPositions[1] + 1
                                : ((aPositions[0] >= 0) ? aPositions[0] + 1: 0));
              pos <= aPositions[2]; ++pos) {
            TRACE9 ("Hearts::findWorstCard (const ICardPile&, const int[4]) - "
                    "Searching for SQ at position " << pos);
            Check3 (pile[pos]->colour () == CardWidget::SPADES);
            if (pile[pos]->number () >= CardWidget::QUEEN)
               return pos;
         }

      // No high spade found: Try to play a heart (after the first round)
      TRACE5 ("Hearts::findWorstCard (const ICardPile&, unsigned int[4]) - "
              "Searching for hearts");
      if (aPositions[CardWidget::HEARTS] != -1)
         return aPositions[CardWidget::HEARTS];
   }

   // If everything else failes: Play a high card
   TRACE5 ("Hearts::findWorstCard (const ICardPile&, unsigned int[4]) - Searching "
           "for high cards");
   int pos (0);
   for (int card (CardWidget::ACE); card >= CardWidget::TWO;
        --card) {
      pos = 0;
      while ((pos = pile.find (CardWidget::NUMBERS (card), pos))
             != -1) {
         CardWidget::COLOURS colour (pile[pos]->colour ());
         TRACE2 ("Hearts::findWorstCard (const ICardPile&, unsigned int[4]) - "
                 "Checking card " << *pile[pos] << " at pos " << pos
                 << " against " << aPlayed[colour] << " cards");
         if (cardsPlayed
             || ((colour == CardWidget::SPADES)
                 ? (card != CardWidget::QUEEN)
                 : (colour != CardWidget::HEARTS)))
            return pos;
         ++pos;
      }
   }
   Check3 (0);
}

//-----------------------------------------------------------------------------
/// Counts the points in the passed pile. The queen of spades counts 13 points
/// and every heart 1 point
/// \param pile: Pile to inspect
/// \returns \c unsigned int: Number of points
//-----------------------------------------------------------------------------
unsigned int Hearts::pointsOfPile (ICardPile& pile) {
   unsigned int points (0);
   for (unsigned int i (0); i < pile.size (); ++i) {
      CardWidget::COLOURS colour (pile[i]->colour ());
      if (colour == CardWidget::HEARTS)
         ++points;
      else
         if ((colour == CardWidget::SPADES)
             && pile[i]->number () == CardWidget::QUEEN)
            points += 13;
   }
   TRACE7 ("Hearts::pointsOfPile (ICardPile&) - Number of points: " << points);
   return points;
}

//-----------------------------------------------------------------------------
/// Changes the names of the playing people
/// \param newPlayer: Array holding the new player
//-----------------------------------------------------------------------------
void Hearts::changeNames (const std::vector<Player*>& newPlayer) {
   Game::changeNames (newPlayer);

   std::vector<Player*> player;
   for (int i (0); i < NUM_PLAYERS; ++i) {
      player.push_back (actPlayers[(i + posServer) & 0x3]);
      players[i].name.set_text (actPlayers[i]->getName ());
   }

   if (pScoreDlg)
      pScoreDlg->update (player);
}

//----------------------------------------------------------------------------
/// Changes the names of the playing people
/// \param newPlayer: Array holding the new player
/// \param pile: ID of the pile to return
//----------------------------------------------------------------------------
ICardPile& Hearts::getPileOfPlayer (unsigned int player, unsigned int pile) {
   Check1 (player < NUM_PLAYERS);
   Check1 (!pile);
   return players[player].hand;
}

//----------------------------------------------------------------------------
/// Handles the messages the server might send for the hearts cardgame
/// \param player: ID of player sending the message
/// \param message: Message received from the server
/// \returns bool: True, if message has completey processed
//----------------------------------------------------------------------------
bool Hearts::handleMessage (unsigned int player, const char* message) {
   if (gameStatus () == EXCHANGE) {
      TRACE1 ("Hearts::handleMessage (unsigned int player, const char*) - "
              << message << " (" << player << ')');

      Tokenize command (message);
      std::string cmd (command.getNextNode ('='));

      if (cmd == "Exchange") {
         std::string cards (command.getNextNode (';'));
         cmd = command.getNextNode ('=');
         unsigned long lPlayer (player);
         if ((cmd == "Player")
             && !stringToNumber (lPlayer, command.getNextNode (';').c_str ())
             && (lPlayer < NUM_PLAYERS)) {
            Check3 (player ? (lPlayer == player) : true);
            if (!player)
               lPlayer = static_cast<unsigned long> (lPlayer);

            register unsigned int save (lPlayer);
            lPlayer = (lPlayer - posServer) & 0x3;
            
            // Don't exchange already exchanged cards
            if (save != posServer) {
               command = cards;
               unsigned long card (0);
               while (command.getNextNode (' ').size ()) {
                  if (stringToNumber (card, command.getActNode ().c_str ()))
                     break;

                  TRACE9 ("Hearts::handleMessage (unsigned int, const char*) - "
                          << lPlayer << ": " << card);
                  card = players[lPlayer].hand.find (static_cast<unsigned int> (card));
                  Check3 (card < players[lPlayer].hand.size ());
                  if (card != -1U)
                     movePile (aExchange[lPlayer], players[lPlayer].hand,
                               card, card);
               }
            }

            TRACE2 ("Hearts::handleMessage (unsigned int player, const char*) - "
                    "Exchanged: " << aExchange[lPlayer].size () << " cards");
            if (aExchange[lPlayer].size () == 3) {
               ConnectionMgr& cmgr (getConnectionMgr ());
               // Inform other clients
               if (cmgr.getMode () == ConnectionMgr::SERVER)
                  broadcastMessage (message);

               if (cardsExchanged (((cmgr.getMode () == ConnectionMgr::SERVER)
                                    ? (cmgr.getClients ().size () + 1)
                                    : NUM_PLAYERS) * 3)) {
                  exchangeCards ();
                  startPlaying ();
               }
            }
            return true;
         }
      }
   }
   return Game::handleMessage (player, message);
}

//----------------------------------------------------------------------------
/// Checks if the number of exchanged cards is equal to the passed value.
/// \param 
/// \returns 
/// \pre Game must be in EXCHANGE state 
//----------------------------------------------------------------------------
bool Hearts::cardsExchanged (unsigned int cards) {
   Check1 (gameStatus () == EXCHANGE);

   for (unsigned i (0); i < NUM_PLAYERS; ++i)
      cards -= aExchange[i].size ();

   TRACE9 ("Hearts::cardsExchanged (unsigned int) - Remaining: " << cards);
   return !(cards - played.size ());
}
