#include "exception_info.h"

#include <typeinfo>

#include "tinfra/test.h"

SUITE(tinfra) {

TEST(exception_info_constructor)
{
    using tinfra::exception_info;
    exception_info r;
    CHECK(!r.exception_object);
    CHECK(!r.exception_type);
    CHECK_EQUAL(0, r.throw_stacktrace.size());
}

TEST(exception_info_basics)
{
    using tinfra::exception_info;
    exception_info r;
    CHECK_EQUAL(false, exception_info::is_exception_active());
    CHECK_EQUAL(false, exception_info::get_current_exception(r));
}

TEST(exception_info_simple_throw)
{
    using tinfra::exception_info;
    exception_info r;
    try {
        CHECK_EQUAL(false, exception_info::is_exception_active());
        throw int(666);
    } catch( ... ) {
        CHECK_EQUAL(true, exception_info::is_exception_active());
        CHECK_EQUAL(true, exception_info::get_current_exception(r));

        CHECK(r.exception_object != 0);
        CHECK(r.exception_type = &typeid(int));
        CHECK_EQUAL(666, *(reinterpret_cast<int*>(r.exception_object)));

        CHECK(r.throw_stacktrace.size() > 0);

    }
    CHECK_EQUAL(false, exception_info::is_exception_active());
}

struct with_exception_check_in_destructor {
    bool expects_exception;
    
    ~with_exception_check_in_destructor()
    {
        CHECK_EQUAL(this->expects_exception, tinfra::exception_info::is_exception_active());
        CHECK_EQUAL(this->expects_exception, std::uncaught_exception());
        if( this->expects_exception ) {
            using tinfra::exception_info;
            exception_info r;

            CHECK_EQUAL(true, exception_info::get_current_exception(r));

            CHECK(r.exception_object != 0);
            CHECK(r.exception_type = &typeid(int));
            CHECK_EQUAL(666, *(reinterpret_cast<int*>(r.exception_object)));
        }
    }
};
TEST(exception_info_visible_in_desctructor)
{
    // check without exception
    {
        with_exception_check_in_destructor foo = { false };
    }
    
    // check with exception
    {
        try {
            with_exception_check_in_destructor foo = { true };
            throw 666;
        } catch( int ) {
        } 
    }
}

}

int main(int argc, char** argv)
{
    return tinfra::test::test_main(argc, argv);
}


