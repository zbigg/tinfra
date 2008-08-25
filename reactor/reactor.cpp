#include <vector>
#include <algorithm>
#include <stdexcept>
#include <list>
#include <map>
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
public:
    enum {
        READ = 1, WRITE = 2
    };
    virtual void add_channel(IOChannel* c) = 0;
    virtual void remove_channel(IOChannel* c) = 0;
    virtual void listen_channel(IOChannel* c, int mask, bool enable) = 0;

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
        channel_props[c] = 0;
    }
    
    virtual void listen_channel(IOChannel* c, int flags, bool enable) 
    {    
        if( enable ) {
            channel_props[c] |=  flags;
        } else {
            channel_props[c] &= ~flags;
        }
    }

    void cleanup() {
        for(ChannelsList::const_iterator i = to_remove.begin(); i != to_remove.end(); ++i ) {
            IOChannel* channel = *i;
            channel_props.erase(channel);
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
    std::map<IOChannel*,int> channel_props;
    void make_fds(PollIOReactor::ChannelsList const& channels, array<pollfd>& result)
    {    
        result.size(channels.size());
        
        int k = 0;
        for(PollIOReactor::ChannelsList::const_iterator i = channels.begin(); i != channels.end(); ++i,k++ )
        {
            IOChannel::ptr c = *i;
            result[k].fd = c->file();
            int props = channel_props[c];
            int events = 0;            
            if( props && IOReactor::READ )
                events |= POLLIN;
            if( props && IOReactor::WRITE )
                events |= POLLOUT;
            result[k].events = events;
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
        std::string peer_address;
        {
            
            tinfra::io::stream* s = tinfra::io::socket::accept_client_connection(main_socket, &peer_address);
            new_socket = s->native();
            s->release();
            delete s;
        }
        on_accept(new_socket, peer_address);
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

class ProtocolHandler {
public:
    /// Called by ProtocolManager when IO has some data buffered
    /// Protocol should consume as much data as he can
    /// And then return number of bytes consumed    
    /// @returns 0 if that protocol is unable to assemble any message - IO must gather more data
    /// @returns > 0 length of consumed message
    virtual int  accept_bytes(const char* , int) = 0;
    
    virtual void eof(int direction) = 0;

    /// check if this protocol handler has finished reading
    virtual bool is_finished() = 0;
};

typedef std::string buffer;

class ProtocolWrapperChannel: public IOChannel {
public:
    ProtocolWrapperChannel(tinfra::io::stream* channel, ProtocolHandler* handler)
        : channel(channel), 
          handler(handler),
          read_eof(false)
    {}
    
protected:
    tinfra::io::stream* channel;
    
    ProtocolHandler* handler;
    
    buffer received_bytes;
    buffer to_send;

    bool   read_eof;
    bool   write_eof;

    virtual int  file() { 
        return channel->native(); 
    }
    
    virtual void close() {
        channel->close();
    }
    
    void data_available(IOReactor& r)
    {
        // general idea
        //  - read till end of buffer
        //  - while possible consume using protocol handler
        //    
        while (! read_eof && ! handler->is_finished() ) {
            while( ! received_bytes.empty() ) {
                int accepted = handler->accept_bytes(received_bytes.data(), received_bytes.size());
                if( accepted == 0 ) 
                    break; // not nough data, try read some
                received_bytes.erase(received_bytes.begin(), received_bytes.begin() + accepted);
            }
            
            try {
                char buffer[1024];
                int readed = channel->read(buffer, sizeof(buffer));
                if( readed == 0 ) {
                    handler->eof(IOReactor::READ);
                    read_eof = true;
                    break;
                }
                received_bytes.append(buffer, readed);
            } catch( tinfra::io::would_block& b) {
                // nothing read so protocol can't continue
                break;
            }
            // something read, retry with protocol
        }
        if( read_eof || handler->is_finished() ) {
            r.listen_channel(this, IOReactor::READ, false);
        } else {
            r.listen_channel(this, IOReactor::READ, true);
        }
    }
    
    void write_possible(IOReactor& r)
    {
        try {
            if( ! write_eof && to_send.size() > 0 ) {
                int written = channel->write(to_send.data(), to_send.size());
                if( written == 0 ) {
                    handler->eof(IOReactor::WRITE);
                    write_eof = true;
                    break;
                }
                if( written > 0 ) {
                    to_send.erase(to_send.begin(), to_send.begin() + written);
                }
            }
        } catch( tinfra::io::would_block&) {
            // ignore it!
        }
        
        if( write_eof || to_send.size() == 0 ) {
            r.listen_channel(this, IOReactor::WRITE, false);
        } else {
            r.listen_channel(this, IOReactor::WRITE, true);
        }
    }
};

int tinfra_main(int argc, char** argv)
{
    PollIOReactor ctx;
    EchoChannel echo_service;
    
    ctx.timeout = -1;
    
    ctx.add_channel( & echo_service ) ;
    ctx.listen_channel( & echo_service, IOReactor::READ, true);
    
    ctx.loop();
    return 0;
}

int main(int argc, char** argv)
{
    return tinfra::cmd::main(argc,argv, tinfra_main);
}
