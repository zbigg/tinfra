
#include "tinfra/vtpath.h" // we test this
#include "tinfra/test.h"

SUITE(tinfra) {

using tinfra::variant;

// NODE, jsonpath evaulator
//     http://sdlab.naist.jp/members/kazuki-h/JSONPathOnlineEvaluator/
//
static variant make_sample_books()
    /* return following struct as variant-tree
        [
            {
                "name": "Moby Dick",
                "author": "Verne",
            },
            {
                "name": "Solaris",
                "author": "Lem",
            }
        ]
    */
{
    variant v = variant::array();
    v[0] = variant::dict();
    v[0]["name"].set_string("Moby Dick");
    v[0]["author"].set_string("Verne");
    v[1] = variant::dict();
    v[1]["name"].set_string("Solaris");
    v[1]["author"].set_string("Lem");
    return v;
}
TEST(vtpath_child_basics)
{
    using tinfra::variant;
    variant v = make_sample_books();
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

TEST(vtpath_recursive_all)
{
    using tinfra::variant;
    variant v = make_sample_books();
    
    // this shall return all but root
    std::vector<variant*> r = vtpath_visit(v, "$..*");
    
    CHECK_EQUAL(6, r.size());
    
    CHECK_EQUAL(&v[0], r[0]); // first dict first
    CHECK(r[1]->is_string()); // two strings
    CHECK(r[2]->is_string()); // (order unknown, so don't assert on value)
    
    CHECK_EQUAL(&v[1], r[3]); // second dict first
    CHECK(r[4]->is_string()); // two strings
    CHECK(r[5]->is_string()); // 
}

TEST(vtpath_recursive_basics)
{
    using tinfra::variant;
    variant v = make_sample_books();
    tinfra::vtpath_visitor visitor(&v, "$..author");
    
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
