//
// Copyright (c) 2009, Zbigniew Zagorski
// This software licensed under terms described in LICENSE.txt
//

#include "tinfra/buffer.h"

#include <vector>
#include <string>

#include <unittest++/UnitTest++.h>

SUITE(tinfra) {
    
    TEST(buffer_basic_api)
    {
        using tinfra::buffer;
        
        const buffer<long> b(4,1);
        
        CHECK_EQUAL(4, b.size());
        CHECK( b.data() != 0 );
        CHECK_EQUAL( 4, std::distance(b.begin(), b.end()) );
        CHECK( b.end() == b.begin() + 4 );

	CHECK_EQUAL(1, b[0]);
	CHECK_EQUAL(1, b.at(3));
     
        buffer<long> c(b);
        
        CHECK(b == c);
        CHECK_EQUAL(4, c.size());
        
        c[0] = 3;
        c[3] = 0;
        
        CHECK_EQUAL(3, c.at(0));
        CHECK_EQUAL(0, c.at(3));

	CHECK_EQUAL(1, b.at(0));
        CHECK_EQUAL(1, b.at(3));
        
        CHECK(! (b == c) );
        
        c = b;
        CHECK(b == c);
    }
    
    TEST(buffer_copy_api)
    {
        using tinfra::buffer;
        {
            std::vector<short> v;
            v.push_back('a');
            v.push_back('b');
            
            buffer<short> b(v);
            
            CHECK_EQUAL(v.size(), b.size());
            CHECK_EQUAL('a', b[0]);
            CHECK_EQUAL('b', b[1]);
        }
        
        {
            std::string s("xy");
            
            buffer<char> b(s);
            
            CHECK_EQUAL(s.size(), b.size());
            CHECK_EQUAL('x', b[0]);
            CHECK_EQUAL('y', b[1]);
        }
    }
}

// jedit: :tabSize=8:indentSize=4:noTabs=true:mode=c++:


