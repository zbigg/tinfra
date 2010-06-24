#ifndef tinfra_tcp_socket_h_included
#define tinfra_tcp_socket_h_included

#include "socket.h"
#include "tstring.h"

#include <memory>

namespace tinfra {

class tcp_client_socket: public client_stream_socket
{
public:
    tcp_client_socket(handle_type h);
    tcp_client_socket(tstring const& address, int port);
    ~tcp_client_socket();
};

class tcp_server_socket: public socket {
public:
    tcp_server_socket(handle_type h);
    tcp_server_socket(tstring const& address, int port);
    ~tcp_server_socket();

    /// Accepts new connections.
    /// 
    /// Accepts new connection and create client socket object
    /// By default this operation is blocking, with non-blocking
    /// socket, this operation may return non-initialized pointer.
    std::auto_ptr<tcp_client_socket> accept(std::string& address);
};

} // end namespace tinfra

#endif // tinfra_tcp_socket_h_included
