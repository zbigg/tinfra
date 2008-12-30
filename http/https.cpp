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
#include "tinfra/runtime.h"

#include "tinfra/aio.h"
#include "tinfra/aio_net.h"
#include "tinfra/connection_handler.h"

#include "tinfra/trace.h"

#include "protocol_aio_adapter.h"

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

using tinfra::tstring;
using tinfra::regexp;

struct http_request_processor {
    virtual void request_line(
        tstring const& method, 
        tstring const& request_uri,
        tstring const& proto_version) = 0;
    virtual void header(
        tstring const& name, 
        tstring const& value) = 0;
    virtual void finished_headers();
    virtual void content(
        tstring const& data,
        bool last) = 0;
    
    virtual ~http_request_processor() {}
};

class http_request_parser: public tinfra::lazy_protocol  {
    
    http_request_parser(http_request_processor& p)
        : processor(p)
    {
        setup_initial_state();
    }

    bool is_keep_alive() const { return false; }
    
private:
    http_request_processor& processor;

    static const tinfra::regexp http_11_request_line_re;
    static const tinfra::regexp http_10_request_line_re;

    int request_line(tstring const& s) {        
        wait_for_delimiter("\r\n", 
            make_step_method(&http_request_parser::header_line));
                
        // TODO: store/consume request-line
        tinfra::static_tstring_match_result<4> parsed;
        
        if( http_11_request_line_re.matches(s, parsed)) {
            processor.request_line(parsed.groups[1], parsed.groups[2], parsed.groups[3]);
        } else {
            tinfra::static_tstring_match_result<3> parsed_old;
            if( http_10_request_line_re.matches(s, parsed_old) ) {
                processor.request_line(parsed_old.groups[1], parsed_old.groups[2], "");
            } else {
                throw std::runtime_error("HTTP: bad request-line");
            }
        }
        return s.size();
    }
    
    int header_line(tstring const& s) {
        if( s.size() > 2 ) {
            size_t pos = s.find_first_of(':');
            if( pos == tstring::npos )
                throw std::runtime_error("HTTP: bad header");
            
            tstring name(s.data(), pos-1);
            tstring value(s.data() + pos, s.size()-pos-1);
            
            processor.header(name, value);
            
            try {
                handle_protocol_header(name, value);
            } catch( std::exception const& e) {
                throw std::runtime_error("HTTP: bad header");
            }
        } else if( s.size() == 2 ) {
            processor.finished_headers();
            // headers ENDED
            // somehow find content-length
            readed_content_length = 0;
            
            setup_content_retrieval();
        }
        return s.size();
    }
    
    void setup_initial_state()
    {
        wait_for_delimiter("\r\n",
            make_step_method(&http_request_parser::request_line));
        requested_content_length = tstring::npos;
    }
    
    void setup_content_retrieval()
    {        
        size_t remaining_size = requested_content_length - readed_content_length;
        
        if( requested_content_length == tstring::npos ) {
            next(make_step_method(&http_request_parser::content_bytes));
        } if( remaining_size > 0 ) {
            next(make_step_method(&http_request_parser::content_bytes));
        } else if( is_keep_alive() ) {
            setup_initial_state();
        } else {
            finish();
        }
    }
    
    void handle_protocol_header(tstring const& name, tstring const& value)
    {        
        
        if( name == "content-length" ) {
            int n;
            tinfra::from_string(value, n);
            if( n < 0 ) {
                throw std::runtime_error("invalid (negative) content-length");
            }
            requested_content_length = n;                        
            check_content_length(requested_content_length);
        }
    }
    
    void check_content_length(size_t length)
    {
        const size_t max_content_length = 1024*1024*10;
        if( length >= max_content_length ) {
            throw std::runtime_error("to big content-length");
        }
    }
    
    int content_bytes(tstring const& s) {
        size_t remaining_size = requested_content_length - readed_content_length;        
        size_t current_size = std::min(remaining_size, s.size());
        
        readed_content_length += current_size;
        
        check_content_length(readed_content_length);
        
        bool last = (readed_content_length == requested_content_length);        
        processor.content(tstring(s.data(), current_size), last);
                
        setup_content_retrieval();
        return current_size;
    }

    size_t requested_content_length;
    size_t readed_content_length;
};

//
// http_request_parser
//

const tinfra::regexp http_request_parser::http_11_request_line_re("^([A-Z]+)[ ]+([^ ]+)[ ]+([A-Z/0-9\\.]+)");
const tinfra::regexp http_request_parser::http_10_request_line_re("^([A-Z]+)[ ]+([^ ]+)");

class http_request_header {
	tstring method;
	tstring request_uri;
	tstring protocol_version;
	std::vector<http_header_entry> headers;
};

struct http_request_builder: public http_request_processor {
	tinfra::string_pool pool;
	virtual void request_line(tstring const& method, 
	                          tstring const& request_uri,
	                          tstring const& proto_version)
	{
		this->method = pool.create(method);
		this->request_uri = pool.create(request_uri);
		this->protocol_version = pool.create(proto_version);
	}
	virtual void header(tstring const& name, tstring const& value)
	{
		http_header_entry tmp;
		tmp.key = pool.create(name);
		tmp.value = pool.create(value);
		this->headers.push_back(tmp);
	}
	virtual void finished_headers()
	{
	}
	virtual void content(tstring const& data, bool last)
	{
	}
};

class http_header_entry {
	tstring key;
	tstring value;
};


class http_server_protocol {
};

const int DEFAULT_PORT = 10456;
using tinfra::aio::connection_handler;
using tinfra::aio::generic_connection_handler;
using tinfra::generic_factory_impl2;
using tinfra::aio::dispatcher;
using tinfra::aio::listener;
using tinfra::aio::stream;

class http_server_connection_handler: public generic_connection_handler {
public:
	typedef generic_factory_impl2<http_server_connection_handler, connection_handler::factory_type> 
		factory_type;
		
	http_server_connection_handler(std::auto_ptr<tinfra::aio::stream> stream, 
	                                std::string const& client_address)
		: generic_connection_handler(stream, client_address),
		  protocol(*this),
		  protocol_adapter(protocol)
	{
		TINFRA_TRACE_MSG("http_server_connection_handler created");
	}
	
	~http_server_connection_handler()
	{
		TINFRA_TRACE_MSG("http_server_connection_handler destroyed");
	}
	
	listener& get_aio_listener() { return protocol_adapter; }
	
private:
	http_server_protocol protocol;
	protocol_aio_adapter protocol_adapter;
};

int listen(int port)
{

	using tinfra::io::stream;
	using std::auto_ptr;
	using tinfra::aio::connection_handler_aio_adapter;
	using tinfra::aio::dispatcher;

	auto_ptr<stream> listen_stream = tinfra::aio::create_service_stream("", port);
	
	http_server_connection_handler::factory_type connection_handler_factory;
	
	connection_handler_aio_adapter cl(connection_handler_factory);
	
	auto_ptr<dispatcher> dispatcher = tinfra::aio::dispatcher::create();
	
	dispatcher->add( listen_stream.get(), &cl, dispatcher::READ) ;
	
	bool should_continue = true;
	
	while( should_continue ) {
		dispatcher->step();
	
		tinfra::test_interrupt();
	}
	return 0;
}



int https_main(int argc, char** argv)
{
	tinfra::set_interrupt_policy(tinfra::DEFERRED_SIGNAL);
	return listen(DEFAULT_PORT);
}

TINFRA_MAIN(https_main);


