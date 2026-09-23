#ifndef SGTMAYOR_H
#define SGTMAYOR_H

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
#include <bitset>
#include <memory>
#include <string>
#include <vector>

#include <gtkmm/label.h>

#include <card/Pile.h>
#include <card/Set.h>

#include <card/Game.h>

namespace Card {
class ScoreDlg;
struct MessageField;
} // namespace Card
namespace Gio {
class SimpleAction;
}

// Class to handle the Hearts cardgame
class SgtMayor : public Card::Game {
    friend class Settings;
    friend class CardgameAppl;
    friend class CardgameCollection;

  public:
    SgtMayor(Gtk::Box& parent, Gtk::Statusbar& statusbar, Card::Set& cardset, const std::vector<Card::Player*>& player,
             unsigned int posPlayer, Card::MessageLock& mxSerialize);
    ~SgtMayor() override;

    void start() override;
    void clean() override;
    void playOpen(bool open) override;
    const char* name() override { return "Sgt. Mayor"; }
    void changeNames(const std::vector<Card::Player*>& newPlayer) override;
    void resizeCards() override;

#ifdef WITH_NETWORK
    bool handleMessage(unsigned int player, const std::string& message) override;
#endif

  protected:
    Card::IPile* getPileOfPlayer(unsigned int player, unsigned int pile) override;

  private:
    // Protected manager functions
    SgtMayor(const SgtMayor& other) = delete;
    SgtMayor& operator=(const SgtMayor& other) = delete;

    //@Section Event handling
    void cardSelected(unsigned int iCard);
    void cardExchange(unsigned int iCard);
    void cardColourSelect(unsigned int iCard);

    //@Section Virtual methods
    void makeMove(unsigned int player) override;
    bool enableHuman() override;

    //@Section Helper methods
    bool selectTrump();
    void exchangeCards(unsigned int playerBad, unsigned int posBad, unsigned int playerGood);
    void exchangeCards(unsigned int playerBad, unsigned int posBad, unsigned int playerGood, unsigned int posGood);
    void exchangeCards(unsigned int playerBad, unsigned int playerGood);
    void exchange(unsigned int playerBad, unsigned int posBad, unsigned int playerGood, unsigned int posGood);
    void exchgBack(unsigned int playerBad, unsigned int playerGood, unsigned int posGood);
    void exchgNext(Card::HPile* pileGood, Card::HPile* pileBad);
    void showExchangedCard(Card::IPile& pile, unsigned int& pos);
    void doExchangeCards(unsigned int playerBad, unsigned int posBad, unsigned int playerGood, unsigned int posGood);
    void showNeededTricks();
    void showTrump(Card::Widget::COLOURS);
    void doShowTrump(Card::Widget::COLOURS);
    void makeExchange();
    void displayExchangeStatus();
    void startPlaying();
    void playCardDelayed(unsigned int player);
    unsigned int playCard(unsigned int player);
    static unsigned int calcNextPlayer(unsigned int player) { return (++player >= NUM_PLAYERS) ? 0 : player; }
    /// Converts the position of a player in the game (0 .. NUM_PLAYERS - 1;
    /// 0 is the own position) into its index in the vector of players (which
    /// might hold more players than the game needs; as seen from the server the
    /// vector is rotated by its length, the game by NUM_PLAYERS)
    /// \param player Position of player in the game
    /// \returns unsigned int Index of the player in actPlayers
    unsigned int convertPlayer(unsigned int player) const {
        const unsigned int all(actPlayers.size());
        return (((player + posServer) % NUM_PLAYERS) + all - posServer) % all;
    }
    /// Converts the index of a player in the vector of players into its position
    /// in the game (the reverse of convertPlayer)
    /// \param player Index of the player in actPlayers
    /// \returns unsigned int Position of player in the game
    unsigned int localPlayer(unsigned int player) const {
        return (((player + posServer) % actPlayers.size()) + NUM_PLAYERS - posServer) % NUM_PLAYERS;
    }
    static std::string formatNumber(int nr);
#ifdef WITH_NETWORK
    static bool readCardInfo(const Card::MessageField& card, const Card::MessageField& from, unsigned long& idCard,
                             unsigned long& player);
    void handleDeferredMessage();
#endif

    //@Section Computer player
    unsigned int findPos2Play(unsigned int player);
    bool isHighest(const Card::Widget& card) const;
    unsigned int tryToGetTrickWithTrump(const Card::IPile& pile) const;

    void addMenus(const Glib::RefPtr<Gio::Menu>& menu, const Glib::RefPtr<Gio::SimpleActionGroup>& actions) override;
    void removeMenus(const Glib::RefPtr<Gio::Menu>& menu, const Glib::RefPtr<Gio::SimpleActionGroup>& actions) override;
    void showWonCards(bool show = true, unsigned int style = -1U) override;

    static constexpr unsigned int NUM_PLAYERS = 3; // Number of players

    struct playerCards {
        Card::HPile hand; // For players: Cards in the hand
        Card::HPile won;  // Won tricks
        Gtk::Label name;
        Gtk::Label neededTricks;

        playerCards() : hand(), won(), name(), neededTricks() {}

      private:
        playerCards(const playerCards&) = delete;
        playerCards& operator=(const playerCards&) = delete;
    };
    std::array<playerCards, NUM_PLAYERS> players;
    Card::HPile played;
    std::unique_ptr<Card::Widget> pTrump;

    unsigned int bfColours;

    unsigned int startPlayer;
    std::array<std::bitset<13>, 4> playedCards;
    std::array<int, NUM_PLAYERS> diffTricks;

    Glib::RefPtr<Gio::SimpleAction> menuSort;
    Glib::RefPtr<Gio::SimpleAction> menuSort2;
    Glib::RefPtr<Gio::SimpleAction> menuShowScoreDlg;

    std::unique_ptr<Card::ScoreDlg> pScoreDlg;

#ifdef WITH_NETWORK
    bool exchanging{false};         ///< Flag, if the (animated) exchange of cards is running
    bool remoteExchange{false};     ///< Flag, if the message token is kept for a received exchange
    std::string deferredMsg;        ///< Message received while exchanging cards
    unsigned int deferredSender{0}; ///< Sender of the deferred message
#endif

    static constexpr std::array<unsigned int, NUM_PLAYERS> COLS_PLAYER{1, 4, 0};
    static constexpr std::array<unsigned int, NUM_PLAYERS> ROWS_PLAYER{6, 1, 1};

    static unsigned int ENDTRICKS;
};

#endif
