
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
}

} // end suite tinfra
