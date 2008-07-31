#ifndef __tinfra_runtime_h__
#define __tinfra_runtime_h__

#include <vector>
#include <string>

namespace tinfra {

struct stackframe {
    void*       address;
    
    std::string symbol;
    std::string file_name;
    int         line_number;
    
    stackframe();
};

typedef std::vector<stackframe> stacktrace_t;

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

///
/// Initialize handler for fatal exception (win32 structured, SIGSGV etc)
///
/// handler is invoked before terminating program.
///

void initialize_fatal_exception_handler();
void set_fatal_exception_handler(void (*handler) (void));

}

#endif

