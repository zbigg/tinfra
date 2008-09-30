//
// Copyright (C) Zbigniew Zagorski <z.zagorski@gmail.com>,
// licensed to the public under the terms of the GNU GPL (>= 2)
// see the file COPYING for details
// I.e., do what you like, but keep copyright and there's NO WARRANTY.
//

#ifndef __tinfra_aio_h__
#define __tinfra_aio_h__

#include <string>
#include <vector>
#include <map>
#include <memory>

#include "tinfra/io/stream.h"

namespace tinfra {
namespace aio {
    
typedef tinfra::io::stream* Channel;

class Dispatcher;
    
class Listener {
public:
    virtual void event(Dispatcher& d, Channel c, int event) = 0;
    virtual void failure(Dispatcher& d, Channel c, int error) = 0;
    
    virtual ~Listener() {}
};

class Dispatcher {
public:
    enum {
        READ = 1, 
        WRITE = 2
    };
    enum {
        CLIENT = 0,
        SERVICE = 1
    };
    
    virtual Channel create(int type, std::string const& address, Listener* l, int initial_flags) = 0;
    
    virtual void put(Channel c, Listener* l, int initial_flags) = 0;
    
    virtual void close(Channel c) = 0;
    
    virtual void wait(Channel c, int mask, bool enable) = 0;

    virtual void step() = 0;
    
    virtual ~Dispatcher() {}
};

// TODO following network stuff it should in kinda aio_net.h or stg

std::auto_ptr<Dispatcher> create_network_dispatcher();

class Acceptor: public tinfra::aio::Listener {
public:
    virtual void event(tinfra::aio::Dispatcher& d, 
                       tinfra::aio::Channel listener_stream, 
                       int event);

    virtual void accept_connection(Dispatcher& dispatcher, 
                                   std::auto_ptr<tinfra::io::stream>& client_conn, 
                                   std::string client_address) = 0;
}; 
}}

#endif // __tinfra_aio_h__

// jedit: :tabSize=8:indentSize=4:noTabs=true:mode=c++:
