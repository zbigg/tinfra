#include "callfwd.h"

#include <iostream>

#include <tinfra/tinfra.h>

struct signal_type {};
struct shutdown_signal: public signal_type {};
struct info_signal: public signal_type {};

#define LOG(w, e) std::cerr << #w ": " << e << std::endl;
struct my_server {
    bool shutdown;
    int  readed;
    
    my_server():
        shutdown(false), 
        readed(0) 
    {        
    }
    
    void operator()(shutdown_signal)
    {
        LOG(my_server_msg, "shutdown_signal received");
        shutdown = true;
    }
    
    void operator()(info_signal)
    {
        LOG(my_server_msg, "info_signal received");
        
        LOG(my_server_info, "readed: " << readed );
        LOG(my_server_info, "shutdown: " << (int)shutdown );
    }
    
    void operator()(const std::string& input)
    {
        LOG(my_server_msg, "input received");
        readed += input.size();
    }
};

class dummy_writer {
public:
    template <typename T>
    void operator()(tinfra::symbol const&, T const& v)
    {
        std::cout << v << std::endl;
    }
        
    void operator()(tinfra::symbol const&, info_signal const&)
    {
        std::cout << "<>" << std::endl;
    }
    
    void operator()(tinfra::symbol const&, shutdown_signal const&)
    {
        std::cout << "<>" << std::endl;
    }
};
template <typename MSG>
void serialize_message(MSG const& msg, dummy_writer& w)
{
    std::cout << MSG::serial_id << std::endl;
    tinfra::process(msg, w);
}

using tinfra::tstring;

template <typename IMPL>
void foo(tstring const& name, IMPL& target)
{
    LOG(foo-start, name);
    for(int i = 0; i < 20; ++i ) {
        if( (i % 5) == 0 ) 
            target(info_signal());
        std::string akuku = "321";
        target(akuku);
    }
    target(shutdown_signal());
    LOG(foo-end, name);
}

int main()
{
    
    {
        
        my_server r;
        foo("direct", r);
    }
    
    {
        my_server r;
        callfwd::call_forwarder<my_server> cf(r);
        foo("call_forwarder", cf);
    
        LOG(main, "replaying");
        while(! r.shutdown ) {
            cf.pull();
        }
    }
    {
        dummy_writer w;
        callfwd::call_sender<dummy_writer> snd(w);
        foo("send", snd);
    }
}
