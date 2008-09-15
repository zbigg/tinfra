
#include <vector>
#include <algorithm>
#include <stdexcept>
#include <list>
#include <map>

#include <iostream>

//#include "tinfra/aio.h"
#include "aio.h"

#include "tinfra/cmd.h"
#include "tinfra/io/stream.h"
#include "tinfra/io/socket.h"

#include "array.h"

// used by make_nonblocking
// TODO: remove
#include <unistd.h>
#include <fcntl.h>
#include <sys/socket.h>

namespace tinfra {
namespace aio {

//
// StreamChannel
//
    
StreamChannel::StreamChannel(tinfra::io::stream* stream, bool own): 
    stream(stream),
    own(own)
{
}

StreamChannel::~StreamChannel()
{
    if( own )
        delete stream;
}

int StreamChannel::file() { 
    return stream->native(); 
}

void StreamChannel::close() {
    stream->close(); 
}

void StreamChannel::failure(Dispatcher& r)
{
    r.remove_channel(this);
    close();
}
    
void StreamChannel::hangup(Dispatcher& r)
{
    r.remove_channel(this);
    close();
}
//
// ListeningChannel
//

ListeningChannel::ListeningChannel()
    : StreamChannel(0)
{
}
ListeningChannel::ListeningChannel(int port)
    : StreamChannel(tinfra::io::socket::open_server_socket(0, port))
{
}

ListeningChannel::ListeningChannel(std::string const& address, int port)
    : StreamChannel(tinfra::io::socket::open_server_socket(address.c_str(), port))
{
}

void ListeningChannel::data_available(Dispatcher& r) 
{
    std::string peer_address;
    
    tinfra::io::stream* new_socket = tinfra::io::socket::accept_client_connection(get_stream(), &peer_address);
    
    on_accept(r, new_socket, peer_address);
}

//
// GenericDispatcher
//

// TODO, move it to infra

static void set_buffers(int socket)
{
    int r = (1 << 15);
    if( setsockopt(socket, SOL_SOCKET, SO_RCVBUF, (void*)&r, sizeof(r)) ) {
        // TODO: it should be warning
        std::cerr << "unable to set SO_RCVBUF=" << r << " on socket " << socket << std::endl;
    }
    
    int s = (1 << 17);
    if( setsockopt(socket, SOL_SOCKET, SO_SNDBUF, (void*)&s, sizeof(s)) ) {
        // TODO: it should be warning
        std::cerr << "unable to set SO_SNFBUF=" << s << " on socket " << socket << std::endl;
    }
}

void GenericDispatcher::remove_channel(Channel* c) 
{
    to_remove.push_back(c);
}

void GenericDispatcher::add_channel(Channel* c) 
{
    channels.push_back(c);
    channel_props[c] = 0;
    
    tinfra::io::socket::set_blocking(c->file(), false);
    set_buffers(c->file());
}

void GenericDispatcher::listen_channel(Channel* c, int flags, bool enable) 
{    
    if( enable ) {
        channel_props[c] |=  flags;
    } else {
        channel_props[c] &= ~flags;
    }
}

void GenericDispatcher::cleanup() {
    for(ChannelsList::const_iterator i = to_remove.begin(); i != to_remove.end(); ++i ) {
        Channel* channel = *i;
        channel_props.erase(channel);
        
        ChannelsList::iterator ic = std::find(channels.begin(), channels.end(), channel);
        if( ic != channels.end() ) 
            channels.erase(ic);
        
        //throw std::logic_error("test exit");
    }
    
    to_remove.clear();
}

//
// PollDispatcher
//

#include <poll.h>

class PollDispatcher: public tinfra::aio::GenericDispatcher {
public:
    
    int timeout;
    array<pollfd> pollfds;

public:
    
    PollDispatcher(): timeout(-1) {}

    void step()
    {
        make_fds(channels, pollfds);
        
        int r = poll(pollfds.begin(), channels.size(), timeout);
        
        if( r == 0 ) {
            return;
        }
        if( r == -1 ) {
            perror("PollDispatcher::step: poll failed, retrying");
            return;
        }
        
        int i = 0;        
        for(pollfd* pfd = pollfds.begin(); pfd != pollfds.end(); ++pfd,++i ) {
            if( pfd->revents == 0 ) 
                continue;
            
            Channel* channel = channels[i];
            
            
            if( (pfd->revents & POLLERR) == POLLERR ) {
                channel->failure(*this);
                continue;
            }
            
            if( (pfd->revents & POLLHUP) == POLLHUP ) {
                channel->hangup(*this);
                continue;
            }
            
            try {
                if( (pfd->revents & POLLIN  ) == POLLIN ) {
                    channel->data_available(*this);
                }
                if( (pfd->revents & POLLOUT ) == POLLOUT) {
                    channel->write_possible(*this);
                }
            } catch( std::exception& e) {
                channel->close();
                remove_channel(channel);
                
            } catch(...) {
                channel->close();
                remove_channel(channel);
            }
        }
        
        cleanup();
    }
    
private:
    void make_fds(PollDispatcher::ChannelsList const& channels, array<pollfd>& result)
    {    
        result.size(channels.size());
        
        int k = 0;
        for(PollDispatcher::ChannelsList::const_iterator i = channels.begin(); i != channels.end(); ++i,k++ )
        {
            Channel* c = *i;
            result[k].fd = c->file();
            int props = channel_props[c];
            int events = 0;            
            if( (props & Dispatcher::READ) == Dispatcher::READ )
                events |= POLLIN;
            if( (props & Dispatcher::WRITE) == Dispatcher::WRITE )
                events |= POLLOUT;
            result[k].events = events;
            result[k].revents = 0;
        }
    }
};

std::auto_ptr<Dispatcher> createDispatcher()
{
    return std::auto_ptr<Dispatcher>(new PollDispatcher());
}

} } // end namespace tinfra::aio
