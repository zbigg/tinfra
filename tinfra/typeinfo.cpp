//
// Copyright (c) 2009, Zbigniew Zagorski
// This software licensed under terms described in LICENSE.txt
//

#include "config-priv.h"

#include <typeinfo>
#include "typeinfo.h"


#ifdef HAVE_CXXABI_H
#include <cxxabi.h>
#define HAVE__CXA_DEMANGLE
#endif

#include <cstdlib>

namespace tinfra {

std::string demangle_type_info_name(const std::type_info& ti)
{
#ifdef HAVE__CXA_DEMANGLE
    int status = 0;
    char* data = abi::__cxa_demangle(ti.name(), NULL, NULL, &status);
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
        return ti.name();
    }
    
#else
    return ti.name();
#endif
}

} // end of namespace tinfra
