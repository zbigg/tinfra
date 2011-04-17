#include "tinfra/win32/w32_stream.h" // we implement these
#include "tinfra/win32.h"
#include "tinfra/trace.h"

#ifndef NOMINMAX
#define WIN32_LEAN_AND_MEAN
#endif 

#ifndef NOMINMAX
#define NOMINMAX
#endif

#define _WIN32_WINNT 0x0500 // Windows 2000
#include <windows.h>

namespace tinfra {
    
win32::standard_handle_input  in;
win32::standard_handle_output out(false);
win32::standard_handle_output err(true);

namespace win32 {

static HANDLE native_handle(handle_type h)
{
    return reinterpret_cast<HANDLE>(h);
}
static int close_nothrow(handle_type handle)
{
    int rc = ::CloseHandle(native_handle(handle));
    return (rc == 0) ? -1 : 0;
}

//
// tinfra::win32::file_handle
//
file_handle::file_handle(handle_type fd_, bool own_):
    handle2(fd_),
    own(own_)
{
}

file_handle::~file_handle()
{
    if( native_handle(this->handle2) != INVALID_HANDLE_VALUE && this->own) { 
        int r = close_nothrow(this->handle2);
        if( r == -1 ) {
            TINFRA_LOG_ERROR("close failed in destructor");
        }
    }
}

void file_handle::reset(handle_type fd_, bool own_)
{
    if( native_handle(this->handle2) != INVALID_HANDLE_VALUE )
        close();
    this->handle2 = fd_;
    this->own = own_;
}

void file_handle::close()
{
    if( this->own && native_handle(this->handle2) != INVALID_HANDLE_VALUE ) {
        if( close_nothrow(this->handle2) == -1 ) {
            throw_system_error("close failed");
        }
    }
    this->handle2 = reinterpret_cast<intptr_t>(INVALID_HANDLE_VALUE);
}

//
// native_input_stream
//
native_input_stream::native_input_stream(handle_type existing, bool own):
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
    DWORD readed;
    if( ReadFile(native_handle(fd.handle()),
                (LPVOID)dest,
                (DWORD) size,
                &readed,
                NULL) == 0)
    {
        DWORD error = ::GetLastError();
        if( error == ERROR_BROKEN_PIPE )
            return 0;
        throw_system_error("read failed"); 
    }
    return readed;
}

//
// native_output_stream
//

native_output_stream::native_output_stream(handle_type existing, bool own):
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
    DWORD written;
    if( WriteFile(native_handle(fd.handle()),
                  (LPCVOID)data,
                  (DWORD)  size,
                  &written,
                  NULL ) == 0 ) 
    {        
        throw_system_error("write failed"); 
    }
    return written;
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
    const HANDLE handle = ::GetStdHandle(STD_INPUT_HANDLE);
    
    DWORD readed;
    if( ReadFile(handle,
                (LPVOID)data,
                (DWORD) size,
                &readed,
                NULL) == 0)
    {
        DWORD error = ::GetLastError();
        if( error == ERROR_BROKEN_PIPE )
            return 0;
        throw_system_error("standard input read failed"); 
    }
    //printf("win32_stream: readed %i bytes\n", readed);
    return readed;
}

//
// standard_handle_output
//

standard_handle_output::standard_handle_output(bool is_err):
    is_err_(is_err)
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
    HANDLE handle = ::GetStdHandle(is_err_ ? STD_ERROR_HANDLE
                                           : STD_OUTPUT_HANDLE);
    DWORD written;
    if( WriteFile(handle,
                  (LPCVOID)data,
                  (DWORD)  size,
                  &written,
                  NULL ) == 0 ) 
    {
        throw_system_error("write to stdout/stderr failed"); 
    }
    //printf("win32_stream: written %i bytes\n", written);
    return written;
}

void standard_handle_output::sync()
{
}

} // end namespace tinfra::win32
} // end namespace tinfra

