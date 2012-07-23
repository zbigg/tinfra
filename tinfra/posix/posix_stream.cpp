//
// Copyright (c) 2010-2011, Zbigniew Zagorski
// This software licensed under terms described in LICENSE.txt
//

#include "tinfra/posix/posix_stream.h" // we implement these

#include "tinfra/os_common.h"
#include "tinfra/trace.h"
#include "tinfra/logger.h"

#include <unistd.h>
#include <errno.h>

namespace tinfra {
    
posix::standard_handle_input  in;
posix::standard_handle_output out(1);
posix::standard_handle_output err(2);

namespace posix {

static int close_nothrow(int fd)
{
    int rc = ::close(fd);
    return rc;
}

//
// file_descriptor
//

file_descriptor::file_descriptor(int fd_, bool own_):
    fd(fd_),
    own(own_)
{
}

file_descriptor::~file_descriptor()
{
    if( this->fd != -1 && this->own) { 
        int r = close_nothrow(this->fd);
        if( r == -1 ) {
            TINFRA_LOG_ERROR("close failed in destructor");
        }
    }
}

void file_descriptor::reset(int fd_, bool own_)
{
    if( this->fd != -1 )
        close();
    this->fd = fd_;
    this->own = own_;
}

void file_descriptor::close()
{
    if( this->own && this->fd != -1 ) {
        if( close_nothrow(this->fd) == -1 ) {
            throw_errno_error(errno, "close failed");
        }
    }
    this->fd = -1;
}

//
// native_input_stream
//
native_input_stream::native_input_stream(int existing, bool own):
    fd(existing, own)
{
}
native_input_stream::~native_input_stream()
{
}
    
// input_stream implementation
void native_input_stream::close()
{
    fd.close();
}
int native_input_stream::read(char* dest, int size)
{
    const int file = this->fd.handle();
    while( true ) {
        int r = ::read(file, dest, size);
        if( r < 0 && errno == EINTR ) 
            continue;
        if( r < 0 ) 
            throw_errno_error(errno, "read from stdin failed");
        return r;
     }
}

//
// native_output_stream
//

native_output_stream::native_output_stream(int existing, bool own):
    fd(existing, own)
{
}

native_output_stream::~native_output_stream()
{
}

void native_output_stream::close()
{
    fd.close();
}
int native_output_stream::write(const char* data, int size)
{
    const int file = this->fd.handle();
    while( true ) {
        int w = ::write(file, data, size);
        if( w < 0 && errno == EINTR )
            continue;
        if( w < 0 ) 
            throw_errno_error(errno, "write failed");
        return w;
    }
}
void native_output_stream::sync()
{
}


//
// standard_handle_input
//

standard_handle_input::standard_handle_input()
{
}

standard_handle_input::~standard_handle_input()
{
}

void standard_handle_input::close()
{
}

int standard_handle_input::read(char* data, int size)
{
    while( true ) {
        int r = ::read(0, data, size);
        if( r < 0 && errno == EINTR ) 
            continue;
        if( r < 0 ) 
            throw_errno_error(errno, "read from stdin failed");
        return r;
    }
}

//
// standard_handle_output
//

standard_handle_output::standard_handle_output(int fd):
    fd_(fd)
{
}

standard_handle_output::~standard_handle_output()
{
}

void standard_handle_output::close()
{
}

int standard_handle_output::write(const char* data, int size) 
{
    while( true ) {
        int w = ::write(fd_, data, size);
        if( w < 0 && errno == EINTR )
            continue;
        if( w < 0 ) {
            //std::string msg
            //throw_errno_error(errno, "write failed");
        }
        return w;
    }
}

void standard_handle_output::sync()
{
}

} // end namespace tinfra::posix
} // end namespace tinfra

