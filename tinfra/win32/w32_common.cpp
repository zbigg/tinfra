//
// Copyright (C) Zbigniew Zagorski <z.zagorski@gmail.com>,
// licensed to the public under the terms of the GNU GPL (>= 2)
// see the file COPYING for details
// I.e., do what you like, but keep copyright and there's NO WARRANTY.
//

#include "tinfra/platform.h"

#include "tinfra/win32.h"

#include "tinfra/fmt.h"
#include "tinfra/string.h"
#include "tinfra/io/stream.h"

#include <stdexcept>

#ifndef NOMINMAX
#define WIN32_LEAN_AND_MEAN
#endif 

#ifndef NOMINMAX
#define NOMINMAX
#endif

#define _WIN32_WINNT 0x0500 // Windows 2000
#include <windows.h>

namespace tinfra { namespace win32 {
    
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

template <typename E>
static void throw_system_error2(unsigned int error, const std::string& message)
{
    std::string error_str = get_error_string(error);
    std::string full_message = tinfra::fmt("%s: %s") % message % error_str;
    throw E(full_message);
}

void throw_system_error(unsigned int error, std::string const& message)
{
    switch( error ) {
    case ERROR_FILE_NOT_FOUND:
    case ERROR_PATH_NOT_FOUND:
    
    case WSAHOST_NOT_FOUND:
    case WSANO_DATA:
    case WSAECONNREFUSED:
        throw_system_error2<std::logic_error>(error, message);
    
    case WSAEFAULT:
    case ERROR_INVALID_HANDLE:
        throw_system_error2<std::invalid_argument>(error, message);
    
    case WSAEWOULDBLOCK:
        throw_system_error2<tinfra::io::would_block>(error, message);
        
    default:
        throw_system_error2<std::runtime_error>(error, message);
    }
}
void throw_system_error(std::string const& message)
{
    unsigned int error = ::GetLastError();
    throw_system_error(error, message);
}
} } // end namespace tinfra::win32
