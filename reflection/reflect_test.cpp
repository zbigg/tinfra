#include "reflect.h"

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
        TINFRA_ASSERT(foo == "abcdefghijklmnoprstuvw1234567890");
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
int main()
{
    using std::vector;
    using tinfra::any;
    using namespace tinfra::reflect;
    
    
    foo_implementation lm;
    {
        method_info* mi  = default_method_info::create("no_arg", &foo_interface::no_arg);
        vector<any> args;
        any result = mi->invoke(any::by_ref(lm), args);
        
        lock_managet_status ifoo = result.get<lock_managet_status>();
        TINFRA_ASSERT(ifoo.lock_count == 666);
    }
    {
        method_info* mi = default_method_info::create("one_arg_const_ref", &foo_interface::one_arg_const_ref);
        vector<any> args;
        args.push_back(any::from_copy<std::string>("abcdefghijklmnoprstuvw1234567890"));
        any result = mi->invoke(any::by_ref(lm), args);
        
        lock_handle ifoo = result.get<lock_handle>();
        TINFRA_ASSERT(ifoo == 777);
    }
    
    {
        method_info* mi = default_method_info::create("one_arg_ref", &foo_interface::one_arg_ref);
        vector<any> args;
        std::string foo("encepencewktorejrence");
        args.push_back(any::by_ref(foo));
        any result = mi->invoke(any::by_ref(lm), args);
        
        TINFRA_ASSERT(foo == "bols");
    }
    
    {
        method_info* mi = default_method_info::create("no_return", &foo_interface::no_return);
        vector<any> args;
        std::string foo("encepencewktorejrence");
        args.push_back(any::by_ref(foo));
        mi->invoke(any::by_ref(lm), args);
        
        TINFRA_ASSERT(foo == "bols");
    }
}
