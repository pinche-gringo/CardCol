//$Id$

//PROJECT     : Cardgames
//SUBSYSTEM   : Hearts
//REFERENCES  :
//TODO        :
//BUGS        :
//REVISION    : $Revision$
//AUTHOR      : Markus Schwab
//CREATED     : 02.01.2003
//COPYRIGHT   : Anticopyright (A) 2003

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

#include <gtk--/label.h>
#include <gtk--/button.h>
#include <gtk--/separator.h>

#define CHECK 9
#define TRACELEVEL 9
#include <Trace_.h>
#include <ANumeric.h>

#include "HeartsScore.h"


/*--------------------------------------------------------------------------*/
//Purpose   : (Default-)Constructor; Shows the dialog
//Parameters: points0: Points of player 0
//            points1: Points of player 1
//            points2: Points of player 2
//            points3: Points of player 3
/*--------------------------------------------------------------------------*/
HeartsScoreDlg::HeartsScoreDlg (unsigned int points0, unsigned int points1,
                                unsigned int points2, unsigned int points3)
   : Dialog (), client (new Gtk::HBox), ok (new Gtk::Button (_("OK"))) {
   TRACE9 ("HeartsScoreDlg::HeartsScoreDlg (unsinged int, unsinged int, "
           "unsinged int, unsinged int) - " << points0 << '/' << points1
           << '/' << points2 << '/' << points3);
   Check3 (ok);
   ok->set_usize (90, 30);
   ok->show ();
   ok->clicked.connect (bind (slot (this, &HeartsScoreDlg::command), OK));
   get_action_area ()->pack_start (*ok, false, false, 5);
   ok->set_flags (GTK_CAN_DEFAULT);
   ok->grab_default ();

   for (unsigned int i (0);
        i < (sizeof (aColumns) / sizeof (aColumns[0])); ++i) {
      string player (_("Player %1"));
      player.replace (player.find ("%1"), 2, (char)(i + '0'));

      aColumns[i].setTitle (player);

      client->pack_start (aColumns[i].getBox (), true, true, 5);
   }

   client->show ();
   get_vbox ()->pack_start (*client, false, false, 5);


   addPoints (points0, points1, points2, points3);
   show ();
}

/*--------------------------------------------------------------------------*/
//Purpose   : Destructor
/*--------------------------------------------------------------------------*/
HeartsScoreDlg::~HeartsScoreDlg () {
}


/*--------------------------------------------------------------------------*/
//Purpose   : Adds a line to the scores
//Parameters: points0: Points of player 0
//            points1: Points of player 1
//            points2: Points of player 2
//            points3: Points of player 3
/*--------------------------------------------------------------------------*/
void HeartsScoreDlg::addPoints (unsigned int points0, unsigned int points1,
                                unsigned int points2, unsigned int points3) {
   static unsigned int* values[] = { &points0, &points1, &points2, &points3 };

   for (unsigned int i (0);
        i < (sizeof (aColumns) / sizeof (aColumns[0])); ++i) {
      aColumns[i].addEntry (*values[i]);
   }
}

/*--------------------------------------------------------------------------*/
//Purpose   : Callback after button-events
//Parameters: cmd: ID of pressed button
/*--------------------------------------------------------------------------*/
void HeartsScoreDlg::command (commands cmd) {
   hide ();
}


/*--------------------------------------------------------------------------*/
//Purpose   : Constructor
/*--------------------------------------------------------------------------*/
HeartsScoreDlg::column::column ()
   : pBox (new Gtk::VBox ()) , pTitle (new Gtk::Label ())
     , pSum (new Gtk::Label ()) , pSep (new Gtk::HSeparator ()) {
   pBox->show ();
   pTitle->show ();
   pSum->show ();
   pSep->show ();

   pBox->pack_start (*pTitle, false, false, 5);
   pBox->pack_end (*pSep, false, false, 5);
   pBox->pack_end (*pSum, false, false, 5);
}

/*--------------------------------------------------------------------------*/
//Purpose   : Destructor
/*--------------------------------------------------------------------------*/
HeartsScoreDlg::column::~column () {
}
      

/*--------------------------------------------------------------------------*/
//Purpose   : Adds a value to the column
//Parameters: points: Number to add to column
/*--------------------------------------------------------------------------*/
void HeartsScoreDlg::column::addEntry (unsigned int points) {
   Check3 (pBox); 
   Gtk::Label* label (Gtk::manage (new Gtk::Label
                                   (ANumeric::toString ((unsigned long)points))));
   label->show ();
   pBox->pack_start (*label, false, false, 5);
}

/*--------------------------------------------------------------------------*/
//Purpose   : Sets the "title" (the first line) of the column
//Parameters: title: New "title"
/*--------------------------------------------------------------------------*/
void HeartsScoreDlg::column::setTitle (const string& title) {
   Check3 (pTitle);
   pTitle->set_text (title);
}
