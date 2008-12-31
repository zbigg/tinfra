//
// Copyright (C) 2008 Zbigniew Zagorski <z.zagorski@gmail.com>,
// licensed to the public under the terms of the GNU GPL (>= 2)
// see the file COPYING for details
// I.e., do what you like, but keep copyright and there's NO WARRANTY.
//

#include <stdexcept>
#include <string>

#include <iostream>
#include <memory>

#include "tinfra/aio.h"
#include "tinfra/fmt.h"
#include "tinfra/connection_handler.h"


namespace tinfra {
namespace aio {

namespace detail {
	
template <typename T>
class dynamic_listener: public listener {
	typedef std::auto_ptr<T> value_type;
	
	listener&   delegate_;
	value_type  value_;
public:
	dynamic_listener(listener& delegate, T* object):
		delegate_(delegate),
		value_(object) 
	{}
	
	dynamic_listener(listener& delegate, value_type& object):
		delegate_(delegate),
		value_(object) 
	{}
	virtual void event(dispatcher& d, stream* c, int event)
	{
		delegate_.event(d,c,event);
	}
	virtual void failure(dispatcher& d, stream* c, int error)
	{
		delegate_.failure(d,c,error);
	}
	virtual void removed(dispatcher& d, stream* c)
	{
		delegate_.removed(d,c);
		delete this;
	}
};

} // end namespace detail

void connection_handler_aio_adapter::accept_connection(
                        dispatcher& dispatcher,
                        std::auto_ptr<stream>& client_stream,
                        std::string const& client_address)
{
	stream* client_stream_tmp = client_stream.get();
	std::auto_ptr<connection_handler> handler( 
	    handler_factory_.create(client_stream, client_address) );
		 
	listener* dl = new detail::dynamic_listener<connection_handler>(
	    handler->get_aio_listener(), 
	    handler);
	
	dispatcher.add( client_stream_tmp, dl, dispatcher::READ );
}

void connection_handler_aio_adapter::failure(dispatcher& d, stream* c, int error)
{
	throw std::runtime_error(tinfra::fmt("listener stream failure: %s") % error);
}

} } // end namespace tinfra::aio

