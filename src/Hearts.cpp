//PROJECT     : Cardgames
//SUBSYSTEM   : Hearts
//REFERENCES  :
//TODO        :
//BUGS        :
//REVISION    : $Revision$
//AUTHOR      : Markus Schwab
//CREATED     : 24.12.2002
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


#include <cstring>

#include <sstream>

#include <cardgames-cfg.h>

#include <glibmm/main.h>

#include <gtkmm/menu.h>
#include <gtkmm/stock.h>
#include <gtkmm/statusbar.h>
#include <gtkmm/messagedialog.h>

#include <YGP/Check.h>
#include <YGP/Trace.h>
#include <YGP/ConnMgr.h>

#include <card/Images.h>
#include <card/Window.h>
#include <card/ScoreDlg.h>
#include <card/ComputerPlayer.h>

#include "Hearts.h"


const unsigned int Hearts::COLS_PLAYER[NUM_PLAYERS] = {5, 9, 5, 3};
const unsigned int Hearts::ROWS_PLAYER[NUM_PLAYERS] = {10, 7, 3, 7};


unsigned int Hearts::ENDPOINTS(100);


//-----------------------------------------------------------------------------
/// Constructor
/// \param parent Parent widget to display the game in
/// \param statusbar Status bar widget to display information about the game
/// \param cardset Cardset to use
/// \param player Vector of player
/// \param posPlayer Position of player for the server
/// \param mxSerialize Mutex to serialize messages from the server
//-----------------------------------------------------------------------------
Hearts::Hearts(Gtk::Box& parent, Gtk::Statusbar& statusbar, Card::Set& cardset,
               const std::vector<Card::Player*>& player, unsigned int posPlayer,
               YGP::Mutex& mxSerialize)
   : Game(parent, statusbar, cardset, player, posPlayer, mxSerialize, 18, 12),
     playedSQ(false), player2Exchange(3),
     played(Card::IPile::COMPRESSED, Card::IPile::SHOWFACE),
     pScoreDlg(NULL), idMrg(), menuSort(), menuSort2(), menuShowScoreDlg()
 {
   TRACE9("Hearts::Hearts(Box&, Statusbar&, Card::Set&, ...");
   Card::HPile* hand0(new Card::HPile); players[0].hand = hand0;
   Card::HPile* won0(new Card::HPile); players[0].won = won0;
   Card::HPile* hand2(new Card::HPile); players[2].hand = hand2;
   Card::HPile* won2(new Card::HPile); players[2].won = won2;

   Card::VPile* hand1(new Card::VPile); players[1].hand = hand1;
   Card::VPile* won1(new Card::VPile); players[1].won = won1;
   Card::VPile* hand3(new Card::VPile); players[3].hand = hand3;
   Card::VPile* won3(new Card::VPile); players[3].won = won3;

   attach(*won0, COLS_PLAYER[0], COLS_PLAYER[0] + 3, ROWS_PLAYER[0] + 4, ROWS_PLAYER[0] + 5, Gtk::EXPAND);
   attach(*hand0, COLS_PLAYER[0], COLS_PLAYER[0] + 3, ROWS_PLAYER[0], ROWS_PLAYER[0] + 1, Gtk::EXPAND);

   attach(*won1, COLS_PLAYER[1] + 2, COLS_PLAYER[1] + 3, ROWS_PLAYER[1], ROWS_PLAYER[1] + 1, Gtk::EXPAND);
   attach(*hand1, COLS_PLAYER[1], COLS_PLAYER[1] + 1, ROWS_PLAYER[1], ROWS_PLAYER[1] + 1, Gtk::EXPAND);

   attach(*won2, COLS_PLAYER[2], COLS_PLAYER[2] + 3, ROWS_PLAYER[2] - 2, ROWS_PLAYER[2] - 1, Gtk::EXPAND);
   attach(*hand2, COLS_PLAYER[2], COLS_PLAYER[2] + 3, ROWS_PLAYER[2], ROWS_PLAYER[2] + 1, Gtk::EXPAND);

   attach(*won3, COLS_PLAYER[3] - 2, COLS_PLAYER[3] - 1, ROWS_PLAYER[3], ROWS_PLAYER[3] + 1, Gtk::EXPAND);
   attach(*hand3, COLS_PLAYER[3], COLS_PLAYER[3] + 1, ROWS_PLAYER[3], ROWS_PLAYER[3] + 1, Gtk::EXPAND);

   // Show and attach card-piles
   changeNames(player);
   for (unsigned int i(0); i < NUM_PLAYERS; ++i) {
      attach(players[i].name, COLS_PLAYER[i], COLS_PLAYER[i] +((i & 1) ? 1 : 3),
             ROWS_PLAYER[i] + 2, ROWS_PLAYER[i] + 3, Gtk::EXPAND, Gtk::EXPAND, 1);

      players[i].won->setShowOption(Card::IPile::SHOWBACK);
      players[i].hand->setShowOption(i ? Card::IPile::SHOWBACK : Card::IPile::SHOWFACE);

      players[i].hand->setStyle((i & 1) ? Card::IPile::QUITE_COMPRESSED : Card::IPile::COMPRESSED);
      players[i].won->setStyle(Card::IPile::VERY_COMPRESSED);
   }

   // Show played area
   played.setStyle(Card::IPile::COMPRESSED);
   attach(played, 6, 7, 7, 8, Gtk::SHRINK, Gtk::SHRINK, 5);

   resizeCards();
   show_all();
}

//-----------------------------------------------------------------------------
/// Destructor
//-----------------------------------------------------------------------------
Hearts::~Hearts() {
   TRACE9("Hearts::~Hearts()");
   delete pScoreDlg;
   clean();
}


