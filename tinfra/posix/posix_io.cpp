#include "tinfra/io/stream.h"
#include "tinfra/fmt.h"
#include "tinfra/os_common.h"

#include <stdexcept>

#include <sys/types.h>
#include <sys/stat.h>
#include <fcntl.h>
#include <errno.h>
#include <unistd.h>

namespace tinfra {
namespace io {
namespace posix {

typedef int handle_type;
static const handle_type invalid_handle = -1;

class posix_stream: public stream {
    handle_type handle_;

public:
    posix_stream(handle_type handle): handle_(handle) {}
    virtual ~posix_stream();
    void close();
    
    int seek(int pos, stream::seek_origin origin);
    int read(char* data, int size);
    int write(const char* data, int size);
    void sync();
        
    intptr_t native() const 
    {
        return handle_;
    }
    void release() 
    {
        handle_ = invalid_handle;
    }
    handle_type get_native() const { return handle_; }
private:
    int close_nothrow();
};    
static void throw_io_exception(const char* message);

posix_stream::~posix_stream()
{
    if( handle_ != invalid_handle ) {
        if( close_nothrow() == -1 ) {
            // TODO: add silent failures reporting
            // int err = get_last_error();
            // tinfra::silent_failure(fmt("file close failed: %i" % blabla )
        }
    }
}

void posix_stream::close()
{
    if( close_nothrow() == -1 ) 
        throw_errno_error(errno, "close failed");
}

stream* open_native(int fd)
{
    return new posix_stream(fd);
}

stream* open_file(const char* name, openmode mode)
{
    int flags = 0;
    {
	const bool fread  = (mode & std::ios::in) == std::ios::in;
	const bool fwrite = (mode & std::ios::out) == std::ios::out;
	if( fread && fwrite )
	    flags |=  O_RDWR | O_CREAT;
	else if( fread )
	    flags |= O_RDONLY;
	else if ( fwrite )
	    flags |= O_WRONLY | O_CREAT;
	else
	    throw std::invalid_argument("bad openmode");
	if( (mode & std::ios::trunc) == std::ios::trunc) flags |= O_TRUNC;
	if( (mode & std::ios::app) == std::ios::app) flags |= O_APPEND;
    }
    int fd = ::open(name, flags, 00644);
    if( fd == -1 ) 
        throw_errno_error(errno, fmt("unable to open '%s'") % name);
    return new posix_stream(fd);
}

int posix_stream::close_nothrow()
{
    int rc = ::close(handle_);
    handle_ = invalid_handle;
    return rc;
}

int posix_stream::seek(int pos, stream::seek_origin origin)
{
    int whence;
    switch( origin ) {
    case stream::start:
        whence = SEEK_SET;
        break;
    case stream::current:
        whence = SEEK_CUR;
        break;
    case stream::end:
        whence = SEEK_END;
        break;
    }
    off_t e = lseek(handle_, pos, whence);
    if( e == (off_t)-1 )
	throw_errno_error(errno, "seek failed");
    return (int)e;
}

int posix_stream::read(char* data, int size)
{
    int r = ::read(handle_, data, size);
    if( r < 0 ) 
        throw_errno_error(errno, "read failed");
    return r;
}

int posix_stream::write(char const* data, int size)
{
    int w = ::write(handle_, data, size);
    if( w < 0 ) 
        throw_errno_error(errno, "write failed");
    return w;
}

void posix_stream::sync()
{
}

} // end namespace tinfra::io::posix

stream* open_file(const char* name, openmode mode)
{
    return posix::open_file(name, mode);
}

stream* open_native(intptr_t handle)
{
    return posix::open_native(static_cast<int>(handle));
}

} }
