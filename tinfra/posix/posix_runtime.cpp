#include <exception>
#include <iostream>
#include <iomanip>
#include <string>
#include <sstream>
#include <vector>
#include <stdexcept>


#include "tinfra/exception.h"
#include "tinfra/exeinfo.h"
#include "tinfra/tinfra_lex.h"
#include "tinfra/subprocess.h"

#ifdef linux
#include <execinfo.h>
#include <signal.h>
#define HAVE_BACKTRACE
#endif

extern "C" void tinfra_fatal_sighandler(int signo);

namespace tinfra {

bool is_stacktrace_supported()
{
#ifdef HAVE_BACKTRACE
    return true;
#else
    return false;
#endif
}

bool get_stacktrace(stacktrace_t& dest)
{
#ifdef HAVE_BACKTRACE
    // Reference
    // http://www-128.ibm.com/developerworks/library/l-cppexcep.html?ca=dnt-68
    // http://www.delorie.com/gnu/docs/glibc/libc_665.html
    //
    void* addresses[256];
    int size = ::backtrace(addresses, 256);    
    const int ignore_stacks = 1;
    dest.reserve(size-ignore_stacks);
    for( int i = ignore_stacks; i < size; i++ ) {
	dest.push_back(addresses[i]);
    }
    return true;
#else
    return false;
#endif
}

bool get_debug_info(void* address, debug_info& result)
{
    std::ostringstream cmd;
    cmd << "addr2line -e " << get_exepath() << " -sfC " << std::hex << address;
    using std::vector;
    using std::string;
    string cmdo;
    try {
        cmdo = capture_command(cmd.str());
    } catch( std::exception& e) {
        return false;
    }
    vector<string> lines = split(cmdo,"\r\n");
    if( lines.size() != 2 ) 
        return false;
    
    vector<string> place = split(lines[1], ":");
    
    if( place.size() != 2 ) 
        return false;
    
    if( lines[0] == "??" || place[0] == "??" ) 
        return false;
    
    result.function = lines[0];
    result.source_file = place[0];
    from_string<int>(place[1], result.source_line);
    return true;
}

void initialize_platform_runtime()
{
    ::signal(SIGSEGV, &tinfra_fatal_sighandler);
    ::signal(SIGBUS,  &tinfra_fatal_sighandler);
    ::signal(SIGABRT, &tinfra_fatal_sighandler);    
}

} // end of namespace tinfra

extern "C" void tinfra_fatal_sighandler(int signo)
{
    signal(SIGABRT, SIG_DFL);
    char buf[64];
    snprintf(buf, sizeof(buf), "fatal signal %i received", signo);
    
    tinfra::fatal_exit(buf);
}


