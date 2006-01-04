#ifndef DECKSELECT_H
#define DECKSELECT_H

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

#include <gtkmm/box.h>
#include <gtkmm/label.h>
#include <gtkmm/image.h>
#include <gtkmm/iconview.h>
#include <gtkmm/liststore.h>

#include <XGP/XDialog.h>


// Class to select the card decks to use
class DeckSelectDlg : public XGP::XDialog {
 private:
   /**Class describing the columns in the deck-lists
    */
   class DeckColumns : public Gtk::TreeModel::ColumnRecord {
    public:
      DeckColumns () { add (path); add (name); add (icon); }

      Gtk::TreeModelColumn<std::string> path;
      Gtk::TreeModelColumn<Glib::ustring> name;
      Gtk::TreeModelColumn<Glib::RefPtr<Gdk::Pixbuf> > icon;
   };

 public:
   DeckSelectDlg (const std::string& deck, const std::string& back);
   virtual ~DeckSelectDlg ();

   /// Creates a DeckSelect-dialog on the heap
   /// \param deck: Pre-selected deck
   /// \param back: Pre-selected back
   /// \returns DeckSelectDlg*: Created dialog
   static DeckSelectDlg* create (const std::string& deck, const std::string& back) {
      DeckSelectDlg* dlg (new DeckSelectDlg (deck, back));
      dlg->signal_response ().connect (mem_fun (*dlg, &DeckSelectDlg::free));
      return dlg; }

   /// Signal emitted, when changing deck/back is confirmed
   SigC::Signal2<void, const std::string&, const std::string&> setDecks;

 protected:
   virtual void okEvent ();
   virtual void command (int action);

   Glib::RefPtr<Gdk::Pixbuf> getImage (const std::string& file);

 private:
   // Prohibited manager-functions
   DeckSelectDlg ();
   DeckSelectDlg (const DeckSelectDlg&);

   void deckSelected ();
   void backSelected ();

   void deckActivated (const Gtk::TreeModel::Path& deck);
   void backActivated (const Gtk::TreeModel::Path& back);

   const DeckSelectDlg& operator= (const DeckSelectDlg&);

   DeckColumns cols;
   Glib::RefPtr<Gtk::ListStore> mDecks;
   Glib::RefPtr<Gtk::ListStore> mBacks;

   Gtk::HBox     boxDecks;
   Gtk::Label    txtDecks;
   Gtk::Image    selDeck;
   Gtk::IconView decks;

   Gtk::Image    selBack;
   Gtk::HBox     boxBack;
   Gtk::Label    txtBack;
   Gtk::IconView backs;
};


#endif
