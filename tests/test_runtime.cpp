//
// Copyright (C) Zbigniew Zagorski <z.zagorski@gmail.com>,
// licensed to the public under the terms of the GNU GPL (>= 2)
// see the file COPYING for details
// I.e., do what you like, but keep copyright and there's NO WARRANTY.
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
