//$Id$

//PROJECT     : Cardgames
//SUBSYSTEM   : Common
//REFERENCES  :
//TODO        :
//BUGS        :
//REVISION    : $Revision$
//AUTHOR      : Markus Schwab
//CREATED     : 29.03.2002
//COPYRIGHT   : Anticopyright (A) 2002

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
const Glib::RefPtr<Gdk::Pixmap> CardImages::getCardImage (unsigned int nr) const {
   TRACE8 ("CardImages::getCardImage (unsigned int) - Request for card " << nr);
   Check1 (nr < numberOfCards ());
   Check3 (cards_[nr]);
   return cards_[nr];
}

/*--------------------------------------------------------------------------*/
//Purpose   : Loads the cards (faces)
//Parameters: parent: Parent window
//            path: Path to files
//            back: File containing background picture
//            thread: Flag if loading in thread
/*--------------------------------------------------------------------------*/
void CardImages::loadDecks (const Glib::RefPtr<Gdk::Window> parent,
                            const std::string& path,
                            bool thread) throw (std::string) {
   TRACE1 ("CardImages::loadDecks (const Gdk::Window&, const char*) - " << path
           << "; Threaded: " << (thread ? "Yes" : "No"));

   std::string file (path);
   if (file[file.size () - 1] != File::DIRSEPARATOR)
      file += File::DIRSEPARATOR;

   for (int i = 1; i <= numberOfCards (); ++i) {
      std::string temp;
      std::ostringstream out (temp);
      out << file << i << ".xpm";
      TRACE9 ("CardImages::loadDecks (const Gdk::Window&, const char*) - File "
              << out.str () << "; Temp:" << temp);

      if (thread)
         gdk_threads_enter ();

      Gdk::Color color;
      cards_[i - 1] = Gdk::Pixmap::create_from_xpm (parent, color, out.str ());
      Check3 (cards_[i - 1]);
      if (thread)
         gdk_threads_leave ();

      if (errno) {
         std::string error (_("Can't create picture from file `%1'!\nReason: %2"));
         error.replace (error.find ("%1"), 2, temp);
         error.replace (error.find ("%2"), 2, strerror (errno));
         throw (error);
      }
   }
}

/*--------------------------------------------------------------------------*/
//Purpose   : Loads the background card
//Parameters: parent: Parent window
//            back: File containing background picture
//            thread: Flag if loading in thread
/*--------------------------------------------------------------------------*/
void CardImages::loadBack (const Glib::RefPtr<Gdk::Window> parent,
                           const std::string& back,
                           bool thread) throw (std::string) {
   TRACE1 ("CardImages::loadBack (const Gdk::Window&, const char*) - " << back);

   Gdk::Color color;

   if (thread)
      gdk_threads_enter ();
   back_ = Gdk::Pixmap::create_from_xpm (parent, color, back);
   if (thread)
      gdk_threads_leave ();

   if (errno) {
      std::string error (_("Can't create picture from file `%1'!\nReason: %2"));
      error.replace (error.find ("%1"), 2, back);
      error.replace (error.find ("%2"), 2, strerror (errno));
      throw (error);
   }
}
