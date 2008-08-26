#include <vector>
#include <algorithm>
#include <stdexcept>
#include <list>
#include <map>
#include <poll.h>

#include <iostream>
#include <sstream>

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
            
            ChannelsList::iterator ic = std::find(channels.begin(), channels.end(), channel);
            if( ic != channels.end() ) 
                channels.erase(ic);
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
                    //std::cerr << "channel failure!" << std::endl;
                    channel->failure(*this);
                    continue;
                }
                if( (pfd->revents & POLLHUP) == POLLHUP ) {
                    //std::cerr << "channel hangup!" << std::endl;
                    channel->hangup(*this);
                    continue;
                }
                try {
                    if( (pfd->revents & POLLIN  ) == POLLIN ) {
                        //std::cerr << "data available!" << std::endl;
                        channel->data_available(*this);
                    }
                    if( (pfd->revents & POLLOUT ) == POLLOUT) {
                        //std::cerr << "write possible" << std::endl;
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
            if( (props & IOReactor::READ) == IOReactor::READ )
                events |= POLLIN;
            if( (props & IOReactor::WRITE) == IOReactor::WRITE )
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
    
    void data_available(IOReactor& r) 
    {
        std::string peer_address;
        
        tinfra::io::stream* new_socket = tinfra::io::socket::accept_client_connection(main_socket, &peer_address);
        
        on_accept(r, new_socket, peer_address);
    }
    
protected:
    virtual void on_accept(IOReactor&, tinfra::io::stream* channel, std::string const& remote_peer) = 0;
};


class EchoChannel: public ListeningIOChannel {
public:
    EchoChannel(int port = 9999): ListeningIOChannel(port) {}
protected: 
    virtual void on_accept(IOReactor&, tinfra::io::stream* channel, std::string const& remote_peer)
    {
        std::cerr << "got connection from '" << remote_peer << "'" << std::endl;
        channel->write("hello\r\n",7);
        delete channel;
    }
    
};


class ProtocolHandler {
public:
    /// Called by ProtocolManager when IO has some data buffered
    /// Protocol should consume as much data as he can
    /// And then return number of bytes consumed    
    /// @returns 0 if that protocol is unable to assemble any message - IO must gather more data
    /// @returns > 0 length of consumed message
    virtual int  accept_bytes(const char* , int, tinfra::io::stream*) = 0;
    
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
          closed(false),
          read_eof(false),
          write_eof(false),
          public_stream(*this)
    {}
    ~ProtocolWrapperChannel()
    {
        delete handler;
    }
    tinfra::io::stream* get_input_stream() { return &public_stream; }
    tinfra::io::stream* get_output_stream() { return &public_stream; }
    
protected:
    tinfra::io::stream* channel;
    
    ProtocolHandler* handler;
    
    buffer received_bytes;
    buffer to_send;

    bool   closed;
    bool   read_eof;
    bool   write_eof;    

    class BufferedNonBlockingStream: public tinfra::io::stream {
        ProtocolWrapperChannel& base;
    public:
        
        BufferedNonBlockingStream(ProtocolWrapperChannel& b): base(b) {}

        virtual intptr_t native() const 
        {
            return base.channel->native();
        }
        
        virtual void release()
        {
            throw std::logic_error("ProtocolWrapperChannel::BufferedNonBlockingStream: release() not supported");
        }
        
        virtual void close()
        {
            base.close();
        }
        
        virtual int seek(int pos, seek_origin origin = start)
        {
            throw std::logic_error("ProtocolWrapperChannel::BufferedNonBlockingStream: seek() not supported");
        }
        
        virtual int read(char* dest, int size)
        {
            int readed = 0;
            while( readed < size ) {
                if( (int)base.received_bytes.size() >= size ) {
                    memcpy(dest, base.received_bytes.data(), size);
                    base.received_bytes.erase(0, size);
                    return size;
                } else {
                    int r = base.read_next_chunk();
                    if( r == 0 )
                        return readed;
                    if( r == -1 )
                        throw tinfra::io::would_block("");
                }
            }
            return readed;
        }
        virtual int write(const char* data, int size)
        {
            if( base.write_eof || base.closed )
                return 0;
            int written = 0;
            try {
                written = base.channel->write(data, size);
                if( written == 0 ) {
                    base.write_eof = true;
                    return 0;
                }
            } catch( tinfra::io::would_block& w) {
                // ignore it, written = 0, so all will be buffered
            }
            if( written < size ) {
                base.to_send.append(data + written, size - written);
                written = size;
            }
            return written;
        }
        virtual void sync() 
        {
            // not supported
        }
    };
    
    BufferedNonBlockingStream public_stream;
    
    
    virtual int  file() { 
        return channel->native(); 
    }
    
    virtual void close() {
        channel->close();
        closed = true;
    }
    
    void data_available(IOReactor& r)
    {
        if( closed ) {
            update_listen_status(r);
            return;
        }
        // general idea
        //  - read till end of buffer
        //  - while possible consume using protocol handler
        //    
        while (! read_eof && ! handler->is_finished() ) {
            while( ! received_bytes.empty() ) {
                int accepted = handler->accept_bytes(received_bytes.data(), received_bytes.size(), get_output_stream());
                if( accepted == 0 ) 
                    break; // not nough data, try read some
                received_bytes.erase(0, accepted);
            }
            
            if( read_next_chunk() <= 0 )
                break;
            // something read, retry with protocol
        }
        update_listen_status(r);
    }
    
    int read_next_chunk()
    {
        if( closed ) 
            return 0;
        try {
            char buffer[1024];
            int readed = channel->read(buffer, sizeof(buffer));
            if( readed == 0 ) {
                handler->eof(IOReactor::READ);
                read_eof = true;
                return 0;
            }
            received_bytes.append(buffer, readed);
            return readed;
        } catch( tinfra::io::would_block& b) {
            // nothing read so protocol can't continue
            return -1;
        }
    }
    
    void write_possible(IOReactor& r)
    {
        if( closed ) {
            update_listen_status(r);
            return;
        }
        try {
            if( ! write_eof && to_send.size() > 0 ) {
                int written = channel->write(to_send.data(), to_send.size());
                if( written == 0 ) {
                    handler->eof(IOReactor::WRITE);
                    write_eof = true;
                    
                } else if( written > 0 ) {
                    to_send.erase(0, written);
                }
            }
        } catch( tinfra::io::would_block&) {
            // ignore it!
        }
        update_listen_status(r);
    }
    
    void update_listen_status(IOReactor& r)
    {
        if( closed ) {
            r.remove_channel(this);
            return;
        }
        if( read_eof || handler->is_finished() ) {
            r.listen_channel(this, IOReactor::READ, false);
        } else {
            r.listen_channel(this, IOReactor::READ, true);
        }
        
        if( write_eof || to_send.size() == 0 ) {
            r.listen_channel(this, IOReactor::WRITE, false);
        } else {
            r.listen_channel(this, IOReactor::WRITE, true);
        }
    }
};

class HTTPProtocolHandler: public ProtocolHandler {
public:
    enum {
        BEFORE_REQUEST,
        HEADERS,
        READING_POST,
        FINISHED
    } state;
    HTTPProtocolHandler()
        : state(BEFORE_REQUEST) 
    {
    }
    
    virtual int  accept_bytes(const char* data, int length, tinfra::io::stream* channel)
    {        
        switch( state ) {
        case BEFORE_REQUEST:
            {
                std::string line;
                if( !expect_line(data, length, line) )
                    return 0;
                //std::cerr << "HTTP REQUEST: " << line;
                state = HEADERS;
                return line.size();
            }
        case HEADERS:
            {
                std::string line;
                if( !expect_line(data, length, line) )
                    return 0;
                
                //std::cerr << "HTTP HEADER : " << line;
                if( line.size() == 2 ) {
                    state = FINISHED;                
                    send_response(channel,"Hello you");
                }
                return line.size();     
            }
            
        case READING_POST:
            break;
        default:
            return 0;
        }
        throw std::logic_error("bad state");
    }
    
    bool expect_line(const char* data, int length, std::string& dest)
    {
        if( length < 2 ) 
            return 0;
        const char* lf = static_cast<char*>( memchr(data, '\n', length ) );
        if( lf == 0 ) 
            return false;
        if( lf > data && lf[-1] == '\r' ) {
            dest.append(data, (lf-data)+1);
            return true;
        }
        return false;
    }
    
    void send_response(tinfra::io::stream* channel, std::string const& content)
    {
        std::ostringstream s;
        s << "HTTP/1.0 200 OK\r\n"
          << "Content-type: text/plain\r\n"
          << "Content-length: " << content.size() << "\r\n"
          << "\r\n"
          << content;
        channel->write(s.str().data(), s.str().size());
        channel->close();
    }
    
    virtual void eof(int direction)
    {
    }

    /// check if this protocol handler has finished reading
    virtual bool is_finished()
    {
        return state == FINISHED;
    }
};

class HTTPServerChannel: public ListeningIOChannel {
public:
    HTTPServerChannel(int port = 9999): ListeningIOChannel(port) {}
protected: 
    virtual void on_accept(IOReactor& r, tinfra::io::stream* channel, std::string const& remote_peer)
    {
        // TODO: should analyze ownership of objects
        // now protocol is not deleted
        ProtocolWrapperChannel* protocol = new ProtocolWrapperChannel(channel, new HTTPProtocolHandler());
        
        r.add_channel(protocol);
        r.listen_channel(protocol, IOReactor::READ, true);
    }
};

int tinfra_main(int argc, char** argv)
{
    PollIOReactor ctx;
    EchoChannel echo_service;
    HTTPServerChannel http_service(18080);
    ctx.timeout = -1;
    
    ctx.add_channel( & echo_service ) ;
    ctx.listen_channel( & echo_service, IOReactor::READ, true);
    
    ctx.add_channel( & http_service ) ;
    ctx.listen_channel( & http_service, IOReactor::READ, true);
    
    ctx.loop();
    return 0;
}

int main(int argc, char** argv)
{
    return tinfra::cmd::main(argc,argv, tinfra_main);
}
