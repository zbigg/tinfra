#include "tinfra/io/stream.h"
#include "tinfra/fmt.h"

#include <sstream>
#include <iostream>
#include <cstring>

#include <zcompat/zpio.h>

using namespace std;

namespace tinfra {
namespace detail {

/*
class stream {
    virtual ~stream() {}
    enum {
        start,
        end,
        current
    } seek_origin;        
    void close() = 0;
    int seek(int pos, seek_origin origin = start) = 0;
    int read(char* dest, int size) = 0;
    int write(const char* data, int size) = 0;
};
*/
    
class zcompatstream_: public stream {
    ZSTREAM stream_;
public:
    zcompatstream_(ZSTREAM stream): stream_(stream) {}
    void close() {
        if( stream_ && ::zclose(stream_) == -1 ) {
            throw io_exception(::zstrerror(stream_->error));
        }
        stream_ = 0;
    }
    int seek(int pos, stream::seek_origin origin)
    {
        int whence = origin == stream::start ? ZSEEK_SET :
                     origin == stream::end   ? ZSEEK_END :
                                               ZSEEK_CUR;
        int result = ::zseek(stream_, pos, whence);
        if( result == -1 ) {
            throw io_exception(::zstrerror(stream_->error));
        }
        return result;
    }
    int read(char* data, int size)
    {
        int result = ::zread(stream_, data, size);
        if( result == -1 ) {
            throw io_exception(::zstrerror(stream_->error));
        }
        return result;
    }
    int write(const char* data, int size)
    {
        int result = ::zwrite(stream_, data, size);
        if( result == -1 ) {
            throw io_exception(::zstrerror(stream_->error));
        }
        return result;
    }
    void sync() 
    {
    }
};
    
static int ios_to_zcompat_openmode(ios::openmode mode)
{
    int result = 0;
    if( (mode & ios::in) == ios::in)       result |= ZO_READ;
    if( (mode & ios::out) == ios::out)     result |= ZO_WRITE | ZO_CREAT;
    if( (mode & ios::app) == ios::app)     result |= ZO_APPEND | ZO_CREAT;
    if( (mode & ios::trunc) == ios::trunc) result |= ZO_TRUNC;
    //if( mode & ios::bin == ios::bin)     result |= ZO_BINARY;  else result |= ZO_TEXT;
    return result;
}

static stream* open_file(char const* name, ios::openmode mode)
{
    int zcompat_mode = ios_to_zcompat_openmode(mode);
    ZSTREAM s = zfopen(name, zcompat_mode);
    if( s == 0 ) {
        throw io_exception(fmt("unable to open '%s' : %s") % name % zstrerror(errno));
    }
    return new zcompatstream_(s);
}

static stream* open_socket(char const* address, int port)
{
    ZSTREAM s = zsopen(address,  port);
    if( s == 0 ) {
        throw io_exception(fmt("unable to connect '%s:%i' : %s") % address % port % zstrerror(errno) );
    }
    return new zcompatstream_(s);
}

static stream* open_command_pipe(char const* command, ios::openmode mode)
{
    int zcompat_mode = ios_to_zcompat_openmode(mode);
    ZSTREAM s = ::zpopen(command, zcompat_mode);
    if( s == 0 ) {
        throw io_exception(zstrerror(errno));
    }
    return new zcompatstream_(s);
}

static stream* open_anon_pipe()
{    
    throw io_exception("unimplemented");
}

} //end namespace detail

zstreambuf::zstreambuf(detail::stream* stream, bool own)
    : stream_(stream), own_(own),
      buffer_(0), buffer_size_(0), own_buffer_(false)
{
}

zstreambuf::zstreambuf(char const* name, ios::openmode mode) 
    : stream_(0), own_(false),
      buffer_(0), buffer_size_(0), own_buffer_(false)
{
    stream_ = detail::open_file(name, mode);
    own_ = true;
}

zstreambuf::~zstreambuf() { 
    close();
}

zstreambuf& zstreambuf::open_file(char const* name, ios::openmode mode)
{
    close();
    stream_ = detail::open_file(name, mode);
    own_ = true;
    return *this;
}

zstreambuf& zstreambuf::open_socket(char const* target, int port)
{
    close();
    stream_ = detail::open_socket(target, port);    
    own_ = true;
    return *this;
    }

zstreambuf& zstreambuf::open_pipe(char const* command, ios::openmode mode)
{
    close();
    stream_ = detail::open_socket(command, mode);
    own_ = true;
    return *this;
}

zstreambuf& zstreambuf::open_pipe()
{
    close();
    stream_ = detail::open_anon_pipe();
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
bool zstreambuf::need_buf()
{
    // TODO: some condition for controlling buffer support
    //       and default buffer size
    const int default_buffer_size =  31268;
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
        setg(buffer_, buffer_ + 1, buffer_ + buffer_size_);
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

// 
// seek
//
zstreambuf::pos_type zstreambuf::seekoff (off_type off, ios::seekdir dir, ios::openmode)
{
    detail::stream::seek_origin origin = (
                 dir == ios::beg ? detail::stream::start : 
                 dir == ios::cur ? detail::stream::current :
                                   detail::stream::end );    
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

} //end namespace tinfra

