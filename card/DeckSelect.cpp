// PROJECT     : Cardgames
// SUBSYSTEM   : Common/DeckSelect
// REFERENCES  :
// TODO        :
// BUGS        :
// AUTHOR      : Markus Schwab
// CREATED     : 29.8.2002
// COPYRIGHT   : Copyright (C) 2002 - 2018, 2026

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

#include <algorithm>
#include <cstring>
#include <utility>

#include <glibmm/fileutils.h>
#include <glibmm/main.h>

#include <gdkmm/pixbuf.h>

#include <gtkmm/cssprovider.h>
#include <gtkmm/image.h>
#include <gtkmm/messagedialog.h>
#include <gtkmm/scrolledwindow.h>

// YGP/DirSrch.h evaluates HAVE_DIRENT_H before including ygp-cfg.h itself
#include <ygp-cfg.h>

#include <YGP/Check.h>
#include <YGP/DirSrch.h>
#include <YGP/Trace.h>

#include "Images.h"

#include "DeckSelect.h"

namespace Card {

//-----------------------------------------------------------------------------
/// Constructor; adds all controls to the dialog
/// \param deck Name of deck to preselect
/// \param back Name of back to preselect
//-----------------------------------------------------------------------------
DeckSelectDlg::DeckSelectDlg(std::string deck, std::string back)
    : XGP::XDialog(_("Select carddeck"), OKCANCEL), setDecks(), cols(), mDecks(), mBacks(), boxDecks(),
      txtDecks(_("Available decks")), selDeck(), decks(), rendererDecks(), selBack(), boxBack(),
      txtBack(_("Available backgrounds")), backs(), rendererBacks(), apply(nullptr), defDeck(std::move(deck)),
      defBack(std::move(back)), mxLoaded(), loaded(), imagesLoaded(), loader() {
    TRACE3("DeckSelectDlg::DeckSelectDlg(2x std::string) - " << defDeck << " - " << defBack);

    // Remark: Without an explicit default size, the window would grow to the
    //     natural (unconstrained) size of the icon-views, easily filling the
    //     screen
    set_default_size(800, 600);

    Gtk::ScrolledWindow* scrl(Gtk::make_managed<Gtk::ScrolledWindow>());
    scrl->set_has_frame(true);
    scrl->set_child(decks);
    scrl->set_policy(Gtk::PolicyType::NEVER, Gtk::PolicyType::AUTOMATIC);
    scrl->set_propagate_natural_width(true);
    scrl->set_propagate_natural_height(true);
    scrl->set_max_content_width(600);
    scrl->set_max_content_height(250);
    scrl->set_hexpand();
    scrl->set_vexpand();
    scrl->set_margin(5);

    boxDecks.append(*scrl);
    // Remark: The picture is put into an own (vertical) box, to reserve exactly
    //     the space of a card. Directly in the horizontal box it would be
    //     measured for the whole height of that box; and - having an image -
    //     ask for the width of the image scaled to that height
    selDeck.set_size_request(Images::WIDTH, Images::HEIGHT);
    selDeck.set_halign(Gtk::Align::CENTER);
    Gtk::Box* boxSel(Gtk::make_managed<Gtk::Box>(Gtk::Orientation::VERTICAL));
    boxSel->set_valign(Gtk::Align::CENTER);
    boxSel->set_margin(5);
    boxSel->append(selDeck);
    boxDecks.append(*boxSel);

    scrl = Gtk::make_managed<Gtk::ScrolledWindow>();
    scrl->set_has_frame(true);
    scrl->set_child(backs);
    scrl->set_policy(Gtk::PolicyType::NEVER, Gtk::PolicyType::AUTOMATIC);
    scrl->set_propagate_natural_width(true);
    scrl->set_propagate_natural_height(true);
    scrl->set_max_content_width(600);
    scrl->set_max_content_height(250);
    scrl->set_hexpand();
    scrl->set_vexpand();
    scrl->set_margin(5);

    boxBack.append(*scrl);
    selBack.set_size_request(Images::WIDTH, Images::HEIGHT);
    selBack.set_halign(Gtk::Align::CENTER);
    boxSel = Gtk::make_managed<Gtk::Box>(Gtk::Orientation::VERTICAL);
    boxSel->set_valign(Gtk::Align::CENTER);
    boxSel->set_margin(5);
    boxSel->append(selBack);
    boxBack.append(*boxSel);

    Check3(get_content_area());
    txtDecks.set_margin(5);
    get_content_area()->append(txtDecks);
    boxDecks.set_hexpand();
    boxDecks.set_vexpand();
    boxDecks.set_margin(5);
    get_content_area()->append(boxDecks);
    txtBack.set_margin(5);
    get_content_area()->append(txtBack);
    boxBack.set_hexpand();
    boxBack.set_vexpand();
    boxBack.set_margin(5);
    get_content_area()->append(boxBack);

    mDecks = Gtk::ListStore::create(cols);
    decks.set_model(mDecks);

    decks.signal_selection_changed().connect(mem_fun(*this, &DeckSelectDlg::deckSelected));
    decks.signal_item_activated().connect(mem_fun(*this, &DeckSelectDlg::deckActivated));

    resize(decks, 0);

    mBacks = Gtk::ListStore::create(cols);
    backs.set_model(mBacks);
    backs.signal_selection_changed().connect(mem_fun(*this, &DeckSelectDlg::backSelected));
    backs.signal_item_activated().connect(mem_fun(*this, &DeckSelectDlg::backActivated));
    resize(backs, 0);

    // Nothing can be confirmed, until at least one image is loaded
    apply = add_button(_("_Apply"), static_cast<int>(Gtk::ResponseType::APPLY));
    apply->set_sensitive(false);
    ok->set_sensitive(false);

    // Remark: Not using set_pixbuf_column(), as GtkCellRendererPixbuf only
    // sizes correctly by the image's real dimensions when bound to a texture
    decks.pack_start(rendererDecks);
    decks.add_attribute(rendererDecks, "texture", cols.texture);
    decks.set_text_column(cols.name);

    backs.pack_start(rendererBacks);
    backs.add_attribute(rendererBacks, "texture", cols.texture);

    // Remark: GtkCellRendererPixbuf measures a cell using the image's real
    // size, but *draws* it clamped to the CSS -gtk-icon-size (16px default);
    // raise that ceiling so the actual image fills the (correctly-sized) cell
    Glib::RefPtr<Gtk::CssProvider> cssIconSize(Gtk::CssProvider::create());
    cssIconSize->load_from_string("iconview { -gtk-icon-size: 999px; }");
    decks.get_style_context()->add_provider(cssIconSize, GTK_STYLE_PROVIDER_PRIORITY_APPLICATION);
    backs.get_style_context()->add_provider(cssIconSize, GTK_STYLE_PROVIDER_PRIORITY_APPLICATION);

    show();

    // Display the dialog at once; the images are loaded in the background and
    // added to the dialog as they become available
    imagesLoaded.connect(mem_fun(*this, &DeckSelectDlg::showLoadedImages));
    loader = std::jthread([this](const std::stop_token& stop) { loadImages(stop); });
}

//-----------------------------------------------------------------------------
/// Destructor
//-----------------------------------------------------------------------------
DeckSelectDlg::~DeckSelectDlg() {
    TRACE9("DeckSelectDlg::~DeckSelectDlg()");

    // Don't let the thread access the object while it is being destroyed
    loader.request_stop();
    if (loader.joinable())
        loader.join();
}

//-----------------------------------------------------------------------------
/// Loads the images of all decks and backs; executed in an own thread
/// \param stop Token signalling that loading should be aborted
//-----------------------------------------------------------------------------
void DeckSelectDlg::loadImages([[maybe_unused]] const std::stop_token& stop) {
    TRACE3("DeckSelectDlg::loadImages(const std::stop_token&)");

#ifdef KDECARDS_DIR
    // First try to load cards stored KDE-3 style(as separate PNGs)
    std::string cardDirs(KDECARDS_DIR);
    Check3(cardDirs[cardDirs.size() - 1] == YGP::File::DIRSEPARATOR);
    cardDirs += "cards-*";
    addFileInDirectories(stop, cardDirs, "11.png", 6);

#    ifdef HAVE_RSVG
    // Then try to load them KDE-4 style(as SVGs, with one PNG to use in the
    // select-dialog) Does not work with librsvg <= 2.26.0
    cardDirs = KDECARDS_DIR;
    Check3(cardDirs[cardDirs.size() - 1] == YGP::File::DIRSEPARATOR);
    cardDirs += "svg-*";
    addFileInDirectories(stop, cardDirs, "11.png", 4);
#    endif

#endif
#ifdef CARDPICS_DIR
    if (stop.stop_requested())
        return;
    addFile(CARDPICS_DIR, "40.png", "Cardpics");
#endif

#ifdef KDECARDS_DIR
    std::string pathDecks(KDECARDS_DIR "decks/");
    YGP::DirectorySearch ds;
    const YGP::File* dir(
        ds.find(pathDecks + "deck*.png", YGP::IDirectorySearch::FILE_NORMAL | YGP::IDirectorySearch::FILE_READONLY));
    while (dir && !stop.stop_requested()) {
        addBack(pathDecks + dir->name());
        dir = ds.next();
    }
#endif

#ifdef CARDPICS_DIR
    if (stop.stop_requested())
        return;
    addBack(CARDPICS_DIR "78.png");
#endif

#ifdef GNOMECARDS_DIR
    YGP::DirectorySearch gs(GNOMECARDS_DIR "*");
    const YGP::File* gfile(gs.find(YGP::IDirectorySearch::FILE_NORMAL | YGP::IDirectorySearch::FILE_READONLY));
    while (gfile && !stop.stop_requested()) {
        std::string file(GNOMECARDS_DIR);
        file += gfile->name();
        Glib::RefPtr<Gdk::Pixbuf> actImg(getImage(file, false));
        if (actImg) {
            // The file contains all cards (13 columns in 5 rows); use the
            // jack of hearts for the deck and the back for the back
            unsigned int widthImg(actImg->get_width() / 13);
            unsigned int heightImg(actImg->get_height() / 5);

            Glib::RefPtr<Gdk::Pixbuf> dest(
                Gdk::Pixbuf::create_subpixbuf(actImg, widthImg * 11, heightImg * 2, widthImg, heightImg));
            queue(LoadedImage::DECK, file,
                  file.substr(std::strlen(GNOMECARDS_DIR), file.rfind('.') - std::strlen(GNOMECARDS_DIR)),
                  dest->scale_simple(Images::WIDTH, Images::HEIGHT, Gdk::InterpType::BILINEAR));

            dest = Gdk::Pixbuf::create_subpixbuf(actImg, widthImg << 1, heightImg << 2, widthImg, heightImg);
            queue(LoadedImage::BACK, file, {}, dest->scale_simple(Images::WIDTH, Images::HEIGHT, Gdk::InterpType::BILINEAR));
        }

        gfile = gs.next();
    }
#endif
}

//-----------------------------------------------------------------------------
/// Hands a loaded image (or an error) over to the GUI thread
/// \param type Type of the entry
/// \param path Path of the deck/back; or the error-message
/// \param name Name to display
/// \param icon Image to display
//-----------------------------------------------------------------------------
void DeckSelectDlg::queue(LoadedImage::Type type, const std::string& path, const Glib::ustring& name,
                          const Glib::RefPtr<Gdk::Pixbuf>& icon) {
    Glib::RefPtr<Gdk::Texture> texture;
    if (icon)
        texture = Gdk::Texture::create_for_pixbuf(icon);

    {
        std::scoped_lock lock(mxLoaded);
        loaded.emplace_back(type, path, name, icon, texture);
    }
    imagesLoaded.emit();
}

//-----------------------------------------------------------------------------
/// Adds the images loaded in the background to the lists
//-----------------------------------------------------------------------------
void DeckSelectDlg::showLoadedImages() {
    std::vector<LoadedImage> images;
    {
        std::scoped_lock lock(mxLoaded);
        images.swap(loaded);
    }
    TRACE8("DeckSelectDlg::showLoadedImages() - " << images.size());

    for (const LoadedImage& image : images) {
        if (image.type == LoadedImage::ERROR) {
            showError(image.path);
            continue;
        }

        const bool isBack(image.type == LoadedImage::BACK);
        const Glib::RefPtr<Gtk::ListStore>& model(isBack ? mBacks : mDecks);
        Gtk::IconView& view(isBack ? backs : decks);

        Gtk::TreeModel::iterator iRow(model->append());
        Gtk::TreeRow row(*iRow);
        row[cols.path] = image.path;
        row[cols.name] = image.name;
        row[cols.icon] = image.icon;
        row[cols.texture] = image.texture;
        resize(view, model->children().size());

        TRACE9("DeckSelectDlg::showLoadedImages() - Comparing " << image.path << " with " << (isBack ? defBack : defDeck));
        // Don't overrule a selection the user made while the images are still loading
        if ((image.path == (isBack ? defBack : defDeck)) && view.get_selected_items().empty())
            view.select_path(model->get_path(iRow));

        apply->set_sensitive();
        ok->set_sensitive();
    }
}

//-----------------------------------------------------------------------------
/// Displays an error-message (without blocking the dialog)
/// \param msg Message to display
//-----------------------------------------------------------------------------
void DeckSelectDlg::showError(const std::string& msg) {
    Gtk::MessageDialog* dlg(new Gtk::MessageDialog(*this, msg, false, Gtk::MessageType::ERROR, Gtk::ButtonsType::OK, true));
    dlg->signal_response().connect([dlg](int) { delete dlg; });
    dlg->show();
}

//-----------------------------------------------------------------------------
/// Sets the size of the passed view according to the number of its images
/// \param view View to resize
/// \param images Number of images in the view
//-----------------------------------------------------------------------------
void DeckSelectDlg::resize(Gtk::IconView& view, size_t images) {
    unsigned int height(132 * ((static_cast<unsigned int>(images) >> 2) + 1));
    view.set_size_request((images > 3) ? 20 + (88 << 2) : 20 + 88 * static_cast<int>(images), std::min(height, 270U));
}

//-----------------------------------------------------------------------------
/// Adds the passed files for all directories found by the DirectorySearch
/// \param stop Token signalling that loading should be aborted
/// \param dir File-regular expression where carddecks can be found
/// \param file File within all the directories
/// \param offName Offset where name starts in filename
//-----------------------------------------------------------------------------
void DeckSelectDlg::addFileInDirectories(const std::stop_token& stop, const std::string& dir, const std::string& file,
                                         unsigned int offName) {
    Check1(dir.size());
    Check1(file.size());
    TRACE8("DeckSelectDlg::addFileInDirectories(const std::stop_token&, 2x const std::string&, unsigned) "
           "- Searching in path "
           << dir);

    std::string path;
    YGP::DirectorySearch ds(dir.c_str());
    const YGP::File* iDir(ds.find(YGP::IDirectorySearch::FILE_DIRECTORY | YGP::IDirectorySearch::FILE_READONLY));
    while (iDir && !stop.stop_requested()) {
        path = iDir->path();
        path += iDir->name();
        path += YGP::File::DIRSEPARATOR;
        addFile(path, file, iDir->name() + offName);

        iDir = ds.next();
    }
}

//-----------------------------------------------------------------------------
/// Loads the passed file and adds it to the list of decks
/// \param path Path to file to add
/// \param name File to add
/// \param display Name as displayed in the list
//-----------------------------------------------------------------------------
void DeckSelectDlg::addFile(const std::string& path, const std::string& name, const std::string& display) {
    TRACE3("DeckSelectDlg::addFile(3x const std::string&) - Adding " << path << name << " as " << display);
    Check1(path.size());
    Check1(name.size());
    Check1(display.size());

    Glib::RefPtr<Gdk::Pixbuf> actImg(getImage(path + name));
    if (actImg)
        queue(LoadedImage::DECK, path, display, actImg);
}

//-----------------------------------------------------------------------------
/// Loads the passed file and adds it to the list of backs
/// \param file File to add
//-----------------------------------------------------------------------------
void DeckSelectDlg::addBack(const std::string& file) {
    TRACE3("DeckSelectDlg::addBack(const std::string&) - Adding " << file);

    Glib::RefPtr<Gdk::Pixbuf> actImg(getImage(file));
    if (actImg)
        queue(LoadedImage::BACK, file, {}, actImg);
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
/// Returns an image specified by the passed file; errors are reported to
/// the GUI thread
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
        if (scale && ((imgBuf->get_height() != static_cast<int>(Images::HEIGHT)) ||
                      (imgBuf->get_width() != static_cast<int>(Images::WIDTH))))
            imgBuf = imgBuf->scale_simple(Images::WIDTH, Images::HEIGHT, Gdk::InterpType::BILINEAR);
    }
    catch (Gdk::PixbufError& e) {
        err = e.what();
    }
    catch (Glib::FileError& e) {
        err = e.what();
    }
    catch (...) {
        err = _("Unknown error");
    }
    if (err.size()) {
        std::string msg(_("Error loading image from file `%1'!\n\nReason: %2"));
        msg.replace(msg.find("%1"), 2, file);
        msg.replace(msg.find("%2"), 2, err);
        queue(LoadedImage::ERROR, msg, {}, {});
        imgBuf.reset();
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

        selDeck.set_pixbuf(img);
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

        selBack.set_pixbuf(img);
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
        Gtk::TreePath pathBack(*(backs.get_selected_items().begin()));
        Gtk::TreeRow rowBack(*mBacks->get_iter(pathBack));
        back = rowBack[cols.path];
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
        Gtk::TreePath pathDeck(*(decks.get_selected_items().begin()));
        Gtk::TreeRow rowDeck(*mDecks->get_iter(pathDeck));
        deck = rowDeck[cols.path];
    }
    setDecks.emit(deck, back);
}

} // namespace Card
