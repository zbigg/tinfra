//
// Copyright (c) 2009, Zbigniew Zagorski
// This software licensed under terms described in LICENSE.txt
//

#include "tinfra/runtime.h"
#include <cstdlib>

#include <unittest++/UnitTest++.h>

SUITE(tinfra) {
    TEST(get_environment_vs_getenv)
    {
        using std::getenv;
        using tinfra::environment_t;
        environment_t const env = tinfra::get_environment();
        
        
        for( environment_t::const_iterator ie = env.begin(); ie != env.end(); ++ie ) {
            const char* getenv_value = getenv(ie->first.c_str());
            
            CHECK_EQUAL(getenv_value, ie->second.c_str());
        }
    }
} // end SUITE(fmt)
