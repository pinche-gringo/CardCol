//$Id$

//PROJECT     : Cardgames
//SUBSYSTEM   : Common/DeckSelect
//REFERENCES  :
//TODO        :
//BUGS        :
//REVISION    : $Revision$
//AUTHOR      : Markus Schwab
//CREATED     : 29.8.2002
//COPYRIGHT   : Copyright (C) 2002 - 2006

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


#include <cardgames-cfg.h>

#include <gdkmm/pixbuf.h>
#include <gdkmm/pixmap.h>

#include <gtkmm/stock.h>
#include <gtkmm/image.h>
#include <gtkmm/messagedialog.h>
#include <gtkmm/scrolledwindow.h>

#include <YGP/Check.h>
#include <YGP/Trace.h>
#include <YGP/DirSrch.h>

#include "DeckSelect.h"


//-----------------------------------------------------------------------------
/// Constructor; adds all controls to the dialog
/// \param deck: Name of deck to preselect
/// \param back: Name of back to preselect
//-----------------------------------------------------------------------------
DeckSelectDlg::DeckSelectDlg (const std::string& deck, const std::string& back)
   : XGP::XDialog (_("Select carddeck"), OKCANCEL),
     txtDecks (_("Available decks")),
     txtBack (_("Available backgrounds")) {
   TRACE3 ("CarddeckSelectDlg::CarddeckSelectDlg (2x const std::string&) - " << deck << " - " << back);

   Gtk::ScrolledWindow* scrl (new Gtk::ScrolledWindow);
   scrl->set_shadow_type (Gtk::SHADOW_ETCHED_IN);
   scrl->add (decks);
   scrl->set_policy (Gtk::POLICY_NEVER, Gtk::POLICY_AUTOMATIC);

   boxDecks.pack_start (*manage (scrl), true, true, 50);
   boxDecks.pack_start (selDeck, false, 5);

   scrl = new Gtk::ScrolledWindow;
   scrl->set_shadow_type (Gtk::SHADOW_ETCHED_IN);
   scrl->add (backs);
   scrl->set_policy (Gtk::POLICY_NEVER, Gtk::POLICY_AUTOMATIC);

   boxBack.pack_start (*manage (scrl), true, true, 50);
   boxBack.pack_start (selBack, false, 5);

   Check3 (get_vbox ());
   get_vbox ()->pack_start (txtDecks, false, false, 5);
   get_vbox ()->pack_start (boxDecks, true, true, 5);
   get_vbox ()->pack_start (txtBack, false, false, 5);
   get_vbox ()->pack_start (boxBack, true, true, 5);

   mDecks = Gtk::ListStore::create (cols);
   decks.set_model (mDecks);

   decks.signal_selection_changed ().connect (mem_fun (*this, &DeckSelectDlg::deckSelected));

#ifdef KDECARDS_DIR
   std::string cardDirs (KDECARDS_DIR); Check3 (cardDirs[cardDirs.size () - 1] == YGP::File::DIRSEPARATOR);
   cardDirs += "cards-*";
   YGP::DirectorySearch ds (cardDirs);

   TRACE8 ("DeckSelectDlg::DeckSelectDlg (const char*) - Searching in path "
           << cardDirs);
   const YGP::File* dir (ds.find (YGP::IDirectorySearch::FILE_DIRECTORY
                                  | YGP::IDirectorySearch::FILE_READONLY));
   while (dir) {
      Gtk::TreeRow row (*mDecks->append ());
      std::string file (dir->path ());
      file += dir->name ();
      file += YGP::File::DIRSEPARATOR;
      row[cols.path] = file;
      row[cols.name] = dir->name () + 6;

      file += "14.png";
      row[cols.icon] = getImage (file);

      TRACE9 ("DeckSelectDlg::DeckSelectDlg (2x const std::string&) - Comparing "
              << (std::string (dir->path ()) + dir->name ()) << " with " << deck);
      if ((std::string (dir->path ()) + dir->name ()) == deck)
         decks.select_path (mDecks->get_path (row));

      dir = ds.next ();
   }
#endif
#ifdef CARDPICS_DIR
   Gtk::TreeRow row (*mDecks->append ());
   row[cols.path] = CARDPICS_DIR;
   row[cols.name] = "Cardpics";
   row[cols.icon] = getImage (CARDPICS_DIR "24.png");

   TRACE9 ("DeckSelectDlg::DeckSelectDlg (2x const std::string&) - Comparing "
	   << CARDPICS_DIR "24.png" << " with " << deck);
   if (deck == CARDPICS_DIR)
      decks.select_path (mDecks->get_path (row));
#endif

   unsigned int height (132 * ((mDecks->children ().size () >> 2) + 1));
   decks.set_size_request ((mDecks->children ().size () > 3) ? 20 + (88 << 2) : 20 + 88 * mDecks->children ().size (),
			   height < 270 ? height : 270);

   mBacks = Gtk::ListStore::create (cols);
   backs.set_model (mBacks);
   backs.signal_selection_changed ().connect (mem_fun (*this, &DeckSelectDlg::backSelected));

#ifdef KDECARDS_DIR
   std::string pathDecks (KDECARDS_DIR "decks/");
   dir = ds.find (pathDecks + "deck*.png", YGP::IDirectorySearch::FILE_NORMAL
                  | YGP::IDirectorySearch::FILE_READONLY);
   while (dir) {
      Gtk::TreeRow row (*mBacks->append ());
      row[cols.path] = pathDecks + dir->name ();
      row[cols.icon] = getImage (pathDecks + dir->name ());

      TRACE9 ("DeckSelectDlg::DeckSelectDlg (2x const std::string&) - Comparing "
              << (pathDecks + dir->name ()) << " with " << back);
      if ((pathDecks + dir->name ()) == back)
         backs.select_path (mBacks->get_path (row));

      dir = ds.next ();
   }
#endif

#ifdef CARDPICS_DIR
   row = (*mBacks->append ());
   row[cols.path] = CARDPICS_DIR;
   row[cols.icon] = getImage (CARDPICS_DIR "78.png");

   TRACE9 ("DeckSelectDlg::DeckSelectDlg (2x const std::string&) - Comparing "
	   << CARDPICS_DIR "78.png" << " with " << deck);
   if (back == CARDPICS_DIR "78.png")
      backs.select_path (mBacks->get_path (row));
#endif

   height = 132 * ((mBacks->children ().size () >> 2) + 1);
   backs.set_size_request ((mBacks->children ().size () > 3) ? 20 + (88 << 2) : 20 + 88 * mBacks->children ().size (),
			   height < 270 ? height : 270);

   if (mDecks->children ().size () || mBacks->children ().size ()) {
      Gtk::Button& apply (*manage (new Gtk::Button (Gtk::Stock::APPLY)));
      apply.signal_clicked ().connect
	 (bind (mem_fun (*this, &DeckSelectDlg::command), Gtk::RESPONSE_APPLY));
      get_action_area ()->pack_end (apply, false, false, 5);
      apply.show ();
   }
   else
      ok->set_sensitive (false);

   decks.set_pixbuf_column (cols.icon);
   decks.set_text_column (cols.name);
   backs.set_pixbuf_column (cols.icon);

   show_all ();
}

