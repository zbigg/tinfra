
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
//
// ChannelContainer
//

struct ChannelEntry {
    Channel*  channel;
    int       flags;
    Listener* listener;
    bool      removed;
};
typedef std::vector<ChannelEntry> ChannelsEntryList;

class ChannelContainer {
public:
    void    remove(Channel* c);
    
    void    add(Channel* c, Listener*l, int flags);
    
    ChannelEntry* get(Channel* c);

    void cleanup();

    ChannelsEntryList entries;
};

void ChannelContainer::remove(Channel* c) 
{
    ChannelEntry* e = get(c);
    if( e ) 
        e->removed = true;
}

void ChannelContainer::add(Channel* c, Listener*l, int flags) 
{
    ChannelData d = { c, flags, l, false };
    entries.push_back(d);
}

ChannelEntry* ChannelContainer::get(Channel* c) 
{    
    for( ChannelsEntryList::const_iterator i = entries.begin(); i != entries.end(); ++i ) {
        if( i->channel == c ) 
            return i;
    }
    return 0;
}

void ChannelContainer::cleanup() {
    for(ChannelsEntryList::iterator i = entries.begin(); i != entries.end();  ) {
        if( i->removed ) {
            i = channels->erase(i);
        } else {
            ++i;
        }
    }
}

//
// PollDispatcher
//

#include <poll.h>

class PollDispatcher: public tinfra::aio::Dispatcher {
public:
    
    int timeout;
    array<pollfd> pollfds;

public:
    
    PollDispatcher(): timeout(-1) {}

    void step()
    {
        make_fds(channels.entries, pollfds);
        
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
            
            ChannelData& channel_data = channels.entries[i];
            
            
            if( (pfd->revents & POLLERR) == POLLERR ) {
                channel_data.listener->failure(*this, channel_data->channel, 1);
                continue;
            }
            
            if( (pfd->revents & POLLHUP) == POLLHUP ) {
                channel_data.listener->failure(*this, channel_data->channel, 0);
                continue;
            }
            
            try {
                if( (pfd->revents & POLLIN  ) == POLLIN ) {
                    channel_data.listener->event(*this, channel_data->channel, READ);
                }
                if( (pfd->revents & POLLOUT ) == POLLOUT) {
                    channel_data.listener->event(*this, channel_data->channel, WRITE);
                }
            } catch( std::exception& e) {
                close(&channel_data);
                
            } catch(...) {
                close(&channel_data);
            }
        }
        
        cleanup();
    }
    
    virtual Channel* create(int type, std::string const& address, Listener* l)
    {        
        Channel* r;
        std::string host = address;
        int port = 80;
        switch( type ) {
        case CLIENT:
            r = tinfra::io::socket::open_tcp_connection(host);
            break;
        case SERVICE:
            r = tinfra::io::socket::open_server_socket(host.c_str(), port);
            break;
        default:
            return 0;
        }
        
        channels.
    }
    
    virtual void close(Channel* c)
    {
        close(channels.get(c));
    }
    
    virtual void wait(Channel* c, Listener* listener, int flags, bool enable)
    {
        ChannelContainer::ChannelData* cd = channels.get(c);
        cd->listener = listener;
        
        if( enable ) {
            cd->flags |=  flags;
        } else {
            cd->flags &=  ~ flags;
        }        
    }
    
private:
    ChannelContainer channels;

    void close(ChannelContainer::ChannelData* cd)
    {
        cd->channel->close();
        delete cd->channel;
        cd->removed = true;
    }
    
    void make_fds(ChannelsEntryList const& channels, array<pollfd>& result)
    {    
        result.size(channels.size());
        
        int k = 0;
        for(ChannelsEntryList::const_iterator i = channels.begin(); i != channels.end(); ++i,k++ )
        {
            ChannelData const cd& = *i;
            result[k].fd = cd->native();
            int flags =  cd->flags;
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

std::auto_ptr<Dispatcher> create_network_dispatcher()
{
    return std::auto_ptr<Dispatcher>(new PollDispatcher());
}

} } // end namespace tinfra::aio
