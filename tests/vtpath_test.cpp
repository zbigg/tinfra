
#include "tinfra/vtpath.h" // we test this
#include "tinfra/test.h"

SUITE(tinfra) {

using tinfra::variant;

TEST(vtpath_basics)
{
    using tinfra::variant;
    
    variant v = variant::array();
    v[0] = variant::dict();
    v[0]["name"].set_string("Moby Dick");
    v[0]["author"].set_string("Verne");
    v[1] = variant::dict();
    v[1]["name"].set_string("Solaris");
    v[1]["author"].set_string("Lem");
    
    tinfra::vtpath_visitor visitor(&v, "$.*.author");
    
    variant* r = 0;
    
    CHECK_EQUAL(true, visitor.fetch_next(r));
    CHECK_EQUAL(&v[0]["author"], r);
    CHECK_EQUAL("Verne", r->get_string());
    
    
    CHECK_EQUAL(true, visitor.fetch_next(r));
    CHECK_EQUAL(&v[1]["author"], r);
    CHECK_EQUAL("Lem", r->get_string());
    
    CHECK_EQUAL(false, visitor.fetch_next(r));
}

} // end suite tinfra
