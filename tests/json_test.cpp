
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

    std::string  written = tinfra::json_write(r);
    tinfra::variant reparsed = tinfra::json_parse(written);

    CHECK( reparsed == r );
}

TEST(json_integer_basic)
{
    using tinfra::variant;
    using tinfra::json_parse;
    CHECK_EQUAL(variant(0), json_parse("0"));
    CHECK_EQUAL(variant(-1), json_parse("-1"));
    CHECK_EQUAL(variant(256), json_parse("256"));
    CHECK_EQUAL(variant(-2147483648), json_parse("-2147483648"));
    CHECK_EQUAL(variant(2147483647), json_parse("2147483647"));
    
    CHECK_EQUAL(variant(-2147483648), json_parse("-2147483648"));
    CHECK_EQUAL(variant(2147483647), json_parse("2147483647"));
    CHECK_EQUAL(variant(-9223372036854775807), json_parse("-9223372036854775807"));
    CHECK_EQUAL(variant(9223372036854775807), json_parse("9223372036854775807"));
}

TEST(json_string_basic)
{
    using tinfra::variant;
    using tinfra::json_parse;
    CHECK_EQUAL(variant(""), json_parse("\"\""));
    CHECK_EQUAL(variant("foo"), json_parse("\"foo\""));
    CHECK_EQUAL(variant("foo\\\"bar\""), json_parse("\"foo\\\\\\\"bar\\\"\""));
    CHECK_EQUAL(variant("foo\r\nbar\t\b"), json_parse("\"foo\\r\\nbar\\t\\b\""));
}

TEST(json_string_errors)
{
    using tinfra::json_parse;
    
    CHECK_THROW( json_parse("\"fooo"), std::runtime_error);   // not finished string
    CHECK_THROW( json_parse("\"fooo\\"), std::runtime_error); // stray at end of not finished string
    CHECK_THROW( json_parse("\"fooo\\\""), std::runtime_error); // not finished again
    CHECK_THROW( json_parse("\"foo\\X"), std::runtime_error); // unkown escape character
}
TEST(json_parse_errors)
{
    using tinfra::json_parse;
    
    CHECK_THROW( json_parse("{"), std::runtime_error); // not finished
    CHECK_THROW( json_parse("{ XXX: 12 }"), std::runtime_error); // expecting string
    CHECK_THROW( json_parse("{ \"YYY\" 12 }"), std::runtime_error); // expecting colon
}
} // end suite tinfra
