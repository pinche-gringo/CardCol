// PROJECT     : Cardgames
// SUBSYSTEM   : Sgt. Mayor
// REFERENCES  :
// TODO        :
// BUGS        :
// AUTHOR      : Markus Schwab
// CREATED     : 11.4.2004
// COPYRIGHT   : Copyright (C) 2004 - 2009, 2026

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

#include <array>
#include <iomanip>
#include <sstream>

#include <cardgames-cfg.h>

#include <gtkmm/messagedialog.h>
#include <gtkmm/statusbar.h>

#include <giomm/menu.h>
#include <giomm/simpleactiongroup.h>

#include <XGP/XDialog.h>

#include <YGP/ANumeric.h>
#include <YGP/Check.h>
#include <YGP/ConnMgr.h>
#include <YGP/Trace.h>

#include <card/ComputerPlayer.h>
#include <card/Images.h>
#include <card/Random.h>
#include <card/RemotePlayer.h>
#include <card/ScoreDlg.h>
#include <card/Tokenize.h>
#include <card/Window.h>

#include "SgtMayor.h"

unsigned int SgtMayor::ENDTRICKS(10);

//-----------------------------------------------------------------------------
/// Constructor
/// \param parent Parent widget to display the game in
/// \param statusbar Status bar widget to display information about the game
/// \param cardset Cardset to use
/// \param player Vector of player
/// \param posPlayer Position of player for the server
/// \param mxSerialize Mutex to serialize messages from the server
//-----------------------------------------------------------------------------
SgtMayor::SgtMayor(Gtk::Box& parent, Gtk::Statusbar& statusbar, Card::Set& cardset, const std::vector<Card::Player*>& player,
                   unsigned int posPlayer, YGP::Mutex& mxSerialize)
    : Game(parent, statusbar, cardset, player, posPlayer, mxSerialize, 10, 8),
      played(Card::IPile::COMPRESSED, Card::IPile::SHOWFACE), pTrump(nullptr), bfColours(0),
      startPlayer(Card::randomNumber(NUM_PLAYERS)), menuSort(), menuSort2(), menuShowScoreDlg(), pScoreDlg(nullptr) {
    TRACE9("SgtMayor::SgtMayor(Box&, Statusbar&, Card::Set&, ...)");

    // Show and attach card-piles
    changeNames(player);
    for (unsigned int i(0); i < NUM_PLAYERS; ++i) {
        players[i].name.show();
        players[i].name.set_margin_start(1);
        players[i].name.set_margin_end(1);
        players[i].name.set_margin_top(2);
        players[i].name.set_margin_bottom(2);
        attach(players[i].name, COLS_PLAYER[i], ROWS_PLAYER[i] + (i ? 1 : 2), (i ? 3 : 5), 1);

        players[i].neededTricks.set_margin_top(5);
        players[i].neededTricks.set_margin_bottom(5);
        attach(players[i].neededTricks, COLS_PLAYER[i], ROWS_PLAYER[i] + (i ? 2 : 3), (i ? 3 : 5), 1);

        players[i].won.set_margin_start(1);
        players[i].won.set_margin_end(1);
        players[i].won.set_margin_top(5);
        players[i].won.set_margin_bottom(5);
        attach(players[i].won, COLS_PLAYER[i], ROWS_PLAYER[i] + (i ? -1 : +1), (i ? 3 : 5), 1);

        players[i].hand.set_margin_start(5);
        players[i].hand.set_margin_end(5);
        attach(players[i].hand, COLS_PLAYER[i], ROWS_PLAYER[i], (i ? 3 : 5), 1);
        TRACE9("SgtMayor::SgtMayor() - Attach at: " << COLS_PLAYER[i] << '/' << COLS_PLAYER[i] + (i ? 3 : 5) << " - "
                                                    << ROWS_PLAYER[i] << '/' << ROWS_PLAYER[i] + 1);

        players[i].hand.setStyle(i ? Card::IPile::QUITE_COMPRESSED : Card::IPile::COMPRESSED);
        players[i].hand.setShowOption(i ? Card::IPile::SHOWBACK : Card::IPile::SHOWFACE);
        players[i].won.setStyle(Card::IPile::QUITE_COMPRESSED);
        players[i].won.setShowOption(Card::IPile::SHOWBACK);
    }

    // Show played area
    played.setStyle(Card::IPile::COMPRESSED);
    played.set_margin_start(5);
    played.set_margin_end(5);
    attach(played, 2, 4, 3, 1);

    resizeCards();

    diffTricks.fill(0);
}

//-----------------------------------------------------------------------------
/// Destructor
//-----------------------------------------------------------------------------
SgtMayor::~SgtMayor() {
    TRACE9("SgtMayor::~SgtMayor()");
    pScoreDlg.reset();
    clean();
}

//-----------------------------------------------------------------------------
/// Makes the move for the next player.
/// \param player Actual player
//-----------------------------------------------------------------------------
void SgtMayor::makeMove(unsigned int player) {
    if ((player + posServer) >= NUM_PLAYERS)
        player -= posServer;
    TRACE5("SgtMayor::makeMove() - Turn of player - " << player);
    Check1(gameStatus() == PLAYING);

    pos1Play = findPos2Play(player);
    Card::HPile& pile(players[player].hand);
    Card::Widget& card(*pile[pos1Play]);

    // Remember card as being played
    playedCards[card.colour()].set(card.number());

    TRACE8("SgtMayor::makeMove(unsigned int) - Going to play card at pos " << pos1Play);
    flipCards2Play(pile, pos1Play, pos1Play);
    animateCard(played, pile, pos1Play).sigAnimation.connect(bind(mem_fun(*this, &SgtMayor::playCardDelayed), player));
}

//-----------------------------------------------------------------------------
/// Delays playing the next card a while
//-----------------------------------------------------------------------------
void SgtMayor::playCardDelayed(unsigned int player) {
    player = playCard(player);
    if (player != -1U) {
        setNextPlayer(convertPlayer(player));
        makeNextMoves();
    }
}

