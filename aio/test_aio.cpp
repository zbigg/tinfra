
#include <unittest++/UnitTest++.h>

#include <memory>
#include <string>

#include "tinfra/aio.h"
#include "tinfra/connection_handler.h"

SUITE(tinfra_aio) {
    using tinfra::aio::connection_handler;
    using tinfra::aio::connection_handler_aio_adapter;
    
    using tinfra::aio::dispatcher;
    using tinfra::io::stream;
    using std::auto_ptr;
    
    struct foo_connection_factory: public connection_handler::factory_type {        
        connection_handler* create(auto_ptr<stream> sss, std::string const& address)
        {
            return 0;
        }
    };
    
    
    
    TEST(api_compilation) {
        auto_ptr<stream> listen_stream = tinfra::aio::create_service_stream("", 8392);
	
        foo_connection_factory factory_instance;
	connection_handler_aio_adapter cl(factory_instance);
	
	auto_ptr<dispatcher> D = tinfra::aio::dispatcher::create();	
	D->add( listen_stream.get(), &cl, dispatcher::READ);
	
	bool should_continue = false; // it's API test only
	
	while( should_continue ) {
		D->step();
	
		//tinfra::test_interrupt();
	}
    }
}

// jedit: :tabSize=8:indentSize=4:noTabs=true:mode=c++
