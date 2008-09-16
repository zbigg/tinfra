//
// Copyright (C) Zbigniew Zagorski <z.zagorski@gmail.com>,
// licensed to the public under the terms of the GNU GPL (>= 2)
// see the file COPYING for details
// I.e., do what you like, but keep copyright and there's NO WARRANTY.
//

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
