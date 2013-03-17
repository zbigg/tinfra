
#include "tinfra/vtpath.h" // we test this
#include "tinfra/json.h"   // for json_parse
#include "tinfra/test.h"

SUITE(tinfra) {

using tinfra::variant;
using std::vector;

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

tinfra::variant make_jsonpath_reference_example()
{
    return tinfra::json_parse(
            "{ \"store\": {\n"
            "    \"book\": [ \n"
            "      { \"category\": \"reference\",\n"
            "        \"author\": \"Nigel Rees\",\n"
            "        \"title\": \"Sayings of the Century\",\n"
            "        \"price\": 8.95\n"
            "      },\n"
            "      { \"category\": \"fiction\",\n"
            "        \"author\": \"Evelyn Waugh\",\n"
            "        \"title\": \"Sword of Honour\",\n"
            "        \"price\": 12.99\n"
            "      },\n"
            "      { \"category\": \"fiction\",\n"
            "        \"author\": \"Herman Melville\",\n"
            "        \"title\": \"Moby Dick\",\n"
            "        \"isbn\": \"0-553-21311-3\",\n"
            "        \"price\": 8.99\n"
            "      },\n"
            "      { \"category\": \"fiction\",\n"
            "        \"author\": \"J. R. R. Tolkien\",\n"
            "        \"title\": \"The Lord of the Rings\",\n"
            "        \"isbn\": \"0-395-19395-8\",\n"
            "        \"price\": 22.99\n"
            "      }\n"
            "    ],\n"
            "    \"bicycle\": {\n"
            "      \"color\": \"red\",\n"
            "      \"price\": 19.95\n"
            "    }\n"
            "  }\n"
            "}\n");
}
/*
    this one fails now
TEST(vtpath_reference_1)
    // $.store.book[*].author
    //     the authors of all books in the store
{
    vector<variant*> r = tinfra::vtpath_visit(make_jsonpath_reference_example(),
                                              "$.store.book[*].author");
    
    CHECK_EQUAL(4, r.size());
    CHECK_EQUAL("Nigel Rees", r[0]->get_string());
    CHECK_EQUAL("Evelyn Waugh", r[1]->get_string());
    CHECK_EQUAL("Herman Melville", r[2]->get_string());
    CHECK_EQUAL("J. R. R. Tolkien", r[3]->get_string());
}
*/
TEST(vtpath_reference_2)
    // $..author
    //     all authors
{
    variant sample = make_jsonpath_reference_example();
    vector<variant*> r = tinfra::vtpath_visit(sample,
                                              "$..author");
    
    CHECK_EQUAL(4, r.size());
    CHECK_EQUAL("Nigel Rees", r[0]->get_string());
    CHECK_EQUAL("Evelyn Waugh", r[1]->get_string());
    CHECK_EQUAL("Herman Melville", r[2]->get_string());
    CHECK_EQUAL("J. R. R. Tolkien", r[3]->get_string());
}

TEST(vtpath_reference_3)
    // $.store.*
    //     all things in store, which are some books and a red bicycle
{
    variant sample = make_jsonpath_reference_example();
    vector<variant*> r = tinfra::vtpath_visit(sample,
                                              "$.store.*");
    
    CHECK_EQUAL(2, r.size());
    
    // this shall return bicycle dict and books array
    // NODE, the current dict is lexicographically ordered
    // so result is valid as long as we use std::map and not some
    // other dict!
    CHECK(r[1]->is_array());
    CHECK_EQUAL(4, r[1]->size());
    
    CHECK(r[0]->is_dict());
    CHECK_EQUAL(2, r[0]->size());
}

TEST(vtpath_reference_4)
    // $.store..price
    //    the price of everything in the store.
{
    variant sample = make_jsonpath_reference_example();
    vector<variant*> r = tinfra::vtpath_visit(sample,
                                              "$.store..price");

    CHECK_EQUAL(5, r.size());
    
    // this shall return bicycle dict and books array
    // NODE, the current dict is lexicographically ordered
    // so result is valid as long as we use std::map and not some
    // other dict!
    CHECK(r[0]->is_double());
    CHECK_EQUAL(19.95, r[0]->get_double());
    
    CHECK(r[1]->is_double());
    CHECK_EQUAL(8.95, r[1]->get_double());
    
    // skip 2,3
    
    CHECK(r[4]->is_double());
    CHECK_EQUAL(22.99, r[4]->get_double());
}

/*
    fails for now
TEST(vtpath_reference_5)
    // $..book[2]
    //     the third book
{
    variant sample = make_jsonpath_reference_example();
    vector<variant*> r = tinfra::vtpath_visit(sample,
                                              "$..book[2]");
    CHECK_EQUAL(1, r.size());
    CHECK(r[0]->is_dict());
    CHECK_MAP_ENTRY_MATCH(variant("Moby Dick"),"author",r[0]->get_dict());
}
*/

/*
TEST(vtpath_reference_6)
    // $..book[(@.length-1)]
    //     the last book in order.
{
    // TBD, not implemented
}
*/

/*
TEST(vtpath_reference_7)
    // $..book[-1:]
    //     the last book in order.
{
    // TBD, not implemented
}
*/

/*
TEST(vtpath_reference_8)
    // $..book[0,1]
    //     the first two books
{
    // multiple selectors not implemented
}
*/

/*
TEST(vtpath_reference_9)
    // $..book[:2]
    //     the first two books
{
    // slicing not implemented
}
*/

/*
TEST(vtpath_reference_10)
    // $..book[?(@.isbn)]
    //     filter all books with isbn number
{
    // filter expressions not implemented
}
*/

/*
TEST(vtpath_reference_11)
    // $..book[?(@.price<10)]
    //     filter all books cheapier than 10
{
    // filter predicates not implemented
}
*/


TEST(vtpath_reference_12)
    // $..*
    //     All members of JSON structure.
{
    variant sample = make_jsonpath_reference_example();
    vector<variant*> r = tinfra::vtpath_visit(sample, "$..*");
    CHECK_EQUAL(27, r.size());
    CHECK_EQUAL(&(sample["store"]), r[0]);
    CHECK_EQUAL(&(sample["store"]["bicycle"]), r[1]);
    CHECK_EQUAL(&(sample["store"]["bicycle"]["color"]), r[2]);
    CHECK_EQUAL(&(sample["store"]["bicycle"]["price"]), r[3]);
    CHECK_EQUAL(&(sample["store"]["book"]), r[4]);
    CHECK_EQUAL(&(sample["store"]["book"][0]), r[5]);
    CHECK_EQUAL(&(sample["store"]["book"][0]["author"]), r[6]);
}


} // end suite tinfra
