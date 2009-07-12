//
// Copyright (C) Zbigniew Zagorski <z.zagorski@gmail.com>,
// licensed to the public under the terms of the GNU GPL (>= 2)
// see the file COPYING for details
// I.e., do what you like, but keep copyright and there's NO WARRANTY.
//

#include "tinfra/symbol.h"

#include <unittest++/UnitTest++.h>

#include "tinfra/mo.h"

namespace tinfra_mo_test {
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
}

namespace tinfra {
    template<> 
    struct mo_traits<tinfra_mo_test::point>: public tinfra::struct_mo_traits<tinfra_mo_test::point> {};
}

SUITE(tinfra)
{
    using tinfra_mo_test::point;
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

// jedit: :tabSize=8:indentSize=4:noTabs=true:mode=c++:
