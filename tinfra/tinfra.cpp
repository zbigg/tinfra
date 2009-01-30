//
// Copyright (C) Zbigniew Zagorski <z.zagorski@gmail.com>,
// licensed to the public under the terms of the GNU GPL (>= 2)
// see the file COPYING for details
// I.e., do what you like, but keep copyright and there's NO WARRANTY.
//

#include <typeinfo>
#include "tinfra.h"

#ifdef __GNUC__
#define HAVE__CXA_DEMANGLE
#endif

#ifdef HAVE__CXA_DEMANGLE
#include <cxxabi.h>
#endif

#include <cstdlib>


namespace tinfra {

std::string demangle_typeinfo_name(const std::type_info& t)
{
#ifdef HAVE__CXA_DEMANGLE
    int status = 0;
    char* data = abi::__cxa_demangle(t.name(), NULL, NULL, &status);
    switch( status ) {
    case 0:
        {
            std::string result = data;
            std::free(data);
            return result;
        }
    case -1:
    case -2:
    case -3:
    default:
        return t.name();
    }
    
#else
    return t.name();
#endif
}

} // end of namespace tinfra

