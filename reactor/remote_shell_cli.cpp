//
// Copyright (C) 2008 Zbigniew Zagorski <z.zagorski@gmail.com>,
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

#include "tinfra/subprocess.h"

#include "aio.h"
#include "aio/net.h"
#include "protocols.h"
using tinfra::aio::Dispatcher;
using tinfra::aio::Channel;


#include "remote_shell.h"

using tinfra::subprocess;
using tinfra::io::stream;
using std::auto_ptr;
using std::string;

static void initialize_async_file(int file)
{
    tinfra::io::socket::set_blocking(file, false);
}

using std::string;
using std::vector;

int serve()
{
    auto_ptr<stream> in  = tinfra::io::open_native(0);
    auto_ptr<stream> out = tinfra::io::open_native(1);
    
    initialize_async_file(0);
    initialize_async_file(1);
    
    remote_shell::ServiceHandler service;
    
    //ProtocolListener should renamed to protocol_aio_adaptor
    ProtocolListener adaptor(in.get(), out.get(), &service);    
    
    auto_ptr<Dispatcher> dispatcher = tinfra::aio::Dispatcher::create();
    dispatcher->add( in.get(),  Dispatcher::READ, &adaptor);
    dispatcher->add( out.get(), 0               , &adaptor);
    
    while( ! service.is_finished() ) {        
        dispatcher->step();
        tinfra::test_interrupt();
    }
    
    return 0;
}

const int DEFAULT_PORT = 10456;

int listen(int port)
{
    auto_ptr<stream> listen_stream = tinfra::aio::create_service_stream("", port);
    
        
    remote_shell::ServiceHandler service;
    
    //ProtocolListener should renamed to protocol_aio_adaptor
    ProtocolListener adaptor(listen_stream.get(), &service);
    
    auto_ptr<Dispatcher> dispatcher = tinfra::aio::Dispatcher::create();    
    dispatcher->add( listen_stream.get(), Dispatcher::READ, &adaptor ) ;
    
    while( ! service.is_finished() ) {
        dispatcher->step();
        
        tinfra::test_interrupt();
    }
    return 0;
}

typedef vector<string> string_list;

class stdout_autosh_client_handler: public remote_shell::ClientHandler {
    virtual void accept_input(int stream, const string& input, stream* feedback)
    {
    }
    
    /// Communication finished.
    ///
    /// Called by protocol handler when communication has been finished for any reason:
    ///    exit_code >= 0  - it's an clean exit and exit_code means remote process exit code
    ///    exit_code == -1 - communication error - link failed and dropped
    virtual void finished(int exit_code)
    {
        exit_code = 1;
    }
    
    int exit_code;
    
};

int invoke(string const& address, int port, string_list const& command)
{
    stdout_autosh_client_handler client;
    
    auto_ptr<stream> connection_stream = tinfra::aio::create_client_stream(address, port);
    auto_ptr<stream> local_input = tinfra::io::open_native(0);
    initialize_async_file(0);
    
    ProtocolListener adaptor(listen_stream.get(), &service);
    
    auto_ptr<Dispatcher> dispatcher = tinfra::aio::Dispatcher::create();
    
    dispatcher->add( connection_stream.get(), Dispatcher::READ, &adaptor ) ;
    dispatcher->add( local_input.get(),       Dispatcher::READ, &client ) ;
    
    client.invoke("uname -a");
    
    while( !client.is_finished() ) {
        dispatcher->step();
        
        tinfra::test_interrupt();
    }
    
    return client;
}

int autosh_main(int argc, char** argv)
{
    if( argc == 2 && std::strcmp(argv[1], "serve") == 0 ) {
        return serve();
    }
    if( argc == 2 && std::strcmp(argv[1], "listen") == 0 ) { 
        return listen(DEFAULT_PORT);
    }
    
    if( argc < 3 ) {
        app->fail("bad usage");
        return 1;
    }
    
    string address = argv[1];
    string_list command;
    for( int i = 2; i < argc; ++i )
        command.push_back(argv[i]);
    return invoke(address, DEFAULT_PORT, command);
}

TINFRA_MAIN(autosh_main)


