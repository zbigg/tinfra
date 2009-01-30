//
// Copyright (C) 2008 Zbigniew Zagorski <z.zagorski@gmail.com>,
// licensed to the public under the terms of the GNU GPL (>= 2)
// see the file COPYING for details
// I.e., do what you like, but keep copyright and there's NO WARRANTY.
//

#include "tinfra/sftp/sftp_protocol.h"
#include "tinfra/sftp/sftp_vfs.h"


#include <stdexcept>
#include <iostream>
#include <sstream>
#include <iomanip>
#include <iterator>
#include <algorithm>

#include "tinfra/tinfra.h"
#include "tinfra/subprocess.h"
#include "tinfra/fmt.h"

#include <sys/stat.h>

// TODO: move to specific utility module
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
        out << tinfra::TypeTraitsGeneric<T>::name() << " { ";
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
    
    s << tinfra::TypeTraitsGeneric<T>::name() << " { ";
    tinfra::tt_process(v, printer);
    s << "}";
    return s.str();
}

std::string hexify(const char* buf, size_t length)
{
    std::ostringstream r;
    for(size_t i = 0; i < length; ++i ) {
        if( i > 0 )
            r << " ";
        r << std::setw(2) << std::setfill('0') << std::hex << (int)(unsigned char)(buf[i]);
    }
    return r.str();
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

#define DOUT(a) do { std::cerr << __FILE__ << ":" << __LINE__ << ": " << a << std::endl; } while(false)

namespace tinfra {
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

template <typename T>
void send(tinfra::io::stream* to_server, T const& packet)
{
    std::string packet_buffer;
        
    writer sink(packet_buffer);
    
    packet_header header;    
    header.type       = T::type;    
    
    tinfra::process(header, sink);
    tinfra::tt_process(packet, sink);
    
    uint32 real_size = packet_buffer.size() - sizeof(uint32);
    uint32 real_size_net = htonl(real_size);
    packet_buffer.replace(0, 
                          sizeof(uint32), reinterpret_cast<const char*>(&real_size_net), 
                          sizeof(uint32));
    
    DOUT( "writing packet: " << struct_to_string(packet) );    
    to_server->write( packet_buffer.data(), packet_buffer.size() );
    //DOUT( "written bytes: " << hexify(packet_buffer.data(), packet_buffer.size()) );
}

void check_status(status_packet& response)
{
    if( response.status_code == SSH_FX_OK ) {
        return;
    }
    
    switch( (int)response.status_code ) {
    // should be ignored here
    case SSH_FX_EOF:
        return;
        
    // general failures which doesn't invalidate connection
    case SSH_FX_NO_SUCH_FILE:
    case SSH_FX_PERMISSION_DENIED:
    case SSH_FX_FAILURE:
        throw std::runtime_error(tinfra::fmt("sftp(%i): %s") % response.status_code % response.error_message);
        
    case SSH_FX_OP_UNSUPPORTED:
        throw std::runtime_error(tinfra::fmt("sftp(%i): %s") % response.status_code % response.error_message);
        
    
    // fatal failures, protocol end
    case SSH_FX_BAD_MESSAGE:
        throw std::runtime_error(tinfra::fmt("sftp(%i): %s") % response.status_code % response.error_message);
    
    case SSH_FX_NO_CONNECTION:
    case SSH_FX_CONNECTION_LOST:
        // protocol violation, abort connection
        throw std::runtime_error(tinfra::fmt("sftp(%i): %s") % response.status_code % response.error_message);
    }
}

template <typename T>
void do_read_packet(tinfra::io::stream* from_server, size_t packet_length, T& packet)
{
    char packet_buf[packet_length];
    
    read_for_sure(from_server, packet_buf, packet_length);
    
    //DOUT( "readed  packet: " << hexify(packet_buf, packet_length) );
    
    reader rrr(packet_buf, packet_length);
    tinfra::tt_mutate(packet, rrr);
    DOUT( "decoded packet: " << struct_to_string(packet));
}

int expect_header(tinfra::io::stream* s, packet_header& result, byte expected_type)
{
    const int PACKET_HEADER_SIZE = 5;
    char packet_header_buf[PACKET_HEADER_SIZE];
    
    read_for_sure(s, packet_header_buf, PACKET_HEADER_SIZE);
    //DOUT( "readed  header: " << hexify(packet_header_buf, PACKET_HEADER_SIZE) );
    
    reader rrr(packet_header_buf, PACKET_HEADER_SIZE);
    tinfra::tt_mutate(result, rrr);
    
    DOUT( "decoded header: " << struct_to_string(result) );
    
    if( result.type == expected_type )
        return SSH_FX_OK;
    
    if( result.type != SSH_FXP_STATUS )
        throw std::runtime_error(fmt("expected packet=%i, actual=%i") % expected_type % (int)result.type);
        
    status_packet status_response;
    const int packet_length = result.length - sizeof(sftp::byte);
    do_read_packet(s, packet_length, status_response);
    
    // throw in case of fatal errors
    // ignore OKs and EOF
    check_status(status_response);
    return status_response.status_code;
}

template <typename T>
int expect(tinfra::io::stream* from_server, T& packet)
{
    packet_header header;
    int r = expect_header(from_server, header, T::type);
    if( r != SSH_FX_OK )
        return r;
    
    const int packet_length = header.length - sizeof(sftp::byte);    
    do_read_packet(from_server, packet_length, packet);    
    return SSH_FX_OK;
}


class sftp_vfs: public tinfra::generic_vfs {
    class remote_handle {
    protected:
        std::string _handle;
        sftp_vfs&   _fs;
        bool        _opened;
    public:
        remote_handle(sftp_vfs& fs): 
            _fs(fs),
            _opened(false)
        {
        }
        
        virtual ~remote_handle() {
            if( close_nothrow() < 0 ) {
                // TODO: silent warning
            }
        }
        
        
        void close()
        {
            if( close_nothrow() < 0 ) {
                throw std::runtime_error("sftp close failed");
            }
        }
        
        operator std::string const&() const { return _handle; }
        
        void open_file(tstring const& name, int flags)
        {
            open_packet request;
            request.request_id = _fs.get_next_request_id();
            request.filename = name.str();
            request.flags = flags;
            
            send(_fs.to_server, request);
            
            handle_packet response;
            expect(_fs.from_server, response);
            
            _handle = response.handle;
            _opened = true;
        }
        
        void open_dir(tstring const& path)
        {
            open_dir_packet request;
            request.request_id = _fs.get_next_request_id();
            request.path = path.str();
            
            send(_fs.to_server, request);
            
            handle_packet response;
            expect(_fs.from_server, response);
            
            _handle = response.handle;
            _opened = true;
        }
        void stat(attr& attrs)
        {
            fstat_packet request;
            request.request_id = _fs.get_next_request_id();
            request.handle = _handle;
            
            send(_fs.to_server, request);
            
            attrs_packet response;
            expect(_fs.from_server, response);
            
            attrs = response.attrs;
        }
    private:
        int close_nothrow()
        {
            if( !_opened )
                return 0;
            close_packet request;
            request.request_id = _fs.get_next_request_id();
            request.handle = _handle;
            
            _opened = false;
            
            send(_fs.to_server, request);
            
            status_packet response;
            expect(_fs.from_server, response);
            
            check_status(response);
            return 0;
        }
        
    };
    
    class remote_file: public tinfra::io::stream, 
                       public remote_handle 
    {
        size_t position;
    public:
        remote_file(sftp_vfs& fs): 
            remote_handle(fs)
        {
        }

        void close()
        {
            remote_handle::close();
        }
        int seek(int pos, seek_origin origin = start)
        {
            switch( origin ) {
            case start: 
                position = pos; 
                break;
            case current: 
                position += pos; 
                break;
            case end:
                {
                    attr file_attrs;
                    stat(file_attrs);
                    position = file_attrs.size + pos;
                    break;
                }
            }
            return position;
        }
        
        int read(char* dest, int size)
        {
            read_packet request;
            request.request_id = _fs.get_next_request_id();
            request.handle = _handle;
            request.offset = position;
            
            send(_fs.to_server, request);
            
            data_packet response;
            
            const int r = expect(_fs.from_server, response);
            if( r == SSH_FX_EOF ) {
                return 0;
            }
            
            int copy_size = std::min(response.data.size(), (size_t)size);
            memcpy(dest, response.data.data(), copy_size);
            position += copy_size;
            return copy_size;
        }
        
        int write(const char* data, int size)
        {
            write_packet request;
            request.request_id = _fs.get_next_request_id();
            request.handle = _handle;
            request.data.assign(data, size);
            request.offset = position;
            
            send(_fs.to_server, request);
            
            status_packet response;
            expect(_fs.from_server, response);
            
            // TODO: check status
            position += size;
            return size;
        }
        
        void sync() 
        {
        }
    
        intptr_t native() const { return -1; }
        void release() {}
    };
public:
    virtual tinfra::fs::file_name_list roots()
    {
        tinfra::fs::file_name_list result;
        result.push_back("/");
        return result;
    }
    
    virtual void list_files(tstring const& path, tinfra::fs::file_list_visitor& visitor)
    {
        remote_handle handle(*this);
        
        handle.open_dir(path);
        while (true ) {
            read_dir_packet request;
            request.request_id = get_next_request_id();
            request.handle = handle;
            send(to_server, request);
            
            name_packet response;
            const int r = expect(from_server, response);
            if( r == SSH_FX_EOF ) 
                break;
            
            for( prefixed_list<name_element>::const_iterator i = response.elements.begin();
                 i != response.elements.end();
                 ++i )
            {
                visitor.accept(i->filename.c_str());
            }
        }
    }
    
    virtual tinfra::fs::file_info stat(tstring const& path)
    {
        stat_packet request;
        request.request_id = get_next_request_id();
        request.path = path.str();
        
        send(to_server, request);
        
        attrs_packet response;
        expect(from_server, response);
        
        tinfra::fs::file_info result;
        result.size   = response.attrs.size;
        result.is_dir = S_ISDIR(response.attrs.permissions);
        result.modification_time = response.attrs.mtime;
        result.access_time = response.attrs.atime;
        
        return result;
    }
    
    virtual tinfra::io::stream* open(tstring const& path, tinfra::io::openmode mode)
    {
        std::auto_ptr<remote_file> result(new remote_file(*this));
        
        result->open_file(path, stdcpp_to_sftp_openmode(mode));
        
        return result.release();
    }
    
    int stdcpp_to_sftp_openmode(tinfra::io::openmode mode)
    {
        int result = 0;
        
	if( (mode & std::ios::in) == std::ios::in )
	    result |= SSH_FXF_READ;
	if ( (mode & std::ios::out) == std::ios::out )
	    result |= SSH_FXF_WRITE | SSH_FXF_CREAT;
	
	if( (mode & std::ios::trunc) == std::ios::trunc) 
            result |= SSH_FXF_TRUNC;
	if( (mode & std::ios::app) == std::ios::app) 
            result |= SSH_FXF_APPEND;
        
        return result;
    }
    
    virtual void rm(tstring const& name)
    {
        remove_packet request;
        request.request_id = get_next_request_id();
        request.filename = name.str();
        
        send(to_server, request);
        
        status_packet response;
        expect(from_server, response);
        
        check_status(response);
    }

    virtual void rmdir(tstring const& name)
    {
        rmdir_packet request;
        request.request_id = get_next_request_id();
        request.path = name.str();
        
        send(to_server, request);
        
        status_packet response;
        expect(from_server, response);
        
        check_status(response);
    }
    
    virtual void mkdir(tstring const& name)
    {
        mkdir_packet request;
        request.request_id = get_next_request_id();
        request.path = name.str();
        
        send(to_server, request);
        
        status_packet response;
        expect(from_server, response);
        
        check_status(response);
    }
        
    

    virtual void mv(tstring const& src, tstring const& dst)
    {
        rename_packet request;
        request.request_id = get_next_request_id();
        request.oldpath = src.str();
        request.newpath = dst.str();
        
        send(to_server, request);
        
        status_packet response;
        expect(from_server, response);
        
        check_status(response);
    }
    /*
    implemented by tinfra::fs::generic_vfs
    
    virtual void copy(const char* src, const char* dest);
    
    virtual void recursive_copy(const char* src, const char* dest);
    
    virtual void recursive_rm(const char* src);
        
    virtual bool is_file(const char* name);
    virtual bool is_dir(const char* name);
    
    virtual bool exists(const char* name);    
    */
    sftp_vfs(std::string const& target, std::string const& command):
        base_command(command),
        target(target),
        ssh(tinfra::subprocess::create()),
        to_server(0), from_server(0),
        next_request_id(0)
    {
        start();
    }
    
    ~sftp_vfs()
    {
        close();
    }
    
    void close()
    {
        to_server->close();
        ssh->wait();
    }
    
private:
    void start()
    {
        string command = tinfra::fmt("%s %s sftp") % base_command % target;
        ssh->set_stdout_mode(tinfra::subprocess::REDIRECT);
        ssh->set_stdin_mode(tinfra::subprocess::REDIRECT);
    
        ssh->start(command.c_str());
        
        from_server = ssh->get_stdout();
        to_server   = ssh->get_stdin();
        
        initialize_protocol();
    }
    
    void initialize_protocol()
    {
        init_packet init;
        init.version = 2;
        
        send(to_server, init);
        
        version_packet version;
        expect(from_server, version);
    }
    
    int get_next_request_id()
    {
        return ++next_request_id;
    }
    
    std::string base_command;
    std::string target;
    
    std::auto_ptr<tinfra::subprocess> ssh;
    
    tinfra::io::stream* to_server;
    tinfra::io::stream* from_server;
    
    int next_request_id;
};

auto_ptr<vfs> create(std::string const& sftp_subsystem_command)
{
    //return auto_ptr<vfs>(new sftp_vfs(sftp_subsystem_command));
    return auto_ptr<vfs>();
}

auto_ptr<vfs> create(std::string const& target, std::string const& ssh_subsystem_command)
{
    return auto_ptr<vfs>(new sftp_vfs(target, ssh_subsystem_command));
}

} } // end namespace tinfra::sftp


