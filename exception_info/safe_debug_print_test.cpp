#include "safe_debug_print.h"

#include "tinfra/memory_stream.h"
#include "tinfra/test.h"
#include "tinfra/lex.h"

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

template <typename T>
static void test_numbers()
{
    CHECK_EQUAL("0", unsafe_wrap_debug_print(T(0)));
    CHECK_EQUAL("1", unsafe_wrap_debug_print(T(1)));
    CHECK_EQUAL("255", unsafe_wrap_debug_print(T(255)));
    CHECK_EQUAL("32767", unsafe_wrap_debug_print(T(32767)));

    T min = std::numeric_limits<T>::min();
    CHECK_EQUAL(tinfra::to_string(min), unsafe_wrap_debug_print(min));
    T max = std::numeric_limits<T>::max();
    CHECK_EQUAL(tinfra::to_string(max), unsafe_wrap_debug_print(max));
}

TEST(safe_debug_print_integers)
{
    test_numbers<short>();
    test_numbers<unsigned short>();
    test_numbers<int>();
    test_numbers<unsigned int>();
    test_numbers<long>();
    test_numbers<unsigned long>();
    test_numbers<long long>();
    test_numbers<unsigned long long>();
}

template <typename T>
static void test_char_numbers()
{
    CHECK_EQUAL("0", unsafe_wrap_debug_print(T(0)));

    T min = std::numeric_limits<T>::min();
    CHECK_EQUAL(tinfra::to_string((int)min), unsafe_wrap_debug_print(min));
    T max = std::numeric_limits<T>::max();
    CHECK_EQUAL(tinfra::to_string((int)max), unsafe_wrap_debug_print(max));
}

TEST(safe_debug_print_char_numbers)
{
    test_char_numbers<char>();
    test_char_numbers<signed char>();
    test_char_numbers<unsigned char>();

}

TEST(safe_debug_print_pointer)
{
    void* p = reinterpret_cast<void*>(0xdeadbeef);
    CHECK_EQUAL("0xdeadbeef", unsafe_wrap_debug_print(p));
    const void* p2 = 0;
    CHECK_EQUAL("0x0", unsafe_wrap_debug_print(p2));
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

