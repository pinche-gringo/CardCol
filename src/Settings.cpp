//$Id$

//PROJECT     : Cardgames
//SUBSYSTEM   : Settings
//REFERENCES  :
//TODO        :
//BUGS        :
//REVISION    : $Revision$
//AUTHOR      : Markus Schwab
//CREATED     : 28.4.2005
//COPYRIGHT   : Copyright (C) 2005, 2006

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

#include <YGP/Check.h>
#include <YGP/Trace.h>

#include <ComputerPlayer.h>

#ifdef WITH_BURACO
#  include "Buraco.h"
#  include "BuracoCards.h"
#endif
#ifdef WITH_HEARTS
#  include "Hearts.h"
#endif
#ifdef WITH_ROVHULT
#  include "Rovhult.h"
#  include "CardValue.h"
#endif
#ifdef WITH_SGTMAYOR
#  include "SgtMayor.h"
#endif

#include "Options.h"

#include "Settings.h"


XGP::XAttributeSpinEntry<unsigned int> Settings::* Settings::intFields[] =
   {
#ifdef WITH_BURACO
     &Settings::maxBuracoPoints,
#endif
#ifdef WITH_HEARTS
     &Settings::maxHeartsPoints,
#endif
#ifdef WITH_SGTMAYOR
     &Settings::tricksSgtMayor,
#endif
     &Settings::timeout
   };

Settings* Settings::instance (NULL);


//-----------------------------------------------------------------------------
/// Constructor
/// \param options: Options to change
//-----------------------------------------------------------------------------
Settings::Settings (Options& options)
   : XGP::XDialog (OKCANCEL),
     adjTimeout (0, 100.0, 10000.0, 1, 100),
     gameType (GameTypes::get ()),
     timeout (ComputerPlayer::TIMEOUT, adjTimeout),
#ifdef WITH_BURACO
     adjBPoints (0, 0, 100000.0, 1, 100),
     maxBuracoPoints (Buraco::ENDPOINTS, adjBPoints),
     numBuracoCards (BuracoCards::get ()),
#endif
#ifdef WITH_HEARTS
     adjHPoints (0, 0, 100000.0, 1, 100),
     maxHeartsPoints (Hearts::ENDPOINTS, adjHPoints),
#endif
#ifdef WITH_ROVHULT
     cardNuke (CardValue::get ()),
     cardReverse (CardValue::get ()),
     cardSkip (CardValue::get ()),
#endif
#ifdef WITH_SGTMAYOR
     adjTricks (0, 3, 100000.0, 1, 3),
     tricksSgtMayor (SgtMayor::ENDTRICKS, adjTricks),
