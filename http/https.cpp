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

namespace tinfra {
namespace aio {
class managed_dispatcher: public dispatcher {
public:    
    managed_dispatcher(dispatcher& impl);
    
    void remove(stream* c) = 0;
    
    /// Change wait state.
    ///
    /// Change wait state of stream. flags given by mask parameter
    /// are disabled/enabled according to enable parameter.
    virtual void wait(stream* c, int mask, bool enable) = 0;

    /// Perform on dispatching step.
    ///
    /// This call will wait until at least one event occurs.
    /// 
    /// TODO. There should be timeout parameter.
    /// TODO. There should be feedback about what dispatcher has
    ///       done during this one step.
    virtual void step() = 0;
    
    virtual ~dispatcher() {}
};




using tinfra::subprocess;
using tinfra::io::stream;
using std::auto_ptr;
using std::string;

static void initialize_async_file(int file)
{
    tinfra::io::socket::set_blocking(file, false);
}

class connection_listener: public tinfra::aio::acceptor_listener {
public:
    virtual void accept_connection(Dispatcher&, 
                                   std::auto_ptr<stream>&, 
                                   std::string const&);
};

void connection_listener::accept_connection(
                        Dispatcher& dispatcher,
                        std::auto_ptr<stream>& client,
                        std::string const&)
{
    
    dispatcher->add( client.get(), Dispatcher::READ, &cl );
    
    
}

std::string fake_response;
static void build_fake_response()
{
    std::string fake_str = "abcdefgh\r\n";
    unsigned size = 10000000;
    fake_response.reserve(size + fake_str.size());
    for(unsigned i = 0; i < size/fake_str.size(); i++ ) {
            fake_response.append(fake_str);
    }
}

const int DEFAULT_PORT = 10456;

int listen(int port)
{
    auto_ptr<stream> listen_stream = tinfra::aio::create_service_stream("", port);
    
    request_pool        rp;
    connection_listener cl(rp);
    
    auto_ptr<Dispatcher> dispatcher = tinfra::aio::Dispatcher::create();
    
    dispatcher->add( listen_stream.get(), Dispatcher::READ, &cl ) ;
    
    while( ! service.is_finished() ) {
        dispatcher->step();
        
        tinfra::test_interrupt();
    }
    return 0;
}



int https_main(int argc, char** argv)
{
    return listen(DEFAULT_PORT);
}

TINFRA_MAIN(https_main);


