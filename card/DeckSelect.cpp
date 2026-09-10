//PROJECT     : Cardgames
//SUBSYSTEM   : Common/DeckSelect
//REFERENCES  :
//TODO        :
//BUGS        :
//AUTHOR      : Markus Schwab
//CREATED     : 29.8.2002
//COPYRIGHT   : Copyright (C) 2002 - 2018

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

#include <glibmm/fileutils.h>

#include <gdkmm/pixbuf.h>

#include <gtkmm/image.h>
#include <gtkmm/messagedialog.h>
#include <gtkmm/scrolledwindow.h>

#include <ygp-cfg.h>
#include <YGP/Check.h>
#include <YGP/Trace.h>
#include <YGP/DirSrch.h>

#include "Images.h"

#include "DeckSelect.h"


namespace Card {

//-----------------------------------------------------------------------------
/// Constructor; adds all controls to the dialog
/// \param deck Name of deck to preselect
/// \param back Name of back to preselect
//-----------------------------------------------------------------------------
DeckSelectDlg::DeckSelectDlg(const std::string& deck, const std::string& back)
   : XGP::XDialog(_("Select carddeck"), OKCANCEL),
     setDecks(), cols(), mDecks(), mBacks(), boxDecks(), txtDecks(_("Available decks")),
     selDeck(), decks(), selBack(), boxBack(), txtBack(_("Available backgrounds")),
     backs() {
   TRACE3("DeckSelectDlg::DeckSelectDlg(2x const std::string&) - " << deck << " - " << back);

   Gtk::ScrolledWindow* scrl(Gtk::make_managed<Gtk::ScrolledWindow>());
   scrl->set_has_frame(true);
   scrl->set_child(decks);
   scrl->set_policy(Gtk::PolicyType::NEVER, Gtk::PolicyType::AUTOMATIC);
   scrl->set_hexpand(); scrl->set_vexpand();
   scrl->set_margin(50);

   boxDecks.append(*scrl);
   selDeck.set_margin(5);
   boxDecks.append(selDeck);

   scrl = Gtk::make_managed<Gtk::ScrolledWindow>();
   scrl->set_has_frame(true);
   scrl->set_child(backs);
   scrl->set_policy(Gtk::PolicyType::NEVER, Gtk::PolicyType::AUTOMATIC);
   scrl->set_hexpand(); scrl->set_vexpand();
   scrl->set_margin(50);

   boxBack.append(*scrl);
   selBack.set_margin(5);
   boxBack.append(selBack);

   Check3(get_content_area());
   txtDecks.set_margin(5);
   get_content_area()->append(txtDecks);
   boxDecks.set_hexpand(); boxDecks.set_vexpand();
   boxDecks.set_margin(5);
   get_content_area()->append(boxDecks);
   txtBack.set_margin(5);
   get_content_area()->append(txtBack);
   boxBack.set_hexpand(); boxBack.set_vexpand();
   boxBack.set_margin(5);
   get_content_area()->append(boxBack);

   mDecks = Gtk::ListStore::create(cols);
   decks.set_model(mDecks);

   decks.signal_selection_changed().connect(mem_fun(*this, &DeckSelectDlg::deckSelected));
   decks.signal_item_activated().connect(mem_fun(*this, &DeckSelectDlg::deckActivated));

#ifdef KDECARDS_DIR
   // First try to load cards stored KDE-3 style(as separate PNGs)
   std::string cardDirs(KDECARDS_DIR); Check3(cardDirs[cardDirs.size() - 1] == YGP::File::DIRSEPARATOR);
   cardDirs += "cards-*";
   addFileInDirectories(cardDirs, "11.png", deck, 6);

#ifdef HAVE_RSVG
   // Then try to load them KDE-4 style(as SVGs, with one PNG to use in the select-dialog)
   // Does not work with librsvg <= 2.26.0
   cardDirs = KDECARDS_DIR; Check3(cardDirs[cardDirs.size() - 1] == YGP::File::DIRSEPARATOR);
   cardDirs += "svg-*";
   addFileInDirectories(cardDirs, "11.png", deck, 4);
#endif

#endif
#ifdef CARDPICS_DIR
   addFile(CARDPICS_DIR, "40.png", "Cardpics", deck);
#endif

   unsigned int height(132 *((mDecks->children().size() >> 2) + 1));
   decks.set_size_request((mDecks->children().size() > 3) ? 20 +(88 << 2) : 20 + 88 * mDecks->children().size(),
			   height < 270 ? height : 270);

   mBacks = Gtk::ListStore::create(cols);
   backs.set_model(mBacks);
   backs.signal_selection_changed().connect(mem_fun(*this, &DeckSelectDlg::backSelected));
   backs.signal_item_activated().connect(mem_fun(*this, &DeckSelectDlg::backActivated));

   Glib::RefPtr<Gdk::Pixbuf> actImg;
#ifdef KDECARDS_DIR
   std::string pathDecks(KDECARDS_DIR "decks/");
   YGP::DirectorySearch ds;
   const YGP::File* dir(ds.find(pathDecks + "deck*.png", YGP::IDirectorySearch::FILE_NORMAL
				  | YGP::IDirectorySearch::FILE_READONLY));
   while (dir) {
      if ((actImg = getImage(pathDecks + dir->name()))) {
	 Gtk::TreeModel::iterator iRow(mBacks->append());
	 Gtk::TreeRow row(*iRow);
	 row[cols.path] = pathDecks + dir->name();
	 row[cols.icon] = actImg;

	 TRACE9("DeckSelectDlg::DeckSelectDlg(2x const std::string&) - Comparing "
		 << (pathDecks + dir->name()) << " with " << back);
	 if ((pathDecks + dir->name()) == back)
	    backs.select_path(mBacks->get_path(iRow));
      }

      dir = ds.next();
   }
#endif

#ifdef CARDPICS_DIR
   if ((actImg = getImage(CARDPICS_DIR "78.png"))) {
      Gtk::TreeModel::iterator iRow(mBacks->append());
      Gtk::TreeRow row(*iRow);
      row[cols.path] = CARDPICS_DIR "78.png";
      row[cols.icon] = actImg;

      TRACE9("DeckSelectDlg::DeckSelectDlg(2x const std::string&) - Comparing "
             << CARDPICS_DIR "78.png" << " with " << deck);
      if (back == CARDPICS_DIR "78.png")
	 backs.select_path(mBacks->get_path(iRow));
   }
#endif

#ifdef GNOMECARDS_DIR
   YGP::DirectorySearch gs(GNOMECARDS_DIR "*");
   const YGP::File* gfile(gs.find(YGP::IDirectorySearch::FILE_NORMAL | YGP::IDirectorySearch::FILE_READONLY));
   while (gfile) {
      Gtk::TreeModel::iterator iRow(mDecks->append());
      Gtk::TreeRow row(*iRow);
      std::string file(GNOMECARDS_DIR);
      file += gfile->name();
      actImg = getImage(file, false);
      if (actImg) {
	 unsigned int widthImg(actImg->get_width() / 13);
	 unsigned int heightImg(actImg->get_height() / 5);

	 row[cols.path] = file;
	 row[cols.name] = file.substr(strlen(GNOMECARDS_DIR), file.rfind('.') - strlen(GNOMECARDS_DIR));
	 Glib::RefPtr<Gdk::Pixbuf> dest(Gdk::Pixbuf::create_subpixbuf(actImg, widthImg * 11, heightImg * 2, widthImg, heightImg));
	 row[cols.icon] = dest->scale_simple(Images::WIDTH, Images::HEIGHT, Gdk::InterpType::BILINEAR);

	 TRACE9("DeckSelectDlg::DeckSelectDlg(2x const std::string&) - Comparing "
                << (std::string(GNOMECARDS_DIR) + gfile->name()) << " with " << deck);
	 if (deck == (std::string(GNOMECARDS_DIR) + gfile->name()))
	    decks.select_path(mDecks->get_path(iRow));

	 iRow = mBacks->append();
	 row = *iRow;
	 row[cols.path] = file;
	 dest = Gdk::Pixbuf::create_subpixbuf(actImg, widthImg << 1, heightImg << 2, widthImg, heightImg);
	 row[cols.icon] = dest->scale_simple(Images::WIDTH, Images::HEIGHT, Gdk::InterpType::BILINEAR);

	 TRACE9("DeckSelectDlg::DeckSelectDlg(2x const std::string&) - Comparing "
		 << (std::string(GNOMECARDS_DIR) + gfile->name()) << " with " << back);
	 if (back == (std::string(GNOMECARDS_DIR) + gfile->name()))
	    backs.select_path(mBacks->get_path(iRow));
      }

      gfile = gs.next();
   }
#endif

   height = 132 *((mBacks->children().size() >> 2) + 1);
   backs.set_size_request((mBacks->children().size() > 3) ? 20 +(88 << 2) : 20 + 88 * mBacks->children().size(),
			   height < 270 ? height : 270);

   if (mDecks->children().size() || mBacks->children().size())
      add_button(_("_Apply"), static_cast<int>(Gtk::ResponseType::APPLY));
   else
      ok->set_sensitive(false);

   decks.set_pixbuf_column(cols.icon);
   decks.set_text_column(cols.name);
   backs.set_pixbuf_column(cols.icon);

   show();
}

//-----------------------------------------------------------------------------
/// Destructor
//-----------------------------------------------------------------------------
DeckSelectDlg::~DeckSelectDlg() {
   TRACE9("DeckSelectDlg::~DeckSelectDlg()");
}


//-----------------------------------------------------------------------------
/// Adds the passed files for all directories found by the DirectorySearch
/// \param dir File-regular expression where carddecks can be found
/// \param file File within all the directories
/// \param defaultDeck Previously selected deck
/// \param offName Offset where name starts in filename
//-----------------------------------------------------------------------------
void DeckSelectDlg::addFileInDirectories(const std::string& dir, const std::string& file,
					  const std::string& defaultDeck, unsigned int offName) {
   Check1(dir.size()); Check1(file.size()); Check1(defaultDeck.size());
   TRACE8("DeckSelectDlg::addFileInDirectories(3x const std::string&, unsigned) - Searching in path " << dir);

   std::string path;
   YGP::DirectorySearch ds(dir.c_str());
   const YGP::File* iDir(ds.find(YGP::IDirectorySearch::FILE_DIRECTORY
				   | YGP::IDirectorySearch::FILE_READONLY));
   while (iDir) {
      path = iDir->path();
      path += iDir->name();
      path += YGP::File::DIRSEPARATOR;
      addFile(path, file, iDir->name() + offName, defaultDeck);

      iDir = ds.next();
   }
}

//-----------------------------------------------------------------------------
/// Adds the passed file to the list
/// \param path Path to file to add
/// \param name File to add
/// \param display Name as displayed in the list
/// \param defaultDeck Previously selected deck
//-----------------------------------------------------------------------------
void DeckSelectDlg::addFile(const std::string& path, const std::string& name,
			     const std::string& display, const std::string& defaultDeck) {
   TRACE3("DeckSelectDlg::addFile(4x const std::string&) - Adding " << path << name << " as " << display);
   Check1(path.size()); Check1(name.size()); Check1(display.size());

   Glib::RefPtr<Gdk::Pixbuf> actImg(getImage(path + name));
   if (actImg) {
      Gtk::TreeModel::iterator iRow(mDecks->append());
      Gtk::TreeRow row(*iRow);
      row[cols.icon] = actImg;
      row[cols.path] = path;
      row[cols.name] = display;

      TRACE9("DeckSelectDlg::addFile(4x const std::string&) - Comparing " << (std::string)row[cols.path] << " with " << defaultDeck);
      if (defaultDeck == path)
	 decks.select_path(mDecks->get_path(iRow));
   }
}

//-----------------------------------------------------------------------------
/// Callback after selecting a button
/// \param action ID of selected button
//-----------------------------------------------------------------------------
void DeckSelectDlg::command(int action) {
   TRACE9("DeckSelectDlg::command(int) - Command: " << action);
   if (action == static_cast<int>(Gtk::ResponseType::APPLY)) {
      std::string deck, back;
      if (decks.get_selected_items().size()) {
	 Gtk::TreePath path(*(decks.get_selected_items().begin()));
	 Gtk::TreeRow row(*mDecks->get_iter(path));
	 deck = row[cols.path];
      }

      if (backs.get_selected_items().size()) {
	 Gtk::TreePath path(*(backs.get_selected_items().begin()));
	 Gtk::TreeRow row(*mBacks->get_iter(path));
	 back = row[cols.path];
      }
      setDecks.emit(deck, back);
   }
}

//-----------------------------------------------------------------------------
/// Callback after selecting the OK button
//-----------------------------------------------------------------------------
void DeckSelectDlg::okEvent() {
   command(static_cast<int>(Gtk::ResponseType::APPLY));
   XGP::XDialog::okEvent();
}

//-----------------------------------------------------------------------------
/// Returns an image specified by the passed file
/// \param file File containing the image
/// \param scale Flag, if image should be scaled
/// \returns Glib::RefPtr<Gdk::Pixbuf> Created image
//-----------------------------------------------------------------------------
Glib::RefPtr<Gdk::Pixbuf> DeckSelectDlg::getImage(const std::string& file, bool scale) {
   TRACE9("DeckSelectDlg::getImage(const std::string&, bool) - " << file);
   Glib::RefPtr<Gdk::Pixbuf> imgBuf;
   std::string err;

   try {
      imgBuf = Gdk::Pixbuf::create_from_file(file.c_str());
      if (scale &&((imgBuf->get_height() != (int)Images::WIDTH) ||(imgBuf->get_width() != (int)Images::HEIGHT)))
	 imgBuf = imgBuf->scale_simple(Images::WIDTH, Images::HEIGHT, Gdk::InterpType::BILINEAR);
   }
   catch(Gdk::PixbufError& e) {
      err = e.what();
   }
   catch(Glib::FileError& e) {
      err = e.what();
   }
   catch(...) {
      err = _("Unknown error");
   }
   if (err.size()) {
      std::string msg(_("Error loading image from file `%1'!\n\nReason: %2"));
      msg.replace(msg.find("%1"), 2, file);
      msg.replace(msg.find("%2"), 2, err);
      Gtk::MessageDialog dlg(msg, false, Gtk::MessageType::ERROR, Gtk::ButtonsType::OK);
      XGP::runModal(dlg);
   }

   return imgBuf;
}

//-----------------------------------------------------------------------------
/// Callback after changing the selected deck
//-----------------------------------------------------------------------------
void DeckSelectDlg::deckSelected() {
   TRACE8("DeckSelectDlg::deckSelected()");

   if (decks.get_selected_items().size()) {
      Gtk::TreePath path(*(decks.get_selected_items().begin()));
      Gtk::TreeRow row(*mDecks->get_iter(path));
      Glib::RefPtr<Gdk::Pixbuf> img(row[cols.icon]);

      selDeck.set(img);
   }
}

//-----------------------------------------------------------------------------
/// Callback after changing the selected back
//-----------------------------------------------------------------------------
void DeckSelectDlg::backSelected() {
   TRACE8("DeckSelectDlg::backSelected()");

   if (backs.get_selected_items().size()) {
      Gtk::TreePath path(*(backs.get_selected_items().begin()));
      Gtk::TreeRow row(*mBacks->get_iter(path));
      Glib::RefPtr<Gdk::Pixbuf> img(row[cols.icon]);

      selBack.set(img);
   }
}

//-----------------------------------------------------------------------------
/// Callback when a deck is activated
/// \param path Activated deck
//-----------------------------------------------------------------------------
void DeckSelectDlg::deckActivated(const Gtk::TreeModel::Path& path) {
   TRACE8("DeckSelectDlg::deckActivated(const Gtk::TreeModel::Path&)");

   std::string deck, back;
   Gtk::TreeRow row(*mDecks->get_iter(path));
   deck = row[cols.path];

   if (backs.get_selected_items().size()) {
      Gtk::TreePath path(*(backs.get_selected_items().begin()));
      Gtk::TreeRow row(*mBacks->get_iter(path));
      back = row[cols.path];
   }
   setDecks.emit(deck, back);
}

//-----------------------------------------------------------------------------
/// Callback when a back is activated
/// \param path Activated back
//-----------------------------------------------------------------------------
void DeckSelectDlg::backActivated(const Gtk::TreeModel::Path& path) {
   TRACE8("DeckSelectDlg::backActivated(const Gtk::TreeModel::Path&)");

   std::string deck, back;
   Gtk::TreeRow row(*mBacks->get_iter(path));
   back = row[cols.path];

   if (decks.get_selected_items().size()) {
      Gtk::TreePath path(*(decks.get_selected_items().begin()));
      Gtk::TreeRow row(*mDecks->get_iter(path));
      deck = row[cols.path];
   }
   setDecks.emit(deck, back);
}

}