//-----------------------------------------------------------------------------
/// Makes the move for the next player.
/// \param player Actual player
//-----------------------------------------------------------------------------
void Hearts::makeMove(unsigned int player) {
   TRACE5("Hearts::makeMove() - Turn of player " << player);
   Check1(gameStatus() == PLAYING);

   Card::IPile& pile(*players[player].hand);
   unsigned int pos2Play(findPos2Play(player));
   TRACE8("Hearts::makeMove(unsigned int) - Going to play card at pos " << pos2Play);
   Check3(pos2Play < pile.size());

   aPlayed[pile[pos2Play]->colour()]++;
   if ((pile[pos2Play]->colour() == Card::Widget::SPADES)
       && (pile[pos2Play]->number() == Card::Widget::QUEEN))
      playedSQ = true;

   flipCards2Play(pile, pos2Play, pos2Play);
   animateCard(played, pile, pos2Play).sigAnimation.connect(mem_fun(*this, &Hearts::finishMove));
}

//-----------------------------------------------------------------------------
/// Finishes the move; calculates the next player and - if necessary -
/// moves the won cards to the winner.
//-----------------------------------------------------------------------------
void Hearts::finishMove() {
   unsigned int next(calcNextPlayer(currentPlayer()));

   if (played.size() == NUM_PLAYERS)
      Glib::signal_timeout().connect(bind_return(bind(mem_fun(*this, &Hearts::takeWonCards), next), false),
	  Card::ComputerPlayer::TIMEOUT - 50);

   if (players[next].hand->size()) {
      setNextPlayer(next);
      makeNextMoves();
   }
}

//-----------------------------------------------------------------------------
/// Picks up the won cards
/// \param player Player taking won cards
//-----------------------------------------------------------------------------
void Hearts::takeWonCards(unsigned int player) {
   TRACE9("Hearts::takeWonCards(unsigned int) - " << player);
   Check1(player < NUM_PLAYERS);
   if (played.size() == NUM_PLAYERS) {
      players[player].won->getCards(played, 0, NUM_PLAYERS - 1);

      if (!player) {
	 enableWonCards(*players[0].won);
	 menuSort->set_sensitive();
	 menuSort2->set_sensitive();
      }
   }
}

//-----------------------------------------------------------------------------
/// Starts the game by dealing the cards
//-----------------------------------------------------------------------------
void Hearts::start() {
   TRACE9("Hearts::start()");
   Game::start();

   // Hide won pile again (if not in debug-mode)
#if TRACELEVEL > 0
   if (players[1].won->getShowOption() == Card::IPile::SHOWBACK)
#endif
      showWonCards(false);

   Check2(!played.size());
   Card::IPile pile;
   if (randomiseCardsToPile(pile)) {
      for (unsigned int i(0); i < NUM_PLAYERS; ++i) {
	 Card::IPile* actPile(players[(i - posServer) & 0x3].hand);
	 actPile->getCards(pile, 0, cards.size() / NUM_PLAYERS - 1);
	 actPile->sortByColour();
      }

      if (pScoreDlg) {
         unsigned int player;
         int points;
         pScoreDlg->getMaxPoints(points, player);
         if ((unsigned int)points >= ENDPOINTS) {
	    menuShowScoreDlg->set_sensitive(false);
	    delete pScoreDlg;
	    pScoreDlg = NULL;
         }
      }

      if (player2Exchange) {
         Glib::ustring stat(_("Select 3 cards to exchange with %1"));
         Check3(actPlayers.size() > player2Exchange);
         Check3(actPlayers[player2Exchange & 0x3]);
         stat.replace(stat.find ("%1"), 2, actPlayers[player2Exchange]->getName());
         status.pop();
         status.push(stat);
         setGameStatus(EXCHANGE);
         setNextPlayer(0);
         enableHuman();
      }
      else
         startPlaying();
   }
}

//-----------------------------------------------------------------------------
/// Remove cards from everything which can hold them
//-----------------------------------------------------------------------------
void Hearts::clean() {
   TRACE9("Hearts::clean()");
   for (unsigned int i(0); i < NUM_PLAYERS; ++i) {
      players[i].hand->clear();
      players[i].won->clear();
   }

   played.clear();
   Game::clean();

   menuSort->set_sensitive(false);
   menuSort2->set_sensitive(false);
}

//-----------------------------------------------------------------------------
/// Shows or hides the cards of the computer player
/// \param open Flag if cards should be shown or hidden
//-----------------------------------------------------------------------------
void Hearts::playOpen(bool open) {
   for (unsigned int i(1); i < NUM_PLAYERS; ++i) {
      players[i].hand->setShowOption(open ? Card::IPile::SHOWFACE : Card::IPile::SHOWBACK);
      players[i].hand->setStyle(open ? Card::IPile::COMPRESSED : Card::IPile::QUITE_COMPRESSED);
      players[i].won->setShowOption(open ? Card::IPile::SHOWFACE : Card::IPile::SHOWBACK);
      players[i].won->setStyle(open ? Card::IPile::COMPRESSED : Card::IPile::VERY_COMPRESSED);
   }
   players[0].won->setShowOption(open ? Card::IPile::SHOWFACE : Card::IPile::SHOWBACK);
   players[0].won->setStyle(open ? Card::IPile::COMPRESSED : Card::IPile::VERY_COMPRESSED);
}

//-----------------------------------------------------------------------------
/// Enables the cards of the human player
/// \returns Flag, if time should be continued
/// \remarks Depending of the status of the game (PLAYING2) also the top card
///     of the played pile is enabled
//-----------------------------------------------------------------------------
bool Hearts::enableHuman() {
   Check3(activeCards.empty());
   Check3((gameStatus() == PLAYING) || (gameStatus() == EXCHANGE));
   TRACE2("Hearts::enableHuman() - Human has " << players[0].hand->size() << " cards");

   for (int i(players[0].hand->size() - 1); i >= 0; --i)
      activeCards.push_back((*players[0].hand)[i]->signal_clicked() .connect(bind(mem_fun(*this, &Hearts::cardSelected), i)));

   if (gameStatus() == EXCHANGE)
      for (int i(played.size() - 1); i >= 0; --i)
         activeCards.push_back
            (played[i]->signal_clicked().connect(bind(mem_fun(*this, &Hearts::takeCard), i)));

   return Game::enableHuman();
}

//-----------------------------------------------------------------------------
/// Callback after clicking on a card in the played field
/// \param iCard Offset of card in hand
//-----------------------------------------------------------------------------
void Hearts::takeCard(unsigned int iCard) {
   TRACE9("Hearts::takeCard(unsigned int) - Picking up card " << iCard);
   Check1(iCard < played.size());
   Check1(gameStatus() == EXCHANGE);

   played.resize(iCard, Card::IPile::NORMAL);
   animateCard(*players[0].hand, played, iCard).sigAnimation.connect(mem_fun(*this, &Hearts::cardTaken));
}

//-----------------------------------------------------------------------------
/// Action after animation of taken card is finished
//-----------------------------------------------------------------------------
void Hearts::cardTaken() {
   TRACE9("Hearts::cardTaken()");
   players[0].hand->sortByColour();
   makeNextMoves();
}

//-----------------------------------------------------------------------------
/// Callback after clicking on a card in hand
/// \param iCard Offset of card in hand
//-----------------------------------------------------------------------------
void Hearts::cardSelected(unsigned int iCard) {
   TRACE5("Hearts::cardSelected(unsigned int) - Position " << iCard);
   Check1(iCard < players[0].hand->size());
   Check3((gameStatus() == PLAYING) || (gameStatus() == EXCHANGE));

   // Pick up won pile
   if (played.size() == NUM_PLAYERS)
      takeWonCards(0);

   // Hide won pile again (if not in debug-mode)
#if TRACELEVEL > 0
   if (players[1].won->getShowOption() == Card::IPile::SHOWBACK)
#endif
      showWonCards(false);

   if (moveSelectedCardToPlayed(0, iCard)) {
      if (gameStatus() == PLAYING) {
         if (getConnectionMgr().getMode() != YGP::ConnectionMgr::NONE) {
            // Send played card to all clients (if any)
            std::ostringstream msg;
            msg << "Play=" << played[played.size() - 1]->id() << ";Target=0";
            if (getConnectionMgr().getMode() == YGP::ConnectionMgr::CLIENT)
               ignoreNextMsg = true;
            broadcastMessage(msg.str());
         }
      }
      else {
         Check3(gameStatus() == EXCHANGE);
         if (played.size() == 3) {
            // Exchange the cards in pre-play
            TRACE7("Hearts::cardSelected(unsigned int) - Finished exchange");

            if (getConnectionMgr().getMode() == YGP::ConnectionMgr::NONE) {
               exchangeCards();
	       return;
	    }
            else {
               std::ostringstream msg;
               msg << "Exchange=" << played[0]->id() << ' ' << played[1]->id() << ' ' << played[2]->id() << ";Player=" << posServer;
               broadcastMessage(msg.str());

               YGP::ConnectionMgr& cmgr(getConnectionMgr());
               if ((cmgr.getMode() == YGP::ConnectionMgr::SERVER)
                   && cardsExchanged((cmgr.getClients().size() + 1) * 3)) {
                  exchangeCards();
		  return;
	       }
               else {
                  status.pop();
                  status.push(_("Waiting for other player to exchange their cards ..."));
               }
            }
            disableHuman();
            return;
         }
      }
   }
}

//-----------------------------------------------------------------------------
/// Starts the playing phase of the game
//-----------------------------------------------------------------------------
void Hearts::startPlaying() {
   TRACE7("Hearts::startPlaying()");

   // Clear variables for a new game
   memset(aPlayed, 0, sizeof(aPlayed));
   playedSQ = false;
   setGameStatus(PLAYING);

   // Search for startplayer
   unsigned int nextPlayer(0);
   for (unsigned int i(1); i < NUM_PLAYERS; ++i)
      if (((*players[i].hand)[0]->number() == Card::Widget::TWO)
          && ((*players[i].hand)[0]->colour() == Card::Widget::CLUBS)) {
         TRACE7("Hearts::startPlaying() - Start with player " << i);
         nextPlayer = i;
         break;
      }
   Check3(nextPlayer < NUM_PLAYERS);
   player2Exchange = (player2Exchange - 1) & 0x3;

   setNextPlayer(nextPlayer);
   displayTurn(nextPlayer);

   YGP::ConnectionMgr& cmgr(getConnectionMgr());
   if (((cmgr.getMode() == YGP::ConnectionMgr::NONE)
        && nextPlayer)
       || ((cmgr.getMode() == YGP::ConnectionMgr::SERVER)
           && (nextPlayer > getConnectionMgr().getClients().size()))) {
      unsigned int pos2Play(0);
      flipCards2Play(*players[nextPlayer].hand, pos2Play, pos2Play);
      animateCard(played, *players[nextPlayer].hand, pos2Play).sigAnimation.connect(mem_fun(*this, &Hearts::finishMove));
   }
   else
      makeNextMoves();
}

//-----------------------------------------------------------------------------
/// Checks who has played the highest card and would therefore win the played
/// pile
/// \returns \c ID of player with the highest card
//-----------------------------------------------------------------------------
unsigned int Hearts::check4Winner() const {
   Card::Widget::COLOURS colour(played[0]->colour());
   Card::Widget::NUMBERS highest(played[0]->number());
   unsigned int pos(0);
   for (unsigned int i(1); i < played.size(); ++i)
      if ((played[i]->colour() == colour) && (played[i]->number() > highest)) {
         pos = i;
         highest = played[i]->number();
         TRACE9("Hearts::check4Winner(unsinged int, unsinged int) - New high card at " << i);
      }

   return pos;
}

//-----------------------------------------------------------------------------
/// Checks if the round is at end and gives the cards to winner if so
/// \param player ID of player who did the last turn
/// \returns unsigned int Next player
//-----------------------------------------------------------------------------
unsigned int Hearts::calcNextPlayer(unsigned int player) {
   TRACE9("Hearts::calcNextPlayer(unsigned int) - " << player);
   Check1(player < NUM_PLAYERS); Check2(played.size() <= NUM_PLAYERS);

   if (played.size() == NUM_PLAYERS)
      player = (player + check4Winner() - NUM_PLAYERS + 1) & 0x3;
   else
      player = ((player + 1) & 0x3);

   if (!players[player].hand->size()) {
      setGameStatus(STOPPED);
      if (!pScoreDlg) {
         pScoreDlg = Card::ScoreDlg::create(actPlayers);
         pScoreDlg->get_window()->set_transient_for(get_window());
	 menuShowScoreDlg->set_sensitive();
      }

      int aScore[NUM_PLAYERS];
      memset(aScore, 0, sizeof(aScore));
      aScore[player] = pointsOfPile(played);    // Adds points still on table
      for (unsigned int i(0); i < NUM_PLAYERS; ++i) {
         aScore[i] += pointsOfPile(*players[i].won);
         if (aScore[i] == 26) {
            aScore[0] = aScore[1] = aScore[2] = aScore[3] = 26;
            aScore[i] = 0;
            break;
         }
      }

      pScoreDlg->addPoints(aScore);
      pScoreDlg->display();

      Glib::ustring stat(_("Round ended"));
      unsigned int player;
      int points;
      pScoreDlg->getMaxPoints(points, player);
      if ((unsigned int)points >= ENDPOINTS) {
         stat = _("Game ended; %1 won");
         pScoreDlg->getMinPoints(points, player);

         Check3(actPlayers.size() > player);
         Check3(actPlayers[player]);
         stat.replace(stat.find("%1"), 2, actPlayers[player]->getName());
      }

      status.pop();
      status.push(stat);
   }
   else
      displayTurn(player);

   TRACE4("Hearts::calcNextPlayer(unsinged int) - Continuing with player " << player);
   return player;
}

