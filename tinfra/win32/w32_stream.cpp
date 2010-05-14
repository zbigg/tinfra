#include "tinfra/win32/w32_stream.h" // we implement these
#include "tinfra/win32.h"

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

