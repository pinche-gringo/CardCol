//$Id$

//PROJECT     : Cardgames
//SUBSYSTEM   : Common
//REFERENCES  :
//TODO        :
//BUGS        :
//REVISION    : $Revision$
//AUTHOR      : Markus Schwab
//CREATED     : 27.03.2002
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


#include <string>

// Includes for Gtk--/Gdk--
#include <gdk--/bitmap.h>
#include <gdk--/pixmap.h>

#define DEBUG 0
#include <Check.h>
#include <Trace_.h>

#include "CardWidget.h"


/*--------------------------------------------------------------------------*/
//Purpose   : Constructor; adds all controls to the dialog
//Parameters: filename: Name of bitmap-file (actually png-file)
/*--------------------------------------------------------------------------*/
CardWidget::CardWidget (const std::string& filename) {
   TRACE3 ("CardWidget::CardWidget (const std::string&) - " << filename);

   Gdk_Color color (&Widget::gtkobj ()->style->bg[GTK_STATE_NORMAL]);
   Gdk_Pixmap pixmap;
   pixmap.create_from_xpm (pixmap, color, filename);

   Gdk_Bitmap bitmap;
   add_pixmap (pixmap, bitmap);
}

/*--------------------------------------------------------------------------*/
//Purpose   : Destructor
/*--------------------------------------------------------------------------*/
CardWidget::~CardWidget () {
   TRACE9 ("CardWidget::~CardWidget");
   hide ();
}
