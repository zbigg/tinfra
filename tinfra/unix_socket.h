//
// Copyright (c) 2010-2011, Zbigniew Zagorski
// This software licensed under terms described in LICENSE.txt
//

#ifndef tinfra_unix_socket_h_included
#define tinfra_unix_socket_h_included

#include "tinfra/socket.h" // we implement this interface
#include "tinfra/tstring.h"

namespace tinfra {
class unix_client_socket: public client_stream_socket
{
public:
    unix_client_socket(handle_type h);
    unix_client_socket(tstring const& path);

    ~unix_client_socket();
};


class unix_server_socket: public socket {
    enum create_policy {
        DELETE_IF_EXISTS,
        FAIL_IF_EXISTS
    }
    unix_server_socket(handle_type h);
    unix_server_socket(tstring const& path, creation_policy policy = FAIL_IF_EXISTS);
    ~unix_server_socket();

    /// Accepts new connections.
    /// 
    /// Accepts new connection and create client socket object
    /// By default this operation is blocking, with non-blocking
    /// socket, this operation may return non-initialized pointer.
    std::auto_ptr<unix_client_socket> accept(std::string& address);
};

} // end namespace tinfra

#endif // tinfra_unix_socket_h_included
