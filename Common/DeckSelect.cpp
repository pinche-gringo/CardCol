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

#include <gtk--/pixmap.h>

#include <Check.h>
#include <Trace_.h>

#include <DirSrch.h>
#include <Cardset-config.h>

#include "DeckSelect.h"


/*--------------------------------------------------------------------------*/
//Purpose   : Constructor; adds all controls to the dialog
//Parameters: path: Path to carddecks
//            deck: Name of deck to preselect
//            back: Name of back to preselect
/*--------------------------------------------------------------------------*/
ICarddeckSelectDlg::ICarddeckSelectDlg (const char* path, const std::string& deck,
                                        const std::string& back)
   : Dialog (), txtDecks (_("Available decks")), ok (_("OK"))
   , apply (_("Apply")), cancel (_("Cancel")), decks (), boxDecks ()
   , txtBack (_("Available backgrounds")), backs (), boxBack ()
   , selDeck (), selBack (), offDeck (-1), offBack (-1)
   , box (GTK_BUTTONBOX_END, 5), scrlBack (), scrlDeck () {
   TRACE3 ("CarddeckSelectDlg::CarddeckSelectDlg (const char*) - " << path
           << " (" << deck << " - " << back << ')');

   set_title (_("Select carddeck"));

   ok.set_flags (GTK_CAN_DEFAULT);
   cancel.set_flags (GTK_CAN_DEFAULT);
   apply.set_flags (GTK_CAN_DEFAULT);

   ok.set_usize (90, -1);
   cancel.set_usize (90, -1);
   apply.set_usize (90, -1);

   scrlDeck.set_policy (GTK_POLICY_AUTOMATIC, GTK_POLICY_AUTOMATIC);
   scrlBack.set_policy (GTK_POLICY_AUTOMATIC, GTK_POLICY_AUTOMATIC);

   boxDecks.pack_start (scrlDeck, true, true, 50);
   boxDecks.pack_start (selDeck, false, false, 5);
   scrlDeck.add_with_viewport (decks);

   boxBack.pack_start (scrlBack, true, true, 50);
   boxBack.pack_start (selBack, false, false, 5);
   scrlBack.add_with_viewport (backs);

   Check3 (get_vbox ());
   get_vbox ()->pack_start (txtDecks, false, false, 5);
   get_vbox ()->pack_start (boxDecks, true, true, 5);
   get_vbox ()->pack_start (txtBack, false, false, 5);
   get_vbox ()->pack_start (boxBack, true, true, 5);

   box.pack_start (ok, false, false, 5);
   box.pack_start (apply, false, false, 5);
   box.pack_start (cancel, false, false, 5);

   Check3 (get_action_area ());
   get_action_area ()->pack_start (box, true, true);

   ok.clicked.connect (bind (slot (this, &ICarddeckSelectDlg::command), OK));
   apply.clicked.connect (bind (slot (this, &ICarddeckSelectDlg::command), APPLY));
   cancel.clicked.connect (bind (slot (this, &ICarddeckSelectDlg::command), CANCEL));

   std::string cardDirs (path ? path : CARDSET_PATH);
   if (cardDirs.size ()
       && (cardDirs[cardDirs.size () - 1] != File::DIRSEPARATOR))
      cardDirs += File::DIRSEPARATOR;
   cardDirs += "Deck*";
   DirectorySearch ds (cardDirs);

   Gdk_Color color;

   TRACE8 ("ICarddeckSelectDlg::ICarddeckSelectDlg (const char*) - Searching in path "
           << cardDirs);
   const File* dir (ds.find (IDirectorySearch::FILE_DIRECTORY
                             | IDirectorySearch::FILE_READONLY));
   unsigned int offset (0);
   aFiles.push_back (dir->path ());

   show_all ();

   Gdk_Pixmap img;
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

      Button* temp (new Button ());
      temp->clicked.connect (bind (slot (this, &ICarddeckSelectDlg::deckSelect), ++offset));
      temp->show ();

      img.create_from_xpm (get_window (), color, file);
      temp->add_pixmap (img, NULL);
      aDecks.push_back (temp);

      decks.resize ((offset >> 2) + 1, 4);
      decks.attach (*temp, offset & 0x3, (offset & 0x3) + 1, offset >> 2,
                    (offset >> 2) + 1, 0, 0, 5, 5);

      TRACE9 ("ICarddeckSelectDlg::ICarddeckSelectDlg (const char*) - Comparing "
              << pathDeck << " with " << deck);
      if (pathDeck == deck)
         deckSelect (offset);

      dir = ds.next ();
   }
   if (offDeck == -1)
      deckSelect (1);

   TRACE9 ("ICarddeckSelectDlg::ICarddeckSelectDlg (const char*) - Decksize = "
           << (img.width () << 2) << '/'
           << img.height () * ((offset >> 2) + 1));
   unsigned int height ((img.height () + 20) * ((offset >> 2) + 1));
   scrlDeck.set_usize ((img.width () + 25) << 2,
                       height < 250 ? height : 250);

   unsigned int offsetBack (offset + 1);
   dir = ds.find (aFiles[0] + "/back*.xpm", IDirectorySearch::FILE_NORMAL
                  | IDirectorySearch::FILE_READONLY);
   while (dir) {
      TRACE9 ("ICarddeckSelectDlg::ICarddeckSelectDlg (const char*) - Reading "
              "background file " << dir->path () << dir->name ());
      aFiles.push_back (dir->name ());

      Button* temp (new Button ());
      temp->clicked.connect (bind (slot (this, &ICarddeckSelectDlg::backSelect), ++offset));
      temp->show ();

      TRACE9 ("ICarddeckSelectDlg::ICarddeckSelectDlg (const char*) - Comparing "
              << (aFiles[0] + dir->name ()) << " with " << back);
      if ((aFiles[0] + dir->name ()) == back)
         backSelect (offset);

      Gdk_Pixmap img;
      img.create_from_xpm (get_window (), color, aFiles[0] + dir->name ());
      temp->add_pixmap (img, NULL);
      aBacks.push_back (temp);

      backs.resize (((offset - offsetBack) >> 2) + 1, 4);
      backs.attach (*temp, (offset - offsetBack) & 0x3, ((offset - offsetBack) & 0x3) + 1,
                    (offset - offsetBack) >> 2, ((offset - offsetBack) >> 2) + 1,
                    0, 0, 5, 5);

      dir = ds.next ();
   }
   if (offBack == -1)
      backSelect (offsetBack);

   offset -= offBack;
   TRACE9 ("ICarddeckSelectDlg::ICarddeckSelectDlg (const char*) - Decksize = "
           << (img.width () << 2) << '/'
           << img.height () * ((offset >> 2) + 1));
   height = ((img.height () + 20) * ((offset >> 2) + 1));
   scrlBack.set_usize ((img.width () + 25) << 2,
                       height < 250 ? height : 250);

   ok.grab_default ();
}

