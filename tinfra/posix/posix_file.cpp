//
// Copyright (c) 2009, Zbigniew Zagorski
// This software licensed under terms described in LICENSE.txt
//

#include "tinfra/file.h" // we implement this
#include "../platform.h"
#ifdef TINFRA_POSIX

#include "tinfra/fmt.h"
#include "tinfra/os_common.h"
#include "tinfra/runtime.h"
#include <stdexcept>

#include <sys/types.h>
#include <sys/stat.h>
#include <fcntl.h>
#include <errno.h>
#include <unistd.h>

namespace tinfra {

static int close_nothrow(int fd)
{
    int rc = ::close(fd);
    return rc;
}

file::file(handle_type h):
    handle_(h)
{
}

file::file(tstring const& name, int flags):
    handle_(-1)
{
    int open_flags = 0;
    {
	const bool fread  = (flags & FOM_READ) == FOM_READ;
	const bool fwrite = (flags & FOM_WRITE) == FOM_WRITE;
	if( fread && fwrite )
	    open_flags |=  O_RDWR | O_CREAT;
	else if( fread )
	    open_flags |= O_RDONLY;
	else if ( fwrite )
	    open_flags |= O_WRONLY | O_CREAT;
	else
	    throw std::invalid_argument("bad openmode");
	
	if( (flags & FOM_CREATE) == FOM_CREATE)
	    open_flags |= O_CREAT;
	
	if( (flags & FOM_TRUNC) == FOM_TRUNC) 
	    open_flags |= O_TRUNC;
	
	if( (flags & FOM_APPEND) == FOM_APPEND)
	    open_flags |= O_APPEND;
    }
    string_pool temporary_context;  
    const int create_permissions = 00644;
    this->handle_ = ::open(name.c_str(temporary_context), open_flags, create_permissions);
    if( this->handle_ == -1 ) 
        throw_errno_error(errno, fmt("unable to open '%s'") % name);
}
file::~file()
{
    if( this->handle_ != -1 ) {
        if( close_nothrow(this->handle_) == -1 ) {
            // TODO: add silent failures reporting
            // int err = get_last_error();
            // tinfra::silent_failure(fmt("file close failed: %i" % blabla )
        }
        handle_ = -1;
    }
}

void file::close()
{
    if( close_nothrow(this->handle_) == -1 ) 
        throw_errno_error(errno, "close failed");
    this->handle_ = -1;
}

int file::seek(int pos, seek_origin origin)
{
    int whence;
    switch( origin ) {
    case SO_START:
        whence = SEEK_SET;
        break;
    case SO_CURRENT:
        whence = SEEK_CUR;
        break;
    case SO_END:
        whence = SEEK_END;
        break;
    default:
        assert(0);
        // TODO, it should be TINFRA_HARD_ASSERT or what ?
        throw std::logic_error("bad seek_origin");
    }
    off_t e = lseek(handle_, pos, whence);
    if( e == (off_t)-1 )
	throw_errno_error(errno, "seek failed");
    return (int)e;
}

int file::read(char* data, int size)
{
    while( true ) {
        int r = ::read(handle_, data, size);
        if( r < 0 && errno == EINTR ) {
            tinfra::test_interrupt();
            continue;
        }
        if( r < 0 ) 
            throw_errno_error(errno, "read failed");
        return r;
    }
}

int file::write(char const* data, int size)
{
    while( true ) {
        int w = ::write(handle_, data, size);
        if( w < 0 && errno == EINTR ) {
            tinfra::test_interrupt();
            continue;
        }
        if( w < 0 ) 
            throw_errno_error(errno, "write failed");
        return w;
    }
}

fs::file_info file::stat()
{
    throw std::logic_error("file::stat() not implemented");   
}

void file::sync()
{
}

intptr_t file::native() const
{
    return this->handle_;
}

void     file::release()
{
    this->handle_ = -1;
}

} // end namespace tinfra

#endif // TINFRA_POSIX

