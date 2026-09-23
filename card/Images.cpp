// PROJECT     : Cardgames
// SUBSYSTEM   : Common
// REFERENCES  :
// TODO        :
// BUGS        :
// AUTHOR      : Markus Schwab
// CREATED     : 29.03.2002
// COPYRIGHT   : Copyright (C) 2002 - 2018, 2024, 2026

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

#include <cerrno>
#include <cstdlib>

#include <iomanip>
#include <sstream>

#include <cardgames-cfg.h>

#ifdef HAVE_RSVG
#    include <librsvg/rsvg.h>
#endif

#include <YGP/Check.h>
#include <YGP/Exception.h>
#include <YGP/File.h>
#include <YGP/Trace.h>

#include <glibmm/error.h>
#include <gtkmm/widget.h>

#include "Images.h"

namespace Card {

unsigned int Images::HEIGHT(88);
unsigned int Images::WIDTH(66);

/**Helper-class to actually load cardimages
 */
class ImageLoader {
  public:
    ImageLoader() { TRACE1("ImageLoader::ImageLoader()"); }
    virtual ~ImageLoader();

    virtual std::string convert2File(unsigned int nrImage);
    virtual void loadFronts(std::vector<Glib::RefPtr<Gdk::Pixbuf>>& cards, const std::string& path);
    virtual void loadBack(Glib::RefPtr<Gdk::Pixbuf>& back, const std::string& path);

  protected:
    Glib::RefPtr<Gdk::Pixbuf> loadImage(const std::string& file);
};

//-----------------------------------------------------------------------------
/// Destructor
//-----------------------------------------------------------------------------
ImageLoader::~ImageLoader() {}

//-----------------------------------------------------------------------------
/// Loads the cards(faces)
/// \param cards Vector of pixbufs to load the cards into
/// \param path Path to files
//-----------------------------------------------------------------------------
void ImageLoader::loadFronts(std::vector<Glib::RefPtr<Gdk::Pixbuf>>& cards, const std::string& path) {
    TRACE1("ImageLoader::loadFronts(std::vector<Glib::RefPtr<Gdk::Pixbuf>>&, const std::string&) -\n\tPath: " << path);

    std::string file(path);
    if (file[file.size() - 1] != YGP::File::DIRSEPARATOR)
        file += YGP::File::DIRSEPARATOR;

    std::string actFile;
    for (unsigned int i(0); i < 52; ++i) {
        actFile = file + convert2File(i);
        TRACE8("Images::loadFronts(std::vector<Glib::RefPtr<Gdk::Pixbuf>>&, const std::string&) -\n\tFile: " << actFile);

        cards[i] = loadImage(actFile);
        if ((cards[i]->get_height() != (int)Images::HEIGHT) || (cards[i]->get_width() != (int)Images::WIDTH))
            cards[i] = cards[i]->scale_simple(Images::WIDTH, Images::HEIGHT, Gdk::InterpType::BILINEAR);
        Check3(cards[i]);
    } // end-for
}

//-----------------------------------------------------------------------------
/// Loads the card(back)
/// \param back Pixbuf to load the card into
/// \param file File to load from
//-----------------------------------------------------------------------------
void ImageLoader::loadBack(Glib::RefPtr<Gdk::Pixbuf>& back, const std::string& file) {
    back = loadImage(file);
    if ((back->get_height() != (int)Images::HEIGHT) || (back->get_width() != (int)Images::WIDTH))
        back = back->scale_simple(Images::WIDTH, Images::HEIGHT, Gdk::InterpType::BILINEAR);
}

//-----------------------------------------------------------------------------
/// Loads an image from the passed file
/// \param file File to load
/// \returns Glib::RefPtr<Gdk::Pixbuf> Created image
//-----------------------------------------------------------------------------
Glib::RefPtr<Gdk::Pixbuf> ImageLoader::loadImage(const std::string& file) {
    Glib::RefPtr<Gdk::Pixbuf> img;
    try {
        img = Gdk::Pixbuf::create_from_file(file);
    }
    catch (Glib::Error& e) {
        throw YGP::FileError(e.what());
    }
    catch (...) {
        throw YGP::FileError(_("Unknown error"));
    }
    return img;
}

//-----------------------------------------------------------------------------
/// Converts an image-number to a file-name. Default style: nrImage -> "(nrImage
/// +1).png"
/// \param nrImage Number of image to convert(in the range 0 - 51)
/// \returns std::string Filename(KDE-style)
//-----------------------------------------------------------------------------
std::string ImageLoader::convert2File(unsigned int nrImage) {
    Check3(nrImage < 52);
    std::ostringstream out;
    out << (nrImage + 1) << ".png";
    return out.str();
}

#ifdef CARDPICS_DIR
/**Helper-class to load cardpcis cardimages
 */
class CardpicsLoader : public ImageLoader {
  public:
    CardpicsLoader() { TRACE1("CardpicsLoader::CardpicsLoader()"); }
    virtual ~CardpicsLoader();

