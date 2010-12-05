//
// Copyright (c) 2009, Zbigniew Zagorski
// This software licensed under terms described in LICENSE.txt
//

#include "tinfra/memory_pool.h"
#include "tinfra/test.h"


SUITE(tinfra)
{
    TEST(memory_pool_small)
    {
        tinfra::raw_memory_pool pool(1);
        CHECK( pool.alloc(1) != 0 );
        
        // TODO: decide if memory pool should throw, return null or assert in this
        //       case (too big allocation)
        //CHECK( pool.alloc(2) == 0 );
        CHECK_THROW( pool.alloc(2), std::logic_error );
        for( int i = 0; i < 100; ++i ) {
            CHECK(pool.alloc(1) != 0);
        }
    }
}
