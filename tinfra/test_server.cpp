#include "tinfra/io/stream.h"
#include "tinfra/io/socket.h"
#include "tinfra/thread.h"
#include "tinfra/string.h"
#include <ostream>
#include <istream>
#include <iostream>
#include <sstream>
#include <string>

#include <unittest++/UnitTest++.h>
#include "tinfra/test.h"

class Client: public tinfra::Runnable {
    std::auto_ptr<tinfra::io::stream> client;
    tinfra::io::socket::Server& server;
public:
    Client(std::auto_ptr<tinfra::io::stream> _client, tinfra::io::socket::Server& _server)
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

class TestServer: public tinfra::io::socket::Server, public tinfra::Runnable {
public:
    virtual void onAccept(std::auto_ptr<tinfra::io::stream> client) {
        Runnable* worker = new Client(client, *this);        
        //tinfra::Thread::start_detached(*worker);
        worker->run();
    }
    
    virtual void run()
    {
        tinfra::io::socket::Server::run();
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

TEST(test_server)
{
    TestServer server;
    server.bind("localhost", 10900);
    tinfra::Thread server_thread = tinfra::Thread::start(server);    
    {        
        std::auto_ptr<tinfra::io::stream> client = std::auto_ptr<tinfra::io::stream>(tinfra::io::socket::open_client_socket("localhost",10900));
        
        tinfra::io::zstreambuf ibuf(client.get());
        tinfra::io::zstreambuf obuf(client.get());
        
        std::istream in(&ibuf);
        std::ostream out(&obuf);
        
        CHECK_EQUAL( "zbyszek", invoke(in,out, "zbyszek"));
        CHECK_EQUAL( "A", invoke(in,out, "A"));
        CHECK_EQUAL( "", invoke(in,out, ""));
        CHECK_EQUAL( "quitting", invoke(in,out, "stop"));
    }
    server.stop();
    server_thread.join();
}
