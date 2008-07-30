#include <exception>
#include <iostream>
#include <iomanip>
#include <string>
#include <vector>

#include "exception.h"

#ifdef linux
#include <execinfo.h>
#include <signal.h>
#define HAVE_BACKTRACE
#endif

namespace tinfra {

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

void print_stacktrace(std::ostream& out, int ignore_frames)
{
    stacktrace_t stacktrace;
    populate_stacktrace(stacktrace,ignore_frames);

    for( stacktrace_t::const_iterator i = stacktrace.begin(); i != stacktrace.end(); ++i ) {
            out << std::setfill('0') << std::setw(sizeof(i->address)) << std::hex << i->address 
                << " (" << i->symbol << ")" 
                << std::endl;
    }
}
//
// generic_exception implementation
//

generic_exception::generic_exception(std::string const& message): _message(message) {
    populate_stacktrace(_stacktrace, 2);
    /*
    std::cerr << "exception: " << message << std::endl;
    for( stacktrace_t::const_iterator i = _stacktrace.begin(); i != _stacktrace.end(); ++i ) {
	std::cerr << "    at 0x" << std::setfill('0') << std::setw(8) << std::hex << (long)i->address 
	          << "(" << i->symbol << ")" << std::endl;
    }
    */
}


} // end of namespace tinfra
