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

#include <gdkmm/pixmap.h>


// Class to load & store the images of the used cards
class CardImages {
 public:
   CardImages (unsigned int cards) : cards_ (cards) { }
   ~CardImages ();

   const Glib::RefPtr<Gdk::Pixmap> getCardImage (unsigned int nr) const;

   const Glib::RefPtr<Gdk::Pixmap> getCardBackground () const { return back_; }
   void setCardBackground (const Glib::RefPtr<Gdk::Pixmap> back) { back_ = back; }

   void loadDecks (const Glib::RefPtr<Gdk::Window> parent, const std::string& path,
                  bool thread = true) throw (std::string);
   void loadBack (const Glib::RefPtr<Gdk::Window> parent, const std::string& file,
                  bool thread = true) throw (std::string);
   void load (const Glib::RefPtr<Gdk::Window> parent, const std::string& path,
              const std::string& back, bool thread = true) throw (std::string) {
      loadDecks (parent, path, true);
      loadBack (parent, back);
   }
   void load (unsigned int cards, const Glib::RefPtr<Gdk::Window> parent,
              const std::string& path, const std::string& back,
              bool thread = true) throw (std::string) {
      cards_.reserve (cards);
      load (parent, path, back, thread); }

   unsigned int numberOfCards () const { return cards_.size (); }

 private:
   std::vector<Glib::RefPtr<Gdk::Pixmap> > cards_;
   Glib::RefPtr<Gdk::Pixmap> back_;

   void unload ();
};

#endif
