#include "get_exception.h"

#include <cassert>
#include <sstream>
#include <iostream>

std::string std_exception_str(std::exception const* e)
{
    return e->what();
}

std::string int_exception_str(int const* e)
{
    std::ostringstream buf;
    buf << *e;
    return buf.str();
}

void test_non_active_outside_catch()
{
    assert(is_exception_active() == false);
    
    exception_info a;
    assert(get_exception_info(a) == false);
}

void test_exception_active_in_catch()
{
    try {
        throw 33;
    } catch( int ) {
        assert(is_exception_active() == true);
        exception_info a;
        assert(get_exception_info(a) == true);
        assert(*a.type == typeid(int));
        assert(*(int*)a.pointer == 33);
        assert(get_any_exception_string() == "int(33)");
    }
}

struct foo_class {
    ~foo_class() {
        assert(is_exception_active() == true);
        exception_info a;
        assert(get_exception_info(a) == true);
        assert(*a.type == typeid(int));
        assert(*(int*)a.pointer == 66);
        assert(get_any_exception_string() == "int(66)");
    }
};

// this hack works only on Unix with dynamic loader
// and relocatable __cxa_throw
void test_exception_active_in_destructor()
{
    try {
        foo_class f;
        (void)f;
        throw 66;
        // foo_class destroyed
    } catch( int ) {
    }
    assert(is_exception_active() == false);
    exception_info a;
    assert(get_exception_info(a) == false);
}

extern void throw_an_exception(int now_or_later);
extern int back_call(int a) { throw_an_exception(a); return 0;}

void test_exception_get_current_stacktrace_works()
{
    try {
    	::close(23);
        throw_an_exception(2);
        // foo_class destroyed
        ::close(329);

    } catch( ... ) {
	    stacktrace_raw raw_stacktrace = get_current_exception_stacktrace();
	    assert(raw_stacktrace.size() > 0);
	    
	    stacktrace_symbols stacktrace_with_symbols = get_stacktrace_symbols(raw_stacktrace);
	    
	    
	    assert(stacktrace_with_symbols.size() > 6);
	    assert(stacktrace_with_symbols[0].find("throw_an_exception") != std::string::npos);
	    assert(stacktrace_with_symbols[1].find("back_call") != std::string::npos);
	    assert(stacktrace_with_symbols[2].find("throw_an_exception") != std::string::npos);
	    assert(stacktrace_with_symbols[3].find("back_call") != std::string::npos);
	    assert(stacktrace_with_symbols[4].find("throw_an_exception") != std::string::npos);
	    assert(stacktrace_with_symbols[5].find("test_exception_get_current_stacktrace_works") != std::string::npos);
	    
	    
	    std::cout << "stacktrace:\n";
	    for( unsigned i = 0; i <stacktrace_with_symbols.size(); ++i ) {
	    	    std::cout << stacktrace_with_symbols[i] << "\n";
	    } 
    }
    std::vector<std::string> zzz(100);
}

int main()
{
    register_exception_type<std::exception>(&std_exception_str);
    register_exception_type<int>(&int_exception_str);
    
    test_non_active_outside_catch();
    test_exception_active_in_catch();
    test_exception_active_in_destructor();
    test_exception_get_current_stacktrace_works();
    
    return 0;
}
