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

#include <string>

#include <gdkmm/texture.h>

#include <gtkmm/box.h>
#include <gtkmm/cellrendererpixbuf.h>
#include <gtkmm/iconview.h>
#include <gtkmm/label.h>
#include <gtkmm/liststore.h>
#include <gtkmm/picture.h>

#include <XGP/XDialog.h>

#include <card/Pile.h>

namespace YGP {
class DirectorySearch;
}

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
    DeckSelectDlg(const std::string& deck, const std::string& back);
    virtual ~DeckSelectDlg();

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
    virtual void okEvent();
    virtual void command(int action);

    Glib::RefPtr<Gdk::Pixbuf> getImage(const std::string& file, bool scale = true);
    void addFileInDirectories(const std::string& dir, const std::string& file, const std::string& defaultDeck,
                              unsigned int offName = 0);
    void addFile(const std::string& path, const std::string& name, const std::string& display, const std::string& defaultDeck);

  private:
    // Prohibited manager-functions
    DeckSelectDlg();
    DeckSelectDlg(const DeckSelectDlg&);

    void deckSelected();
    void backSelected();

    void deckActivated(const Gtk::TreeModel::Path& deck);
    void backActivated(const Gtk::TreeModel::Path& back);

    const DeckSelectDlg& operator=(const DeckSelectDlg&);

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
};

} // namespace Card

#endif
