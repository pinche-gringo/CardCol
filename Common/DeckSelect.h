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

#include <gtk--/dialog.h>

#include "SmartPtr.h"


// Forward declarations
namespace Gtk {
   class HBox;
   class Label;
   class Button;
   class Packer;
}
   
using namespace Gtk;


// Class to select the card decks to use
class ICarddeckSelectDlg : public Dialog {
 public:
   ICarddeckSelectDlg (const char* path = NULL);
   virtual ~ICarddeckSelectDlg ();

   typedef enum { OK, APPLY, CANCEL } commands;

 protected:
   virtual void command (commands action) = 0;

 private:
   typedef SmartPtr<HBox>   PHBox;
   typedef SmartPtr<Label>  PLabel;
   typedef SmartPtr<Button> PButton;
   typedef SmartPtr<Packer> PPacker;

   // Prohibited manager-functions
   ICarddeckSelectDlg ();
   ICarddeckSelectDlg (const ICarddeckSelectDlg&);
   
   const ICarddeckSelectDlg& operator= (const ICarddeckSelectDlg&);

   PButton ok;
   PButton apply;
   PButton cancel;
   PLabel  txtDecks;
   PPacker decks;
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
      (obj.*pCallback) (action); }

 private:
   T& obj;
   PCALLBACK pCallback;
};


#endif
