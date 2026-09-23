#ifndef PLAYERDLG_H
#define PLAYERDLG_H

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

#include <memory>
#include <vector>

#include <XGP/XDialog.h>

namespace Gtk {
class Label;
class Entry;
class Grid;
} // namespace Gtk

namespace Card {
class Player;
}

namespace Card {

// Class to enter the names of the players
class PlayerDlg : public XGP::XDialog {
  public:
    explicit PlayerDlg(std::vector<Player*>& player);
    ~PlayerDlg() override;

    /// Creates a new player-dialogue; showing the passed players. Any
    /// changes to the names are also reflected into the passed names
    /// \param player Vector holding name of all players
    /// \returns PlayerDlg* Pointer to the created dialoge
    static PlayerDlg* create(std::vector<Player*>& player) {
        PlayerDlg* dlg(new PlayerDlg(player));
        dlg->signal_response().connect(mem_fun(*dlg, &PlayerDlg::free));
        return dlg;
    }

    /// Signal emitted, when OK clicked
    sigc::signal<void()> sigCommit;

  protected:
    void okEvent() override; ///< Callback after clicking OK

  private:
    /// \name Prohibited manager functions
    //@{
    PlayerDlg(const PlayerDlg& other) = delete;
    const PlayerDlg& operator=(const PlayerDlg& other) = delete;
    //@}

    std::unique_ptr<Gtk::Grid> pClient;

    struct line {
        Gtk::Label* label;
        Gtk::Entry* value;

        line(const Glib::ustring& label, const Glib::ustring& attribute);
        ~line();

        void attach(Gtk::Grid& table, unsigned int line);

      private:
        line(const line&) = delete;
        line& operator=(const line&) = delete;
    };

    std::vector<std::unique_ptr<line>> aPlayers;
    std::vector<Player*>& values;
};

} // namespace Card

#endif
