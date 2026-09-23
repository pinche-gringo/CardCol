#ifndef ROVHULT_H
#define ROVHULT_H

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
#include <map>
#include <vector>

#include <gtkmm/label.h>

#include <card/Pile.h>
#include <card/Set.h>

#include <card/Game.h>

namespace Gtk {
class DragSource;
class DropTarget;
} // namespace Gtk

/**Class to handle the Rovhult-cardgame
 */
class Rovhult : public Card::Game {
    friend class Settings;
    friend class CardgameAppl;
    friend class CardgameCollection;

  public:
    // Manager functions
    Rovhult(Gtk::Box& parent, Gtk::Statusbar& statusbar, Card::Set& cardset, const std::vector<Card::Player*>& players,
            unsigned int posPlayer, Card::MessageLock& mxSerialize);
    ~Rovhult() override;

    void end(bool restart) override;
    void start() override;
    void playOpen(bool open) override;
    void clean() override;
    const char* name() override { return "Rovhult"; }
    void changeNames(const std::vector<Card::Player*>& newPlayer) override;
    void resizeCards() override;

    bool handleMessage(unsigned int player, const std::string& msg) override;

  protected:
    Card::IPile* getPileOfPlayer(unsigned int player, unsigned int pile) override;
    bool executeRemoteMove(Card::IPile& pile, unsigned int target) override;

  private:
    enum { EXCHANGE = Game::LAST, EXCHANGED };

    // Protected manager functions
    Rovhult(const Rovhult&) = delete;
    Rovhult& operator=(const Rovhult&) = delete;

    // Drag and drop handling
    bool cardDroppedOnTable(unsigned int handCard, unsigned int pile);
    bool cardDroppedOnHand(unsigned int tablePile, unsigned int card);

    void registerHandDND(Card::Widget& card, unsigned int iCard);
    void registerTableDND(Card::Widget& card, unsigned int pile);
    void unregisterDND(Card::Widget& card) const;
    void unregisterDND();

    // Event-handling
    void pileSelected(unsigned int pile);
    void handSelected(unsigned int pos);
    void finishedExchange(unsigned int iCard);
    void takeCards();
    void cardsTaken(unsigned int player);

    bool doPileSelected(unsigned int player, unsigned int pile);
    bool doSwapCards(unsigned int pile, unsigned int card);

    // Helper functions
    void movePlayedCardsToLoser(unsigned int nrLoser);
    int nextAvailablePlayer(unsigned int actPlayer) const;
    void makeMove(unsigned int player) override;
    bool enableHuman() override;
    void dealCards();
    void playCardsFromHand(unsigned int player, unsigned int start, unsigned int end);
    void exchangeAutoplayerCards();
    void sortReserve(unsigned int player);

    unsigned int numberOfEqualTopCards() const;
    bool played4Equal();
    void fillUpPile(Card::IPile& pile, unsigned int minCards);

    int makeTurn(unsigned int player);
    void findCard2Play(unsigned int player, unsigned int& start, unsigned int& end) const;
    void showCards2Play(unsigned int player, unsigned int start, unsigned int end);
    void showCardOfPile(unsigned int player, unsigned int pile, bool invalid) const;
    unsigned int selectRandomCard(unsigned int player);

    static int compareCards(const Card::Widget& lhs, const Card::Widget& rhs);
    bool getPileLimits(unsigned int player, Card::Widget::NUMBERS& min, Card::Widget::NUMBERS& max) const;
    bool existOnlySpecialCards(const Card::IPile& pile, unsigned int start, unsigned int end) const;
    bool isSpecialCard(Card::Widget::NUMBERS nr) const { return (nr == Card::Widget::TEN) || (nr == Card::Widget::TWO); }
    static unsigned int getCardValue(const Card::Widget& card);

    int skip(Card::Widget::NUMBERS nr, const Card::IPile& pile, unsigned int pos) const {
        if (pile[pos]->number() == nr) {
            pos = pile.findLastEqual(pos) + 1;
            return (pos < pile.size()) ? static_cast<int>(pos) : -1;
        }
        return pos;
    }

    bool cardValid(Card::Widget::NUMBERS nr, bool silent = false) const;
    void executeMove(unsigned int player);
    void unmarkAndExecuteMove(unsigned int player, unsigned int count);
    bool unmarkAndMoveToLoser(unsigned int player, unsigned int start, unsigned int end);

    void sendExchangedCards(unsigned int player);
    void makeRemoteMove(unsigned int player, unsigned int start, unsigned int end);
#ifdef WITH_NETWORK
    bool applyExchange(unsigned int player, const std::string& message);
#endif

    static constexpr unsigned int NUM_PLAYERS = 4; // Number of players

    // Columns and rows for the cards of the players
    static constexpr std::array<unsigned int, NUM_PLAYERS> COLS_PLAYER{7, 13, 7, 1};
    static constexpr std::array<unsigned int, NUM_PLAYERS> ROWS_PLAYER{13, 7, 4, 7};

    Card::HInfoPile played;
    Card::VInfoPile staple; // Cards on staple
    struct playerCards {
        Card::HPile hand;                   // For players: Cards in the hand
        std::array<Card::VPile, 3> reserve; // Reserve-cards (for end-game)
        Gtk::Label name;

        playerCards() : hand(), name() {}

      private:
        playerCards(const playerCards&) = delete;
        playerCards& operator=(const playerCards&) = delete;
    };
    std::array<playerCards, NUM_PLAYERS> players;

    unsigned int aExchanged;
    unsigned int remoteTarget; ///< Kind of the received move to execute (see getPileOfPlayer; 5 - 7 are stored as 4)

    // Preventing endless-loops
    unsigned int cEndgame;
    bool noMoreHumans() const;

    void disconnectCard(const Card::Widget& card);
    void disconnectCardInHand(const Card::Widget& card);
    void disconnectCardOnTable(const Card::Widget& card);

    // Drop targets (accepting drops onto a card) and drag sources (dragging
    // from a card); GTK4 replaces the whole GTK3 DND API with per-widget
    // event controllers which must be kept alive/removable.
    std::map<const Card::Widget*, Glib::RefPtr<Gtk::DropTarget>> aTableDND;
    std::map<const Card::Widget*, Glib::RefPtr<Gtk::DropTarget>> aHandDND;
    std::map<const Card::Widget*, Glib::RefPtr<Gtk::DragSource>> aHandData;
    std::map<const Card::Widget*, Glib::RefPtr<Gtk::DragSource>> aTableData;

    static Card::Widget::NUMBERS cardNuke;
    static Card::Widget::NUMBERS cardSkip;
    static Card::Widget::NUMBERS cardReverse;
};

#endif
