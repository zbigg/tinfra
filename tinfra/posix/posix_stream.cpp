#include "tinfra/posix/posix_stream.h" // we implement these

#include "os_common.h"

#include <unistd.h>
#include <errno.h>

namespace tinfra {
    
posix::standard_handle_input  in;
posix::standard_handle_output out(1);
posix::standard_handle_output err(2);

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
        if( w < 0 ) 
            throw_errno_error(errno, "write failed");
        return w;
    }
}

void standard_handle_output::sync()
{
}

} // end namespace tinfra::win32
} // end namespace tinfra

