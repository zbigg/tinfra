#include "tinfra/io/stream.h"
#include "tinfra/io/win32.h"
#include "tinfra/fmt.h"

#include <windows.h>

namespace tinfra {
namespace io {
namespace win32 {

static const HANDLE invalid_handle = 0;

class win32_stream: public stream {
    HANDLE handle_;

public:
    win32_stream(HANDLE handle): handle_(handle) {}
    virtual ~win32_stream();
    void close();
    
    int seek(int pos, stream::seek_origin origin);
    int read(char* data, int size);
    int write(const char* data, int size);
    void sync();
    HANDLE get_native() const { return handle_; }
private:
    int close_nothrow();
};

static void throw_io_exception(const char* message);

win32_stream::~win32_stream()
{
    if( handle_ != invalid_handle ) {
        if( close_nothrow() == -1 ) {
            // TODO: add silent failures reporting
            // int err = get_last_error();
            // tinfra::silent_failure(fmt("file close failed: %i" % blabla )
        }
    }
}

void win32_stream::close()
{
    if( close_nothrow() == -1 ) 
        throw_io_exception("close failed");
}

static void throw_io_exception(const char* message)
{
    unsigned int error = ::GetLastError();
    throw new io_exception(fmt("%s: %s(%i)") % message % get_error_string(error) % error);
}

stream* open_native(void* handle)
{
    return new win32_stream((HANDLE)handle);
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
    return new win32_stream(handle);
}

int win32_stream::close_nothrow()
{
    int rc = ::CloseHandle(handle_);
    handle_ = invalid_handle;
    return (rc == 0) ? -1 : 0;
}

int win32_stream::seek(int pos, stream::seek_origin origin)
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

int win32_stream::read(char* data, int size)
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

int win32_stream::write(char const* data, int size)
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

void win32_stream::sync()
{
}

std::string get_error_string(unsigned int error_code)
{
    LPVOID lpMsgBuf;
    if( ::FormatMessage(
	FORMAT_MESSAGE_ALLOCATE_BUFFER | FORMAT_MESSAGE_FROM_SYSTEM,
	NULL,
	error_code,
	MAKELANGID(LANG_NEUTRAL, SUBLANG_DEFAULT), // Default language
	(LPTSTR) &lpMsgBuf,
	0,
	NULL
	) < 0 || lpMsgBuf == NULL) {

	return fmt("unknown error: %i") % error_code;
    }
    std::string result((char*)lpMsgBuf);
    ::LocalFree(lpMsgBuf);
    strip_inplace(result);
    return result;
}

} // end namespace tinfra::io::win32

stream* open_file(const char* name, std::ios::openmode mode)
{
    return win32::open_file(name, mode);
}

stream* open_native(void* handle)
{
    return win32::open_native(handle);
}

} }

