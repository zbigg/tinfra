//
// Copyright (c) 2009, Zbigniew Zagorski
// This software licensed under terms described in LICENSE.txt
//

//
// win32/runtime.cpp services
//

#include "../platform.h"
#ifdef TINFRA_W32

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
        const tstring env_entry(p);
        const size_t eq_pos = env_entry.find_first_of('=');
        
        if( eq_pos != tstring::npos && eq_pos > 0) {
            
            const tstring name = env_entry.substr(0, eq_pos);
            const tstring value = env_entry.substr(eq_pos+1);
            
            result[name.str()] = value.str();
        }
        p += env_entry.size() + 1;
    }
    
    FreeEnvironmentStrings(es);
    return result;
}


} // end of namespace tinfra

#endif // TINFRA_W32

