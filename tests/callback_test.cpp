#include <unittest++/UnitTest++.h>


#include "tinfra/callback.h"
#include "tinfra/value_guard.h"

#include <iostream>
#include <sstream>



SUITE(tinfra) {
    static bool foo_called = false;
    static void foo(int a)
    {
        CHECK_EQUAL(666, a);
        foo_called = true;
    }
    
    TEST(callback_make_from_function_by_val)
    {
        tinfra::value_guard<bool> foo_call_guard(foo_called);
        
        using tinfra::make_callback;
        using tinfra::callback;
        
        callback<int> cfoo = make_callback(foo);
        cfoo(666);
        CHECK( foo_called );
    }
    
    static bool bar_called = false;
    static void bar(std::string const& x)
    {
        CHECK_EQUAL("spam", x);
        bar_called = true;
    }
    
    TEST(callback_make_from_function_by_ref)
    {
        tinfra::value_guard<bool> bar_call_guard(bar_called);
        
        using tinfra::make_callback;
        using tinfra::callback;
                
        callback<std::string> cbar = make_callback(bar);
        cbar("spam");
        CHECK( bar_called );
    }
    
}
