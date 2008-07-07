#ifndef CARDPILE_H
#define CARDPILE_H

//$Id$

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

#include <string>
#include <vector>
#include <algorithm>

#include <gtkmm/box.h>
#include <gtkmm/image.h>
#include <gtkmm/button.h>
#ifdef HAVE_GTKMM2_12
#  include <gtk/gtkeventbox.h>

#  define SET_TIP(widget, text)      (widget).set_tooltip_text (text)
#  define UNSET_TIP(widget)          (widget).set_has_tooltip (false)
#else
#  include <gtkmm/tooltips.h>

#  define SET_TIP(widget, text)      tt.set_tip ((card), (text))
#  define UNSET_TIP(widget)          tt.unset_tip (widget)
#endif

#include <YGP/Check.h>
#include <YGP/Trace.h>
#include <YGP/ANumeric.h>

#include <CardWidget.h>


/**Class to display a pile of cards on the screen
 * \todo Don't derive from std::vector
 */
class ICardPile : public std::vector<CardWidget*> {
 public:
   typedef enum { NORMAL = 0, COMPRESSED, QUITE_COMPRESSED, VERY_COMPRESSED,
                  TOTALLY_COMPRESSED, LAST } PileStyle;
   typedef enum { SHOWBACK = 0, SHOWFACE, DONT_CHANGE } ShowOpt;

   typedef bool (*CMPFUNC) (const CardWidget*, const CardWidget*);
   typedef int (*CMPFUNC2) (const CardWidget&, const CardWidget&);

   ICardPile (PileStyle style = NORMAL, ShowOpt show = DONT_CHANGE);
   virtual ~ICardPile ();

   /// \name Methods to access pile first-in-last-out
   //@{
   void flipTopCard ();
   void showTopCardFace (bool visible = true);
   void showTopCardBack () { showTopCardFace (false); }

   void setTopCards (const std::vector<CardWidget*>& staple);
   void setTopCards (const std::vector<CardWidget*>& staple, bool visible);
   void setTopCards (const std::vector<CardWidget*>& staple, bool visible, bool lastVisible) {
      setTopCards (staple, visible);
      showTopCardFace (lastVisible); }

   virtual void setTopCard (CardWidget& newCard);
   void setTopCard (CardWidget& newCard, bool visible) {
      newCard.showFace (visible);
      setTopCard (newCard); }

   CardWidget& getTopCard () const {
      Check3 (size ()); return *operator[] (size () - 1); }
   virtual CardWidget& removeTopCard ();
   CardWidget& removeShownTopCard (bool visible = true) {
      CardWidget& topCard (removeTopCard ());
      topCard.showFace (visible);
      return topCard; }
   //@}

   /// \name Methods to random access pile
   //@{
   virtual unsigned int insert (CardWidget& card, unsigned int pos);
   unsigned int insertSorted (CardWidget& card, CMPFUNC fnSort = compCardsByNr);
   unsigned int insertColourSorted (CardWidget& card) {
      return insertSorted (card, compCards); }
   void append (CardWidget& card) { setTopCard (card); }
   virtual CardWidget& remove (CardWidget& card);
   virtual CardWidget& remove (unsigned int pos);

   virtual void move (unsigned int dest, unsigned int source);

   CardWidget* get (unsigned int id) const;
   //@}

