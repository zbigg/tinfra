#include "tinfra/io/stream.h"
#include <string>

#include <unittest++/UnitTest++.h>


using namespace std;

#define CHECK_VALUE(m, k, T, v)       \
        m.put<T>(k,v);                \
        CHECK(m.contains<T>(k));      \
        CHECK_EQUAL(v, m.get<T>(k) )  

TEST(multitype_map)
{
    CHECK_VALUE("4", int, int(t)
} 