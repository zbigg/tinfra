//
// Copyright (c) 2009, Zbigniew Zagorski
// This software licensed under terms described in LICENSE.txt
//

#include "tinfra/platform.h"

#include "tinfra/runtime.h"

#include "tinfra/exeinfo.h"
#include "tinfra/lex.h"
#include "tinfra/subprocess.h"
#include "tinfra/string.h"

#include <exception>
#include <iomanip>
#include <string>
#include <sstream>
#include <vector>
#include <stdexcept>

#include <stdio.h>

#include <signal.h>

#ifdef linux
#include <execinfo.h>
#define HAVE_BACKTRACE
#endif

extern "C" void tinfra_fatal_sighandler(int signo);
extern "C" void tinfra_interrupt_sighandler(int);

extern char** environ;

namespace tinfra {

environment_t get_environment()
{
    environment_t result;
    for( char** ie = environ; *ie != 0; ++ie ) {
        tstring all(*ie);
        size_t eq_pos = all.find_first_of('=');
        if( eq_pos == tstring::npos ) 
            continue;
        tstring name = all.substr(0, eq_pos);
        tstring value = all.substr(eq_pos+1);
        
        result[name.str()] = value.str();
    }
    return result;
}

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
    
    ::signal(SIGINT,  &tinfra_interrupt_sighandler);
    ::signal(SIGTERM, &tinfra_interrupt_sighandler);
}

void uninstall_abort_handler()
{
    signal(SIGABRT, SIG_DFL);
}

} // end of namespace tinfra

extern "C" void tinfra_fatal_sighandler(int signo)
{
    char buf[64];
    snprintf(buf, sizeof(buf), "fatal signal %i received", signo);
    tinfra::fatal_exit(buf);
}

extern "C" void tinfra_interrupt_sighandler(int)
{
    tinfra::interrupt();
}


