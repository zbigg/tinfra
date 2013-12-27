//
// Copyright (c) 2012, Zbigniew Zagorski
// This software licensed under terms described in LICENSE.txt
//

#include "tinfra/trace.h" // we test thi

#include "tinfra/test.h" // test infra
    
SUITE(tinfra) {
    
    TEST(trace_inheritance_doesnt_fuck_up_local_tracer)
    {
        tinfra::tracer fake_global("global", false);
        tinfra::tracer local(fake_global, "local", true);
        
        CHECK(local.is_enabled());
        CHECK(local.is_enabled_inherit());
        CHECK(local.is_enabled());
    }
}
