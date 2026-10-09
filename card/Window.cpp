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

#include <utility>

#include <YGP/Trace.h>

#include <gtkmm/box.h>
#include <gtkmm/fixed.h>

#include "Pile.h"
#include "Widget.h"

#include "Window.h"

namespace Card {

namespace {
/// Number of steps of an animation (as done by XGP::AnimatedWindow)
constexpr unsigned int ANIMATION_STEPS(10);

/// Returns the position of the passed widget relative to the passed layer
/// \param widget Widget to inspect
/// \param layer Layer to which the position is relative to
/// \param x X-coordinate; unchanged if the position can't be determined
/// \param y Y-coordinate; unchanged if the position can't be determined
/// \returns bool True, if the position could be determined
bool getPosition(const Gtk::Widget& widget, const Gtk::Fixed& layer, double& x, double& y) {
    if (auto pos(widget.compute_point(layer, Gdk::Graphene::Point(0.0F, 0.0F))); pos) {
        x = pos->get_x();
        y = pos->get_y();
        return true;
    }
    return false;
}
} // namespace

//-----------------------------------------------------------------------------
/// Constructor; adds the (yet invisible) widget to the passed layer
/// \param layer Layer to show the animated cards in
//-----------------------------------------------------------------------------
StandIn::StandIn(Gtk::Fixed& layer) : layer(layer), box(std::make_unique<Gtk::Box>()) {
    box->set_visible(false);
    layer.put(*box, 0, 0);
}

//-----------------------------------------------------------------------------
/// Destructor
//-----------------------------------------------------------------------------
StandIn::~StandIn() {
    restore();
    // Remark: If the layer has been destroyed, the box has no parent anymore
    if (box->get_parent())
        layer.remove(*box);
}

//-----------------------------------------------------------------------------
/// Shows copies of the passed cards at their position (in the layer) and
/// makes the real cards invisible. Nothing is shown, if the cards are not
/// visible.
/// \param src Pile holding the cards
/// \param first First card to show
/// \param last Last card to show
//-----------------------------------------------------------------------------
void StandIn::show(IPile& src, unsigned int first, unsigned int last) {
    TRACE8("StandIn::show(IPile&, 2x unsigned int) - " << first << '/' << last);
    Check1(first <= last);
    Check1(last < src.size());
    Check1(hidden.empty());

    double x, y;
    if (!src[first]->get_mapped() || !getPosition(*src[first], layer, x, y))
        return;

    // Arrange the copies like the cards in the pile; only the last is shown completely
    bool vertical((src.getWidget() != nullptr) && (src.getWidget()->get_orientation() == Gtk::Orientation::VERTICAL));
    box->set_orientation(vertical ? Gtk::Orientation::VERTICAL : Gtk::Orientation::HORIZONTAL);
    for (unsigned int i(first); i <= last; ++i) {
        Widget& card(*src[i]);
        if (!card.get_visible())
            continue;

        auto* copy(Gtk::make_managed<Widget>(card));
        int size(vertical ? card.get_height() : card.get_width());
        if ((i < last) && (size > 0))
            vertical ? copy->set_size_request(-1, size) : copy->set_size_request(size, -1);
        box->append(*copy);

        card.set_opacity(0);
        hidden.push_back(&card);
    }

    layer.move(*box, x, y);
    box->set_visible(true);
}

//-----------------------------------------------------------------------------
/// Hides the copies of the cards and shows the real cards again
//-----------------------------------------------------------------------------
void StandIn::restore() {
    for (auto* card : hidden)
        card->set_opacity(1);
    hidden.clear();

    box->set_visible(false);
    while (Gtk::Widget* child = box->get_first_child())
        box->remove(*child);
}

//-----------------------------------------------------------------------------
/// Constructor
/// \param layer Layer to show the animated cards in
/// \param dest Destination pile
/// \param posDest Where to put the card in the destination
//-----------------------------------------------------------------------------
AnimatedCard::AnimatedCard(Gtk::Fixed& layer, IPile& dest, unsigned int posDest)
    : StandIn(layer), XGP::AnimatedWindow(layer, *box), sigAnimation(), dest(dest), posDest(posDest) {
    TRACE9("AnimatedCard::AnimatedCard(Gtk::Fixed&, IPile&, unsigned int) - " << posDest);
}

//-----------------------------------------------------------------------------
/// Destructor
//-----------------------------------------------------------------------------
AnimatedCard::~AnimatedCard() = default;

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
/// Cleanup of the animation; shows the real cards again
//-----------------------------------------------------------------------------
void AnimatedCard::cleanup() {
    TRACE5("AnimatedCard::cleanup() - " << static_cast<int>(posDest) << '/' << dest.size());
    restore();

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
/// \param x X-coordinate of destination (relative to the animation layer)
/// \param y Y-coordinate of destination (relative to the animation layer)
/// \remarks If the destination can't be determined, the cards stay where they are
//-----------------------------------------------------------------------------
void AnimatedCard::getEndPos(double& x, double& y) {
    Check2(dest.size());
    Widget* target((posDest == -1U) ? dest[0] : dest[(posDest >= dest.size()) ? posDest - 1 : posDest]);
    Check2(target);

    fixed.get_child_position(widget, x, y);
    getPosition(*target, fixed, x, y);
    TRACE9("AnimatedCard::getEndPos(2x double&) - " << static_cast<int>(posDest) << " Dest: " << x << '/' << y);
}

//-----------------------------------------------------------------------------
/// Constructor
/// \param layer Layer to show the animated cards in
/// \param dest Destination pile
/// \param posDest Where to put the card in the destination
/// \param src Source pile
/// \param posSrc Position of the card to animate
//-----------------------------------------------------------------------------
Window::Window(Gtk::Fixed& layer, IPile& dest, unsigned int posDest, IPile& src, unsigned int posSrc)
    : AnimatedCard(layer, dest, posDest), src(src), posSrc(posSrc) {
    TRACE9("Window::Window(Gtk::Fixed&, 2x(IPile&, unsigned int)) - " << posSrc << " -> " << posDest);
    Check1(posSrc < src.size());
}

//-----------------------------------------------------------------------------
/// Destructor
//-----------------------------------------------------------------------------
Window::~Window() = default;

//-----------------------------------------------------------------------------
/// Creates an Window-object
/// \param layer Layer to show the animated card in
/// \param dest Destination pile
/// \param posDest Where to put the card in the destination
/// \param src Source pile
/// \param posSrc Position of the card to animate
/// \returns Window* Created window to animate
/// \pre The card must be shown (to get its position)
//-----------------------------------------------------------------------------
Window* Window::create(Gtk::Fixed& layer, IPile& dest, unsigned int posDest, IPile& src, unsigned int posSrc) {
    Check1(posSrc < src.size());
    return new Window(layer, dest, posDest, src, posSrc);
}

//-----------------------------------------------------------------------------
/// Additional actions when starting the animation
//-----------------------------------------------------------------------------
void Window::start() {
    TRACE8("Window::start()");
    AnimatedCard::start();
    src.resize(posSrc, IPile::NORMAL);
    show(src, posSrc, posSrc);
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
/// \param layer Layer to show the animated cards in
/// \param dest Destination pile
/// \param posDest Where to put the card in the destination
/// \param src Source pile
/// \param start First card to animate from source
/// \param end Last card to animate from source
//-----------------------------------------------------------------------------
PileWindow::PileWindow(Gtk::Fixed& layer, IPile& dest, unsigned int posDest, IPile& src, unsigned int start,
                       unsigned int end)
    : Window(layer, dest, posDest, src, start), last(end) {
    TRACE8("PileWindow::PileWindow(...) - " << start << '/' << end);
    Check3(src.size());
    Check3(start <= end);
    Check3(end < src.size());
    Check1(posDest <= dest.size());
}

//-----------------------------------------------------------------------------
/// Destructor
//-----------------------------------------------------------------------------
PileWindow::~PileWindow() = default;

//-----------------------------------------------------------------------------
/// Additional actions when starting the animation
//-----------------------------------------------------------------------------
void PileWindow::start() {
    TRACE8("PileWindow::start()");
    // Remark: Window::start() is skipped, as it maximises the first card; like
    // in a pile only the last card is shown completely
    AnimatedCard::start();
    src.resize(last, IPile::NORMAL);
    show(src, posSrc, last);
}

//-----------------------------------------------------------------------------
/// Cleanup of the animation; moves the animated cards to the distination pile
//-----------------------------------------------------------------------------
void PileWindow::cleanup() {
    TRACE5("PileWindow::cleanup() - " << posSrc << '/' << last << " -> " << static_cast<int>(posDest));
    Window::cleanup();

    while (last-- > posSrc) {
        TRACE5("PileWindow::cleanup() - Move " << posSrc << '/' << last << " -> " << static_cast<int>(posDest));
        dest.insert(src.remove(posSrc), ++posDest);
    }
    TRACE8("PileWindow::cleanup() - Finished");
}

//-----------------------------------------------------------------------------
/// Constructor
/// \param layer Layer to show the animated cards in
/// \param dest Destination pile
/// \param posDest Where to put the card in the destination
/// \param src Source pile
/// \param start First card to animate from source
/// \param end Last card to animate from source
//-----------------------------------------------------------------------------
PileWindows::PileWindows(Gtk::Fixed& layer, IPile& dest, unsigned int posDest, IPile& src, unsigned int start,
                         unsigned int end)
    : PileWindow(layer, dest, posDest, src, start, end), wins() {
    TRACE8("PileWindows::PileWindows(...) - " << start << '/' << end);
}

//-----------------------------------------------------------------------------
/// Destructor
//-----------------------------------------------------------------------------
PileWindows::~PileWindows() = default;

//-----------------------------------------------------------------------------
/// Creates a PileWindow object
/// \param layer Layer to show the animated cards in
/// \param dest Destination pile
/// \param posDest Where to put the card in the destination
/// \param src Source pile
/// \param start First card of source to animate
/// \param end Last card of source to animate
/// \returns PileWindow* Created window to animate
/// \pre The first card must be shown somewhere (to get its position)
//-----------------------------------------------------------------------------
PileWindows* PileWindows::create(Gtk::Fixed& layer, IPile& dest, unsigned int posDest, IPile& src, unsigned int start,
                                 unsigned int end) {
    Check3(src.size());
    Check3(start <= end);
    Check3(end < src.size());
    Check1(posDest <= dest.size());
    return new PileWindows(layer, dest, posDest, src, start, end);
}

//-----------------------------------------------------------------------------
/// Additional actions when starting the animation
//-----------------------------------------------------------------------------
void PileWindows::start() {
    TRACE8("PileWindows::start()");
    PileWindow::start();

    steps = ANIMATION_STEPS;
    for (auto& win : wins) {
        TRACE9("PileWindows::start() - Subwin: " << (&win - wins.data()));
        Check3(win);
        win->standIn.show(win->source, win->first, win->last);
    }
}

//-----------------------------------------------------------------------------
/// Returns the position where to animate the card to; moves the further cards
/// one step closer to it (like XGP::AnimatedWindow does with the main cards)
/// \param x X-coordinate of destination
/// \param y Y-coordinate of destination
//-----------------------------------------------------------------------------
void PileWindows::getEndPos(double& x, double& y) {
    PileWindow::getEndPos(x, y);

    if (steps)
        --steps;
    for (auto& win : wins) {
        TRACE9("PileWindows::getEndPos(2x double&) - Subwin: " << (&win - wins.data()));
        Check3(win);
        Gtk::Box& box(*win->standIn.box);
        if (box.get_visible()) {
            double x2, y2;
            fixed.get_child_position(box, x2, y2);
            fixed.move(box, x2 + (x - x2) / (steps + 1), y2 + (y - y2) / (steps + 1));
        }
    }
}

//-----------------------------------------------------------------------------
/// Cleanup of the animation; moves the animated cards to the destination pile
//-----------------------------------------------------------------------------
void PileWindows::cleanup() {
    TRACE5("PileWindows::cleanup() - Subwindows: " << wins.size());
    PileWindow::cleanup();

    // Move the animated cards to their target; remove the animated widget
    for (const auto& win : wins) {
        win->standIn.restore();

        unsigned int first(win->first);
        unsigned int last(win->last);
        Check3(win->posDest <= dest.size());
        TRACE8("PileWindows::cleanup() - Moving [" << first << '/' << last << "] of " << win->source.size() << " to "
                                                   << win->posDest << " of " << dest.size());
        do
            dest.insert(win->source.remove(first), (win->posDest)++);
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

    auto win(std::make_unique<AnimatedPile>(fixed, src, start, end));
    win->posDest = posDest;
    wins.push_back(std::move(win));
}

//-----------------------------------------------------------------------------
/// Constructor
/// \param layer Layer to show the animated cards in
/// \param src Source pile
/// \param start First card of source to animate
/// \param end Last card of source to animate
//-----------------------------------------------------------------------------
PileWindows::AnimatedPile::AnimatedPile(Gtk::Fixed& layer, IPile& src, unsigned int start, unsigned int end)
    : standIn(layer), source(src), first(start), last(end), posDest(0) {
    TRACE9("PileWindows::AnimatedPile::AnimatedPile(Gtk::Fixed&, IPile&, 2x unsigned int)");
}

} // namespace Card
