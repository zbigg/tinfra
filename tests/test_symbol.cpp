//
// Copyright (c) 2009, Zbigniew Zagorski
// This software licensed under terms described in LICENSE.txt
//

#include "tinfra/symbol.h"
#include "tinfra/test.h" // test infra

using tinfra::symbol;

SUITE(tinfra)
{
    TEST(symbol_basic)
    {
        CHECK_EQUAL( symbol("a"), symbol("a") );
        CHECK(symbol("c") != symbol("a") );
        
        CHECK_EQUAL("a", symbol("a").c_str());       
    }
    
    TEST(symbol_null)
    {
        CHECK_EQUAL(0, symbol::null);
        CHECK_EQUAL(0, symbol());
        CHECK_EQUAL(symbol::null, symbol());
        CHECK_EQUAL(symbol::null, symbol::get("null"));
        
        CHECK_EQUAL(false, (bool)symbol());
        CHECK_EQUAL(false, (bool)symbol::null);
    }
    
    TEST(symbol_registry)
    {
        CHECK( symbol::find("--4382789") == symbol::null);
        
        CHECK( symbol::get("aaaa")   != symbol::null);
        CHECK( symbol::find("aaaa")  != symbol::null);
    }
}
