//$Id$

//PROJECT     : Cardgames
//SUBSYSTEM   : Common/DeckSelect
//REFERENCES  :
//TODO        :
//BUGS        :
//REVISION    : $Revision$
//AUTHOR      : Markus Schwab
//CREATED     : 29.8.2002
//COPYRIGHT   : Copyright (C) 2002 - 2004

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

#include <YGP/Check.h>
#include <YGP/Trace.h>
#include <YGP/DirSrch.h>

#include "DeckSelect.h"


static const char* const DEFAULTFILE = "14.png";


//-----------------------------------------------------------------------------
/// Constructor; adds all controls to the dialog
/// \param path: Path to carddecks
/// \param deck: Name of deck to preselect
/// \param back: Name of back to preselect
//-----------------------------------------------------------------------------
ICarddeckSelectDlg::ICarddeckSelectDlg (const char* path, const std::string& deck,
                                        const std::string& back)
   : XGP::XDialog (_("Select carddeck"), OKCANCEL)
     , offDeck (-1), offBack (-1)
     , boxDecks (), txtDecks (_("Available decks")), selDeck (), decks ()
     , selBack (), boxBack (), txtBack (_("Available backgrounds")), backs ()
     , box (Gtk::BUTTONBOX_END, 5) {
   TRACE3 ("CarddeckSelectDlg::CarddeckSelectDlg (const char*) - " << path
           << " (" << deck << " - " << back << ')');

   boxDecks.pack_start (decks, true, true, 50);
   boxDecks.pack_start (selDeck, false, 5);

   boxBack.pack_start (backs, true, true, 50);
   boxBack.pack_start (selBack, false, 5);

   Check3 (get_vbox ());
   get_vbox ()->pack_start (txtDecks, false, false, 5);
   get_vbox ()->pack_start (boxDecks, true, true, 5);
   get_vbox ()->pack_start (txtBack, false, false, 5);
   get_vbox ()->pack_start (boxBack, true, true, 5);

   std::string cardDirs (path ? path : CARDDECKS_DIR);
   if (cardDirs.size ()
       && (cardDirs[cardDirs.size () - 1] != YGP::File::DIRSEPARATOR))
      cardDirs += YGP::File::DIRSEPARATOR;
   aFiles.push_back (cardDirs);
   cardDirs += "cards-*";
   YGP::DirectorySearch ds (cardDirs);

   TRACE8 ("ICarddeckSelectDlg::ICarddeckSelectDlg (const char*) - Searching in path "
           << cardDirs);
   const YGP::File* dir (ds.find (YGP::IDirectorySearch::FILE_DIRECTORY
                                  | YGP::IDirectorySearch::FILE_READONLY));
   unsigned int offset (0);

   show_all ();

   while (dir) {
      TRACE9 ("ICarddeckSelectDlg::ICarddeckSelectDlg (const char*) - Found dir "
              << dir->name ());

      std::string pathDeck (aFiles[0]);
      aFiles.push_back (dir->name ());
      pathDeck += dir->name ();

      std::string file (pathDeck);
      file += YGP::File::DIRSEPARATOR;
      file += DEFAULTFILE;
      TRACE6 ("ICarddeckSelectDlg::ICarddeckSelectDlg (const char*) - Reading file "
              << file);

      Gtk::Button* temp (createButton (file));
      temp->signal_clicked ().connect
         (bind (mem_fun (*this, &ICarddeckSelectDlg::deckSelect), offset));
      aDecks.push_back (temp);

      decks.add (*temp);

      TRACE9 ("ICarddeckSelectDlg::ICarddeckSelectDlg (const char*) - Comparing "
              << pathDeck << " with " << deck);
      if (pathDeck == deck)
         deckSelect (offset);

      dir = ds.next ();
      ++offset;
   }

   int height (50), width (50);
   if (aDecks.size ())
      aDecks.front ()->get_size_request (width, height);
   height = (height + 20) * ((offset >> 2) + 1);
   width = (width + 25) << 2;
   TRACE1 ("ICarddeckSelectDlg::ICarddeckSelectDlg (const char*, const"
           "std::string&, const std::string&) - Decksize:  " << width
           << '/' << (height < 250 ? height : 250));
   decks.set_size_request (width + 25, height < 270 ? height : 270);

   if ((offDeck == -1) && (aFiles.size () > 1))
      deckSelect (1);

   unsigned int offsetBack (offset + 1);
   std::string pathDecks (aFiles[0] + "decks/");
   dir = ds.find (pathDecks + "deck*.png", YGP::IDirectorySearch::FILE_NORMAL
                  | YGP::IDirectorySearch::FILE_READONLY);
   while (dir) {
      TRACE9 ("ICarddeckSelectDlg::ICarddeckSelectDlg (const char*) - Reading "
              "background file " << dir->path () << dir->name ());
      aFiles.push_back (dir->name ());

      Gtk::Button* temp (createButton (pathDecks + dir->name ()));
      temp->signal_clicked ().connect
         (bind (mem_fun (*this, &ICarddeckSelectDlg::backSelect), ++offset));
      aBacks.push_back (temp);

      TRACE9 ("ICarddeckSelectDlg::ICarddeckSelectDlg (const char*) - Comparing "
              << (pathDecks + dir->name ()) << " with " << back);
      if ((pathDecks + dir->name ()) == back)
         backSelect (offset);

      backs.add (*temp);
      dir = ds.next ();
   }
   if ((offBack == -1) && (aFiles.size () > static_cast<unsigned int> (offBack)))
      backSelect (offsetBack);

   offset -= offBack;

   height = width = 50;
   if (aBacks.size ())
      aBacks.front ()->get_size_request (width, height);
   height = (height + 20) * ((offset >> 2) + 1);
   width = (width + 25) << 2;
   TRACE1 ("ICarddeckSelectDlg::ICarddeckSelectDlg (const char*, const"
           "std::string&, const std::string&) - Backsize:  " << width
           << '/' << (height < 250 ? height : 250));
   backs.set_size_request (width + 25, height < 270 ? height : 270);

   if ((offDeck != -1) || (offBack != -1)) {
      Gtk::Button& apply (*manage (new Gtk::Button (Gtk::Stock::APPLY)));
      apply.signal_clicked ().connect
	 (bind (mem_fun (*this, &ICarddeckSelectDlg::command), Gtk::RESPONSE_APPLY));
      get_action_area ()->pack_end (apply, false, false, 5);
      apply.show ();
   }
   else
      ok->set_sensitive (false);
}

