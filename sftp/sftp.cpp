#include "tinfra/cmd.h"
#include "tinfra/tinfra.h"

#include "sftp_protocol.h"

#include "tinfra/subprocess.h"

#include <stdexcept>
#include <iostream>
#include <sstream>
#include <iomanip>
#include <iterator>

#include "tinfra/fmt.h"


int request_id = 0;

using namespace sftp;
using tinfra::fmt;

class field_printer {
    std::ostream& out;
    bool need_separator;
public:
    field_printer(std::ostream& o) : out(o), need_separator(false) {}
    
    template <class T>
    void operator () (const tinfra::symbol& s, T const& t) 
        {
        if( need_separator ) {
            out << ", ";            
        }
        out << s.str() << " = " << t;
        need_separator = true;
    }
    
    template <typename T>
    void managed_struct(T const& v, tinfra::symbol const& s)
    {
        if( need_separator ) 
            out << ", ";
        out << "{ ";
        need_separator = false;
        tinfra::tt_process<T>(v, *this);
        out << " } ";
        need_separator = true;
    }
};

template<typename T>
std::string struct_to_string(T const& v)
{
    std::ostringstream s;
    field_printer printer(s);
    
    s << "{ ";
    tinfra::tt_process(v, printer);
    s << "}";
    return s.str();
}

#define DOUT(a) do { std::cerr << __FILE__ << ":" << __LINE__ << ": " << a << std::endl; } while(false)

namespace sftp {
    
    std::ostream& operator<<(std::ostream& s, extension_pair const& l)
    {
        return s << struct_to_string(l);
    }
    
    std::ostream& operator<<(std::ostream& s, name_element const& l)
    {
        return s << struct_to_string(l);
    }
    
    /*
    std::ostream& operator<<(std::ostream& s, extension_pair const& l)
    {
        return s << "{ name = " << l.name << ", " << l.data << " = " << tinfra::escape_c(l.data) << " }";
    }
    */
    
    template <typename T>
    std::ostream& operator<<(std::ostream& s, fill_list<T> const& l)
    {
        s << "[ ";
        std::copy(l.begin(), l.end(), std::ostream_iterator<T>(s, ", "));
        s << "]";
        return s;
    }
    
    template <typename T>
    std::ostream& operator<<(std::ostream& s, prefixed_list<T> const& l)
    {
        s << "[ ";
        std::copy(l.begin(), l.end(), std::ostream_iterator<T>(s, ", "));
        s << "]";
        return s;
    }
}

std::string hexify(const char* buf, size_t length)
{
    std::ostringstream r;
    for(int i = 0; i < length; ++i ) {
        if( i > 0 )
            r << " ";
        r << std::setw(2) << std::setfill('0') << std::hex << (int)(unsigned char)(buf[i]);
    }
    return r.str();
}


template <typename T>
void send(tinfra::subprocess* p, T const& packet)
{
    std::string packet_buffer;
        
    writer sink(packet_buffer);
    
    packet_header header;    
    header.type       = T::type;    
    
    tinfra::process(header, sink);
    tinfra::process(packet, sink);
    
    uint32 real_size = packet_buffer.size() - sizeof(uint32);
    uint32 real_size_net = htonl(real_size);
    packet_buffer.replace(0, 
                          sizeof(uint32), reinterpret_cast<const char*>(&real_size_net), 
                          sizeof(uint32));
    
    DOUT( "writing packet: " << struct_to_string(packet) );    
    p->get_stdin()->write( packet_buffer.data(), packet_buffer.size() );
    DOUT( "written bytes: " << hexify(packet_buffer.data(), packet_buffer.size()) );
}

void read_for_sure(tinfra::io::stream* s, char* buffer, size_t size)
{
    size_t readed = 0;
    while( readed < size ) {
        int r = s->read(buffer + readed, size - readed);
        if( r <= 0 ) {
            throw std::runtime_error("unexpected EOF");
        }
        readed += r;
    }
}

template <typename T>
void expect(tinfra::subprocess* p, T& packet)
{
    packet_header header;
    {
        const int PACKET_HEADER_SIZE = 5;
        char packet_header_buf[PACKET_HEADER_SIZE];
        
        read_for_sure(p->get_stdout(), packet_header_buf, PACKET_HEADER_SIZE);
        DOUT( "readed  header: " << hexify(packet_header_buf, PACKET_HEADER_SIZE) );
        
        reader rrr(packet_header_buf, PACKET_HEADER_SIZE);
        tinfra::tt_mutate(header, rrr);
        
        DOUT( "decoded header: " << struct_to_string(header) );
        
        if( header.type != T::type ) {
            throw std::runtime_error(fmt("expected packet=%i, actual=%i") % (int)T::type % (int)header.type);
        }
    }
    {
        const int packet_length = header.length - 1;
        char packet_buf[packet_length];
        
        read_for_sure(p->get_stdout(), packet_buf, packet_length);
        
        DOUT( "readed  packet: " << hexify(packet_buf, packet_length) );
        
        reader rrr(packet_buf, packet_length);
        tinfra::tt_mutate(packet, rrr);
        DOUT( "decoded packet: " << struct_to_string(packet));
    }
}


int sftp_main(int argc, char** argv)
{
    std::auto_ptr<tinfra::subprocess> sp = tinfra::subprocess::create();
    
    sp->set_stdout_mode(tinfra::subprocess::REDIRECT);
    sp->set_stdin_mode(tinfra::subprocess::REDIRECT);
    
    sp->start("ssh -s localhost sftp");
    
    {
        init_packet init;
        init.version = 2;
        
        send(sp.get(), init);
        
        version_packet version;
        expect(sp.get(), version);
    }
    
    {
        stat_packet request;
        request.request_id = ++request_id;
        request.path = ".zbiggrc";
        //request.flags = 0xffff & ~(0x2);
        
        send(sp.get(), request);
        
        attrs_packet response;
        expect(sp.get(), response);
    }
    
    {
        open_dir_packet request;
        request.request_id = ++request_id;
        request.path = ".";
        
        send(sp.get(), request);
        
        handle_packet response;
        expect(sp.get(), response);
        
        std::string handle = response.handle;
        
        read_dir_packet request2;
        request2.request_id = ++request_id;
        request2.handle = handle;
        send(sp.get(), request2);
        
        name_packet response2;
        expect(sp.get(), response2);        
    }
    sp->terminate();
    
    sp->wait();
    return sp->get_exit_code();
}

TINFRA_MAIN(sftp_main);
