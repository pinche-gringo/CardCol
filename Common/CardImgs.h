#ifndef CARDIMGS_H
#define CARDIMGS_H

//$Id$

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

#include <string>
#include <vector>

#include <gdkmm/pixbuf.h>


/**Class to load & store the images of the used cards
*/
class CardImages {
 public:
   CardImages () : cards_ (52) { }
   ~CardImages ();

   unsigned int addImage (const char* const* data) {
      cards_.push_back (Gdk::Pixbuf::create_from_xpm_data (data));
      return cards_.size () - 1; }
   void delImage (unsigned int pos) {
      cards_.erase (cards_.begin () + pos); }

   const Glib::RefPtr<Gdk::Pixbuf> getCardImage (unsigned int nr) const;

   const Glib::RefPtr<Gdk::Pixbuf> getCardBackground () const { return back_; }
   void setCardBackground (const Glib::RefPtr<Gdk::Pixbuf> back) { back_ = back; }

   void loadDecks (const std::string& path) throw (Glib::ustring);
   void loadBack (const std::string& file) throw (Glib::ustring);


   /// Loads the cards (faces and background)
   /// \param path: Path to files
   /// \param back: File containing background picture
   /// \throw Glib::ustring: An describing text in case of error
   void load (const std::string& path, const std::string& back) throw (Glib::ustring) {
      loadDecks (path);
      loadBack (back);
   }
   /// Loads the cards (faces and background)
   /// \param cards: Number of cards expected
   /// \param path: Path to files
   /// \param back: File containing background picture
   /// \throw Glib::ustring: An describing text in case of error
   void load (unsigned int cards, const std::string& path, const std::string& back) throw (Glib::ustring) {
      cards_.reserve (cards);
      load (path, back); }

   unsigned int size () const { return cards_.size (); }
   bool hasBack () const { return back_; }

 private:
   std::vector<Glib::RefPtr<Gdk::Pixbuf> > cards_;
   Glib::RefPtr<Gdk::Pixbuf> back_;

   void unload ();
};

#endif
