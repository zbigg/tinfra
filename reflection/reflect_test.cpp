#include "reflect.h"
#include "manual_rtti_system.h"

#include "tinfra/test.h"
#include "tinfra/cmd.h"
#include "tinfra/typeinfo.h"

#include <map>
#include <iostream>

//
// test application
//

struct lock_managet_status {
    int lock_count;
};

typedef int lock_handle;

struct foo_interface {
    
    virtual lock_managet_status no_arg() = 0;
    virtual lock_handle         one_arg_const_ref(std::string const& name) = 0;
    virtual int                 one_arg_ref(std::string& result) = 0;
    virtual void                no_return(std::string& result) = 0;
    //virtual lock_handle         remove_lock(lock_handle& lh) = 0;
    
};

struct foo_implementation: public foo_interface {
    virtual lock_managet_status no_arg()
    {
        lock_managet_status res = { 666 };
        return res;
    }
    
    virtual lock_handle one_arg_const_ref(std::string const& foo)
    {
        TINFRA_ASSERT(foo == "abcdefghijklmnoprstuvw1registration_registration_harness34567890");
        return 777;
    }
    
    virtual int one_arg_ref(std::string& foo)
    {
        TINFRA_ASSERT(foo == "encepencewktorejrence");
        foo = "bols";
        return 1;
    }
    
    virtual void no_return(std::string& foo)
    {
        TINFRA_ASSERT(foo == "encepencewktorejrence");
        foo = "bols";
    }
};

#define CHECK_TYPES_EQUAL(a, b) \
    CHECK_EQUAL( typeid(a).name(), typeid(b).name() ) 

TEST(remove_const_test)
{
    using tinfra::reflect::remove_const;
    
    //CHECK_TYPES_EQUAL( int, float );
    
    CHECK_TYPES_EQUAL( int, typename remove_const<int>::type );
    CHECK_TYPES_EQUAL( int, remove_const<const int>::type);
    CHECK_TYPES_EQUAL( int, remove_const<int const>::type);
    CHECK_TYPES_EQUAL( int*, remove_const<int* const>::type);
    // and doesn't remove const of target type 
    CHECK_TYPES_EQUAL( const int&, remove_const<const int&>::type);    
}


TEST(remove_ref_test)
{
    tinfra::test::test_run_summary trs;
    
    using tinfra::reflect::remove_ref;
    
    CHECK_TYPES_EQUAL( int, remove_ref<int>::type);
    CHECK_TYPES_EQUAL( int, remove_ref<int&>::type);
    CHECK_TYPES_EQUAL( int&, remove_ref<const int&>::type);
    CHECK_TYPES_EQUAL( int*, remove_ref<int*&>::type);    
}

TEST(manual_rtti_system_basics)
{
    using namespace tinfra::reflect;
    manual_rtti_system R;
    
    type_info* ti = R.for_name("char");
    
    CHECK_EQUAL("char",     ti->name());
    CHECK_EQUAL(TIT_NORMAL, ti->type());
    CHECK_EQUAL(0,          ti->modifiers());
}

TEST(manual_rtti_system_derivation)
{
    using namespace tinfra::reflect;
    manual_rtti_system R;
    
    type_info* rti = R.ensure_registered<char*>("char*");
    type_info* ti  = R.for_name("char*");    
    CHECK_EQUAL(rti, ti);
    
    // check that target is the same as one registered for "char'
    type_info* char_ti  = R.for_name("char");
    CHECK_EQUAL(char_ti, ti->target_type());
    
    // check that later derivation returns same
    type_info* derived_ti  = R.derive(TIT_POINTER, char_ti);
    CHECK_EQUAL(derived_ti, rti);
    
    CHECK_EQUAL("char*",     ti->name());
    CHECK_EQUAL(TIT_POINTER, ti->type());
}

TEST(manual_rtti_check_all)
{
    using namespace tinfra::reflect;
    manual_rtti_system R;
    
    R.ensure_basics_registered<manual_rtti_system>("tinfra::reflect::manual_rtti_system");
    
    
    std::vector<type_info*> all_types = R.get_all();
    int idx = 0;
    for( std::vector<type_info*>::const_iterator i = all_types.begin(); i != all_types.end(); ++i ) {
        type_info* ti = *i;
        
        std::cout << idx++ <<        
             ": name:" << ti->name() <<
             " (type:" << ti->type() <<
             " mod:" << ti->modifiers() << 
             " target: " << (ti->target_type() 
                              ? ti->target_type()->name() 
                              : std::string("n/a") ) << 
             ")\n";
                                
    }
}

TEST(reflect_test)
{
    using std::vector;
    using tinfra::any;
    using namespace tinfra::reflect;
    using tinfra::reflect::remove_const;


    foo_implementation lm;
    {
        method_info* mi  = default_method_info::create("no_arg", &foo_interface::no_arg);
        vector<any> args;
        any result = mi->invoke(any::by_ref(lm), args);
        
        lock_managet_status ifoo = result.get<lock_managet_status>();
        CHECK_EQUAL(666, ifoo.lock_count);
    }
    {
        method_info* mi = default_method_info::create("one_arg_const_ref", &foo_interface::one_arg_const_ref);
        vector<any> args;
        args.push_back(any::from_copy<std::string>("abcdefghijklmnoprstuvw1registration_registration_harness34567890"));
        any result = mi->invoke(any::by_ref(lm), args);
        
        lock_handle ifoo = result.get<lock_handle>();
        CHECK_EQUAL(777, ifoo);
    }
    
    {
        method_info* mi = default_method_info::create("one_arg_ref", &foo_interface::one_arg_ref);
        vector<any> args;
        std::string foo("encepencewktorejrence");
        args.push_back(any::by_ref(foo));
        any result = mi->invoke(any::by_ref(lm), args);
        
        CHECK_EQUAL("bols", foo);
    }
    
    {
        method_info* mi = default_method_info::create("no_return", &foo_interface::no_return);
        vector<any> args;
        std::string foo("encepencewktorejrence");
        args.push_back(any::by_ref(foo));
        mi->invoke(any::by_ref(lm), args);
        
        CHECK_EQUAL("bols", foo);
    }
}

TINFRA_MAIN(tinfra::test::test_main);

