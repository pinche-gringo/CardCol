//$Id$

//PROJECT     : Cardgames
//SUBSYSTEM   : Common/DeckSelect
//REFERENCES  :
//TODO        : - Use button labels from General-lib
//BUGS        :
//REVISION    : $Revision$
//AUTHOR      : Markus Schwab
//CREATED     : 29.8.2002
//COPYRIGHT   : Anticopyright (A) 2002

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

#include <gdkmm/pixmap.h>

#include <gtkmm/stock.h>
#include <gtkmm/image.h>

#include <Check.h>
#include <Trace_.h>

#include <DirSrch.h>
#include <Cardset-config.h>

#include "DeckSelect.h"


static const char* const DEFAULTFILE = "14.xpm";


/*--------------------------------------------------------------------------*/
//Purpose   : Constructor; adds all controls to the dialog
//Parameters: path: Path to carddecks
//            deck: Name of deck to preselect
//            back: Name of back to preselect
/*--------------------------------------------------------------------------*/
ICarddeckSelectDlg::ICarddeckSelectDlg (const char* path, const std::string& deck,
                                        const std::string& back)
   : XDialog (OKCANCEL), txtDecks (_("Available decks"))
   , decks (), boxDecks ()
   , txtBack (_("Available backgrounds")), backs (), boxBack ()
   , selDeck (), selBack (), offDeck (-1), offBack (-1)
   , box (Gtk::BUTTONBOX_END, 5), scrlBack (), scrlDeck () {
   TRACE3 ("CarddeckSelectDlg::CarddeckSelectDlg (const char*) - " << path
           << " (" << deck << " - " << back << ')');

   set_title (_("Select carddeck"));

   add_button (Gtk::Stock::APPLY, Gtk::RESPONSE_APPLY);

   scrlDeck.set_policy (Gtk::POLICY_AUTOMATIC, Gtk::POLICY_AUTOMATIC);
   scrlBack.set_policy (Gtk::POLICY_AUTOMATIC, Gtk::POLICY_AUTOMATIC);

   boxDecks.pack_start (scrlDeck, true, true, 50);
   boxDecks.pack_start (selDeck, false, false, 5);
   scrlDeck.add (decks);

   boxBack.pack_start (scrlBack, true, true, 50);
   boxBack.pack_start (selBack, false, false, 5);
   scrlBack.add (backs);

   Check3 (get_vbox ());
   get_vbox ()->pack_start (txtDecks, false, false, 5);
   get_vbox ()->pack_start (boxDecks, true, true, 5);
   get_vbox ()->pack_start (txtBack, false, false, 5);
   get_vbox ()->pack_start (boxBack, true, true, 5);

   std::string cardDirs (path ? path : CARDSET_PATH);
   if (cardDirs.size ()
       && (cardDirs[cardDirs.size () - 1] != File::DIRSEPARATOR))
      cardDirs += File::DIRSEPARATOR;
   cardDirs += "Deck*";
   DirectorySearch ds (cardDirs);

   TRACE8 ("ICarddeckSelectDlg::ICarddeckSelectDlg (const char*) - Searching in path "
           << cardDirs);
   const File* dir (ds.find (IDirectorySearch::FILE_DIRECTORY
                             | IDirectorySearch::FILE_READONLY));
   unsigned int offset (0);
   aFiles.push_back (dir->path ());

   show_all ();

   while (dir) {
      TRACE9 ("ICarddeckSelectDlg::ICarddeckSelectDlg (const char*) - Found dir "
              << dir->name ());

      std::string pathDeck (aFiles[0]);
      aFiles.push_back (dir->name ());
      pathDeck += dir->name ();

      std::string file (pathDeck);
      file += File::DIRSEPARATOR;
      file += DEFAULTFILE;
      TRACE9 ("ICarddeckSelectDlg::ICarddeckSelectDlg (const char*) - Reading file "
              << file);

      Gtk::Button* temp (createButton (file));
      temp->signal_clicked ().connect
         (bind (slot (*this, &ICarddeckSelectDlg::deckSelect), ++offset));
      aDecks.push_back (temp);

      decks.resize ((offset >> 2) + 1, 4);
      decks.attach (*temp, offset & 0x3, (offset & 0x3) + 1, offset >> 2,
                    (offset >> 2) + 1, Gtk::SHRINK, Gtk::SHRINK, 5, 5);

      TRACE9 ("ICarddeckSelectDlg::ICarddeckSelectDlg (const char*) - Comparing "
              << pathDeck << " with " << deck);
      if (pathDeck == deck)
         deckSelect (offset);

      dir = ds.next ();
   }
   if (offDeck == -1)
      deckSelect (1);

   int height (10), width (10);
   if (aDecks.size ())
      aDecks.front ()->get_size_request (width, height);
   height = (height + 20) * ((offset >> 2) + 1);
   width = (width + 25) << 2;
   scrlDeck.set_size_request (width, height < 250 ? height : 250);

   TRACE9 ("ICarddeckSelectDlg::ICarddeckSelectDlg (const char*) - Decksize = "
           << width << '/' << height);

   unsigned int offsetBack (offset + 1);
   dir = ds.find (aFiles[0] + "/back*.xpm", IDirectorySearch::FILE_NORMAL
                  | IDirectorySearch::FILE_READONLY);
   while (dir) {
      TRACE9 ("ICarddeckSelectDlg::ICarddeckSelectDlg (const char*) - Reading "
              "background file " << dir->path () << dir->name ());
      aFiles.push_back (dir->name ());

      Gtk::Button* temp (createButton (aFiles[0] + dir->name ()));
      temp->signal_clicked ().connect
         (bind (slot (*this, &ICarddeckSelectDlg::backSelect), ++offset));
      aBacks.push_back (temp);

      TRACE9 ("ICarddeckSelectDlg::ICarddeckSelectDlg (const char*) - Comparing "
              << (aFiles[0] + dir->name ()) << " with " << back);
      if ((aFiles[0] + dir->name ()) == back)
         backSelect (offset);


      backs.resize (((offset - offsetBack) >> 2) + 1, 4);
      backs.attach (*temp, (offset - offsetBack) & 0x3, ((offset - offsetBack) & 0x3) + 1,
                    (offset - offsetBack) >> 2, ((offset - offsetBack) >> 2) + 1,
                    Gtk::SHRINK, Gtk::SHRINK, 5, 5);

      dir = ds.next ();
   }
   if (offBack == -1)
      backSelect (offsetBack);

   offset -= offBack;

   height = 10;
   width = 10;
   if (aBacks.size ())
      aBacks.front ()->get_size_request (width, height);
   height = (height + 20) * ((offset >> 2) + 1);
   width = (width + 25) << 2;
   scrlBack.set_size_request (width, height < 250 ? height : 250);
}

