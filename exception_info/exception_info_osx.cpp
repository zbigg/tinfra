#include "exception_info.h"

#include "tinfra/trace.h"

#include <cxxabi.h>	    // for __cxxabiv1
#include <execinfo.h> 	// for ::backtrace
#include <typeinfo>
#include <iostream>

namespace tinfra {

//
// exception_info
//

__thread void*                 current_exc_ptr = 0;
__thread std::type_info const* current_exc_type = 0;
__thread void*                 current_exc_stacktrace_frames[100];
__thread size_t                current_exc_stacktrace_count = 0;

exception_info::exception_info():
    exception_object(0), 
    exception_type(0)
{
}

bool exception_info::is_exception_active()
{
    return std::uncaught_exception() || __cxxabiv1::__cxa_current_primary_exception();
}

static void get_current_exception_stacktrace(stacktrace_t& vec)
{
    void** ptr = current_exc_stacktrace_frames;
    vec.assign(ptr+1, ptr+current_exc_stacktrace_count);
}

bool exception_info::get_current_exception(exception_info& result)
{
    result.exception_object = __cxxabiv1::__cxa_current_primary_exception();
    if( result.exception_object ) {
        result.exception_type = __cxxabiv1::__cxa_current_exception_type();
    } else if( std::uncaught_exception() ) {
        result.exception_object = current_exc_ptr;
        result.exception_type = current_exc_type;
    } else {
        return false;
    }
    
    get_current_exception_stacktrace(result.throw_stacktrace);
    return true;
}

void exception_info_callback(void *exception_object, std::type_info const* exception_type )
{
    current_exc_ptr  = exception_object;
    current_exc_type = exception_type;
    
    // remember stacktrace of exception
    {
        const int MAX_SIZE = sizeof(current_exc_stacktrace_frames)/sizeof(void*);
        const int size = ::backtrace(current_exc_stacktrace_frames, MAX_SIZE);
        current_exc_stacktrace_count = size;
        //std::cerr << "got stacktrace of throw(). size=" << size << "\n"; 
    }
}

} // end namespace tinfra

