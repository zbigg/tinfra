#include "tinfra/exception.h"
///
/// fatal exception handling for posix-like systems
///

#include <iostream>
#include <csignal>

static void (*fatal_exception_handler) (void) = 0;

namespace tinfra {
void fatal_sighandler(int)
{
    if( fatal_exception_handler ) {
	print_stacktrace(std::cerr,2);
	fatal_exception_handler();
    }
    abort();
}
void initialize_fatal_exception_handler()
{
    signal(SIGSEGV, &fatal_sighandler);
    signal(SIGBUS, &fatal_sighandler);
    
    // TODO: register signals
    // SIGSEGV: show_stack & abort
    // SIGINT: throw interrupt
    // SIGTERM: throw interrupt ? mark terminate flag
}

void set_fatal_exception_handler(void (*handler) (void))
{
    fatal_exception_handler = handler;
}

} //end namespace tinfra



