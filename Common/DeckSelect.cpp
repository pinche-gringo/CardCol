//$Id$

//PROJECT     : Cardgames
//SUBSYSTEM   : Common/DeckSelect
//REFERENCES  :
//TODO        :
//BUGS        :
//REVISION    : $Revision$
//AUTHOR      : Markus Schwab
//CREATED     : 29.8.2002
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


#include <gtk--/box.h>
#include <gtk--/label.h>
#include <gtk--/button.h>
#include <gtk--/packer.h>

#define CHECK 9
#define TRACELEVEL 9
#include "Check.h"
#include "Trace_.h"

#include <cardgames-cfg.h>

#include "DeckSelect.h"


/*--------------------------------------------------------------------------*/
//Purpose   : Constructor; adds all controls to the dialog
//Parameters: path: Path to carddecks
/*--------------------------------------------------------------------------*/
ICarddeckSelectDlg::ICarddeckSelectDlg (const char* path)
   : Dialog (), txtDecks (new Label (_("Available decks")))
     , ok (new Button (_("OK"))), apply (new Button (_("Apply")))
     , cancel (new Button (_("Cancel"))), decks (new Packer ()) {
   TRACE3 ("CarddeckSelectDlg::CarddeckSelectDlg (const char*)");
   Check3 (txtDecks); Check3 (ok); Check3 (apply); Check3 (cancel); Check3 (decks);

   if (!path)
      path = PKGDIR;

   Check3 (get_vbox ());
   get_vbox ()->pack_start (*txtDecks, true, false, 5);
   get_vbox ()->pack_start (*decks, true, false, 5);

   get_action_area ()->pack_start (*ok);
   get_action_area ()->pack_start (*apply);
   get_action_area ()->pack_start (*cancel);

   ok->clicked.connect (bind (slot (this, &ICarddeckSelectDlg::command), OK));
   apply->clicked.connect (bind (slot (this, &ICarddeckSelectDlg::command), APPLY));
   cancel->clicked.connect (bind (slot (this, &ICarddeckSelectDlg::command), CANCEL));

   show_all ();
}

/*--------------------------------------------------------------------------*/
//Purpose   : Destructor
/*--------------------------------------------------------------------------*/
ICarddeckSelectDlg::~ICarddeckSelectDlg () {
   TRACE9 ("CarddeckSelectDlg::~CarddeckSelectDlg ()");
}
