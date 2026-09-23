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

#include <vector>

#include <gtkmm/button.h>
#include <gtkmm/statusbar.h>

#include <YGP/ConnMgr.h>
#include <YGP/Exception.h>
#include <YGP/Mutex.h>
#include <YGP/Thread.h>

#include <card/Images.h>
#include <card/Pile.h>
#include <card/Set.h>

#include <XGP/XApplication.h>

namespace Gtk {
class Dialog;
}
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
    CardgameCollection(Options& opts);
    ~CardgameCollection();

    Gtk::Box& getClient() { return *XGP::XApplication::getClient(); }
    Gtk::Statusbar& getStatusbar() { return status; }
    Card::Set& getCards() { return cards; }
    const std::vector<Card::Player*>& getPlayer() const { return aPlayer; }
    YGP::ConnectionMgr& getConnectionMgr() { return cmgr; };
    YGP::Mutex& getClientMutex() { return mxThreadCmd; }
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
    Glib::RefPtr<Gio::SimpleAction> apMenus[LAST];
    Glib::RefPtr<Gio::SimpleAction> actChgGame; ///< Radio-action selecting the active game

    Glib::RefPtr<Gio::Menu> menuGameSection;          ///< Section filled by the active game's addMenus()
    Glib::RefPtr<Gio::SimpleActionGroup> actionsGame; ///< Actions used by the active game's menu

    // Protected manager functions
    CardgameCollection(const CardgameCollection&);
    const CardgameCollection& operator=(const CardgameCollection&);

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
    int handleGlobalMessage(unsigned int player, const std::string& msg) throw(YGP::ParseError);
    bool handleMessage(unsigned int player, const std::string msg);
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

    bool showMessage(const std::string msg);
    static void closeDialog(int, const Gtk::Dialog* dlg);
    virtual void gameEvents(unsigned int status);
    virtual void showAboutbox();
    virtual const char* getHelpfile();

    void* changeCards(void* opt);
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

    static const char* xpmGame[];
    static const char* xpmAuthor[];

    Gtk::Statusbar status;
    Gtk::Box filler; ///< Placeholder pushing the statusbar to the bottom until a game exists

    Card::Images cardFaces;
    Card::Set cards;

#ifdef WITH_NETWORK
    typedef YGP::OThread<CardgameCollection> THRDAPPL;
    std::vector<THRDAPPL*> aCommThreads;
    YGP::Mutex mxGuiCmd;

    ChatDlg* dlgChat;
#endif
    YGP::Mutex mxThreadCmd;
    YGP::ConnectionMgr cmgr;
    unsigned int playerPos;

    Options& options;
    std::vector<Card::Player*> aPlayer;

    int oldGame, actGame;
    unsigned int restart;

    Card::Game* game;

    static int POSX;
    static int POSY;
    static unsigned int WIDTH;
    static unsigned int HEIGHT;
};

#endif
