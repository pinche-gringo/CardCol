#ifndef PLAYERDLG_H
#define PLAYERDLG_H

//$Id$

// This program is free software; you can redistribute it and/or modify
// it under the terms of the GNU General Public License as published by
// the Free Software Foundation; either version 2 of the License, or
// (at your option) any later version.

// This program is distributed in the hope that it will be useful,
// but WITHOUT ANY WARRANTY; without even the implied warranty of
// MERCHANTABILITY or FITNESS FOR A PARTICULAR PURPOSE.  See the
// GNU General Public License for more details.

// You should have received a copy of the GNU General Public License
// along with this program; if not, write to the Free Software
// Foundation, Inc., 59 Temple Place - Suite 330, Boston, MA 02111-1307, USA.


#include <string>
#include <vector>

#include <XDialog.h>


namespace Gtk {
   class Label;
   class Entry;
   class Table;
}


class PlayerDlg : public XDialog {
 public:
   PlayerDlg (vector<string>& names);
   virtual ~PlayerDlg ();

   static PlayerDlg* perform (vector<string>& names) {
      return new PlayerDlg (names); }

 private:
   //Prohibited manager functions
   PlayerDlg (const PlayerDlg& other);
   const PlayerDlg& operator= (const PlayerDlg& other);

   virtual void okEvent ();

   Gtk::Table* pClient;

   typedef struct line {
      Gtk::Label* label;
      Gtk::Entry* value;

      line (const char* label, string& attribute);
      line (string& label, string& attribute);
      ~line ();

      void show ();
      void attach (Gtk::Table& table, unsigned int line);
   } line;

   vector<line*> aPlayers;
   vector<string>& values;
};

#endif
