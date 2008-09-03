#ifndef __tinfra_runtime_h__
#define __tinfra_runtime_h__

#include <vector>
#include <string>

#include "tinfra/exeinfo.h"

namespace tinfra {

#define TINFRA_THROW(a) do { \
    std::cerr << tinfra::get_exepath() << ": " << # a << std::endl; \
    if( false && tinfra::is_stacktrace_supported() ) { \
        tinfra::stacktrace_t t; \
        if( tinfra::get_stacktrace(t) ) \
            tinfra::print_stacktrace(t, std::cerr); \
    } else { \
        std::cerr << "\tat " << __func__ << "(" << __FILE__ << ":" << __LINE__ << ")" << std::endl; \
    } \
    throw a; \
    } while(0)

typedef std::vector<void*> stacktrace_t;

//
// stacktrace support
//

///
/// Runtime check if stackrace is supported on this platform.
///
bool is_stacktrace_supported();

///
/// Get stacktrace of current context.
///
bool get_stacktrace(stacktrace_t& t);

///
/// Print the stacktrace.
///
void print_stacktrace(stacktrace_t const& st, std::ostream& out);


// debug information support

struct debug_info {
    std::string source_file;
    int         source_line;
    std::string function;
};

bool get_debug_info(void* address, debug_info& dest);
///
/// Initialize handler for fatal exception (win32 structured, SIGSGV etc)
///
/// handler is invoked before terminating program.
///

void initialize_fatal_exception_handler();
void set_fatal_exception_handler(void (*handler) (void));

/// Call this if you're in place there is no escape
///
/// Prints message, stacktrace (if possible) and aborts.
void fatal_exit(const char* message);

/// Fatal exit printing given stackrace.
///
/// Prints message, stacktrace (if not empty) and aborts.
/// Used internally.
void fatal_exit(const char* message, stacktrace_t& stacktrace);

/// Gets the stack bottom for this thread.
///
/// Reliable on linux and on threads created using tinfra::Thread 
/// and tinfra::ThreadSet.
/// On other platforms returns approximation.
const void* get_stack_bottom();

}

#endif

