//
// Copyright (C) Zbigniew Zagorski <z.zagorski@gmail.com>,
// licensed to the public under the terms of the GNU GPL (>= 2)
// see the file COPYING for details
// I.e., do what you like, but keep copyright and there's NO WARRANTY.
//

#ifndef __tinfra__io__socket_h__
#define __tinfra__io__socket_h__

#include <string>

namespace tinfra {
namespace io {
    
class stream;

namespace socket {
    
stream* open_client_socket(char const* address, int port);
stream* open_server_socket(char const* listening_host, int port);
stream* accept_client_connection(stream* server_socket,std::string* peer_address = 0);

void set_blocking(intptr_t socket, bool blocking);

} } }

#endif