#endif
     startGame (options.type) {
   Check3 (instance == NULL);
   instance = this;

   set_title (_("Preferences"));

   Gtk::Notebook& nb (*manage (new Gtk::Notebook));
   Gtk::Table& pagGeneral (*manage (new Gtk::Table (2, 2)));

   Gtk::Label* lbl (manage (new Gtk::Label (_("_Delay of computer player (ms):"), Gtk::ALIGN_LEFT, Gtk::ALIGN_CENTER, true)));
   lbl->set_mnemonic_widget (timeout);
   pagGeneral.attach (*lbl,    0, 1, 0, 1, Gtk::FILL, Gtk::FILL, 5, 3);
   pagGeneral.attach (timeout, 1, 2, 0, 1, Gtk::FILL | Gtk::EXPAND, Gtk::FILL, 5, 3);

   lbl = manage (new Gtk::Label (_("D_efault game:"), Gtk::ALIGN_LEFT, Gtk::ALIGN_CENTER, true));
   lbl->set_mnemonic_widget (gameType);
   pagGeneral.attach (*lbl,     0, 1, 1, 2, Gtk::FILL, Gtk::FILL, 5, 3);
   pagGeneral.attach (gameType, 1, 2, 1, 2, Gtk::FILL | Gtk::EXPAND, Gtk::FILL, 5, 3);

   gameType.set_active_text (GameTypes::get ()[options.type]);
   nb.append_page (pagGeneral, _("_General"), true);

#ifdef WITH_BURACO
   Gtk::Table& pagBuraco (*manage (new Gtk::Table (2, 2)));
   lbl = manage (new Gtk::Label (_("_Points to end game:"), Gtk::ALIGN_LEFT, Gtk::ALIGN_CENTER, true));
   lbl->set_mnemonic_widget (maxBuracoPoints);
   pagBuraco.attach (*lbl,            0, 1, 0, 1, Gtk::FILL, Gtk::FILL, 5, 3);
   pagBuraco.attach (maxBuracoPoints, 1, 2, 0, 1, Gtk::FILL | Gtk::EXPAND, Gtk::FILL, 5, 3);

   lbl = manage (new Gtk::Label (_("_Number of cards:"), Gtk::ALIGN_LEFT, Gtk::ALIGN_CENTER, true));
   lbl->set_mnemonic_widget (cardNuke);
   pagBuraco.attach (*lbl,           0, 1, 1, 2, Gtk::FILL, Gtk::FILL, 5, 3);
   pagBuraco.attach (numBuracoCards, 1, 2, 1, 2, Gtk::FILL | Gtk::EXPAND, Gtk::FILL, 5, 3);
   numBuracoCards.set_active_text (BuracoCards::get ()[Buraco::CARDS2DEAL]);

   nb.append_page (pagBuraco, _("_Buraco"), true);
#endif

#ifdef WITH_HEARTS
   Gtk::Box& pagHearts (*manage (new Gtk::HBox));
   lbl = manage (new Gtk::Label (_("_Points to end game:"), Gtk::ALIGN_LEFT, Gtk::ALIGN_CENTER, true));
   lbl->set_mnemonic_widget (maxHeartsPoints);
   pagHearts.pack_start (*lbl, Gtk::PACK_SHRINK, 5);
   pagHearts.pack_start (maxHeartsPoints, Gtk::PACK_EXPAND_WIDGET, 5);

   nb.append_page (pagHearts, _("_Hearts"), true);
#endif

#ifdef WITH_ROVHULT
   Gtk::Table& pagRovhult (*manage (new Gtk::Table (3, 2)));
   lbl = manage (new Gtk::Label (_("_Nuke card (default 10):"), Gtk::ALIGN_LEFT, Gtk::ALIGN_CENTER, true));
   lbl->set_mnemonic_widget (cardNuke);
   pagRovhult.attach (*lbl,        0, 1, 0, 1, Gtk::FILL, Gtk::FILL, 5, 3);
   pagRovhult.attach (cardNuke,    1, 2, 0, 1, Gtk::FILL | Gtk::EXPAND, Gtk::FILL, 5, 3);
   cardNuke.set_active_text (CardValue::get ()[Rovhult::cardNuke]);
   cardNuke.signal_changed ().connect (bind (mem_fun (*this, &Settings::chgValueRovhult), 0));

   lbl = manage (new Gtk::Label (_("Re_verse card (default 7):"), Gtk::ALIGN_LEFT, Gtk::ALIGN_CENTER, true));
   lbl->set_mnemonic_widget (cardReverse);
   pagRovhult.attach (*lbl,        0, 1, 1, 2, Gtk::FILL, Gtk::FILL, 5, 3);
   pagRovhult.attach (cardReverse, 1, 2, 1, 2, Gtk::FILL | Gtk::EXPAND, Gtk::FILL, 5, 3);
   cardReverse.set_active_text (CardValue::get ()[Rovhult::cardReverse]);
   cardReverse.signal_changed ().connect (bind (mem_fun (*this, &Settings::chgValueRovhult), 1));

   lbl = manage (new Gtk::Label (_("_Skip card (default 8):"), Gtk::ALIGN_LEFT, Gtk::ALIGN_CENTER, true));
   lbl->set_mnemonic_widget (cardSkip);
   pagRovhult.attach (*lbl,        0, 1, 2, 3, Gtk::FILL, Gtk::FILL, 5, 3);
   pagRovhult.attach (cardSkip,    1, 2, 2, 3, Gtk::FILL | Gtk::EXPAND, Gtk::FILL, 5, 3);
   cardSkip.set_active_text (CardValue::get ()[Rovhult::cardSkip]);
   cardSkip.signal_changed ().connect (bind (mem_fun (*this, &Settings::chgValueRovhult), 2));

   nb.append_page (pagRovhult, _("_Rovhult"), true);
#endif

#ifdef WITH_SGTMAYOR
   Gtk::Box& pagSgtMayor (*manage (new Gtk::HBox));
   lbl = manage (new Gtk::Label (_("_Tricks to win game:"), Gtk::ALIGN_LEFT, Gtk::ALIGN_CENTER, true));
   lbl->set_mnemonic_widget (tricksSgtMayor);
   pagSgtMayor.pack_start (*lbl, Gtk::PACK_SHRINK, 5);
   pagSgtMayor.pack_start (tricksSgtMayor, Gtk::PACK_EXPAND_WIDGET, 5);

   nb.append_page (pagSgtMayor, _("Sgt. _Mayor"), true);
#endif

#if !defined (WITH_BURACO) && !defined (WITH_HEARTS) && !defined (WITH_ROVHULT) && !defined (WITH_SGTMAYOR)
   nb.set_show_tabs (0);
#endif

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

    startGame = GameTypes::get ()[gameType.get_active_text ()];

#ifdef WITH_ROVHULT
    Rovhult::cardNuke = static_cast<CardWidget::NUMBERS> (CardValue::get ()[cardNuke.get_active_text ()]);
    Rovhult::cardReverse = static_cast<CardWidget::NUMBERS> (CardValue::get ()[cardReverse.get_active_text ()]);
    Rovhult::cardSkip = static_cast<CardWidget::NUMBERS> (CardValue::get ()[cardSkip.get_active_text ()]);
#endif

#ifdef WITH_BURACO
    Buraco::CARDS2DEAL = BuracoCards::get ()[numBuracoCards.get_active_text ()];
#endif
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

#ifdef WITH_ROVHULT
//-----------------------------------------------------------------------------
/// Callback when a value of the Røvhult-settings have been changed
/// \param which: ID of changed control
//-----------------------------------------------------------------------------
void Settings::chgValueRovhult (unsigned int which) {
   TRACE8 ("Settings::chgValueRovhult (unsigned int) - " << which);
   Check1 (which < 3);

   XGP::EnumEntry* fields[] = { &cardNuke, &cardReverse, &cardSkip };
   bool valuesOK (true);
   for (unsigned int i (0); i < (sizeof (fields) / sizeof (*fields)); ++i) {
      Check3 (fields[i]);
      if ((i != which)
	  && (fields[i]->get_active_text () == fields[which]->get_active_text ())) {
	 valuesOK = false;
	 break;
      }
   }

   ok->set_sensitive (valuesOK);
}

#endif
