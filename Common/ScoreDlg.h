#ifndef SCOREDLG_H
#define SCOREDLG_H

//$Id$

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
#include <vector>

#include <ANumeric.h>
#include <SmartPtr.h>

#include <XDialog.h>
#include <XAttrLabel.h>


namespace Gtk {
   class HBox;
   class Label;
   class Widget;
   class Separator;
}


// Class to display the score of the cardgames
class ScoreDlg : public XDialog {
 public:
   ScoreDlg (const std::vector<std::string>& playerNames);
   virtual ~ScoreDlg ();

   static ScoreDlg* perform (const std::vector<std::string>& playerNames) {
      return new ScoreDlg (playerNames); }

   void update (const std::vector<std::string>& playerNames);

   void addPoints (int aPoints[]);
   void addPoints (const std::vector<int>& aPoints);

   void getMaxPoints (int& points, unsigned int& player);
   void getMinPoints (int& points, unsigned int& player);

 private:
   //Prohibited manager functions
   ScoreDlg (const ScoreDlg& other);
   const ScoreDlg& operator= (const ScoreDlg& other);

   virtual void okEvent ();

   typedef SmartPtr<Gtk::Box>        PBox;
   typedef SmartPtr<Gtk::Label>      PLabel;
   typedef SmartPtr<Gtk::Separator>  PSeparator;

   PBox client;

   class column {
    public:
      column ();
      ~column ();

      void addEntry (int points);
      void setTitle (const std::string& title);

      int getPoints () const { return pSum->getAttribute (); }
      Gtk::Box& getBox () const { return *pBox; }

    private:
      typedef XAttributeLabel2<ANumeric> NumLabel;
      typedef SmartPtr<NumLabel>         PNumLabel;

      PBox       pBox;
      PLabel     pTitle;
      PNumLabel  pSum;
      PSeparator pSep;
   };

   std::vector<column*> aColumns;
};

#endif
