#ifndef tinfra_call_ranger_h_included
#define tinfra_call_ranger_h_included

#include "tinfra/trace.h"  // for tinfra::stacktrace_t
#include "tinfra/stream.h" // for tinfra::output_stream, tinfra::err
#include "tinfra/safe_debug_print.h" // for safe_debug_print_func & make_safe_debug_print_func

#include "exception_info.h" // for 


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
///
/// use cases
///   a well armored public function is full of several assertions
///   that in traditional c++ look like this:
///   
///   if( something ) {
///       throw_or_return_failure("foo: %s", some_value);
///   }
///   the problem is that the message content is usually very limited 
///   and don't show all local influential variables (arguments, intermediates)
///   mostly because it's too heavy to dump all arguments in every exception
///   (which would be wrong for other reasons)
///   
///   call_ranger allows to use diagnostic facility (tracer/logger) to
///   dump all 'influential variables' when exiting function with exception
///   which in 'more-or-less stateless' function shall constitute enough
///   input for developer to analyze exception/assertion reason.

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

struct call_ranger_frame {
    // where call ranger has been instantiated ?
    const tinfra::source_location* source_location;

    // previous stack
    call_ranger_frame*              previous;

    // registered local variables
    call_ranger_variable*           variables;
};

void dump_call_info(call_ranger_frame const& frame, tinfra::output_stream& out, bool recursive);
void log_exceptional_leave(call_ranger_frame const& frame, tinfra::output_stream& out, bool recursive);

struct call_ranger_callback {
    virtual ~call_ranger_callback() {}
    virtual void exceptional_leave(call_ranger_frame& data) = 0;
};

struct call_ranger {
    // thread local interface
    static call_ranger_frame* get_thread_local();

    // global settings
    static void                  init_default_callback();
    static call_ranger_callback* get_default_callback();
    static void                  set_default_callback(call_ranger_callback*);

    // internal details
    call_ranger_frame* get_frame();

    call_ranger_frame* get_previous_frame() const;
        /// get pointer to previous (i.e parent)
        /// call_ranger for this thread 

    tinfra::source_location get_source_location() const;
        /// return source location info of this
        /// instance

    const call_ranger_variable* get_variables() const;
        /// return single linked list of variables connected to
        /// this instance

    call_ranger(tinfra::source_location const* source_location, call_ranger_callback* callback = 0);
    ~call_ranger();
    
    void push_variable(call_ranger_variable*);
private:
    call_ranger_frame      frame;
    call_ranger_callback*  callback;

    void register_instance();
    void unregister_instance();
};


//
// implementation classes: call_ranger_variable & call_ranger
//


//
// inline implementation
//

inline call_ranger::call_ranger(tinfra::source_location const* sl, call_ranger_callback* callback)
{
    this->frame.source_location = sl;
    this->frame.variables = 0;
    this->callback  = callback;
    this->register_instance();
}

inline call_ranger::~call_ranger()
{
    if( TINFRA_UNLIKELY(tinfra::exception_info::is_exception_active() && this->callback) ) {
        this->callback->exceptional_leave(this->frame);
    }
    this->unregister_instance();
}



} // end namespace tinfra

#endif // tinfra_call_ranger_h_included

