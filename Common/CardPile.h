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

#include <gtk--/box.h>
#include <gtk--/tooltips.h>

#include <ANumeric.h>

#include <Check.h>

#include <CardWidget.h>


// Class to display a pile of cards on the screen
class ICardPile {
 public:
   typedef enum { NORMAL = 0, COMPRESSED, QUITE_COMPRESSED, VERY_COMPRESSED,
                  LAST } PileStyle;
   typedef enum { SHOWBACK = 0, SHOWFACE, DONT_CHANGE } ShowOpt;

   typedef bool (*CMPFUNC) (const CardWidget*, const CardWidget*);

   ICardPile (PileStyle style = NORMAL, ShowOpt show = DONT_CHANGE);
   virtual ~ICardPile ();

   // Methods to access pile first-in-last-out
   void flipTopCard ();
   void showTopCardFace (bool visible = true);
   void showTopCardBack () { showTopCardFace (false); }

   void setTopCards (const vector<CardWidget*>& staple);
   void setTopCards (const vector<CardWidget*>& staple, bool visible);
   void setTopCards (const vector<CardWidget*>& staple, bool visible, bool lastVisible) {
      setTopCards (staple, visible);
      showTopCardFace (lastVisible); }

   virtual void setTopCard (CardWidget& newCard);
   void setTopCard (CardWidget& newCard, bool visible) {
      newCard.showFace (visible);
      setTopCard (newCard); }

   CardWidget& getTopCard () const {
      Check3 (cards.size ()); return *cards[cards.size () - 1]; }
   virtual CardWidget& removeTopCard ();
   CardWidget& removeShownTopCard (bool visible = true) {
      CardWidget& topCard (removeTopCard ());
      topCard.showFace (visible);
      return topCard; }

   // Methods to random access pile
   virtual void insert (CardWidget& card, unsigned int pos);
   void insertSorted (CardWidget& card);
   void insertColorSorted (CardWidget& card);
   void append (CardWidget& card) { setTopCard (card); }
   virtual CardWidget& remove (CardWidget& card);
   virtual CardWidget& remove (unsigned int pos);

   CardWidget* get (unsigned int id) const;
   CardWidget& at (unsigned int pos) const {
      Check3 (pos < cards.size ()); return *cards[pos]; }

   int find (CardWidget& card, CMPFUNC fnComp) const {
      int pos (find1EqualOrBigger (card, fnComp));
      return ((pos != -1)
              && !fnComp (cards[pos], &card)) ? pos : -1; }
   int findByNr (CardWidget& card) const { return find (card, compCardsByNr); }
   int findByColor (CardWidget& card) const { return find (card, compCards); }
   int find1EqualOrBigger (const CardWidget& card, CMPFUNC fnComp) const {
      CardWidget* const* low (lower_bound (cards.begin (), cards.end (),
                                           &card, fnComp));
      return ((low != cards.end () && !fnComp (*low, &card))
              ? (low - cards.begin ()) : -1); }
   int find1EqualOrBiggerByNr (CardWidget& card) const {
      return find1EqualOrBigger (card, compCardsByNr); }
   int find1EqualOrBiggerByColor (CardWidget& card) const {
      return find1EqualOrBigger (card, compCards); }
   int findFirstEqualOrBigger (CardWidget::NUMBERS nr) const;
   int findLastEqualOrBigger (CardWidget::NUMBERS nr) const {
      int pos (findFirstEqualOrBigger (nr));
      return (pos == -1) ? - 1 : findLastEqual (pos); }
   int findLastEqual (unsigned int pos) const;

   bool exists (CardWidget::NUMBERS nr, unsigned int start = 0) const {
      for (; start < cards.size (); ++start)
         if (cards[start]->number () == nr)
            return true;
      return false;
   }
   bool exists (CardWidget& card) const { exists (&card); }
   bool exists (CardWidget* card) const {
      return ::find (cards.begin (), cards.end (), card) != cards.end (); }

   // General management-functions
   // - resize is actually a virtual static method but as this does
   //   not exist and static alone does not work neither: virtual
   virtual void resize (CardWidget& card, PileStyle s) const = 0;
   unsigned int numberOfCards () const { return cards.size (); }
   void clear ();
   void setStyle (PileStyle s);
   PileStyle getStyle () const { return style; }
   void setShowOption (ShowOpt show);
   ShowOpt getShowOption () const { return showOpt; }

   bool topCardShowsFace () const { return getTopCard ().showsFace (); }

   virtual void sort (CMPFUNC fnSort);
   void sortByNumber () { sort (compCardsByNr); }
   void sortByColor () { sort (compCards); }

 protected:
   vector<CardWidget*> cards;
   PileStyle style;
   ShowOpt showOpt;

 private:
   static bool compCards (const CardWidget* a, const CardWidget* b);
   static bool compCardsByNr (const CardWidget* a, const CardWidget* b);
};


