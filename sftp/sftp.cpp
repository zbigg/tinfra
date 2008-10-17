#include "tinfra/cmd.h"
#include "tinfra/tinfra.h"

#include "sftp_protocol.h"

#include "tinfra/subprocess.h"

#include <stdexcept>
#include "tinfra/fmt.h"


int request_id = 0;

using namespace sftp;
using tinfra::fmt;

template <typename T>
void send(tinfra::subprocess* p, T const& packet)
{
    std::string packet_buffer;
        
    writer sink(packet_buffer);
    
    packet_header header;    
    header.type       = T::type;
    header.request_id = ++request_id;
    
    tinfra::process(header, sink);
    tinfra::process(packet, sink);
    
    uint32 real_size = packet_buffer.size() - sizeof(uint32);
    uint32 real_size_net = htonl(real_size);
    packet_buffer.replace(0, 
                          sizeof(uint32), reinterpret_cast<const char*>(&real_size_net), 
                          sizeof(uint32));
    
    p->get_stdin()->write( packet_buffer.data(), packet_buffer.size() );
}

void read_for_sure(tinfra::io::stream* s, char* buffer, size_t size)
{
    size_t readed;
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
        const int PACKET_HEADER_SIZE = 9;
        char packet_header_buf[PACKET_HEADER_SIZE];
        
        read_for_sure(p->get_stdout(), packet_header_buf, PACKET_HEADER_SIZE);
        reader rrr(packet_header_buf, PACKET_HEADER_SIZE);
        tinfra::mutate(header, rrr);
        
        if( header.type != T::type ) {
            throw std::runtime_error(fmt("expected packet=%i, actual=%i") % (int)T::type % header.type);
        }
        
        if( header.request_id != request_id ) {
            throw std::runtime_error(fmt("expected rewquest_id=%i, actual=%i") % request_id % header.request_id);
        }
    }
    {
        const int packet_length = header.length - 5;
        char packet_buf[packet_length];
        
        read_for_sure(p->get_stdout(), packet_buf, packet_length);
        reader rrr(packet_buf, packet_length);
        tinfra::mutate(packet, rrr);
    }    
}


int sftp_main(int argc, char** argv)
{
    std::auto_ptr<tinfra::subprocess> sp = tinfra::subprocess::create();
    
    sp->set_stdout_mode(tinfra::subprocess::REDIRECT);
    sp->set_stdin_mode(tinfra::subprocess::REDIRECT);
    
    sp->start("plink -s zagorzbi@us000176 sftp");
    
    {
        init_packet init;
        init.version = 3;
        
        send(sp.get(), init);
        
        version_packet version;
        expect(sp.get(), version);
    }
    sp->terminate();
    
    sp->wait();
    return sp->get_exit_code();
}

TINFRA_MAIN(sftp_main);
