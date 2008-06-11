#ifndef SCOREDLG_H
#define SCOREDLG_H

//$Id$

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
// along with libYGP.  If not, see <http://www.gnu.org/licenses/>.


#include <vector>

#include <YGP/ANumeric.h>
#include <YGP/SmartPtr.h>

#include <XGP/XDialog.h>
#include <XGP/XAttrLabel.h>


namespace Gtk {
   class HBox;
   class Label;
   class Widget;
   class Separator;
}

class Player;


// Class to display the score of the cardgames
class ScoreDlg : public XGP::XDialog {
 public:
   ScoreDlg (const std::vector<Player*>& player);
   virtual ~ScoreDlg ();

   static ScoreDlg* create (const std::vector<Player*>& player) {
      return new ScoreDlg (player); }

   void update (const std::vector<Player*>& player);

   void addPoints (int aPoints[]);
   void addPoints (const std::vector<int>& aPoints);

   void getMaxPoints (int& points, unsigned int& player);
   void getMinPoints (int& points, unsigned int& player);

   static int LASTX;
   static int LASTY;

 private:
   //Prohibited manager functions
   ScoreDlg (const ScoreDlg& other);
   const ScoreDlg& operator= (const ScoreDlg& other);

   virtual void okEvent ();

   typedef YGP::SmartPtr<Gtk::Box>        PBox;
   typedef YGP::SmartPtr<Gtk::Label>      PLabel;
   typedef YGP::SmartPtr<Gtk::Separator>  PSeparator;

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
      typedef YGP::SmartPtr<NumLabel>              PNumLabel;

      PBox       pBox;
      PLabel     pTitle;
      PNumLabel  pSum;
      PSeparator pSep;
   };

   std::vector<column*> aColumns;
};

#endif
