#include <exception>
#include <iostream>
#include <iomanip>
#include <string>
#include <vector>

#ifdef linux
#include <execinfo.h>
#include <signal.h>
#endif

#include "exception.h"

namespace tinfra {

void populate_stacktrace(stacktrace_t& dest,int ignore_stacks)
{
    // Reeference
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
}

//
// generic_exception implementation
//

generic_exception::generic_exception(std::string const& message): _message(message) {
    populate_stacktrace(_stacktrace, 1);
    std::cerr << "exception: " << message << std::endl;
    for( stacktrace_t::const_iterator i = _stacktrace.begin(); i != _stacktrace.end(); ++i ) {
	std::cerr << "    at 0x" << std::setfill('0') << std::setw(8) << std::hex << (long)i->address 
	          << "(" << i->symbol << ")" << std::endl;
    }
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

} // end of namespace tinfra
