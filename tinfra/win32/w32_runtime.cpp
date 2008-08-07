//
// win32/runtime.cpp services
//

#include "tinfra/runtime.h"

namespace tinfra {

bool is_stacktrace_supported()
{
    return false;
}
bool get_stacktrace(stacktrace_t& t) 
{
    return false;
}

bool get_debug_info(void* address, debug_info& result)
{
    return false;
}


} // end of namespace tinfra
