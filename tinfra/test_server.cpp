//
// Copyright (C) Zbigniew Zagorski <z.zagorski@gmail.com>,
// licensed to the public under the terms of the GNU GPL (>= 2)
// see the file COPYING for details
// I.e., do what you like, but keep copyright and there's NO WARRANTY.
//

#include "tinfra/io/stream.h"
#include "tinfra/io/socket.h"
#include "tinfra/thread.h"
#include "tinfra/string.h"
#include "tinfra/server.h"
#include <ostream>
#include <istream>
#include <iostream>
#include <sstream>
#include <string>

#include <unittest++/UnitTest++.h>
#include "tinfra/test.h"

class Client: public tinfra::Runnable {
    std::auto_ptr<tinfra::io::stream> client;
    tinfra::net::Server& server;
public:
    Client(std::auto_ptr<tinfra::io::stream> _client, tinfra::net::Server& _server)
        : client(_client), server(_server) {}
        
    virtual void run()
    {
        
        bool connected = true;
        std::string response;
        //response = "hello!";
        //client->write(response.c_str(), response.size());
        //client->write("\r\n",2);
        while( connected ) {            
            {
                tinfra::io::zstreambuf buf(client.get());
                std::istream in(&buf);
                std::string cmd;
                std::getline(in, cmd);
                tinfra::strip_inplace(cmd);
                //std::cerr << "S<" << cmd << std::endl;
                if( cmd == "stop") {
                    server.stop();
                    connected = false;
                    response = "quitting";
                } else if( cmd == "close" ) {
                    connected = false;
                    response = "bye";
                } else {
                    response = cmd;
                }
            }
            {   
                //std::cerr << "S>" << response << std::endl;
                client->write(response.c_str(), response.size());
                client->write("\r\n",2);
            }
        }
        delete this;
    }
};

class TestServer: public tinfra::net::Server, public tinfra::Runnable {
public:
    virtual void onAccept(std::auto_ptr<tinfra::io::stream> client, std::string const&) {
        Runnable* worker = new Client(client, *this);        
        //tinfra::Thread::start_detached(*worker);
        worker->run();
    }
    
    virtual void run()
    {
        tinfra::net::Server::run();
    }
};

std::string invoke(std::istream& in, std::ostream& out, std::string const& msg)
{
    out << msg << std::endl;
    out.flush();
    //std::cerr << "C>'" << tinfra::escape_c(msg) << "'" << std::endl;
    std::string tmp;
    std::getline(in, tmp);
    tinfra::strip_inplace(tmp);
    //std::cerr << "C<'" << tinfra::escape_c(tmp) << "'" << std::endl;
    return tmp;
}
SUITE(tinfra_server)
{
    TEST(test_server_generic)
    {
        TestServer server;
        server.bind("localhost", 10900);
        tinfra::ThreadSet ts;
        tinfra::Thread server_thread = ts.start(server);    
        {        
            std::auto_ptr<tinfra::io::stream> client(tinfra::io::socket::open_client_socket("localhost",10900));
            
            tinfra::io::zstreambuf ibuf(client.get());
            tinfra::io::zstreambuf obuf(client.get());
            
            std::istream in(&ibuf);
            std::ostream out(&obuf);
	    
            // TODO: this test fails with "zyszek" sometimes
	    //       because zstreambuf is broken
	    //       in following read sequence: 
	    //  sendto(4, "zbyszek", 7, 0, NULL, 0)     = 7
	    //  sendto(4, "\n", 1, 0, NULL, 0)          = 1
	    //  recvfrom(4, "z", 1, 0, NULL, NULL)      = 1
	    //  recvfrom(4, "byszek", 32768, 0, NULL, NULL) = 6
	    //  recvfrom(4, "\r\n", 32768, 0, NULL, NULL) = 2
	    //
	    // and thus std::getline( returns "zyszek\r\n")
		
            CHECK_EQUAL( "zbyszek", invoke(in,out, "zbyszek"));
	    
            CHECK_EQUAL( "A", invoke(in,out, "A"));
            CHECK_EQUAL( "", invoke(in,out, ""));
            CHECK_EQUAL( "quitting", invoke(in,out, "stop"));
        }
        server.stop();
    }

    // check if Server::stop can abort blocking accept call
    TEST(test_server_stop)
    {
        TestServer server;
        server.bind("localhost", 10901);
        tinfra::ThreadSet ts;
        ts.start(server);
        
        server.stop();
    }
}
