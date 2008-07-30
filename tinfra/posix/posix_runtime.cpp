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

bool is_stacktrace_supported()
{
#ifdef HAVE_BACKTRACE
    return true;
#else
    return false;
#endif
}
bool get_stacktrace(stacktrace_t& t) 
{
#ifdef HAVE_BACKTRACE
    // Reference
    // http://www-128.ibm.com/developerworks/library/l-cppexcep.html?ca=dnt-68
    // http://www.delorie.com/gnu/docs/glibc/libc_665.html
    //
    void* addresses[256];
    int size = ::backtrace(addresses, 256);
    char** symbols = ::backtrace_symbols(addresses,size);
    dest.reserve(size-ignore_stacks);
    for( int i = ignore_stacks; i < size; i++ ) {
	stackentry a;
	a.address = addresses[i];
	a.symbol = symbols[i];
	dest.push_back(a);
    }
    ::free(symbols);
    return true;
#else
    return false;
#endif
}

} // end of namespace tinfra
