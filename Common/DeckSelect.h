#ifndef DECKSELECT_H
#define DECKSELECT_H

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

#include <gtk--/box.h>
#include <gtk--/label.h>
#include <gtk--/button.h>
#include <gtk--/table.h>
#include <gtk--/buttonbox.h>
#include <gtk--/scrolledwindow.h>

#include <XDialog.h>

using namespace Gtk;


// Class to select the card decks to use
class ICarddeckSelectDlg : public XDialog {
 public:
   ICarddeckSelectDlg (const char* path, const std::string& deck,
                       const std::string& back);
   virtual ~ICarddeckSelectDlg ();

   typedef enum { APPLY } commands;

   void getSelection (std::string& deck, std::string& back) const {
      deck = aFiles[0] + aFiles[offDeck];
      back = aFiles[0] + aFiles[offBack]; }

 protected:
   virtual void command (commands action);
   virtual void deckSelect (unsigned int offset);
   virtual void backSelect (unsigned int offset);

   int offDeck;
   int offBack;

 private:
   // Prohibited manager-functions
   ICarddeckSelectDlg ();
   ICarddeckSelectDlg (const ICarddeckSelectDlg&);
   
   const ICarddeckSelectDlg& operator= (const ICarddeckSelectDlg&);

   Button apply;

   HBox   boxDecks;
   Label  txtDecks;
   Button selDeck;
   Table  decks;
   ScrolledWindow scrlDeck;

   Button selBack;
   HBox   boxBack;
   Label  txtBack;
   Table  backs;
   ScrolledWindow scrlBack;

   HButtonBox box;

   vector<Button*> aDecks;
   vector<Button*> aBacks;
   vector<std::string> aFiles;

   static const char* const DEFAULTFILE = "14.xpm";
};


template <class T>
class CarddeckSelectDlg : public ICarddeckSelectDlg {
 public:
   typedef void (T::*PCALLBACK) (const ICarddeckSelectDlg&);

   CarddeckSelectDlg (T& parent, PCALLBACK callback, const char* path,
                      const std::string& deck, const std::string& back)
      : ICarddeckSelectDlg (path, deck, back), obj (parent), pCallback (callback) { }
   virtual ~CarddeckSelectDlg () { }

   static CarddeckSelectDlg* create (T& parent, PCALLBACK callback, const char* path,
                                     const std::string& deck, const std::string& back) {
      return new CarddeckSelectDlg (parent, callback, path, deck, back);
   }

 protected:
   virtual void okEvent () {
      (obj.*pCallback) (*this);
      ICarddeckSelectDlg::okEvent (); }
   virtual void command (commands action) {
      ICarddeckSelectDlg::command (action);
      (obj.*pCallback) (*this); }

 private:
   T& obj;
   PCALLBACK pCallback;
};


#endif
