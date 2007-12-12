
///
/// fatal exception handling for posix-like systems
///

#ifdef linux
#include <execinfo.h>
#include <signal.h>
#define HAVE_BACKTRACE
#endif

static void (*fatal_exception_handler) (void) = 0;

namespace tinfra {
    
void initialize_fatal_exception_handler()
{    
    // TODO: register signals
    // SIGSEGV: show_stack & abort
    // SIGINT: throw interrupt
    // SIGTERM: throw interrupt ? mark terminate flag
}

void set_fatal_exception_handler(void (*handler) (void))
{
    fatal_exception_handler = handler;
}

void populate_stacktrace(stacktrace_t& dest,int ignore_stacks)
{
#ifdef HAVE_BACKTRACE
    // Reference
    // http://www-128.ibm.com/developerworks/library/l-cppexcep.html?ca=dnt-68
    // http://www.delorie.com/gnu/docs/glibc/libc_665.html
    //
    void* addresses[256];
    int size = ::backtrace(addresses, 256);
    char** symbols = ::backtrace_symbols(addresses,size);
    for( int i = ignore_stacks; i < size; i++ ) {
	stackentry a;
	a.address = addresses[i];
	a.symbol = symbols[i];
	dest.push_back(a);
    }
    ::free(symbols);
#endif
}

#ifdef linux

template<int SIGNO>
class AutoExceptionHandler {
    public:
    AutoExceptionHandler() {
	std::cerr << "instantiating sig handler sig:" << SIGNO << " description: " << get_description() << std::endl;
	signal(SIGNO, &AutoExceptionHandler<SIGNO>::handler);
    }    
    static void handler(int signo)
    {
	std::cerr << "signal: " << signo << std::endl;
	signal(SIGNO, &AutoExceptionHandler<SIGNO>::handler);
	throw generic_exception(get_description());
    }
    static std::string get_description();
};

AutoExceptionHandler<SIGSEGV> sigsegv_handler;
AutoExceptionHandler<SIGINT>  sigint_handler;
AutoExceptionHandler<SIGTERM>  sigterrm_handler;

template<>
std::string AutoExceptionHandler<SIGSEGV>::get_description() {
    return "Segmentation fault Z";
}

template<>
std::string AutoExceptionHandler<SIGINT>::get_description() {
    return "Interrupt Z";
}

template<>
std::string AutoExceptionHandler<SIGTERM>::get_description() {
    return "Termination request Z";
}

#endif
}

