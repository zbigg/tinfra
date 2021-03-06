//
// Copyright (c) 2009, Zbigniew Zagorski
// This software licensed under terms described in LICENSE.txt
//

#include "tinfra/platform.h"

#include "tinfra/server.h"
#include "tinfra/trace.h"

#include "tinfra/fmt.h"

#include <stdexcept>

namespace tinfra { namespace net {
//
// Server implementation
//

Server::Server()
    : stopped_(false), stopping_(false), bound_port_(0)
{
}
Server::Server(const char* address, int port)
    : stopped_(false), stopping_(false), bound_port_(0)
{
    bind(address, port);
}

Server::~Server()
{
    if( ! stopped_ ) {
        try {
            stop();
        } catch( std::exception&) {
            // TODO: tinfra::silent_failure(e);
        }
    }
}

void Server::bind(const char* address, int port)
{
    server_socket_.reset(new tcp_server_socket(address, port));
    if( address ) {
        bound_address_ = address;
    }
    bound_port_ = port;
}

void Server::run()
{
    // FIXME: stopped_ flag has write/read race (as detected by helgrind) zbigg/2010/11/19 
    while( !stopped_ ) {
        std::string peer_address;
        std::auto_ptr<tcp_client_socket> client_socket(server_socket_->accept(peer_address));
        TINFRA_GLOBAL_TRACE(fmt("server %s:%s: accepted new connection from %s (stopping=%s)") 
            %  bound_address_ % bound_port_ % peer_address % stopping_);
        if( !stopping_ ) 
            onAccept(client_socket, peer_address);
        else  {
            stopped_ = true;
        }
    }
    server_socket_.reset();
}

void Server::stop()
{
    if( stopped_ ) 
        return;
    
    std::string connect_address = bound_address_;
    if( bound_address_.size() == 0 )
        connect_address = "localhost";
    
    try {
        TINFRA_GLOBAL_TRACE(fmt("stopping server %s:%s") %  connect_address % bound_address_);
        stopping_ = true;
        {
            tcp_client_socket fake_client(connect_address, bound_port_);
        }
    } catch( std::exception& e)  {
        const std::string error_message = (fmt("unable to stop server %s:%s: %s") %  connect_address % bound_address_ % e.what()).str();
        throw std::runtime_error(error_message);
    }
}

} }