//-----------------------------------------------------------------------------
/// Moves the selected card to the played pile
/// \param player ID of player
/// \param card Offset of card to play
/// \returns bool Status of move; true: Card could be moved; false else
//-----------------------------------------------------------------------------
bool Hearts::moveSelectedCardToPlayed(unsigned int player, unsigned int card) {
   TRACE5("Hearts::moveSelectedCardToPlayed(unsigned int, unsigned int) - Player " << player << "; Pos.  " << card);
   Check1(player < NUM_PLAYERS);
   Check1(card < players[player].hand->size());
   Check1((gameStatus() == PLAYING) || (gameStatus() == EXCHANGE));

   if (gameStatus() == PLAYING) {
      Card::Widget& actCard(*(*players[player].hand)[card]);
      Card::Widget::COLOURS playColour(actCard.colour());
      unsigned int cardsPlayed(0);
      for (unsigned int i(0); i < NUM_PLAYERS; ++i)
         cardsPlayed += players[i].won->size();

      try {
	 if (played.size()) {
	    // The same colour must be played again (if available)
	    Card::Widget::COLOURS colour(played[0]->colour());
	    if ((playColour != colour) && players[player].hand->exists(colour))
	       throw _("You must play a card with an equal colour as the first played one!");
	 }
	 else {
	    // The game must be started with the two of clubs
	    if (!cardsPlayed) {
	       if ((actCard.colour() != Card::Widget::CLUBS) || (actCard.number() != Card::Widget::TWO))
		  throw _("The game must be started with the two of clubs!");
	    }

	    // One can start with a heart only if there has been one played before
	    if (((playColour == Card::Widget::HEARTS) && !aPlayed[Card::Widget::HEARTS])
		&& ((*players[player].hand)[0]->colour() != Card::Widget::HEARTS))
	       throw _("You can't start with a heart, if they have not been played before!");
	 }

	 // The queen of spades can't be played in the first round
	 if (!cardsPlayed) {
	    if ((actCard.colour() == Card::Widget::SPADES) && (actCard.number() == Card::Widget::QUEEN))
	       throw _("The queen of spades can't be played in the first round!");

	    if (((playColour == Card::Widget::HEARTS) && !aPlayed[Card::Widget::HEARTS])
		&& ((*players[player].hand)[0]->colour() != Card::Widget::HEARTS))
	       throw _("Hearts can't be played in the first round!");
	 }
      }
      catch (Glib::ustring& error) {
	 Gtk::MessageDialog dlg(error, Gtk::MESSAGE_ERROR);
	 dlg.set_title(_("Hearts"));
	 dlg.run();
	 return false;
      }

      Check3((unsigned)playColour < (unsigned)(sizeof(aPlayed) / sizeof(aPlayed[0])));
      aPlayed[playColour]++;

      Card::Window& win(animateCard(played, *players[player].hand, card));
      win.sigAnimation.connect(mem_fun(*this, &Hearts::finishMove));
   }
   else {
      players[player].hand->resize(card, Card::IPile::NORMAL);
      // If there are already two cards exchanged (and thus the 3rd is going
      // to be exchanged) start exchanging of cards for the computer players
      Card::Window& win(animateCard(played, *players[player].hand, card));
      win.sigAnimation.connect((played.size () != 2)
                               ? mem_fun (*this, &Hearts::makeNextMoves)
                               : mem_fun (*this, &Hearts::exchangeCards));
   }
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
void Hearts::exchangeCards() {
   TRACE8("Hearts::exchangeCards() - with " << player2Exchange);
   Check1(player2Exchange); Check1(player2Exchange < NUM_PLAYERS);
   Check3(played.size() == 3);

   aExchange[0].getCards(played); Check9(aExchange[0].size() == 3);
   if (getConnectionMgr().getMode() != YGP::ConnectionMgr::CLIENT) {
      for (unsigned int i(getConnectionMgr().getClients().size() + 1);
           i < NUM_PLAYERS; ++i) {
         TRACE8("Hearts::exchangeCards() - Player " << i);

         int posColours[4];
         Card::IPile& source(*players[i].hand);
         getPositionOfColours(source, posColours);

         unsigned int moved(0);
         unsigned int cCards(numberOfCards(posColours, Card::Widget::CLUBS));
         // If nr. of clubs or diamonds are < 3 -> Use them
         if (cCards &&(cCards < 3)) {
            TRACE3("Hearts::exchangeCards() - Getting rid of all clubs: 0 - " << cCards - 1 << "; " << cCards << " cards");
            aExchange[i].getCards(source, 0, posColours[Card::Widget::CLUBS]);
            moved = cCards;
         }
         cCards = numberOfCards(posColours, Card::Widget::DIAMONDS);
         if (cCards &&(cCards) < (3 - moved)) {
            TRACE3("Hearts::exchangeCards() - Getting rid of all diamonds: "
                    << posColours[Card::Widget::DIAMONDS] - moved  - cCards + 1
                    << " - " << posColours[Card::Widget::DIAMONDS] - moved << "; " << cCards << " cards");
            aExchange[i].getCards(source, posColours[Card::Widget::DIAMONDS] - moved - cCards + 1,
                                  posColours[Card::Widget::DIAMONDS] - moved);
            moved += cCards;
         }

         // If nr. of spades < 5 get rid of high spades (especially the queen)
         cCards = numberOfCards(posColours, Card::Widget::SPADES);
         if (cCards && (cCards < 5)) {
            // Search for the queen of spades and get rid of cards equal or
            // bigger
            unsigned int start(posColours[Card::Widget::SPADES] - moved - cCards + 1);
            while (start <= (posColours[Card::Widget::SPADES] - moved - 1)
                   && (source[start]->number() < Card::Widget::QUEEN)) {
                TRACE9("Hearts::exchangeCards() - Checking spades at " << start);
                Check3(source[start]->colour() == Card::Widget::SPADES);
                ++start;
            }

            if (source[start]->number() == Card::Widget::QUEEN) {
               TRACE3("Hearts::exchangeCards() - Getting rid of queen of spades at " << start);
               aExchange[i].getCards(source, start, start);
               moved++;
            }

            if ((moved < 3) && (start < (posColours[Card::Widget::SPADES] - moved))) {
                start = posColours[Card::Widget::SPADES] - moved;
               cCards = 2 - moved;
               TRACE3("Hearts::exchangeCards() - Getting rid of all high spades: "
                      << start -  cCards << " - " << start << "; " <<(cCards + 1) << " cards");
               aExchange[i].getCards(source, start - cCards, start);
               moved += cCards + 1;
            }
         }

         // Get rid of high cards
         int cardPos;
         for (unsigned int nr(Card::Widget::ACE); moved < 3; --nr) {
            Check3(nr > Card::Widget::TWO);
            TRACE8("Hearts::exchangeCards() - Getting rid of high cards - " << nr);
            if ((cardPos = source.find(Card::Widget::NUMBERS(nr))) != -1) {
               TRACE3("Hearts::exchangeCards() - Getting rid of high card at " << cardPos);
               aExchange[i].getCards(source, cardPos, cardPos);
               moved++;
            }
         }

         if (getConnectionMgr().getMode() == YGP::ConnectionMgr::SERVER) {
            std::ostringstream msg;
            msg << "Exchange=" << aExchange[i][0]->id() << ' ' << aExchange[i][1]->id() << ' '
                << aExchange[i][2]->id() << ";Player=" << i << ';';
            broadcastMessage(msg.str());
         }
      }
   }

   Check3(played.empty()); Check3(aExchange[player2Exchange].size() == 3);
   played.getCards(aExchange[NUM_PLAYERS - player2Exchange]);
   Glib::signal_timeout().connect
     (bind_return(mem_fun(*this, &Hearts::finishExchangeCards), false), Card::ComputerPlayer::TIMEOUT);
}

//-----------------------------------------------------------------------------
/// Finishes exchanging the cards and starts the game
//-----------------------------------------------------------------------------
void Hearts::finishExchangeCards() {
   TRACE9("Hearts::finishExchangeCards()");
   aExchange[NUM_PLAYERS - player2Exchange].getCards(played);

   for (unsigned int i(0); i < NUM_PLAYERS; ++i) {
      TRACE9("Hearts::finishExchangeCards() - " << i << " gives to " << ((i + player2Exchange) & 0x3));
      Check3(aExchange[i].size() == 3);
      Card::IPile& target(*players[(i + player2Exchange) & 0x3].hand);
      target.getCards(aExchange[i]);

      target.sortByColour();
      Check3(target.size() == (cards.size() / NUM_PLAYERS));
      Check3(aExchange[i].empty());
   }

   startPlaying();
}

//-----------------------------------------------------------------------------
/// Stores the last position of each colour in the pile
/// \param pile Pile to inspect
/// \param result Array of position of last cards of earch colour
//-----------------------------------------------------------------------------
void Hearts::getPositionOfColours(Card::IPile& pile, int result[4]) {
   memset(result, (char)-1, sizeof(int[4]));
   for (unsigned int i(0); i < (pile.size() - 1); ++i)
      if (pile[i]->colour() != pile[i + 1]->colour())
         result[pile[i]->colour()] = i;
   result[pile[pile.size() - 1]->colour()] = pile.size() - 1;

   TRACE9("Hearts::getPositionOfColours(Card::IPile&, unsigned int) - Pos. of cards: " << result[0] << ", "
          << result[1] << ", " << result[2] << ", " << result[3]);
}

//-----------------------------------------------------------------------------
/// Calculate the number of cards out of the positions
/// \param aPositions Array of positions
/// \param colour Colour whose number should be calculated
/// \returns unsigned int Number of cards for colour
//-----------------------------------------------------------------------------
unsigned int Hearts::numberOfCards(const int aPositions[4], Card::Widget::COLOURS colour) {
   Check1(colour <= Card::Widget::HEARTS);
   unsigned int nr(0), col((unsigned int)colour);
   if (aPositions[colour] != -1) {
      nr = aPositions[colour] + 1;
      while (col)
         if (aPositions[--col] != -1) {
            nr -= aPositions[col] + 1;
            break;
         }
   }

   TRACE9("Hearts::size(int, Card::Widget::COLOURS) - Cards: " << nr);
   return nr;
}

//-----------------------------------------------------------------------------
/// Searches for the card to play
/// \param player Player to inspect
//-----------------------------------------------------------------------------
unsigned int Hearts::findPos2Play(unsigned int player) {
   Check1(player < NUM_PLAYERS);
   TRACE8("Hearts::findPos2Play(unsigned int)");

   Card::IPile& pile(*players[player].hand);
   int aPos[4];
   getPositionOfColours(pile, aPos);

   if (played.size()) {
      // Check if cards of the same colour are available
      return (aPos[played[0]->colour()] == -1) ? findWorstCard(pile, aPos) : findLowerCard(pile, aPos);
   }
   else {
      // Player starts the round: If he has loads of spades: Play them
      unsigned int nrSpades(numberOfCards(aPos, Card::Widget::SPADES));
      unsigned int missingSpades(cards.size() - nrSpades - aPlayed[Card::Widget::SPADES]);
      // If there are still spades left (with other players) and either the player has no high
      // spades or loads of spades: Play them
      if (missingSpades && !playedSQ
          && (((aPos[Card::Widget::SPADES] != -1)
               && (pile[aPos[Card::Widget::SPADES]]->number()
                   < Card::Widget::QUEEN))
              || (((missingSpades / 3) + 1) < nrSpades))) {
         unsigned int pos((aPos[1] >= 0)
                          ? aPos[1] + 1
                          :((aPos[0] >= 0) ? aPos[0] + 1: 0));
         TRACE5("Hearts::findPos2Play(unsigned int) - Starting with spade at " << pos << " (" << *pile[pos] << ')');
         return pos;
      }

      // Else: Search for a low card
      int pos(0);
      for (unsigned int card(Card::Widget::TWO); card <= Card::Widget::ACE;
           ++card) {
         pos = 0;
         while ((pos = pile.find(Card::Widget::NUMBERS(card), pos)) != -1) {
            Card::Widget::COLOURS colour(pile[pos]->colour());
            // Play the lowest card, if there are still cards of that colour
            // owned by other players and - if it is a heart - there are
            // already played hearts.
            TRACE9("Hearts::findPos2Play(unsigned int) - Analyzing " << *pile[pos] << "; Played: "
                   << aPlayed[colour] << "; I have: " << numberOfCards(aPos, colour));
            if ((aPlayed[colour] + numberOfCards(aPos, colour)) < (cards.size() / NUM_PLAYERS)) {
               TRACE8("Hearts::findPos2Play(unsigned int) - Considering to play " << *pile[pos]
                      << "; Played: " << aPlayed[colour]);

                if ((colour != Card::Widget::HEARTS) || aPlayed[Card::Widget::HEARTS]) {
                   TRACE5("Hearts::findPos2Play(unsigned int) - Starting with " << *pile[pos]);
                   return pos;
                }
            }
            ++pos;
         }
      }
      TRACE1("Hearts::findPos2Play(unsigned int) - All cards for player " << player);
   }
   return 0;
}

//-----------------------------------------------------------------------------
/// Find a lower card than the previously played ones
/// \param pile Pile from which to play
/// \param aPositions Array with positions of cards
/// \returns unsigned int Position of card to play
//-----------------------------------------------------------------------------
unsigned int Hearts::findLowerCard(const Card::IPile& pile, const int aPositions[4]) const {
   TRACE9("Hearts::findLowerCard(const Card::IPile&, const int[4]");
   Card::Widget::COLOURS colour(played[0]->colour());
   unsigned int posWinner(check4Winner());

   // Play queen of spades, if there's already a higher card in the pile
   if ((colour == Card::Widget::SPADES) && (played[posWinner]->number() > Card::Widget::QUEEN))
      return ((pile[aPositions[Card::Widget::SPADES]]->number() == Card::Widget::QUEEN)
              || (numberOfCards(aPositions, Card::Widget::SPADES) == 1)
              || (pile[aPositions[Card::Widget::SPADES] - 1]->number()
                  != Card::Widget::QUEEN)
              ? aPositions[Card::Widget::SPADES] : aPositions[Card::Widget::SPADES] - 1);

   // Play high card of the colour, if last player and pile contains no
   // counting card, except if that would mean to play the queen of spades.
   if ((played.size() == (NUM_PLAYERS - 1)) && !pointsOfPile(played))
      return ((colour != Card::Widget::SPADES)
              || (pile[aPositions[Card::Widget::SPADES]]->number() != Card::Widget::QUEEN)
              || numberOfCards (aPositions, Card::Widget::SPADES) == 1)
         ? aPositions[colour] : aPositions[colour] - 1;
   else {
      // Play highest card lower than the previously played ones
      int card(0);
      Check3(played.size());
      Card::Widget::NUMBERS highest(played[posWinner]->number());
      TRACE9("Hearts::findLowerCard(const Card::IPile&, unsigned int[4]) - Try to be below " << *played[posWinner]);

      // Search for a lower card
      unsigned int nrCards(numberOfCards(aPositions, colour));
      card = aPositions[colour] + 1; Check3(card >= 1);
      do {
         Check3(pile[card - 1]->colour() == colour);
         if (pile[--card]->number() < highest) {
            // Found a lower card; test if the highest is the ace of
            // spades and you have the queen and are about to play the king
            if ((colour == Card::Widget::SPADES) && (highest == Card::Widget::ACE) && card
                && (pile[card - 1]->number () == Card::Widget::QUEEN)
                && (pile[card - 1]->colour () == Card::Widget::SPADES))
               --card;

            TRACE5("Hearts::findLowerCard(const Card::IPile&, unsigned int[4]) - Playing card at " << card  << ": " << *pile[card]);
            return card;
         }
      } while (--nrCards);

      // Try to not play the queen of spades, if possible
      if ((colour == Card::Widget::SPADES) && (card < aPositions[Card::Widget::SPADES])
          && (pile[card]->number() == Card::Widget::QUEEN)) {
         Check3 (pile[card + 1]->colour () == Card::Widget::SPADES);
         ++card;
      }

      if (played.size() == (NUM_PLAYERS - 1))
         card = aPositions[colour];
      TRACE5("Hearts::findLowerCard(const Card::IPile&, unsigned int[4]) - Forced to play card at " << card << ": " << *pile[card]);
      return card;
   }
}

//-----------------------------------------------------------------------------
/// Find the worst card to play (defined as having the highest number of bad
/// points (like the queen of spades with 13 points and every heart with 1
/// point) or the highest numbered card).
/// \param pile Pile from which to play
/// \param aPositions Array with positions of cards
/// \returns unsigned int Position of card to play or -1
//-----------------------------------------------------------------------------
unsigned int Hearts::findWorstCard(const Card::IPile& pile, const int aPositions[4]) const {
   TRACE9("Hearts::findWorstCard(const Card::IPile&, const int[4]");

   // Search for queen of spades or any heart or a high card
   unsigned int cardsPlayed(0);
   for (unsigned int i(0); i < NUM_PLAYERS; ++i)
      cardsPlayed += players[i].won->size();

   // If there are already some tricks won
   if (cardsPlayed) {
      TRACE5("Hearts::findWorstCard(const Card::IPile&, const int[4]) - Searching for SQ");
      if (aPositions[2] > 0)
         for (int pos((aPositions[1] >= 0) ? aPositions[1] + 1 : ((aPositions[0] >= 0) ? aPositions[0] + 1: 0));
            pos <= aPositions[2]; ++pos) {
            TRACE9("Hearts::findWorstCard(const Card::IPile&, const int[4]) - Searching for SQ at position " << pos);
            Check3(pile[pos]->colour() == Card::Widget::SPADES);
            if (pile[pos]->number() >= Card::Widget::QUEEN)
               return static_cast<unsigned int>(pos);
         }

      // No high spade found: Try to play a heart
      TRACE5("Hearts::findWorstCard(const Card::IPile&, unsigned int[4]) - Searching for hearts");
      if (aPositions[Card::Widget::HEARTS] != -1)
         return aPositions[Card::Widget::HEARTS];
   }

   // If everything else failes: Play a high card
   TRACE5("Hearts::findWorstCard(const Card::IPile&, unsigned int[4]) - Searching for high cards");
   int pos(0);
   for (int card(Card::Widget::ACE); card >= Card::Widget::TWO;
        --card) {
      pos = 0;
      while ((pos = pile.find(Card::Widget::NUMBERS(card), pos))
             != -1) {
         Card::Widget::COLOURS colour(pile[pos]->colour());
         TRACE2("Hearts::findWorstCard(const Card::IPile&, unsigned int[4]) - Checking card " << *pile[pos]
                << " at pos " << pos << " against " << aPlayed[colour] << " cards");
         if (cardsPlayed
             || ((colour == Card::Widget::SPADES)
                 ? (card != Card::Widget::QUEEN)
                 : (colour != Card::Widget::HEARTS)))
            return pos;
         ++pos;
      }
   }
   Check3(0);
   return -1U;
}

//-----------------------------------------------------------------------------
/// Counts the points in the passed pile. The queen of spades counts 13 points
/// and every heart 1 point
/// \param pile Pile to inspect
/// \returns unsigned int Number of points
//-----------------------------------------------------------------------------
unsigned int Hearts::pointsOfPile(const Card::IPile& pile) {
   unsigned int points(0);
   for (unsigned int i(0); i < pile.size(); ++i) {
      Card::Widget::COLOURS colour(pile[i]->colour());
      if (colour == Card::Widget::HEARTS)
         ++points;
      else
         if ((colour == Card::Widget::SPADES)
             && pile[i]->number() == Card::Widget::QUEEN)
            points += 13;
   }
   TRACE7("Hearts::pointsOfPile(Card::IPile&) - Number of points: " << points);
   return points;
}

//-----------------------------------------------------------------------------
/// Changes the names of the playing people
/// \param newPlayer Array holding the new player
//-----------------------------------------------------------------------------
void Hearts::changeNames(const std::vector<Card::Player*>& newPlayer) {
   Game::changeNames(newPlayer);

   std::vector<Card::Player*> player;
   for (unsigned int i(0); i < NUM_PLAYERS; ++i) {
      player.push_back(actPlayers[(i + posServer) & 0x3]);
      players[i].name.set_text(actPlayers[i]->getName());
   }

   if (pScoreDlg)
      pScoreDlg->update(player);
}

//----------------------------------------------------------------------------
/// Converts a pile-number to the actual pile
/// \param newPlayer Array holding the new player
/// \param pile ID of the pile to return
/// \returns Card::IPile* Pointer to pile to use or NULL
//----------------------------------------------------------------------------
Card::IPile* Hearts::getPileOfPlayer(unsigned int player, unsigned int pile) {
   return ((player >= NUM_PLAYERS) || pile) ? NULL : players[player].hand;
}

//----------------------------------------------------------------------------
/// Handles the messages the server might send for the hearts cardgame
/// \param player ID of player sending the message
/// \param message Message received from the server
/// \returns bool True, if message has been completey processed
//----------------------------------------------------------------------------
bool Hearts::handleMessage(unsigned int player, const std::string& message) {
#if 0
   if (gameStatus() == EXCHANGE) {
      TRACE1("Hearts::handleMessage(unsigned int player, const std::string&) - " << message << " (" << player << ')');

      YGP::Tokenize command(message);
      std::string cmd(command.getNextNode('='));

      if (cmd == "Exchange") {
         std::string cards(command.getNextNode(';'));
         cmd = command.getNextNode('=');
         unsigned long lPlayer(player);
         if ((cmd == "Player")
             && !stringToNumber(lPlayer, command.getNextNode(';').c_str())
             && (lPlayer < NUM_PLAYERS)) {
            Check3(player ? (lPlayer == player) : true);

            register unsigned int save(lPlayer);
            lPlayer = (lPlayer - posServer) & 0x3;

            // Don't exchange already exchanged cards
            if (save != posServer) {
               command = cards;
               unsigned long card(0);
               while (command.getNextNode(' ').size()) {
                  if (stringToNumber(card, command.getActNode().c_str()))
                     break;

                  TRACE9("Hearts::handleMessage(unsigned int, const std::string&) - " << lPlayer << ": " << card);
                  card = players[lPlayer].hand->find(static_cast<unsigned int>(card));
                  Check3(card < players[lPlayer].hand->size());
                  if (card != -1U)
                     aExchange[lPlayer].getCards(*players[lPlayer].hand, card, card);
               }
            }

            TRACE2("Hearts::handleMessage(unsigned int player, const std::string&) - Exchanged: "
                   << aExchange[lPlayer].size() << " cards");
            if (aExchange[lPlayer].size() == 3) {
               YGP::ConnectionMgr& cmgr(getConnectionMgr());
               // Inform other clients
               if (cmgr.getMode() == YGP::ConnectionMgr::SERVER)
                  broadcastMessage(message);

               if (cardsExchanged(((cmgr.getMode() == YGP::ConnectionMgr::SERVER)
                                   ? (cmgr.getClients().size() + 1) : NUM_PLAYERS) * 3))
                  exchangeCards();
            }
            return true;
         }
      }
   }
#endif
   return Game::handleMessage(player, message);
}

//----------------------------------------------------------------------------
/// Checks if the number of exchanged cards is equal to the passed value.
/// \param cards Number of cards to exchange
/// \returns bool True, if all cards have been exchanged
/// \pre Game must be in EXCHANGE state
//----------------------------------------------------------------------------
bool Hearts::cardsExchanged(unsigned int cards) {
   Check1(gameStatus() == EXCHANGE);

   for (unsigned i(0); i < NUM_PLAYERS; ++i)
      cards -= aExchange[i].size();

   TRACE9("Hearts::cardsExchanged(unsigned int) - Remaining: " << cards);
   return !(cards - played.size());
}

//-----------------------------------------------------------------------------
/// Adds game-specific menus
/// \param mgrUI UIManager to add to
//-----------------------------------------------------------------------------
void Hearts::addMenus(Glib::RefPtr<Gtk::UIManager> mgrUI) {
   Check1(mgrUI);
   Glib::ustring ui("<menubar name='Menu'>"
                    "  <placeholder name='GameMenu'>"
                    "    <menu action='MB'>"
                    "      <menuitem action='HeartSort'/>"
                    "      <menuitem action='HeartSortCol'/>"
                    "      <separator/>"
                    "      <menuitem action='showScoreDlg'/>"
                    "    </menu></placeholder></menubar>");

   Glib::RefPtr<Gtk::ActionGroup> grpAction(Gtk::ActionGroup::create());
   grpAction->add(Gtk::Action::create("MB", _("H_earts")));
   grpAction->add(menuSort=Gtk::Action::create("HeartSort", Gtk::Stock::SORT_ASCENDING,
                                               _("_Sort won cards (by number)")),
                  Gtk::AccelKey("<shft>S"), mem_fun(*this, &Hearts::sortWonByNumber));
   grpAction->add(menuSort2=Gtk::Action::create("HeartSortCol", Gtk::Stock::SORT_ASCENDING,
                                                _("Sort won cards (by _colour)")),
                  Gtk::AccelKey("S"), mem_fun(*this, &Hearts::sortWonByColour));
   grpAction->add(menuShowScoreDlg=Gtk::Action::create("showScoreDlg", Gtk::Stock::EDIT,
                                                       _("Show score dialog")),
                  Gtk::AccelKey("<shft><ctl>S"), bind(ptr_fun(&Card::ScoreDlg::display), &pScoreDlg));

   mgrUI->insert_action_group(grpAction);
   idMrg = mgrUI->add_ui_from_string(ui);

   menuShowScoreDlg->set_sensitive(false);
}

//-----------------------------------------------------------------------------
/// Removes the game-specific menus
/// \param mgrUI UIManager to remove from
//-----------------------------------------------------------------------------
void Hearts::removeMenus(Glib::RefPtr<Gtk::UIManager> mgrUI) {
   Check1(mgrUI);
   mgrUI->remove_ui(idMrg);
}

//-----------------------------------------------------------------------------
/// Actions to take when the cards are resized
/// \pre The cardsize must be set in Card::Images::WIDTH/HEIGHT
//-----------------------------------------------------------------------------
void Hearts::resizeCards() {
   for (unsigned int i(0); i < NUM_PLAYERS; ++i) {
      Gtk::Box* hand(dynamic_cast<Gtk::Box*>(players[i].hand)); Check3(hand);
      Gtk::Box* won(dynamic_cast<Gtk::Box*>(players[i].won)); Check3(won);

      if (i & 1) {
	 hand->set_size_request(Card::Images::WIDTH + 5, Card::Images::HEIGHT + 12 * 7);
	 won->set_size_request(Card::Images::WIDTH + 5, Card::Images::HEIGHT + 12 * 7);
      }
      else {
	 hand->set_size_request(Card::Images::WIDTH + 12 * 18, Card::Images::HEIGHT + 5);
	 won->set_size_request(Card::Images::WIDTH + 12 * 7, Card::Images::HEIGHT + 5);
      }
   }
   played.set_size_request(Card::Images::WIDTH + 150, Card::Images::HEIGHT);
}
