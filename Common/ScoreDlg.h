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

#include <gtk--/dialog.h>

#include <Check.h>

#include <SmartPtr.h>

namespace Gtk {
   class HBox;
   class Label;
   class Button;
   class Widget;
   class Separator;
}


// Class to display the score of the Hearts cardgame
class HeartsScoreDlg : public Gtk::Dialog {
 public:
   HeartsScoreDlg (unsigned int points0, unsigned int points1,
                   unsigned int points2, unsigned int points3);
   virtual ~HeartsScoreDlg ();

   static HeartsScoreDlg* perform (unsigned int points0, unsigned int points1,
                                   unsigned int points2, unsigned int points3) {
      return new HeartsScoreDlg (points0, points1, points2, points3); }
   static HeartsScoreDlg* perform (unsigned int aPoints[4]) {
      Check1 (aPoints);
      return new HeartsScoreDlg (aPoints[0], aPoints[1], aPoints[2], aPoints[3]); }

   void addPoints (unsigned int points0, unsigned int points1,
                   unsigned int points2, unsigned int points3);
   void addPoints (unsigned int aPoints[4]) {
      Check1 (aPoints);
      addPoints (aPoints[0], aPoints[1], aPoints[2], aPoints[3]); }

 protected:
   typedef enum { OK } commands;
   void command (commands cmd);

 private:
   //Prohibited manager functions
   HeartsScoreDlg (const HeartsScoreDlg& other);
   const HeartsScoreDlg& operator= (const HeartsScoreDlg& other);

   typedef SmartPtr<Gtk::Box>        PBox;
   typedef SmartPtr<Gtk::Label>      PLabel;
   typedef SmartPtr<Gtk::Button>     PButton;
   typedef SmartPtr<Gtk::Separator>  PSeparator;

   PBox    client;
   PButton ok;

   class column {
    public:
      column ();
      ~column ();

      void addEntry (unsigned int points);
      void setTitle (const string& title);

      Gtk::Box& getBox () const { return *pBox; }

    private:
      PBox       pBox;
      PLabel     pTitle;
      PLabel     pSum;
      PSeparator pSep;
   } aColumns[4];
};

#endif