//-----------------------------------------------------------------------------
/// Starts the game by dealing the cards
//-----------------------------------------------------------------------------
void SgtMayor::start() {
    TRACE9("SgtMayor::start()");
    Game::start();

    Check2(!played.size());
    Card::IPile pile;
    if (randomiseCardsToPile(pile)) {
        // Delete the score-dialog if the game has ended
        if (pScoreDlg) {
            unsigned int player;
            int points;
            pScoreDlg->getMaxPoints(points, player);
            Check3(points >= 0);
            if (static_cast<unsigned int>(points) >= ENDTRICKS) {
                menuShowScoreDlg->set_enabled(false);
                pScoreDlg.reset();

                diffTricks.fill(0);
            }
        }

        if (getConnectionMgr().getMode() != YGP::ConnectionMgr::CLIENT) {
            setNextPlayer(startPlayer = calcNextPlayer(startPlayer));
            broadcastStartPlayer(startPlayer);
        }

        Check3(cards.size() == 52);
        for (unsigned int i(0); i < NUM_PLAYERS; ++i)
            for (unsigned int j(0); j < (cards.size() / NUM_PLAYERS);)
                if ((pile.getTopCard().number() != Card::Widget::TWO) || (pile.getTopCard().colour() != Card::Widget::CLUBS)) {
                    players[(NUM_PLAYERS + i - posServer) % NUM_PLAYERS].hand.insertColourSorted(pile.removeTopCard());
                    ++j;
                }
                else
                    pile.removeTopCard();
        Check3(pile.empty());

        for (auto& playedCard : playedCards)
            playedCard.reset();
        bfColours = 0;

        if (getConnectionMgr().getMode() != YGP::ConnectionMgr::CLIENT) {
            showNeededTricks();
            makeExchange();
        }
    }
}

//-----------------------------------------------------------------------------
/// Shows the tricks each player needs
//-----------------------------------------------------------------------------
void SgtMayor::showNeededTricks() {
    TRACE9("SgtMayor::showNeededTricks() - Startplayer " << startPlayer);
    // Separate this from dealing the cards, to give the client a chance to
    // receive and perform the ActPlayer-message (to set the start-player)
    static constexpr std::array<char, NUM_PLAYERS> neededTricks{'6', '3', '8'};
    for (unsigned int i(0); i < NUM_PLAYERS; ++i) {
        Glib::ustring needed(_("(needs %1 tricks)"));
        needed.replace(needed.find("%1"), 2, 1, neededTricks[i]);
        players[(i + startPlayer) % NUM_PLAYERS].neededTricks.set_text(needed);
    }
}

//-----------------------------------------------------------------------------
/// Remove cards from everything which can hold them
//-----------------------------------------------------------------------------
void SgtMayor::clean() {
    TRACE9("SgtMayor::clean()");
    for (auto& player : players) {
        player.hand.clear();

        for (auto* card : player.won)
            card->show();
        player.won.clear();
    }

    if (pTrump) {
        remove(*pTrump);
        pTrump.reset();
    }

    menuSort->set_enabled(false);
    menuSort2->set_enabled(false);

    played.clear();
    Game::clean();
}

//-----------------------------------------------------------------------------
/// Shows or hides the cards of the computer player
/// \param open Flag if cards should be shown or hidden
//-----------------------------------------------------------------------------
void SgtMayor::playOpen(bool open) {
    for (unsigned int i(1); i < NUM_PLAYERS; ++i) {
        players[i].hand.setShowOption(open ? Card::IPile::SHOWFACE : Card::IPile::SHOWBACK);
        players[i].hand.setStyle(open ? Card::IPile::COMPRESSED : Card::IPile::QUITE_COMPRESSED);
    }

    for (auto& player : players) {
        player.won.setShowOption(open ? Card::IPile::SHOWFACE : Card::IPile::SHOWBACK);
        player.won.setStyle(open ? Card::IPile::COMPRESSED : Card::IPile::QUITE_COMPRESSED);

        for (auto c(player.won.begin()); c != player.won.end(); ++c)
            if (((c - player.won.begin()) % 3) != 2)
                open ? (*c)->show() : (*c)->hide();
    }
}

//-----------------------------------------------------------------------------
/// Enables the cards of the human player
/// \returns \c Flag, if time should be continued
/// \remarks Depending of the status of the game (PLAYING2) also the top card
///     of the played pile is enabled
//-----------------------------------------------------------------------------
bool SgtMayor::enableHuman() {
    Check3(activeCards.empty());
    Check3(!currentPlayer());
    TRACE2("SgtMayor::enableHuman() - Human has " << players[0].hand.size() << " cards");

    for (int i(players[0].hand.size() - 1); i >= 0; --i)
        activeCards.push_back(players[0].hand[i]->signal_clicked().connect(bind(mem_fun(*this, (&SgtMayor::cardSelected)), i)));

    return Game::enableHuman();
}

//-----------------------------------------------------------------------------
/// Callback after clicking on a card in hand
/// \param iCard Offset of card in hand
//-----------------------------------------------------------------------------
void SgtMayor::cardSelected(unsigned int iCard) {
    TRACE5("SgtMayor::cardSelected(unsigned int) - Position " << iCard);
    Check1(iCard < players[0].hand.size());
    Check2(gameStatus() == PLAYING);

    Card::HPile& pile(players[0].hand);
    Card::Widget& selCard(*pile[iCard]);
    Card::Widget::COLOURS playColour(selCard.colour());

    if (played.size()) {
        // The same colour must be played again (if available)
        Card::Widget::COLOURS colour(played[0]->colour());
        if (playColour != colour) {
            if (pile.exists(colour)) {
                Gtk::MessageDialog dlg(_("Play a card with an equal colour as "
                                         "the first played one!"),
                                       false, Gtk::MessageType::ERROR);
                dlg.set_title(PACKAGE " - SgtMayor");
                XGP::runModal(dlg);
                return;
            }
            bfColours |= (1 << played[0]->colour());
            Check2(pTrump);
            if (playColour != pTrump->colour())
                bfColours |= (1 << playColour);
        }
    }

    if (getConnectionMgr().getMode() != YGP::ConnectionMgr::NONE) {
        // Send played card to all clients (if any)
        std::ostringstream msg;
        msg << "Play=" << selCard.id() << ";Target=0";
        if (getConnectionMgr().getMode() == YGP::ConnectionMgr::CLIENT)
            ignoreNextMsg = true;
        broadcastMessage(msg.str());
    }

    animateCard(played, pile, iCard).sigAnimation.connect(bind(mem_fun(*this, &SgtMayor::playCardDelayed), 0));
    disableHuman();
}

