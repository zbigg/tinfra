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

} // end suite tinfra
