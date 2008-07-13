#ifndef CARDIMGS_H
#define CARDIMGS_H

//$Id$

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


/**Class to load & store the images of the used cards
*/
class CardImages {
 public:
   CardImages () : cards_ (52) { }
   ~CardImages ();

   unsigned int addImage (const char* const* data) {
      Glib::RefPtr<Gdk::Pixbuf> card (Gdk::Pixbuf::create_from_xpm_data (data));
      card = card->scale_simple (WIDTH, HEIGHT, Gdk::INTERP_BILINEAR);
      cards_.push_back (card);
      return cards_.size () - 1; }
   void delImage (unsigned int pos) {
      cards_.erase (cards_.begin () + pos); }

   const Glib::RefPtr<Gdk::Pixbuf> getCardImage (unsigned int nr) const;

   const Glib::RefPtr<Gdk::Pixbuf> getCardBackground () const { return back_; }
   void setCardBackground (const Glib::RefPtr<Gdk::Pixbuf> back) { back_ = back; }

   void loadDecks (const std::string& path) throw (YGP::FileError);
   void loadBack (const std::string& file) throw (YGP::FileError);

   /// Loads the cards (faces and background)
   /// \param path Path to files
   /// \param back File containing background picture
   /// \throw YGP::FileError An describing text in case of error
   void load (const std::string& path, const std::string& back) throw (YGP::FileError) {
      loadDecks (path);
      loadBack (back);
   }
   /// Loads the cards (faces and background)
   /// \param cards Number of cards expected
   /// \param path Path to files
   /// \param back File containing background picture
   /// \throw YGP::FileError An describing text in case of error
   void load (unsigned int cards, const std::string& path, const std::string& back) throw (YGP::FileError) {
      cards_.reserve (cards);
      load (path, back); }

   unsigned int size () const { return cards_.size (); }
   bool hasBack () const { return back_; }

   void resizeAll ();

   static unsigned int HEIGHT;
   static unsigned int WIDTH;

 private:
   std::vector<Glib::RefPtr<Gdk::Pixbuf> > cards_;
   Glib::RefPtr<Gdk::Pixbuf> back_;

   void unload ();
};

#endif