//-----------------------------------------------------------------------------
/// Destructor
//-----------------------------------------------------------------------------
DeckSelectDlg::~DeckSelectDlg () {
   TRACE9 ("CarddeckSelectDlg::~CarddeckSelectDlg ()");
}


//-----------------------------------------------------------------------------
/// Callback after selecting a button
/// \param action: ID of selected button
//-----------------------------------------------------------------------------
void DeckSelectDlg::command (int action) {
   TRACE9 ("DeckSelectDlg::command (int) - Command: " << action);
   if (action == Gtk::RESPONSE_APPLY) {
      std::string deck, back;
      if (decks.get_selected_items ().size ()) {
	 Gtk::TreePath path (*(decks.get_selected_items ().begin ()));
	 Gtk::TreeRow row (*mDecks->get_iter (path));
	 deck = row[cols.path];
      }

      if (backs.get_selected_items ().size ()) {
	 Gtk::TreePath path (*(backs.get_selected_items ().begin ()));
	 Gtk::TreeRow row (*mBacks->get_iter (path));
	 back = row[cols.path];
      }
      setDecks.emit (deck, back);
   }
}

//-----------------------------------------------------------------------------
/// Callback after selecting the OK button
//-----------------------------------------------------------------------------
void DeckSelectDlg::okEvent () {
   command (Gtk::RESPONSE_APPLY);
   XGP::XDialog::okEvent ();
}

//-----------------------------------------------------------------------------
/// Returns an image specified by the passed file
/// \param file: File containing the image
/// \returns Glib::RefPtr<Gdk::Pixbuf>: Created image
//-----------------------------------------------------------------------------
Glib::RefPtr<Gdk::Pixbuf> DeckSelectDlg::getImage (const std::string& file) {
   TRACE9 ("DeckSelectDlg::setButtonImage (const std::string&) - " << file);
   Glib::RefPtr<Gdk::Pixbuf> imgBuf;
   std::string err;

   try {
      imgBuf = Gdk::Pixbuf::create_from_file (file.c_str ());
      if ((imgBuf->get_height () != 72) || (imgBuf->get_width () != 96))
	 imgBuf = imgBuf->scale_simple (72, 96, Gdk::INTERP_BILINEAR);
   }
   catch (Gdk::PixbufError& e) {
      err = e.what ();
   }
   catch (Glib::FileError& e) {
      err = e.what ();
   }
   catch (...) {
      err = _("Unknown error");
   }
   if (err.size ()) {
      std::string msg (_("Error loading image from file `%1'!\n\nReason: %2"));
      msg.replace (msg.find ("%1"), 2, file);
      msg.replace (msg.find ("%2"), 2, err);
      Gtk::MessageDialog dlg (msg, false, Gtk::MESSAGE_ERROR, Gtk::BUTTONS_OK);
      dlg.run ();
   }

   return imgBuf;
}

//-----------------------------------------------------------------------------
/// Callback after changing the selected deck
//-----------------------------------------------------------------------------
void DeckSelectDlg::deckSelected () {
   TRACE8 ("DeckSelectDlg::deckSelected ()");

   if (decks.get_selected_items ().size ()) {
      Gtk::TreePath path (*(decks.get_selected_items ().begin ()));
      Gtk::TreeRow row (*mDecks->get_iter (path));
      Glib::RefPtr<Gdk::Pixbuf> img (row[cols.icon]);

      selDeck.set (img);
   }
}

//-----------------------------------------------------------------------------
/// Callback after changing the selected back
//-----------------------------------------------------------------------------
void DeckSelectDlg::backSelected () {
   TRACE8 ("DeckSelectDlg::backSelected ()");

   if (backs.get_selected_items ().size ()) {
      Gtk::TreePath path (*(backs.get_selected_items ().begin ()));
      Gtk::TreeRow row (*mBacks->get_iter (path));
      Glib::RefPtr<Gdk::Pixbuf> img (row[cols.icon]);

      selBack.set (img);
   }
}
