#ifndef CARDCOL_H
#define CARDCOL_H

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

#include <array>
#include <memory>
#include <string>
#include <vector>

#include <gtkmm/button.h>
#include <gtkmm/statusbar.h>

#include <YGP/ConnMgr.h>
#include <YGP/Exception.h>
#include <YGP/Thread.h>

#include <card/Images.h>
#include <card/MessageLock.h>
#include <card/Pile.h>
#include <card/Set.h>

#include <XGP/XApplication.h>

namespace Gio {
class Menu;
class SimpleAction;
class SimpleActionGroup;
} // namespace Gio

namespace Card {
class Game;
class Player;
} // namespace Card

class Options;
class ChatDlg;

/**Class to handle the cardgame collection
 */
class CardgameCollection : public XGP::XApplication {
    friend class CardgameAppl;

  public:
    // Manager functions
    explicit CardgameCollection(Options& opts);
    ~CardgameCollection() override;

    Gtk::Box& getClient() { return *XGP::XApplication::getClient(); }
    Gtk::Statusbar& getStatusbar() { return status; }
    Card::Set& getCards() { return cards; }
    const std::vector<Card::Player*>& getPlayer() const { return aPlayer; }
    YGP::ConnectionMgr& getConnectionMgr() { return cmgr; };
    Card::MessageLock& getClientMutex() { return mxThreadCmd; }
    unsigned int getPlayerPosition() const { return playerPos; }

#ifdef WITH_ROVHULT
    static void checkRovhultSpecialCards();
#endif

  private:
    // IDs for menus
    enum {
        NEW = 0,
#ifdef WITH_NETWORK
        CONNECT,
        CHAT,
#endif
        END,
        LAST
    };
    std::array<Glib::RefPtr<Gio::SimpleAction>, LAST> apMenus;
    Glib::RefPtr<Gio::SimpleAction> actChgGame; ///< Radio-action selecting the active game

    Glib::RefPtr<Gio::Menu> menuGameSection;          ///< Section filled by the active game's addMenus()
    Glib::RefPtr<Gio::SimpleActionGroup> actionsGame; ///< Actions used by the active game's menu

    // Prohibited manager functions
    CardgameCollection(const CardgameCollection&) = delete;
    const CardgameCollection& operator=(const CardgameCollection&) = delete;

    // Event-handling
    void newGame();
    void endGame();
#ifdef WITH_NETWORK
    void autoConnect(const Options& options);
    void connect();
    void showChatDlg();
    void closeChat(int);
    bool stopClientWaiting();
    void removeCommThreads();
    void initCommunication();
    void* waitForMessages(void*);
    int handleGlobalMessage(unsigned int player, const std::string& msg);
    bool handleMessage(unsigned int player, const std::string& msg);
    void sendMessage(const Glib::ustring& msg);
    void broadcastMsg(const std::string& msg, unsigned int exclude = -1U);
    void broadcastNames();
    void sendSettings();
#endif
    void exit();
    void changeGame(int game);
    void showChangeDeckDlg();
    void changeNames();
    void editPreferences();
    void savePreferences();
#if TRACELEVEL >= 0
    void toggleDebug();
#endif

    static bool showMessage(const std::string& msg);
    virtual void gameEvents(unsigned int status);
    void showAboutbox() override;
    const char* getHelpfile() override;

    bool changeCards(unsigned int what);
    void loadCards();
    void resizeCards();
    bool restartGame();
    void startGame();
    void doStartGame();

    bool terminateGameAndExit();
    bool wait4EndGameAndExit();

    void changeDecks(const std::string& deck, const std::string& back);
    void changePlayernames();
    void makePlayer();

    Gtk::Box boxStatus; ///< Row holding the statusbar and game specific status information
    Gtk::Statusbar status;
    Gtk::Box filler; ///< Placeholder pushing the statusbar to the bottom until a game exists

    Card::Images cardFaces;
    Card::Set cards;

#ifdef WITH_NETWORK
    using THRDAPPL = YGP::OThread<CardgameCollection>;
    std::vector<THRDAPPL*> aCommThreads;

    ChatDlg* dlgChat;
#endif
    Card::MessageLock mxThreadCmd; ///< Serialises the processing of received messages
    YGP::ConnectionMgr cmgr;
    unsigned int playerPos;

    Options& options;
    std::vector<Card::Player*> aPlayer;

    int oldGame, actGame;
    unsigned int restart;

    std::unique_ptr<Card::Game> game;
    std::string helpFile; ///< Name of the helpfile (returned by getHelpfile())

    static int POSX;
    static int POSY;
    static unsigned int WIDTH;
    static unsigned int HEIGHT;
};

#endif
