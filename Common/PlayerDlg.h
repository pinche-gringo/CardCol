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


#include <vector>

#include <XDialog.h>


namespace Gtk {
   class Label;
   class Entry;
   class Table;
}


// Class to enter the names of the players
class IPlayerDlg : public XDialog {
 public:
   IPlayerDlg (std::vector<Glib::ustring>& names);
   virtual ~IPlayerDlg ();

   static IPlayerDlg* create (std::vector<Glib::ustring>& names) {
      IPlayerDlg* dlg (new IPlayerDlg (names));
      dlg->signal_delete_event ().connect (slot (*dlg, &XDialog::free));
      return dlg;
   }

 protected:
   virtual void okEvent ();

 private:
   //Prohibited manager functions
   IPlayerDlg (const IPlayerDlg& other);
   const IPlayerDlg& operator= (const IPlayerDlg& other);

   Gtk::Table* pClient;

   typedef struct line {
      Gtk::Label* label;
      Gtk::Entry* value;

      line (const Glib::ustring& label, Glib::ustring& attribute);
      ~line ();

      void attach (Gtk::Table& table, unsigned int line);
   } line;

   std::vector<line*>        aPlayers;
   std::vector<Glib::ustring>& values;
};


// Class to enter the names of the players with a callback to inform about the
// changes.
template <class T>
class PlayerDlg : public IPlayerDlg {
 public:
   typedef void (T::*PCALLBACK) ();

   PlayerDlg (T& parent, PCALLBACK callback, std::vector<Glib::ustring>& names)
      : IPlayerDlg (names), obj (parent), pCallback (callback) { }
   virtual ~PlayerDlg () { }
   
   static PlayerDlg* create (T& parent, PCALLBACK callback,
                             std::vector<Glib::ustring>& names) {
      PlayerDlg<T>* dlg (new PlayerDlg (parent, callback, names));
      dlg->signal_delete_event ().connect (slot (*dlg, &XDialog::free));
      dlg->get_window ()->set_transient_for (parent.get_window ());
      return dlg;
   }

 protected:
   virtual void okEvent () {
      IPlayerDlg::okEvent ();
      (obj.*pCallback) (); }

 private:
   PlayerDlg ();
   PlayerDlg (const PlayerDlg&);
   PlayerDlg& operator= (const PlayerDlg&);

   T& obj;
   PCALLBACK pCallback;
};

#endif
