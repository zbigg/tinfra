//
// Copyright (c) 2009, Zbigniew Zagorski
// This software licensed under terms described in LICENSE.txt
//

//
// win32/runtime.cpp services
//

#include "tinfra/runtime.h"
#include "tinfra/win32.h"

#include <stdio.h>
#include <windows.h>

namespace tinfra {

environment_t get_environment()
{
    char* const es = GetEnvironmentStrings();
    const char* p = es;
    environment_t result;
    while( *p ) {
        const tstring current(p);
        const size_t eq_pos = current.find_first_of('=');
        if( eq_pos != tstring::npos ) {
            
            const tstring name = current.substr(0, eq_pos);
            const tstring value = current.substr(eq_pos+1);
            
            result[name.str()] = value.str();
        }
        p += current.size() + 1;
    }
    
    FreeEnvironmentStrings(es);
    return result;
}


} // end of namespace tinfra
