#include "tinfra/io/socket.h"
#include "tinfra/server.h"

#include "tinfra/fmt.h"
#include <iostream>
#include <stdexcept>

namespace tinfra { namespace net {
//
// Server implementation
//
using namespace tinfra::io::socket;
using namespace tinfra::io;

Server::Server()
    : stopping_(false), stopped_(false), bound_port_(0)
{
}
Server::Server(const char* address, int port)
    : stopping_(false), stopped_(false), bound_port_(0)
{
    bind(address, port);
}

Server::~Server()
{
    if( ! stopped_ ) {
        try {
            stop();
        } catch( std::exception& e) {
            // TODO: tinfra::silent_failure(e);
        }
    }
}

void Server::bind(const char* address, int port)
{
    server_socket_ = std::auto_ptr<stream>(open_server_socket(address,port));
    if( address ) {
        bound_address_ = address;
    }
    bound_port_ = port;
}

void Server::run()
{
    while( !stopped_ ) {
        std::string peer_address;
        std::auto_ptr<stream> client_socket(accept_client_connection(server_socket_.get(), &peer_address));
        if( !stopping_ ) 
            onAccept(client_socket, peer_address);
        else 
            stopped_ = true;
    }
    server_socket_->close();
}

void Server::stop()
{
    if( stopped_ ) 
        return;
    
    std::string connect_address = bound_address_;
    if( bound_address_.size() == 0 )
        connect_address = "localhost";
    
    try {
        
        //std::cerr << "atempting to stop SERVER" << std::endl;
        stopping_ = true;
        stream* f = open_client_socket(connect_address.c_str(), bound_port_);
        if( f ) {
            delete f;
            stopped_ = true;
        }
    } catch( std::exception& e)  {
        throw std::runtime_error(fmt("unable to stop server %s:%s: %s") %  connect_address % bound_address_ % e.what());
    }
    stopping_ = false;
}

} }
