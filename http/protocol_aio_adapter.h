//
// Copyright (C) 2008 Zbigniew Zagorski <z.zagorski@gmail.com>,
// licensed to the public under the terms of the GNU GPL (>= 2)
// see the file COPYING for details
// I.e., do what you like, but keep copyright and there's NO WARRANTY.
//

#ifndef __tinfra_buffered_connection_handler_h__
#define __tinfra_buffered_connection_handler_h__

#include "tinfra/tstring.h"
#include "tinfra/aio.h"
#include <memory>

namespace tinfra {
namespace aio {

class protocol {
public:
	/** Consume input
	
	   Framework calls when IO has some data buffered.
	   
	   Protocol should consume as much data as he can and then return number 
	   of bytes consumed
	   
	   @returns 0 if that protocol is unable to assemble any message - 
	              IO must gather more data
	   @returns > 0 length of consumed message
	*/
	virtual int   process_input(tinfra::tstring const& input, tinfra::io::stream*) = 0;
	
	/** EOF occurred.
	
	Framework informs that channel has reached EOF when reading.
	*/
	virtual void  eof(tinfra::io::stream*) = 0;
};

class protocol_aio_adapter: public tinfra::aio::listener {
	class buffer_impl;
	class reader_impl;
	class writer_impl;
	
	std::auto_ptr<reader_impl> reader_;
	std::auto_ptr<writer_impl> writer_;
	
	protocol& protocol_;
public:
	protocol_aio_adapter(protocol& p);
	
	virtual void event(dispatcher& d, stream* c, int event);
	
	virtual void failure(dispatcher& d, stream* c, int error);
	
private:
	stream* get_feedback_channel();
};

} }

// end of namespace tinfra::aio

#endif
