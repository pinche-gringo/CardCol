//$Id$

//PROJECT     : Cardgames
//SUBSYSTEM   : Common
//REFERENCES  :
//TODO        :
//BUGS        :
//REVISION    : $Revision$
//AUTHOR      : Markus Schwab
//CREATED     : 29.03.2002
//COPYRIGHT   : Anticopyright (A) 2002, 2003

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

#include <errno.h>
#include <stdlib.h>

#include <sstream>

#include <cardgames-cfg.h>

#include <Check.h>
#include <Trace_.h>

#include <gtkmm/widget.h>

#include <File.h>

#include "CardImgs.h"


/*--------------------------------------------------------------------------*/
//Purpose   : Destructor
/*--------------------------------------------------------------------------*/
CardImages::~CardImages () {
   TRACE9 ("CardImages::~CardImages ()");
}


/*--------------------------------------------------------------------------*/
//Purpose   : Retrieves the specified cardnumber
//Parameters: nr: Number of card to retrieve
/*--------------------------------------------------------------------------*/
const Glib::RefPtr<Gdk::Pixbuf> CardImages::getCardImage (unsigned int nr) const {
   TRACE8 ("CardImages::getCardImage (unsigned int) - Request for card " << nr);
   Check1 (nr < size ());
   Check3 (cards_[nr]);
   return cards_[nr];
}

/*--------------------------------------------------------------------------*/
//Purpose   : Loads the cards (faces)
//Parameters: path: Path to files
//            thread: Flag if loading in thread
/*--------------------------------------------------------------------------*/
void CardImages::loadDecks (const std::string& path, bool thread) throw (std::string) {
   TRACE1 ("CardImages::loadDecks (const Gdk::Window&, const char*) - " << path
           << "; Threaded: " << (thread ? "Yes" : "No"));

   std::string file (path);
   if (file[file.size () - 1] != File::DIRSEPARATOR)
      file += File::DIRSEPARATOR;

   std::string err;

   for (int i = 1; i <= size (); ++i) {
      std::ostringstream out;
      out << file << i << ".png";
      TRACE9 ("CardImages::loadDecks (const Gdk::Window&, const char*) - File "
              << out.str ());

      if (thread)
         gdk_threads_enter ();

      try {
         cards_[i - 1] = Gdk::Pixbuf::create_from_file (out.str ());
         Check3 (cards_[i - 1]);
      }
      catch (Gdk::PixbufError& e) {
         err = e.what ();
      }
      catch (Glib::FileError& e) {
         err = e.what ();
      }
      catch (...) {
         err = _("Unknown error");
      }
      if (thread)
         gdk_threads_leave ();

      if (err.size ())
         throw (err);
   }
}

/*--------------------------------------------------------------------------*/
//Purpose   : Loads the background card
//Parameters: back: File containing background picture
//            thread: Flag if loading in thread
/*--------------------------------------------------------------------------*/
void CardImages::loadBack (const std::string& back,
                           bool thread) throw (std::string) {
   TRACE1 ("CardImages::loadBack (const Gdk::Window&, const char*) - " << back);
   std::string err;

   if (thread)
      gdk_threads_enter ();
   try {
      back_ = Gdk::Pixbuf::create_from_file (back);
   }
   catch (Gdk::PixbufError& e) {
      err = e.what ();
   }
   catch (Glib::FileError& e) {
      err = e.what ();
   }
   catch (...) {
      err = _("Unknown error");
   }
   if (thread)
      gdk_threads_leave ();

   if (err.size ())
      throw (err);
}
