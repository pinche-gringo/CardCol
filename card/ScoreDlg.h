#ifndef SCOREDLG_H
#define SCOREDLG_H

//$Id: ScoreDlg.h,v 1.1 2009/06/14 07:03:27 g17m0 Exp $

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


#include <vector>

#include <boost/scoped_ptr.hpp>

#include <YGP/ANumeric.h>

#include <XGP/XDialog.h>
#include <XGP/XAttrLabel.h>


namespace Gtk {
   class Box;
   class Label;
   class Widget;
   class Separator;
}

namespace Card {
   class Player;
}


namespace Card {

// Class to display the score of the cardgames
class ScoreDlg : public XGP::XDialog {
 public:
   ScoreDlg (const std::vector<Player*>& player);
   virtual ~ScoreDlg ();

   /// Creates a new score-dialogue; showing the passed players
   /// \param player Vector holding name of all players
   /// \returns ScoreDlg* Pointer to the created dialoge
   static ScoreDlg* create (const std::vector<Player*>& player) {
      return new ScoreDlg (player); }

   void update (const std::vector<Player*>& player);

   void addPoints (int aPoints[]);
   void addPoints (const std::vector<int>& aPoints);

   void getMaxPoints (int& points, unsigned int& player);
   void getMinPoints (int& points, unsigned int& player);

   static int LASTX;
   static int LASTY;

   static void display (ScoreDlg** widget);
   void display ();

 private:
   //Prohibited manager functions
   ScoreDlg (const ScoreDlg& other);
   const ScoreDlg& operator= (const ScoreDlg& other);

   virtual void okEvent ();

   typedef boost::scoped_ptr<Gtk::Box>       PBox;
   typedef boost::scoped_ptr<Gtk::Label>     PLabel;
   typedef boost::scoped_ptr<Gtk::Separator> PSeparator;

   PBox client;

   class column {
    public:
      column ();
      ~column ();

      void addEntry (int points);
      void setTitle (const Glib::ustring& title);

      int getPoints () const { return pSum->getAttribute (); }
      Gtk::Box& getBox () const { return *pBox; }

    private:
      typedef XGP::XAttributeLabel2<YGP::ANumeric> NumLabel;
      typedef boost::scoped_ptr<NumLabel>          PNumLabel;

      PBox       pBox;
      PLabel     pTitle;
      PNumLabel  pSum;
      PSeparator pSep;
      Gtk::Widget* pLastEntry;      ///< Last entry-widget added, to insert the next one after it
   };

   std::vector<column*> aColumns;
};

}

#endif
