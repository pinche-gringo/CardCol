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

namespace Card {
class Widget;
}

namespace Card {

/**Baseclass for animated windows in the cardgame collection
 */
class AnimatedCard : public XGP::AnimatedWindow {
  public:
    ~AnimatedCard() override;

    /// Signal emitted, when the animation is finished
    sigc::signal<void()> sigAnimation;

    void getEndPos(int& x, int& y) override;
    void start() override;
    void cleanup() override;
    void finish() override;

  protected:
    AnimatedCard(IPile& dest, unsigned int posDest, Gtk::Widget& src);

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

    static Window* create(IPile& dest, unsigned int posDest, IPile& src, unsigned int posSrc);

    void start() override;
    void cleanup() override;

  protected:
    Window(IPile& dest, unsigned int posDest, IPile& src, unsigned int posSrc);

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
    /// \param dest Destination pile
    /// \param posDest Where to put the card in the destination
    /// \param src Source pile; should be a Pile<T>
    /// \param start First card of source to move
    /// \param end Last card of source to move
    /// \returns PileWindow* Created window to animate
    /// \pre The first card must be shown somewhere (to get its position)
    static PileWindow* create(IPile& dest, unsigned int posDest, IPile& src, unsigned int start, unsigned int end) {
        Check1(src.getWidget());
        Check1(dynamic_cast<Gtk::Box*>(src.getWidget()));
        return new PileWindow(dest, posDest, src, start, end);
    }

    void start() override;
    void cleanup() override;
    void getEndPos(int& x, int& y) override;

  protected:
    PileWindow(IPile& dest, unsigned int posDest, IPile& src, unsigned int start, unsigned int end);

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

    static PileWindows* create(IPile& dest, unsigned int posDest, IPile& src, unsigned int start, unsigned int end);

    void start() override;
    void getEndPos(int& x, int& y) override;
    void cleanup() override;

    void addWindow(IPile& src, unsigned int start, unsigned int end);
    void addWindow(unsigned int posDest, IPile& src, unsigned int start, unsigned int end);

  protected:
    PileWindows(IPile& dest, unsigned int posDest, IPile& src, unsigned int start, unsigned int end);

  private:
    PileWindows(const PileWindows&) = delete;
    PileWindows& operator=(const PileWindows&) = delete;

    struct AnimatedPile : public XGP::AnimatedWindow {
        AnimatedPile(IPile& src, unsigned int start, unsigned int end);
        ~AnimatedPile() override = default;

        IPile& source;
        unsigned int first, last;
        unsigned int posDest; ///< Target position in destination

        void start() override;
        void getEndPos(int& x, int& y) override;
        void animateTo(int x, int y);
    };

    std::vector<std::unique_ptr<AnimatedPile>> wins;
};

} // namespace Card

#endif
