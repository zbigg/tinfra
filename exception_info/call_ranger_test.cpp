#include "call_ranger.h"

#include "tinfra/test.h"

SUITE(tinfra) {

TEST(call_ranger_test_nesting)
{
    CHECK(tinfra::call_ranger::get_thread_local() == 0);
    {
        CALL_RANGER_GLOBAL();
        tinfra::call_ranger_frame* parent = tinfra::call_ranger::get_thread_local();
        CHECK(parent != 0);

        {
            CALL_RANGER_GLOBAL();
            tinfra::call_ranger_frame* nested = tinfra::call_ranger::get_thread_local();

            CHECK(parent != nested);
            CHECK_EQUAL(parent, nested->previous);
        }
    }
}

TEST(call_ranger_test_variable_registration)
{
    CALL_RANGER_GLOBAL();

    CHECK(tinfra::call_ranger::get_thread_local() != 0 );

    CHECK(tinfra::call_ranger::get_thread_local()->variables == 0);

    int z = 666;
    const char* arg = "abc";
    CALL_RANGER_ARG(z);

    const tinfra::call_ranger_variable* z_entry = tinfra::call_ranger::get_thread_local()->variables;
    {
        CHECK( z_entry != 0);

        CHECK_EQUAL(&z,   z_entry->object);
        CHECK_EQUAL("z",  z_entry->name);
        CHECK(z_entry->next == 0);
    }

    CALL_RANGER_ARG(arg);

    const tinfra::call_ranger_variable* arg_entry = tinfra::call_ranger::get_thread_local()->variables;
    {
        CHECK( arg_entry != 0 );

        CHECK_EQUAL(&arg,     arg_entry->object);
        CHECK_EQUAL("arg",    arg_entry->name);
        CHECK_EQUAL(z_entry,  arg_entry->next);
    }

    {
        CALL_RANGER_GLOBAL();
        const char* foo = "foo";
        CALL_RANGER_ARG(foo);

        tinfra::dump_call_info(*tinfra::call_ranger::get_thread_local(), tinfra::out, true);
    }
}

TEST(call_ranger_test_calls_callback_on_exception)
{
    tinfra::call_ranger::init_default_callback();

    try {
        int id = 777;
        const char* arg = "abc";

        CALL_RANGER_GLOBAL();
        CALL_RANGER_ARG(id);
        CALL_RANGER_ARG(arg);

        throw int(2);
    } catch( int foo) {
    }
}


} // end SUITE(tinfra)

int main(int argc, char** argv)
{
    return tinfra::test::test_main(argc, argv);
}


