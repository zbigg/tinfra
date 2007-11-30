#include <exception>
#include <iostream>
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

} // end of namespace tinfra
