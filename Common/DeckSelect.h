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
#include <gtk--/packer.h>
#include <gtk--/dialog.h>


using namespace Gtk;


// Class to select the card decks to use
class ICarddeckSelectDlg : public Dialog {
 public:
   ICarddeckSelectDlg (const char* path = NULL);
   virtual ~ICarddeckSelectDlg ();

   typedef enum { OK, APPLY, CANCEL } commands;

   void getSelection (std::string& deck, std::string& back) const {
      deck = aFiles[0] + aFiles[offDeck];
      back = aFiles[0] + aFiles[offBack]; }

 protected:
   virtual void command (commands action) = 0;
   virtual void deckSelect (unsigned int offset);
   virtual void backSelect (unsigned int offset);

   unsigned int offDeck;
   unsigned int offBack;

 private:
   // Prohibited manager-functions
   ICarddeckSelectDlg ();
   ICarddeckSelectDlg (const ICarddeckSelectDlg&);
   
   const ICarddeckSelectDlg& operator= (const ICarddeckSelectDlg&);

   Button ok;
   Button apply;
   Button cancel;

   HBox   boxDecks;
   Label  txtDecks;
   Button selDeck;
   Packer decks;

   HBox   boxBack;
   Label  txtBack;
   Packer backs;
   Button selBack;

   vector<Button*> aDecks;
   vector<Button*> aBacks;
   vector<std::string> aFiles;

   static const char* const DEFAULTFILE = "14.xpm";
};


template <class T>
class CarddeckSelectDlg : public ICarddeckSelectDlg {
 public:
   typedef void (T::*PCALLBACK) (ICarddeckSelectDlg::commands);

   CarddeckSelectDlg (T& parent, PCALLBACK callback, const char* path = NULL)
      : ICarddeckSelectDlg (path), obj (parent), pCallback (callback) { }
   virtual ~CarddeckSelectDlg () { }

   static const CarddeckSelectDlg* create (T& parent, PCALLBACK callback,
                                           const char* path = NULL) {
      return new CarddeckSelectDlg (parent, callback, path);
   }

 protected:
   virtual void command (commands action) {
      (obj.*pCallback) (action);
      if (action != APPLY)
         delete this;
   }

 private:
   T& obj;
   PCALLBACK pCallback;
};


#endif
