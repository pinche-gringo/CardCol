#ifndef HEARTSSCORE_H
#define HEARTSSCORE_H

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

#include <XDialog.h>

#include <Check.h>

#include <SmartPtr.h>
#include <XAttrLabel.h>


namespace Gtk {
   class HBox;
   class Label;
   class Widget;
   class Separator;
}


// Class to display the score of the Hearts cardgame
class HeartsScoreDlg : public XDialog {
 public:
   HeartsScoreDlg (const vector<string>& playerNames);
   virtual ~HeartsScoreDlg ();

   static HeartsScoreDlg* perform (const vector<string>& playerNames) {
      return new HeartsScoreDlg (playerNames); }

   void update (const vector<string>& playerNames);

   void addPoints (unsigned int points0, unsigned int points1,
                   unsigned int points2, unsigned int points3);
   void addPoints (unsigned int aPoints[4]);

   void getMaxPoints (unsigned int& points, unsigned int& player);
   void getMinPoints (unsigned int& points, unsigned int& player);

 private:
   //Prohibited manager functions
   HeartsScoreDlg (const HeartsScoreDlg& other);
   const HeartsScoreDlg& operator= (const HeartsScoreDlg& other);

   virtual void okEvent ();

   typedef SmartPtr<Gtk::Box>        PBox;
   typedef SmartPtr<Gtk::Label>      PLabel;
   typedef SmartPtr<Gtk::Separator>  PSeparator;

   PBox client;

   class column {
    public:
      column ();
      ~column ();

      void addEntry (unsigned int points);
      void setTitle (const string& title);

      unsigned int getPoints () const { return pSum->getAttribute (); }
      Gtk::Box& getBox () const { return *pBox; }

    private:
      typedef XAttributeLabel2<unsigned int> IntLabel;
      typedef SmartPtr<IntLabel>            PIntLabel;

      PBox       pBox;
      PLabel     pTitle;
      PIntLabel  pSum;
      PSeparator pSep;
   } aColumns[4];
};

#endif
