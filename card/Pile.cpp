// PROJECT     : Cardgames
// SUBSYSTEM   : Common
// REFERENCES  :
// TODO        :
// BUGS        :
// AUTHOR      : Markus Schwab
// CREATED     : 03.04.2002
// COPYRIGHT   : Copyright (C) 2002 - 2018, 2026

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

#include "Pile.h"

namespace Card {

//-----------------------------------------------------------------------------
/// Constructor; adds all controls to the dialog
/// \param set Specifier for type of cardset
/// \param show Flag, if cards show their faces
//-----------------------------------------------------------------------------
IPile::IPile(PileStyle s, ShowOpt show) : style(s), showOpt(show), cards() {
    TRACE3("IPile::IPile(PileStyle) - " << (int)style);
    Check3(s < LAST);
}

//-----------------------------------------------------------------------------
/// Destructor
//-----------------------------------------------------------------------------
IPile::~IPile() { TRACE9("IPile::~IPile()"); }

//-----------------------------------------------------------------------------
/// Sets the top card of the pile
/// \param newCard New top-card
//-----------------------------------------------------------------------------
void IPile::setTopCard(Widget& card) {
    TRACE5("IPile::setTopCard(Widget&) - Card " << card << " -> new size: " << size() + 1);

    if ((style > NORMAL) && size()) {
        Check3(operator[](size() - 1));
        resize(size() - 1, style);
    }

    if (showOpt < DONT_CHANGE)
        card.showFace((bool)showOpt);

    cards.push_back(&card);
    resize(size() - 1, NORMAL);
}

//-----------------------------------------------------------------------------
/// Returns and removes the top card of the pile
/// \returns Widget& Reference to (removed) card
//-----------------------------------------------------------------------------
Widget& IPile::removeTopCard() {
    TRACE5("IPile::removeTopCard() - Size: " << size());
    Check3(size() > 0);
    Check3(operator[](size() - 1));

    Widget& card(getTopCard());
    cards.pop_back();

    TRACE5("IPile::removeTopCard() - Card " << card << " -> new size: " << size());

    if (style > NORMAL)
        resize(size() - 1, NORMAL);
    return card;
}

//-----------------------------------------------------------------------------
/// Flips the topmost card of the pile
//-----------------------------------------------------------------------------
void IPile::flipTopCard() {
    Check3(size() > 0);

    getTopCard().flip();
}

//-----------------------------------------------------------------------------
/// Sets the top card of the pile visible as indicated
/// \param visible Flag if cardface should be shown or back
//-----------------------------------------------------------------------------
void IPile::showTopCardFace(bool visible) {
    Check3(size() > 0);

    getTopCard().showFace(visible);
}

//-----------------------------------------------------------------------------
/// Method to append a card in compressed style without changing the size of
/// other cards
/// \param card: Card to append
/// \note Usefull when a bunch of cards is added to the pile
//-----------------------------------------------------------------------------
void IPile::insertCardFast(Widget& card, unsigned int offset) {
    TRACE3("IPile::insertCardFast(Widget&, unsigned int) " << offset)
    cards.insert(begin() + offset, &card);
    Check3(operator[](size() - 1));

    if (showOpt < DONT_CHANGE)
        card.showFace((bool)showOpt);
    resize(size() - 1, style);
}

//-----------------------------------------------------------------------------
/// Method to remove the first card from the pile without actualising the size
/// of the other cards in the pile
/// \param offset Offset of card to remove
/// \returns Widget& Removed card
/// \note Usefull when a bunch of cards is removed
//-----------------------------------------------------------------------------
Widget& IPile::removeCardFast(unsigned int offset) {
    std::vector<Widget*>::iterator i(begin() + offset);
    Widget& card(**i);
    cards.erase(i);
    return card;
}

//-----------------------------------------------------------------------------
/// Adds various cards to the pile
/// \param visible Flag if cardface should be shown or back
//-----------------------------------------------------------------------------
void IPile::setTopCards(const std::vector<Widget*>& staple) {
    std::vector<Widget*>::const_iterator i;

    for (i = staple.begin(); i != staple.end(); ++i) {
        Check3(*i);
        insertCardFast(**i, size());
    }
    resize(size() - 1, NORMAL);
}

//-----------------------------------------------------------------------------
/// Adds various cards to the pile
/// \param visible Flag if cardface should be shown or back
//-----------------------------------------------------------------------------
void IPile::setTopCards(const std::vector<Widget*>& staple, bool visible) {
    std::vector<Widget*>::const_iterator i;

    for (i = staple.begin(); i != staple.end(); ++i) {
        Check3(*i);
        (*i)->showFace(visible);
        insertCardFast(**i, size());
    }
    resize(size() - 1, NORMAL);
}

//-----------------------------------------------------------------------------
/// Removes all cards from pile
//-----------------------------------------------------------------------------
void IPile::clear() {
    while (size())
        removeCardFast(0);
}

//-----------------------------------------------------------------------------
/// Returns the card with the passed ID
/// \param id ID of card to return
/// \returns Widget* Pointer to card with passed ID (or NULL)
//-----------------------------------------------------------------------------
Widget* IPile::get(unsigned int id) const {
    std::vector<Widget*>::const_iterator i;

    for (i = begin(); i != end(); ++i) {
        Check3(*i);
        if ((*i)->id() == id)
            return *i;
    }
    return NULL;
}

//-----------------------------------------------------------------------------
/// Inserts a card into the pile
/// \param card Card to insert
/// \param pos Position of new card
/// \returns unsigned int Position where card was inserted
//-----------------------------------------------------------------------------
unsigned int IPile::insert(Widget& card, unsigned int pos) {
    TRACE5("IPile::insert(Widget, unsigned int&) - Card " << card << " at " << pos);
    Check3(pos <= size());

    if (showOpt < DONT_CHANGE)
        card.showFace((bool)showOpt);

    std::vector<Widget*>::iterator i(cards.insert(begin() + pos, &card));

    if (style > NORMAL) // Cards to display compressed?
        resize((pos == (size() - 1)) ? pos - 1 : pos, style);

    return i - begin();
}

//-----------------------------------------------------------------------------
/// Inserts a card into the pile; sorted by the passed function
/// \param card Card to insert
/// \param fnSort Function, how to sort
/// \returns unsigned int Position where card was inserted
//-----------------------------------------------------------------------------
unsigned int IPile::insertSorted(Widget& card, CMPFUNC fnSort) {
    TRACE5("IPile::insertSorted(Widget&, CMPFUNC) - Card " << card);
    return insert(card, (upper_bound(begin(), end(), &card, fnSort) - begin()));
}

//-----------------------------------------------------------------------------
/// Removes the passed card from the collection
/// \param card Card to remove
//-----------------------------------------------------------------------------
Widget& IPile::remove(Widget& card) {
    TRACE8("IPile::remove(Widget&) - " << card);
    Check3(size() > 0);

    // Search for card and remove it
    std::vector<Widget*>::iterator i(std::find(begin(), end(), &card));
    Check3(i != end());
    i = cards.erase(i);

    // Check if we have to resize a card
    if (style > NORMAL)
        // If last card was removed: Resize new last card (if any)
        (i == end()) ? resize(i - begin() - 1, NORMAL) : resize(card, NORMAL);

    return card;
}

//-----------------------------------------------------------------------------
/// Removes the card at the passed position from the collection
/// \param card Card (position) to remove
//-----------------------------------------------------------------------------
Widget& IPile::remove(unsigned int pos) {
    Check1(size() > pos);
    TRACE8("IPile::remove(unsigned int) - Card at pos " << pos << " (" << *operator[](pos) << ')');

    // Remove card on passed position
    std::vector<Widget*>::iterator i(begin() + pos);
    Widget* pTemp(*i);
    Check3(pTemp);
    i = cards.erase(i);

    // Check if we have to resize a card
    if (style > NORMAL)
        // If last card was removed: Resize new last card (if any)
        (i == end()) ? resize(size() - 1, NORMAL) : resize(*pTemp, NORMAL);

    return *pTemp;
}

//-----------------------------------------------------------------------------
/// Changes the drawing-style of the collection
/// \param s New style
//-----------------------------------------------------------------------------
void IPile::setStyle(PileStyle s) {
    TRACE5("IPile::setStyle(PileStyle) - Size = " << size());

    if (s == style)
        return;

    style = s;
    for (int i(0); i < static_cast<int>(size() - 1); ++i) {
        Check3(operator[](i));
        resize(i, style);
    }
    if (size())
        resize(size() - 1, NORMAL);
}

//-----------------------------------------------------------------------------
/// Sorts the cards in the pile with regard of the colour
/// \param a Card to compare
/// \param b Card to compare
/// \returns bool True, if a < b
//-----------------------------------------------------------------------------
bool IPile::compCards(const Widget* a, const Widget* b) {
    Check3(a);
    Check3(b);
    TRACE9("IPile::compCards(const Widget*, const Widget*) - " << *a << " < " << *b << " = "
           << ((a->colour() == b->colour()) ? a->number() < b->number() : a->colour() < b->colour()));
    return ((a->colour() == b->colour()) ? a->number() < b->number() : a->colour() < b->colour());
}

//-----------------------------------------------------------------------------
/// Sorts the cards in the pile without regard of the colour
/// \param a Card to compare
/// \param b Card to compare
/// \returns bool True, if a < b
//-----------------------------------------------------------------------------
bool IPile::compCardsByNr(const Widget* a, const Widget* b) {
    Check3(a);
    Check3(b);
    TRACE9("IPile::compCardsByNr(const Widget*, const Widget*) - " << a->number() << " < " << b->number()
	   << " == " << (a->number() < b->number()));
    return a->number() < b->number();
}

//-----------------------------------------------------------------------------
/// Sorts the cards in the pile without regard of the colour
/// \param a Card to compare
/// \param b Card to compare
/// \returns bool True, if a < b
//-----------------------------------------------------------------------------
bool IPile::compCardsByID(const Widget* a, const Widget* b) {
    Check3(a);
    Check3(b);
    return a->id() < b->id();
}

//-----------------------------------------------------------------------------
/// Sorts the cards in the pile according the past function
//-----------------------------------------------------------------------------
void IPile::sort(CMPFUNC fnSort) { std::sort(begin(), end(), fnSort); }

//-----------------------------------------------------------------------------
/// Finds the first card being equal or bigger than the past one
/// \param nr Number of card (2, 3, 4, ... Ace) to search for
/// \returns int Position of card in pile (or -1, if none found)
/// \pre Cards must be sorted (as the search is binary)
//-----------------------------------------------------------------------------
int IPile::findFirstEqualOrBigger(Widget::NUMBERS nr) const {
    unsigned int first(0), last(size());
    unsigned int middle(0);

    TRACE8("IPile::findFirstEqualOrBigger(Widget::NUMBERS) - Searching for " << nr << " in " << size() << " cards");

    while (last > first) {
        middle = first + ((last - first) >> 1);

        TRACE5("IPile::findFirstEqualOrBigger(Widget::NUMBERS) - Data = [" << first << "-(" << middle << ")-" << last
	       << ") = " << *operator[](middle));

        Check3(operator[](first));
        Check3(operator[](middle));
        Check3(operator[](first)->number() <= operator[](middle)->number());
        Check3((last == size()) ? 1 : operator[](last) && (operator[](middle)->number() <= operator[](last)->number()));

        if (operator[](middle)->number() < nr)
            first = middle + 1;
        else
            last = middle;

        Check3(middle <= size());
        Check3(last <= size());
        Check3(first <= last);
        Check3(middle <= last);
    }

#if TRACELEVEL > 4
    TRACE("IPile::findFirstEqualOrBigger(Widget::NUMBERS) - End = [" << first << "-(" << middle << ")-" << last << ')');
    if (first < size())
        TRACE("\t-> " << *operator[](first))
    else
        TRACE("\t-> Not found");
#endif

    return ((first < size()) && ((operator[](first)->number() >= nr)) ? static_cast<int>(first) : -1);
}

//-----------------------------------------------------------------------------
/// Finds the first card being equal or bigger than the past one
/// \param nr Colour of card (club, diamond, ...) to search for
/// \returns int Position of card in pile (or -1, if none found)
/// \pre Cards must be sorted (as the search is binary)
//-----------------------------------------------------------------------------
int IPile::findFirstEqualOrBiggerColour(Widget::COLOURS col) const {
    unsigned int first(0), last(size());
    unsigned int middle(0);

    TRACE8("IPile::findFirstEqualOrBiggerColour(Widget::COLOURS) - Searching for " << col << " in " << size() << " cards");

    while ((last - first) > 0) {
        middle = first + ((last - first) >> 1);

        TRACE5("IPile::findFirstEqualOrBiggerColour(Widget::COLOURS) - Data = [" << first << "-(" << middle << ")-" << last
	       << ") = " << *operator[](middle));

        Check3(operator[](first));
        Check3(operator[](middle));
        Check3(operator[](first)->colour() <= operator[](middle)->colour());
        Check3((last == size()) ? 1 : operator[](last) && (operator[](middle)->colour() <= operator[](last)->colour()));

        if (operator[](middle)->colour() < col)
            first = middle + 1;
        else
            last = middle;

        Check3(middle <= size());
        Check3(last <= size());
        Check3(first <= last);
        Check3(middle <= last);
    }

#if TRACELEVEL > 4
    TRACE("IPile::findFirstEqualOrBigger(Widget::COLOURS) - End = [" << first << "-(" << middle << ")-" << last << ')');
    if (first < size())
        TRACE("\t-> " << *operator[](first))
    else
        TRACE("\t-> Not found");
#endif

    return ((first < size()) && ((operator[](first)->colour() >= col)) ? static_cast<int>(first) : -1);
}

//-----------------------------------------------------------------------------
/// Finds the last card having an equal number as the passed card
/// \param pos Card whose (equal) number has to be found
/// \returns int Position of card in pile
/// \pre Cards must be sorted
//-----------------------------------------------------------------------------
int IPile::findLastEqual(unsigned int pos) const {
    Check3(pos < size());

    Widget::NUMBERS nr(operator[](pos)->number());
    while (++pos < size()) {
        if (operator[](pos)->number() != nr)
            break;
    }

    TRACE5("IPile::findLastEqual(unsigned int) - Card " << *operator[](pos - 1) << " at position " << pos - 1);
    return pos - 1;
}

//-----------------------------------------------------------------------------
/// Finds the last card having an equal colour as the passed card
/// \param pos Card whose (equal) colour has to be found
/// \returns int Position of card in pile
/// \pre Cards must be sorted
//-----------------------------------------------------------------------------
int IPile::findLastEqualColour(unsigned int pos) const {
    Check3(pos < size());

    Widget::COLOURS col(operator[](pos)->colour());
    while (++pos < size()) {
        if (operator[](pos)->colour() != col)
            break;
    }

    TRACE5("IPile::findLastEqualColour(unsigned int) - Card " << *operator[](pos - 1) << " at position " << pos - 1);
    return pos - 1;
}

//-----------------------------------------------------------------------------
/// Finds the first card having an equal number as the passed card
/// \param pos Card whose (equal) number has to be found
/// \returns int Position of card in pile
/// \pre Cards must be sorted
//-----------------------------------------------------------------------------
int IPile::findFirstEqual(unsigned int pos) const {
    TRACE5("IPile::findFirstEqual(unsigned int) - Checking card " << pos);
    Check1(pos < size());
    Check3(operator[](pos));

    Widget::NUMBERS nr(operator[](pos)->number());
    while (pos--) {
        TRACE9("IPile::findFirstEqual(unsigned int) - Checking card" << " at " << pos << " = " << *operator[](pos));
        if (operator[](pos)->number() != nr)
            break;
    }
    TRACE5("IPile::findFirstEqual(unsigned int) - Card at position " << (pos + 1) << " = " << *operator[](pos + 1));
    return pos + 1;
}

//-----------------------------------------------------------------------------
/// Finds the first card having an equal colour as the passed card
/// \param pos Card whose (equal) colour has to be found
/// \returns int Position of card in pile
/// \pre Cards must be sorted
//-----------------------------------------------------------------------------
int IPile::findFirstEqualColour(unsigned int pos) const {
    TRACE5("IPile::findFirstEqualColour(unsigned int) - Checking card " << pos);
    Check1(pos < size());
    Check3(operator[](pos));

    Widget::COLOURS col(operator[](pos)->colour());
    while (pos--) {
        TRACE9("IPile::findFirstEqualColour(unsigned int) - Checking card" << " at " << pos << " = " << *operator[](pos));
        if (operator[](pos)->colour() != col)
            break;
    }
    TRACE5("IPile::findFirstEqualColour(unsigned int) - Card at position " << (pos + 1) << " = " << *operator[](pos + 1));
    return pos + 1;
}

//-----------------------------------------------------------------------------
/// Makes the staple display either back or faces of the cards
/// \param show Option of how to display the cards
//-----------------------------------------------------------------------------
void IPile::setShowOption(ShowOpt show) {
    showOpt = show;

    if (showOpt < DONT_CHANGE) {
        std::vector<Widget*>::iterator i;

        for (i = begin(); i != end(); ++i) {
            Check3(*i);
            (*i)->showFace(showOpt);
        }
    }
}

//-----------------------------------------------------------------------------
/// Moves the card at pos source to pos dest
/// \param source Position of card to move
/// \param dest New position of card
//-----------------------------------------------------------------------------
void IPile::move(unsigned int dest, unsigned int source) {
    TRACE5("IPile::move(unsigned int, unsigned int) - Card from pos " << source << " to " << dest);
    insert(remove(source), dest);
}

//-----------------------------------------------------------------------------
/// Searches for the first card having the passed number
/// \param nr Number to search for
/// \param start Position of start of search
/// \returns int Offset of found card or -1
//-----------------------------------------------------------------------------
int IPile::find(Widget::NUMBERS nr, unsigned int start) const {
    for (; start < size(); ++start)
        if (operator[](start)->number() == nr)
            return start;

    return -1;
}

//-----------------------------------------------------------------------------
/// Searches for the first card having the passed colour
/// \param colour Colour to search for
/// \param start Position of start of search
/// \returns int Offset of found card or -1
//-----------------------------------------------------------------------------
int IPile::find(Widget::COLOURS colour, unsigned int start) const {
    for (; start < size(); ++start)
        if (operator[](start)->colour() == colour)
            return start;

    return -1;
}

//-----------------------------------------------------------------------------
/// Searches for the first card having the passed id
/// \param id ID of card to search for
/// \param id ID of card to search for
/// \returns int Offset of found card or -1
//-----------------------------------------------------------------------------
int IPile::find(unsigned int id, unsigned int start) const {
    Check1(start < size());
    for (const_iterator i(begin() + start); i != end(); ++i)
        if ((*i)->id() == id)
            return i - begin();
    return -1;
}

//-----------------------------------------------------------------------------
/// Resizing of a card in the pile
/// \param card Card to resize
/// \param PileStyle Style of pile
//-----------------------------------------------------------------------------
void IPile::resize(Widget& card, PileStyle) {
    TRACE1("IPile::resize - base");
    card.set_size_request(Images::WIDTH, Images::HEIGHT);
}

//-----------------------------------------------------------------------------
/// Checks if the passed pile contains a pair matching the passed card
/// \param card Card where to find a pair to
/// \param cmp Method to compare two cards. This method gets the two cards to
///        compare as input as must return an integer describing their
///        difference (0: Equal). A pair can have a difference of at most
///        [-2 - 2]
/// \param doubles True, if the same card (id) can be included more than once
/// \returns bool True, if the pile contains a matching pair
//-----------------------------------------------------------------------------
bool IPile::hasFittingPair(const Widget& card, CMPFUNC2 cmp, bool doubles) const {
    TRACE3("IPile::pileHasFittingPair(const Widget*, CMPFUNC2, bool) - " << card);

    unsigned int nrs(0);
    unsigned int bCols(0);

    std::vector<unsigned int> foundCards(4);
    if (!doubles)
        foundCards.push_back(card.id());

    for (const_iterator p(begin()); (p = getFittingCard(card, p, cmp)) != end(); ++p) {
        if (*p == &card) // Skip card if its the passed one
            continue;

        int diff(cmp(**p, card));
        if (diff) {
            diff = (diff < 0) ? (diff + 2) : (diff + 1);
            Check3(diff < 4);
            TRACE1("IPile::pileHasFittingPair(const Widget*, CMPFUNC2, bool) - " << **p << " diff: " << diff);
            if (!(bCols & (1 << diff))) {
                // The card is valid, if either a card bordering the one the
                // inspect and this one has been found. Note that for aces the
                // bordering card must be in the same direction as the card to
                // inspect (e.g. K-A-2 is not valid; only Q-K-A!)
                if ((card.number() == Widget::ACE) ? (bCols & (0x1 << (diff ^ 0x1)))
                                                   : (bCols & (diff ? (0x5 << (diff - 1)) : 0x2)))
                    return true;
                bCols |= (1 << diff);
            }
        }
        else {
            // Filter out doubles (if specififed)
            if (!doubles) {
                std::vector<unsigned int>::const_iterator i(foundCards.begin());
                do {
                    TRACE1("IPile::pileHasFittingPar(const Widget*, CMPFUNC2, bool)) - " << *i << '-' << (*p)->id());
                    if (*i == (*p)->id())
                        break;
                }
                while (++i != foundCards.end());
                if (i != foundCards.end())
                    continue;

                foundCards.push_back((*p)->id());
                TRACE1("IPile::pileHasFittingPair(const Widget*, CMPFUNC2, bool) - Adding non-double " << **p);
            }
            if (++nrs == 2)
                return true;
        }
    }
    TRACE7("Pile::pileHasFittingPair(const Widget*, CMPFUNC2, bool) - " << card << " matches " << nrs << '/' << std::hex << bCols
	   << std::dec);
    return false;
}

//-----------------------------------------------------------------------------
/// Returns a card fitting to the passed on
/// \param card Card where to find a fitting one to
/// \param start Position where to start the search
/// \param cmp Method to compare two cards. This method gets the two cards to
///        compare as input as must return an integer describing their
///        difference (0: Equal). A pair can have a difference of at most
///        [-2 - 2]
/// \returns IPile::iterator Position of matching card or pile.end ()
/// \pre start must be a valid iterator in pile
//-----------------------------------------------------------------------------
IPile::iterator IPile::getFittingCard(const Widget& card, const_iterator start, CMPFUNC2 cmp) const {
    TRACE9("IPile::getFittingCard(const Widget*, const_iterator, CMPFUNC2) - " << card);

    while (start != end()) {
        if ((static_cast<unsigned int>(cmp(card, **start) + 2)) < 5)
            break;
        ++start;
    }

#if TRACELEVEL > 8
    if (start == end()) {
        TRACE("IPile::getFittingCard(const Widget*, const_iterator, CMPFUNC2) - End");
    }
    else {
        TRACE("IPile::getFittingCard (const Widget*, const_iterator, CMPFUNC2) - "
              "Found "
              << **start);
    }
#endif
    return (const_cast<IPile*>(this)->begin() + (start - (const_iterator)begin()));
}

//----------------------------------------------------------------------------
/// Sorts a series of matching cards to the end of the pile
/// \param aPos Map holding the positions of the cards in the pile
/// \param aOrder Sorted order of the cards
/// \returns unsigned int Position of start of sorted serie
//----------------------------------------------------------------------------
unsigned int IPile::sortColourSerie(std::map<unsigned int, unsigned int>& aPos, std::vector<unsigned int>& aOrder) {
    unsigned int pos(1);
    for (std::vector<unsigned int>::reverse_iterator p(aOrder.rbegin()); p != aOrder.rend(); ++p) {
        std::map<unsigned int, unsigned int>::const_iterator v(aPos.find(*p));
        Check3(v != aPos.end());
        TRACE9("IPile::sortColourSeries(...) - Moving " << v->second << " to end " << ((*p < 2) ? *p : 0));
        Check3((p - aOrder.rbegin()) >= 0);
        // Move the card to the end of the staple; If it belongs before the
        // first card move it before the other cards.
        move(size() - pos, v->second);
        unsigned int oldOrder(*p);
        if (((p + 1) != aOrder.rend()) && (*(p + 1) < oldOrder))
            ++pos;
    }
    TRACE9("IPile::sortColourSeries(...) - Moved to pos " << size() - aPos.size());
    return size() - aPos.size();
}

//-----------------------------------------------------------------------------
/// Helper-function to correct the found matching cards
/// \param elem Element to remove from the found ones
/// \param aPos Map holding the positions of the cards in the pile
/// \param aOrder Sorted order of the cards
//-----------------------------------------------------------------------------
void IPile::deleteElement(unsigned int elem, std::map<unsigned int, unsigned int>& aPos, std::vector<unsigned int>& aOrder) {
    Check3(aPos.find(elem) != aPos.end());
    Check3(std::find(aOrder.begin(), aOrder.end(), elem) != aOrder.end());
    aOrder.erase(std::find(aOrder.begin(), aOrder.end(), elem));
    aPos.erase(aPos.find(elem));
}

//----------------------------------------------------------------------------
/// Gets a series of matching cards
/// \param card Card to compare
/// \param aPos Map holding the positions of the cards in the pile
/// \param aOrder Sorted order of the cards
/// \param cmp Method to compare two cards. This method gets the two cards to
///        compare as input as must return an integer describing their
///        difference (0: Equal). A pair can have a difference of at most
///        [-2 - 2]
/// \returns unsigned int The number of matching cards in a row
//----------------------------------------------------------------------------
unsigned int IPile::getSeries(Widget& card, std::map<unsigned int, unsigned int>& aPos, std::vector<unsigned int>& aOrder,
                              CMPFUNC2 cmp, bool doubles) {
    TRACE1("IPile::getSeries(...) for " << card);
    unsigned int nrs(0);
    unsigned int bCols(0x4);

    std::vector<unsigned int> foundCards(4);
    unsigned int cDoubles(0);
    if (!doubles)
        foundCards.push_back(card.id());

    for (const_iterator p(begin()); ((p = getFittingCard(card, p, cmp)) != end()); ++p) {
        // The card itself goes always in the middle (the 3rd position)
        if (*p == &card) {
            aPos[2] = p - begin();
            aOrder.push_back(2);
            ++nrs;
        }
        else {
            int diff(cmp(**p, card));
            TRACE1("IPile::getSeries(...) - " << **p << " differs " << diff);
            Check3(static_cast<unsigned int>(diff + 2) < 5);
            if (diff) {
                diff += 2;
                Check3(diff <= 4);
                if (!(bCols & (1 << diff))) {
                    Check3(aPos.find(diff) == aPos.end());
                    bCols |= (1 << diff);
                    aPos[diff] = p - begin();
                    aOrder.push_back(diff);
                    if (aPos.size() == 7)
                        break;
                }
            }
            else {
                // Filter out doubles (if specififed)
                if (!doubles) {
                    std::vector<unsigned int>::const_iterator i(foundCards.begin());
                    do {
                        TRACE1("IPile::getSeries(...) - " << *i << '-' << (*p)->id());
                        if (*i == (*p)->id())
                            break;
                    }
                    while (++i != foundCards.end());
                    if (i != foundCards.end()) {
                        ++cDoubles;
                        continue;
                    }

                    foundCards.push_back((*p)->id());
                    TRACE1("IPile::getSeries(...) - Adding non-double " << **p);

                    if (cDoubles)
                        move(p - begin() - cDoubles, p - begin());
                }
                ++nrs;
            }
        }
    }

    // Check if the series of colors is a valid one
    TRACE1("IPile::getSeries(...) - Serie: " << std::hex << bCols << std::dec);
    Check3(bCols & 0x4);

    // Delete cards having no direct access to the analysed one
    if ((bCols & 0x3) == 0x1) {
        Check3(aPos.find(1) == aPos.end());
        deleteElement(0, aPos, aOrder);
        bCols &= ~0x1;
    }
    if ((bCols & 0x18) == 0x10) {
        Check3(aPos.find(3) == aPos.end());
        deleteElement(4, aPos, aOrder);
        bCols &= ~0x10;
    }

    // Special handling of series of colours for an ace, to avoid the problem
    // with 3-K-A of one colour.
    if (card.number() == Widget::ACE) {
        if ((bCols & 0xa) == 0xa) {
            if ((bCols & 0x18) == 0x18) {
                if (aPos.find(0) != aPos.end())
                    deleteElement(0, aPos, aOrder);
                if (aPos.find(1) != aPos.end())
                    deleteElement(1, aPos, aOrder);
            }
            else {
                if (aPos.find(3) != aPos.end())
                    deleteElement(3, aPos, aOrder);
                if (aPos.find(4) != aPos.end())
                    deleteElement(4, aPos, aOrder);
            }
        }
    }

    return nrs;
}

//----------------------------------------------------------------------------
/// Find the worst (lowest) card in the pile
/// \return unsigned int Position of the lowest card in the pile
//----------------------------------------------------------------------------
unsigned int IPile::findLowestCard() const {
    TRACE9("IPile::findLowestCard() const - Analysing " << size() << " cards");
    Check3(size());

    unsigned int pos(0);
    for (IPile::const_iterator i(begin() + 1); i != end(); ++i)
        if ((*i)->number() < operator[](pos)->number())
            pos = i - begin();

    TRACE8("IPile::findLowestCard() const - Lowest card: " << *operator[](pos) << " at " << pos);
    return pos;
}

//----------------------------------------------------------------------------
/// Find the worst (lowest) card in the pile
/// \param excludeColour Special colour, which is not included in the search
/// \return unsigned int Position of the lowest card in the pile
//----------------------------------------------------------------------------
unsigned int IPile::findLowestCard(Widget::COLOURS excludeColour) const {
    TRACE9("IPile::findLowestCard(Widget::COLOURS) const - Analysing " << size() << " cards")
    Check3(size());

    unsigned int pos(0);
    for (IPile::const_iterator i(begin() + 1); i != end(); ++i)
        if (((*i)->colour() != excludeColour) && ((*i)->number() < operator[](pos)->number()))
            pos = i - begin();

    TRACE8("IPile::findLowestCard(Widget::COLOURS) const - Lowest card: " << *operator[](pos) << " at " << pos);
    return pos;
}

//----------------------------------------------------------------------------
/// Finds the last card with the passed colour in the pile.
/// \param col Colour to search for
/// \return int Position of found card; -1 if there's no bigger card
/// \pre The pile must be sorted by colour
//----------------------------------------------------------------------------
int IPile::findLastEqualOrBiggerColour(Widget::COLOURS col) const {
    TRACE9("IPile::findLastEqualOrBiggerColour(Widget::COLOURS)");
    int pos(findFirstEqualOrBiggerColour(col));
    return ((pos == -1) ? -1 : ((operator[](pos)->colour() == col) ? findLastEqualColour(pos) : -1));
}

//-----------------------------------------------------------------------------
/// Gets the size of all its children
/// \param width Width of all its children added up
/// \param height Height of all its children added up
/// \remarks To be implemented by derived classes
//-----------------------------------------------------------------------------
void IPile::getSize(int& width, int& height) { width = height = -1; }

//-----------------------------------------------------------------------------
/// Moves cards from another pile
/// \param posDest Position where to move card
/// \param src Source pile
/// \param start First card to move
/// \param end Last card to move; -1: Move til end
//-----------------------------------------------------------------------------
void IPile::getCards(unsigned int posDest, IPile& src, unsigned int start, int end) {
    TRACE3("IPile::getCards(unsigned int, IPile&, unsigned int, int) - moving "
           "from pos "
           << start << " to " << end);
    Check3(src.size());
    Check3(start < src.size());

    if (end == -1)
        end = src.size() - 1;
    Check1(end < static_cast<int>(src.size()));
    Check1(static_cast<int>(start) <= end);

    if (posDest && (posDest == size())) {
        TRACE1("IPile::getCards(uint, uint, end) - resizing");
        resize(size() - 1, style);
    }

    while ((unsigned int)end-- > start) {
        Widget& card(src.removeCardFast(start));
        if (style != src.style) {
            if (style == TOTALLY_COMPRESSED)
                card.hide();
            else
                card.show();
        }
        insertCardFast(card, posDest++);
    }
    insert(src.remove(start), posDest);
}

/// Implementation of the getCompressedSize() method for Card::VBox
template <> unsigned int Pile<Card::VBox>::getCompressedSize(PileStyle s) {
    int height[(int)LAST] = {(int)Images::HEIGHT, 15, 7, 1};
    return height[s];
}

/// Implementation of the getSize() method for Card::VBox
template <> void Pile<Card::VBox>::getSize(int& width, int& height) {
    if (size()) {
        height = Images::HEIGHT;
        width = Images::WIDTH;
        if (size() > 1)
            height += getCompressedSize() * (size() - 1);
    }
    else
        IPile::getSize(width, height);
}

/// Implementation of the resize() methods for Card::VBox
template <> void Pile<Card::VBox>::resize(unsigned int pos, PileStyle s) { IPile::resize(pos, s); }
template <> void Pile<Card::VBox>::resize(Card::Widget& card, PileStyle s) {
    if (s == TOTALLY_COMPRESSED)
        card.hide();
    else {
        TRACE1("VPile::resize " << card << '/' << getCompressedSize(s));
        card.set_size_request(-1, getCompressedSize(s));
        if (style == TOTALLY_COMPRESSED)
            card.show();
    }
}

/// Implementation of the getCompressionRate() method for Card::HBox
template <> unsigned int HPile::getCompressedSize(PileStyle s) {
    int width[(int)LAST] = {(int)Images::WIDTH, 18, 7, 1};
    return width[s];
}

/// Implementation of the getSize() method for Card::HBox
template <> void HPile::getSize(int& width, int& height) {
    if (size()) {
        height = Images::HEIGHT;
        width = Images::WIDTH;
        if (size() > 1)
            width += getCompressedSize() * (size() - 1);
    }
    else
        IPile::getSize(width, height);
}

/// Implementation of the resize() methods for Card::HBox
template <> void HPile::resize(unsigned int pos, PileStyle s) { IPile::resize(pos, s); }
template <> void HPile::resize(Card::Widget& card, PileStyle s) {
    TRACE1("HPile::resize " << getCompressedSize(s));
    if (s == TOTALLY_COMPRESSED)
        card.hide();
    else {
        card.set_size_request(getCompressedSize(s), -1);
        if (style == TOTALLY_COMPRESSED)
            card.show();
    }
}

} // namespace Card