//-----------------------------------------------------------------------------
/// Destructor
//-----------------------------------------------------------------------------
ICarddeckSelectDlg::~ICarddeckSelectDlg () {
   TRACE9 ("CarddeckSelectDlg::~CarddeckSelectDlg ()");

   for (std::vector<Gtk::Button*>::iterator i (aDecks.begin ());
        i != aDecks.end (); ++i)
      delete *i;

   for (std::vector<Gtk::Button*>::iterator i (aBacks.begin ());
        i != aBacks.end (); ++i)
      delete *i;
}


//-----------------------------------------------------------------------------
/// Callback after selecting a carddeck
/// \param offset: Position of name in aFiles array
//-----------------------------------------------------------------------------
void ICarddeckSelectDlg::deckSelect (unsigned int offset) {
   TRACE9 ("ICarddeckSelectDlg::deckSelect (const std::string&) - Position "
           << offset);
   Check3 (offset < aFiles.size ());
   TRACE3 ("ICarddeckSelectDlg::deckSelect (const std::string&) - Selected "
           << aFiles[0] << aFiles[offset + 1]);

   setButtonImage (selDeck, aFiles[0] + aFiles[offDeck = offset + 1]
                   + YGP::File::DIRSEPARATOR + DEFAULTFILE);
}

//-----------------------------------------------------------------------------
/// Callback after selecting a background
/// \param offset: Position of name in aFiles array
//-----------------------------------------------------------------------------
void ICarddeckSelectDlg::backSelect (unsigned int offset) {
   TRACE9 ("ICarddeckSelectDlg::backSelect (const std::string&) - Position "
           << offset);
   Check3 (offset < aFiles.size ());

   TRACE3 ("ICarddeckSelectDlg::backSelect (const std::string&) - Selected "
           << aFiles[0] << "decks/" << aFiles[offset]);

   setButtonImage (selBack, aFiles[0] + "decks" + YGP::File::DIRSEPARATOR
                   + aFiles[offBack = offset]);
}

//-----------------------------------------------------------------------------
/// Callback after selecting a button
/// \param action: ID of selected button
//-----------------------------------------------------------------------------
void ICarddeckSelectDlg::command (int action) {
   TRACE9 ("ICarddeckSelectDlg::command (int) - Command: " << action);
}

//-----------------------------------------------------------------------------
/// Creates a pixmap-button, with an image from the passed file
/// \param file: File containing the image
//-----------------------------------------------------------------------------
Gtk::Button* ICarddeckSelectDlg::createButton (const std::string& file) {
   TRACE9 ("ICarddeckSelectDlg::createButton (const std::string&) - " << file);
   Gtk::Button* temp (new Gtk::Button ());

   setButtonImage (*temp, file);
   temp->show ();
   return temp;
}

//-----------------------------------------------------------------------------
/// Sets an image for the passed button
/// \param button: Button to change
/// \param file: File containing the image
//-----------------------------------------------------------------------------
void ICarddeckSelectDlg::setButtonImage (Gtk::Button& button, const std::string& file) {
   TRACE9 ("ICarddeckSelectDlg::setButtonImage (Gtk::Button&, const std::string&) - "
           << file);
   Glib::RefPtr<Gdk::Pixbuf> imgBuf;
   std::string err;

   try {
      imgBuf = Gdk::Pixbuf::create_from_file (file.c_str ());
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
      return;
   }

   Glib::RefPtr<Gdk::Pixmap> img;
   Glib::RefPtr<Gdk::Bitmap> bitmap;

   button.remove ();
   imgBuf->render_pixmap_and_mask (img, bitmap, 0);

   int x, y;
   button.add_pixmap (img, bitmap);
   img->get_size (x, y);
   button.set_size_request (x + 6, y + 6);

   button.set_relief (Gtk::RELIEF_NONE);
   dynamic_cast <Gtk::Image*> (button.get_child ())->set_alignment (0.0, 0.0);
}
