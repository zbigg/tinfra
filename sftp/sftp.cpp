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
        out << " }";
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
void send(tinfra::io::stream* to_server, T const& packet)
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
    to_server->write( packet_buffer.data(), packet_buffer.size() );
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

void expect_header(tinfra::io::stream* s, packet_header& result, byte expected_type)
{
    const int PACKET_HEADER_SIZE = 5;
    char packet_header_buf[PACKET_HEADER_SIZE];
    
    read_for_sure(s, packet_header_buf, PACKET_HEADER_SIZE);
    DOUT( "readed  header: " << hexify(packet_header_buf, PACKET_HEADER_SIZE) );
    
    reader rrr(packet_header_buf, PACKET_HEADER_SIZE);
    tinfra::tt_mutate(result, rrr);
    
    DOUT( "decoded header: " << struct_to_string(result) );
    
    if( result.type != expected_type ) {
        throw std::runtime_error(fmt("expected packet=%i, actual=%i") % expected_type % (int)result.type);
    }
}

template <typename T>
void expect(tinfra::io::stream* from_server, T& packet)
{
    packet_header header;
    expect_header(from_server, header, T::type);
    
    const int packet_length = header.length - sizeof(sftp::byte);
    char packet_buf[packet_length];
    
    read_for_sure(from_server, packet_buf, packet_length);
    
    DOUT( "readed  packet: " << hexify(packet_buf, packet_length) );
    
    reader rrr(packet_buf, packet_length);
    tinfra::tt_mutate(packet, rrr);
    DOUT( "decoded packet: " << struct_to_string(packet));
}

class sftp_vfs {
public:
    virtual ~vfs() {}
    
    virtual tinfra::fs::file_name_list roots()
    {
    }
    
    virtual void list_files(const char* path, tinfra::fs::file_list_visitor& visitor)
    {
        std::string handle
        {            
            open_dir_packet request;
            request.request_id = get_next_request_id();
            request.path = path;
            
            send(to_server, request);
            
            handle_packet response;
            expect(from_server, response);
            
            handle = response.handle;
        }
        {
            read_dir_packet request;
            request.request_id = get_next_request_id();
            request.handle = handle;
            send(to_server, request);
            
            name_packet response;
            expect(from_server, response);
            
            for( prefixed_list<name_element>::const_iterator i = response.elements.begin();
                 i != response.elements.end();
                 ++i )
            {
                visitor.accept(i->filename.c_str());
            }
        }
        { // TODO: this handle should be closed even in case of exception 
          // unconditionally!
            close_packet request;
            request.request_id = get_next_request_id();
            request.handle = handle;
            
            send(to_server, request);
            
            status_packet response;
            expect(from_server, response);
        }
    }
    
    virtual tinfra::fs::file_info stat(const char* path)
    {
        stat_packet request;
        request.request_id = get_next_request_id();
        request.path = path;
        
        send(to_server, request);
        
        attrs_packet response;
        expect(from_server), response);
        
        tinfra::fs::file_info result;
        result.size   = response.size;
        result.is_dir = S_ISDIR(response.permissions);
        result.modification_time = response.mtime;
        result.access_time = response.atime;
        
        return result;
    }
    
    virtual tinfra::io::stream* open(const char* path, tinfra::io::openmode mode)
    {
    }

    virtual void rm(const char* name)
    {
    }

    virtual void rmdir(const char* name)
    {
    }
    
    virtual void mkdir(const char* name)
    {
    }
        
    virtual void copy(const char* src, const char* dest)
    {
    }

    virtual void mv(const char* src, const char* dst)
    {
    }
    
    virtual void recursive_copy(const char* src, const char* dest)
    {
    }
    
    virtual void recursive_rm(const char* src)
    {
    }
        
    virtual bool is_file(const char* name)
    {
    }
    virtual bool is_dir(const char* name)
    {
    }
    virtual bool exists(const char* name)
    {
    }
private:
    
    sftp_vfs(std::string const& target, std::string const& command):
        base_command(command),
        target(target),
        ssh(tinfra::subprocess::create()),
        to_server(0), from_server(0),
        next_request_id(0)
    {
        start();
    }
    
    void close()
    {
        to_server->close();
        ssh->wait();
    }
    
    void start()
    {
        string command = tinfra::fmt("%s %s sftp") % base_command % target;
        ssh->set_stdout_mode(tinfra::subprocess::REDIRECT);
        ssh->set_stdin_mode(tinfra::subprocess::REDIRECT);
    
        ssh->start("ssh -s localhost sftp");
        
        from_server = ssh->get_stdout();
        to_server   = ssh->get_stdin();
        
        initialize_protocol();
    }
    
    void initialize_protocol()
    {
        init_packet init;
        init.version = 2;
        
        send(sp.get(), init);
        
        version_packet version;
        expect(sp.get(), version);
    }
    
    int get_next_request_id() {
        return ++next_request_id;
    }
    
    std::string base_command;
    std::string target;
    
    std::auto_ptr<tinfra::subprocess> ssh;
    
    tinfra::io::stream* from_server;
    tinfra::io::stream* to_server;
    
    int next_request_id;
};


int sftp_main(int argc, char** argv)
{
    tinfra::set_interrupt_policy(tinfra::DEFERRED_SIGNAL);
    
    sftp_vfs fs("localhost", "ssh -s");
    
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
    sp->get_stdin()->close();
    //sp->terminate();
    
    sp->wait();
    return sp->get_exit_code();
}

TINFRA_MAIN(sftp_main);
