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
#include <Trace_.h>

#include <CardWidget.h>


// Class to display a pile of cards on the screen
class ICardPile {
 public:
   typedef enum { NORMAL = 0, COMPRESSED, VERY_COMPRESSED, LAST } PileStyle;

   ICardPile (PileStyle style = NORMAL, bool access = true);
   virtual ~ICardPile ();

   // Methods to access pile first-in-last-out
   void flipTopCard ();
   void setTopCardVisible (bool visible = true);
   void setTopCardInvisible () { setTopCardVisible (false); }

   void setTopCards (const vector<CardWidget*>& staple);
   void setTopCards (const vector<CardWidget*>& staple, bool visible);
   void setTopCards (const vector<CardWidget*>& staple, bool visible, bool lastVisible) {
      setTopCards (staple, visible);
      setTopCardVisible (lastVisible); }

   virtual void setTopCard (CardWidget& newCard);
   void setTopCard (CardWidget& newCard, bool visible) {
      newCard.setVisible (visible);
      setTopCard (newCard); }

   CardWidget& getTopCard () const {
      Check3 (cards.size ()); return *cards[cards.size () - 1]; }
   virtual CardWidget& removeTopCard ();

   // Methods to random access pile
   virtual void insert (CardWidget& card, unsigned int pos);
   void insertSorted (CardWidget& card);
   void append (CardWidget& card) { setTopCard (card); }
   virtual CardWidget& remove (CardWidget& card);
   virtual CardWidget& remove (unsigned int pos);

   CardWidget* get (unsigned int id) const;
   CardWidget& at (unsigned int pos) const {
      Check3 (pos < cards.size ()); return *cards[pos]; }

   int findFirstEqualOrBigger (CardWidget::NUMBERS nr) const;
   int findLastEqualOrBigger (CardWidget::NUMBERS nr) const {
      int pos (findFirstEqualOrBigger (nr));
      return (pos == -1) ? - 1 : findLastEqual (pos); }
   int findLastEqual (unsigned int pos) const;

   bool exists (CardWidget::NUMBERS nr) const {
      int pos (findFirstEqualOrBigger (nr));
      return (pos != -1) && (at (pos).number () == nr); }
   bool exists (CardWidget& card) const { exists (&card); }
 bool exists (CardWidget* card) const {
      return find (cards.begin (), cards.end (), card) != cards.end (); }

   // General management-functions
   virtual void resize (CardWidget& card, PileStyle s) const = 0;
   unsigned int numberOfCards () const { return cards.size (); }
   void clear ();
   void setStyle (PileStyle s);
   void setAccessable (bool access);

   bool topCardVisible () const { return getTopCard ().visible (); }

   virtual void sortByNumber ();
   virtual void sortByColor ();

 protected:
   vector<CardWidget*> cards;
   PileStyle style;
   bool accessable;

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
   CardPile (PileStyle style = NORMAL, bool access = true)
      : ICardPile (style, access) { }
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
   virtual CardWidget& remove (unsigned int pos) {
      CardWidget& card (ICardPile::remove (pos));
      T::remove (card);
      return card; }

   virtual void resize (CardWidget& card, PileStyle s) const { }
   virtual void sortByNumber () {
      if (cards.size ()) {
         resize (getTopCard (), style);
         ICardPile::sortByNumber ();
         resortGUI (); } }
   virtual void sortByColor () {
      if (cards.size ()) {
         resize (getTopCard (), style);
         ICardPile::sortByColor ();
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
   static unsigned int height[(int)LAST] = { card.getImageHeight (), 15, 1 };
   TRACE5 ("CardVPile::resize (CardWidget&, PileStyle) - " << (int)s << " (" << height[0] << ')');
   card.set_usize (-1, height[(int)s]);
}

void CardHPile::resize (CardWidget& card, PileStyle s) const {
   static unsigned int width[(int)LAST] = { card.getImageWidth (), 18, 1 };
   TRACE5 ("CardHPile::resize (CardWidget&, PileStyle) - " << (int)s << " (" << width[0] << ')');
   card.set_usize (width[(int)s], -1);
}


// Specializations of CardPile, displaying the number of cards as tooltip
// (especially usefull, if the pile is (very) compressed ;) )
template <class T> class CardInfoPile : public CardPile<T> {
 public:
   CardInfoPile (PileStyle style = NORMAL, bool access = true)
      : CardPile<T> (style, access), tt (NULL) { }
   CardInfoPile (Gtk::Tooltips& tips, PileStyle style = NORMAL, bool access = true)
      : CardPile<T> (style, access), tt (&tips) {
      tt->set_delay (2); }
   virtual ~CardInfoPile () { }

   virtual void setTopCard (CardWidget& newCard) {
      CardPile<T>::setTopCard (newCard);
      setTooltips (); }
   void setTopCard (CardWidget& newCard, bool visible) {
      CardPile<T>::setTopCard (newCard, visible); }

   virtual CardWidget& removeTopCard () {
      CardWidget& card (CardPile<T>::removeTopCard ());
      Check (tt);
      tt->set_tip (card);
      setTooltips ();
      return card; }

   virtual void insert (CardWidget& card, unsigned int pos) {
      CardPile<T>::insert (card, pos);
      setTooltips (); }

   virtual CardWidget& remove (CardWidget& card) {
      CardPile<T>::remove (card);
      Check (tt);
      tt->set_tip (card);
      setTooltips ();
      return card; }
   virtual CardWidget& remove (unsigned int pos) {
      CardWidget& card (CardPile<T>::remove (pos));
      Check (tt);
      tt->set_tip (card);
      setTooltips ();
      return card; }

   void setTooltipObject (Gtk::Tooltips& tips) { tt = &tips; }

 protected:
   virtual void resortGUI () {
      CardPile<T>::resortGUI ();
      setTooltips (); }

   virtual void setTooltips () {
      Check (tt);
      for (int i (0); i < cards.size (); ++i) {
         std::string tip (_("Card number %1 of %2"));
         tip.replace (tip.find ("%1"), 2,
                      ANumeric::toString ((unsigned long)(i + 1)));
         tip.replace (tip.find ("%2"), 2,
                      ANumeric::toString ((unsigned long)cards.size ()));
         tt->set_tip (*cards[i], tip);
      }
   }

 private:
    Gtk::Tooltips* tt;
};


typedef CardInfoPile<Gtk::VBox>  CardVInfoPile;
typedef CardInfoPile<Gtk::HBox>  CardHInfoPile;


#endif
