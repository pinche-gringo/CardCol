#ifndef CARD_MESSAGE_H
#define CARD_MESSAGE_H

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

#include <array>
#include <string>
#include <string_view>

#include <boost/asio/ip/tcp.hpp>
#include <boost/asio/read.hpp>
#include <boost/asio/write.hpp>

namespace Card {

/// Character terminating every message exchanged with the partners
inline constexpr char MESSAGE_END = '\0';

/// Sends the passed message (followed by its terminator)
/// \param socket Socket to write to
/// \param msg Message to send
/// \throw boost::system::system_error In case of an error
inline void sendMessage(boost::asio::ip::tcp::socket& socket, std::string_view msg) {
    const std::array buffers{boost::asio::buffer(msg), boost::asio::buffer(&MESSAGE_END, 1)};
    boost::asio::write(socket, buffers);
}

/// Receives the next message (without its terminator)
/// \param socket Socket to read from
/// \returns std::string Received message
/// \throw boost::system::system_error In case of an error (with the code
///     boost::asio::error::eof if the partner closed the connection)
/// \remarks The message is read byte-wise, so no data following it is consumed
///     and the socket can be handed over to another reader
inline std::string receiveMessage(boost::asio::ip::tcp::socket& socket) {
    std::string msg;
    for (char c; boost::asio::read(socket, boost::asio::buffer(&c, 1)), c != MESSAGE_END;)
        msg += c;
    return msg;
}

} // namespace Card

#endif
