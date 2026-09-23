#ifndef SCOREDLG_H
#define SCOREDLG_H

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

#include <YGP/ANumeric.h>

#include <XGP/XAttrLabel.h>
#include <XGP/XDialog.h>

namespace Gtk {
class Box;
class Label;
class Widget;
class Separator;
} // namespace Gtk

namespace Card {
class Player;
}

namespace Card {

// Class to display the score of the cardgames
class ScoreDlg : public XGP::XDialog {
  public:
    explicit ScoreDlg(const std::vector<Player*>& player);
    ~ScoreDlg() override;

    /// Creates a new score-dialogue; showing the passed players
    /// \param player Vector holding name of all players
    /// \returns ScoreDlg* Pointer to the created dialoge
    static ScoreDlg* create(const std::vector<Player*>& player) { return new ScoreDlg(player); }

    void update(const std::vector<Player*>& player);

    void addPoints(int aPoints[]);
    void addPoints(const std::vector<int>& aPoints);

    void getMaxPoints(int& points, unsigned int& player);
    void getMinPoints(int& points, unsigned int& player);

    static int LASTX;
    static int LASTY;

    static void display(ScoreDlg** widget);
    void display();

  private:
    // Prohibited manager functions
    ScoreDlg(const ScoreDlg& other) = delete;
    const ScoreDlg& operator=(const ScoreDlg& other) = delete;

    void okEvent() override;

    using PBox = std::unique_ptr<Gtk::Box>;
    using PLabel = std::unique_ptr<Gtk::Label>;
    using PSeparator = std::unique_ptr<Gtk::Separator>;

    PBox client;

    class column {
      public:
        column();
        ~column();

        void addEntry(int points);
        void setTitle(const Glib::ustring& title);

        int getPoints() const { return pSum->getAttribute(); }
        Gtk::Box& getBox() const { return *pBox; }

      private:
        using NumLabel = XGP::XAttributeLabel2<YGP::ANumeric>;
        using PNumLabel = std::unique_ptr<NumLabel>;

        PBox pBox;
        PLabel pTitle;
        PNumLabel pSum;
        PSeparator pSep;
        Gtk::Widget* pLastEntry; ///< Last entry-widget added, to insert the next
                                 ///< one after it
    };

    std::vector<std::unique_ptr<column>> aColumns;
};

} // namespace Card

#endif