// Specializations of ICardPile to display. The template must support:
//
//   - pack_start, remove, reorder_child - Methods
//
// Designed to be used with the Gtk::?Box-classes
template <class T> class CardPile : public T, public ICardPile {
 public:
   CardPile (PileStyle style = NORMAL, ShowOpt show = DONT_CHANGE)
      : ICardPile (style, show) { }
   virtual ~CardPile () { }

   virtual void setTopCard (CardWidget& newCard) {
      ICardPile::setTopCard (newCard);
      pack_start (newCard, false); }
   void setTopCard (CardWidget& newCard, bool visible) {
      ICardPile::setTopCard (newCard, visible); }

   virtual CardWidget& removeTopCard () {
      CardWidget& card (ICardPile::removeTopCard ());
      T::remove (card);
      return card; }

   virtual void insert (CardWidget& card, unsigned int pos) {
      ICardPile::insert (card, pos);
      pack_start (card, false);
      reorder_child (card, pos); }

   virtual CardWidget& remove (CardWidget& card) {
      T::remove (ICardPile::remove (card));
      return card; }
   CardWidget& remove (CardWidget& card, bool visible) {
      card.showFace (visible);
      return remove (card); }
   virtual CardWidget& remove (unsigned int pos) {
      CardWidget& card (ICardPile::remove (pos));
      T::remove (card);
      return card; }
   CardWidget& remove (unsigned int pos, bool visible) {
      CardWidget& card (remove (pos));
      card.showFace (visible);
      return card; }

   virtual void resize (CardWidget& card, PileStyle s) const { }
   virtual void sort (CMPFUNC fnSort) {
      if (cards.size ()) {
         resize (getTopCard (), style);
         ICardPile::sort (fnSort);
         resortGUI (); } }

 protected:
   virtual void resortGUI () {
      for (int i (0); i < cards.size (); ++i)
         Gtk::Box::reorder_child (*cards[i], i);
      resize (getTopCard (), NORMAL);
      }
};


typedef CardPile<Gtk::VBox>  CardVPile;
typedef CardPile<Gtk::HBox>  CardHPile;


void CardVPile::resize (CardWidget& card, PileStyle s) const {
   static unsigned int height[(int)LAST] = { card.getImageHeight (), 15, 7, 1 };
   card.set_usize (-1, height[(int)s]);
}

void CardHPile::resize (CardWidget& card, PileStyle s) const {
   static unsigned int width[(int)LAST] = { card.getImageWidth (), 18, 7, 1 };
   card.set_usize (width[(int)s], -1);
}


// Specializations of CardPile, displaying the number of cards as tooltip
// (especially usefull, if the pile is (very) compressed ;) )
template <class T> class CardInfoPile : public CardPile<T> {
 public:
   CardInfoPile (PileStyle style = NORMAL, ShowOpt show = DONT_CHANGE)
      : CardPile<T> (style, show) {
      tt.set_delay (2); }
   virtual ~CardInfoPile () { }

   virtual void setTopCard (CardWidget& newCard) {
      CardPile<T>::setTopCard (newCard);
      setTooltips (); }
   void setTopCard (CardWidget& newCard, bool visible) {
      CardPile<T>::setTopCard (newCard, visible); }

   virtual CardWidget& removeTopCard () {
      CardWidget& card (CardPile<T>::removeTopCard ());
      tt.set_tip (card);
      setTooltips ();
      return card; }

   virtual void insert (CardWidget& card, unsigned int pos) {
      CardPile<T>::insert (card, pos);
      setTooltips (); }

   virtual CardWidget& remove (CardWidget& card) {
      CardPile<T>::remove (card);
      tt.set_tip (card);
      setTooltips ();
      return card; }
   CardWidget& remove (CardWidget& card, bool visible) {
      CardPile<T>::remove (card, visible); }
   virtual CardWidget& remove (unsigned int pos) {
      CardWidget& card (CardPile<T>::remove (pos));
      tt.set_tip (card);
      setTooltips ();
      return card; }
   virtual CardWidget& remove (unsigned int pos, bool visible) {
      CardPile<T>::remove (pos, visible); }

   void showTips (bool on = true) { on ? tt.enable () : tt.disable (); }

 protected:
   virtual void resortGUI () {
      CardPile<T>::resortGUI ();
      setTooltips (); }

   virtual void setTooltips () {
      std::string tip (ngettext ("%1 card", "%1 cards", cards.size ()));
      tip.replace (tip.find ("%1"), 2,
                   ANumeric::toString ((unsigned long)cards.size ()));
      for (int i (0); i < cards.size (); ++i)
         tt.set_tip (*cards[i], tip);
   }

 private:
    Gtk::Tooltips tt;
};


typedef CardInfoPile<Gtk::VBox>  CardVInfoPile;
typedef CardInfoPile<Gtk::HBox>  CardHInfoPile;


#endif
