#ifndef CARDWINDOW_H
#define CARDWINDOW_H

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

#include <memory>
#include <vector>

#include <card/Pile.h>

#include <YGP/Check.h>

#include <XGP/AnimWindow.h>

namespace Gtk {
class Box;
class Fixed;
} // namespace Gtk
namespace Card {
class Widget;
}

namespace Card {

/**Copies of the cards, which are shown (and moved) in the animation layer
 * instead of the real cards; those are made invisible meanwhile.
 *
 * \remarks The widget is not managed; so it is not destroyed with the
 *     animation layer (which would end the animation with piles maybe already
 *     destroyed)
 */
class StandIn {
  public:
    explicit StandIn(Gtk::Fixed& layer);
    ~StandIn();

    void show(IPile& src, unsigned int first, unsigned int last);
    void restore();

    Gtk::Fixed& layer;
    std::unique_ptr<Gtk::Box> box; ///< Widget holding the copies of the cards

  private:
    StandIn(const StandIn&) = delete;
    StandIn& operator=(const StandIn&) = delete;

    std::vector<Widget*> hidden; ///< Cards made invisible while animating
};

/**Baseclass for animated windows in the cardgame collection
 * \remarks Derives first from StandIn, so its widget exists when passed to
 *     XGP::AnimatedWindow (and still exists, when that is destroyed)
 */
class AnimatedCard : protected StandIn, public XGP::AnimatedWindow {
  public:
    ~AnimatedCard() override;

    /// Signal emitted, when the animation is finished
    sigc::signal<void()> sigAnimation;

    void getEndPos(double& x, double& y) override;
    void start() override;
    void cleanup() override;
    void finish() override;

  protected:
    AnimatedCard(Gtk::Fixed& layer, IPile& dest, unsigned int posDest);

    IPile& dest;
    unsigned int posDest;

  private:
    AnimatedCard() = delete;
    AnimatedCard(const AnimatedCard&) = delete;
    AnimatedCard& operator=(const AnimatedCard&) = delete;
};

/**Window holding exactly one card. This window can be used to animate a
 * card or fully show it if its put in a pile.
 */
class Window : public AnimatedCard {
  public:
    ~Window() override;

    static Window* create(Gtk::Fixed& layer, IPile& dest, unsigned int posDest, IPile& src, unsigned int posSrc);

    void start() override;
    void cleanup() override;

  protected:
    Window(Gtk::Fixed& layer, IPile& dest, unsigned int posDest, IPile& src, unsigned int posSrc);

    IPile& src;
    unsigned int posSrc;

  private:
    Window() = delete;
    Window(const Window&) = delete;
    Window& operator=(const Window&) = delete;
};

/**Window holding a pile of cards, which can be used for animation.
 */
class PileWindow : public Window {
  public:
    ~PileWindow() override;

    /// Creates a PileWindow object
    /// \param layer Layer to show the animated cards in
    /// \param dest Destination pile
    /// \param posDest Where to put the card in the destination
    /// \param src Source pile; should be a Pile<T>
    /// \param start First card of source to move
    /// \param end Last card of source to move
    /// \returns PileWindow* Created window to animate
    /// \pre The first card must be shown somewhere (to get its position)
    static PileWindow* create(Gtk::Fixed& layer, IPile& dest, unsigned int posDest, IPile& src, unsigned int start,
                              unsigned int end) {
        Check1(src.getWidget());
        Check1(dynamic_cast<Gtk::Box*>(src.getWidget()));
        return new PileWindow(layer, dest, posDest, src, start, end);
    }

    void start() override;
    void cleanup() override;

  protected:
    PileWindow(Gtk::Fixed& layer, IPile& dest, unsigned int posDest, IPile& src, unsigned int start, unsigned int end);

    unsigned int last;

  private:
    PileWindow() = delete;
    PileWindow(const PileWindow&) = delete;
    PileWindow& operator=(const PileWindow&) = delete;
};

/**Window holding a pile of cards, which can be used for animation.
 */
class PileWindows : public PileWindow {
  public:
    ~PileWindows() override;

    static PileWindows* create(Gtk::Fixed& layer, IPile& dest, unsigned int posDest, IPile& src, unsigned int start,
                               unsigned int end);

    void start() override;
    void getEndPos(double& x, double& y) override;
    void cleanup() override;

    void addWindow(IPile& src, unsigned int start, unsigned int end);
    void addWindow(unsigned int posDest, IPile& src, unsigned int start, unsigned int end);

  protected:
    PileWindows(Gtk::Fixed& layer, IPile& dest, unsigned int posDest, IPile& src, unsigned int start, unsigned int end);

  private:
    PileWindows(const PileWindows&) = delete;
    PileWindows& operator=(const PileWindows&) = delete;

    /// Further cards to animate (moved in step with the cards of the PileWindow)
    struct AnimatedPile {
        AnimatedPile(Gtk::Fixed& layer, IPile& src, unsigned int start, unsigned int end);

        StandIn standIn;
        IPile& source;
        unsigned int first, last;
        unsigned int posDest; ///< Target position in destination
    };

    std::vector<std::unique_ptr<AnimatedPile>> wins;
    unsigned int steps{0}; ///< Remaining steps of the animation
};

} // namespace Card

#endif
