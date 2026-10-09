#ifndef CARD_RANDOM_H
#define CARD_RANDOM_H

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

#include <cstdlib>
#include <random>

namespace Card {

/// Returns the seed the random engine (of the current thread) has been
/// initialised with: The value of the environment variable CARDCOL_SEED (if
/// set) or a random value.
/// \remarks Every random decision of the games is derived from this engine,
///     so setting CARDCOL_SEED replays a game exactly (as long as the
///     human player makes the same moves)
inline unsigned int& randomSeed() {
    thread_local unsigned int seed{[] {
        const char* env(std::getenv("CARDCOL_SEED"));
        return (env && *env) ? static_cast<unsigned int>(std::strtoul(env, nullptr, 0)) : std::random_device{}();
    }()};
    return seed;
}

inline std::mt19937& randomEngine() {
    thread_local std::mt19937 engine{randomSeed()};
    return engine;
}

/// Re-initialises the random engine (of the current thread) with the passed seed
/// \param seed Seed to use
inline void seedRandom(unsigned int seed) {
    randomSeed() = seed;
    randomEngine().seed(seed);
}

/// Returns a random number in the range [0, upper)
inline unsigned int randomNumber(unsigned int upper) {
    return std::uniform_int_distribution<unsigned int>(0, upper - 1)(randomEngine());
}

} // namespace Card

#endif
