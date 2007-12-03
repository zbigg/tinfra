#include "multitype_map.h"
#include <string>

#include <unittest++/UnitTest++.h>


using namespace std;

#define CHECK_VALUE(m, k, T, v)       \
        m.put<T>(k,v);                \
        CHECK(m.contains<T>(k));      \
        CHECK_EQUAL(v, m.get<T>(k) )  

TEST(multitype_map)
{
    tinfra::multitype_map<string> m;
    
    CHECK_VALUE(m, "a", int, 5);
    CHECK_VALUE(m, "b", double, 2.4);
    CHECK_VALUE(m, "c", string, "Z");
    
    m.clear();
    CHECK( !m.contains<int>("a") );
    CHECK( !m.contains<double>("a") );
    CHECK( !m.contains<string>("Z") );
}
