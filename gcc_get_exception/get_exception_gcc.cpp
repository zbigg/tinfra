#include "get_exception.h"	// we implement this

#include <string>	// for std::string
#include <map>		// for std::map
#include <cxxabi.h>	// for ::__do_upcast
#include <execinfo.h> 	// for ::backtrace
#include <cstdlib>	// for std::free, std::malloc
#include <dlfcn.h>  	// for ::dlsym
#include <typeinfo> 	// for std::typeinfo
#include <iostream>

#if 1
#include <iostream>
#define TRACE(a) std::cerr << __FILE__ << ":" << __LINE__ << ": " << a << std::endl
#else
#define TRACE(a)
#endif

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

static std::string demangle(const char* raw_name)
{
	std::string result = raw_name;
	int status = 0;
	char* data = abi::__cxa_demangle(raw_name, NULL, NULL, &status);
	if( status == 0 ) {	
		result.assign(data);
		std::free(data);
	}
	return result;
}

struct converter_type_info {
	any_to_string_t       to_string;
	std::type_info const* type;
};

typedef std::map<std::string, converter_type_info> converters_map_t;
converters_map_t converters;

void register_exception_type(std::type_info const& ti, any_to_string_t converter)
{
    TRACE("TI: registered converter for " << ti.name());
    converter_type_info n = { converter, &ti };
    converters[ti.name()] = n;
}

std::string get_exception_string(std::type_info const& ti, void* exception_ptr)
{
    std::string exc_name = demangle(ti.name());
    std::string exc_descr = "??";
    TRACE("TI: looking for exception n=" << ti.name() << " d=" << exc_name);
    if( converters.find(ti.name()) != converters.end()) {
            
        any_to_string_t converter = converters[ti.name()].to_string;
        TRACE("TI: found direct converter");
        exc_descr = converter(exception_ptr);
    } else {
        // search for candidate to which we can upcast
        for( converters_map_t::const_iterator i = converters.begin(); i!=converters.end(); ++i ) {
            converter_type_info const& cti = i->second;
            void* adjusted_ptr = exception_ptr;
            if( ti.__do_upcast(dynamic_cast<const abi::__class_type_info*>(cti.type),&adjusted_ptr) ) {
                TRACE("TI: found upcast to n=" << cti.type->name());
                any_to_string_t converter = cti.to_string;
                exc_descr = converter(adjusted_ptr);
                break;
            }
        }
    }
    std::ostringstream buf;
    buf << exc_name << "(" << exc_descr << ")";
    return buf.str();
}

__thread void* current_exc_ptr = 0;
__thread std::type_info const* current_exc_type = 0;
__thread void* current_exc_stacktrace_frames[100];
__thread size_t current_exc_stacktrace_count = 0;

exception_info::exception_info()
    : pointer(0), type(0)
{
}

/// Get information about current active exception.
/// @return NULL if no exception is active
bool get_exception_info(exception_info& result)
{
    __cxa_eh_globals* gp = __cxa_get_globals();
	
    if( gp == 0 ) {
        return false;
    } else if( gp->caughtExceptions != 0 ) {
        __cxa_exception* cp = gp->caughtExceptions;
        result.pointer = cp->adjustedPtr;
        result.type = cp->exceptionType;
        return true;
    } else if( gp->uncaughtExceptions > 0 ) {
        result.pointer = current_exc_ptr;
        result.type = current_exc_type;
        return true;
    }
    return false;
}

/// Get string describing exception information.
std::string get_any_exception_string(exception_info const& einfo)
{
    return get_exception_string(*einfo.type, einfo.pointer);
}

std::string get_any_exception_string()
{
    exception_info einfo;
    if( !get_exception_info(einfo) ) {
        return "";
    }
    return get_any_exception_string(einfo);
}

bool        is_exception_active()
{
    __cxa_eh_globals* gp = __cxa_get_globals();
	
    if( gp == 0 || (gp->caughtExceptions == 0 && gp->uncaughtExceptions == 0)) {
        return false;
    } else {
        return true;
    }
}

stacktrace_raw get_current_exception_stacktrace()
{
	void** ptr = current_exc_stacktrace_frames;
	std::vector<void*> result(ptr+1, ptr+current_exc_stacktrace_count);
	return result;
}

stacktrace_symbols get_stacktrace_symbols(stacktrace_raw const& stack)
{
	// this is very rare situation we might consider running
	// addr2line
	char** stack_symbols_raw = ::backtrace_symbols(& stack[0], stack.size() );
	if( stack_symbols_raw == 0 ) {
		return stacktrace_symbols();
	}
	try {
		stacktrace_symbols result( stack_symbols_raw, stack_symbols_raw+stack.size() ); // might throw!
	
		::free(stack_symbols_raw);
		return result;
	} catch( ... ) {
		::free(stack_symbols_raw);
		throw;
	}
}

typedef void (*cxa_throw_type)(void* , void *, void (*) (void *));

cxa_throw_type orig_cxa_throw = 0;

cxa_throw_type load_orig_throw_code()
{
	cxa_throw_type cxa_throw_ptr = (cxa_throw_type)( dlsym(RTLD_NEXT, "__cxa_throw") );
	//std::cerr << "cxa_throw_hijack: loaded original cxa_throw " << (void*)cxa_throw_ptr << "\n";
	return cxa_throw_ptr;
}

extern "C" 
void __cxa_throw(void *thrown_exception, void *pvtinfo, void (*dest) (void *) )
{
	std::type_info const* tinfo = reinterpret_cast<std::type_info const*>(pvtinfo);	
	//std::cerr << "CXA_THROW: detected throw of exception of " << tinfo->name()<< "\n";
	if( orig_cxa_throw == 0 ) {
		orig_cxa_throw = load_orig_throw_code();
	}
	if( orig_cxa_throw == 0 || orig_cxa_throw == &__cxa_throw ) {
		std::cerr << "CXA_THROW: unable to find original g++ throw routine (__cxa_throw), aborting " << tinfo->name()<< "\n";
		abort();
	}
	// remember currently active exception
        current_exc_ptr = thrown_exception;
        current_exc_type = tinfo;
        
        // remember stacktrace of exception
        {
        	const int MAX_SIZE = sizeof(current_exc_stacktrace_frames)/sizeof(void*);
        	const int size = ::backtrace(current_exc_stacktrace_frames, MAX_SIZE);
        	current_exc_stacktrace_count = size;
        	TRACE("got stacktrace of throw(). size=" << size); 
        }
        
	orig_cxa_throw(thrown_exception, pvtinfo, dest);
}