   /// \name Find methods
   //@{
   int find (const CardWidget& card, CMPFUNC fnComp) const {
      int pos (find1EqualOrBigger (card, fnComp));
      return ((pos != -1)
              && !fnComp (operator[] (pos), &card)) ? pos : -1; }
   int findByNr (const CardWidget& card) const { return find (card, compCardsByNr); }
   int findByColour (const CardWidget& card) const { return find (card, compCards); }
   int findByID (const CardWidget& card) const { return find (card, compCardsByID); }
   int find1EqualOrBigger (const CardWidget& card, CMPFUNC fnComp) const {
      std::vector<CardWidget*>::const_iterator i
         (std::lower_bound (begin (), end (), &card, fnComp));
      return ((i != end () && !fnComp (*i, &card))
              ? (i - begin ()) : -1); }
   int find1EqualOrBiggerByNr (const CardWidget& card) const {
      return find1EqualOrBigger (card, compCardsByNr); }
   int find1EqualOrBiggerByColour (const CardWidget& card) const {
      return find1EqualOrBigger (card, compCards); }
   int findFirstEqualOrBigger (CardWidget::NUMBERS nr) const;
   int findFirstEqualOrBiggerColour (CardWidget::COLOURS nr) const;
   int findLastEqualOrBigger (CardWidget::NUMBERS nr) const {
      int pos (findFirstEqualOrBigger (nr));
      return (pos == -1) ? - 1 : findLastEqual (pos); }
   int findLastEqualOrBiggerColour (CardWidget::COLOURS col) const;
   int findLastEqual (unsigned int pos) const;
   int findLastEqualColour (unsigned int pos) const;
   int findFirstEqual (unsigned int pos) const;
   int findFirstEqualColour (unsigned int pos) const;
   int find (CardWidget::NUMBERS nr, unsigned int start = 0) const;
   int find (CardWidget::COLOURS colour, unsigned int start = 0) const;
   int find (unsigned int id, unsigned int start = 0) const;

   unsigned int findLowestCard () const;
   unsigned int findLowestCard (CardWidget::COLOURS excludeColour) const;
   //@}

   /// \name Test availability
   //@{
   bool exists (CardWidget::NUMBERS nr, unsigned int start = 0) const {
      return find (nr, start) != -1; }
   bool exists (CardWidget::COLOURS colour, unsigned int start = 0) const {
      return find (colour, start) != -1; }
   bool exists (const CardWidget& card) const { return exists (&card); }
   bool exists (const CardWidget* card) const {
      return std::find (begin (), end (), card) != end (); }
   //@}

   /// \name General management-functions
   //@{
   virtual void getSize (int& width, int& height);
   virtual void resize (unsigned int pos, PileStyle s) {
      if (pos != -1U)
         resize (*operator[] (pos), s); }
   virtual void resize (CardWidget& card, PileStyle s);
   void clear ();
   void setStyle (PileStyle s);
   PileStyle getStyle () const { return style; }
   void setShowOption (ShowOpt show);
   ShowOpt getShowOption () const { return showOpt; }

   bool topCardShowsFace () const { return getTopCard ().showsFace (); }

   bool hasFittingPair (const CardWidget& card, CMPFUNC2 cmp, bool doubles = true) const;
   iterator getFittingCard (const CardWidget& card, CMPFUNC2 cmp) const {
      return getFittingCard (card, begin (), cmp); }
   iterator getFittingCard (const CardWidget& card, const_iterator start, CMPFUNC2 cmp) const;

   unsigned int sortColourSerie (std::map<unsigned int, unsigned int>& aPos,
                                 std::vector<unsigned int>& aOrder);
   unsigned int getSeries (CardWidget& card, std::map<unsigned int, unsigned int>& aPos,
                           std::vector<unsigned int>& aOrder, CMPFUNC2 cmp,
                           bool doubles = true);

   virtual Gtk::Box* getWidget () const { return NULL; }
   //@}

   /// \name Sorting
   //@{
   virtual void sort (CMPFUNC fnSort);
   void sortByNumber () { sort (compCardsByNr); }
   void sortByColour () { sort (compCards); }

   static bool compCards (const CardWidget* a, const CardWidget* b);
   static bool compCardsByNr (const CardWidget* a, const CardWidget* b);
   static bool compCardsByID (const CardWidget* a, const CardWidget* b);
   //@}

 protected:
   PileStyle style;
   ShowOpt showOpt;

