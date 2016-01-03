//
// Copyright (c) 2011, Zbigniew Zagorski
// This software licensed under terms described in LICENSE.txt
//

#include "tinfra/assert.h"  // we test this
#include "tinfra/test.h" // for test infra

#include "tinfra/typeinfo.h" // for demangle_type_info_name
//#include <typeifo>

SUITE(tinfra) {

    static void foo_function() {
        TINFRA_ASSERT( 0 == 1);
    }

    TEST(assert_simple_throw)
    {
        using tinfra::demangle_type_info_name;
        try {
            foo_function();
            CHECK_EQUAL(0,1); // fail
        } catch( std::logic_error const& e) {
            CHECK_EQUAL( demangle_type_info_name(typeid(std::logic_error)), demangle_type_info_name(typeid(e)));

            CHECK_STRING_CONTAINS( "0 == 1", e.what());
            CHECK_STRING_CONTAINS( "assertion", e.what());
            CHECK_STRING_CONTAINS( "foo_function", e.what());
            CHECK_STRING_CONTAINS( "assert_test.cpp", e.what());
        }
    }

}

// jedit: :tabSize=8:indentSize=4:noTabs=true:mode=c++:



