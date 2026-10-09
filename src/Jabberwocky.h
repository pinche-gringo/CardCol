#ifndef JABBERWOCKY_H
#define JABBERWOCKY_H

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
#include <optional>
#include <vector>

#include <gtkmm/label.h>

#include <YGP/ANumeric.h>

#include <card/Pile.h>
#include <card/Set.h>

#include <card/Game.h>

#include "JabberwockyRules.h"

namespace Card {
class ScoreDlg;
}
namespace Gtk {
class Button;
class SpinButton;
} // namespace Gtk
namespace Gio {
class Menu;
class SimpleAction;
class SimpleActionGroup;
} // namespace Gio

/**Class to handle the Jabberwocky card game
 */
class Jabberwocky : public Card::Game {
  public:
    Jabberwocky(Gtk::Box& parent, Gtk::Statusbar& statusbar, Card::Set& cardset, const std::vector<Card::Player*>& player,
                unsigned int posPlayer, Card::MessageLock& mxSerialize);
    ~Jabberwocky() override;

    void start() override;
    void clean() override;
    void playOpen(bool open) override;
    const char* name() override { return "Jabberwocky"; }
    void changeNames(const std::vector<Card::Player*>& newPlayer) override;
    void resizeCards() override;

    bool handleMessage(unsigned int player, const std::string& message) override;

  protected:
    Card::IPile* getPileOfPlayer(unsigned int player, unsigned int pile) override;

  private:
    Jabberwocky() = delete;
    Jabberwocky(const Jabberwocky& other) = delete;

    Jabberwocky& operator=(const Jabberwocky& other) = delete;

    //@Section Virtual methods
    void makeMove(unsigned int player) override;
    bool enableHuman() override;
    void addMenus(const Glib::RefPtr<Gio::Menu>& menu, const Glib::RefPtr<Gio::SimpleActionGroup>& actions) override;
    void removeMenus(const Glib::RefPtr<Gio::Menu>& menu, const Glib::RefPtr<Gio::SimpleActionGroup>& actions) override;

    //@Section helper methods
    void cardSelected(unsigned int pos);
    void makeBids(unsigned int start = 0);
    void startGame();
    void placedBid(Gtk::SpinButton* value, Gtk::Button* commit, unsigned int start);
    void showBid(unsigned int player);

    void showCards2Play(unsigned int player);
    int playCard(unsigned int player);
    void finishMove();
    unsigned int sumOtherBids(unsigned int player) const;
    void takeWonCards(unsigned int player);
    JabberwockyRules::Table& currentTable();

    static Card::Widget::COLOURS trumpColour; ///< Colour of the trumps (for sorting)
    static bool compByColourAccTrumps(const Card::Widget* a, const Card::Widget* b);

    static constexpr unsigned int NUM_PLAYERS = JabberwockyRules::NUM_PLAYERS; // Number of players

    struct playerCards {
        Card::HPile hand; // For players: Cards in the hand
        Card::HPile won;  // Won ticks
        Gtk::Label name;
        YGP::ANumeric bid;

        playerCards() : hand(), won(), name(), bid() {}

      private:
        playerCards(const playerCards&) = delete;
        playerCards& operator=(const playerCards&) = delete;
    };
    std::array<playerCards, NUM_PLAYERS> players;
    Card::HPile played;
    Card::Widget* pTrump;

    unsigned int startPlayer;
    unsigned int turn;
    int idxMenu; ///< Index of this game's submenu-item within the passed Gio::Menu

    std::unique_ptr<Gtk::SpinButton> pBidValue; ///< Widget to enter the human's bid (while active); else NULL
    std::unique_ptr<Gtk::Button> pBidCommit;    ///< Button to commit the human's bid (while active); else NULL

    /// State of the round (played cards, ...); needed by the rules and the computer player
    std::optional<JabberwockyRules::Table> table;

    std::unique_ptr<Card::ScoreDlg> pScoreDlg;

    Glib::RefPtr<Gio::SimpleAction> menuSort;
    Glib::RefPtr<Gio::SimpleAction> menuSort2;
    Glib::RefPtr<Gio::SimpleAction> menuShowScoreDlg;

    static constexpr std::array<unsigned int, NUM_PLAYERS> COLS_PLAYER{7, 13, 7, 1};
    static constexpr std::array<unsigned int, NUM_PLAYERS> ROWS_PLAYER{10, 8, 4, 8};
};

#endif
