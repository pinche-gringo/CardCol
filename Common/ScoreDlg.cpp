//$Id$

//PROJECT     : Cardgames
//SUBSYSTEM   : Common
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


#include <climits>

#include <cardgames-cfg.h>

#include <gtkmm/label.h>
#include <gtkmm/separator.h>

#include <Check.h>
#include <Trace_.h>

#include "ScoreDlg.h"


/*--------------------------------------------------------------------------*/
//Purpose   : (Default-)Constructor; Shows the dialog
//Parameters: playerNames: Vector with names of players
/*--------------------------------------------------------------------------*/
ScoreDlg::ScoreDlg (const std::vector<std::string>& playerNames)
   : XDialog (OK), client (new Gtk::HBox) {
   TRACE9 ("ScoreDlg::ScoreDlg ()");
   set_title (_("Score"));

   for (unsigned int i (0); i < playerNames.size (); ++i) {
      aColumns.push_back (new column ());
      client->pack_start (aColumns.back ()->getBox (), true, true, 15);
   }
   update (playerNames);

   client->show ();
   get_vbox ()->pack_start (*client, Gtk::SHRINK, 5);

   show ();
}

/*--------------------------------------------------------------------------*/
//Purpose   : Destructor
/*--------------------------------------------------------------------------*/
ScoreDlg::~ScoreDlg () {
   for (std::vector<column*>::iterator i (aColumns.begin ());
        i != aColumns.end (); ++i)
      delete *i;
}


/*--------------------------------------------------------------------------*/
//Purpose   : Adds a line to the scores
//Parameters: aPoints: Array of points
/*--------------------------------------------------------------------------*/
void ScoreDlg::addPoints (int aPoints[]) {
   for (std::vector<column*>::iterator i (aColumns.begin ());
        i != aColumns.end (); ++i) {
      TRACE5 ("ScoreDlg::addPoints (unsinged int[]) - " << *aPoints);
      (*i)->addEntry (*aPoints++);
   }
}

/*--------------------------------------------------------------------------*/
//Purpose   : Adds a line to the scores
//Parameters: aPoints: Vector of points
/*--------------------------------------------------------------------------*/
void ScoreDlg::addPoints (const std::vector<int>& aPoints) {
   Check1 (aPoints.size () <= aColumns.size ());

   std::vector<int>::const_iterator p (aPoints.begin ());
   for (std::vector<column*>::iterator i (aColumns.begin ());
        i != aColumns.end (); ++i) {
      TRACE5 ("ScoreDlg::addPoints (std::vector<int>&) - " << *p);
      (*i)->addEntry (*p);
      ++p;
   }
}

/*--------------------------------------------------------------------------*/
//Purpose   : Callback after selecting OK; Hides the dialog
/*--------------------------------------------------------------------------*/
void ScoreDlg::okEvent () {
   hide ();
}


/*--------------------------------------------------------------------------*/
//Purpose   : Gets the player with highest number of points and the points
//Parameters: points: Reference where to put the highest points
//            player: Reference where to put the player with the highest points
/*--------------------------------------------------------------------------*/
void ScoreDlg::getMaxPoints (int& points, unsigned int& player) {
   points = INT_MIN;
   for (std::vector<column*>::iterator i (aColumns.begin ());
        i != aColumns.end (); ++i) {
      TRACE9 ("ScoreDlg::addPoints (int&, unsigned int&) - " << points
              << '/' << (*i)->getPoints () << ": "
              << (((*i)->getPoints () > points) ? '>' : '<'));
      if ((*i)->getPoints () > points) {
         player = i - aColumns.begin ();
         points = (*i)->getPoints ();
      }
   }
}

/*--------------------------------------------------------------------------*/
//Purpose   : Gets the player with highest number of points and the points
//Parameters: points: Reference where to put the highest points
//            player: Reference where to put the player with the highest points
/*--------------------------------------------------------------------------*/
void ScoreDlg::getMinPoints (int& points, unsigned int& player) {
   points = INT_MAX;
   for (std::vector<column*>::iterator i (aColumns.begin ());
        i != aColumns.end (); ++i)
      if ((*i)->getPoints () < points) {
         player = i - aColumns.begin ();
         points = (*i)->getPoints ();
      }
}

/*--------------------------------------------------------------------------*/
//Purpose   : Changes the names of the playing people
//Parameters: newNames: Array holding the new names of the players
/*--------------------------------------------------------------------------*/
void ScoreDlg::update (const std::vector<std::string>& playerNames) {
   Check1 (playerNames.size () <= aColumns.size ());

   std::vector<std::string>::const_iterator p (playerNames.begin ());
   for (std::vector<column*>::iterator i (aColumns.begin ());
        i != aColumns.end (); ++i) {
      (*i)->setTitle (*p);
      ++p;
   }
}


/*--------------------------------------------------------------------------*/
//Purpose   : Constructor
/*--------------------------------------------------------------------------*/
ScoreDlg::column::column ()
   : pBox (new Gtk::VBox ()) , pTitle (new Gtk::Label ())
     , pSum (new NumLabel (0)) , pSep (new Gtk::HSeparator ()) {
   pBox->show ();
   pTitle->show ();
   pSum->show ();
   pSep->show ();

   pTitle->set_justify (Gtk::JUSTIFY_CENTER);
   pTitle->set_alignment (0.5, 0);
   pSum->set_alignment (1.0, 0);

   pBox->pack_start (*pTitle, false, false, 5);
   pBox->pack_end (*pSum, false, false, 3);
   pBox->pack_end (*pSep, false, false, 0);
}

/*--------------------------------------------------------------------------*/
//Purpose   : Destructor
/*--------------------------------------------------------------------------*/
ScoreDlg::column::~column () {
}
      

/*--------------------------------------------------------------------------*/
//Purpose   : Adds a value to the column
//Parameters: points: Number to add to column
/*--------------------------------------------------------------------------*/
void ScoreDlg::column::addEntry (int points) {
   Check3 (pBox); 
   NumLabel* label (Gtk::manage (new NumLabel (points)));
   label->set_alignment (1.0, 0);
   label->show ();
   pBox->pack_start (*label, false, false, 0);

   pSum->getAttribute () += points;
   pSum->update ();
}

/*--------------------------------------------------------------------------*/
//Purpose   : Sets the "title" (the first line) of the column
//Parameters: title: New "title"
/*--------------------------------------------------------------------------*/
void ScoreDlg::column::setTitle (const std::string& title) {
   Check3 (pTitle);
   pTitle->set_text (title);
}
