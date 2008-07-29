#include "tinfra/exception.h"
///
/// fatal exception handling for posix-like systems
///

#include <iostream>
#include <csignal>

static void (*fatal_exception_handler) (void) = 0;

namespace tinfra {
extern "C" void tinfra_fatal_sighandler(int)
{
    print_stacktrace(std::cerr,2);
    if( fatal_exception_handler ) {
	fatal_exception_handler();
    }
    signal(SIGABRT, SIG_DFL);
    abort();
}
void initialize_fatal_exception_handler()
{    
    std::signal(SIGSEGV, &tinfra_fatal_sighandler);
    std::signal(SIGBUS,  &tinfra_fatal_sighandler);
    std::signal(SIGABRT, &tinfra_fatal_sighandler);
    
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



