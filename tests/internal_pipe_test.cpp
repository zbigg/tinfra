//
// Copyright (c) 2010, Zbigniew Zagorski
// This software licensed under terms described in LICENSE.txt
//

#include <unittest++/UnitTest++.h>
#include "tinfra/internal_pipe.h"
#include "tinfra/tstring.h"

#include <stdexcept>

SUITE(tinfra)
{
    using tinfra::internal_pipe;
    using tinfra::tstring;
    
    /// this test checks if general
    /// API works and "no-op" usage 
    /// has defined behaviour
    TEST(internal_pipe_simple_sequential_flow)
    {
        const tstring sample("XXX YYY ZZZ");
        internal_pipe pipe(internal_pipe::UNLIMITED);
        
        // write sample data to buffer
        CHECK_EQUAL( sample.size(), pipe.write(sample.data(), sample.size()));
        pipe.close();
        
        // write shall throw when pipe is closed
        CHECK_THROW( pipe.write("aa", 2), std::logic_error);
        
        char buf[100];
        // read should return all (and nothing more)data written to pipe at one shot
        {
            CHECK_EQUAL( sample.size(), pipe.read(buf, sizeof(buf)) );
            tstring readed(buf, sample.size());
            CHECK_EQUAL( sample, readed);
        }
        
        // each subsequent read should return 0
        CHECK_EQUAL( 0 , pipe.read(buf, sizeof(buf)) );
    }
}
