#ifndef CARDPILE_H
#define CARDPILE_H

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


#include <cardgames-cfg.h>

#include <string>
#include <vector>
#include <algorithm>

#include <gtkmm/box.h>
#include <gtkmm/image.h>
#include <gtkmm/button.h>
#include <gtkmm/tooltips.h>

#include <Trace_.h>
#include <Check.h>
#include <ANumeric.h>

#include <CardWidget.h>


/**Class to display a pile of cards on the screen
 */
class ICardPile : public std::vector<CardWidget*> {
 public:
   typedef enum { NORMAL = 0, COMPRESSED, QUITE_COMPRESSED, VERY_COMPRESSED,
                  TOTALLY_COMPRESSED, LAST } PileStyle;
   typedef enum { SHOWBACK = 0, SHOWFACE, DONT_CHANGE } ShowOpt;

   typedef bool (*CMPFUNC) (const CardWidget*, const CardWidget*);

   ICardPile (PileStyle style = NORMAL, ShowOpt show = DONT_CHANGE);
   virtual ~ICardPile ();

   // Methods to access pile first-in-last-out
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

   // Methods to random access pile
   virtual void insert (CardWidget& card, unsigned int pos);
   void insertSorted (CardWidget& card, CMPFUNC fnSort = compCardsByNr);
   void insertColourSorted (CardWidget& card) {
      insertSorted (card, compCards); }
   void append (CardWidget& card) { setTopCard (card); }
   virtual CardWidget& remove (CardWidget& card);
   virtual CardWidget& remove (unsigned int pos);

   virtual void move (unsigned int dest, unsigned int source);

   CardWidget* get (unsigned int id) const;

   int find (CardWidget& card, CMPFUNC fnComp) const {
      int pos (find1EqualOrBigger (card, fnComp));
      return ((pos != -1)
              && !fnComp (operator[] (pos), &card)) ? pos : -1; }
   int findByNr (CardWidget& card) const { return find (card, compCardsByNr); }
   int findByColour (CardWidget& card) const { return find (card, compCards); }
   int findByID (CardWidget& card) const { return find (card, compCardsByID); }
   int find1EqualOrBigger (const CardWidget& card, CMPFUNC fnComp) const {
      std::vector<CardWidget*>::const_iterator i
         (std::lower_bound (begin (), end (), &card, fnComp));
      return ((i != end () && !fnComp (*i, &card))
              ? (i - begin ()) : -1); }
   int find1EqualOrBiggerByNr (CardWidget& card) const {
      return find1EqualOrBigger (card, compCardsByNr); }
   int find1EqualOrBiggerByColour (CardWidget& card) const {
      return find1EqualOrBigger (card, compCards); }
   int findFirstEqualOrBigger (CardWidget::NUMBERS nr) const;
   int findLastEqualOrBigger (CardWidget::NUMBERS nr) const {
      int pos (findFirstEqualOrBigger (nr));
      return (pos == -1) ? - 1 : findLastEqual (pos); }
   int findLastEqual (unsigned int pos) const;
   int findFirstEqual (unsigned int pos) const;
   int find (CardWidget::NUMBERS nr, unsigned int start = 0) const;
   int find (CardWidget::COLOURS colour, unsigned int start = 0) const;
   int find (unsigned int id, unsigned int start = 0) const;

   bool exists (CardWidget::NUMBERS nr, unsigned int start = 0) const {
      return find (nr, start) != -1; }
   bool exists (CardWidget::COLOURS colour, unsigned int start = 0) const {
      return find (colour, start) != -1; }
   bool exists (CardWidget& card) const { return exists (&card); }
   bool exists (CardWidget* card) const {
      return std::find (begin (), end (), card) != end (); }

   // General management-functions
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

   virtual void sort (CMPFUNC fnSort);
   void sortByNumber () { sort (compCardsByNr); }
   void sortByColour () { sort (compCards); }

 protected:
   PileStyle style;
   ShowOpt showOpt;

