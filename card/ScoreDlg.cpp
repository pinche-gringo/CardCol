//PROJECT     : Cardgames
//SUBSYSTEM   : Common
//REFERENCES  :
//TODO        :
//BUGS        :
//AUTHOR      : Markus Schwab
//CREATED     : 02.01.2003
//COPYRIGHT   : Copyright (C) 2002 - 2004, 2007 - 2009, 2026

// This file is part of CardCol.
//
// CardCol is free software: you can redistribute it and/or modify
// it under the terms of the GNU General Public License as published by
// the Free Software Foundation, either version 3 of the License, or
// (at your option) any later version.
//
// CardCol is distributed in the hope that it will be useful,
// but WITHOUT ANY WARRANTY; without even the implied warranty of
// MERCHANTABILITY or FITNESS FOR A PARTICULAR PURPOSE.  See the
// GNU General Public License for more details.
//
// You should have received a copy of the GNU General Public License
// along with CardCol.  If not, see <http://www.gnu.org/licenses/>.


#include <climits>

#include <cardgames-cfg.h>

#include <gtkmm/label.h>
#include <gtkmm/separator.h>

#include <YGP/Check.h>
#include <YGP/Trace.h>

#include "Pile.h"
#include "Player.h"
#include "ScoreDlg.h"


namespace Card {

int ScoreDlg::LASTX(-1);
int ScoreDlg::LASTY(-1);

//-----------------------------------------------------------------------------
/// (Default-)Constructor; Shows the dialog
/// \param player Vector with player
//-----------------------------------------------------------------------------
ScoreDlg::ScoreDlg(const std::vector<Player*>& player)
   : XDialog(OK), client(new Card::HBox), aColumns() {
   TRACE9("ScoreDlg::ScoreDlg()");
   set_title(_("Score"));

   for (unsigned int i(0); i < player.size(); ++i) {
      aColumns.push_back(new column());
      Gtk::Box& box(aColumns.back()->getBox());
      box.set_hexpand(); box.set_vexpand();
      box.set_margin_start(15); box.set_margin_end(15);
      client->append(box);
   }
   update(player);

   client->show();
   client->set_margin(5);
   get_content_area()->append(*client);
   display();
}

//-----------------------------------------------------------------------------
/// Destructor
//-----------------------------------------------------------------------------
ScoreDlg::~ScoreDlg() {
   TRACE9("ScoreDlg::~ScoreDlg()");
   for (std::vector<column*>::iterator i(aColumns.begin());
        i != aColumns.end(); ++i)
      delete *i;

   // Remark: Under GTK4 a client can no longer query a window's position
   // (see AnimWindow.h); LASTX/LASTY are kept only for source compatibility.
}


//-----------------------------------------------------------------------------
/// Adds a line to the scores
/// \param aPoints Array of points
//-----------------------------------------------------------------------------
void ScoreDlg::addPoints(int aPoints[]) {
   for (std::vector<column*>::iterator i(aColumns.begin());
        i != aColumns.end(); ++i) {
      TRACE5("ScoreDlg::addPoints(unsinged int[]) - " << *aPoints);
     (*i)->addEntry(*aPoints++);
   }
}

//-----------------------------------------------------------------------------
/// Adds a line to the scores
/// \param aPoints Vector of points
//-----------------------------------------------------------------------------
void ScoreDlg::addPoints(const std::vector<int>& aPoints) {
   Check1(aPoints.size() <= aColumns.size());

   std::vector<int>::const_iterator p(aPoints.begin());
   for (std::vector<column*>::iterator i(aColumns.begin()); i != aColumns.end(); ++i) {
      TRACE5("ScoreDlg::addPoints(std::vector<int>&) - " << *p);
      (*i)->addEntry(*p);
      ++p;
   }
}

//-----------------------------------------------------------------------------
/// Callback after selecting OK; Hides the dialog
//-----------------------------------------------------------------------------
void ScoreDlg::okEvent () {
   TRACE9 ("ScoreDlg::okEvent ()");
   // Remark: Under GTK4 a client can no longer query a window's position (see AnimWindow.h)
   hide ();
}


//-----------------------------------------------------------------------------
/// Gets the player with highest number of points and the points
/// \param points Reference where to put the highest points
/// \param player Reference where to put the player with the highest points
//-----------------------------------------------------------------------------
void ScoreDlg::getMaxPoints(int& points, unsigned int& player) {
   points = INT_MIN;
   for (std::vector<column*>::iterator i(aColumns.begin()); i != aColumns.end(); ++i) {
      TRACE9("ScoreDlg::getMaxPoints(int&, unsigned int&) - " << points << '/'
	      << (*i)->getPoints() << ": " << (((*i)->getPoints() > points) ? '>' : '<'));
      if ((*i)->getPoints() > points) {
         player = i - aColumns.begin();
         points = (*i)->getPoints();
      }
   }
}

//-----------------------------------------------------------------------------
/// Gets the player with highest number of points and the points
/// \param points Reference where to put the highest points
/// \param player Reference where to put the player with the highest points
//-----------------------------------------------------------------------------
void ScoreDlg::getMinPoints(int& points, unsigned int& player) {
   points = INT_MAX;
   for (std::vector<column*>::iterator i(aColumns.begin());
        i != aColumns.end(); ++i)
      if ((*i)->getPoints() < points) {
         player = i - aColumns.begin();
         points = (*i)->getPoints();
      }
}

//-----------------------------------------------------------------------------
/// Changes the names of the playing people
/// \param player Array holding the new player
//-----------------------------------------------------------------------------
void ScoreDlg::update(const std::vector<Player*>& player) {
   Check1(player.size() <= aColumns.size());

   std::vector<Player*>::const_iterator p(player.begin());
   for (std::vector<column*>::iterator i(aColumns.begin()); i != aColumns.end(); ++i) {
      (*i)->setTitle((*p)->getName());
      ++p;
   }
}


//-----------------------------------------------------------------------------
/// Constructor
//-----------------------------------------------------------------------------
ScoreDlg::column::column()
   : pBox(new Card::VBox()) , pTitle(new Gtk::Label())
     , pSum(new NumLabel(0)) , pSep(new Gtk::Separator(Gtk::Orientation::HORIZONTAL))
     , pLastEntry(NULL) {
   pBox->show();
   pTitle->show();
   pSum->show();
   pSep->show();

   pTitle->set_justify(Gtk::Justification::CENTER);
   pTitle->set_xalign(0.5); pTitle->set_yalign(0);
   pSum->set_xalign(1.0); pSum->set_yalign(0);

   pTitle->set_vexpand();
   pTitle->set_margin_top(5); pTitle->set_margin_bottom(5);
   pBox->append(*pTitle);
   pLastEntry = pTitle.get();

   pBox->append(*pSep);

   pSum->set_margin_top(3); pSum->set_margin_bottom(3);
   pBox->append(*pSum);
}

//-----------------------------------------------------------------------------
/// Destructor
//-----------------------------------------------------------------------------
ScoreDlg::column::~column() {
}


//-----------------------------------------------------------------------------
/// Adds a value to the column
/// \param points Number to add to column
//-----------------------------------------------------------------------------
void ScoreDlg::column::addEntry(int points) {
   Check3(pBox); Check3(pLastEntry);
   NumLabel* label(Gtk::make_managed<NumLabel>(points));
   label->set_xalign(1.0); label->set_yalign(0);
   label->show();
   pBox->insert_child_after(*label, *pLastEntry);
   pLastEntry = label;

   pSum->getAttribute() += points;
   pSum->update();
}

//-----------------------------------------------------------------------------
/// Sets the "title" (the first line) of the column
/// \param title New "title"
//-----------------------------------------------------------------------------
void ScoreDlg::column::setTitle(const Glib::ustring& title) {
   Check3(pTitle);
   pTitle->set_text(title);
}

//-----------------------------------------------------------------------------
/// Shows the passed widget (if not NULL)
/// \param widget Pointer to pointer to widget to show
/// \pre widget must not be NULL
//-----------------------------------------------------------------------------
void ScoreDlg::display(ScoreDlg** dlg) {
   Check1(dlg);
   if (*dlg)
      (*dlg)->display();
}

//-----------------------------------------------------------------------------
/// Shows the passed widget (if not NULL)
/// \param widget Pointer to pointer to widget to show
/// \pre widget must not be NULL
//-----------------------------------------------------------------------------
void ScoreDlg::display() {
   if (!get_visible()) {
      // Remark: Under GTK4 a client can no longer set a window's position (see AnimWindow.h)
      show();
   }
}

}
