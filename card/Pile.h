#ifndef CARDPILE_H
#define CARDPILE_H

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

#include <algorithm>
#include <map>
#include <string>
#include <vector>

#include <gtk/gtk.h>
#include <gtkmm/box.h>
#include <gtkmm/button.h>
#include <gtkmm/image.h>

#define SET_TIP(widget, text) (widget).set_tooltip_text(text)
#define UNSET_TIP(widget) (widget).set_has_tooltip(false)

#include <YGP/ANumeric.h>
#include <YGP/Check.h>
#include <YGP/Trace.h>

#include <card/Widget.h>

namespace Card {

/// Gtk::Box with a fixed vertical orientation, replacing the removed Gtk::VBox
class VBox : public Gtk::Box {
  public:
    VBox() : Gtk::Box(Gtk::Orientation::VERTICAL) {}
};

/// Gtk::Box with a fixed horizontal orientation, replacing the removed
/// Gtk::HBox
class HBox : public Gtk::Box {
  public:
    HBox() : Gtk::Box(Gtk::Orientation::HORIZONTAL) {}
};

/**Class to display a pile of cards on the screen
 */
class IPile {
  public:
    typedef enum { NORMAL = 0, COMPRESSED, QUITE_COMPRESSED, VERY_COMPRESSED, TOTALLY_COMPRESSED, LAST } PileStyle;
    typedef enum { SHOWBACK = 0, SHOWFACE, DONT_CHANGE } ShowOpt;

    typedef bool (*CMPFUNC)(const Widget*, const Widget*);
    typedef int (*CMPFUNC2)(const Widget&, const Widget&);

    typedef std::vector<Widget*>::reference reference;
    typedef std::vector<Widget*>::const_reference const_reference;
    typedef std::vector<Widget*>::iterator iterator;
    typedef std::vector<Widget*>::const_iterator const_iterator;

    IPile(PileStyle style = NORMAL, ShowOpt show = DONT_CHANGE);
    virtual ~IPile();

    /// \name Methods to access pile first-in-last-out
    //@{
    void flipTopCard();
    void showTopCardFace(bool visible = true);
    void showTopCardBack() { showTopCardFace(false); }

    void setTopCards(const std::vector<Widget*>& staple);
    void setTopCards(const std::vector<Widget*>& staple, bool visible);
    void setTopCards(const std::vector<Widget*>& staple, bool visible, bool lastVisible) {
        setTopCards(staple, visible);
        showTopCardFace(lastVisible);
    }

    virtual void setTopCard(Widget& newCard);
    void setTopCard(Widget& newCard, bool visible) {
        newCard.showFace(visible);
        setTopCard(newCard);
    }

    Widget& getTopCard() const {
        Check3(size());
        return *operator[](size() - 1);
    }
    virtual Widget& removeTopCard();
    Widget& removeShownTopCard(bool visible = true) {
        Widget& topCard(removeTopCard());
        topCard.showFace(visible);
        return topCard;
    }
    //@}

    /// \name Methods to random access pile
    //@{
    virtual unsigned int insert(Widget& card, unsigned int pos);
    unsigned int insertSorted(Widget& card, CMPFUNC fnSort = compCardsByNr);
    unsigned int insertColourSorted(Widget& card) { return insertSorted(card, compCards); }
    void append(Widget& card) { setTopCard(card); }
    virtual Widget& remove(Widget& card);
    virtual Widget& remove(unsigned int pos);

    virtual void move(unsigned int dest, unsigned int source);

    Widget* get(unsigned int id) const;
    //@}

    /// \name std::vector-like interface
    //@{
    unsigned int size() const { return cards.size(); }
    bool empty() const { return cards.empty(); }

    reference operator[](size_t __n) { return cards[__n]; }
    const_reference operator[](size_t __n) const { return cards[__n]; }
    iterator begin() { return cards.begin(); }
    const_iterator begin() const { return cards.begin(); }
    iterator end() { return cards.end(); }
    const_iterator end() const { return cards.end(); }
    //@}

