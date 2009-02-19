#include "callfwd.h"

#include <iostream>
#include <sstream>

#include "tinfra/tinfra.h"

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
    
    template <typename T>
    void managed_struct(T const& v, tinfra::symbol const&)
    {
        tinfra::process(v, *this);
    }
    
    void operator()(tinfra::symbol const& sym, info_signal const&)
    {
        put(sym, "<info_signal>");
    }
    
    void operator()(tinfra::symbol const& sym, shutdown_signal const&)
    {
        put(sym, "<shutdown_signal>");
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
    

    void operator()(tinfra::symbol const& sym, std::string& v)
    {
        v = readline();
    }
    
    template <typename T>
    void managed_struct(T& v, tinfra::symbol const&)
    {
        tinfra::mutate(v, *this);
    }
    
    void operator()(tinfra::symbol const&, info_signal&)
    {
        readline();
    }
    
    void operator()(tinfra::symbol const&, shutdown_signal&)
    {
        readline();
    }
};

} // end namespace dummy_protocol

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
        std::ostringstream bufout;
        dummy_protocol::writer w(bufout);
        callfwd::call_sender<dummy_protocol::writer> snd(w);
        foo("send", snd);
        
        std::cout << "RECORDED\n" << bufout.str();
        
        std::cout << "REPLAYING\n";
        
        std::istringstream bufin(bufout.str());
        dummy_protocol::reader r(bufin);
        
        my_server srv;
        callfwd::call_receiver<my_server, dummy_protocol::reader> cf(srv, r);
        
        cf.register_message<info_signal>();
        cf.register_message<shutdown_signal>();
        cf.register_message<std::string>();
        while( !!bufin ) {
            cf.pull();
        }
    }
    
}
