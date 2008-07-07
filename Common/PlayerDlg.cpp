//$Id$

//PROJECT     : Cardgames
//SUBSYSTEM   : Common
//REFERENCES  :
//TODO        :
//BUGS        :
//REVISION    : $Revision$
//AUTHOR      : Markus Schwab
//CREATED     : 07.01.2003
//COPYRIGHT   : Copyright (C) 2002 - 2004, 2008

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
#include <gtkmm/label.h>
#include <gtkmm/entry.h>
#include <gtkmm/table.h>

#include <YGP/Check.h>
#include <YGP/Trace.h>

#include "Player.h"
#include "PlayerDlg.h"


//-----------------------------------------------------------------------------
/// Constructor
/// \param player: Vector containing the players
//-----------------------------------------------------------------------------
PlayerDlg::PlayerDlg (std::vector<Player*>& player)
   : XDialog (OKCANCEL), pClient (new Gtk::Table (player.size () + 1, 3)), values (player) {
   TRACE2 ("PlayerDlg::PlayerDlg (std::vector<Player*>&) - Players: "
           << player.size ());
   Check1 (player.size () > 1);
   Check1 (player.size () < 10);

   set_title (_("Set the name of the player"));

   aPlayers.push_back (new line (_("_Human:"), player[0]->getName ()));
   aPlayers.back ()->attach (*pClient, 0);

   for (unsigned int i (1); i < player.size (); ++i) {
      Glib::ustring label (_("Player _%1:"));
      label.replace (label.find ("%1"), 2, 1, (char)('0' + i));
      aPlayers.push_back (new line (label, player[i]->getName ()));

      aPlayers.back ()->attach (*pClient, i);
   }

   get_vbox ()->pack_start (*pClient, false, false, 5);
   show_all ();
}

//-----------------------------------------------------------------------------
/// Destructor
//-----------------------------------------------------------------------------
PlayerDlg::~PlayerDlg () {
   delete pClient;

   for (unsigned int i (0); i < aPlayers.size (); ++i)
      delete aPlayers[i];
   aPlayers.clear ();
}

//-----------------------------------------------------------------------------
/// Handling of the OK button; closes dialog with commiting data
//-----------------------------------------------------------------------------
void PlayerDlg::okEvent () {
   for (unsigned int i (0); i < values.size (); ++i) {
      Check3 (aPlayers[i]); Check3 (aPlayers[i]->value);
      values[i]->setName (aPlayers[i]->value->get_text ());
   }

   sigCommit.emit ();
   return XDialog::okEvent ();
}


//-----------------------------------------------------------------------------
/// Constructor
/// \param label: Text for label
/// \param attribute: Value for entryfield (to be updated)
//-----------------------------------------------------------------------------
PlayerDlg::line::line (const Glib::ustring& labelVal, const Glib::ustring& attribute)
    : label (manage (new Gtk::Label (labelVal, 0, 0.5, true)))
      , value (manage (new Gtk::Entry ())) {
   TRACE9 ("PlayerDlg::line::line (const Glib::ustring&, Glib::ustring&)");
   value->set_text (attribute);
   label->set_mnemonic_widget (*value);
}

//-----------------------------------------------------------------------------
/// Destructor
//-----------------------------------------------------------------------------
PlayerDlg::line::~line () {
   TRACE9 ("PlayerDlg::line::~line ()");
}


//-----------------------------------------------------------------------------
/// Attaches the values of the structure to the passed table
/// \param table: Table where to attach the values to
/// \param line: Line in which to attach
//-----------------------------------------------------------------------------
void PlayerDlg::line::attach (Gtk::Table& table, unsigned int line) {
   Check3 (label); Check3 (value);
   table.attach (*label, 1, 2, line + 1, line + 2, Gtk::FILL, Gtk::FILL, 5, 3);
   table.attach (*value, 2, 3, line + 1, line + 2, Gtk::FILL | Gtk::EXPAND,
                 Gtk::FILL | Gtk::EXPAND, 5, 3);
}
