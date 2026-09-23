// PROJECT     : Cardgames
// SUBSYSTEM   : libCard
// REFERENCES  :
// TODO        :
// BUGS        :
// AUTHOR      : Markus Schwab
// CREATED     : 20.05.2007
// COPYRIGHT   : Copyright (C) 2007 - 2009, 2026

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

#include <YGP/Trace.h>

#include <gtkmm/native.h>

#include "Pile.h"
#include "Widget.h"

#include "Window.h"

namespace Card {

namespace {
/// Returns the surface a widget is (indirectly) shown on
/// \remarks Replaces the GTK3 Gtk::Widget::get_window(); under GTK4 a
///     widget no longer owns its own Gdk::Window, only its toplevel does
Glib::RefPtr<Gdk::Surface> surfaceOf(Gtk::Widget& w) {
    Gtk::Native* native(w.get_native());
    return native ? native->get_surface() : Glib::RefPtr<Gdk::Surface>();
}
} // namespace

//-----------------------------------------------------------------------------
/// Constructor
/// \param dest Destination pile
/// \param posDest Where to put the card in the destination
/// \param src Source card
//-----------------------------------------------------------------------------
AnimatedCard::AnimatedCard(IPile& dest, unsigned int posDest, Gtk::Widget& src)
    : XGP::AnimatedWindow(surfaceOf(src)), sigAnimation(), dest(dest), posDest(posDest) {
    TRACE9("AnimatedCard::AnimatedCard(IPile&, unsigned int, Gtk::Widget&) - " << posDest);
}

//-----------------------------------------------------------------------------
/// Destructor
//-----------------------------------------------------------------------------
AnimatedCard::~AnimatedCard() {}

//-----------------------------------------------------------------------------
/// Additional actions when starting the animation
//-----------------------------------------------------------------------------
void AnimatedCard::start() {
    TRACE5("AnimatedCard::start() - " << posDest << '/' << dest.size());
    if (dest.empty()) {
        TRACE8("AnimatedCard::start() - Adding empty card");
        Widget* card(Widget::getEmpty());
        card->show();
        dest.setTopCard(*card);
        posDest = -1U;
    }
}

//-----------------------------------------------------------------------------
/// Cleanup of the animation; moves the animated card to the distination pile
//-----------------------------------------------------------------------------
void AnimatedCard::cleanup() {
    TRACE5("AnimatedCard::cleanup() - " << (int)posDest << '/' << dest.size());
    if (posDest == -1U) {
        TRACE8("AnimatedCard::cleanup() - Removing empty card");
        Check1(dest.size() == 1);
        delete &dest.removeTopCard();
        posDest = 0;
    }
}

//-----------------------------------------------------------------------------
/// Callback when the animation starts
//-----------------------------------------------------------------------------
void AnimatedCard::finish() {
    TRACE8("AnimatedCard::finish()");
    sigAnimation.emit();
}

//-----------------------------------------------------------------------------
/// Returns the position where to animate the card to
/// \param x X-coordinate of destination
/// \param y Y-coordinate of destination
//-----------------------------------------------------------------------------
void AnimatedCard::getEndPos(int& x, int& y) {
    Check2(dest.size());
    Widget* widget((posDest == -1U) ? dest[0] : dest[(posDest >= dest.size()) ? posDest - 1 : posDest]);
    Check2(widget);

    // Remark: Under GTK4 there is no way to query a widget's on-screen
    // position anymore (see AnimWindow.h); animateTo() is a no-op, so the
    // actual coordinates returned here are inconsequential.
    x = y = 0;
    TRACE9("AnimatedCard::getEndPos(2x int&) - " << (int)posDest << " Dest: " << x << '/' << y);
}

//-----------------------------------------------------------------------------
/// Constructor
/// \param dest Destination pile
/// \param posDest Where to put the card in the destination
/// \param card Card to show
/// \param card Card to animate
//-----------------------------------------------------------------------------
Window::Window(IPile& dest, unsigned int posDest, IPile& src, unsigned int posSrc)
    : AnimatedCard(dest, posDest, *src[posSrc]), src(src), posSrc(posSrc) {
    TRACE9("Window::Window(2x(IPile&, unsigned int)) - " << posSrc << " -> " << posDest);
    Check1(posSrc < src.size());
}

//-----------------------------------------------------------------------------
/// Destructor
//-----------------------------------------------------------------------------
Window::~Window() {}

//-----------------------------------------------------------------------------
/// Creates an Window-object
/// \param dest Destination pile
/// \param posDest Where to put the card in the destination
/// \param src Source card
/// \returns Window* Created window to animate
/// \pre The card must be shown (to get its position)
//-----------------------------------------------------------------------------
Window* Window::create(IPile& dest, unsigned int posDest, IPile& src, unsigned int posSrc) {
    Check1(posSrc < src.size());
    return new Window(dest, posDest, src, posSrc);
}

//-----------------------------------------------------------------------------
/// Additional actions when starting the animation
//-----------------------------------------------------------------------------
void Window::start() {
    TRACE8("Window::start()");
    AnimatedCard::start();
    // Remark: win->raise() is not possible anymore under GTK4 (see AnimWindow.h)
    src.resize(posSrc, IPile::NORMAL);
}

//-----------------------------------------------------------------------------
/// Cleanup of the animation; moves the animated card to the distination pile
//-----------------------------------------------------------------------------
void Window::cleanup() {
    TRACE5("Window::cleanup() - " << posSrc << " -> " << posDest);
    AnimatedCard::cleanup();
    dest.insert(src.remove(posSrc), posDest);
    TRACE8("Window::cleanup() - finish");
}

//-----------------------------------------------------------------------------
/// Constructor
/// \param win Window
/// \param dest Destination pile
/// \param posDest Where to put the card in the destination
/// \param src Source pile
/// \param start First card to animate from source
/// \param end Last card to animate from source
//-----------------------------------------------------------------------------
PileWindow::PileWindow(IPile& dest, unsigned int posDest, IPile& src, unsigned int start, unsigned int end)
    : Window(dest, posDest, src, start), last(end) {
    TRACE8("PileWindow::PileWindow(...) - " << start << '/' << end);
    Check3(src.size());
    Check3(start <= end);
    Check3(end < src.size());
    Check1(posDest <= dest.size());
}

//-----------------------------------------------------------------------------
/// Destructor
//-----------------------------------------------------------------------------
PileWindow::~PileWindow() {}

//-----------------------------------------------------------------------------
/// Moves to passed window to the passed coordinates
/// \param animWindow Window to animate
/// \param x X-coordinate
/// \param y Y-coordinate
/// \returns bool Always false
//-----------------------------------------------------------------------------
void PileWindow::getEndPos(int& x, int& y) {
    TRACE8("PileWindow::getEndPos(2x int&)");
    Check2(src.size() > last);
    Window::getEndPos(x, y);

    // Also move the remaining cards
    Glib::RefPtr<Gdk::Surface> oldWin(win);
    for (unsigned int i(posSrc + 1); i <= last; ++i) {
        win = surfaceOf(*src[i]);
        Check3(win);
        animateTo(x, y);
    }
    win = oldWin;
}

//-----------------------------------------------------------------------------
/// Additional actions when starting the animation
//-----------------------------------------------------------------------------
void PileWindow::start() {
    TRACE8("PileWindow::start()");
    Window::start();
    // Remark: raising the remaining cards is not possible anymore under GTK4 (see
    // AnimWindow.h)
}

//-----------------------------------------------------------------------------
/// Cleanup of the animation; moves the animated cards to the distination pile
//-----------------------------------------------------------------------------
void PileWindow::cleanup() {
    TRACE5("PileWindow::cleanup() - " << posSrc << '/' << last << " -> " << (int)posDest);
    Window::cleanup();

    while (last-- > posSrc) {
        TRACE5("PileWindow::cleanup() - Move " << posSrc << '/' << last << " -> " << (int)posDest);
        dest.insert(src.remove(posSrc), ++posDest);
    }
    TRACE8("PileWindow::cleanup() - Finished");
}

//-----------------------------------------------------------------------------
/// Constructor
/// \param dest Destination pile
/// \param posDest Where to put the card in the destination
//-----------------------------------------------------------------------------
PileWindows::PileWindows(IPile& dest, unsigned int posDest, IPile& src, unsigned int start, unsigned int end)
    : PileWindow(dest, posDest, src, start, end), wins() {
    TRACE8("PileWindows::PileWindows(...) - " << start << '/' << end);
}

//-----------------------------------------------------------------------------
/// Destructor
//-----------------------------------------------------------------------------
PileWindows::~PileWindows() {
    for (std::vector<AnimatedPile*>::iterator i(wins.begin()); i != wins.end(); ++i)
        delete *i;
}

//-----------------------------------------------------------------------------
/// Creates a PileWindow object
/// \param dest Destination pile
/// \param posDest Where to put the card in the destination
/// \param src Source pile
/// \param start First card of source to animate
/// \param end Last card of source to animate
/// \returns PileWindow* Created window to animate
/// \pre The first card must be shown somewhere (to get its position)
//-----------------------------------------------------------------------------
PileWindows* PileWindows::create(IPile& dest, unsigned int posDest, IPile& src, unsigned int start, unsigned int end) {
    Check3(src.size());
    Check3(start <= end);
    Check3(end < src.size());
    Check1(posDest <= dest.size());
    return new PileWindows(dest, posDest, src, start, end);
}

//-----------------------------------------------------------------------------
/// Additional actions when starting the animation
//-----------------------------------------------------------------------------
void PileWindows::start() {
    TRACE8("PileWindow::start()");
    PileWindow::start();

    for (std::vector<AnimatedPile*>::iterator i(wins.begin()); i != wins.end(); ++i) {
        TRACE9("PileWindows::start() - Subwin: " << (i - wins.begin()));
        Check3(*i);
        (*i)->start();
    }
}

//-----------------------------------------------------------------------------
/// Returns the position where to animate the card to
/// \param x X-coordinate of destination
/// \param y Y-coordinate of destination
//-----------------------------------------------------------------------------
void PileWindows::getEndPos(int& x, int& y) {
    PileWindow::getEndPos(x, y);

    for (std::vector<AnimatedPile*>::iterator i(wins.begin()); i != wins.end(); ++i) {
        TRACE9("PileWindows::getEndPos(2x int&) - Subwin: " << (i - wins.begin()));
        Check3(*i);
        (*i)->animateTo(x, y);
    }
}

//-----------------------------------------------------------------------------
/// Cleanup of the animation; moves the animated card to the destination pile
//-----------------------------------------------------------------------------
void PileWindows::cleanup() {
    TRACE5("PileWindows::cleanup() - Subwindows: " << wins.size());
    PileWindow::cleanup();

    // Move the animated cards to their target; remove the animated widget
    for (std::vector<AnimatedPile*>::const_iterator i(wins.begin()); i != wins.end(); ++i) {
        unsigned int first((*i)->first);
        unsigned int last((*i)->last);
        Check3((*i)->posDest <= dest.size());
        TRACE8("PileWindows::cleanup() - Moving [" << first << '/' << last << "] of " << (*i)->source.size() << " to "
	       << (*i)->posDest << " of " << dest.size());
        do
            dest.insert((*i)->source.remove(first), ((*i)->posDest)++);
        while (first < last--);
    }
    TRACE8("PileWindows::cleanup() - Finished");
}

//-----------------------------------------------------------------------------
/// Adds a window to animate.
/// \param dest Position in the destination
/// \param src Source pile
/// \param start First card of source to animate
/// \param end Last card of source to animate
//-----------------------------------------------------------------------------
void PileWindows::addWindow(unsigned int dest, IPile& src, unsigned int start, unsigned int end) {
    TRACE3("PileWindows::addWindow(unsigned int, IPile& src, 2x unsigned int) - " << start << '-' << end << " -> " << dest);
    Check1(start <= end);
    Check1(end < src.size());
    addWindow(src, start, end);
    wins.back()->posDest = dest;
}

//-----------------------------------------------------------------------------
/// Adds a window to animate.
/// \param src Source pile
/// \param start First card of source to animate
/// \param end Last card of source to animate
//-----------------------------------------------------------------------------
void PileWindows::addWindow(IPile& src, unsigned int start, unsigned int end) {
    TRACE3("PileWindows::addWindow(IPile& src, 2x unsigned int) - " << start << '-' << end);
    Check1(start <= end);
    Check1(end < src.size());

    AnimatedPile* win(new AnimatedPile(src, start, end));
    Check3(win);
    win->posDest = posDest;
    wins.push_back(win);
}

//-----------------------------------------------------------------------------
/// Default-ctr
/// \param src Source pile
/// \param start First card of source to animate
/// \param end Last card of source to animate
//-----------------------------------------------------------------------------
PileWindows::AnimatedPile::AnimatedPile(IPile& src, unsigned int start, unsigned int end)
    : XGP::AnimatedWindow(surfaceOf(*src[start])), source(src), first(start), last(end), posDest(0) {
    TRACE9("PileWindows::AnimatedPile::AnimatedPile(IPile&, 2x unsigned int)");
}

//-----------------------------------------------------------------------------
/// Returns the position where to animate the card to
/// \param x X-coordinate of destination
/// \param y Y-coordinate of destination
//-----------------------------------------------------------------------------
void PileWindows::AnimatedPile::getEndPos(int& x, int& y) {
    TRACE1("PileWindows::AnimatedPile::getEndPos(2x int& x)");
    Check(0);
    x = y = 0;
}

//-----------------------------------------------------------------------------
/// Additional actions when starting the animation
//-----------------------------------------------------------------------------
void PileWindows::AnimatedPile::start() {
    TRACE1("PileWindows::AnimatedPile::start()");
    // Remark: raising the cards is not possible anymore under GTK4 (see
    // AnimWindow.h)
}

//-----------------------------------------------------------------------------
/// Animates all specified cards in the source-pile to the passed coordinates
/// \param x X-coordinate of destination
/// \param y Y-coordinate of destination
//-----------------------------------------------------------------------------
void PileWindows::AnimatedPile::animateTo(int x, int y) {
    TRACE5("PileWindows::AnimatedPile::animateTo(2x int) - " << first << '/' << last << " to " << x << '/' << y);
    XGP::AnimatedWindow::animateTo(x, y);

    // Also move the remaining cards
    Glib::RefPtr<Gdk::Surface> oldWin(win);
    for (unsigned int i(first + 1); i <= last; ++i) {
        win = surfaceOf(*source[i]);
        Check3(win);
        TRACE9("PileWindows::AnimatedPile::animateTo(2x int) - Mapped " << win->get_mapped());
        if (win->get_mapped())
            XGP::AnimatedWindow::animateTo(x, y);
    }
    win = oldWin;
}

} // namespace Card
