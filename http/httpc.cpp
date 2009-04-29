class http_client_connection_handler: public generic_connection_handler, public tinfra::http::protocol_listener {
public:
	typedef generic_factory_impl2<http_client_connection_handler, connection_handler::factory_type> 
		factory_type;
		
	http_server_connection_handler(std::auto_ptr<tinfra::aio::stream> stream, 
	                                std::string const& client_address)
		: generic_connection_handler(stream, client_address),
		  protocol(*this, tinfra::http::protocol_parser::CLIENT),
		  aio_adapter(protocol)
	{
		TINFRA_TRACE_MSG("http_server_connection_handler created");
	}
	
	~http_server_connection_handler()
	{
		TINFRA_TRACE_MSG("http_server_connection_handler destroyed");
	}
	
	listener& get_aio_listener() { return aio_adapter; }
	
	virtual void request_line(tstring const& method, 
	                          tstring const& request_uri,
	                          tstring const& proto_version)
	{
		// not used, we're HTTP client
	}
	virtual void response_line(tstring const& proto, tstring const& status, tstring const& status_message) 
	{
		
	}
	virtual void header(tstring const& name, tstring const& value)
	{
		
	}
	virtual void finished_headers()
	{
	}
	
	virtual void content(tstring const& data, bool last)
	{
	}
private:
	tinfra::http::protocol_parser protocol;
	tinfra::aio::buffered_aio_adapter aio_adapter;
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
    
    {
        auto_ptr<dispatcher> dispatcher = tinfra::aio::dispatcher::create();	
        dispatcher->add( listen_stream.get(), &cl, dispatcher::READ) ;
        
        bool should_continue = true;
        
        while( should_continue ) {
                dispatcher->step();
        
                tinfra::test_interrupt();
        }
    }
    return 0;
}
