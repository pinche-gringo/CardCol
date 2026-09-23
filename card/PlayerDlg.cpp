// PROJECT     : Cardgames
// SUBSYSTEM   : Common
// REFERENCES  :
// TODO        :
// BUGS        :
// AUTHOR      : Markus Schwab
// CREATED     : 07.01.2003
// COPYRIGHT   : Copyright (C) 2002 - 2004, 2009, 2026

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

#include <gtkmm/box.h>
#include <gtkmm/entry.h>
#include <gtkmm/grid.h>
#include <gtkmm/label.h>

#include <YGP/Check.h>
#include <YGP/Trace.h>

#include "Player.h"
#include "PlayerDlg.h"

namespace Card {

//-----------------------------------------------------------------------------
/// Constructor
/// \param player Vector containing the players
//-----------------------------------------------------------------------------
PlayerDlg::PlayerDlg(std::vector<Player*>& player)
    : XDialog(OKCANCEL), sigCommit(), pClient(new Gtk::Grid()), aPlayers(), values(player) {
    TRACE2("PlayerDlg::PlayerDlg(std::vector<Player*>&) - Players: " << player.size());
    Check1(player.size() > 1);
    Check1(player.size() < 10);

    set_title(_("Set the name of the player"));

    aPlayers.push_back(new line(_("_Human:"), player[0]->getName()));
    aPlayers.back()->attach(*pClient, 0);

    for (unsigned int i(1); i < player.size(); ++i) {
        Glib::ustring label(_("Player _%1:"));
        label.replace(label.find("%1"), 2, 1, (char)('0' + i));
        aPlayers.push_back(new line(label, player[i]->getName()));

        aPlayers.back()->attach(*pClient, i);
    }

    pClient->set_margin(5);
    get_content_area()->append(*pClient);
    show();
}

//-----------------------------------------------------------------------------
/// Destructor
//-----------------------------------------------------------------------------
PlayerDlg::~PlayerDlg() {
    delete pClient;

    for (unsigned int i(0); i < aPlayers.size(); ++i)
        delete aPlayers[i];
    aPlayers.clear();
}

//-----------------------------------------------------------------------------
/// Handling of the OK button; closes dialog with commiting data
//-----------------------------------------------------------------------------
void PlayerDlg::okEvent() {
    for (unsigned int i(0); i < values.size(); ++i) {
        Check3(aPlayers[i]);
        Check3(aPlayers[i]->value);
        values[i]->setName(aPlayers[i]->value->get_text());
    }

    sigCommit.emit();
    return XDialog::okEvent();
}

//-----------------------------------------------------------------------------
/// Constructor
/// \param label Text for label
/// \param attribute Value for entryfield (to be updated)
//-----------------------------------------------------------------------------
PlayerDlg::line::line(const Glib::ustring& labelVal, const Glib::ustring& attribute)
    : label(Gtk::make_managed<Gtk::Label>(labelVal, true)), value(Gtk::make_managed<Gtk::Entry>()) {
    TRACE9("PlayerDlg::line::line(const Glib::ustring&, Glib::ustring&)");
    label->set_xalign(0);
    label->set_yalign(0.5);
    value->set_text(attribute);
    label->set_mnemonic_widget(*value);
}

//-----------------------------------------------------------------------------
/// Destructor
//-----------------------------------------------------------------------------
PlayerDlg::line::~line() { TRACE9("PlayerDlg::line::~line()"); }

//-----------------------------------------------------------------------------
/// Attaches the values of the structure to the passed table
/// \param table Table where to attach the values to
/// \param line Line in which to attach
//-----------------------------------------------------------------------------
void PlayerDlg::line::attach(Gtk::Grid& table, unsigned int line) {
    Check3(label);
    Check3(value);
    label->set_margin(3);
    label->set_margin_start(5);
    label->set_margin_end(5);
    table.attach(*label, 1, line + 1);

    value->set_hexpand();
    value->set_vexpand();
    value->set_margin(3);
    value->set_margin_start(5);
    value->set_margin_end(5);
    table.attach(*value, 2, line + 1);
}

} // namespace Card
