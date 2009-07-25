#include <unittest++/UnitTest++.h>


#include "tinfra/callfwd.h"

#include <iostream>
#include <sstream>

#include "tinfra/tinfra.h"

struct signal_type {};
struct shutdown_signal: public signal_type {};
struct info_signal: public signal_type {};

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
        shutdown = true;
    }
    
    void operator()(info_signal)
    {
    }
    
    void operator()(const std::string& input)
    {
        readed += input.size();
    }
};

namespace dummy_protocol {
class writer {
    typedef std::ostream ostream;
    ostream& out_;
public:
    writer(ostream& out): out_(out) {}
        
    template <typename T>
    void operator()(tinfra::symbol const& sym, T const& v)
    {
        put(sym, v);
    }
    
    void operator()(tinfra::symbol const& sym, info_signal const&)
    {
        put(sym, "<info_signal>");
    }
    
    void operator()(tinfra::symbol const& sym, shutdown_signal const&)
    {
        put(sym, "<shutdown_signal>");
    }
    
    template <typename T>
    void mstruct(tinfra::symbol const&, T const& v)
    {
        tinfra::mo_process(v, *this);
    }
    
private:
    template <typename T>
    void put(tinfra::symbol const& sym, T const& v)
    {
        out_ << sym << ": " << v << std::endl;
    }
};

class reader {
private:
    typedef std::istream istream;
    istream& in_;
public:
    reader(istream& in): in_(in) {}
    
    std::string readline() {
        std::string result;
        getline(in_, result);
        size_t p = result.find_first_of(' ');
        result = result.substr(p+1);
        return result;
    }
    

    void operator()(tinfra::symbol const& , std::string& v)
    {
        v = readline();
    }
    
    void operator()(tinfra::symbol const&, info_signal&)
    {
        readline();
    }
    
    void operator()(tinfra::symbol const&, shutdown_signal&)
    {
        readline();
    }
    
    template <typename T>
    void mstruct(tinfra::symbol const&, T& v)
    {
        tinfra::mo_mutate(v, *this);
    }
};

} // end namespace dummy_protocol

using tinfra::tstring;

template <typename IMPL>
void foo(tstring const& name, IMPL& target)
{
    for(int i = 0; i < 20; ++i ) {
        if( (i % 5) == 0 ) 
            target(info_signal());
        std::string akuku = "321";
        target(akuku);
    }
    target(shutdown_signal());
}

SUITE(tinfra) {
    // TODO: add asserions about actual work
    // Currently these test check only API of
    // - callfwd::delegator
    // - callfwd::process
    // - callfwd::call_sender
    
    TEST(callfwd_delegator)
    {
        my_server r;
        callfwd::call_queue q;
        callfwd::delegator<my_server> cf(q);
        foo("call_forwarder", cf);
    
        while(! r.shutdown ) {
            callfwd::process(q, r);
        }
        
        // TODO: add actual checks to this test
    }
    
    TEST(callfwd_send_receive)
    {
        std::ostringstream bufout;
        dummy_protocol::writer w(bufout);
        callfwd::call_sender<dummy_protocol::writer> snd(w);
        foo("send", snd);
                
        std::istringstream bufin(bufout.str());
        dummy_protocol::reader source(bufin);
        
        my_server srv;
        callfwd::dispatch_map<my_server, dummy_protocol::reader> dispatcher;
        
        dispatcher.register_message<info_signal>();
        dispatcher.register_message<shutdown_signal>();
        dispatcher.register_message<std::string>();
        
        while( !srv.shutdown ) {
            callfwd::process(source, dispatcher, srv);
        }
        // TODO: add actual checks to this test
    }
}
