#ifndef GAME_H
#define GAME_H

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

#include <cardgames-cfg.h>

#include <memory>
#include <deque>
#include <string>
#include <vector>

#include <gtkmm/grid.h>

#include <YGP/Exception.h>

#include <card/MessageLock.h>
#include <card/Pile.h>

// Forward declarations
namespace Gtk {
class Box;
class Dialog;
class Statusbar;
class PopoverMenu;
} // namespace Gtk
namespace Gio {
class Menu;
class SimpleActionGroup;
} // namespace Gio
namespace YGP {
class ConnectionMgr;
} // namespace YGP
namespace Card {
class Set;
class Player;
class Widget;
class Window;
class PileWindow;
class PileWindows;
} // namespace Card

namespace Card {

/**Abstract base class providing usefull methods for card games.
 */
class Game : public Gtk::Grid {
  public:
    /// Stati of the game
    enum {
        NONE = 0,     ///< Class created; game not started
        INITIALIZING, ///< Initialization phase (dealing cards, ...)
        STOPPED,      ///< Game has been ended
        TOSTOP,       ///< Game should be ended (but can't be at the moment)
        PLAYING,      ///< Game is being played
        LAST
    };

    Game(Gtk::Box& parent, Gtk::Statusbar& statusbar, Set& cardset, const std::vector<Player*>& player, unsigned int posPlayer,
         MessageLock& mxSerialize, unsigned int rows, unsigned int columns);
    ~Game() override;

    /// \name Managing
    //@{
    virtual void start();
    virtual void stop();
    virtual void end(bool startNew);
    virtual void playOpen(bool) {}
    /// Informs the parent about status changes
    virtual void control(unsigned int status) const;
    virtual YGP::ConnectionMgr& getConnectionMgr() const = 0;
    virtual void clean();
    virtual const char* name() = 0;
    virtual void changeNames(const std::vector<Player*>& newPlayer);
    virtual void addMenus(const Glib::RefPtr<Gio::Menu>& menu, const Glib::RefPtr<Gio::SimpleActionGroup>& actions);
    virtual void removeMenus(const Glib::RefPtr<Gio::Menu>& menu, const Glib::RefPtr<Gio::SimpleActionGroup>& actions);
    virtual void resizeCards();
    //@}

    /// \name Carddeck information
    //@{
    virtual unsigned int numberOfDecks() const { return 1; }
    virtual unsigned int numberOfJokers() const { return 0; }
    //@}

    virtual bool handleMessage(unsigned int player, const std::string& msg);
    bool ignoreMessage(const std::string& msg);

    /// \name Status handling
    //@{
    /// Checks if the game is being played
    /// \returns bool True, if the game is running (i.e. can't be interrupted at
    /// the moment)
    bool isRunning() const { return statGame >= PLAYING; }
    /// Checks if the game can be stopped at the moment (only when it's the
    /// turn of the human)
    virtual bool canBeStopped() const;

    /// Handling the actual game status
    /// \returns unsigned int Status of the game
    unsigned int gameStatus() const { return statGame; }
    void setGameStatus(unsigned int newStatus);
    bool isShowingCardsToPlay() const { return pos1Play != -1U; }
    //@}

    /// Sets the position of the game/player for a network game; this
    /// position is relative to the server (i.e. the player inviting
    /// the (other) players and starting the game
    /// \param posPlayer Position of player as seen from the server
    void setPlayerPosition(unsigned int posPlayer) { posServer = posPlayer; }

    virtual void disableHuman();
    /// Sets the internal status of the game to turn ended
    void endTurn() { stati.pendingTurn = 0; }

    /// \name Player actions
    //@{
    virtual bool enableHuman();
    bool makeComputerMove();
    //@}

    /// \name Methods to make the game load/save before dealing
    //@{
    void setCardOrder(const char* order) {
        cardOrder = order;
        data = cardOrder.data();
    }
    const char* getCardOrder() const { return cardOrder.data(); }
    void clearCardOrder() {
        cardOrder.clear();
        data = nullptr;
    }
    //@}

  protected:
    virtual IPile* getPileOfPlayer(unsigned int player, unsigned int pile) = 0;
    virtual bool executeRemoteMove(IPile& pile, unsigned int target);
    virtual unsigned int getActTarget() const;

    /// \name Communication helper methods
    //@{
    void broadcastMessage(const std::string& msg) const;
    void sendMove(const std::string& msg) const;
    void broadcastStartPlayer(unsigned int startplayer);
    //@}

    /// Returns the current player
    unsigned int currentPlayer() const { return actPlayer; }
    /// Sets the next player
    void setNextPlayer(unsigned int player);

