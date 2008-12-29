//
// Copyright (C) 2008 Zbigniew Zagorski <z.zagorski@gmail.com>,
// licensed to the public under the terms of the GNU GPL (>= 2)
// see the file COPYING for details
// I.e., do what you like, but keep copyright and there's NO WARRANTY.
//

#ifndef __tinfra_aio_connection_handler_h__
#define __tinfra_aio_connection_handler_h__

#include <iostream>
#include <string>
#include <memory>

#include "tinfra/aio.h"
#include "tinfra/aio_net.h"
#include "tinfra/generic_factory.h"

namespace tinfra {
namespace aio {

class connection_handler {
public:
	virtual listener& get_aio_listener() = 0;
	virtual ~connection_handler() {}
	
	typedef generic_factory_base2<connection_handler, 
	                              std::auto_ptr<tinfra::aio::stream>, 
				      std::string const&> 
		factory_type;
};

class connection_handler_aio_adapter: public tinfra::aio::connection_listener {
	connection_handler::factory_type& handler_factory_;
	
public:
	connection_handler_aio_adapter(connection_handler::factory_type& handler_factory):
	       handler_factory_(handler_factory)
	{}
	
    	virtual void accept_connection(dispatcher&, 
	                               std::auto_ptr<stream>&, 
	                               std::string const&);
	
	virtual void failure(dispatcher&, stream*, int);
};

class generic_connection_handler: public connection_handler {
	std::auto_ptr<tinfra::aio::stream> stream_;
	std::string client_address_;
public:
	typedef generic_factory_impl2<generic_connection_handler, connection_handler::factory_type> 
		factory_type;
	
	generic_connection_handler(std::auto_ptr<tinfra::aio::stream> stream, 
				      std::string const& client_address)
		: stream_(stream),
		  client_address_(client_address)
	{
	}
	
	tinfra::aio::stream* get_stream() const { 
		return stream_.get(); 
	}
	
	std::string          get_client_address() const {
		return client_address_;
	}
};

} } // end namespace tinfra::aio

#endif // __tinfra_aio_connection_handler_h__


