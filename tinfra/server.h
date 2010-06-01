//
// Copyright (c) 2009, Zbigniew Zagorski
// This software licensed under terms described in LICENSE.txt
//

#ifndef tinfra_server_h_included
#define tinfra_server_h_included

#include "tcp_socket.h"
#include <memory>
#include <string>

namespace tinfra {
namespace net {
    
class Server {
    

    std::auto_ptr<tcp_server_socket> server_socket_;
    volatile bool stopped_;
    volatile bool stopping_;
    std::string   bound_address_;
    int           bound_port_;
public:
    Server();
    Server(const char* address, int port);
    virtual ~Server();
    void bind(const char* address, int port);
    void run();
    void stop();
    bool stopped() const { return stopped_; }
protected:

    virtual void onAccept(std::auto_ptr<tcp_client_socket> client, std::string const& peer_address) = 0;
};

} }
#endif