 private:
   static bool compCards (const CardWidget* a, const CardWidget* b);
   static bool compCardsByNr (const CardWidget* a, const CardWidget* b);
   static bool compCardsByID (const CardWidget* a, const CardWidget* b);
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
      pack_start (newCard, Gtk::PACK_SHRINK); }
   void setTopCard (CardWidget& newCard, bool visible) {
      ICardPile::setTopCard (newCard, visible); }

   virtual CardWidget& removeTopCard () {
      CardWidget& card (ICardPile::removeTopCard ());
      T::remove (card);
      return card; }

   virtual void insert (CardWidget& card, unsigned int pos) {
      pack_start (card, Gtk::PACK_SHRINK);
      reorder_child (card, pos);
      ICardPile::insert (card, pos);
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

 protected:
   virtual void resortGUI () {
      for (unsigned int i (0); i < size (); ++i)
         Gtk::Box::reorder_child (*operator[] (i), i);
      if (size ())
         resize (size () - 1, NORMAL);
   }
};


typedef CardPile<Gtk::VBox>  CardVPile;
typedef CardPile<Gtk::HBox>  CardHPile;


/**Implementation of the resize() methods for Gtk::VBox
 */
inline void CardVPile::resize (unsigned int pos, PileStyle s) {
   ICardPile::resize (pos, s); }
inline void CardVPile::resize (CardWidget& card, PileStyle s) {
   if (s == TOTALLY_COMPRESSED)
      card.hide ();
   else {
      int height[(int)LAST] = { card.getImageHeight (), 15, 7, 1 };
      card.set_size_request (-1, height[s]);
      if (style == TOTALLY_COMPRESSED)
         card.show ();
   }
}

/**Implementation of the resize() methods for Gtk::HBox
 */
inline void CardHPile::resize (unsigned int pos, PileStyle s) {
   ICardPile::resize (pos, s); }
inline void CardHPile::resize (CardWidget& card, PileStyle s) {
   if (s == TOTALLY_COMPRESSED)
      card.hide ();
   else {
      int width[(int)LAST] = { card.getImageWidth(), 18, 7, 1 };
      card.set_size_request (width[s], -1);
      if (style == TOTALLY_COMPRESSED)
         card.show ();
   }
}


/**Specializations of CardPile, displaying the number of cards as tooltip
 * (especially usefull, if the pile is (very/totally) compressed ;) )
 */
template <class T> class CardInfoPile : public CardPile<T> {
 public:
   CardInfoPile (ICardPile::PileStyle style = NORMAL,
                 ICardPile::ShowOpt show = DONT_CHANGE)
      : CardPile<T> (style, show) { }
   virtual ~CardInfoPile () { }

   virtual void setTopCard (CardWidget& newCard) {
      CardPile<T>::setTopCard (newCard);
      setTooltips (); }
   void setTopCard (CardWidget& newCard, bool visible) {
      CardPile<T>::setTopCard (newCard, visible); }

   virtual CardWidget& removeTopCard () {
      CardWidget& card (CardPile<T>::removeTopCard ());
      tt.unset_tip (card);
      setTooltips ();
      return card; }

   virtual void insert (CardWidget& card, unsigned int pos) {
      CardPile<T>::insert (card, pos);
      setTooltips (); }

   virtual CardWidget& remove (CardWidget& card) {
      CardPile<T>::remove (card);
      tt.unset_tip (card);
      setTooltips ();
      return card; }
   CardWidget& remove (CardWidget& card, bool visible) {
      CardPile<T>::remove (card, visible); }
   virtual CardWidget& remove (unsigned int pos) {
      CardWidget& card (CardPile<T>::remove (pos));
      tt.unset_tip (card);
      setTooltips ();
      return card; }
   virtual CardWidget& remove (unsigned int pos, bool visible) {
      return CardPile<T>::remove (pos, visible); }

   virtual void move (unsigned int dest, unsigned int source) {
      CardPile<T>::move (dest, source);
      setTooltips (); }


   void showTips (bool on = true) { on ? tt.enable () : tt.disable (); }

 protected:
   virtual void resortGUI () {
      CardPile<T>::resortGUI ();
      setTooltips (); }

   virtual void setTooltips () {
      std::string tip (ngettext ("%1 card", "%1 cards", size ()));
      tip.replace (tip.find ("%1"), 2,
                   ANumeric::toString (size ()));
      for (unsigned int i (0); i < size (); ++i)
         tt.set_tip (*operator[] (i), tip);
   }

 private:
    Gtk::Tooltips tt;
};

typedef CardInfoPile<Gtk::VBox>  CardVInfoPile;
typedef CardInfoPile<Gtk::HBox>  CardHInfoPile;

#endif