 private:
   static void deleteElement (unsigned int elem,
			      std::map<unsigned int, unsigned int>& aPos,
			      std::vector<unsigned int>& aOrder);
};


/**Specializations of ICardPile to display. The template must support the
 * following methods:
 *
 *   - pack_start()
 *   - remove()
 *   - reorder_child()
 *
 * Designed to be used with the Gtk::?Box-classes
 */
template <class T> class CardPile : public T, public ICardPile {
 public:
   CardPile (PileStyle style = NORMAL, ShowOpt show = DONT_CHANGE)
      : ICardPile (style, show) { }
   virtual ~CardPile () { }

   virtual void setTopCard (CardWidget& newCard) {
      ICardPile::setTopCard (newCard);
      T::pack_start (newCard, Gtk::PACK_SHRINK); }
   void setTopCard (CardWidget& newCard, bool visible) {
      ICardPile::setTopCard (newCard, visible); }

   virtual CardWidget& removeTopCard () {
      CardWidget& card (ICardPile::removeTopCard ());
      T::remove (card);
      return card; }

   virtual unsigned int insert (CardWidget& card, unsigned int pos) {
      T::pack_start (card, Gtk::PACK_SHRINK);
      T::reorder_child (card, pos);
      return ICardPile::insert (card, pos);
   }

   virtual CardWidget& remove (CardWidget& card) {
      T::remove (card);
      return ICardPile::remove (card); }
   CardWidget& remove (CardWidget& card, bool visible) {
      card.showFace (visible);
      return remove (card); }
   virtual CardWidget& remove (unsigned int pos) {
      T::remove (*operator[] (pos));
      return ICardPile::remove (pos); }
   CardWidget& remove (unsigned int pos, bool visible) {
      CardWidget& card (remove (pos));
      card.showFace (visible);
      return card; }

   virtual void resize (unsigned int pos, PileStyle s) { ICardPile::resize (pos, s); }
   virtual void resize (CardWidget& card, PileStyle s) { Check (0); }
   virtual void sort (CMPFUNC fnSort) {
      if (size ()) {
         resize (size () - 1, style);
         ICardPile::sort (fnSort);
         resortGUI (); } }
   static unsigned int getCompressedSize (PileStyle s) { return 0; }
   unsigned int getCompressedSize () { return getCompressedSize (style); }
   virtual void getSize (int& width, int& height) { ICardPile::getSize (width, height); }

   virtual Gtk::Box* getWidget () const { return static_cast<T*> (const_cast<CardPile*> (this)); }

 protected:
   virtual void resortGUI () {
      for (unsigned int i (0); i < size (); ++i)
         Gtk::Box::reorder_child (*operator[] (i), i);
      if (size ())
         resize (size () - 1, NORMAL);
   }

 private:
   CardPile (const CardPile<T>& other);
   CardPile<T>& operator= (const CardPile<T>& other);
};


typedef CardPile<Gtk::VBox>  CardVPile;
typedef CardPile<Gtk::HBox>  CardHPile;


/// Implementation of the getCompressionRate() method for Gtk::VBox
template <> inline unsigned int CardVPile::getCompressedSize (PileStyle s) {
   int height[(int)LAST] = { CardImages::HEIGHT, 15, 7, 1 };
   return height[s];
}

/// Implementation of the getSize() method for Gtk::VBox
template <> inline void CardVPile::getSize (int& width, int& height) {
   if (size ()) {
      height = CardImages::HEIGHT;
      width = CardImages::WIDTH;
      if (size () > 1)
	 height += getCompressedSize () * (size () - 1);
   }
   else
      ICardPile::getSize (width, height);
}

/// Implementation of the resize() methods for Gtk::VBox
template <> inline void CardVPile::resize (unsigned int pos, PileStyle s) {
   ICardPile::resize (pos, s); }
template <> inline void CardVPile::resize (CardWidget& card, PileStyle s) {
   if (s >= TOTALLY_COMPRESSED)
      card.hide ();
   else {
      card.set_size_request (-1, getCompressedSize (s));
      if (style == TOTALLY_COMPRESSED)
         card.show ();
   }
}

/// Implementation of the getCompressionRate() method for Gtk::HBox
template <> inline unsigned int CardHPile::getCompressedSize (PileStyle s) {
   int width[(int)LAST] = { CardImages::WIDTH, 18, 7, 1 };
   return width[s];
}

/// Implementation of the getSize() method for Gtk::HBox
template <> inline void CardHPile::getSize (int& width, int& height) {
   if (size ()) {
      height = CardImages::HEIGHT;
      width = CardImages::WIDTH;
      if (size () > 1)
	 width += getCompressedSize () * (size () - 1);
   }
   else
      ICardPile::getSize (width, height);
}

///Implementation of the resize() methods for Gtk::HBox
template <> inline void CardHPile::resize (unsigned int pos, PileStyle s) {
   ICardPile::resize (pos, s); }
template <> inline void CardHPile::resize (CardWidget& card, PileStyle s) {
   if (s >= TOTALLY_COMPRESSED)
      card.hide ();
   else {
      card.set_size_request (getCompressedSize (s), -1);
      if (style == TOTALLY_COMPRESSED)
         card.show ();
   }
}


/**Specializations of CardPile, displaying the number of cards as tooltip
 * (especially usefull, if the pile is (very/totally) compressed ;) )
 */
template <class T> class CardInfoPile : public CardPile<T> {
 public:
   CardInfoPile (ICardPile::PileStyle style = ICardPile::NORMAL,
                 ICardPile::ShowOpt show = ICardPile::DONT_CHANGE)
      : CardPile<T> (style, show) { }
   virtual ~CardInfoPile () { }

   virtual void setTopCard (CardWidget& newCard) {
      CardPile<T>::setTopCard (newCard);
      setTooltips (); }
   void setTopCard (CardWidget& newCard, bool visible) {
      CardPile<T>::setTopCard (newCard, visible); }

   virtual CardWidget& removeTopCard () {
      CardWidget& card (CardPile<T>::removeTopCard ());
      UNSET_TIP (card);
      setTooltips ();
      return card; }

   virtual unsigned int insert (CardWidget& card, unsigned int pos) {
      unsigned int rc (CardPile<T>::insert (card, pos));
      setTooltips ();
      return rc; }

   virtual CardWidget& remove (CardWidget& card) {
      CardPile<T>::remove (card);
      UNSET_TIP (card);
      setTooltips ();
      return card; }
   CardWidget& remove (CardWidget& card, bool visible) {
      CardPile<T>::remove (card, visible);
      return card; }
   virtual CardWidget& remove (unsigned int pos) {
      CardWidget& card (CardPile<T>::remove (pos));
      UNSET_TIP (card);
      setTooltips ();
      return card; }
   virtual CardWidget& remove (unsigned int pos, bool visible) {
      return CardPile<T>::remove (pos, visible); }

   virtual void move (unsigned int dest, unsigned int source) {
      CardPile<T>::move (dest, source);
      setTooltips (); }

 protected:
   virtual void resortGUI () {
      CardPile<T>::resortGUI ();
      setTooltips (); }

   virtual void setTooltips () {
      Glib::ustring tip (ngettext ("%1 card", "%1 cards", CardPile<T>::size ()));
      tip.replace (tip.find ("%1"), 2,
                   YGP::ANumeric::toString (CardPile<T>::size ()));
      for (unsigned int i (0); i < CardPile<T>::size (); ++i)
         SET_TIP (*(CardPile<T>::operator[] (i)), tip);
   }

 private:
#ifndef HAVE_GTKMM2_12
    Gtk::Tooltips tt;
#endif
};

typedef CardInfoPile<Gtk::VBox>  CardVInfoPile;
typedef CardInfoPile<Gtk::HBox>  CardHInfoPile;

#endif
