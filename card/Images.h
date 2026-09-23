#ifndef CARDIMGS_H
#define CARDIMGS_H

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
#include <vector>

#include <gdkmm/pixbuf.h>

#include <YGP/Exception.h>

namespace Card {

/**Class to load & store the images of the used cards
 */
class Images {
  public:
    Images() : cards_(52), back_() {}
    ~Images();

    unsigned int addImage(const char* const* data) {
        Glib::RefPtr<Gdk::Pixbuf> card(Gdk::Pixbuf::create_from_xpm_data(data));
        card = card->scale_simple(WIDTH, HEIGHT, Gdk::InterpType::BILINEAR);
        cards_.push_back(card);
        return cards_.size() - 1;
    }
    void delImage(unsigned int pos) { cards_.erase(cards_.begin() + pos); }

    const Glib::RefPtr<Gdk::Pixbuf> getCardImage(unsigned int nr) const;

    const Glib::RefPtr<Gdk::Pixbuf> getCardBackground() const { return back_->copy(); }
    void setCardBackground(const Glib::RefPtr<Gdk::Pixbuf> back) { back_ = back; }

    void loadDecks(const std::string& path);
    void loadBack(const std::string& file);

    /// Loads the cards(faces and background)
    /// \param path Path to files
    /// \param back File containing background picture
    void load(const std::string& path, const std::string& back) {
        loadDecks(path);
        loadBack(back);
    }
    /// Loads the cards(faces and background)
    /// \param cards Number of cards expected
    /// \param path Path to files
    /// \param back File containing background picture
    void load(unsigned int cards, const std::string& path, const std::string& back) {
        cards_.reserve(cards);
        load(path, back);
    }

    unsigned int size() const { return cards_.size(); }
    bool hasBack() const { return bool(back_); }

    void resizeAll();

    static unsigned int HEIGHT;
    static unsigned int WIDTH;

  private:
    std::vector<Glib::RefPtr<Gdk::Pixbuf>> cards_;
    Glib::RefPtr<Gdk::Pixbuf> back_;

    void unload();
};

} // namespace Card

#endif
