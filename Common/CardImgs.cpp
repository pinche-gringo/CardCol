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

#include <cardgames-cfg.h>

#include <Check.h>
#include <Trace_.h>

#include <gtk--/widget.h>

#include <File.h>
#include <ANumeric.h>

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
const Gdk_Pixmap& CardImages::getCardImage (unsigned int nr) const {
   Check3 (nr < numberOfCards ());

   return cards_[nr];
}

/*--------------------------------------------------------------------------*/
//Purpose   : Constructor; adds all controls to the dialog
//Parameters: parent: Parent window
//            path: Path to files; NULL for defaultpath (in datadir)
/*--------------------------------------------------------------------------*/
void CardImages::load (const Gdk_Window& parent, const char* path) throw (std::string) {
   TRACE1 ("CardImages::load (const Gdk_Window&, const char*) - " << path);

   std::string file (makeDirString (path));

   std::string temp;
   ANumeric nr;
   Gdk_Color color;

   for (int i = 0; i < numberOfCards (); ++i) {
      nr = i + 1;
      temp = file + nr.toUnformatedString () + ".xpm";
      TRACE3 ("CardImages::load (const Gdk_Window&, const char*) - File " << temp);

      gdk_threads_enter ();
      cards_[i].create_from_xpm (parent, color, temp);
      gdk_threads_leave ();

      if (errno)
         break;
   }

   if (!errno) {
      temp = file + "back.xpm";
      TRACE3 ("CardImages::load (const Gdk_Window&, const char*) - File " << temp);

      gdk_threads_enter ();
      back_.create_from_xpm (parent, color, temp);
      gdk_threads_leave ();
   }
   if (errno) {
      std::string error (_("Can't create picture from file `%1'!\nReason: %2"));
      error.replace (error.find ("%1"), 2, temp);
      error.replace (error.find ("%2"), 2, strerror (errno));
      throw (error);
   }
}

/*--------------------------------------------------------------------------*/
//Purpose   : Creates a string for the directory containing the pixmaps
//Paramaters: path: Suggestion for the path (may be NULL)
//Returns   : Path to icons (including trailing backlslash)
/*--------------------------------------------------------------------------*/
std::string CardImages::makeDirString (const char* path) {
   std::string dir (path ? path : PKGDIR);
   if (dir.empty ())
      dir = ".";

   if (dir[dir.size () - 1] != File::DIRSEPARATOR)
      dir += File::DIRSEPARATOR;

   return dir;
}
