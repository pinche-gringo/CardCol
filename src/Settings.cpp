//$Id$

//PROJECT     : Cardgames
//SUBSYSTEM   : Settings
//REFERENCES  :
//TODO        :
//BUGS        :
//REVISION    : $Revision$
//AUTHOR      : Markus Schwab
//CREATED     : 28.4.2005
//COPYRIGHT   : Copyright (C) 2005

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


#include <cardgames-cfg.h>

#include <gtkmm/box.h>
#include <gtkmm/label.h>
#include <gtkmm/table.h>
#include <gtkmm/notebook.h>

#include <ComputerPlayer.h>

#include "Buraco.h"
#include "Options.h"

#include "Settings.h"


XGP::XAttributeSpinEntry<unsigned int> Settings::* Settings::intFields[] =
   { &Settings::timeout, &Settings::maxBuracoPoints };

Settings* Settings::instance (NULL);


//-----------------------------------------------------------------------------
/// Constructor
/// \param options: Options to change
//-----------------------------------------------------------------------------
Settings::Settings (Options& options)
   : XGP::XDialog (OKCANCEL),
     adjPoints (0, 0, 100000.0, 1, 100),
     adjTimeout (0, 0, 10000.0, 1, 100),
     gameType (types),
     timeout (ComputerPlayer::TIMEOUT, adjTimeout),
     maxBuracoPoints (Buraco::ENDPOINTS, adjPoints),
     startGame (options.type) {
   Check3 (instance == NULL);
   instance =  this;

   set_title (_("Preferences"));

   Gtk::Notebook& nb (*manage (new Gtk::Notebook));
   Gtk::Table& pagGeneral (*manage (new Gtk::Table (2, 2)));

   Gtk::Label* lbl (manage (new Gtk::Label (_("_Delay of computer player (ms):"), Gtk::ALIGN_LEFT, Gtk::ALIGN_CENTER, true)));
   lbl->set_mnemonic_widget (timeout);
   pagGeneral.attach (*lbl,    0, 1, 0, 1, Gtk::FILL, Gtk::FILL, 5);
   pagGeneral.attach (timeout, 1, 2, 0, 1, Gtk::FILL | Gtk::EXPAND, Gtk::FILL, 5);

   lbl = manage (new Gtk::Label (_("_Default game:"), Gtk::ALIGN_LEFT, Gtk::ALIGN_CENTER, true));
   lbl->set_mnemonic_widget (gameType);
   pagGeneral.attach (*lbl,     0, 1, 1, 2, Gtk::FILL, Gtk::FILL, 5);
   pagGeneral.attach (gameType, 1, 2, 1, 2, Gtk::FILL | Gtk::EXPAND, Gtk::FILL, 5);

   gameType.set_active_text (types[options.type]);

   Gtk::Box& pagBuraco (*manage (new Gtk::HBox));
   lbl = manage (new Gtk::Label (_("_Points to end game:"), Gtk::ALIGN_LEFT, Gtk::ALIGN_CENTER, true));
   lbl->set_mnemonic_widget (maxBuracoPoints);
   pagBuraco.pack_start (*lbl, Gtk::PACK_SHRINK, 5);
   pagBuraco.pack_start (maxBuracoPoints, Gtk::PACK_EXPAND_WIDGET, 5);

   nb.append_page (pagGeneral, _("_General"), true);
   nb.append_page (pagBuraco, _("_Buraco"), true);

   get_vbox ()->pack_start (nb, true, true, 5);
   show_all_children ();
   show ();
}

//-----------------------------------------------------------------------------
/// Destructor
//-----------------------------------------------------------------------------
Settings::~Settings () {
   instance = NULL;
}

//-----------------------------------------------------------------------------
/// Handling of the OK button; closes the dialog with commiting data
//-----------------------------------------------------------------------------
void Settings::okEvent () {
   ok->grab_focus ();
   for (unsigned int i (0); i < (sizeof (intFields) / sizeof (*intFields)); ++i)
      (this->*intFields[i]).commit ();

    startGame = types[gameType.get_active_text ()];
}

//-----------------------------------------------------------------------------
/// Creates or selects (if already existing) a dialog to change the
/// preferences.
/// \param parent: Parent window
/// \returns Settings*: Pointer to the created window
//-----------------------------------------------------------------------------
Settings* Settings::create (const Glib::RefPtr<Gdk::Window>& parent,
			    Options& options) {
   if (instance == NULL) {
      new Settings (options); Check3 (instance);
      instance->get_window ()->set_transient_for (parent);
      instance->signal_response ().connect (mem_fun (*instance, &Settings::free));
   }
   else
      instance->present ();
   return instance;
}
