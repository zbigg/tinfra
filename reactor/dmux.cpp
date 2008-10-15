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
#include "dmux.h"

using tinfra::aio::Dispatcher;
using tinfra::aio::Channel;

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
} } // end namespace dmux::S


using tinfra::fmt;

namespace dmux {


class CommonProtocolHandler: public message_raw::ProtocolHandler {
public:
    // protocol dispatcher
    virtual void accept_message(const std::string& message, tinfra::io::stream* channel)
    {
        try {
            network_serializer::reader r(message.str(), message.size());
            message_header h;
            tinfra::process(h, r);
            
            switch( h.message_type ) {
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
    virtual void handle_connect(message_header const& h, connect& c, tinfra::io::stream* channel) = 0;
    virtual void handle_event(message_header const& h, event& c, tinfra::io::stream* channel) = 0;
    
    template <typename T>
    virtual void reply(message_header const& in_reply_to,
                       message_type_t message_type,
                       int status,
                       T const& message,
                       tinfra::io::stream* channel)
    {
        std::string reply_buffer;
        reply_buffer.reserve(1024);
        network_serializer::writer w(reply_buffer);
        
        message_header header;
        header.message_type = message_type;
        header.request_id = get_next_request_id()
        header.last_response_id = in_reply_to.request_id;
        header.status = status;
        
        tinfra::process(header, w);
        tinfra::process(message, w);
        
        send_message(channel, reply_buffer);
    }
    
    template <typename T>
    void send(message_type_t message, int status, T const& message, tinfra::io::stream* channel)
    {
        message_header header;
        header.message_type = message_type;
        header.request_id = get_next_request_id()
        header.last_response_id = last_accepted_request_id;
        header.status = status;
        
        std::string message_buffer;
        message_buffer.reserve(1024);
        
        network_serializer::writer w(message_buffer);
        tinfra::process(header, w);
        tinfra::process(message, w);
        
        send_message(channel, message_buffer);
    }
    
    // transport events
    virtual void write_completed(size_t bytes_sent, size_t bytes_queued)
    {
        
    }
    
    virtual void eof(int direction)
    {
        // TODO: eof on main stream
        // inform ALL listeners that their channels are closed
    }

    /// check if this protocol handler has finished reading
    virtual bool is_finished()
    {
        return false;
    }
    
    class ConnectionOutputStream: public tinfra::io::stream {
        CommonProtocolHandler& parent;
        int connection_id;
        tinfra::io::stream* channel;
        bool closed_requested;
    public:
        ConnectionOutputStream(CommonProtocolHandler& parent,
                                int connection_id, 
                                tinfra::io::stream* channel) :
            parent(parent), 
            connection_id(connection_id),
            channel(channel),
            close_requested(false)
        {}
            
        
        void close()
        {
            close_requested = true;
        }
        
        bool is_close_requested() const {
            return close_requested;
            
        }
        int seek(int , seek_origin)
        {
            throw std::logic_error("protocols::CommonProtocolHandler::ConnectionOutputStream::seek not available");
        }
        int read(char*, int)
        {
            throw std::logic_error("protocols::CommonProtocolHandler::ConnectionOutputStream::read not available");
        }
        int write(const char* data, int size)
        {
            if( close_requested )
                throw tinfra::logic_error("trying to write to closed stream");
            event ev;
            ev.flags = DATA;
            ev.connection_id = this->connection_id;
            ev.data.assign(data, size);
            
            this->parent.send(EVENT, 0, ev, this->channel);
            
            return size;
        }
        void sync()
        {
        }
    
        intptr_t native() const { return -1; }
        void release() {}
    };
protected:
    
    Listener* get_listener(int connection_id) const{
        ConnectionHandlerMap::const_iterator i = connections.find(connection_id);
        if( i != connections.end() )
            return *i;
        else
            return 0;
    }
    
    void add_listener(int connection_id, Listener* listener) {
        listsner.insert(connection_id, listener);
    }
    void remove_listener(int connection_id)
    {
        connections.erase(connection_id);
    }
    
    int last_accepted_request_id; // id of last accepted request
    
    typedef std::map<unsigned int, Listener*> ConnectionHandlerMap;
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
    /// Open a service.
    ///
    /// Throws std::domain_error if service given by address cannot be opened.
    Listener* open(std::string const& address);
    
    /// Close the service.
    void      close(Listener*);
};

class ServerProtocolHandler: public CommonProtocolHandler {
    
    ServiceManager& service_manager;
    int next_connection_id;
    ServerProtocolHandler(ServiceManager& sm): 
        service_manager(sm),
        next_connection_id(0)
    {}
        
    // protocol events
    virtual void handle_info(message_header const& h, tinfra::io::stream* channel)
    {
    }
    virtual void handle_connect(message_header const& h, connect& c, tinfra::io::stream* channel)
    {
        event result;
        try {
            Listener* listener = service_manager->open(c.address());
            int connection_id = ++next_connection_id;
            add_listener(connection_id, listener);

            result.connection_id = connection_id;
            result.connection_status = ESTABLISHED;
            
            reply(h, EVENT, 0, result, channel);
        } catch( std::domain_error& de) {
            result.connection_status = FAILED;
            result.connection_id = 0;
            
            reply(h, EVENT, 0, result, channel);
        }
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
            ConnectionOutputStream s(*this, c.connection_id, channel);
            // TODO: buffer data and then provide to listener
            const char* buffer = 0;
            size_t      buffer_size = 0;
            int bytes_accepted = listener->accept_bytes(buffer, buffer_size, &s);
            // TODO: remove accepted data from buffer
            
            if( s.is_close_requested() ) {
                // TODO listener to remove, should flush buffers and reject
            }
        }
        if( closed ) {
            listener->eof(Dispatcher::READ | Dispatcher::WRITE);
        }
    }

};
};

} // end namespace dmux
