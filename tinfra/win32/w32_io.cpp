#include "tinfra/io/stream.h"
#include "tinfra/fmt.h"
#include "tinfra/win32.h"

#include <stdio.h>
#include <windows.h>

namespace tinfra {
namespace win32 {

using tinfra::io::stream;
using tinfra::io::io_exception;
    
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

    intptr_t native() const 
    {
        return reinterpret_cast<intptr_t>(handle_);
    }
    void release() 
    {
        handle_ = invalid_handle;
    }

    HANDLE get_native() const { return handle_; }
private:
    int close_nothrow();
};

static void throw_get_last_error(const char* message);

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
        throw_get_last_error("close failed");
    
}

static void throw_get_last_error(const char* message)
{
    throw_system_error(message);
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
        throw_get_last_error(fmt("unable to open %s") % name);
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
        throw_get_last_error("seek failed");
        // doesn't return
        return -1;
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
        DWORD error = ::GetLastError();
        if( error == ERROR_BROKEN_PIPE )
            return 0;
        throw_get_last_error("read failed"); 
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
        throw_get_last_error("write failed"); 
    }
    return written;
}

void win32_stream::sync()
{
}

} } // end namespace tinfra::win32

//
// link win32 io as default IO
//

namespace tinfra { namespace io {
   
stream* open_file(const char* name, std::ios::openmode mode)
{
    return tinfra::win32::open_file(name, mode);
}

stream* open_native(intptr_t handle)
{
    return tinfra::win32::open_native(reinterpret_cast<HANDLE>(handle));
}
    
} } // end namespace tinfra::io
