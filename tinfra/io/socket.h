//
// Copyright (C) Zbigniew Zagorski <z.zagorski@gmail.com>,
// licensed to the public under the terms of the GNU GPL (>= 2)
// see the file COPYING for details
// I.e., do what you like, but keep copyright and there's NO WARRANTY.
//

#ifndef tinfra_io_socket_h_included
#define tinfra_io_socket_h_included

#include <string>

namespace tinfra {
namespace io {
    
class stream;

namespace socket {
    
enum {
	BLOCK     = 0,
	NON_BLOCK = 1
};

/// Open a TCP client socket.
///
/// Supported flags are:
///    tinfra::io::socket::BLOCK     - open in blocking mode (default)
///    tinfra::io::socket::NON_BLOCK - open in non-blocking mode
///
/// In non-blocking mode socket will be in not-connected state and one must
/// use some dispatcing mechanism (select, poll, completion ports) to check
/// whether connection will occur.
///
/// NOTE: Name resolution CURRENTLY is always blocking.
///
/// @param address  target host address (ip address or name)
/// @param port     target port
/// @param flags    choose socket options

stream* open_client_socket(char const* address, int port, int flags = 0);

/// Open a TCP client socket.
///
/// The socket is opened in blocking mode.
///
/// @param    listening_host   (0  | "") means all interfaces, else address of interface to listen on
stream* open_server_socket(char const* listening_host, int port);

/// Accept TCP connection from client.
///
/// On non-blocking socket it might return NULL if there are no pending connections/
///
/// On blocking socket it will wait infinitely until new connection arrives.
///
/// @param server_socket  on which socket we accept connection
/// @param peer_address   OUT, client address, destination is filled if not NULL
stream* accept_client_connection(stream* server_socket,std::string* peer_address = 0);

void set_blocking(intptr_t socket, bool blocking);

} } }

#endif

// jedit: :tabSize=8:indentSize=4:noTabs=true:mode=c++:

