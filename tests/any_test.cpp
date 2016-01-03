//
// Copyright (c) 2009, Zbigniew Zagorski
// This software licensed under terms described in LICENSE.txt
//

#include "tinfra/any.h"  // we test this
#include "tinfra/test.h" // for test infra

SUITE(tinfra) {

    TEST(any_copied_api)
    {
        using tinfra::any;

        any f = any::from_copy<int>(2);

        CHECK( f.type() == typeid(int) );

        CHECK_EQUAL( 2, f.get<int>() );
    }

    TEST(any_ref_api)
    {
        using tinfra::any;
        int victim = 2;
        any f = any::by_ref<int>(victim);

        CHECK( f.type() == typeid(int) );

        CHECK_EQUAL( &victim, f.get_raw() );

        CHECK_EQUAL( 2, f.get<int>() );

        victim = 3;

        CHECK_EQUAL( 3, f.get<int>() );
    }

    struct counted {
        static int instance_count;

        counted() {
            instance_count++;
        }
        ~counted() {
            instance_count--;
        }
    };
    int counted::instance_count = 0;

    TEST(any_auto_ptr_api)
    {
        using tinfra::any;

        CHECK_EQUAL( 0, counted::instance_count);

        {
            counted* victim = new counted();
            CHECK_EQUAL( 1, counted::instance_count);

            any f = any::from_new<counted>(victim);

            CHECK( f.type() == typeid(counted) );

            CHECK_EQUAL( victim, f.get_raw());
            CHECK_EQUAL( victim, &f.get<counted>());

        }
        CHECK_EQUAL( 0, counted::instance_count);
    }
}

// jedit: :tabSize=8:indentSize=4:noTabs=true:mode=c++:



