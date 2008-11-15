#ifndef PLAYERDLG_H
#define PLAYERDLG_H

//$Id$

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


#include <vector>

#include <XGP/XDialog.h>


namespace Gtk {
   class Label;
   class Entry;
   class Table;
}

class Player;


// Class to enter the names of the players
class PlayerDlg : public XGP::XDialog {
 public:
   PlayerDlg (std::vector<Player*>& player);
   virtual ~PlayerDlg ();

   /// Creates a new player-dialogue; showing the passed players. Any
   /// changes to the names are also reflected into the passed names
   /// \param player Vector holding name of all players
   /// \returns PlayerDlg* Pointer to the created dialoge
   static PlayerDlg* create (std::vector<Player*>& player) {
      PlayerDlg* dlg (new PlayerDlg (player));
      dlg->signal_response ().connect (mem_fun (*dlg, &PlayerDlg::free));
      return dlg;
   }

   /// Signal emitted, when OK clicked
   sigc::signal<void> sigCommit;

 protected:
   virtual void okEvent ();                     ///< Callback after clicking OK

 private:
   /// \name Prohibited manager functions
   //@{
   PlayerDlg (const PlayerDlg& other);
   const PlayerDlg& operator= (const PlayerDlg& other);
   //@}

   Gtk::Table* pClient;

   typedef struct line {
      Gtk::Label* label;
      Gtk::Entry* value;

      line (const Glib::ustring& label, const Glib::ustring& attribute);
      ~line ();

      void attach (Gtk::Table& table, unsigned int line);

    private:
      line (const line&);
      line& operator= (const line&);
   } line;

   std::vector<line*>    aPlayers;
   std::vector<Player*>& values;
};

#endif
