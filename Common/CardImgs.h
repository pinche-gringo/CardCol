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
#include <vector.h>

#include <gdk--/pixmap.h>


// Class to load & store the images of the used cards
class CardImages {
 public:
   CardImages (unsigned int cards) : cards_ (cards) { }
   ~CardImages ();

   const Gdk_Pixmap& getCardImage (unsigned int nr) const;

   const Gdk_Pixmap& getCardBackground () const { return back_; }
   void setCardBackground (const Gdk_Pixmap& back) { back_ = back; }

   void load (const Gdk_Window& parent, const char* path = NULL) throw (std::string);
   void load (unsigned int cards, const Gdk_Window& parent,
              const char* path = NULL) throw (std::string) {
      cards_.reserve (cards);
      load (parent, path); }

   unsigned int numberOfCards () const { return cards_.size (); }

 private:
   vector<Gdk_Pixmap> cards_;
   Gdk_Pixmap back_;

   void unload ();

   static std::string makeDirString (const char* path);
};

#endif
