#include <vector>
#include <algorithm>
#include <stdexcept>
#include <list>
#include <poll.h>

#include <iostream>

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

class IOChannel;

class IOReactor {
    enum {
        READ = 1, WRITE = 2
    };
public:
    virtual void add_channel(IOChannel* c) = 0;
    virtual void remove_channel(IOChannel* c) = 0;
    virtual void listen_channel(IOChannel* c, int flags) = 0;

    virtual ~IOReactor() {}
};

class IOChannel {
public:
    typedef IOChannel* ptr;
    
    virtual int  file() = 0;
    
    virtual void close() = 0;
        
    virtual void failure(IOReactor& r) { 
        r.remove_channel(this); 
        close(); 
    }
    
    virtual void hangup(IOReactor& r) { 
        r.remove_channel(this); 
        close(); 
    }
    virtual void data_available(IOReactor&) {
        std::cerr << "IOChannel: warning data_available on abstract IOChannel!" << std::endl;
    }
    virtual void write_possible(IOReactor&) {
        std::cerr << "IOChannel: warning write_possible on abstract IOChannel!" << std::endl;
    }
        
    virtual ~IOChannel() {}
};

class PollIOReactor: public IOReactor {
public:
    
    int timeout;
    
    void remove_channel(IOChannel* c) 
    {
        to_remove.push_back(c);
    }
    
    virtual void add_channel(IOChannel* c) 
    {
        channels.push_back(c);
    }
    
    virtual void listen_channel(IOChannel* c,int flags) 
    {
        // TODO: add list of flags for each channel
    }

    void cleanup() {
        for(ChannelsList::const_iterator i = to_remove.begin(); i != to_remove.end(); ++i ) {
            IOChannel* channel = *i;
            
            channels.erase(std::find(channels.begin(), channels.end(), channel));
        }
        
        to_remove.clear();
    }
    
public:
    void loop()
    {
        array<pollfd> pollfds;
        while( channels.size() > 0) {
            make_fds(channels, pollfds);
            
            int r = poll(pollfds.begin(), channels.size(), timeout);
            
            if( r == 0 ) {
                continue;
            }
            if( r == -1 ) {
                perror("loop: poll failed, retrying");
                continue;
            }
            
            int i = 0;        
            for(pollfd* pfd = pollfds.begin(); pfd != pollfds.end(); ++pfd,++i ) {
                if( pfd->revents == 0 ) 
                    continue;
                
                IOChannel::ptr channel = channels[i];
                if( (pfd->revents & POLLERR) == POLLERR ) {
                    std::cerr << "channel failure!" << std::endl;
                    channel->failure(*this);
                    continue;
                }
                if( (pfd->revents & POLLHUP) == POLLHUP ) {
                    std::cerr << "channel hangup!" << std::endl;
                    channel->hangup(*this);
                    continue;
                }
                try {
                    if( (pfd->revents & POLLIN  ) == POLLIN ) {
                        std::cerr << "data available!" << std::endl;
                        channel->data_available(*this);
                    }
                    if( (pfd->revents & POLLOUT ) == POLLOUT) {
                        std::cerr << "write possible" << std::endl;
                        channel->write_possible(*this);
                    }
                } catch(std::exception& e) {
                    channel->close();
                    remove_channel(channel);
                    
                } catch(...) {
                    channel->close();
                    remove_channel(channel);
                }
            }
            
            cleanup();
        }
    }
    
private:
    typedef std::vector<IOChannel*> ChannelsList;

    ChannelsList channels;
    ChannelsList to_remove;
    
    void make_fds(PollIOReactor::ChannelsList const& channels, array<pollfd>& result)
    {    
        result.size(channels.size());
        
        int k = 0;
        for(PollIOReactor::ChannelsList::const_iterator i = channels.begin(); i != channels.end(); ++i,k++ )
        {
            IOChannel::ptr c = *i;
            result[k].fd = c->file();
            result[k].events = POLLIN | POLLOUT; // TODO: use list of flags for each channel
            result[k].revents = 0;
        }
    }
};

class ListeningIOChannel: public IOChannel 
{
    tinfra::io::stream* main_socket;
    
public:
    ListeningIOChannel(int port)
        : main_socket(0)
    {
        listen(port);
    }
    
    int file() { 
        return main_socket->native(); 
    }
    
    void close() { 
        main_socket->close(); 
    }
    
    void listen(int port)
    {
        main_socket = tinfra::io::socket::open_server_socket(0, port);
    }
    
    void data_available(IOReactor&) 
    {
        int new_socket;
        {
            tinfra::io::stream* s = tinfra::io::socket::accept_client_connection(main_socket);
            new_socket = s->native();
            s->release();
            delete s;
        }
        on_accept(new_socket,"");
    }
    
protected:
    virtual void on_accept(int socket, std::string const& remote_peer) = 0;
};


class EchoChannel: public ListeningIOChannel {
public:
    EchoChannel(int port = 9999): ListeningIOChannel(port) {}
protected: 
    virtual void on_accept(int socket, std::string const& remote_peer)
    {
        std::cerr << "got connection from '" << remote_peer << "'" << std::endl;
        ::write(socket, "hello\r\n",7);
        ::close(socket);
    }
    
};

int tinfra_main(int argc, char** argv)
{
    PollIOReactor ctx;
    EchoChannel echo_service;
    
    ctx.timeout = -1;
    
    ctx.add_channel( & echo_service ) ;
    
    ctx.loop();
    return 0;
}

int main(int argc, char** argv)
{
    return tinfra::cmd::main(argc,argv, tinfra_main);
}
