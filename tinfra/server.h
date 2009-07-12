//
// Copyright (C) Zbigniew Zagorski <z.zagorski@gmail.com>,
// licensed to the public under the terms of the GNU GPL (>= 2)
// see the file COPYING for details
// I.e., do what you like, but keep copyright and there's NO WARRANTY.
//

#ifndef tinfra_server_h_included
#define tinfra_server_h_included

#include <memory>
#include <string>
#include "tinfra/io/stream.h"

namespace tinfra {
namespace net {
    
typedef tinfra::io::stream stream;

class Server {
    

    std::auto_ptr<stream> server_socket_;
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

    virtual void onAccept(std::auto_ptr<stream> client, std::string const& peer_address) = 0;
};

} }
#endif