    /// \name Find methods
    //@{
    int find(const Widget& card, CMPFUNC fnComp) const {
        int pos(find1EqualOrBigger(card, fnComp));
        return ((pos != -1) && !fnComp(operator[](pos), &card)) ? pos : -1;
    }
    int findByNr(const Widget& card) const { return find(card, compCardsByNr); }
    int findByColour(const Widget& card) const { return find(card, compCards); }
    int findByID(const Widget& card) const { return find(card, compCardsByID); }
    int find1EqualOrBigger(const Widget& card, CMPFUNC fnComp) const {
        std::vector<Widget*>::const_iterator i(std::lower_bound(begin(), end(), &card, fnComp));
        return ((i != end() && !fnComp(*i, &card)) ? (i - begin()) : -1);
    }
    int find1EqualOrBiggerByNr(const Widget& card) const { return find1EqualOrBigger(card, compCardsByNr); }
    int find1EqualOrBiggerByColour(const Widget& card) const { return find1EqualOrBigger(card, compCards); }
    int findFirstEqualOrBigger(Widget::NUMBERS nr) const;
    int findFirstEqualOrBiggerColour(Widget::COLOURS nr) const;
    int findLastEqualOrBigger(Widget::NUMBERS nr) const {
        int pos(findFirstEqualOrBigger(nr));
        return (pos == -1) ? -1 : findLastEqual(pos);
    }
    int findLastEqualOrBiggerColour(Widget::COLOURS col) const;
    int findLastEqual(unsigned int pos) const;
    int findLastEqualColour(unsigned int pos) const;
    int findFirstEqual(unsigned int pos) const;
    int findFirstEqualColour(unsigned int pos) const;
    int find(Widget::NUMBERS nr, unsigned int start = 0) const;
    int find(Widget::COLOURS colour, unsigned int start = 0) const;
    int find(unsigned int id, unsigned int start = 0) const;

    unsigned int findLowestCard() const;
    unsigned int findLowestCard(Widget::COLOURS excludeColour) const;
    //@}

    /// \name Test availability
    //@{
    bool exists(Widget::NUMBERS nr, unsigned int start = 0) const { return find(nr, start) != -1; }
    bool exists(Widget::COLOURS colour, unsigned int start = 0) const { return find(colour, start) != -1; }
    bool exists(const Widget& card) const { return exists(&card); }
    bool exists(const Widget* card) const { return std::find(begin(), end(), card) != end(); }
    //@}

    /// \name General management-functions
    //@{
    virtual void getSize(int& width, int& height);
    virtual void resize(unsigned int pos, PileStyle s) {
        TRACE1("IPile::resize - pos " << pos);
        if (pos != -1U)
            resize(*operator[](pos), s);
    }
    virtual void resize(Widget& card, PileStyle s);
    void clear();
    void setStyle(PileStyle s);
    PileStyle getStyle() const { return style; }
    void setShowOption(ShowOpt show);
    ShowOpt getShowOption() const { return showOpt; }

    bool topCardShowsFace() const { return getTopCard().showsFace(); }

    bool hasFittingPair(const Widget& card, CMPFUNC2 cmp, bool doubles = true) const;
    iterator getFittingCard(const Widget& card, CMPFUNC2 cmp) const { return getFittingCard(card, begin(), cmp); }
    iterator getFittingCard(const Widget& card, const_iterator start, CMPFUNC2 cmp) const;

    unsigned int sortColourSerie(std::map<unsigned int, unsigned int>& aPos, std::vector<unsigned int>& aOrder);
    unsigned int getSeries(Widget& card, std::map<unsigned int, unsigned int>& aPos, std::vector<unsigned int>& aOrder,
                           CMPFUNC2 cmp, bool doubles = true);

    virtual Gtk::Box* getWidget() const { return NULL; }
    //@}

