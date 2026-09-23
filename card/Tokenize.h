#ifndef CARD_TOKENIZE_H
#define CARD_TOKENIZE_H

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

#include <algorithm>
#include <cctype>
#include <string>
#include <string_view>
#include <utility>
#include <vector>

namespace Card {

/**Class to split a string into sub-nodes (replacing the removed YGP::Tokenize).

   The sub-strings are extracted from the character behind the last found
   sub-string to a passed separator (or to the end of the string). The
   separator is not included in the result.

   If no further sub-string is available an empty string is returned. That also
   means that two separators behind each other are silently skipped.
*/
class Tokenize {
  public:
    explicit Tokenize(std::string text) : text_(std::move(text)), node_(text_) {}

    /// Returns the next sub-string up to (but excluding) the passed separator
    std::string getNextNode(char split) {
        pos_ = text_.find_first_not_of(split, pos_);
        if (pos_ == std::string::npos) {
            pos_ = text_.size();
            node_.clear();
            return node_;
        }

        const auto end(std::min(text_.find(split, pos_), text_.size()));
        node_ = text_.substr(pos_, end - pos_);
        pos_ = end;
        return node_;
    }

    /// Returns the current sub-string (or the whole string, if splitting has
    /// not started yet)
    const std::string& getActNode() const { return node_; }

  private:
    std::string text_;
    std::string node_;
    std::string::size_type pos_{0};
};

/// Splits the passed text into its alphanumeric words; every other character
/// (whitespace, punctuation, ...) is treated as separator
inline std::vector<std::string> splitWords(std::string_view text) {
    std::vector<std::string> words;
    auto isSeparator([](unsigned char c) { return !std::isalnum(c); });
    for (auto start(text.begin()); start != text.end();) {
        start = std::find_if_not(start, text.end(), isSeparator);
        auto end(std::find_if(start, text.end(), isSeparator));
        if (start != end)
            words.emplace_back(start, end);
        start = end;
    }
    return words;
}

} // namespace Card

#endif
