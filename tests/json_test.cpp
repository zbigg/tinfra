
#include "tinfra/json.h" // we test this
#include "tinfra/test.h"

SUITE(tinfra) {

using tinfra::variant;

const char* json_sample1 = 
    "[ { \"name\": \"Moby Dick\","
    "    \"author\": \"Verne\" },"
    "  { \"name\": \"Solaris\","
    "    \"author\": \"Lem\"} ]";
TEST(json_sample1)
{
    tinfra::variant r = tinfra::json_parse(json_sample1);
    
    CHECK(r.is_array());
    CHECK(r[0].is_dict());
    CHECK(r[1].is_dict());
    
    //CHECK(r[0].is_dict());
    //CHECK(r[0].is_dict());
}

} // end suite tinfra