//-----------------------------------------------------------------------------
/// Callback after clicking on a card to select the special colour
/// \param iCard Offset of card in hand
//-----------------------------------------------------------------------------
void SgtMayor::cardColourSelect(unsigned int iCard) {
    TRACE5("SgtMayor::cardColourSelect(unsigned int) - Position " << iCard);
    Check1(iCard < players[0].hand.size());
    Check2(gameStatus() == PLAYING);

    showTrump(players[0].hand[iCard]->colour());
    disableHuman();
    makeNextMoves();
}

//-----------------------------------------------------------------------------
/// Callback after clicking on a card to exchange bad cards with good ones
/// \param iCard Offset of card in hand
//-----------------------------------------------------------------------------
void SgtMayor::cardExchange(unsigned int iCard) {
    TRACE5("SgtMayor::cardExchange(unsigned int) - Position " << iCard);
    Check3(diffTricks[0]);

    disableHuman();
    for (unsigned int i(1); i < NUM_PLAYERS; ++i) {
        if (diffTricks[i] < 0) {
            exchangeCards(0, iCard, i);
            break;
        }
    }
}

//-----------------------------------------------------------------------------
/// Selects the trump
/// \returns bool False, if a human must select the trump colour
//-----------------------------------------------------------------------------
bool SgtMayor::selectTrump() {
    TRACE9("SgtMayor::selectTrump()");

    Glib::ustring stat;
    unsigned int trumpPlayer((startPlayer + 2) % 3);
    if (trumpPlayer) {
        unsigned int displayPlayer(convertPlayer(startPlayer));
        TRACE9("SgtMayor::selectTrump() - Trumpplayer: " << trumpPlayer);
        if (typeid(*actPlayers[trumpPlayer]) == typeid(Card::RemotePlayer)) {
            status.pop();
            stat = _("Waiting for %1 to select the special colour ...");
            stat.replace(stat.find("%1"), 2, actPlayers[displayPlayer]->getName());
            status.push(stat);
        }
        else {
            Check3(typeid(*actPlayers[trumpPlayer]) == typeid(Card::ComputerPlayer));
            // Find special colour
            Card::IPile& pile(players[trumpPlayer].hand);
            std::array<int, 4> number{};
            std::array<int, 4> points{};

            for (unsigned int i(0); i < (pile.size() - 1); ++i) {
                ++number[pile[i]->colour()];
                points[pile[i]->colour()] += pile[i]->number() + 1;
            }

            unsigned int trumpColour(0);
            for (unsigned int i(1); i < 4; ++i) {
                if ((number[i] > number[i - 1]) || ((number[i] == number[i - 1]) && (points[i] > points[i - 1])))
                    trumpColour = i;
            }
            showTrump(static_cast<Card::Widget::COLOURS>(trumpColour));
            displayTurn(displayPlayer, stat);
        }
    }
    else {
        status.pop();
        stat = _("Select the special colour");
        status.push(stat);

        for (int i(players[0].hand.size() - 1); i >= 0; --i)
            activeCards.push_back(
                players[0].hand[i]->signal_clicked().connect(bind(mem_fun(*this, (&SgtMayor::cardColourSelect)), i)));
        return false;
    }
    return true;
}

//-----------------------------------------------------------------------------
/// Starts the playing phase of the game
//-----------------------------------------------------------------------------
void SgtMayor::startPlaying() {
    TRACE7("SgtMayor::startPlaying()");
    Check3(!diffTricks[0]);
    Check3(!diffTricks[1]);
    Check3(!diffTricks[2]);

    playedCards[Card::Widget::CLUBS].set(Card::Widget::TWO);
    setNextPlayer(convertPlayer(startPlayer));
    if (selectTrump())
        makeNextMoves();
}

