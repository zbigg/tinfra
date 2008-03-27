#include "tinfra/io/stream.h"
#include "tinfra/io/socket.h"
#include "tinfra/fmt.h"

#include <sstream>
#include <iostream>
#include <cstring>
#include <cctype>
#include <algorithm>

using namespace std;

namespace tinfra {
namespace io {

//
// general IO connectors
//

stream* open_socket(char const* address, int port)
{
    return socket::open_client_socket(address, port);
}

stream* open_command_pipe(char const* command, ios::openmode mode)
{
    throw io_exception("command pipe: unimplemented");
}

stream* open_anon_pipe()
{    
    throw io_exception("anon_pipe: unimplemented");
}

//
// zstreambuf implementation
//

zstreambuf::zstreambuf(stream* stream, bool own)
    : stream_(stream), own_(own),
      buffer_(0), buffer_size_(0), own_buffer_(false)
{
}

zstreambuf::zstreambuf(char const* name, ios::openmode mode) 
    : stream_(0), own_(false),
      buffer_(0), buffer_size_(0), own_buffer_(false)
{
    open_file(name, mode);
}

zstreambuf::~zstreambuf() { 
    close();
}

zstreambuf& zstreambuf::open_file(char const* name, ios::openmode mode)
{
    close();
    stream_ = tinfra::io::open_file(name, mode);
    own_ = true;
    return *this;
}

zstreambuf& zstreambuf::open_socket(char const* target, int port)
{
    close();
    stream_ = tinfra::io::open_socket(target, port);    
    own_ = true;
    return *this;
    }

zstreambuf& zstreambuf::open_pipe(char const* command, ios::openmode mode)
{
    close();
    stream_ = tinfra::io::open_command_pipe(command, mode);
    own_ = true;
    return *this;
}

zstreambuf& zstreambuf::open_pipe()
{
    close();
    stream_ = tinfra::io::open_anon_pipe();
    own_ = true;
    return *this;
}

void zstreambuf::close()
{
    setbuf(0, 0);
    if(own_) {
        stream_->close();
        delete stream_;
    }
    stream_ = 0;
    own_ = false; 
}

streambuf* zstreambuf::setbuf (char * buffer, streamsize buffer_size)
{
    if( own_buffer_ )
        delete [] buffer_;
    buffer_ = buffer;
    buffer_size_ = buffer_size;
    own_buffer_ = false;
    return this;
}

static const int default_buffer_size =  32768;

bool zstreambuf::need_buf()
{
    // TODO: some condition for controlling buffer support
    //       and default buffer size
    
    if( buffer_ ) {
        return true;
    } else if( default_buffer_size > 0 ) {
        buffer_ = new char[default_buffer_size];
        buffer_size_ = default_buffer_size;
        own_buffer_ = true;
    } else {
        return false;
    }
}

int zstreambuf::sync() {
    //zprintf("sync\n");
    if( pptr() > pbase() ) {            
        write(pbase(), pptr() - pbase());
        setp(buffer_, buffer_ + buffer_size_);
    }
    stream_->sync();
    return 0;
}

streamsize zstreambuf::showmanyc ()
{
    return 0;
}

//
// input implementation
//
zstreambuf::int_type zstreambuf::underflow ()
{
    //zprintf("underflow\n");
    if( need_buf() ) {
        int read_result = read(buffer_, buffer_size_);
        if( read_result == 0 )
            return streambuf::traits_type::eof();
        setg( buffer_, buffer_, buffer_ + read_result);
        return *gptr();
    } else {
        char a[1];
        int read_result = read(a, 1);
        if( read_result == 0 )
            return streambuf::traits_type::eof();
        return a[0];
    }
}

zstreambuf::int_type zstreambuf::uflow ()
{
    //zprintf("uflow\n");
    int_type result = underflow();
    gbump(1);
    return result;
}

zstreambuf::int_type zstreambuf::pbackfail (int_type c)
{
    //zprintf("pbackfail: %i\n", c);
    // XXX: how to implement it?
    //  hint: http://www.cplusplus.com/reference/iostream/streambuf/pbackfail.html
    return streambuf::traits_type::eof();
}
std::streamsize zstreambuf::xsgetn (zstreambuf::char_type *data, std::streamsize size)
{
    std::streamsize readed = 0;
    {
        const std::streamsize available_in_buffer = egptr() - gptr();
        if( available_in_buffer ) {
            const std::streamsize to_read_from_buffer = std::min(size, available_in_buffer);
            std::memcpy(data, gptr(), to_read_from_buffer);
            gbump(to_read_from_buffer);
            if( to_read_from_buffer == size ) 
                return size;
            size -= to_read_from_buffer;
            data += to_read_from_buffer;
            readed += to_read_from_buffer;
        }
    }
    readed += read(data, size);
    return readed;
}

//
// output
//
zstreambuf::int_type zstreambuf::overflow (int_type c)
{
    //zprintf("overflow: %i\n", c);
    if (need_buf() ) {
        if( pptr() > pbase() ) {            
            write(pbase(), pptr() - pbase());
        }
        buffer_[0] = c;
        setp(buffer_, buffer_ + buffer_size_);
        pbump(1);
        return 1;
    } else {
        char theC = c;
        int write_result = write(&theC, 1);
        if( write_result == 0 ) 
            return streambuf::traits_type::eof();
        else
            return write_result;
    }
}

std::streamsize zstreambuf::xsputn (const zstreambuf::char_type *data, std::streamsize size) 
{
    std::streamsize written = 0;
    { // first try ... fill in rest of buffer
        std::streamsize available_buffer  = epptr() - pptr();
        if( available_buffer > 0) {
            std::streamsize to_buffer_len = std::min(size, available_buffer);
            std::memcpy(pptr(), data, to_buffer_len);
            pbump(to_buffer_len);
            if( size == to_buffer_len ) 
                return size;
            written += to_buffer_len;
            data += to_buffer_len;
            size -= to_buffer_len;
        }
    }
    
    if( pptr() > pbase() ) sync();
        
    std::streamsize possible_buffer_size = buffer_size_ ? buffer_size_ : default_buffer_size;
    
    if( size <= possible_buffer_size && need_buf() ) {
        std::streamsize available_buffer  = epptr() - pptr();
        if( available_buffer >= size ) {
            std::memcpy(pptr(), data, size);
            pbump(size);
            return written + size;
        } 
    }
    written += write(data, size);
    return written;
}
// 
// seek
//
zstreambuf::pos_type zstreambuf::seekoff (off_type off, ios::seekdir dir, ios::openmode)
{
    sync();
    setg(0,0,0);
    stream::seek_origin origin = (
                 dir == ios::beg ? stream::start : 
                 dir == ios::cur ? stream::current :
                                   stream::end );    
    return stream_->seek(off, origin);
}

zstreambuf::pos_type zstreambuf::seekpos (pos_type pos, ios::openmode)
{
    return seekoff(pos, ios::beg);
}

int zstreambuf::read(char* dest, int size) {
    int result = stream_->read(dest, size);
    //zprintf("readed: %i\n", result);
    //zwrite(zstdout, dest, result);
    return result;
}

int zstreambuf::write(const char* data, int size) {
    //zprintf("writing: %i\n", size);
    //zwrite(zstdout, data, size);
    return stream_->write(data, size);
}

//
// misc
//

void copy(std::streambuf& in, std::streambuf& out)
{
    char buffer[8192];
    std::streamsize readed;
    while( (readed = in.sgetn(buffer, sizeof(buffer))) > 0 ) 
    {
        std::streamsize written = 0;
        while( written < readed ) 
        {
            std::streamsize wt = out.sputn(buffer + written, readed-written);
            if( wt < 0 ) {
                std::string error_str = "?";
                throw generic_exception(fmt("error writing file: %s") % error_str);
            }
            written += wt;
        }
    }
}
} } //end namespace tinfra::io

