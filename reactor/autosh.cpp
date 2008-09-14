#include <stdexcept>
#include <string>

#include <iostream>
#include <sstream>

#include "tinfra/cmd.h"
#include "tinfra/io/stream.h"
#include "tinfra/io/socket.h"
#include "tinfra/symbol.h"

#include "aio.h"
//#include "tinfra/aio.h"

#include "protocols.h"

using tinfra::aio::Dispatcher;
using tinfra::aio::Channel;

#define TINFRA_DECLARE_STRUCT template <typename F> void apply(F& field) const
#define FIELD(a) field(S::a, a)

namespace AutoSH {
    namespace S {
        extern tinfra::symbol message_type;
        extern tinfra::symbol message_length;
        
        extern tinfra::symbol request_id;
        extern tinfra::symbol command;
        extern tinfra::symbol environment;
        
        extern tinfra::symbol channel_id;
        extern tinfra::symbol stream_id;
        extern tinfra::symbol what;
        extern tinfra::symbol status;
        extern tinfra::symbol data;
        
        extern tinfra::symbol description;
    }
    
    struct MessageHeader {        
        int           message_type;
        size_t        message_length;
        
        TINFRA_DECLARE_STRUCT {
            FIELD(message_type);
            FIELD(message_length);
        }
    };

    struct InvokeRequest {
        unsigned long request_id;
        
        std::vector<std::string> command;
        std::vector<std::string> environment;
        
        TINFRA_DECLARE_STRUCT {
            FIELD(request_id);
            FIELD(command);
            FIELD(environment);
        }
    };
    
    struct InvokeResult {
        unsigned long request_id;
        int           status;
        std::string   description;
        
        int           channel_id;
        
        TINFRA_DECLARE_STRUCT {
            FIELD(request_id);
            FIELD(status);
            FIELD(description);
            FIELD(channel_id);
        }
    };
    
    struct ChannelEvent {
        int           channel_id;
        int           stream_id;
        int           what;
        int           status;
        std::string   data;
        
        TINFRA_DECLARE_STRUCT {
            FIELD(channel_id);
            FIELD(stream_id);
            FIELD(what);
            FIELD(status);
            FIELD(data);
        }
    };
}

namespace AutoSH { namespace S {
    tinfra::symbol message_type("message_type");
    tinfra::symbol message_type("message_length");
    
    tinfra::symbol message_id("request_id");
    tinfra::symbol command("command");
    tinfra::symbol environment("environment");
    
    tinfra::symbol channel_id("channel_id");
    tinfra::symbol stream_id("stream_id");
    tinfra::symbol status("status");
    tinfra::symbol what("what");
    tinfra::symbol data("data");
    
    tinfra::symbol description("description");
} } // end namespace AutoSH::S

namespace AutoSH {

class ProtocolHandler: public ::ProtocolHandler {
public:
    enum {
        BEFORE_MESSAGE_HEADER,
        READING_MESSAGE,
        FINISHED
    } state;
    
    ProtocolHandler()
        : state(BEFORE_MESSAGE_HEADER) 
    {
    }
    
    MessageHeader received_message_header;
    virtual int  accept_bytes(const char* data, int length, tinfra::io::stream* channel)
    {        
        switch( state )  {
        case BEFORE_MESSAGE_HEADER:
            return 0;
        case READING_MESSAGE:
            return 0;
        }
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
    }
    
    virtual void write_completed(size_t bytes_sent, size_t bytes_queued)
    {
        
    }
    
    virtual void eof(int direction)
    {
    }

    /// check if this protocol handler has finished reading
    virtual bool is_finished()
    {
        return false;
    }
};

class ServerChannel: public tinfra::aio::ListeningChannel {
public:
    ServerChannel(int port = 10022): ListeningChannel(port) {}
protected: 
    virtual void on_accept(Dispatcher& d, tinfra::io::stream* stream, std::string const& remote_peer)
    {
        ::ProtocolWrapperChannel* protocol = new ::ProtocolWrapperChannel(stream, new ProtocolHandler());
        
        d.add_channel(protocol);
        d.listen_channel(protocol, Dispatcher::READ, true);
    }
};

} // end namespace AutoSH


int autosh_main(int argc, char** argv)
{
    std::auto_ptr<Dispatcher> ctx = tinfra::aio::createDispatcher();
    
    AutoSH::ServerChannel autosh_service;
    
    ctx->add_channel( & autosh_service ) ;
    ctx->listen_channel( & autosh_service, Dispatcher::READ, true);
    
    while( true ) {
        ctx->step();
    }
    return 0;
}

TINFRA_MAIN(autosh_main);