//-----------------------------------------------------------------------------
/// Searches for the card to play
/// \param player Player to inspect
/// \return unsigned int Card to play
//-----------------------------------------------------------------------------
unsigned int SgtMayor::findPos2Play(unsigned int player) {
    TRACE8("SgtMayor::findPos2Play(unsigned int) - Player " << player);
    Check1(player < NUM_PLAYERS);
    Check2(pTrump);

    unsigned int pos(0);
    Card::IPile& pile(players[player].hand);
    // Get the number of cards of each colour
    std::array<unsigned int, 4> cColours{};
    std::array<unsigned int, 4> posColours{-1U, -1U, -1U, -1U};
    for (unsigned int i(0); i < (pile.size() - 1); ++i) {
        ++cColours[pile[i]->colour()];
        if (pile[i]->colour() != pile[i + 1]->colour())
            posColours[pile[i]->colour()] = i;
    }
    posColours[pile[pile.size() - 1]->colour()] = pile.size() - 1;
    ++cColours[pile[pile.size() - 1]->colour()];
    TRACE8("SgtMayor::findPos2Play(unsigned int) - Nr: " << cColours[0] << '/' << cColours[1] << '/' << cColours[2] << '/'
                                                         << cColours[3]);
    TRACE9("SgtMayor::findPos2Play(unsigned int) - Pos: "
           << static_cast<int>(posColours[0]) << '/' << static_cast<int>(posColours[1]) << '/' << static_cast<int>(posColours[2])
           << '/' << static_cast<int>(posColours[3]));
    TRACE9("SgtMayor::findPos2Play(unsigned int) - Out: " << std::hex << bfColours << std::dec);

    switch (played.size()) {
    case 0: {
        // - Play trumps?
        Check3(pTrump);
        unsigned int trumpsLeft(13 - playedCards[pTrump->colour()].count());
        TRACE8("SgtMayor::findPos2Play(unsigned int) - Trumps: " << trumpsLeft << '/' << playedCards[pTrump->colour()].count());
        Check3(trumpsLeft <= 13);
        // If others have trumps left, but we have more; if player is not the
        // startplayer, assume, that the 3rd player has no more trumps left
        if ((trumpsLeft > cColours[pTrump->colour()]) &&
            ((trumpsLeft / ((player == startPlayer) ? 3 : 2)) < cColours[pTrump->colour()])) {
            Check3(pile.size() > posColours[pTrump->colour()]);
            pos = posColours[pTrump->colour()];
            if (!isHighest(*pile[posColours[pTrump->colour()]]))
                pos -= cColours[pTrump->colour()] - 1;
            break;
        }
        trumpsLeft -= cColours[pTrump->colour()];

        // - Have dead cards?
        int maxDiff(0);
        Card::Widget::COLOURS maxColour(Card::Widget::HEARTS);
        for (unsigned int i(0); i < cColours.size(); ++i) {
            int diff(cColours[i] - (13 - playedCards[i].count()) / 3);
            if ((diff > maxDiff) && (static_cast<int>(i) != pTrump->colour())) {
                maxDiff = diff;
                maxColour = static_cast<Card::Widget::COLOURS>(i);
            }
        }
        TRACE8("SgtMayor::findPos2Play(unsigned int) - Dead cards: " << maxDiff << ": " << static_cast<int>(maxColour));
        if (maxDiff) {
            Check2(pTrump);
            Check3(posColours[maxColour] != -1U);
            pos = posColours[maxColour];
            Check3((playedCards[maxColour].count() + cColours[maxColour]) <= 13);
            if (!isHighest(*pile[posColours[maxColour]]) ||
                (trumpsLeft &&
                 (((playedCards[maxColour].count() + cColours[maxColour]) > 11) ||
                  ((bfColours & (0x111 << maxColour)) && !((bfColours >> maxColour) & (bfColours >> pTrump->colour()))))))
                pos -= cColours[maxColour] - 1;
            break;
        }

        // Try to find the highest card
        pos = 0;
        while (pos < pile.size()) {
            pos = pile.findLastEqualColour(pos);
            if (isHighest(*pile[pos]) && (pile[pos]->colour() != pTrump->colour()) &&
                !(trumpsLeft && (bfColours & (0x110 << pile[pos]->colour()))))
                break;
            ++pos;
        }

        if (pos >= pile.size())
            pos = pile.findLowestCard(pTrump->colour());
        break;
    }

    case 1: {
        Check3((playedCards[played[0]->colour()].count() + cColours[played[0]->colour()]) <= 13);
        bool nextHasntColour((bfColours & ((1 << played[0]->colour()) << (calcNextPlayer(player) << 2))) ||
                             ((playedCards[played[0]->colour()].count() + cColours[played[0]->colour()]) > 12));
        pos = posColours[played[0]->colour()];
        TRACE9("SgtMayor::findPos2Play(unsigned int) - Play: " << static_cast<int>(pos) << "; Next: " << nextHasntColour);

        if (pos == -1U) {
            Check3((playedCards[pTrump->colour()].count() + cColours[pTrump->colour()]) <= 13);
            bfColours |= ((1 << played[0]->colour()) << (player << 2));
            pos = posColours[pTrump->colour()];
            if ((pos == -1U) || (nextHasntColour && !isHighest(*pile[pos]) &&
                                 ((cColours[pTrump->colour()] + playedCards[pTrump->colour()].count()) < 13))) {
                pos = pile.findLowestCard(pTrump->colour());
                bfColours |= (1 << pTrump->colour());
            }
            else
                pos -= cColours[pTrump->colour()] - 1;
        }
        else
            // If the next is know to not have the colour or player has not the
            // highest left of this colour, play a low one
            if (nextHasntColour || !isHighest(*pile[pos]) || played[0]->number() > pile[pos]->number())
                pos -= cColours[played[0]->colour()] - 1;
        break;
    }

    case 2:
        // The trick is taken by the second player with a trump. Either play a
        // small card or use a bigger trump.
        if ((played[0]->colour() != pTrump->colour()) && (played[1]->colour() == pTrump->colour())) {
            pos = pile.find(played[0]->colour());
            if (pos == -1U) {
                pos = pile.find1EqualOrBiggerByColour(*played[1]);
                if (pos == -1U) {
                    bfColours |= ((1 << played[0]->colour()) << (player << 2));
                    pos = pile.findLowestCard(pTrump->colour());
                    if (pile[pos]->colour() != pTrump->colour())
                        bfColours |= (1 << pTrump->colour());
                }
            }
        }
        else {
            pos = pile.find1EqualOrBiggerByColour(
                ((played[0]->colour() != played[1]->colour()) || (played[0]->number() > played[1]->number())) ? *played[0]
                                                                                                              : *played[1]);
            if ((pos == -1U) || (pile[pos]->colour() != played[0]->colour())) {
                pos = (pile.exists(played[0]->colour())
                           ? pile.find(played[0]->colour())
                           : (bfColours |= ((1 << played[0]->colour()) << (player << 2)), tryToGetTrickWithTrump(pile)));
                if (pile[pos]->colour() != pTrump->colour())
                    bfColours |= (1 << pTrump->colour());
            }
        }
        break;

    default:
        Check3(0);
    }
    Check3(pos < pile.size());
    return pos;
}

//-----------------------------------------------------------------------------
/// Changes the names of the playing people
/// \param newPlayer Array holding the new player
//-----------------------------------------------------------------------------
void SgtMayor::changeNames(const std::vector<Card::Player*>& newPlayer) {
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
/// \param player Actual player
/// \param pile ID of the pile to return
/// \returns Card::IPile* Pointer to pile to use or NULL
//----------------------------------------------------------------------------
Card::IPile* SgtMayor::getPileOfPlayer(unsigned int player, unsigned int pile) {
    TRACE9("SgtMayor::getPileOfPlayer(2x unsigned int) - Player " << player << "; " << pile);
    if ((player + posServer) >= NUM_PLAYERS)
        player -= posServer;
    Check3(player < NUM_PLAYERS);
    Check3(!pile);
    return ((player >= NUM_PLAYERS) || pile) ? nullptr : &players[player].hand;
}

#if 0
//-----------------------------------------------------------------------------
/// Reads card- and playernumber from the next tokens
/// \param src String to analyze
/// \param card Filled with number of card
/// \param player Filled with player number
/// \returns bool True, if parsing was successfull
//-----------------------------------------------------------------------------
bool SgtMayor::readCardInfo (Card::Tokenize& src, unsigned long& card, unsigned long& player) {
   std::string strCard (src.getNextNode (';'));
   std::string from (src.getNextNode ('='));
   std::string strPlayer (src.getNextNode (';'));
   if ((from == "From")
       && !stringToNumber (card, strCard.c_str ())
       && (card < 52)
       && !stringToNumber (player, strPlayer.c_str ())
       && (player < NUM_PLAYERS))
      return true;
   return false;
}

//----------------------------------------------------------------------------
/// Handles the messages the server might send for the Sgt.Mayor cardgame
/// \param player ID of player sending the message
/// \param message Message received from the server
/// \returns bool True, if message has been completey processed
/// \throw YGP::ParseError, YGP::CommError In case of an error an describing text
//----------------------------------------------------------------------------
bool SgtMayor::handleMessage (unsigned int player, const std::string& message) {
   TRACE1 ("SgtMayor::handleMessage (unsigned int player, const std::string&) - "
	   << message << " (" << player << ')');
   Card::Tokenize command (message);
   std::string cmd (command.getNextNode ('='));

   if (cmd == "Exchange") {
      unsigned long card1, card2;
      unsigned long player1, player2;
      if (readCardInfo (command, card1, player1)
	  && (command.getNextNode ('=') == "With")
	  && (readCardInfo (command, card2, player2))) {
	 player1 = (NUM_PLAYERS + player1 - posServer) % NUM_PLAYERS;
	 player2 = (NUM_PLAYERS + player2 - posServer) % NUM_PLAYERS;

	 card1 = players[player1].hand.find (static_cast <unsigned int> (card1));
	 card2 = players[player2].hand.find (static_cast <unsigned int> (card2));
	 if ((card1 == -1U) || (card2 == -1U))
	    throw YGP::ParseError (N_("Card not found!"));

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
	    doShowTrump ((Card::Widget::COLOURS)trumpColour);
	 else
	    showTrump ((Card::Widget::COLOURS)trumpColour);
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
      showNeededTricks ();
      makeExchange ();
   }
   return rc;
}
#endif

//----------------------------------------------------------------------------
/// Shows the special colour on the board (the ace with that colour). Also
/// inform connected player about it
/// \param colour The special colour to display
//----------------------------------------------------------------------------
void SgtMayor::showTrump(Card::Widget::COLOURS colour) {
    TRACE9("SgtMayor::showTrump(Card::Widget::COLOURS) - " << colour);
    if (getConnectionMgr().getMode() != YGP::ConnectionMgr::NONE) {
        if (getConnectionMgr().getMode() == YGP::ConnectionMgr::CLIENT)
            ignoreNextMsg = true;

        std::ostringstream msg;
        msg << "Trump=" << colour;
        broadcastMessage(msg.str());
    }

    doShowTrump(colour);
}

//----------------------------------------------------------------------------
/// Shows the special colour on the board (the ace with that colour)
/// \param colour The special colour to display
//----------------------------------------------------------------------------
void SgtMayor::doShowTrump(Card::Widget::COLOURS colour) {
    TRACE9("SgtMayor::doShowTrump(Card::Widget::COLOURS) - " << colour);
    for (unsigned int i(0); i < cards.size(); ++i) {
        if ((cards.getCards()[i]->number() == Card::Widget::ACE) && (cards.getCards()[i]->colour() == colour)) {
            Check3(!pTrump);
            pTrump = std::make_unique<Card::Widget>(*cards.getCards()[i]);
            pTrump->show();
            pTrump->showFace();
            pTrump->set_margin_start(5);
            pTrump->set_margin_end(5);
            pTrump->set_margin_top(1);
            pTrump->set_margin_bottom(1);
            attach(*pTrump, 0, 7, 1, 1);
            displayTurn(convertPlayer(startPlayer));
            return;
        }
    }
    Check3(0);
}

//----------------------------------------------------------------------------
/// Plays the passed card; find winner and give him the cards at the end of a
/// turn.
/// \param player Player on turn
/// \returns unsigned int Next player; or -1U, if end of game
//----------------------------------------------------------------------------
unsigned int SgtMayor::playCard(unsigned int player) {
    TRACE3("SgtMayor::playCard(unsigned int, unsigned int) - Player " << player);

    if (played.size() == NUM_PLAYERS) {
        // Find the winner
        auto i(played.begin());
        Card::Widget::NUMBERS nr((*i)->number());
        Card::Widget::COLOURS col((*i)->colour());
        unsigned int bestPlayer(0);

        Check3(pTrump);
        while (++i != played.end()) {
            TRACE8("SgtMayor::playCard(unsigned int) - Comparing " << (**(i - 1)) << " - " << **i);

            if ((col != pTrump->colour()) && ((*i)->colour() == pTrump->colour())) {
                TRACE9("SgtMayor::playCard(unsigned int) - Found trump ");
                col = pTrump->colour();
                nr = (*i)->number();
                bestPlayer = i - played.begin();
                continue;
            }

            if (((*i)->number() > nr) && ((*i)->colour() == col)) {
                TRACE9("SgtMayor::playCard(unsigned int) - New best card " << **i);
                nr = (*i)->number();
                bestPlayer = i - played.begin();
            }
        }
        TRACE9("SgtMayor::playCard(unsigned int) - Calc. winner from " << player << " and " << bestPlayer);
        player = (player + 1 + bestPlayer) % NUM_PLAYERS;
        TRACE9("SgtMayor::playCard(unsigned int) - Winner " << player);

        // Move the cards to his won pile (but show only one of them)
        while (played.size() > 1) {
            Card::Widget& card(played.remove(0));
            card.hide();
            players[player].won.Card::IPile::append(card);
        }
        players[player].won.Card::IPile::append(played.removeTopCard());
        Check3(played.empty());

        if (!player) {
            enableWonCards(players[0].won);
            menuSort->set_enabled();
            menuSort2->set_enabled();
        }
    }
    else
        player = calcNextPlayer(player);

    if (players[player].hand.size()) {
        Check3((posServer + player) < actPlayers.size());
        displayTurn(convertPlayer(player));
        return player;
    }

    Glib::ustring stat(_("Game ended; %1 has %2 %7, %3 %4 and %5 %6 %8"));
    static constexpr std::array<unsigned int, NUM_PLAYERS> neededTricks{6, 3, 8};
    player = startPlayer;
    for (unsigned int neededTrick : neededTricks) {
        int madeTricks(players[player].won.size() / NUM_PLAYERS);
        diffTricks[player] = madeTricks - neededTrick;
        TRACE9("SgtMayor::playCard(unsigned int) - Player " << player << " made " << madeTricks << " = " << diffTricks[player]);

        player = calcNextPlayer(player);
    }
    Check(!(diffTricks[0] + diffTricks[1] + diffTricks[2]));

    // Create score-dialog
    if (!pScoreDlg) {
        // Resort player for score dialogue
        std::vector<Card::Player*> player;
        for (unsigned int i(0); i < NUM_PLAYERS; ++i)
            player.push_back(actPlayers[i]);

        pScoreDlg.reset(Card::ScoreDlg::create(player));
        Gtk::Window* win(dynamic_cast<Gtk::Window*>(get_root()));
        if (win)
            pScoreDlg->set_transient_for(*win);
        menuShowScoreDlg->set_enabled();
    }

    pScoreDlg->addPoints(diffTricks.data());
    pScoreDlg->display();

    int points;
    pScoreDlg->getMaxPoints(points, player);
    Check3(points >= 0);
    if (points >= static_cast<int>(ENDTRICKS)) {
        Glib::ustring won(_("; %1 won"));
        won.replace(won.find("%1"), 2, actPlayers[convertPlayer(player)]->getName());
        stat += won;
    }

    stat.replace(stat.find("%1"), 2, actPlayers[0]->getName());
    stat.replace(stat.find("%3"), 2, actPlayers[convertPlayer(1)]->getName());
    stat.replace(stat.find("%5"), 2, actPlayers[convertPlayer(2)]->getName());

    stat.replace(stat.find("%2"), 2, formatNumber(diffTricks[0]));
    stat.replace(stat.find("%4"), 2, formatNumber(diffTricks[1]));
    stat.replace(stat.find("%6"), 2, formatNumber(diffTricks[2]));

    stat.replace(stat.find("%7"), 2, (ngettext("trick", "tricks", (diffTricks[0] < 0) ? -diffTricks[0] : diffTricks[0])));
    stat.replace(stat.find("%8"), 2, (ngettext("trick", "tricks", (diffTricks[2] < 0) ? -diffTricks[2] : diffTricks[2])));

    status.pop();
    status.push(stat);
    setGameStatus(STOPPED);
    return -1U;
}

//-----------------------------------------------------------------------------
/// Formats a number with sign character always shown
/// \param nr Number to format
/// \returns std::string Formatted number
/// \remarks Shows the sign always (e.g. also the plus sign (+)
//-----------------------------------------------------------------------------
std::string SgtMayor::formatNumber(int nr) {
    std::ostringstream msg;
    msg << std::showpos << nr;
    return msg.str();
}

//----------------------------------------------------------------------------
/// Checks if the passed card is the highest card of its colour, which has
/// not been played.
/// \param card Card to inspect
/// \return bool True, if card is the highest unplayed one
//----------------------------------------------------------------------------
bool SgtMayor::isHighest(const Card::Widget& card) const {
    TRACE8("SgtMayor::isHighest(const Card::Widget&) - " << card);

    int nr(card.number());
    while (++nr <= Card::Widget::ACE) {
        if (!playedCards[card.colour()][nr])
            return false;
    }
    return true;
}

//----------------------------------------------------------------------------
/// Tries to get the trick with a trump card; returns a bad card, if there's no
/// trump.
/// \param pile Pile to play from
/// \return unsigned int Position of card to play
//----------------------------------------------------------------------------
unsigned int SgtMayor::tryToGetTrickWithTrump(const Card::IPile& pile) const {
    TRACE8("SgtMayor::tryToGetTrickWithTrump(const Card::IPile&)  - Size " << pile.size());
    Check3(pile.size());

    unsigned int pos(pile.find(pTrump->colour()));
    if (pos == -1U)
        pos = pile.findLowestCard(pTrump->colour());
    return pos;
}

//----------------------------------------------------------------------------
/// Exchanges cards between players having too much/too less tricks in the last
/// round.
//----------------------------------------------------------------------------
void SgtMayor::makeExchange() {
    TRACE5("SgtMayor::makeExchange() - Exchanging cards: " << (diffTricks[0] > 0 ? diffTricks[0] : 0) +
                                                                  (diffTricks[1] > 0 ? diffTricks[1] : 0) +
                                                                  (diffTricks[2] > 0 ? diffTricks[2] : 0));
    Check3((diffTricks[0] + diffTricks[1]) == -diffTricks[2]);

    for (unsigned int i(startPlayer); (i - startPlayer) < NUM_PLAYERS; ++i) {
        TRACE9("SgtMayor::makeExchange() - " << i % NUM_PLAYERS << "'s tricks: " << diffTricks[i % NUM_PLAYERS]);

        if (diffTricks[i % NUM_PLAYERS] > 0) {
            Check3((diffTricks[(i + 1) % NUM_PLAYERS] < 0) || (diffTricks[(i + 2) % NUM_PLAYERS] < 0));

            for (unsigned int j(1); j < NUM_PLAYERS; ++j) {
                TRACE9("SgtMayor::makeExchange() - With " << (i + j) % NUM_PLAYERS
                                                          << "'s tricks: " << diffTricks[(i + j) % NUM_PLAYERS]);
                if (diffTricks[(j + i) % NUM_PLAYERS] < 0) {
                    if (i % NUM_PLAYERS) {
                        if ((getConnectionMgr().getMode() == YGP::ConnectionMgr::CLIENT) ||
                            (typeid(*actPlayers[convertPlayer(i % NUM_PLAYERS)]) == typeid(Card::RemotePlayer))) {
                            Glib::ustring msg(_("Waiting for %1 to exchange cards ..."));
                            msg.replace(msg.find("%1"), 2, actPlayers[convertPlayer(i % NUM_PLAYERS)]->getName());
                            status.push(msg);
                        }
                        else
                            exchangeCards(i % NUM_PLAYERS, (i + j) % NUM_PLAYERS);
                    }
                    else {
                        Check3(diffTricks[0] > 0);
                        displayExchangeStatus();

                        for (int i(players[0].hand.size() - 1); i >= 0; --i)
                            activeCards.push_back(
                                players[0].hand[i]->signal_clicked().connect(bind(mem_fun(*this, (&SgtMayor::cardExchange)), i)));
                    }
                    return;
                }
            }
        }
    }

    // Glib::signal_timeout ().connect (bind_return (mem_fun (*this, &SgtMayor::makeExchange), false), 400);
    startPlaying();
}

//-----------------------------------------------------------------------------
/// Displays the number of cards the human can exchange and with whom
//-----------------------------------------------------------------------------
void SgtMayor::displayExchangeStatus() {
    Glib::ustring msg;
    unsigned int exchg(1);

    if ((diffTricks[1] < 0) && (diffTricks[2] < 0)) {
        msg = _("You can exchange %1 %2; %4 with %3 (and than %5 with %6)!");

        msg.replace(msg.find("%4"), 2, 1, static_cast<char>((diffTricks[1] < 0) ? ('0' - diffTricks[1]) : ('0' + diffTricks[1])));
        msg.replace(msg.find("%5"), 2, 1, static_cast<char>((diffTricks[2] < 0) ? ('0' - diffTricks[2]) : ('0' + diffTricks[2])));
        msg.replace(msg.find("%6"), 2, actPlayers[convertPlayer(2)]->getName());
    }
    else {
        msg = _("You can exchange %1 %2 with %3!");
        if (diffTricks[2] < 0)
            exchg = 2;
    }
    msg.replace(msg.find("%1"), 2, 1, static_cast<char>((diffTricks[0] < 0) ? ('0' - diffTricks[0]) : ('0' + diffTricks[0])));
    msg.replace(msg.find("%2"), 2, (ngettext("bad card", "bad cards", (diffTricks[0] < 0) ? -diffTricks[0] : diffTricks[0])));
    msg.replace(msg.find("%3"), 2, actPlayers[convertPlayer(exchg)]->getName());

    status.pop();
    status.push(msg);
}

//----------------------------------------------------------------------------
/// Exchanges a good card from playerGood with a bad card from player bad
/// \param playerBad Player giving away a bad card
/// \param playerGood Player giving away a good card
//----------------------------------------------------------------------------
void SgtMayor::exchangeCards(unsigned int playerBad, unsigned int playerGood) {
    TRACE7("SgtMayor::exchangeCards(unsigned int, unsigned int) - Players " << playerBad << " and " << playerGood);
    Check1(playerBad < NUM_PLAYERS);
    Check1(playerGood < NUM_PLAYERS);

    unsigned int posBad(players[playerBad].hand.findLowestCard());
    Check3(posBad < players[playerBad].hand.size());
    exchangeCards(playerBad, posBad, playerGood);
}

//-----------------------------------------------------------------------------
/// Animated exchange of a card; after the exchange, the receiver gives back a card.
/// \param playerBad Player giving away a bad card
/// \param posBad Position of bad card to give away
/// \param playerGood Player giving away a good card
/// \param posGood Position of good card to give away
//-----------------------------------------------------------------------------
void SgtMayor::exchange(unsigned int playerBad, unsigned int posBad, unsigned int playerGood, unsigned int posGood) {
    Check1(playerBad < NUM_PLAYERS);
    Check1(playerGood < NUM_PLAYERS);
    Check2(playerBad != playerGood);
    Check2(posBad < players[playerBad].hand.size());
    Check2(posGood < players[playerGood].hand.size());

    Card::HPile& pileBad(players[playerBad].hand);
    Card::HPile& pileGood(players[playerGood].hand);
    TRACE9("SgtMayor::exchange(4x unsigned int ) - Player " << playerBad << " and " << playerGood << " exchange "
                                                            << *pileBad[posBad] << " and " << *pileGood[posGood]);

    if (!playerGood)
        flipCards2Play(pileBad, posBad, posBad);
    animateCard(pileGood, pileBad, posBad)
        .sigAnimation.connect(bind(mem_fun(*this, &SgtMayor::exchgBack), playerBad, playerGood, posGood));
}

//-----------------------------------------------------------------------------
/// Animated exchange of a card; after the exchange further cards might be exchanged
/// \param playerBad Player giving away a bad card
/// \param playerGood Player giving away a good card
/// \param posGood Position of good card to give away
//-----------------------------------------------------------------------------
void SgtMayor::exchgBack(unsigned int playerBad, unsigned int playerGood, unsigned int posGood) {
    Card::HPile& pileGood(players[playerGood].hand);
    Card::HPile& pileBad(players[playerBad].hand);

    if (!playerBad)
        flipCards2Play(pileGood, posGood, posGood);
    animateCard(pileBad, pileGood, posGood).sigAnimation.connect(bind(mem_fun(*this, &SgtMayor::exchgNext), &pileGood, &pileBad));
}

//-----------------------------------------------------------------------------
/// Sorts the passed pile and exchanges the next cards (or starts the game)
/// \param pileGood Pile to sort
/// \param pileBad Pile to sort
//-----------------------------------------------------------------------------
void SgtMayor::exchgNext(Card::HPile* pileGood, Card::HPile* pileBad) {
    Check1(pileGood);
    Check1(pileBad);
    pileBad->sortByColour();
    pileGood->sortByColour();
    makeExchange();
}

//----------------------------------------------------------------------------
/// Exchanges a good card from playerGood with a bad card from player bad
/// \param playerBad Player giving away a bad card
/// \param posBad Position of bad card to give away
/// \param playerGood Player giving away a good card
//----------------------------------------------------------------------------
void SgtMayor::exchangeCards(unsigned int playerBad, unsigned int posBad, unsigned int playerGood) {
    TRACE7("SgtMayor::exchangeCards(3x unsigned int int) - Players " << playerBad << " and " << playerGood);
    Check1(playerBad < NUM_PLAYERS);
    Check1(playerGood < NUM_PLAYERS);
    Check1(posBad < players[playerBad].hand.size());

    unsigned int posGood(players[playerGood].hand.findLastEqualOrBiggerColour(players[playerBad].hand[posBad]->colour()));
    if (posGood == -1U)
        posGood = players[playerGood].hand.findLowestCard();
    TRACE9("SgtMayor::exchangeCards(3x unsigned int) - Player " << playerBad << ", card " << posBad << " with " << playerGood
                                                                << "'s " << posGood);
    Check3(posGood < players[playerGood].hand.size());

    exchangeCards(playerBad, posBad, playerGood, posGood);
}

//----------------------------------------------------------------------------
/// Exchanges a good card from playerGood with a bad card from player bad
/// and informs the connected partners about it
/// \param playerBad Player giving away a bad card
/// \param posBad Position of bad card to give away
/// \param playerGood Player giving away a good card
//----------------------------------------------------------------------------
void SgtMayor::exchangeCards(unsigned int playerBad, unsigned int posBad, unsigned int playerGood, unsigned int posGood) {
    TRACE7("SgtMayor::exchangeCards(4x unsigned int int) - Players " << playerBad << " and " << playerGood);
    Check1(playerBad < NUM_PLAYERS);
    Check1(playerGood < NUM_PLAYERS);
    Check1(posBad < players[playerBad].hand.size());
    Check1(posGood < players[playerGood].hand.size());

    // Broadcast exchange-info to others
    if (getConnectionMgr().getMode() != YGP::ConnectionMgr::NONE) {
        std::ostringstream msg;
        msg << "Exchange=" << players[playerGood].hand[posGood]->id() << ";From=" << (playerGood + posServer) % NUM_PLAYERS
            << ";With=" << players[playerBad].hand[posBad]->id() << ";From=" << (playerBad + posServer) % NUM_PLAYERS;

        if (getConnectionMgr().getMode() == YGP::ConnectionMgr::CLIENT)
            ignoreNextMsg = true;
        broadcastMessage(msg.str());
    }
    doExchangeCards(playerBad, posBad, playerGood, posGood);
}

//----------------------------------------------------------------------------
/// Exchanges a good card from playerGood with a bad card from player bad
/// \param playerBad Player giving away a bad card
/// \param posBad Position of bad card to give away
/// \param playerGood Player giving away a good card
/// \param posGood Position of good card to give away
//----------------------------------------------------------------------------
void SgtMayor::doExchangeCards(unsigned int playerBad, unsigned int posBad, unsigned int playerGood, unsigned int posGood) {
    TRACE7("SgtMayor::doExchangeCards(4x unsigned int int) - Players " << playerBad << " and " << playerGood);
    Check1(playerBad < NUM_PLAYERS);
    Check1(playerGood < NUM_PLAYERS);
    Check1(posBad < players[playerBad].hand.size());
    Check1(posGood < players[playerGood].hand.size());
    TRACE9("SgtMayor::doExchangeCards(4x unsigned int ) - Player " << playerBad << " and " << playerGood << " exchange "
                                                                   << *players[playerBad].hand[posBad] << " and "
                                                                   << *players[playerGood].hand[posGood]);
    Check2((players[playerGood].hand[posGood]->colour() == players[playerBad].hand[posBad]->colour()) ||
           (!players[playerGood].hand.exists(players[playerBad].hand[posBad]->colour())));

    --diffTricks[playerBad];
    ++diffTricks[playerGood];

    exchange(playerBad, posBad, playerGood, posGood);
}

//-----------------------------------------------------------------------------
/// Adds game-specific menus
/// \param menu Menu to add game-specific entries to
/// \param actions Action group to add game-specific actions to
//-----------------------------------------------------------------------------
void SgtMayor::addMenus(const Glib::RefPtr<Gio::Menu>& menu, const Glib::RefPtr<Gio::SimpleActionGroup>& actions) {
    Check1(menu);
    Check1(actions);

    Glib::RefPtr<Gio::Menu> sub(Gio::Menu::create());

    menuSort = actions->add_action("SgMayorSort", mem_fun(*this, &SgtMayor::sortWonByNumber));
    sub->append(_("_Sort won cards (by number)"), "game.SgMayorSort");

    menuSort2 = actions->add_action("SgMayorSortCol", mem_fun(*this, &SgtMayor::sortWonByColour));
    sub->append(_("Sort won cards (by _colour)"), "game.SgMayorSortCol");

    Glib::RefPtr<Gio::Menu> sec(Gio::Menu::create());
    menuShowScoreDlg = actions->add_action("showScoreDlg", [this]() {
        if (pScoreDlg)
            pScoreDlg->display();
    });
    sec->append(_("Show score dialog"), "game.showScoreDlg");
    sub->append_section(sec);

    menu->append_submenu(_("_Sgt. Mayor"), sub);

    menuShowScoreDlg->set_enabled(false);
}

//-----------------------------------------------------------------------------
/// Removes the game-specific menus
/// \param menu Menu to remove game-specific entries from
/// \param actions Action group to remove game-specific actions from
//-----------------------------------------------------------------------------
void SgtMayor::removeMenus(const Glib::RefPtr<Gio::Menu>& menu, const Glib::RefPtr<Gio::SimpleActionGroup>& actions) {
    Check1(menu);
    Check1(actions);

    menu->remove(menu->get_n_items() - 1);
    actions->remove_action("SgMayorSort");
    actions->remove_action("SgMayorSortCol");
    actions->remove_action("showScoreDlg");
}

//-----------------------------------------------------------------------------
/// Shows or hides the won cards
/// \param show Flag if to show or to hide the cards
/// \param style Style of the won pile (ignored; only traced)
//-----------------------------------------------------------------------------
void SgtMayor::showWonCards(bool show, [[maybe_unused]] unsigned int style) {
    TRACE9("SgtMayor::showWonCards(bool, unsigned int) - " << show << '/' << style);

    for (auto c(players[0].won.begin()); c != players[0].won.end(); ++c)
        if (show)
            (*c)->show();
        else if (((c - players[0].won.begin()) % 3) != 2)
            (*c)->hide();

    if (show)
        Game::showWonCards(true);
    else
        Game::showWonCards(false, Card::IPile::QUITE_COMPRESSED);
}

//-----------------------------------------------------------------------------
/// Actions to take when the cards are resized
/// \pre The cardsize must be set in CardImages::WIDTH/HEIGHT
//-----------------------------------------------------------------------------
void SgtMayor::resizeCards() {
    played.set_size_request(Card::Images::WIDTH + 150, Card::Images::HEIGHT);
    for (unsigned int i(0); i < NUM_PLAYERS; ++i) {
        players[i].hand.set_size_request(Card::Images::WIDTH + 17 * (i ? 7 : 18), Card::Images::HEIGHT + 5);
        players[i].won.set_size_request(Card::Images::WIDTH + 17 * 7, Card::Images::HEIGHT + 5);
    }
}
