#ifndef __tinfra_aio_h__
#define __tinfra_aio_h__

#include <string>
#include <vector>
#include <map>
#include <memory>

#include "tinfra/io/stream.h"

namespace tinfra {
namespace aio {
    
struct Channel {
    tinfra::stream* stream;
};
class Dispatcher;
    
class Listener {
public:
    virtual void event(Dispatcher& d, Channel* c, int event) = 0;
    virtual void failure(Dispatcher& d, Channel* c, int error) = 0;
    
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
        SERVICE = 0
    }
    virtual Channel* create(int type, std::string const& address, Listener* l) = 0;
    
    virtual void close(Channel* c) = 0;
    
    virtual void wait(Channel* c, Listener* listener, int mask, bool enable) = 0;

    virtual void step() = 0;
    
    virtual ~Dispatcher() {}
};

std::auto_ptr<Dispatcher> create_network_dispatcher();



}}

#endif // __tinfra_aio_h__
