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

#include <Check.h>
#include <Trace_.h>

#include <DirSrch.h>

#include "DeckSelect.h"


/*--------------------------------------------------------------------------*/
//Purpose   : Constructor; adds all controls to the dialog
//Parameters: path: Path to carddecks
/*--------------------------------------------------------------------------*/
ICarddeckSelectDlg::ICarddeckSelectDlg (const char* path)
   : Dialog (), txtDecks (_("Available decks")), ok (_("OK"))
   , apply (_("Apply")), cancel (_("Cancel")), decks () {
   TRACE3 ("CarddeckSelectDlg::CarddeckSelectDlg (const char*)");

   set_title (_("Select carddeck"));

   if (!path)
      path = PKGDIR;

   ok.grab_default ();

   Check3 (get_vbox ());
   get_vbox ()->pack_start (txtDecks, true, false, 5);
   get_vbox ()->pack_start (decks, true, true, 5);

   get_action_area ()->pack_start (ok);
   get_action_area ()->pack_start (apply);
   get_action_area ()->pack_start (cancel);

   show_all ();

   ok.clicked.connect (bind (slot (this, &ICarddeckSelectDlg::command), OK));
   apply.clicked.connect (bind (slot (this, &ICarddeckSelectDlg::command), APPLY));
   cancel.clicked.connect (bind (slot (this, &ICarddeckSelectDlg::command), CANCEL));

   std::string cardDirs (path);
   cardDirs += File::DIRSEPARATOR;
   cardDirs += "deck*";
   DirectorySearch ds (cardDirs);

   Gdk_Color color;

   TRACE8 ("ICarddeckSelectDlg::ICarddeckSelectDlg (const char*) - Searching in path "
           << cardDirs);
   const File* dir (ds.find (IDirectorySearch::FILE_DIRECTORY
                             | IDirectorySearch::FILE_READONLY));
   while (dir) {
      TRACE9 ("ICarddeckSelectDlg::ICarddeckSelectDlg (const char*) - Found dir "
              << dir->name ());

      std::string file (dir->path ());
      file += dir->name ();
      file += File::DIRSEPARATOR;
      file += "14.xpm";
      TRACE9 ("ICarddeckSelectDlg::ICarddeckSelectDlg (const char*) - Reading file "
              << file);

      Button* temp (new Button ());
      temp->show ();
      Gdk_Pixmap img;
      img.create_from_xpm (get_window (), color, file);
      temp->add_pixmap (img, NULL);
      aDecks.push_back (temp);

      decks.add (*temp, GTK_SIDE_LEFT);

      dir = ds.next ();
   }
}

/*--------------------------------------------------------------------------*/
//Purpose   : Destructor
/*--------------------------------------------------------------------------*/
ICarddeckSelectDlg::~ICarddeckSelectDlg () {
   TRACE9 ("CarddeckSelectDlg::~CarddeckSelectDlg ()");

   for (vector<Button*>::iterator i (aDecks.begin ());
        i != aDecks.end (); ++i)
      delete *i;
}
