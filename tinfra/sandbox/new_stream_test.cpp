//
// Copyright (c) 2009, Zbigniew Zagorski
// This software licensed under terms described in LICENSE.txt
//

#include "tinfra/stream.h"
#include "tinfra/test.h" // for test infra

SUITE(tinfra) {

    TEST(basic_parse_test)
    {
        shared_ptr<X> p(new X);
        p->next = shared_ptr<X>(new X);
        p = p->next;
        //CHECK(false);
        CHECK( p->next.get() == 0 );
    }
}

// jedit: :tabSize=8:indentSize=4:noTabs=true:mode=c++:

