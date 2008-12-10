//
// Copyright (C) 2008 Zbigniew Zagorski <z.zagorski@gmail.com>,
// licensed to the public under the terms of the GNU GPL (>= 2)
// see the file COPYING for details
// I.e., do what you like, but keep copyright and there's NO WARRANTY.
//

#ifndef __tinfra_aio_net_h__
#define __tinfra_aio_net_h__

#include <string>
#include <memory>
#include "tinfra/io/stream.h"

#include "aio.h"

namespace tinfra {
namespace aio {

std::auto_ptr<stream>     create_service_stream(std::string const& address, int port);
std::auto_ptr<stream>     create_client_stream(std::string const& address, int port);

class acceptor_listener: public Listener {
public:
    virtual void event(Dispatcher& d, 
                       stream* listener_stream, 
                       int event);

    virtual void accept_connection(Dispatcher& dispatcher, 
                                   std::auto_ptr<stream>& client_conn, 
                                   std::string const& client_address) = 0;
}; 

}} // end namespace tinfra::aio

#endif // __tinfra_aio_net_h__

// jedit: :tabSize=8:indentSize=4:noTabs=true:mode=c++:
