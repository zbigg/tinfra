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
#include "protocols.h"

using tinfra::aio::Dispatcher;
using tinfra::aio::Channel;
using tinfra::aio::Acceptor;

class HelloServerAcceptor: public Acceptor {
public:
protected: 
    virtual void accept_connection(Dispatcher& dispatcher, std::auto_ptr<tinfra::io::stream>& client_conn, std::string client_address)
    {
        std::cerr << "HelloServerAcceptor: handling request from: " << client_address << std::endl;
        
        client_conn->write("hello world\r\n",13);
    }
    virtual void failure(Dispatcher&, Channel listener_stream, int event)
    {
    }
};

std::string fake_response;

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
    
    virtual void write_completed(size_t bytes_sent, size_t bytes_queued)
    {
        
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

class HTTPServerAcceptor: public Acceptor {
public:
protected: 
    virtual void accept_connection(Dispatcher& dispatcher, std::auto_ptr<tinfra::io::stream>& client_conn, std::string client_address)
    {        
        ProtocolListener* protocol = new ProtocolListener(client_conn.get(), new HTTPProtocolHandler());
        
        dispatcher.put(client_conn.release(), protocol, Dispatcher::READ);
    }
    
    virtual void failure(Dispatcher&, Channel listener_stream, int event)
    {
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
    tinfra::set_interrupt_policy(tinfra::DEFERRED_SIGNAL);
    
    build_fake_response();
    std::auto_ptr<Dispatcher> ctx = tinfra::aio::create_network_dispatcher();
    
    HelloServerAcceptor hello_acceptor;
    ctx->create(Dispatcher::SERVICE, ":10801", &hello_acceptor, Dispatcher::READ);        
    
    HTTPServerAcceptor http_acceptor;
    ctx->create(Dispatcher::SERVICE, ":10802", &http_acceptor, Dispatcher::READ);
    
    while( true ) {
        tinfra::test_interrupt();
        ctx->step();
    }
    return 0;
}

int main(int argc, char** argv)
{
    return reactor_main(argc, argv);
    //return tinfra::cmd::main(argc,argv, reactor_main);
}

// jedit: :tabSize=8:indentSize=4:noTabs=true:mode=c++:
