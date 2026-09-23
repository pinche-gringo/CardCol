#ifndef BURACO_H
#define BURACO_H

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
#include <string>
#include <vector>

#include <gtkmm/frame.h>
#include <gtkmm/label.h>
#include <gtkmm/scrolledwindow.h>

#include <card/Pile.h>
#include <card/Set.h>
#include <card/Widget.h>

#include "BuracoPile.h"

#include <card/Game.h>

namespace Gtk {
class DragSource;
class DropTarget;
} // namespace Gtk
namespace Gdk {
class ContentProvider;
}
namespace Gio {
class SimpleAction;
}
namespace Glib {
class ValueBase;
}

namespace Card {
class ScoreDlg;
}

/**Class handling the Buraco cardgame
 */
class Buraco : public Card::Game {
    friend class Settings;
    friend class CardgameAppl;
    friend class CardgameCollection;

  public:
    Buraco(Gtk::Box& parent, Gtk::Statusbar& statusbar, Card::Set& cardset, const std::vector<Card::Player*>& player,
           unsigned int posPlayer, Card::MessageLock& mxSerialize);
    ~Buraco() override;

    void start() override;
    void clean() override;
    const char* name() override { return "Buraco"; }
    void playOpen(bool) override;
    void addMenus(const Glib::RefPtr<Gio::Menu>& menu, const Glib::RefPtr<Gio::SimpleActionGroup>& actions) override;
    void removeMenus(const Glib::RefPtr<Gio::Menu>& menu, const Glib::RefPtr<Gio::SimpleActionGroup>& actions) override;

    void changeNames(const std::vector<Card::Player*>& newPlayer) override;
    void resizeCards() override;

    unsigned int numberOfDecks() const override { return 4; }
    unsigned int numberOfJokers() const override { return 3; }

    bool handleMessage(unsigned int player, const std::string& msg) override;

    static unsigned int getPoints(const Card::Widget& card);
    static bool isJoker(const Card::Widget& card);
    static int cardDistance(const Card::Widget& a, const Card::Widget& b);
    static int cardDistance(const Card::Widget& a, const Card::Widget& b, bool aceIsOne);

  private:
    Buraco(const Buraco& other) = delete;
    const Buraco& operator=(const Buraco& other) = delete;

    static constexpr unsigned int NUM_PLAYERS = 4;              // Number of players
    static constexpr unsigned int NUM_TEAMS = NUM_PLAYERS >> 1; // Number of teams

    //@Section Virtual methods
    void makeMove(unsigned int player) override;
    bool enableHuman() override;
    void disableHuman() override;

    Card::IPile* getPileOfPlayer(unsigned int player, unsigned int pile) override;
    unsigned int getActTarget() const override;
    void endTurn(unsigned int player, unsigned int card2Dump);
    void turnEnded();

#ifdef WITH_NETWORK
    //@Section network handling
    unsigned int getRemotePlayer(unsigned int sender) const;
    bool playRemoteCards(unsigned int sender, const std::string& message);
    bool moveRemoteCard(unsigned int sender, const std::string& message);
    bool undoRemoteMove(unsigned int sender);
    void startRemoteTurn(unsigned int player);
    void remoteMoveDone(unsigned int pile);
#endif

    //@Section Event handling
    void cardSelected(unsigned int iCard);
    void dumpedSelected();
    void doDelayedDumpedSelected();
    void doDumpedSelected();
    void stapleSelected();
    void doStapleSelected();
    bool doRegisterHand(unsigned int first, unsigned int last);

    void undoMove();
    void undoLast(unsigned int player);
    void sortHand();
    void sortHandByColour();

    //@Section helper methods
    void addBuraco4HumanAndEnable();
    void enableHumanHand();
    void enableCard(unsigned int pos);
    static bool containsOnlyJoker(const Card::IPile& pile);
    static bool containsNoJoker(const Card::IPile& pile);
    static bool showJoker(Card::IPile* pile, unsigned int cJokers, bool show);
    void addBuraco(unsigned int player);
    void playCards();
    int executeMove(unsigned int player, unsigned int& pos1Play, unsigned int& pos2Play);
    void endGame();
    bool canClosePile(unsigned int player, unsigned int pile) const;
    bool canGetRidOfCards(unsigned int player) const;
    bool canPlayCards(unsigned int player, unsigned int cards, unsigned int pile = -1U) const;

