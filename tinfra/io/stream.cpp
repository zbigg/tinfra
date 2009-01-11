//
// Copyright (C) Zbigniew Zagorski <z.zagorski@gmail.com>,
// licensed to the public under the terms of the GNU GPL (>= 2)
// see the file COPYING for details
// I.e., do what you like, but keep copyright and there's NO WARRANTY.
//

#include "tinfra/io/stream.h"
#include "tinfra/io/socket.h"
#include "tinfra/fmt.h"

#include <sstream>
#include <iostream>
#include <cstring>
#include <cctype>
#include <algorithm>
#include <stdexcept>

using namespace std;

namespace tinfra {
namespace io {

//
// general IO connectors
//

stream* open_socket(char const* address, int port)
{
    // THROW_ANALYSIS: not implemented, should be linker error
    return socket::open_client_socket(address, port);
}

stream* open_command_pipe(char const* command, openmode mode)
{
    // THROW_ANALYSIS: not implemented, should be linker error
    throw io_exception("command pipe: unimplemented");
}

stream* open_anon_pipe()
{    
    // THROW_ANALYSIS: not implemented, should be linker error
    throw io_exception("anon_pipe: unimplemented");
}

//
// dstream implementation
//

dstream::dstream(stream* input, stream* output)
: input_(input), output_(output) 
{
}

dstream::~dstream() { 
    delete input_;
    delete output_;
}

void dstream::close() {    
    if( input_ )  input_->close();
    if( output_ ) output_->close();
}
int dstream::seek(int pos, seek_origin origin) {
    int r = 0;
    if( input_ )
        r = input_->seek(pos, origin);
    if( output_ )
        r = output_->seek(pos, origin);
    return r;
}
int dstream::read(char* dest, int size) {
    if( !input_ )
        // THROW_ANALYSIS: assertion, programmer error
        throw io_exception("trying to read from write-only stream");
    return input_->read(dest, size);
}
int dstream::write(const char* data, int size) {
    if( !output_ )
        // THROW_ANALYSIS: assertion, programmer error
        throw io_exception("trying to write to read-only stream");
    return output_->write(data, size);
}
void dstream::sync() {
    if( input_ ) input_->sync();
    if( output_ ) output_->sync();
}

intptr_t dstream::native() const
{
    if( input_ ) return input_->native();
    if( output_ ) return output_->native();
    // THROW_ANALYSIS: assertion, programmer error
    throw std::logic_error("dstream::native: not supported call");
}

void dstream::release()
{
    if( input_ ) input_->release();
    if( output_ ) output_->release();
}
stream* create_dstream(stream* input_, stream* output_)
{
    return new dstream(input_, output_);
}

//
// zstreambuf implementation
//

zstreambuf::zstreambuf(stream* stream, bool own)
    : stream_(stream), own_(own),
      buffer_(0), buffer_size_(0), own_buffer_(false)
{
}

zstreambuf::zstreambuf(char const* name, openmode mode) 
    : stream_(0), own_(false),
      buffer_(0), buffer_size_(0), own_buffer_(false)
{
    open_file(name, mode);
}

zstreambuf::~zstreambuf() { 
    close();
}

zstreambuf& zstreambuf::open_file(char const* name, openmode mode)
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

zstreambuf& zstreambuf::open_pipe(char const* command, openmode mode)
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
        return true;
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
zstreambuf::pos_type zstreambuf::seekoff (off_type off, ios::seekdir dir, openmode)
{
    sync();
    setg(0,0,0);
    stream::seek_origin origin = (
                 dir == ios::beg ? stream::start : 
                 dir == ios::cur ? stream::current :
                                   stream::end );    
    return stream_->seek(off, origin);
}

zstreambuf::pos_type zstreambuf::seekpos (pos_type pos, openmode)
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
static const int COPY_BUFFER_SIZE = 65536;

void copy(stream* in, stream* out, size_t max_bytes)
{
    char buffer[COPY_BUFFER_SIZE];
    // TODO: implement max_bytes > 0 case
    if( max_bytes != 0 )
        // THROW_ANALYSIS: not implemented, laziness
        throw std::logic_error("copy(streams, max_bytes != 0) not implemented");
    int readed;
    while( (readed = in->read(buffer, sizeof(buffer))) > 0 ) {
        int written = 0;
        while( written < readed ) {
            int cw = out->write(buffer + written, readed-written);
            if( cw <= 0 ) {
                // THROW_ANALYSIS: domain/environment event, error
                throw std::runtime_error("error copying file, output stream closed?");
            }
            written += cw;
        }
    }
}

void copy(std::streambuf& in, std::streambuf& out)
{
    char buffer[COPY_BUFFER_SIZE];
    std::streamsize readed;
    while( (readed = in.sgetn(buffer, sizeof(buffer))) > 0 ) 
    {
        std::streamsize written = 0;
        while( written < readed ) 
        {
            std::streamsize wt = out.sputn(buffer + written, readed-written);
            if( wt < 0 ) {
                std::string error_str = "?";
                // THROW_ANALYSIS: domain/environment event, error
                throw generic_exception(fmt("error writing file: %s") % error_str);
            }
            written += wt;
        }
    }
}

} } //end namespace tinfra::io

