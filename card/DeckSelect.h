#ifndef DECKSELECT_H
#define DECKSELECT_H

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

#include <map>
#include <mutex>
#include <stop_token>
#include <string>
#include <thread>
#include <vector>

#include <glibmm/dispatcher.h>

#include <gdkmm/pixbuf.h>
#include <gdkmm/texture.h>

#include <gtkmm/box.h>
#include <gtkmm/cellrendererpixbuf.h>
#include <gtkmm/iconview.h>
#include <gtkmm/label.h>
#include <gtkmm/liststore.h>
#include <gtkmm/picture.h>

#include <XGP/XDialog.h>

#include <card/Pile.h>

namespace Card {

// Class to select the card decks to use
class DeckSelectDlg : public XGP::XDialog {
  private:
    /**Class describing the columns in the deck-lists
     */
    class DeckColumns : public Gtk::TreeModel::ColumnRecord {
      public:
        DeckColumns() : path(), name(), icon(), texture() {
            add(path);
            add(name);
            add(icon);
            add(texture);
        }

        Gtk::TreeModelColumn<std::string> path;
        Gtk::TreeModelColumn<Glib::ustring> name;
        Gtk::TreeModelColumn<Glib::RefPtr<Gdk::Pixbuf>> icon;
        Gtk::TreeModelColumn<Glib::RefPtr<Gdk::Texture>> texture;
    };

  public:
    DeckSelectDlg(std::string deck, std::string back);
    ~DeckSelectDlg() override;

    /// Creates a DeckSelect-dialog on the heap
    /// \param deck Pre-selected deck
    /// \param back Pre-selected back
    /// \returns DeckSelectDlg* Created dialog
    static DeckSelectDlg* create(const std::string& deck, const std::string& back) {
        DeckSelectDlg* dlg(new DeckSelectDlg(deck, back));
        dlg->signal_response().connect(mem_fun(*dlg, &DeckSelectDlg::free));
        return dlg;
    }

    /// Signal emitted, when changing deck/back is confirmed
    sigc::signal<void(const std::string&, const std::string&)> setDecks;

  protected:
    void okEvent() override;
    void command(int action) override;

    // Remark: The following methods are executed by the thread loading the
    //     images; they must not access the widgets of the dialog
    void loadImages(const std::stop_token& stop);
    void preloadSelected();
    Glib::RefPtr<Gdk::Pixbuf> getImage(const std::string& file, bool scale = true, bool reportError = true);
    void addFileInDirectories(const std::stop_token& stop, const std::string& dir, const std::string& file,
                              unsigned int offName = 0);
    void addFile(const std::string& path, const std::string& name, const std::string& display);
    void addBack(const std::string& file);

  private:
    /// An image loaded in the background (or the error occured while trying so)
    struct LoadedImage {
        enum Type { DECK, BACK, PREVIEW_DECK, PREVIEW_BACK, ERROR };

        Type type{};
        std::string path; ///< Path of deck/back; or the error-message
        Glib::ustring name;
        Glib::RefPtr<Gdk::Pixbuf> icon;
        Glib::RefPtr<Gdk::Texture> texture;
    };

    // Prohibited manager-functions
    DeckSelectDlg() = delete;
    DeckSelectDlg(const DeckSelectDlg&) = delete;

    void queue(LoadedImage::Type type, const std::string& path, const Glib::ustring& name, const Glib::RefPtr<Gdk::Pixbuf>& icon);
    void showLoadedImages();
    void showError(const std::string& msg);
    static void resize(Gtk::IconView& view, size_t images);

    void deckSelected();
    void backSelected();

    void deckActivated(const Gtk::TreeModel::Path& deck);
    void backActivated(const Gtk::TreeModel::Path& back);

    const DeckSelectDlg& operator=(const DeckSelectDlg&) = delete;

    DeckColumns cols;
    Glib::RefPtr<Gtk::ListStore> mDecks;
    Glib::RefPtr<Gtk::ListStore> mBacks;

    Card::HBox boxDecks;
    Gtk::Label txtDecks;
    Gtk::Picture selDeck;
    Gtk::IconView decks;
    Gtk::CellRendererPixbuf rendererDecks;

    Gtk::Picture selBack;
    Card::HBox boxBack;
    Gtk::Label txtBack;
    Gtk::IconView backs;
    Gtk::CellRendererPixbuf rendererBacks;
    Gtk::Button* apply;

    std::string defDeck; ///< Deck to preselect
    std::string defBack; ///< Back to preselect

    std::mutex mxLoaded;             ///< Protects the loaded images
    std::vector<LoadedImage> loaded; ///< Images loaded, but not yet displayed
    /// Images of the preselected deck/back, which are loaded ahead of all others; only used by the loading thread
    std::map<std::string, Glib::RefPtr<Gdk::Pixbuf>> preloaded;
    Glib::Dispatcher imagesLoaded; ///< Informs the GUI thread about loaded images
    // Remark: Must be the last member, so it is stopped before the others are destroyed
    std::jthread loader; ///< Thread loading the images
};

} // namespace Card

#endif
