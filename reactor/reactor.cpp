#include <stdexcept>
#include <string>

#include <iostream>
#include <sstream>

#include "tinfra/cmd.h"
#include "tinfra/io/stream.h"
#include "tinfra/io/socket.h"

/*
        problem here:
                who manages channels ?
                Dispatcher ? no, stupid - next class with add/remove
                shared_ptr ? no
                a Manager
                
        A Manager manages lifecycle of dynamically living objects
        using probably reference counting.
        
        Manager api:
                add(object) == add & use()
                use(object) incref
                remove(object) decref
                
                it will have tools like 
                        ScopedLock<T> bla(manager, some_object)
*/

#include "aio.h"
//#include "tinfra/aio.h"

using tinfra::aio::Dispatcher;
using tinfra::aio::Channel;

class HelloServer: public tinfra::aio::ListeningChannel {
public:
    HelloServer(int port = 9999): ListeningChannel(port) {}
protected: 
    virtual void on_accept(Dispatcher&, tinfra::io::stream* stream, std::string const& remote_peer)
    {
        std::cerr << "got connection from '" << remote_peer << "'" << std::endl;
        stream->write("hello\r\n",7);
        delete stream;
    }
};


class ProtocolHandler {
public:
    virtual ~ProtocolHandler() {}
    
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

class ProtocolWrapperChannel: public Channel {
public:
    ProtocolWrapperChannel(tinfra::io::stream* channel, ProtocolHandler* handler)
        : channel(channel), 
          handler(handler),
          closed(false),
          close_requested(false),
          read_eof(false),
          write_eof(false),
          public_stream(*this)
    {}
    virtual ~ProtocolWrapperChannel()
    {
        delete handler;
        delete channel;
    }
    tinfra::io::stream* get_input_stream() { return &public_stream; }
    tinfra::io::stream* get_output_stream() { return &public_stream; }
    
protected:
    tinfra::io::stream* channel;
    
    ProtocolHandler* handler;
    
    buffer received_bytes;
    buffer to_send;

    bool   closed;
    bool   close_requested;
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
            if( base.write_eof || base.closed || base.close_requested )
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
        if( to_send.size() > 0 ) {
            close_requested = true;
        } else { 
            closed = true;
            channel->close();
        }
    }
    virtual void failure(Dispatcher& r) { 
        r.remove_channel(this); 
        channel->close();
        delete this;
    }
    
    virtual void hangup(Dispatcher& r) { 
        r.remove_channel(this);
        delete this;
    }
    
    void data_available(Dispatcher& r)
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
        if( closed || close_requested ) 
            return 0;
        try {
            char buffer[1024];
            int readed = channel->read(buffer, sizeof(buffer));
            if( readed == 0 ) {
                handler->eof(Dispatcher::READ);
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
    
    void write_possible(Dispatcher& r)
    {
        if( closed ) {
            update_listen_status(r);
            return;
        }
        try {
            if( ! write_eof && to_send.size() > 0 ) {
                int written = channel->write(to_send.data(), to_send.size());
                if( written == 0 ) {
                    handler->eof(Dispatcher::WRITE);
                    write_eof = true;
                    
                } else if( written > 0 ) {
                    to_send.erase(0, written);
                }
            }
        } catch( tinfra::io::would_block&) {
            // ignore it!
        }
        if( close_requested && to_send.size() == 0 ) {
            close();
        }
        update_listen_status(r);
    }
    
    void update_listen_status(Dispatcher& r)
    {
        if( closed ) {
            r.remove_channel(this);
            delete this;
            return;
        }
        if( read_eof || handler->is_finished() ) {
            r.listen_channel(this, Dispatcher::READ, false);
        } else {
            r.listen_channel(this, Dispatcher::READ, true);
        }
        
        if( write_eof || to_send.size() == 0 ) {
            r.listen_channel(this, Dispatcher::WRITE, false);
        } else {
            r.listen_channel(this, Dispatcher::WRITE, true);
        }
    }
};

std::string fake_response;

namespace http {

struct std_storage_traits {
	typedef std::string string;
	using std::vector;
};

template <typename M = std_storage_traits>
struct HeaderEntry {
	typedef M F;
	F::string name;
	typename M::string content;
}; 

template <typename M = std_storage_traits>
struct Request {
	(typename M)::string              method;
	typename M::string              request_uri;
	typename M::string              http_version;
	
	typename M::template vector<HeaderEntry<M> > header;
	
	typename M::string              content;
};

template <typename M = std_storage_traits>
struct Response {
	typename M::string              protocol;
	int                             response_code;
	typename M::string              response_text;
	
	//typename M::vector<HeaderEntry> header;
	
	typename M::string              content;
};

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
                    send_response(channel,fake_response);
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
          << "Connection: close\r\n"
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

class HTTPServerChannel: public tinfra::aio::ListeningChannel {
public:
    HTTPServerChannel(int port = 9999): ListeningChannel(port) {}
protected: 
    virtual void on_accept(Dispatcher& r, tinfra::io::stream* channel, std::string const& remote_peer)
    {
        // TODO: should analyze ownership of objects
        // now protocol is not deleted
        ProtocolWrapperChannel* protocol = new ProtocolWrapperChannel(channel, new HTTPProtocolHandler());
        
        r.add_channel(protocol);
        r.listen_channel(protocol, Dispatcher::READ, true);
    }
};

static void build_fake_response()
{
    std::string fake_str = "abcdefgh\r\n";
    unsigned size = 10000000;
    fake_response.reserve(size + fake_str.size());
    for(unsigned i = 0; i < size/fake_str.size(); i++ ) {
            fake_response.append(fake_str);
    }
}

int reactor_main(int argc, char** argv)
{
    build_fake_response();
    std::auto_ptr<Dispatcher> ctx = tinfra::aio::createDispatcher();
    
    HelloServer echo_service;
    HTTPServerChannel http_service(18081);
    
    ctx->add_channel( & echo_service ) ;
    ctx->listen_channel( & echo_service, Dispatcher::READ, true);
    
    ctx->add_channel( & http_service ) ;
    ctx->listen_channel( & http_service, Dispatcher::READ, true);
    
    while( true ) {
        ctx->step();
    }
    return 0;
}

int main(int argc, char** argv)
{
    return tinfra::cmd::main(argc,argv, reactor_main);
}
