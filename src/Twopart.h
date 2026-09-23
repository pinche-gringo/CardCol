#ifndef TWOPART_H
#define TWOPART_H

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

namespace Gio {
class Menu;
class SimpleActionGroup;
} // namespace Gio

/**Class to handle the Twopart-cardgame
 */
class Twopart : public Card::Game {
  public:
    // Manager functions
    Twopart(Gtk::Box& parent, Gtk::Statusbar& statusbar, Card::Set& cardset, const std::vector<Card::Player*>& players,
            unsigned int posPlayer, Card::MessageLock& mxSerialize);
    ~Twopart() override;

    void start() override;
    void clean() override;
    void playOpen(bool open) override;
    const char* name() override { return "Twopart"; }
    void changeNames(const std::vector<Card::Player*>& newPlayer) override;
    void resizeCards() override;

    bool handleMessage(unsigned int player, const std::string& msg) override;

  protected:
    Card::IPile* getPileOfPlayer(unsigned int player, unsigned int pile) override;
    bool executeRemoteMove(Card::IPile& pile, unsigned int target) override;

  private:
    // Status of game
    enum { PLAYING2 = Game::LAST };

    // Protected manager functions
    Twopart(const Twopart&) = delete;
    Twopart& operator=(const Twopart&) = delete;

    // Event-handling
    void cardSelected(unsigned int iCard);
    void playedSelected();

    // Helper functions
    void endPickup(unsigned int player);
    void endTurn(unsigned int player);
    bool enableHuman() override;
    unsigned int pickUpPlayedPile(unsigned int player);
    int findNextPlayer(unsigned int player) const;
    int findNextPlayerWithCards(unsigned int player) const;
    void removePlayer(unsigned int player) { bfPlayers &= ~(1 << player); }
    void addPlayer(unsigned int player) { bfPlayers |= 1 << player; }
    static unsigned int playersInBitfield(unsigned int bfPlayers) {
        unsigned int cPlayers(0);
        for (unsigned int i(0); i < NUM_PLAYERS; ++i)
            if (bfPlayers & (1 << i))
                ++cPlayers;
        return cPlayers;
    }
    unsigned int removePlayersWithoutCards();

    void userWants2End(unsigned int input);
    void analyzeLastPlayed(unsigned int startPos, unsigned int cards, int& max, int& maxPos, int& maxEqual, int& maxEqualPos,
                           int& trumps) const;

    unsigned int pos2Player(unsigned int pos) const;
    int findPos2Play(unsigned int player, unsigned int& start, unsigned int& end) const;
    unsigned int findSmallestCard(unsigned int player) const;
    unsigned int findEndOfSerie(unsigned int player, unsigned int start) const;
    unsigned int findStartOfSerie(unsigned int player, unsigned int start) const;
    int findBigger(const Card::IPile& pile, Card::Widget::NUMBERS nr) const;

    bool startPartTwo(unsigned int player);

    void makeMove(unsigned int player) override;
    bool endRound(unsigned int& player);

    void addMenus(const Glib::RefPtr<Gio::Menu>& menu, const Glib::RefPtr<Gio::SimpleActionGroup>& actions) override;
    void removeMenus(const Glib::RefPtr<Gio::Menu>& menu, const Glib::RefPtr<Gio::SimpleActionGroup>& actions) override;

    static std::array<char, 4> sortOrder;
    static bool compByColourAccTrumps(const Card::Widget* a, const Card::Widget* b);

    static constexpr unsigned int NUM_PLAYERS = 4; // Number of players

    unsigned int bfPlayers; // Array indicating players still in round

    // Variables to store positions during playing
    std::array<unsigned int, NUM_PLAYERS - 1> startPos; // Offset of cards played by players
    unsigned int offPos;                                // Offset in startPos-array
    unsigned int startPlayer;                           // Player who started round (needed for endRound)
    unsigned int bfOldPlayers;                          // Array indicating players while starting round

    // Columns and rows for the cards of the players
    static constexpr std::array<unsigned int, NUM_PLAYERS> COLS_PLAYER{1, 13, 7, 1};
    static constexpr std::array<unsigned int, NUM_PLAYERS> ROWS_PLAYER{10, 8, 4, 8};

    std::unique_ptr<Card::Widget> pTrump;

    Card::HInfoPile played;
    Card::VInfoPile staple; // Cards on staple
    struct playerCards {
        Card::HPile hand; // For players: Cards in the hand
        Card::HPile won;  // Reserve-cards (for end-game)
        Gtk::Label name;

        playerCards() : hand(), won(), name() {}

      private:
        playerCards(const playerCards&) = delete;
        playerCards& operator=(const playerCards&) = delete;
    };
    std::array<playerCards, NUM_PLAYERS> players;

    int idxMenu; ///< Index of this game's submenu-item within the passed Gio::Menu
};

#endif