    void flipCards2Play(IPile& pile, unsigned int& start, unsigned int& end);
    void flipCards2Play(IPile& pile, const std::string& cards);
    void displayTurn(unsigned int player);
    void displayTurn(unsigned int player, const Glib::ustring& preText);
    void makeNextMoves();
    bool endRemoteMove(unsigned int player);
    /// Keeps the message token after the current message has been handled
    /// (the handler returns false), til the (remote) move has been finished;
    /// that is when the next player is activated or the status of the game changes
    void keepMessageLock() { stati.remoteMove = 1; }
    void releaseMessageLock();
    virtual void makeMove(unsigned int player) = 0;

    bool randomiseCardsToPile(IPile& pile) const;

    /// \name Animation
    //@{
    Window& animateCard(IPile& dest, IPile& src, unsigned int pos) { return animateCard(dest, dest.size(), src, pos); }
    Window& animateCard(IPile& dest, unsigned int posDest, IPile& src, unsigned int pos);
    PileWindow& animateCards(IPile& dest, IPile& src, unsigned int start, unsigned int end) {
        return animateCards(dest, dest.size(), src, start, end);
    }
    PileWindow& animateCards(IPile& dest, unsigned int posDest, IPile& src, unsigned int start, unsigned int end);
    PileWindows& animateCards2(IPile& dest, IPile& src, unsigned int start, unsigned int end) {
        return animateCards2(dest, dest.size(), src, start, end);
    }
    PileWindows& animateCards2(IPile& dest, unsigned int posDest, IPile& src, unsigned int start, unsigned int end);
    //@}

    bool performCommand(unsigned int player, const std::string& msg);
    static bool stringToNumber(unsigned long& number, const char* text);

    // Handling of won cards (if any)
    void wonCardsSelectedLeft();
    void wonCardsSelectedRight(double x, double y, Card::Widget& card);
    virtual void showWonCards(bool show = true, unsigned int style = -1U);
    int enableWonCards(IPile& pile) {
        pWonPile = &pile;
        return enableActWonCards();
    }
    void disableWonCards();

    void sortWonByNumber();
    void sortWonByColour();

    Gtk::Statusbar& status;
    Set& cards;

    std::vector<sigc::connection> activeCards;
    const std::vector<Player*>& actPlayers;

    MessageLock& mxSerializeMsgs;

    unsigned int posServer; ///< Position the player occupies for the server

    unsigned int pos2Play; ///< Upper border of cards to play
    unsigned int pos1Play; ///< Lower border of cards to play


  private:
    Game(const Game&) = delete;
    Game& operator=(const Game&) = delete;

    bool endGame(bool startNew);

    bool enableActWonCards();

    const char* data; // Data send from server

    unsigned int statGame;

    int actPlayer; // Player who is in turn (needed for timer)
    struct {
        unsigned int restart : 1;
        unsigned int pendingTurn : 1;
        unsigned int remoteMove : 1; ///< A received move is being executed; the message token is kept
    } stati;

    std::vector<sigc::connection> wonCards; // Connections to show won cards
    IPile* pWonPile;
    std::unique_ptr<Gtk::PopoverMenu> pMenuPopSort;

    std::string cardOrder;

    mutable std::deque<std::string> echoes; ///< Messages sent by a client, which the server echoes (and which are to be ignored)
};

/**Specialized Game to inform controler about status-changes.

   The Controller must support a statusbar (accessed by getStatusbar), a
   cardset (accessed by getCards), a Gtk::Box, which can be accessed by
   getClient (), a vector of players (accessed by getPlayer ()) and a method
   called getConnectionMgr to retrieve a ConnectionMgr object.

   \remarks Parent must be derived from Game
*/
template <class Parent, class Controller> class TGame : public Parent {
  public:
    using PCALLBACK = void (Controller::*)(unsigned int);

    /// Constructor
    /// \param controller Object controlling the game
    /// \param callback Method of object to call in case of changes of status
    TGame(Controller& controller, PCALLBACK callback)
        : Parent(controller.getClient(), controller.getStatusbar(), controller.getCards(), controller.getPlayer(),
                 controller.getPlayerPosition(), controller.getClientMutex()),
          obj(controller), pCallback(callback) {}
    /// Destructor
    ~TGame() override = default;

    /// Callback to inform a controller about status changes
    /// \param status New status of the game
    void control(unsigned int status) const override { (obj.*pCallback)(status); }

    /// Returns the connection-manager
    YGP::ConnectionMgr& getConnectionMgr() const override { return obj.getConnectionMgr(); }

  private:
    Controller& obj;
    PCALLBACK pCallback;
};

} // namespace Card

#endif
