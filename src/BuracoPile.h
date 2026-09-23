#ifndef BURACOPILE_H
#define BURACOPILE_H

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

#include <card/Pile.h>

/**Class for piles in the Buraco cardgame
 */
class BuracoPile : public Card::VPile {
  public:
    BuracoPile();
    ~BuracoPile() override;

    void setTopCard(Card::Widget& newCard) override;
    void setTopCard(Card::Widget& newCard, bool visible) { Card::VPile::setTopCard(newCard, visible); }

    unsigned int insert(Card::Widget& card, unsigned int pos) override;

    Card::Widget& remove(Card::Widget& card) override;
    Card::Widget& remove(Card::Widget& card, bool visible);
    Card::Widget& remove(unsigned int pos) override;
    Card::Widget& remove(unsigned int pos, bool visible);

    unsigned int getCardPoints() const;
    unsigned int getPotentialPoints() const { return status.points; }
    int getPoints() const { return ((size() == 7) ? status.points : ((status.points >= 1000) ? -1000 : 0)); }

    unsigned int getPosJoker() const { return status.posJoker; }
    unsigned int getPosFirst() const { return status.posFirst; }
    unsigned int getPosLast() const { return status.posLast; }

    bool getPosition4Card(const Card::Widget& card, unsigned int& pos, unsigned int& move) const;

  protected:
    bool isValid(const Card::Widget& card) const {
        unsigned int pos, move;
        return getPosition4Card(card, pos, move);
    }
    void analysePile();

  private:
    BuracoPile(const BuracoPile& other) = delete;
    const BuracoPile& operator=(const BuracoPile& other) = delete;

    enum { UNDEFINED, NUMBER, COLOUR };

    struct {
        unsigned int posFirst : 3;
        unsigned int posLast : 3;
        unsigned int posJoker : 3;
        unsigned int type : 2;
        unsigned int points : 11;
    } status;
};

#endif
