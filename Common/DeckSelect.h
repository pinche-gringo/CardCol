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

#include <gtkmm/box.h>
#include <gtkmm/label.h>
#include <gtkmm/button.h>
#include <gtkmm/buttonbox.h>

#include <XGP/Folder.h>
#include <XGP/XDialog.h>


// Class to select the card decks to use
class ICarddeckSelectDlg : public XGP::XDialog {
 public:
   ICarddeckSelectDlg (const char* path, const std::string& deck,
                       const std::string& back);
   virtual ~ICarddeckSelectDlg ();

   void getSelection (std::string& deck, std::string& back) const {
      deck = aFiles[0] + aFiles[offDeck];
      back = aFiles[0] + "decks/" + aFiles[offBack]; }

 protected:
   virtual void command (int action);
   virtual void deckSelect (unsigned int offset);
   virtual void backSelect (unsigned int offset);

   Gtk::Button* createButton (const std::string& file);
   void setButtonImage (Gtk::Button& button, const std::string& file);

   int offDeck;
   int offBack;

 private:
   // Prohibited manager-functions
   ICarddeckSelectDlg ();
   ICarddeckSelectDlg (const ICarddeckSelectDlg&);
   
   const ICarddeckSelectDlg& operator= (const ICarddeckSelectDlg&);

   Gtk::HBox   boxDecks;
   Gtk::Label  txtDecks;
   Gtk::Button selDeck;
   XGP::Folder decks;

   Gtk::Button selBack;
   Gtk::HBox   boxBack;
   Gtk::Label  txtBack;
   XGP::Folder backs;

   Gtk::HButtonBox box;

   std::vector<Gtk::Button*> aDecks;
   std::vector<Gtk::Button*> aBacks;
   std::vector<std::string> aFiles;
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
      CarddeckSelectDlg<T>* dlg (new CarddeckSelectDlg (parent, callback,
                                                     path, deck, back));
      dlg->signal_response ().connect (mem_fun (*dlg, &CarddeckSelectDlg<T>::free));
      dlg->get_window ()->set_transient_for (parent.get_window ());
      return dlg;
   }

 protected:
   virtual void okEvent () {
      (obj.*pCallback) (*this);
      ICarddeckSelectDlg::okEvent (); }
   virtual void command (int action) {
      ICarddeckSelectDlg::command (action);
      if (action == Gtk::RESPONSE_APPLY)
         (obj.*pCallback) (*this);
   }

 private:
   T& obj;
   PCALLBACK pCallback;
};


#endif
