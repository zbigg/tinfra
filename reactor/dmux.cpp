//
// Copyright (C) Zbigniew Zagorski <z.zagorski@gmail.com>,
// licensed to the public under the terms of the GNU GPL (>= 2)
// see the file COPYING for details
// I.e., do what you like, but keep copyright and there's NO WARRANTY.
//

#include <stdexcept>
#include <string>

#include <iostream>
#include <sstream>

#include "tinfra/cmd.h"
#include "tinfra/io/stream.h"
#include "tinfra/io/socket.h"
#include "tinfra/symbol.h"

#include "tinfra/subprocess.h"
#include "tinfra/fmt.h"
#include "aio.h"
//#include "tinfra/aio.h"


#include "protocols.h"
#include "message_raw.h"

using tinfra::aio::Dispatcher;
using tinfra::aio::Channel;

#define TINFRA_DECLARE_STRUCT template <typename F> void apply(F& field) const
#define FIELD(a) field(S::a, a)

namespace dmux {
    namespace S {
        extern tinfra::symbol message_type;
        
        extern tinfra::symbol request_id;
        extern tinfra::symbol last_response_id;
        extern tinfra::symbol status;
        
        extern tinfra::symbol address;
        
        extern tinfra::symbol connection_id;
        extern tinfra::symbol connection_status;
        
        extern tinfra::symbol data;
    }
    
    enum message_type_t {
        INFO,
        CONNECT,
        EVENT
    };
    
    struct message_header {        
        unsigned short message_type;
        unsigned int   request_id;
        unsigned int   last_response_id;
        unsigned int   status;
        
        TINFRA_DECLARE_STRUCT {
            FIELD(message_type);
            FIELD(request_id);
            FIELD(last_response_id);
            FIELD(status);
        }
    };

    struct connect {
        std::string address;
        
        TINFRA_DECLARE_STRUCT {
            FIELD(address);
        }
    };
    
    enum connection_status_bits {
        DATA        = 1
        ESTABLISHED = 2,
        CLOSED      = 4,
        FAILED      = 8
    };
    struct event {
        unsigned int  connection_id;
        unsigned int  connection_status;
        std::string   data;
        
        TINFRA_DECLARE_STRUCT {
            FIELD(connection_id);
            FIELD(connection_status);
            FIELD(data);
        }
    };

}

// begin IMPL
#include "tinfra/tinfra.h"
#include "network_serializer.h"

namespace dmux { namespace S {
    tinfra::symbol message_type("message_type");
    
    tinfra::symbol request_id("request_id");
    tinfra::symbol last_response_id("last_response_id");
    tinfra::symbol status("status");
    
    tinfra::symbol address("address");
    
    tinfra::symbol connection_id("connection_id");
    tinfra::symbol connection_status("connection_status")
    
    tinfra::symbol data("data");
} } // end namespace AutoSH::S


using tinfra::fmt;

namespace dmux {

class ProtocolHandler: public message_raw::ProtocolHandler {
public:
    // protocol dispatcher
    virtual void accept_message(const std::string& message, tinfra::io::stream* channel)
    {
        try {
            network_serializer::reader r(message.str(), message.size());
            message_header h;
            tinfra::process(h, r);
            
            switch( h.message_type ) {
            case INFO:
                handle_info(h, channel);
                break;
            case CONNECT:
                {
                    connect c;
                    tinfra::process(c, r);
                    handle_connect(h, c, channel);
                }
                break;
            case EVENT:
                {
                    event e;
                    tinfra::process(e, r);
                    handle_event(h, e, channel);
                }
                break;
            }

        } catch( tinfra::would_block& w) {
            // FATAL: means infomplete/malformed message
            // TODO: should close the protocol
        }
    }
    
    
    // protocol events
    virtual void handle_info(message_header const& h, tinfra::io::stream* channel);
    virtual void handle_connect(message_header const& h, connect& c, tinfra::io::stream* channel);
    virtual void handle_event(message_header const& h, event& c, tinfra::io::stream* channel);
    
    // transport events
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
protected:
    typedef std::map<unsigned int, Listener*> ConnectionHandlerMap
    ConnectionHandlerMap connections;
};

class ClientProtocolHandler: public ProtocolHandler {
    // protocol events
    virtual void handle_info(message_header const& h, tinfra::io::stream* channel)
    {
    }
    virtual void handle_connect(message_header const& h, connect& c, tinfra::io::stream* channel)
    {
        // // PROTERROR: CLIENT doesn't accept connect request
        channel->close();
    }
    virtual void handle_event(message_header const& h, event& c, tinfra::io::stream* channel)
    {
        const bool closed =      (c.connection_status & CLOSED != 0);
        const bool failed =      (c.connection_status & FAILED != 0);
        const bool established = (c.connection_status & ESTABLISHED != 0);
        const bool data =        (c.connection_status & DATA != 0);
        
        if(  failed ) {
            // connection is now ususable
            // inform client about
            // closed/
            return;
        } else if( established ) {
            // established, what to do?
        } 
        if( data ) {
            // have data, forward to client
        }
        if( closed ) {
            // inform that connection is closed
        }
    }
};

/**
    ServiceManager
    
    Creates and destroys instances of Listeners.

    Dispatchers may use ServiceManager to create Listeners for newly open
    connections (call to open(address) ) and then destroy using close(listener)
    method.

    close(listener) is invoked when protocol has ended to use service in
    due any (service dependent or independent) event.

    close(listener) might delete listener instance because it will never be used
    by it's clients.
*/

class ServiceManager {
    Listener* open(std::string const& address);
    void      close(Listener*);
};

class ServerProtocolHandler: public ProtocolHandler {
    
    ServiceManager& service_manager;
    
    ServerProtocolHandler(ServiceManager& sm): service_manager(sm) {}
        
    // protocol events
    virtual void handle_info(message_header const& h, tinfra::io::stream* channel)
    {
    }
    virtual void handle_connect(message_header const& h, connect& c, tinfra::io::stream* channel)
    {
        // CLIENT doesn't accept connect request
        channel->close();
    }
    virtual void handle_event(message_header const& h, event& c, tinfra::io::stream* channel)
    {
        const bool closed =      (c.connection_status & CLOSED != 0);
              bool failed =      (c.connection_status & FAILED != 0);
        const bool established = (c.connection_status & ESTABLISHED != 0);
        const bool data =        (c.connection_status & DATA != 0);
        
        Listener* listener = get_listener(c.connection_id);
        if( established ) {
            // PROTERROR: server can't accept established
            failed = true;
        }
        if(  failed ) {
            // connection is now ususable
            // inform client about
            // closed/
            channel->close();
            return;
        }
        if( data ) {
            // have data, forward to client
        }
        if( closed ) {
            // inform that connection is closed
        }
    }

};
};

} // end namespace dmux