    void sendMoveCard(unsigned int pile, unsigned int from, unsigned int to) const;
    static bool pileHasFittingPair(const Card::IPile& pile, const Card::Widget& card, bool withJokers = false);
    static bool pileHasFittingPair(const Card::IPile& pile, const Card::Widget* exclude = nullptr);
    static bool compByNumberWithJokers(const Card::Widget* a, const Card::Widget* b);
    static bool compByColourWithJokers(const Card::Widget* a, const Card::Widget* b);
    void makeTeamNames(std::vector<Card::Player*>& names) const;
    void setStartPlayer();
    bool cleanup();

    //@Section to handle piles on table
    BuracoPile& makeNewPile(unsigned int team);
    unsigned int cardFitsOnPlayedPile(unsigned int player, unsigned int card);
    int cardFitsOnPile(unsigned int pile, const Card::Widget& card) const;
    void removeCerrado(unsigned int player, BuracoPile& pile);
    void cleanCerrado(unsigned int player);
    void updateInfo();
    bool humanPilesOK(unsigned int except = -1U) const;

    //@Section DND
    void registerTableDND(unsigned int pile, unsigned int start, unsigned int end);
    void registerTableDND(Card::Widget& card, unsigned int nr);
    void unregisterTableDND(Card::Widget& card);
    void registerHandDND(unsigned int start, unsigned int end);
    void registerHandDND(unsigned int iCard);
    void unregisterHandDND(Card::Widget& card);
    Glib::RefPtr<Gdk::ContentProvider> prepareHandDrag(double x, double y, unsigned int cardPos);
    bool cardDropped(const Glib::ValueBase& value, double x, double y, unsigned int card);
    bool cardDroppedOnTable(const Glib::ValueBase& value, double x, double y, unsigned int cardPile);

    std::array<Gtk::Label, NUM_PLAYERS> names;                                  // Names of the player
    std::array<Card::HPile, NUM_PLAYERS> hands;                                 // For all players: Cards in hand
    std::array<std::vector<std::unique_ptr<BuracoPile>>, NUM_TEAMS> tablePiles; // Piles on table
    std::array<std::vector<Card::Widget*>, NUM_TEAMS> reserve;                  // New staple 4 teams
    std::array<int, NUM_TEAMS> points;                                          // Number of points/team
    std::array<unsigned int, NUM_TEAMS> unfinishedMonoPiles;

    std::vector<Card::Player*> nameTeams;
    unsigned int startPlayer;

    Gtk::Label info;
    std::array<Card::HBox, NUM_TEAMS> boxTeam;

    Gtk::Label newPile;
    Card::VInfoPile staple;
    Card::VInfoPile dumped;
    sigc::connection dumpedTop;
    sigc::connection stapleTop;

    struct CONNECTIONS {
        Glib::RefPtr<Gtk::DragSource> drag;
        Glib::RefPtr<Gtk::DropTarget> drop;
    };
    std::map<Card::Widget*, CONNECTIONS> aDNDHand;
    std::map<Card::Widget*, Glib::RefPtr<Gtk::DropTarget>> aDNDTable;

    struct {
        unsigned int startGame : 1;
        unsigned int startTurn : 1;
        unsigned int team1Buraco : 2;
        unsigned int team2Buraco : 2;
        unsigned int pickUpPlayed : 1;
    } gStatus;

    struct undoValue {
        unsigned int destPile : 8;
        unsigned int destPos : 3;
        unsigned int srcPos : 7;
        unsigned int pickUp : 1;
        unsigned int monoPos : 3;

        void assign(unsigned int targetPile, unsigned int targetPos, unsigned int pos) {
            destPile = targetPile;
            destPos = targetPos;
            srcPos = pos;
            monoPos = 7;
            pickUp = 0;
        }
    };
    undoValue undo;

    Card::ScoreDlg* pScoreDlg;

    int idxMenu;
    Glib::RefPtr<Gio::SimpleAction> menuUndo;
    Glib::RefPtr<Gio::SimpleAction> menuSort;
    Glib::RefPtr<Gio::SimpleAction> menuSort2;
    Glib::RefPtr<Gio::SimpleAction> menuShowScoreDlg;

    unsigned int target; ///< Id identifying the target to play to

#ifdef WITH_NETWORK
    unsigned int movedPile{-1U}; ///< Pile in which a remote player moved a joker (for undo; -1U: none)
    unsigned int movedFrom{0};   ///< Position from which the joker was moved
#endif

    // Scroll-ctrls for table (holding boxTeam). Declared last, so they are
    // destroyed first (before the widgets they contain)
    std::array<Gtk::ScrolledWindow, NUM_TEAMS> scrlTable;

    static unsigned int ENDPOINTS;
    static unsigned int CARDS2DEAL;
};

#endif
