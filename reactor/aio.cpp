//
// Copyright (C) Zbigniew Zagorski <z.zagorski@gmail.com>,
// licensed to the public under the terms of the GNU GPL (>= 2)
// see the file COPYING for details
// I.e., do what you like, but keep copyright and there's NO WARRANTY.
//

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
#include "tinfra/regexp.h"
#include "tinfra/fmt.h"

#include "array.h"

// used by make_nonblocking
// TODO: remove
#include <unistd.h>
#include <fcntl.h>
#include <sys/socket.h>

namespace tinfra {
namespace aio {


//
// ChannelContainer
//

struct ChannelEntry {
    Channel   channel;
        
    int       flags;
    Listener* listener;
    
    bool      removed;
};

typedef std::vector<ChannelEntry> ChannelsEntryList;

class ChannelContainer {
public:
    void    remove(Channel c);
    
    void    add(Channel c, Listener*l, int flags);
    
    ChannelEntry* get(Channel c);

    void markAllClosed();
    
    void cleanup();

    ChannelsEntryList entries;
};

void ChannelContainer::remove(Channel c) 
{
    ChannelEntry* e = get(c);
    if( e ) 
        e->removed = true;
}

void ChannelContainer::add(Channel c, Listener*l, int flags) 
{
    ChannelEntry d = { c, flags, l, false };
    entries.push_back(d);
}

ChannelEntry* ChannelContainer::get(Channel c) 
{    
    for( ChannelsEntryList::iterator i = entries.begin(); i != entries.end(); ++i ) {
        if( i->channel == c ) 
            return & (*i);
    }
    return 0;
}

void ChannelContainer::markAllClosed()
{
    for(ChannelsEntryList::iterator i = entries.begin(); i != entries.end(); ++i ) {
        i->removed = true;
    }
}
    
void ChannelContainer::cleanup() {
    for(ChannelsEntryList::iterator i = entries.begin(); i != entries.end();  ) {
        if( i->removed ) {
            delete i->channel;
            i = entries.erase(i);
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
    ~PollDispatcher()
    {
        channels.markAllClosed();
        channels.cleanup();
    }
    void step()
    {
        make_fds(channels.entries, pollfds);
        
        int r = poll(pollfds.begin(), pollfds.size(), timeout);
        
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
            
            ChannelEntry& ce = channels.entries[i];
            
            
            if( (pfd->revents & POLLERR) == POLLERR ) {
                ce.listener->failure(*this, ce.channel, 1);
                close(&ce);
                continue;
            }
            
            if( (pfd->revents & POLLHUP) == POLLHUP ) {
                ce.listener->failure(*this, ce.channel, 0);
                close(&ce);
                continue;
            }
            
            try {
                if( (pfd->revents & POLLIN  ) == POLLIN ) {
                    ce.listener->event(*this, ce.channel, READ);
                }
                if( (pfd->revents & POLLOUT ) == POLLOUT) {
                    ce.listener->event(*this, ce.channel, WRITE);
                }
            } catch( std::exception& e) {
                close(&ce);
                
            } catch(...) {
                close(&ce);
            }
        }
        
        channels.cleanup();
    }
    
    virtual Channel create(int type, std::string const& address, Listener* listener, int initial_flags)
    {
        static tinfra::regexp parse_ip_address("([^:]*):([0-9]+)");        
        Channel channel;
        std::string host = "";
        int port = 0; 
        if( !(tinfra::scanner(parse_ip_address, address.c_str()) % host % port ) )
            throw std::logic_error(tinfra::fmt("malformed network address: %s") % address);
        
        switch( type ) {
        case CLIENT:
            channel = tinfra::io::socket::open_client_socket(host.c_str(), port);
            break;
        case SERVICE:
            channel = tinfra::io::socket::open_server_socket(host.c_str(), port);
            break;
        default:
            return 0;
        }
        
        put(channel, listener, initial_flags);
        return channel;
    }
    
    virtual void put(Channel channel, Listener* listener, int initial_flags)
    {
        channels.add(channel, listener, initial_flags);
    }
    
    virtual void close(Channel c)
    {
        ChannelEntry* cd = channels.get(c);
        assert(cd != 0);
        close(cd);
    }
    
    virtual void wait(Channel c, int flags, bool enable)
    {
        ChannelEntry* cd = channels.get(c);
        assert(cd != 0);
        if( enable ) {
            cd->flags |=  flags;
        } else {
            cd->flags &=  ~flags;
        }        
    }
    
private:
    ChannelContainer channels;

    void close(ChannelEntry* cd)
    {
        if( cd->removed )
            return;
        cd->channel->close();
        cd->removed = true;
    }
    
    void make_fds(ChannelsEntryList const& channels, array<pollfd>& result)
    {    
        result.size(channels.size());
        
        int k = 0;
        for(ChannelsEntryList::const_iterator i = channels.begin(); i != channels.end(); ++i,k++ )
        {
            ChannelEntry const& ce = *i;
            result[k].fd = ce.channel->native();
            int flags =  ce.flags;
            int events = 0;            
            if( (flags & Dispatcher::READ) == Dispatcher::READ )
                events |= POLLIN;
            if( (flags & Dispatcher::WRITE) == Dispatcher::WRITE )
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

// jedit: :tabSize=8:indentSize=4:noTabs=true:mode=c++:
