#include "tinfra/platform.h"
#include "tinfra/fmt.h"

#include "tinfra/win32.h"
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

} } // end namespace tinfra::win32
