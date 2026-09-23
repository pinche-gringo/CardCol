#ifndef MACHIAVELLI_H
#define MACHIAVELLI_H

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
#include <memory>
#include <stack>
#include <utility>
#include <vector>

#include <gtkmm/button.h>
#include <gtkmm/dragsource.h>
#include <gtkmm/droptarget.h>
#include <gtkmm/label.h>
#include <gtkmm/scrolledwindow.h>

#include <giomm/simpleaction.h>

#include <XGP/AutoContainer.h>

#include <card/Game.h>
#include <card/Widget.h>
#include <card/Window.h>

#include "MachiPile.h"

// Forward declarations
namespace YGP {
class CardSet;
class StatusObject;
} // namespace YGP
namespace XGP {
class MessageDlg;
}
namespace Gtk {
class Statusbar;
}

/**Class handling the Machiavelli cardgame
 */
class Machiavelli : public Card::Game {
  public:
    Machiavelli(Gtk::Box& parent, Gtk::Statusbar& statusbar, Card::Set& cardset, const std::vector<Card::Player*>& player,
                unsigned int posPlayer, Card::MessageLock& mxSerialize);
    ~Machiavelli() override;

    void start() override;
    void clean() override;
    const char* name() override { return "Machiavelli"; }
    void playOpen(bool) override;

    void addMenus(const Glib::RefPtr<Gio::Menu>& menu, const Glib::RefPtr<Gio::SimpleActionGroup>& actions) override;
    void removeMenus(const Glib::RefPtr<Gio::Menu>& menu, const Glib::RefPtr<Gio::SimpleActionGroup>& actions) override;

    unsigned int numberOfDecks() const override { return 4; }
    void resizeCards() override;

    bool handleMessage(unsigned int player, const std::string& msg) override;

  private:
    Machiavelli() = delete;
    Machiavelli(const Machiavelli& other) = delete;
    const Machiavelli& operator=(const Machiavelli& other) = delete;

    static constexpr unsigned int NUM_PLAYERS = 4; // Number of players

    /// \name Virtual methods
    //@{
    void makeMove(unsigned int player) override;
    bool enableHuman() override;
    void disableHuman() override;
    void changeNames(const std::vector<Card::Player*>& newPlayer) override;

    Card::IPile* getPileOfPlayer(unsigned int player, unsigned int pile) override;
    unsigned int getActTarget() const override;
    //@}

    /// \name Helper methods
    //@{
    bool setStartPlayer();
    unsigned int findNextPlayer(unsigned int player) const;
    MachiPile& makeNewPile();
    MachiPile& makeNewPile(unsigned int pos);
    void removePile(unsigned int pile);
    bool showCardsToPlay(unsigned int player);
    bool dealCard(unsigned int player);
    void addTableMove(unsigned int pile, unsigned int first, unsigned int nr, unsigned int destPile, unsigned int destPos);
    void checkPiles(YGP::StatusObject& obj, bool mark = false) const;
    void endGame(unsigned int looser);
    bool playSerie(Card::IPile& playerPile);
    bool cardFitsOnPile(const Card::Widget& card, unsigned int offset);
    bool reorderTableToFit(Card::IPile& playerPile);
    bool reorderTableToFit2(Card::IPile& playerPile);
    bool reorderTableToFit3(Card::IPile& playerPile);
    bool reorderTableToFit4();
    void addBorderCards2Missing(unsigned int iPile, unsigned int which = -1U);
    //@}

    /// \name Drag-and-drop methods
    //@{
    void registerTableDND(unsigned int pile, unsigned int start, unsigned int end);
    void registerTableDND(Card::Widget& card, unsigned int nr);
    void unregisterTableDND(Card::Widget& card);
    void unregisterTableDND();
    void registerHandDND(unsigned int start, unsigned int end);
    void registerHandDND(unsigned int iCard);
    void unregisterHandDND(Card::Widget& card);

    bool cardDropped(const Glib::ValueBase& value, unsigned int card);
    bool cardDroppedOnTable(const Glib::ValueBase& value, unsigned int cardPile);

    bool doRegisterHand(unsigned int first, unsigned int last);
    //@}

    /// \name Callback from events
    //@{
    void unmarkAndEnd(MachiPile* pile);
    void endTurn();
    void doEndTurn();
    void undoMove(unsigned int number);
    void removeUndoDlg(int);
    void sortHand();
    void sortHandByColour();
    //@}

    void endComputerMove();

#ifdef WITH_NETWORK
    /// \name Execution of received moves
    //@{
    void playRemoteCards(unsigned int player);
    bool reorderRemote(const std::string& message);
    void endRemoteReorder(MachiPile* src);
    void moveRemote(const std::string& message);
    //@}
#endif

    std::array<Gtk::Label, NUM_PLAYERS> names;  // Names of the player
    std::array<Card::HPile, NUM_PLAYERS> hands; // For all players: Cards in hand

    XGP::AutoContainer piles;                           // Piles on the table
    std::vector<std::unique_ptr<MachiPile>> tablePiles; // Piles on table (owned); for faster access

    unsigned int startPlayer;

    Gtk::Label newPile;
    Glib::RefPtr<Gtk::DropTarget> dstNewPile;
    Card::VInfoPile staple;
    Gtk::Button nextTurn;

    struct CONNECTIONS {
        Glib::RefPtr<Gtk::DragSource> src;
        Glib::RefPtr<Gtk::DropTarget> dst;
    };
    std::map<Card::Widget*, CONNECTIONS> aDNDHand;
    std::map<Card::Widget*, CONNECTIONS> aDNDTable;

    unsigned int target; // Target ((pile << 16) + position) of the cards to play (from a hand)

    /// Cards the computer player moves between the piles on the table during
    /// its actual move (the first value describes the source like
    /// (number of cards << 16) + (pile << 8) + first card, the second the
    /// target like (pile << 16) + position); they are sent to the clients
    /// (as Reorder messages) after the cards played from the hand
    std::vector<std::pair<unsigned int, unsigned int>> posPiles;

    // Structure holding undo-information
    struct undoValue {
        unsigned int destPos : 8;
        unsigned int srcPos : 8;
        unsigned int destPile : 8;
        unsigned int srcPile : 8;
        unsigned int number : 4;
        unsigned int create : 1;

        undoValue(unsigned int targetPile, unsigned int targetPos, unsigned int pile, unsigned int pos, unsigned int nr)
            : destPos(targetPos), srcPos(pos), destPile(targetPile), srcPile(pile), number(nr), create(false) {}
        undoValue(unsigned int targetPile, unsigned int targetPos, unsigned int pile, unsigned int pos, unsigned int nr,
                  bool createPile)
            : destPos(targetPos), srcPos(pos), destPile(targetPile), srcPile(pile), number(nr), create(createPile) {}
    };

    std::stack<undoValue> undo;

    // Structure to store which cards are missing on a pile, to be able to add from hand
    struct missingCards {
        unsigned int pile;
        Card::Widget::NUMBERS nr;
        Card::Widget::COLOURS colour;

        missingCards(unsigned int pile, Card::Widget::NUMBERS nr, Card::Widget::COLOURS colour)
            : pile(pile), nr(nr), colour(colour) {}
        explicit missingCards(unsigned int pile) : pile(pile), nr(Card::Widget::UNREACHABLE), colour() {}
    };
    std::vector<missingCards> missing;

    std::unique_ptr<XGP::MessageDlg> undoDlg;

    Glib::RefPtr<Gio::SimpleAction> undo1;
    Glib::RefPtr<Gio::SimpleAction> undoAll;
    Glib::RefPtr<Gio::SimpleAction> nxtTurn;
};

#endif
