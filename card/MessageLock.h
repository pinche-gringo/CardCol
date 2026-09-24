#ifndef CARD_MESSAGELOCK_H
#define CARD_MESSAGELOCK_H

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

#include <condition_variable>
#include <mutex>

namespace Card {

/**Token serialising the processing of received messages: Only one message is
   processed at a time.

   The thread receiving the messages takes the token before passing a message
   to the GUI, which returns it after (maybe delayed, e.g. after an animation)
   the message has been processed completely. Unlike a mutex it can therefore
   be released by another thread than the one which took it; releasing it
   several times is harmless.
*/
class MessageLock {
  public:
    MessageLock() = default;
    MessageLock(const MessageLock&) = delete;
    MessageLock& operator=(const MessageLock&) = delete;

    /// Takes the token; waits until it is available
    void lock() {
        std::unique_lock guard(mx);
        cv.wait(guard, [this] { return !locked; });
        locked = true;
    }
    /// Returns the token (if taken)
    void unlock() {
        {
            std::lock_guard guard(mx);
            locked = false;
        }
        cv.notify_one();
    }

  private:
    std::mutex mx;
    std::condition_variable cv;
    bool locked{false};
};

} // namespace Card

#endif
