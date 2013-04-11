#ifndef tinfra_call_ranger_h_included
#define tinfra_call_ranger_h_included

#include "tinfra/trace.h"  // for tinfra::stacktrace_t
#include "tinfra/stream.h" // for tinfra::output_stream, tinfra::err

#include "exception_info.h" // for 
#include "safe_debug_print.h" // for safe_debug_print_func & make_safe_debug_print_func


namespace tinfra {

/// call ranger
///
/// call ranger is an stack-allocated object that instruments
/// call frame with following information
///  - can attach "on-demand" debugging information
///  - can be queried (app can see what is on stack above for debugging purpose)
///  - can see whether function is is exiting cleanly or with exception
///
/// usage example:
///     int clazz::method(std::string foo) {
///         CALL_RANGER_GLOBAL();
///         CALL_RANGER_ARG(foo);
///         int x = call_something();
///         db->exec_something(x);
///     }
///   
/// now, call ranger will feature following
/// 1) when exiting from 'method' exception flag is checked and if 
///    exception is active, then all args are dumped into trace log
///    with brief information about "failure-mode" exit
/// 2) when abort() is called from withing function
///    then all registered rangers dump it's state to tracer, so
///    trace contains "pseudo call stack WITH ARGUMENT VALUES"
///
/// note, arguments must have overloaded call_guard_debug_dump class
/// and provide "lock/memory/libc-call-LESS" debug dump of particular
/// variable

#define CALL_RANGER(tracer)  CALL_RANGER_IMPL(tracer)
#define CALL_RANGER_GLOBAL() CALL_RANGER_IMPL(tinfra::global_tracer)
#define CALL_RANGER_ARG(var) CALL_RANGER_ARG_IMPL(var)

//
// implementation macros
//

#ifndef TINFRA_SOURCE_LOCATION_INIT
// TBD, move to tinfra/trace.h
#define TINFRA_SOURCE_LOCATION_INIT { __FILE__, __LINE__, TINFRA_SHORT_FUNCTION }
#endif

#define CALL_RANGER_IMPL(tracer) \
    static tinfra::source_location _tinfra_local_call_ranger_loc = TINFRA_SOURCE_LOCATION_INIT; \
    tinfra::call_ranger            _tinfra_local_call_ranger(&_tinfra_local_call_ranger_loc)

#define CALL_RANGER_ARG_IMPL(variable_name) \
    tinfra:: call_ranger_variable _tinfra_local_call_ranger_var ## variable_name = { 0, #variable_name, (const void*)&(variable_name), tinfra::make_safe_debug_print_func(variable_name) }; \
    _tinfra_local_call_ranger.push_variable(&(_tinfra_local_call_ranger_var ## variable_name))

//
// implementation classes: call_ranger_variable & call_ranger
//

typedef void (*call_ranger_printer_func)(int fd, const void* object);

struct call_ranger_variable {
	call_ranger_variable* next;    // next variable (actually, previous in order of declaration)
	
	const char*           name;    // variable name
	const void*           object;  // object pointer
	safe_debug_print_func printer; // object printer function
};

struct call_ranger {
    call_ranger(tinfra::source_location const* source_location);
    ~call_ranger();

    static call_ranger* get_thread_local();

    void dump_info(tinfra::output_stream& out, bool recursive);
    void inform_about_exceptional_leave(tinfra::output_stream& out);
    
    void push_variable(call_ranger_variable*);
    call_ranger_variable* first_variable();
    
private:
    const tinfra::source_location* source_location;
    call_ranger*          previous;

    call_ranger_variable* variables;
    
    void register_instance();
    void unregister_instance();
};


//
// implementation classes: call_ranger_variable & call_ranger
//


//
// inline implementation
//

inline call_ranger::call_ranger(tinfra::source_location const* sl):
    source_location(sl),
    previous(0),
    variables(0)
{
    this->register_instance();
}

inline call_ranger::~call_ranger()
{
    if( tinfra::exception_info::is_exception_active() ) {
        this->inform_about_exceptional_leave(tinfra::err);
    }
    this->unregister_instance();
}



} // end namespace tinfra

#endif // tinfra_call_ranger_h_included

