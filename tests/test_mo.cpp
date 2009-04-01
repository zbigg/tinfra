//
// Copyright (C) Zbigniew Zagorski <z.zagorski@gmail.com>,
// licensed to the public under the terms of the GNU GPL (>= 2)
// see the file COPYING for details
// I.e., do what you like, but keep copyright and there's NO WARRANTY.
//

#include "tinfra/symbol.h"

#include <unittest++/UnitTest++.h>

#include "tinfra/mo.h"


SUITE(tinfra)
{
    TINFRA_SYMBOL_IMPL(x);
    TINFRA_SYMBOL_IMPL(y);
    
    struct point {
        int x;
        int y;
        TINFRA_MO_MANIFEST(point) {
            TINFRA_MO_FIELD(x);
            TINFRA_MO_FIELD(y);
        }
    };
    using tinfra::symbol;
    
    struct dummy_functor {
        void operator()(symbol const&, int const&) {}
    };
    
    TEST(mo_process_api)
    {
        dummy_functor f;
        const point a = {0,0};
        tinfra::mo_process(a, f);
    }
    
    struct foo_modifier {
        void operator()(symbol const&, int& v ) { v = 1; }
    };
    
    TEST(mo_mutate_api)
    {
        foo_modifier f;
        point a = { 0, 0 };
        tinfra::mo_mutate(a, f);
        
        CHECK_EQUAL(1, a.x);
        CHECK_EQUAL(1, a.y);
    }
}