/*--------------------------------------------------------------------------*/
//Purpose   : Destructor
/*--------------------------------------------------------------------------*/
ICarddeckSelectDlg::~ICarddeckSelectDlg () {
   TRACE9 ("CarddeckSelectDlg::~CarddeckSelectDlg ()");

   for (std::vector<Gtk::Button*>::iterator i (aDecks.begin ());
        i != aDecks.end (); ++i)
      delete *i;

   for (std::vector<Gtk::Button*>::iterator i (aBacks.begin ());
        i != aBacks.end (); ++i)
      delete *i;
}


/*--------------------------------------------------------------------------*/
//Purpose   : Callback after selecting a carddeck
//Parameters: offset: Position of name in aFiles array
/*--------------------------------------------------------------------------*/
void ICarddeckSelectDlg::deckSelect (unsigned int offset) {
   TRACE9 ("ICarddeckSelectDlg::deckSelect (const std::string&) - Position "
           << offset);
   Check3 (offset < aFiles.size ());
   TRACE3 ("ICarddeckSelectDlg::deckSelect (const std::string&) - Selected "
           << aFiles[0] << aFiles[offset]);

   setButtonImage (selDeck, aFiles[0] + aFiles[offDeck = offset]);
}

/*--------------------------------------------------------------------------*/
//Purpose   : Callback after selecting a background
//Parameters: offset: Position of name in aFiles array
/*--------------------------------------------------------------------------*/
void ICarddeckSelectDlg::backSelect (unsigned int offset) {
   TRACE9 ("ICarddeckSelectDlg::backSelect (const std::string&) - Position "
           << offset);
   Check3 (offset < aFiles.size ());

   TRACE3 ("ICarddeckSelectDlg::backSelect (const std::string&) - Selected "
           << aFiles[0] << aFiles[offset]);

   setButtonImage (selBack, aFiles[0] + aFiles[offBack = offset]);
}

/*--------------------------------------------------------------------------*/
//Purpose   : Callback after selecting a button
//Parameters: action: ID of selected button
/*--------------------------------------------------------------------------*/
void ICarddeckSelectDlg::command (int action) {
   TRACE9 ("ICarddeckSelectDlg::command (int) - Command: " << action);
   Check1 (action == Gtk::RESPONSE_APPLY);
}

/*--------------------------------------------------------------------------*/
//Purpose   : Creates a pixmap-button, with an image from the passed file
//Parameters: file: File containing the image
/*--------------------------------------------------------------------------*/
Gtk::Button* ICarddeckSelectDlg::createButton (const std::string& file) {
   Gtk::Button* temp (new Gtk::Button ());

   setButtonImage (*temp, file);
   temp->show ();
   return temp;
}

/*--------------------------------------------------------------------------*/
//Purpose   : Sets an image for the passed button
//Parameters: button: Button to change
//            file: File containing the image
/*--------------------------------------------------------------------------*/
void ICarddeckSelectDlg::setButtonImage (Gtk::Button& button, const std::string& file) {
   Gdk::Color color;
   Glib::RefPtr<Gdk::Pixmap> img;

   button.remove ();
   img->create_from_xpm (get_window (), color, file);
   button.add_pixmap (img, Glib::RefPtr<Gdk::Bitmap> (NULL));
   button.set_relief (Gtk::RELIEF_NONE);
   dynamic_cast <Gtk::Image*> (selBack.get_child ())->set_alignment (0.0, 0.0);
}
