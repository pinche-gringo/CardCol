//PROJECT     : Cardgames
//SUBSYSTEM   : ChatDlg
//REFERENCES  :
//TODO        :
//BUGS        :
//AUTHOR      : Markus Schwab
//CREATED     : 2005-06-29
//COPYRIGHT   : Copyright (C) 2005, 2008, 2026

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


#include <cardgames-cfg.h>

#include <gtkmm/box.h>
#include <gtkmm/entry.h>
#include <gtkmm/button.h>
#include <gtkmm/textview.h>
#include <gtkmm/scrolledwindow.h>

#include <YGP/Check.h>
#include <YGP/Trace.h>

#include "ChatDlg.h"


//-----------------------------------------------------------------------------
/// Constructor
//-----------------------------------------------------------------------------
ChatDlg::ChatDlg ()
   : XGP::XDialog (CANCEL), txtMsg (Gtk::make_managed<Gtk::Entry> ()),
     tvMsgs (Gtk::make_managed<Gtk::TextView> ()),
     msgs (Gtk::TextBuffer::create ()) {
   TRACE9 ("ChatDlg::ChatDlg ()");
   set_title (_("Chat"));

   Gtk::ScrolledWindow& scrlMsgs (*Gtk::make_managed<Gtk::ScrolledWindow> ());
   scrlMsgs.set_has_frame (true);
   scrlMsgs.set_child (*tvMsgs);
   scrlMsgs.set_policy (Gtk::PolicyType::AUTOMATIC, Gtk::PolicyType::AUTOMATIC);

   tvMsgs->set_editable (false);
   tvMsgs->set_wrap_mode (Gtk::WrapMode::WORD);
   tvMsgs->set_buffer (msgs);

   txtMsg->set_activates_default ();

   Check2 (get_content_area ());
   scrlMsgs.set_hexpand (); scrlMsgs.set_vexpand (); scrlMsgs.set_margin (5);
   get_content_area ()->append (scrlMsgs);
   txtMsg->set_margin (5);
   get_content_area ()->append (*txtMsg);

   Gtk::Button& send (*Gtk::make_managed<Gtk::Button> (_("_Send"), true));
   send.set_margin (5);
   get_content_area ()->append (send);
   send.signal_clicked ().connect (mem_fun (*this, &ChatDlg::sendMessage));

   set_default_widget (send);
   txtMsg->grab_focus ();

   tagSender = msgs->create_tag ("Sender");
   tagSender->property_foreground () = "blue";

   set_default_size (300, 150);
   show ();
}

//-----------------------------------------------------------------------------
/// Destructor
//-----------------------------------------------------------------------------
ChatDlg::~ChatDlg () {
   TRACE9 ("ChatDlg::~ChatDlg ()");
}

//-----------------------------------------------------------------------------
/// Callback after clicking the send-button
//-----------------------------------------------------------------------------
void ChatDlg::sendMessage () {
   Check1 (txtMsg);
   Check2 (txtMsg->get_text_length ());
   TRACE1 ("ChatDlg::sendMessage () - " << txtMsg->get_text ());

   signalSend.emit (txtMsg->get_text ());
   txtMsg->set_text ("");
}

//-----------------------------------------------------------------------------
/// Adds a message to the message-list
/// \param sender Sender of message
/// \param msg Sent message
//-----------------------------------------------------------------------------
void ChatDlg::addMessage (const Glib::ustring& sender, const Glib::ustring& msg) {
   TRACE3 ("ChatDlg::addMessage (2x const Glib::ustring&) - " << sender << ": " << msg);
   Glib::RefPtr<Gtk::TextMark> pos (msgs->create_mark (msgs->end ()));
   msgs->insert (msgs->end (), sender);
   msgs->apply_tag (tagSender, pos->get_iter (), msgs->end ());

   msgs->insert (msgs->end (), _(": "));
   msgs->insert (msgs->end (), msg);
   msgs->insert (msgs->end (), "\n");

   Gtk::TextIter i (msgs->end ());
   tvMsgs->scroll_to (i, 0.0);
   txtMsg->grab_focus ();
}