    virtual std::string convert2File(unsigned int nrImage);
};

//-----------------------------------------------------------------------------
/// Destructor
//-----------------------------------------------------------------------------
CardpicsLoader::~CardpicsLoader() {}

//-----------------------------------------------------------------------------
/// Converts an image-number to a file-name(Cardpics-style)
/// \param nrImage Number of image to convert(in the range 0 - 51)
/// \returns std::string Filename(Cardpics-style)
//-----------------------------------------------------------------------------
std::string CardpicsLoader::convert2File(unsigned int nrImage) {
    Check3(nrImage < 52);

    // Special handling of aces
    if (nrImage < 4)
        nrImage *= 14;
    else if (nrImage < 12)
        nrImage = ((59 - nrImage) >> 2) + ((nrImage & 3) * 14);
    else
        nrImage = ((55 - nrImage) >> 2) + ((nrImage & 3) * 14);

    std::ostringstream out;
    out << std::setw(2) << std::setfill('0') << nrImage << ".png";
    return out.str();
}
#endif

#ifdef GNOMECARDS_DIR
/**Helper-class to load the GNOME cardimages
 */
class GnomeLoader : public ImageLoader {
  public:
    GnomeLoader() { TRACE1("GnomeLoader::GnomeLoader()"); }
    virtual ~GnomeLoader();

