#include "tinfra/io/stream.h"
#include "tinfra/io/socket.h"
#include "tinfra/thread.h"
#include <ostream>
#include <istream>
#include <iostream>
#include <sstream>
#include <string>

#include <unittest++/UnitTest++.h>
#include "tinfra/test.h"

static void trim(std::string& s)
{
    int size = s.size();
    if( size == 0 ) return;
    if( s[size-1] == '\n' ) s.erase(--size, 1);
    if( size == 0 ) return;
    if( s[size-1] == '\r' ) s.erase(--size, 1);
}

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
        response = "hello!";
        client->write(response.c_str(), response.size());
        client->write("\r\n",2);
        while( connected ) {            
            {
                tinfra::io::zstreambuf buf(client.get());
                std::istream in(&buf);
                std::string cmd;
                std::getline(in, cmd);
                trim(cmd);
                std::cerr << "S<" << cmd << std::endl;
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
                std::cerr << "S>" << response << std::endl;
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
        tinfra::Thread::start_detached(*worker);
    }
    
    virtual void run()
    {
        bind("localhost", 10900);
        tinfra::io::socket::Server::run();
    }
};

std::string invoke(std::istream& in, std::ostream& out, std::string const& msg)
{
    out << msg << std::endl;
    std::cerr << "C>" << msg << std::endl;
    std::string tmp;
    std::getline(in, tmp);
    trim(tmp);
    std::cerr << "C<" << tmp << std::endl;
    return tmp;
}

TEST(test_server)
{
    TestServer server;
    tinfra::Thread server_thread = tinfra::Thread::start(server);    
    {        
        std::auto_ptr<tinfra::io::stream> client = std::auto_ptr<tinfra::io::stream>(tinfra::io::socket::open_client_socket("localhost",10900));
        
        tinfra::io::zstreambuf ibuf(client.get());
        tinfra::io::zstreambuf obuf(client.get());
        
        std::istream in(&ibuf);
        std::ostream out(&obuf);
        
        CHECK_EQUAL( "zbyszek", invoke(in,out, "zbyszek"));
        CHECK_EQUAL( "", invoke(in,out, ""));
        CHECK_EQUAL( "quitting", invoke(in,out, "stop"));
    }
    server.stop();
    server_thread.join();
}