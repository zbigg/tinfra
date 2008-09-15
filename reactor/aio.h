#ifndef __tinfra_aio_h__
#define __tinfra_aio_h__

#include <string>
#include <vector>
#include <map>
#include <memory>

#include "tinfra/io/stream.h"

namespace tinfra {
namespace aio {
    
class Channel;

class Dispatcher {
public:
    enum {
        READ = 1, 
        WRITE = 2
    };
    
    virtual void add_channel(Channel* c) = 0;
    virtual void remove_channel(Channel* c) = 0;
    virtual void listen_channel(Channel* c, int mask, bool enable) = 0;

    virtual void step() = 0;
    
    virtual ~Dispatcher() {}
};

std::auto_ptr<Dispatcher> createDispatcher();

class Channel {
public:
    virtual ~Channel() {}
        
    virtual int  file() = 0;
    
    virtual void close() = 0;
        
    virtual void failure(Dispatcher& r) = 0;
    
    virtual void hangup(Dispatcher& r) = 0;
    
    virtual void data_available(Dispatcher&) = 0;
    virtual void write_possible(Dispatcher&) = 0;
};

class StreamChannel: public Channel {
    tinfra::io::stream* stream;
    bool own;

public:    
    StreamChannel(tinfra::io::stream* stream, bool own=true);

    ~StreamChannel();
        
    tinfra::io::stream* get_stream() const { return stream; }
    
    virtual int file();
    
    virtual void close();
    
    virtual void failure(Dispatcher& r);
    
    virtual void hangup(Dispatcher& r);
    
    virtual void data_available(Dispatcher&)  { }
    
    virtual void write_possible(Dispatcher&) { }
};

class ListeningChannel: public StreamChannel 
{
public:
    ListeningChannel();
    ListeningChannel(int port);
    ListeningChannel(std::string const& address, int port);
    
    virtual void data_available(Dispatcher& r);
    
protected:
    virtual void on_accept(Dispatcher&, tinfra::io::stream* channel, std::string const& remote_peer) = 0;
};

class GenericDispatcher: public Dispatcher {
public:
    virtual void remove_channel(Channel* c);
    
    virtual void add_channel(Channel* c);
    
    virtual void listen_channel(Channel* c, int flags, bool enable);

protected:    
    void cleanup();

    typedef std::vector<Channel*> ChannelsList;

    ChannelsList channels;
    ChannelsList to_remove;
    std::map<Channel*,int> channel_props;
};

}}

#endif // __tinfra_aio_h__
