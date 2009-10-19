//
// Copyright (C) Zbigniew Zagorski <z.zagorski@gmail.com>,
// licensed to the public under the terms of the GNU GPL (>= 2)
// see the file COPYING for details
// I.e., do what you like, but keep copyright and there's NO WARRANTY.
//

#ifndef tinfra_runtime_h_included
#define tinfra_runtime_h_included

#include <vector>
#include <string>
#include <stdexcept>
#include <map>

#include "tinfra/exeinfo.h"

namespace tinfra {
    
typedef std::map<std::string, std::string> environment_t;

/// 
/// read all envornment variables into a map
///
environment_t get_environment();

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


/// Provoke keyboard interrupt.
///
/// Sets interrupted flag or aborts program depending on interrup settings.
///
/// Used internally by platform specific interrupt handler to singal handler.
void interrupt();

/// Interruption point.
///
/// May generate tinfra::interrupted_exception is program or thread was 
/// interrupted.
void test_interrupt();

enum interrupt_policy {
    IMMEDIATE_ABORT,
    DEFERRED_SIGNAL
};
/// Set interrupt policy.
///
/// Default interrupt policy is IMMEDIATE_ABORT
void set_interrupt_policy(interrupt_policy p);

class interrupted_exception: public std::exception {
public:
    interrupted_exception();
    
    const char* what() const throw ();
};

}

#endif

