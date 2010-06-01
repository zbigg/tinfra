//
// Copyright (c) 2009, Zbigniew Zagorski
// This software licensed under terms described in LICENSE.txt
//

#include "tinfra/thread.h"
#include "tinfra/string.h"
#include "tinfra/server.h"
#include "tinfra/stream.h"
#include "tinfra/tcp_socket.h"

#include "tinfra/trace.h"
#include "tinfra/fmt.h"

#include <ostream>
#include <istream>
#include <iostream>
#include <sstream>
#include <string>

#include <unittest++/UnitTest++.h>
#include "tinfra/test.h"

using tinfra::fmt;
using tinfra::output_stream;
using tinfra::input_stream;
using tinfra::tcp_client_socket;
using tinfra::tstring;

static void writeline(output_stream& out, tstring const& str)
{
    TINFRA_TRACE_MSG(fmt("wrote '%s'") % tinfra::escape_c(str));
    out.write(str.data(), str.size());
    out.write("\n",1);
}

static std::string readline(input_stream& in)
{
    std::string result;
    char c;
    while( true ) {
        int r = in.read(&c, 1);
        if( r == 0 )
            break;
        if( c == '\n' )
            break;
        
        result.append(1, c);
    }
    TINFRA_TRACE_MSG(fmt("readed '%s'") % tinfra::escape_c(result));
    return result;
}

class Client {
    std::auto_ptr<tcp_client_socket> client;
    tinfra::net::Server& server;
public:
    Client(std::auto_ptr<tcp_client_socket> _client, tinfra::net::Server& _server)
        : client(_client), server(_server) {}
        
    virtual void run()
    {
        
        bool connected = true;
        std::string response;
        while( connected ) {            
            std::string cmd = readline(*client);
            {
                tinfra::strip_inplace(cmd);                
                //std::cerr << "S<" << cmd << std::endl;
                if( cmd == "stop") {
                    connected = false;
                    response = "quitting";
                } else if( cmd == "close" ) {
                    connected = false;
                    response = "bye";
                } else {
                    response = cmd;
                }
            }
            writeline(*client, response);
            
            if( !connected ) {
                server.stop();
            }
        }
        delete this;
    }
};

class TestServer: public tinfra::net::Server {
public:
    virtual void onAccept(std::auto_ptr<tinfra::tcp_client_socket> client, std::string const&) {
        Client* worker = new Client(client, *this);
        //tinfra::Thread::start_detached(*worker);
        worker->run();
    }
    
    void operator()()
    {
        tinfra::net::Server::run();
    }
};

std::string invoke(tcp_client_socket& socket, tstring const& msg)
{
    writeline(socket, msg);
    socket.sync();
    std::string tmp = readline(socket);
    tinfra::strip_inplace(tmp);
    return tmp;
}

SUITE(tinfra)
{
    TEST(server_generic)
    {
        TestServer server;
        server.bind("localhost", 10900);
        tinfra::thread::thread_set ts;
        tinfra::thread::thread server_thread = ts.start(tinfra::runnable_ref(server));
        {        
            tcp_client_socket client("localhost",10900);
		
            CHECK_EQUAL( "zbyszek", invoke(client, "zbyszek"));
	    
            CHECK_EQUAL( "A", invoke(client, "A"));
            CHECK_EQUAL( "", invoke(client, ""));
            CHECK_EQUAL( "quitting", invoke(client, "stop"));
        }
        //server.stop();
    }

    // check if Server::stop can abort blocking accept call
    TEST(server_stop)
    {
        TestServer server;
        server.bind("localhost", 10901);
        tinfra::thread::thread_set ts;
        ts.start(tinfra::runnable_ref(server));
        
        server.stop();
    }
}
