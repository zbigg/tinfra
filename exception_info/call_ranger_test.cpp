#include "call_ranger.h"

#include "tinfra/test.h"

SUITE(tinfra) {

TEST(call_ranger_test_nesting)
{
    CHECK(tinfra::call_ranger::get_thread_local() == 0);
    
    {
        CALL_RANGER_GLOBAL();
        tinfra::call_ranger* parent = tinfra::call_ranger::get_thread_local();
        CHECK(parent != 0);
        
        {
            CALL_RANGER_GLOBAL();
            tinfra::call_ranger* nested = tinfra::call_ranger::get_thread_local();
            
            CHECK(parent != nested);
        }
    }
}

TEST(call_ranger_test_variable_registration)
{
    CALL_RANGER_GLOBAL();
    
    CHECK(tinfra::call_ranger::get_thread_local() != 0 );

    int z = 2;
    const char* arg = "abc";
    CALL_RANGER_ARG(z);
    CALL_RANGER_ARG(arg);
    
    tinfra::call_ranger::get_thread_local()->dump_info(tinfra::out, true);
}
}

int main(int argc, char** argv)
{
    return tinfra::test::test_main(argc, argv);
}