/*--------------------------------------------------------------------------*/
//Purpose   : Destructor
/*--------------------------------------------------------------------------*/
ICarddeckSelectDlg::~ICarddeckSelectDlg () {
   TRACE9 ("CarddeckSelectDlg::~CarddeckSelectDlg ()");

   for (vector<Button*>::iterator i (aDecks.begin ());
        i != aDecks.end (); ++i)
      delete *i;

   for (vector<Button*>::iterator i (aBacks.begin ());
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

   selDeck.remove ();

   Gdk_Color color;
   Gdk_Pixmap img;
   img.create_from_xpm (get_window (), color, aFiles[0] + aFiles[offset]
                        + File::DIRSEPARATOR + DEFAULTFILE);
   selDeck.add_pixmap (img, NULL);
   selDeck.set_relief (GTK_RELIEF_NONE);
   dynamic_cast <Gtk::Pixmap*> (selDeck.get_child ())->set_alignment (0.0, 0.0);

   offDeck = offset;
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

   selBack.remove ();

   Gdk_Color color;
   Gdk_Pixmap img;
   img.create_from_xpm (get_window (), color, aFiles[0] + aFiles[offset]);
   selBack.add_pixmap (img, NULL);
   selBack.set_relief (GTK_RELIEF_NONE);

   dynamic_cast <Gtk::Pixmap*> (selBack.get_child ())->set_alignment (0.0, 0.0);

   offBack = offset;
}

/*--------------------------------------------------------------------------*/
//Purpose   : Callback after selecting a button
//Parameters: action: ID of selected button
/*--------------------------------------------------------------------------*/
void ICarddeckSelectDlg::command (commands action) {
   if (action != APPLY)
      delete this;
   else
      lock ();
}
