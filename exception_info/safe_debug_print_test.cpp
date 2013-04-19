#include "safe_debug_print.h"

#include "tinfra/memory_stream.h"
#include "tinfra/test.h"

SUITE(tinfra) {

template <typename T>
static std::string unsafe_wrap_debug_print(T const& v)
{
    std::string result;
    tinfra::memory_output_stream out(result);
    tinfra::safe_debug_print(out, v);

    return result;
}

TEST(safe_debug_print_numerics)
{
    CHECK_EQUAL("0", unsafe_wrap_debug_print(0));
    CHECK_EQUAL("1234506789", unsafe_wrap_debug_print(1234506789));
}

TEST(safe_debug_print_strings)
{
    CHECK_EQUAL("literal", unsafe_wrap_debug_print("literal"));

    char foo[] = "foo";
    CHECK_EQUAL("foo", unsafe_wrap_debug_print("foo"));

    CHECK_EQUAL("std::string", unsafe_wrap_debug_print(std::string("std::string")));
    
    CHECK_EQUAL("\\r\\n", unsafe_wrap_debug_print("\r\n"));
    CHECK_EQUAL(" \\x1", unsafe_wrap_debug_print(" \x01"));
}

} // end SUITE(tinfra)