    virtual void loadFronts(std::vector<Glib::RefPtr<Gdk::Pixbuf>>& cards, const std::string& path);
    virtual void loadBack(Glib::RefPtr<Gdk::Pixbuf>& back, const std::string& path);
};

//-----------------------------------------------------------------------------
/// Destructor
//-----------------------------------------------------------------------------
GnomeLoader::~GnomeLoader() {}

//-----------------------------------------------------------------------------
/// Loads the cards(faces)
/// \param cards Vector of pixbufs to load the cards into
/// \param path Path to files
//-----------------------------------------------------------------------------
void GnomeLoader::loadFronts(std::vector<Glib::RefPtr<Gdk::Pixbuf>>& cards, const std::string& path) {
    TRACE1("GnomeLoader::loadFronts(std::vector<Glib::RefPtr<Gdk::Pixbuf>>&, "
           "const std::string&) -\n\t"
           << path);
#    ifdef HAVE_RSVG
    // New style of reading Gnome cards: Get sub-images by identification
    // Does not work with librsvg <= 2.26.0
    GError* error(NULL);
    rsvg_init();
    RsvgHandle* hSVG(rsvg_handle_new_from_file(path.c_str(), &error));
    if (!hSVG)
        throw YGP::FileError(error->message);

    if (!rsvg_handle_close(hSVG, &error) || error)
        throw YGP::FileError(error->message);

    std::string actCard;
    Glib::RefPtr<Gdk::Pixbuf> actImg;
    const char* colours[] = {"club", "spade", "heart", "diamond"};
    const char* numbers[] = {"10", "jack", "queen", "king"};

    for (unsigned int c(0); c < (sizeof(colours) / sizeof(*colours)); ++c)
        for (unsigned int n(0); n < 13; ++n) {
            actCard = "#";
            if (n > 8)
                actCard += numbers[n - 9];
            else
                actCard += char('1' + n);
            actCard += "_";
            actCard += colours[c];
            TRACE9("GnomeLoader::loadFronts(std::vector<Glib::RefPtr<Gdk::Pixbuf>>&, "
                   "const std::string&) -\n\tCard: "
                   << actCard);

            actImg = Glib::wrap(rsvg_handle_get_pixbuf_sub(hSVG, actCard.c_str()));
            if (actImg)
                cards[c * 13 + n] = actImg->scale_simple(Images::WIDTH, Images::HEIGHT, Gdk::InterpType::BILINEAR);
            else {
                std::string msg(_("Card `%1' not found"));
                msg.replace(msg.find("%1"), 2, actCard);
                throw YGP::FileError(msg);
            }
        }
#    else
    // Old style of reading Gnome cards: Extract cards from certain positions
    Glib::RefPtr<Gdk::Pixbuf> img(loadImage(path));
    unsigned int widthImg(img->get_width() / 13);
    unsigned int heightImg(img->get_height() / 5);

    for (unsigned int i(0); i < 52; ++i) {
        unsigned int x(((i < 4) ? i : (55 - i)) >> 2);
        unsigned int y(i & 3);
        if (y)
            y = 4 - y;
        TRACE8("GnomeLoader::loadFronts(std::vector<Glib::RefPtr<Gdk::Pixbuf>>&, "
               "const std::string&) -\n\tPosition "
               << x << '/' << y);

        cards[i] = Gdk::Pixbuf::create_subpixbuf(img, widthImg * x, heightImg * y, widthImg, heightImg);
        if ((cards[i]->get_height() != (int)Images::HEIGHT) || (cards[i]->get_width() != (int)Images::WIDTH))
            cards[i] = cards[i]->scale_simple(Images::WIDTH, Images::HEIGHT, Gdk::InterpType::BILINEAR);
        Check3(cards[i]);
    } // end-for
#    endif
}

//-----------------------------------------------------------------------------
/// Loads the card(back)
/// \param back Pixbuf to load the card into
/// \param file File to load from
//-----------------------------------------------------------------------------
void GnomeLoader::loadBack(Glib::RefPtr<Gdk::Pixbuf>& back, const std::string& file) {
    Glib::RefPtr<Gdk::Pixbuf> img(loadImage(file));
    unsigned int widthImg(img->get_width() / 13);
    unsigned int heightImg(img->get_height() / 5);

    back = Gdk::Pixbuf::create_subpixbuf(img, widthImg * 2, heightImg << 2, widthImg, heightImg);
    back = back->scale_simple(Images::WIDTH, Images::HEIGHT, Gdk::InterpType::BILINEAR);
}
#endif

//-----------------------------------------------------------------------------
/// Destructor
//-----------------------------------------------------------------------------
Images::~Images() { TRACE9("Images::~Images()"); }

//-----------------------------------------------------------------------------
/// Retrieves the specified cardnumber
/// \param nr Number of card to retrieve
//-----------------------------------------------------------------------------
const Glib::RefPtr<Gdk::Pixbuf> Images::getCardImage(unsigned int nr) const {
    TRACE9("Images::getCardImage(unsigned int) - Request for card " << nr);
    Check1(nr < size());
    Check3(cards_[nr]);
    return cards_[nr]->copy();
}

//-----------------------------------------------------------------------------
/// Loads the cards(faces)
/// \param path Path to files
//-----------------------------------------------------------------------------
void Images::loadDecks(const std::string& path) {
    TRACE1("Images::loadDecks(const std::string&) - " << path);

    ImageLoader* ldr(NULL);
#ifdef GNOMECARDS_DIR
    if (!path.compare(0, strlen(GNOMECARDS_DIR), GNOMECARDS_DIR))
        ldr = new GnomeLoader;
    else
#endif
#ifdef CARDPICS_DIR
        if (path == CARDPICS_DIR)
        ldr = new CardpicsLoader;
    else
#endif
        ldr = new ImageLoader;

    ldr->loadFronts(cards_, path);
}

//-----------------------------------------------------------------------------
/// Loads the background card
/// \param back File containing background picture
//-----------------------------------------------------------------------------
void Images::loadBack(const std::string& back) {
    ImageLoader* ldr(NULL);
#ifdef GNOMECARDS_DIR
    if (!back.compare(0, strlen(GNOMECARDS_DIR), GNOMECARDS_DIR))
        ldr = new GnomeLoader;
    else
#endif
        ldr = new ImageLoader;

    ldr->loadBack(back_, back);
}

//-----------------------------------------------------------------------------
/// Resizes all previous loaded cards
//-----------------------------------------------------------------------------
void Images::resizeAll() {
    back_ = back_->scale_simple(WIDTH, HEIGHT, Gdk::InterpType::BILINEAR);
    for (std::vector<Glib::RefPtr<Gdk::Pixbuf>>::iterator i(cards_.begin()); i != cards_.end(); ++i)
        *i = (*i)->scale_simple(WIDTH, HEIGHT, Gdk::InterpType::BILINEAR);
}

} // namespace Card