    /// \name Sorting
    //@{
    virtual void sort(CMPFUNC fnSort);
    void sortByNumber() { sort(compCardsByNr); }
    void sortByColour() { sort(compCards); }

    static bool compCards(const Widget* a, const Widget* b);
    static bool compCardsByNr(const Widget* a, const Widget* b);
    static bool compCardsByID(const Widget* a, const Widget* b);
    //@}

    void getCards(IPile& src, unsigned int start = 0, int end = -1) { getCards(size(), src, start, end); }
    void getCards(unsigned int posDest, IPile& src, unsigned int start = 0, int end = -1);

  protected:
    PileStyle style;
    ShowOpt showOpt;
    std::vector<Widget*> cards;

    virtual void insertCardFast(Widget& card, unsigned int offset);
    virtual Widget& removeCardFast(unsigned int offset);

  private:
    static void deleteElement(unsigned int elem, std::map<unsigned int, unsigned int>& aPos, std::vector<unsigned int>& aOrder);
};

/**Specializations of IPile to display. The template must support the
 * following methods:
 *
 *   - append()
 *   - remove()
 *   - insert_child_at_start() / insert_child_after()
 *   - reorder_child_at_start() / reorder_child_after()
 *
 * Designed to be used with the Card::VBox/Card::HBox-classes
 */
template <class T> class Pile : public T, public IPile {
  public:
    Pile(PileStyle style = NORMAL, ShowOpt show = DONT_CHANGE) : IPile(style, show) {}
    virtual ~Pile() {}

    virtual void setTopCard(Widget& newCard) {
        IPile::setTopCard(newCard);
        T::append(newCard);
    }
    void setTopCard(Widget& newCard, bool visible) { IPile::setTopCard(newCard, visible); }

    virtual Widget& removeTopCard() {
        Widget& card(IPile::removeTopCard());
        T::remove(card);
        return card;
    }

    virtual unsigned int insert(Widget& card, unsigned int pos) {
        if (pos)
            T::insert_child_after(card, *IPile::operator[](pos - 1));
        else
            T::insert_child_at_start(card);
        return IPile::insert(card, pos);
    }

    virtual Widget& remove(Widget& card) {
        T::remove(card);
        return IPile::remove(card);
    }
    Widget& remove(Widget& card, bool visible) {
        card.showFace(visible);
        return remove(card);
    }
    virtual Widget& remove(unsigned int pos) {
        T::remove(*operator[](pos));
        return IPile::remove(pos);
    }
    Widget& remove(unsigned int pos, bool visible) {
        Widget& card(remove(pos));
        card.showFace(visible);
        return card;
    }

    virtual void resize(unsigned int pos, PileStyle s) {
        Check(0);
        IPile::resize(pos, s);
    }
    virtual void resize(Widget& card, PileStyle s) { Check(0); }
    virtual void sort(CMPFUNC fnSort) {
        if (size()) {
            resize(size() - 1, style);
            IPile::sort(fnSort);
            resortGUI();
        }
    }
    static unsigned int getCompressedSize(PileStyle s) { return 0; }
    unsigned int getCompressedSize() { return getCompressedSize(style); }
    virtual void getSize(int& width, int& height) { IPile::getSize(width, height); }

    virtual Gtk::Box* getWidget() const { return static_cast<T*>(const_cast<Pile*>(this)); }

  protected:
    virtual void resortGUI() {
        for (unsigned int i(0); i < size(); ++i) {
            if (i)
                T::reorder_child_after(*operator[](i), *operator[](i - 1));
            else
                T::reorder_child_at_start(*operator[](i));
        }
        if (size())
            resize(size() - 1, NORMAL);
    }

    virtual void insertCardFast(Widget& card, unsigned int offset) {
        IPile::insertCardFast(card, offset);
        if (offset)
            T::insert_child_after(card, *IPile::operator[](offset - 1));
        else
            T::insert_child_at_start(card);
    }

    virtual Widget& removeCardFast(unsigned int offset) {
        T::remove(**(begin() + offset));
        return IPile::removeCardFast(offset);
    }

  private:
    Pile(const Pile<T>& other);
    Pile<T>& operator=(const Pile<T>& other);
};

typedef Pile<Card::VBox> VPile;
typedef Pile<Card::HBox> HPile;

/// Implementation of the getCompressedSize() method for Card::VBox
template <> unsigned int Pile<Card::VBox>::getCompressedSize(PileStyle s);
/// Implementation of the getSize() method for Card::VBox
template <> void Pile<Card::VBox>::getSize(int& width, int& height);
/// Implementation of the resize() methods for Card::VBox
template <> void Pile<Card::VBox>::resize(unsigned int pos, PileStyle s);
template <> void Pile<Card::VBox>::resize(Card::Widget& card, PileStyle s);

/// Implementation of the getCompressionRate() method for Card::HBox
template <> unsigned int HPile::getCompressedSize(PileStyle s);
/// Implementation of the getSize() method for Card::HBox
template <> void HPile::getSize(int& width, int& height);
/// Implementation of the resize() methods for Card::HBox
template <> void HPile::resize(unsigned int pos, PileStyle s);
template <> void HPile::resize(Card::Widget& card, PileStyle s);

/**Specializations of Pile, displaying the number of cards as tooltip
 * (especially usefull, if the pile is (very/totally) compressed ;) )
 */
template <class T> class InfoPile : public Pile<T> {
  public:
    InfoPile(IPile::PileStyle style = IPile::NORMAL, IPile::ShowOpt show = IPile::DONT_CHANGE) : Pile<T>(style, show) {}
    virtual ~InfoPile() {}

    virtual void setTopCard(Widget& newCard) {
        Pile<T>::setTopCard(newCard);
        setTooltips();
    }
    void setTopCard(Widget& newCard, bool visible) { Pile<T>::setTopCard(newCard, visible); }

    virtual Widget& removeTopCard() {
        Widget& card(Pile<T>::removeTopCard());
        UNSET_TIP(card);
        setTooltips();
        return card;
    }

    virtual unsigned int insert(Widget& card, unsigned int pos) {
        unsigned int rc(Pile<T>::insert(card, pos));
        setTooltips();
        return rc;
    }

    virtual Widget& remove(Widget& card) {
        Pile<T>::remove(card);
        UNSET_TIP(card);
        setTooltips();
        return card;
    }
    Widget& remove(Widget& card, bool visible) {
        Pile<T>::remove(card, visible);
        return card;
    }
    virtual Widget& remove(unsigned int pos) {
        Widget& card(Pile<T>::remove(pos));
        UNSET_TIP(card);
        setTooltips();
        return card;
    }
    virtual Widget& remove(unsigned int pos, bool visible) { return Pile<T>::remove(pos, visible); }

    virtual void move(unsigned int dest, unsigned int source) {
        Pile<T>::move(dest, source);
        setTooltips();
    }

  protected:
    virtual void resortGUI() {
        Pile<T>::resortGUI();
        setTooltips();
    }

    virtual void setTooltips() {
        Glib::ustring tip(ngettext("%1 card", "%1 cards", Pile<T>::size()));
        tip.replace(tip.find("%1"), 2, YGP::ANumeric::toString(Pile<T>::size()));
        for (unsigned int i(0); i < Pile<T>::size(); ++i)
            SET_TIP(*(Pile<T>::operator[](i)), tip);
    }

    virtual Widget& removeCardFast(unsigned int offset) {
        Widget& card(Pile<T>::removeCardFast(offset));
        UNSET_TIP(card);
        return card;
    }
};

typedef InfoPile<Card::VBox> VInfoPile;
typedef InfoPile<Card::HBox> HInfoPile;

} // namespace Card

#endif
