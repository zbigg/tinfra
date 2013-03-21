#include "exception_info.h"

#include "tinfra/trace.h"

#include <cxxabi.h>	    // for ::__do_upcast
#include <execinfo.h> 	// for ::backtrace
#include <typeinfo>

//
// __cxa exception runtime definitions
//

struct __cxa_exception { 
	std::type_info * exceptionType;
	void (*exceptionDestructor) (void *); 
	void* unexpectedHandler;
	void* terminateHandler;
	__cxa_exception * nextException;
	
	int   handlerCount;
	int   handlerSwitchValue;
	const char *        actionRecord;
	const char * languageSpecificData;
	void *  catchTemp;
	void *  adjustedPtr;
	
	void*         unwindHeader;
};

struct __cxa_eh_globals {
	__cxa_exception * caughtExceptions;
	unsigned int      uncaughtExceptions;
};

extern "C" __cxa_eh_globals* __cxa_get_globals();

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
    __cxa_eh_globals* gp = __cxa_get_globals();

    if( gp == 0 || (gp->caughtExceptions == 0 && gp->uncaughtExceptions == 0)) {
        return false;
    } else {
        return true;
    }
}

static void get_current_exception_stacktrace(stacktrace_t& vec)
{
	void** ptr = current_exc_stacktrace_frames;
	vec.assign(ptr+1, ptr+current_exc_stacktrace_count);
}

bool exception_info::get_current_exception(exception_info& result)
{
    __cxa_eh_globals* gp = __cxa_get_globals();
    if( gp == 0 ) {
        return false;
    } else if( gp->caughtExceptions != 0 ) {
        __cxa_exception* cp = gp->caughtExceptions;
        result.exception_object = cp->adjustedPtr;
        result.exception_type = cp->exceptionType;
        get_current_exception_stacktrace(result.throw_stacktrace);
        return true;
    } else if( gp->uncaughtExceptions > 0 ) {
        result.exception_object = current_exc_ptr;
        result.exception_type = current_exc_type;
        get_current_exception_stacktrace(result.throw_stacktrace);
        return true;
    }
    return false;
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
        //TRACE("got stacktrace of throw(). size=" << size); 
    }
}

} // end namespace tinfra

