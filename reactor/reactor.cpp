#include <vector>
#include <algorithm>
#include <stdexcept>
#include <list>
#include <poll.h>

#include <boost/shared_ptr.hpp>
#include "tinfra/cmd.h"
#include "tinfra/io/stream.h"
#include "tinfra/io/socket.h"

#include "array.h"

/// remove all 
template <typename C1, typename C2>
void remove(C1& victim, C2 const& to_remove)
{
    for(typename C2::const_iterator i = to_remove.begin(); i != to_remove.end(); ++i ) {
        typename C1::iterator iv = std::find(victim.begin(), victim.end(), *i);
        victim.erase(iv);
    }
}

struct IOChannel {
    typedef boost::shared_ptr<IOChannel> ptr;
    
    int channel;
    
    virtual void close() { ::close(channel); }
    virtual void failure(int error_code) {}
    virtual void data_available() {}
    virtual void write_possible() {}
};

struct IOContext {
    typedef std::vector<IOChannel::ptr> ChannelsList;
    
    ChannelsList channels;
    
    int timeout;
    
    void shutdown_channel(IOChannel::ptr c) {
        to_shutdown.push_back(c);
    }

    void cleanup() {
        for(ChannelsList::const_iterator i = to_shutdown.begin(); i != to_shutdown.end(); ++i ) {
            IOChannel::ptr channel = *i;
            channel->close();
            channels.erase(std::find(channels.begin(), channels.end(), channel));
        }
        
        to_shutdown.clear();
    }
private:
    ChannelsList to_shutdown;
};

struct ListeningIOChannel: public IOChannel 
{
    tinfra::io::stream* main_socket;
    
    ListeningIOChannel(int port)
        : main_socket(0)
    {
        channel = main_socket->native();
    }
    
    void listen(int port)
    {
        main_socket = tinfra::io::socket::open_server_socket(0, port);
        channel = main_socket->native();
    }
    
    void close()
    {
        if( !main_socket ) 
            return;
        main_socket->close();
        delete main_socket;
        main_socket = 0;
    }
    
    void data_available() 
    {
        int new_socket;
        {
            tinfra::io::stream* s = tinfra::io::socket::accept_client_connection(main_socket);
            new_socket = s->native();
            s->release();
            delete s;
        }
        onAccept(new_socket,"");
    }
    virtual void onAccept(int socket, std::string const& remote_peer) = 0;
};

void make_fds(IOContext::ChannelsList const& channels, array<pollfd>& result)
{    
    result.size(channels.size());
    
    int k =0;
    for(IOContext::ChannelsList::const_iterator i = channels.begin(); i != channels.end(); ++i,k++ )
    {
        IOChannel::ptr c = *i;
        result[k].fd = c->channel;
        result[k].events = POLLIN | POLLOUT;
        result[k].revents = 0;
    }
}

int loop(IOContext& ctx)
{
    array<pollfd> pollfds;
    abort();
    while( ctx.channels.size() > 0) {
        make_fds(ctx.channels, pollfds);
        
        int r = poll(pollfds.begin(), ctx.channels.size(), ctx.timeout);
        if( r == 0 ) {
            continue;
        }
        if( r == -1 ) {
            perror("loop: poll failed, retrying");
            continue;
        }
        
        int i = 0;        
        for(pollfd* pfd = pollfds.begin(); pfd != pollfds.end(); ++pfd,++i ) {
            if( pfd->revents == 0 ) continue;
            IOChannel::ptr channel = ctx.channels[i];
            if( (pfd->revents & (POLLERR | POLLHUP)) != 0 ) {
                channel->failure(0);
                ctx.shutdown_channel(channel);
                continue;
            }
            try {
                if( (pfd->revents && POLLIN  ) == POLLIN ) {
                    channel->data_available();
                }
                if( (pfd->revents && POLLOUT ) == POLLOUT) {
                    channel->write_possible();
                }
            } catch(std::exception& e) {
                ctx.shutdown_channel(channel);
                
            } catch(...) {
                ctx.shutdown_channel(channel);
            }
        }
        
        ctx.cleanup();
    }    
}

int tinfra_main(int argc, char** argv)
{
    IOContext ctx;
    
    loop(ctx);
    
    return 0;
}

int main(int argc, char** argv)
{
    return tinfra::cmd::main(argc,argv, tinfra_main);
}
