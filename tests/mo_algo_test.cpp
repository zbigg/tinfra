
#include "tinfra/mo_algo.h" // we test this
#include "tinfra/mo.h"   

#include "tinfra/test.h" // test framework

#include <string>

    
namespace mo_algo_test {
    
struct address {
    std::string street;
    std::string home;
    std::string apartment;
    std::string city;
    
    TINFRA_MO_MANIFEST(address)
    {
        TINFRA_MO_FIELD(street);
        TINFRA_MO_FIELD(home);
        TINFRA_MO_FIELD(apartment);
        TINFRA_MO_FIELD(city);
    }
};

struct person {
    std::string name;
    address     occupation_address;
    int         year_of_birth;
    
    TINFRA_MO_MANIFEST(person)
    {
        TINFRA_MO_FIELD(name);
        TINFRA_MO_FIELD(occupation_address);
        TINFRA_MO_FIELD(year_of_birth);
    }
};
} // end namespace mo_algo_test

TINFRA_MO_IS_RECORD(mo_algo_test::address);
TINFRA_MO_IS_RECORD(mo_algo_test::person);

//
// mo_algo_test.cpp 
//
SUITE(tinfra) {

using mo_algo_test::person;

person sample_a = { "bob", { "some_street", "12b", "", "CityC" }, 1979 };
person sample_same_as_a(sample_a);

person sample_b = { "bob", { "some_street", "20", "", "CityB" }, 1988 };

TEST(mo_algo_equals)
{
    CHECK( tinfra::mo_equals(sample_a, sample_same_as_a));
    CHECK( tinfra::mo_equals(sample_same_as_a, sample_a));

    CHECK( !tinfra::mo_equals(sample_a, sample_b));
    CHECK( !tinfra::mo_equals(sample_b, sample_a));    
}

TEST(mo_algo_less_than)
{
    CHECK(  tinfra::mo_less_than(sample_a, sample_b));
    CHECK( !tinfra::mo_less_than(sample_b, sample_a));    
}

TEST(mo_algo_swap)
{
    person xa(sample_a);
    person xb(sample_b);
    CHECK( tinfra::mo_equals(xa, sample_a));
    CHECK( tinfra::mo_equals(xb, sample_b));
    
    tinfra::mo_swap(xa, xb);
    CHECK( tinfra::mo_equals(xa, sample_b));
    CHECK( tinfra::mo_equals(xb, sample_a));    
}

} // end suite tinfra

