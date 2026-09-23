#ifndef CHATDLG_H
#define CHATDLG_H

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

#include <glibmm/refptr.h>

#include <XGP/XDialog.h>

namespace Gtk {
class Entry;
class TextTag;
class TextView;
class TextBuffer;
} // namespace Gtk

/**Dialog to enter a message to send to others and to display messages received
 * from others.
 *
 * The dialog emits a signal when a message is to send.
 */
class ChatDlg : public XGP::XDialog {
  public:
    ~ChatDlg() override;

    /// Creates the dialog
    /// \param parent Parent window
    static ChatDlg* create(Gtk::Window& parent) {
        ChatDlg* dlg(new ChatDlg);
        dlg->set_transient_for(parent);
        dlg->signal_response().connect(mem_fun(*dlg, &ChatDlg::free));
        return dlg;
    }

    /// Signal emitted, when OK is selected
    sigc::signal<void(const Glib::ustring&)> signalSend;

    void addMessage(const Glib::ustring& sender, const Glib::ustring& msg);

  protected:
    ChatDlg();
    void sendMessage();

  private:
    // Prohibited manager functions
    ChatDlg(const ChatDlg& other) = delete;
    const ChatDlg& operator=(const ChatDlg& other) = delete;

    Gtk::Entry* const txtMsg;
    Gtk::TextView* const tvMsgs;

    Glib::RefPtr<Gtk::TextTag> tagSender;
    Glib::RefPtr<Gtk::TextBuffer> msgs;
};

#endif
