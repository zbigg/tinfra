#include "tinfra/io/stream.h"
#include "tinfra/io/file.h"
#include "tinfra/fmt.h"

#ifdef _WIN32
#include <windows.h>
#else
#include <sys/types.h>
#include <sys/stat.h>
#include <fcntl.h>
#include <errno.h>
#include <unistd.h>
#endif

namespace tinfra {
namespace io {
namespace file {

#ifdef _WIN32
typedef HANDLE handle_type;
static const handle_type invalid_handle = 0;
#else 
typedef int handle_type;
static const handle_type invalid_handle = -1;
#endif
    
class native_stream: public stream {
    handle_type        handle_;

public:
    native_stream(handle_type handle): handle_(handle) {}
    virtual ~native_stream();
    void close();
    int seek(int pos, stream::seek_origin origin);
    int read(char* data, int size);
    int write(const char* data, int size);
    void sync();
    handle_type get_native() const { return handle_; }
};    

native_stream::~native_stream()
{
    if( handle_ != invalid_handle )
        close();
}

#ifdef _WIN32

static void throw_io_exception(const char* message)
{
    throw new io_exception(fmt("%s: %i") % message % ::GetLastError());
}

stream* open_native(void* handle)
{
    return new native_stream((HANDLE)handle);
}

stream* open_file(const char* name, std::ios::openmode mode)
{
    const bool fread =  (mode & std::ios::in) == std::ios::in;
    const bool fwrite = (mode & std::ios::out) == std::ios::out;
    DWORD dwDesiredAccess = 0;
    if( fread )   dwDesiredAccess |= GENERIC_READ;
    if( fwrite  ) dwDesiredAccess |= GENERIC_WRITE;
        
    DWORD dwCreationDistribution;
    /* convert mode to win32 native, taken from zcompat */
    {
        const bool fcreat = fwrite; // XXX: is it always true in ios ?
                                    // for now we create if we want write
        const bool fexcl = false;   // XXX: should we support it, ios supports it ?
        const bool ftrunc = (mode & std::ios::trunc) == std::ios::trunc;
        
        if( fcreat ) {
            if( fexcl )
                if( ftrunc )    /* fcreat | fexcl | ftrunc */
                    dwCreationDistribution = CREATE_NEW;
                else           /* fcreat | fexcl ----  BAD */
                    dwCreationDistribution = 0;
            else
                if( ftrunc )    /* fcreat | ftrunc */
                    dwCreationDistribution = CREATE_ALWAYS;
                else            /* fcreat */
                    dwCreationDistribution = OPEN_ALWAYS;
        } else if( ftrunc ) {
            if( fexcl )             /* ftrunc | fexcl  ---- BAD */
                dwCreationDistribution = 0;
            else                    /* ftrunc */
                dwCreationDistribution = TRUNCATE_EXISTING;
        } else if( fexcl ) {            /* fexcl ----  BAD*/
            dwCreationDistribution = 0;
        } else
            dwCreationDistribution = OPEN_EXISTING;
    }
    HANDLE handle = CreateFile(name,
                dwDesiredAccess,
                0,
                NULL,
                dwCreationDistribution,
                FILE_ATTRIBUTE_NORMAL,
                NULL);
    if( handle == INVALID_HANDLE_VALUE || handle == NULL ) {
        throw io_exception(fmt("unable to open %s") % name);
    }
    return new native_stream(handle);
}

void native_stream::close()
{
    if( ::CloseHandle(handle_) == 0 ) 
        throw_io_exception("close failed");
    handle_ = invalid_handle;
}

int native_stream::seek(int pos, stream::seek_origin origin)
{
    DWORD native_origin = 0;
    
    switch( origin ) {
    case stream::start:
        native_origin = FILE_BEGIN;
        break;
    case stream::current:
        native_origin = FILE_CURRENT;
        break;
    case stream::end:
        native_origin = FILE_END;
        break;
    }
    DWORD r = SetFilePointer(handle_, (LONG) pos, NULL, native_origin);
    if( r != 0xffffffff ) {
        return (int)r;
    } else {
        throw_io_exception("seek failed");
    }
}

int native_stream::read(char* data, int size)
{
    DWORD readed;
    if( ReadFile(handle_,
                (LPVOID)data,
                (DWORD) size,
                &readed,
                NULL) == 0)
    {
        throw_io_exception("read failed"); 
    }
    return readed;
}

int native_stream::write(char const* data, int size)
{
    DWORD written;
    if( WriteFile(handle_,
                  (LPCVOID)data,
                  (DWORD)  size,
                  &written,
                  NULL ) == 0 ) 
    {
        throw_io_exception("write failed"); 
    }
    return written;
}

void native_stream::sync()
{
}

#else

stream* open_native(void* handle)
{
    return new native_stream((int)handle);
}

static void throw_io_exception(const char* message)
{
    throw io_exception(fmt("%s: %s") % message % strerror(errno));
}

stream* open_file(const char* name, std::ios::openmode mode)
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
	    throw_io_exception("bad openmode");
	if( (mode & std::ios::trunc) == std::ios::trunc) flags |= O_TRUNC;
	if( (mode & std::ios::app) == std::ios::app) flags |= O_APPEND;
    }
    int fd = ::open(name, flags, 00644);
    if( fd == -1 ) throw_io_exception(fmt("unable to open %s") % name);
    return new native_stream(fd);
}

void native_stream::close()
{
    if( ::close(handle_) < 0 ) 
        throw_io_exception("close failed");
    handle_ = invalid_handle;
}

int native_stream::seek(int pos, stream::seek_origin origin)
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
	throw_io_exception("seek failed");
    return (int)e;
}

int native_stream::read(char* data, int size)
{
    int r = ::read(handle_, data, size);
    if( r < 0 ) throw_io_exception("read failed");
    return r;
}

int native_stream::write(char const* data, int size)
{
    int w = ::write(handle_, data, size);
    if( w < 0 ) throw_io_exception("write failed");
    return w;
}

void native_stream::sync()
{
}

#endif
} } }
