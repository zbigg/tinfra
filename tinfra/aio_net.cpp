//
// Copyright (C) 2008 Zbigniew Zagorski <z.zagorski@gmail.com>,
// licensed to the public under the terms of the GNU GPL (>= 2)
// see the file COPYING for details
// I.e., do what you like, but keep copyright and there's NO WARRANTY.
//

#include <iostream>
#include <string>
#include <memory>

#include "tinfra/io/stream.h"
#include "tinfra/io/socket.h"

#include "aio_net.h"

namespace tinfra {
namespace aio {

using namespace tinfra::io::socket;
using std::string;
using std::auto_ptr;
        
static void initialize_async_socket(stream* socket_)
{
    int socket = socket_->native();
    
    set_blocking(socket, false);
} 

std::auto_ptr<stream>     create_service_stream(std::string const& host, int port)
{
    
    auto_ptr<stream> result(open_server_socket(host.c_str(), port) );
    
    initialize_async_socket(result.get());
    
    return result;
}

std::auto_ptr<stream>     create_client_stream(std::string const& host, int port)
{
    auto_ptr<stream> result(open_client_socket(host.c_str(), port) );
    
    initialize_async_socket(result.get());
    
    return result;
}

void acceptor_listener::event(Dispatcher& d, stream* listener_stream, int event)
{
    string client_address;
    
    auto_ptr<stream> client_conn(accept_client_connection(listener_stream, &client_address));
    
    initialize_async_socket(client_conn.get());
    
    accept_connection(d, client_conn, client_address);
}

} } // end namespace tinfra::aio

// jedit: :tabSize=8:indentSize=4:noTabs=true:mode=c++:
