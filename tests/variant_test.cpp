#include "tinfra/variant.h" // we test this

#include "tinfra/test.h"

SUITE(tinfra) {

using tinfra::variant;

TEST(variant_int_basics)
{
    variant v(312);
    CHECK(v.is_integer());
    CHECK_EQUAL(312, v.get_integer());
    
    variant b(v);
    v.set_integer(666);
    CHECK_EQUAL(312, b.get_integer()); // check that copy hasn't changed
    CHECK_EQUAL(666, v.get_integer());
}

TEST(variant_variant_compare)
{
    using tinfra::variant;
    
    // values
    CHECK_EQUAL(variant(), variant::none());
    CHECK_EQUAL(variant(99), variant(99));
    CHECK_EQUAL(variant(1.2), variant(1.2));
    CHECK_EQUAL(variant("aaaabbbbb"), variant("aaaabbbbb")) ;
    
    // dict
    CHECK_EQUAL(variant::dict(), variant::dict());
    {
        variant a = variant::dict();
        a["foo"] = variant("bar");
        a["xyz"] = variant(222);
        
        variant b = variant::dict();
        
        CHECK( a != b );
        b["xyz"] = variant(222);
        
        CHECK( a != b );
        b["foo"] = variant("bar");
        
        CHECK_EQUAL(a, b);
        a["dww"] = variant(666);
        CHECK( a != b );
        b["dww"] = variant(666);
        CHECK( a == b );
    }
    CHECK_EQUAL(variant::array(), variant::array());
    {
        variant a = variant::array();
        a[0] = variant("bar");
        a[1] = variant(222);
        
        variant b = variant::array();
        b[0] = variant("bar");
        b[1] = variant(222);
        CHECK_EQUAL(a, b);
        a[2] = variant(333);
        CHECK( a != b );
        
        b[2] = variant(666);
        CHECK( a != b );
        
        b[2] = variant(333);
        CHECK_EQUAL(a, b);
    }
}

} // end suite tinfra
