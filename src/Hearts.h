#ifndef HEARTS_H
#define HEARTS_H

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
#include <memory>
#include <vector>

#include <gtkmm/label.h>

#include <card/Pile.h>
#include <card/Set.h>

#include <card/Game.h>

#include "HeartsRules.h"

namespace Card {
class ScoreDlg;
}
namespace Gio {
class SimpleAction;
}

// Class to handle the Hearts cardgame
class Hearts : public Card::Game {
    friend class Settings;
    friend class CardgameAppl;
    friend class CardgameCollection;

  public:
    Hearts(Gtk::Box& parent, Gtk::Statusbar& statusbar, Card::Set& cardset, const std::vector<Card::Player*>& player,
           unsigned int posPlayer, Card::MessageLock& mxSerialize);
    ~Hearts() override;

    void start() override;
    void clean() override;
    void playOpen(bool open) override;
    const char* name() override { return "Hearts"; }
    void changeNames(const std::vector<Card::Player*>& newPlayer) override;
    void resizeCards() override;

    bool handleMessage(unsigned int player, const std::string& message) override;

  protected:
    Card::IPile* getPileOfPlayer(unsigned int player, unsigned int pile) override;

  private:
    enum Status { EXCHANGE = Game::LAST };

    // Protected manager functions
    Hearts(const Hearts& other) = delete;
    Hearts& operator=(const Hearts& other) = delete;

    //@Section Event handling
    void cardSelected(unsigned int iCard);
    void takeCard(unsigned int iCard);
    void cardTaken();

    //@Section Virtual methods
    void makeMove(unsigned int player) override;
    bool enableHuman() override;

    //@Section Helper methods
    bool moveSelectedCardToPlayed(unsigned int player, unsigned int card);
    unsigned int calcNextPlayer(unsigned int player);
    HeartsRules::Table currentTable() const;
    void exchangeCards();
    void finishExchangeCards();
    void finishMove();
    void takeWonCards(unsigned int player);
    bool cardsExchanged(unsigned int cards);
    bool allCardsExchanged();

    void startPlaying();

    void addMenus(const Glib::RefPtr<Gio::Menu>& menu, const Glib::RefPtr<Gio::SimpleActionGroup>& actions) override;
    void removeMenus(const Glib::RefPtr<Gio::Menu>& menu, const Glib::RefPtr<Gio::SimpleActionGroup>& actions) override;

    static constexpr unsigned int NUM_PLAYERS = HeartsRules::NUM_PLAYERS; // Number of players

    bool playedSQ;                       // Flag, if the queen of spades has been played
    std::array<unsigned int, 4> aPlayed; // Array holding played cars for each colour

    unsigned int player2Exchange;                   // ID of (next) player to exchange cards with
    std::array<Card::IPile, NUM_PLAYERS> aExchange; // Cards the players are exchanging

    struct playerCards {
        std::unique_ptr<Card::IPile> hand; // For players: Cards in the hand
        std::unique_ptr<Card::IPile> won;  // Won cards
        Gtk::Label name;

        playerCards() : hand(), won(), name() {}

      private:
        playerCards(const playerCards&) = delete;
        playerCards& operator=(const playerCards&) = delete;
    };
    std::array<playerCards, NUM_PLAYERS> players;
    Card::HPile played;

    std::unique_ptr<Card::ScoreDlg> pScoreDlg;

    Glib::RefPtr<Gio::SimpleAction> menuSort;
    Glib::RefPtr<Gio::SimpleAction> menuSort2;
    Glib::RefPtr<Gio::SimpleAction> menuShowScoreDlg;

    static constexpr std::array<unsigned int, NUM_PLAYERS> COLS_PLAYER{5, 9, 5, 3};
    static constexpr std::array<unsigned int, NUM_PLAYERS> ROWS_PLAYER{10, 7, 3, 7};

    static unsigned int ENDPOINTS;
};

#endif
