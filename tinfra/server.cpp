#include "tinfra/io/socket.h"
#include "tinfra/server.h"

namespace tinfra { namespace net {
//
// Server implementation
//
using namespace tinfra::io::socket;
using namespace tinfra::io;

Server::Server()
    : stopped_(false)
{
}
Server::Server(const char* address, int port)
    : stopped_(false)
{
    bind(address, port);
}

void Server::bind(const char* address, int port)
{
    server_socket_ = std::auto_ptr<stream>(open_server_socket(address,port));
    if( address ) 
        bound_address_ = address;
    bound_port_ = port;
}

void Server::run()
{
    while( !stopped_ ) {
        std::auto_ptr<stream> client_socket = std::auto_ptr<stream>(accept_client_connection(server_socket_.get()));
        if( !stopped_ ) 
            onAccept(client_socket);
    }
    server_socket_->close();
}

void Server::stop()
{    
    stopped_ = true;
    
    try {
        std::string connect_address = bound_address_;
        if( bound_address_.size() == 0 )
            connect_address = "localhost";
        stream* f = open_client_socket(connect_address.c_str(), bound_port_);
        if( f ) 
            delete f;
    } catch( io_exception& wtf)  {
    }
}

} }
