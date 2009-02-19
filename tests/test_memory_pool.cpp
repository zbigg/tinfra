//
// Copyright (C) Zbigniew Zagorski <z.zagorski@gmail.com>,
// licensed to the public under the terms of the GNU GPL (>= 2)
// see the file COPYING for details
// I.e., do what you like, but keep copyright and there's NO WARRANTY.
//

#include <unittest++/UnitTest++.h>
#include "tinfra/memory_pool.h"
#include "tinfra/fmt.h"


SUITE(tinfra)
{
    TEST(memory_pool_small)
    {
        tinfra::raw_memory_pool pool(1);
        CHECK( pool.alloc(1) != 0 );
        
        // TODO: decide if memory pool should throw, return null or assert in this
        //       case (too big allocation)
        //CHECK( pool.alloc(2) == 0 );
        CHECK_THROW( pool.alloc(2) == 0, std::logic_error );
        for( int i = 0; i < 100; ++i ) {
            CHECK(pool.alloc(1) != 0);
        }
    }
}
