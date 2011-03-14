#include <tinfra/tcp_socket.h>
#include <tinfra/cmd.h> // for TINFRA_MAIN
#include <tinfra/cli.h>
#include <tinfra/option.h>
#include <tinfra/shared_ptr.h>
#include <tinfra/thread_runner.h>

using tinfra::tcp_client_socket;

void parse_input_stream_parsing_adapter(tinfra::input_stream& is, tinfra::parser& parser)
{
    
};

struct connection_job {
    tinfra::shared_ptr<tcp_client_socket> socket;
    
    void operator()() 
    {
        using tinfra::http:protocol_parser;
        tinfra::input_stream
        
        protocol_parser parser(protocol_parser::SERVER, 
    }
};

struct server_job {
    tinfra::tcp_server_socket& server_socket;
    tinfra::runner&            runner;
    
    void operator()()
    {
        bool finished = false;
        
        while(! finished ) {
            std::string address;
            tinfra::shared_ptr<tcp_client_socket> client_socket( server_socket.accept(address).release() );
            
            //client_socket(client_auto.release());
            connection_job new_job;
            new_job.socket = client_socket;
            runner(new_job);
        }
    }
};

tinfra::option<int> option_port(8080, 'p', "port", "listen port");
tinfra::option<std::string> option_address("", 'b', "bind-address", "bind address (default all)");

int httpd_main_real(tinfra::tstring const&, std::vector<tinfra::tstring>&)
{
    tinfra::thread_runner runner;
    tinfra::tcp_server_socket server_socket(option_address.value(), option_port.value());
    
    server_job server = { 
        server_socket, 
        runner 
    };
    
    server();
    return 0;
}

int httpd_main(int argc, char** argv)
{
    return tinfra::cli_main(argc, argv, &httpd_main_real);
}

TINFRA_MAIN(httpd_main);

