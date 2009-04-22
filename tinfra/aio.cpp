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
#include <cassert>
#include <cstdio>

#include <iostream>

#include "tinfra/aio.h"

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
    stream*   channel;
        
    int       flags;
    listener* the_listener;
    
    bool      removed;
};

typedef std::vector<ChannelEntry> ChannelsEntryList;

class ChannelContainer {
public:
    void    remove(stream* c);
    
    void    add(stream* c, listener*l, int flags);
    
    ChannelEntry* get(stream* c);

    
    void cleanup();

    ChannelsEntryList entries;
};

void ChannelContainer::remove(stream* c) 
{
    ChannelEntry* e = get(c);
    if( e ) 
        e->removed = true;
}

void ChannelContainer::add(stream* c, listener*l, int flags) 
{
    ChannelEntry d = { c, flags, l, false };
    entries.push_back(d);
}

ChannelEntry* ChannelContainer::get(stream* c) 
{    
    for( ChannelsEntryList::iterator i = entries.begin(); i != entries.end(); ++i ) {
        if( i->channel == c ) 
            return & (*i);
    }
    return 0;
}
    
void ChannelContainer::cleanup() {
    for(ChannelsEntryList::iterator i = entries.begin(); i != entries.end();  ) {
        if( i->removed ) {
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

class PollDispatcher: public tinfra::aio::dispatcher {
public:
    
    int timeout;
    array<pollfd> pollfds;

public:
    
    PollDispatcher(): 
        timeout(-1) 
    {
    }
    
    ~PollDispatcher()
    {
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
            
            
            try {
                if( (pfd->revents & POLLIN  ) == POLLIN ) {
                    ce.the_listener->event(*this, ce.channel, READ);
                }
                if( (pfd->revents & POLLOUT ) == POLLOUT) {
                    ce.the_listener->event(*this, ce.channel, WRITE);
                }
            } catch( std::exception& e) {
                close(&ce);
                continue;
                
            } catch(...) {
                close(&ce);
                continue;
            }
            
            if( (pfd->revents & POLLERR) == POLLERR ) {
                ce.the_listener->failure(*this, ce.channel, 1);
                close(&ce);
                continue;
            }
            
            if( (pfd->revents & POLLHUP) == POLLHUP ) {
                ce.the_listener->failure(*this, ce.channel, 0);
                close(&ce);
                continue;
            }
            
            
        }
        
        channels.cleanup();
    }
    
    
    virtual void add(stream* channel, listener* listener, int initial_flags)
    {
        channels.add(channel, listener, initial_flags);
    }
    
    virtual void remove(stream* c)
    {
        ChannelEntry* cd = channels.get(c);
        assert(cd != 0);
        close(cd);
    }
    
    virtual void wait(stream* c, int flags, bool enable)
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
        cd->the_listener->removed(*this, cd->channel);
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
            if( (flags & dispatcher::READ) == dispatcher::READ )
                events |= POLLIN;
            if( (flags & dispatcher::WRITE) == dispatcher::WRITE )
                events |= POLLOUT;
            result[k].events = events;
            result[k].revents = 0;
        }
    }
};


std::auto_ptr<dispatcher> dispatcher::create()
{
    return std::auto_ptr<dispatcher>(new PollDispatcher());
}

//deprecated
std::auto_ptr<dispatcher> create_network_dispatcher()
{
    return dispatcher::create();
}

} } // end namespace tinfra::aio

// jedit: :tabSize=8:indentSize=4:noTabs=true:mode=c++:
