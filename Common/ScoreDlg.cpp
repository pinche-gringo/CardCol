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

#include <gtkmm/label.h>
#include <gtkmm/separator.h>

#include <Trace_.h>

#include "HeartsScore.h"


/*--------------------------------------------------------------------------*/
//Purpose   : (Default-)Constructor; Shows the dialog
//Parameters: playerNames: Vector with names of players
/*--------------------------------------------------------------------------*/
HeartsScoreDlg::HeartsScoreDlg (const std::vector<std::string>& playerNames)
   : XDialog (OK), client (new Gtk::HBox) {
   TRACE9 ("HeartsScoreDlg::HeartsScoreDlg ()");
   Check1 ((sizeof (aColumns) / sizeof (aColumns[0])) < playerNames.size ());

   set_title (_("Score"));

   update (playerNames);
   for (unsigned int i (0);
        i < (sizeof (aColumns) / sizeof (aColumns[0])); ++i)
      client->pack_start (aColumns[i].getBox (), true, true, 5);

   client->show ();
   get_vbox ()->pack_start (*client, Gtk::SHRINK, 5);

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
   TRACE9 ("HeartsScoreDlg::addPoints (4 x unsinged int) - " << points0 << '/'
           << points1 << '/' << points2 << '/' << points3);
   unsigned int values[] = { points0, points1, points2, points3 };
   addPoints (values);
}

/*--------------------------------------------------------------------------*/
//Purpose   : Adds a line to the scores
//Parameters: aPoints: Array of (4) points
/*--------------------------------------------------------------------------*/
void HeartsScoreDlg::addPoints (unsigned int aPoints[4]) {
   for (unsigned int i (0); i < 4; ++i) {
      TRACE5 ("HeartsScoreDlg::addPoints (unsinged int[4]) - " << aPoints[i]);
      aColumns[i].addEntry (aPoints[i]);
   }
}

/*--------------------------------------------------------------------------*/
//Purpose   : Callback after selecting OK; Hides the dialog
/*--------------------------------------------------------------------------*/
void HeartsScoreDlg::okEvent () {
   hide ();
}


/*--------------------------------------------------------------------------*/
//Purpose   : Gets the player with highest number of points and the points
//Parameters: points: Reference where to put the highest points
//            player: Reference where to put the player with the highest points
/*--------------------------------------------------------------------------*/
void HeartsScoreDlg::getMaxPoints (unsigned int& points, unsigned int& player) {
   points = 0;
   for (unsigned int i (0);
        i < (sizeof (aColumns) / sizeof (aColumns[0])); ++i)
      if (aColumns[i].getPoints () > points) {
         player = i;
         points = aColumns[i].getPoints ();
   }
}

/*--------------------------------------------------------------------------*/
//Purpose   : Gets the player with highest number of points and the points
//Parameters: points: Reference where to put the highest points
//            player: Reference where to put the player with the highest points
/*--------------------------------------------------------------------------*/
void HeartsScoreDlg::getMinPoints (unsigned int& points, unsigned int& player) {
   points = -1U;
   for (unsigned int i (0);
        i < (sizeof (aColumns) / sizeof (aColumns[0])); ++i)
      if (aColumns[i].getPoints () < points) {
         player = i;
         points = aColumns[i].getPoints ();
   }
}

/*--------------------------------------------------------------------------*/
//Purpose   : Changes the names of the playing people
//Parameters: newNames: Array holding the new names of the players
/*--------------------------------------------------------------------------*/
void HeartsScoreDlg::update (const std::vector<std::string>& playerNames) {
   for (unsigned int i (0);
        i < (sizeof (aColumns) / sizeof (aColumns[0])); ++i)
      aColumns[i].setTitle (playerNames[i]);
}


/*--------------------------------------------------------------------------*/
//Purpose   : Constructor
/*--------------------------------------------------------------------------*/
HeartsScoreDlg::column::column ()
   : pBox (new Gtk::VBox ()) , pTitle (new Gtk::Label ())
     , pSum (new IntLabel (0U)) , pSep (new Gtk::HSeparator ()) {
   pBox->show ();
   pTitle->show ();
   pSum->show ();
   pSep->show ();

   pBox->pack_start (*pTitle, false, false, 5);
   pBox->pack_end (*pSum, false, false, 3);
   pBox->pack_end (*pSep, false, false, 0);
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
   IntLabel* label (Gtk::manage (new IntLabel (points)));
   label->show ();
   pBox->pack_start (*label, false, false, 0);

   pSum->getAttribute () += points;
   pSum->update ();
}

/*--------------------------------------------------------------------------*/
//Purpose   : Sets the "title" (the first line) of the column
//Parameters: title: New "title"
/*--------------------------------------------------------------------------*/
void HeartsScoreDlg::column::setTitle (const std::string& title) {
   Check3 (pTitle);
   pTitle->set_text (title);
}
