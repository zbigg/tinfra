//
// Copyright (c) 2009, Zbigniew Zagorski
// This software licensed under terms described in LICENSE.txt
//

#include "../platform.h"
#ifdef TINFRA_POSIX

#include "tinfra/runtime.h"

#include "tinfra/exeinfo.h"
#include "tinfra/lex.h"
#include "tinfra/subprocess.h"
#include "tinfra/string.h"
#include "tinfra/trace.h"
#include "tinfra/logger.h"
#include "tinfra/fmt.h"
#include "tinfra/os_common.h"
#include "tinfra/fs.h" 

#include <exception>
#include <iomanip>
#include <string>
#include <sstream>
#include <vector>
#include <stdexcept>
#include <cerrno>

#include <stdio.h>

#include <signal.h>

#if defined(HAVE_BACKTRACE) && defined(HAVE_EXECINFO_H)
#include <execinfo.h>
#endif

#if defined(HAVE_DLADDR)
#include <dlfcn.h>
#if defined(__MACH__) && defined(__APPLE__)
#define DL_info dl_info
#endif
#endif

extern "C" void tinfra_fatal_sighandler(int signo, siginfo_t *, void *);
extern "C" void tinfra_interrupt_sighandler(int signo, siginfo_t *, void *);

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
    // web/howto:
    //     http://www-128.ibm.com/developerworks/library/l-cppexcep.html?ca=dnt-68
    //     http://www.delorie.com/gnu/docs/glibc/libc_665.html
    // linux: man backtrace
    // macosx: man backtrace
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

static std::string local_get_exepath()
{
#ifdef linux
    if( tinfra::fs::exists("/proc/self/exe") ) {
        return tinfra::fs::readlink("/proc/self/exe");
    } 
#endif    
    return get_exepath();

}

#ifdef linux
bool get_debug_info_addr2line(void* address, debug_info& result)
{
    std::ostringstream cmd;
    cmd << "addr2line -e " << local_get_exepath() << " -sfC " << std::hex << address;
    using std::vector;
    using std::string;
    string cmdo;
    try {
        cmdo = capture_command(cmd.str());
    } catch( std::exception& e) {
        return false;
    }
    vector<string> lines = split_lines(cmdo);
    if( lines.size() != 2 ) 
        return false;
    
    vector<string> place = split(lines[1], ':');
    
    if( place.size() != 2 ) 
        return false;
    
    if( lines[0] == "??" || place[0] == "??" ) 
        return false;
    
    result.function = lines[0];
    result.source_file = place[0];
    from_string<int>(place[1], result.source_line);
    return true;
}
#endif

#if defined(HAVE_DLADDR)
bool get_debug_info_dladdr(void* address, debug_info& result)
{
    DL_info info;
    memset(&info, 0, sizeof(info));
    int r = dladdr(address, &info);
    if( r == 0 ) {
        // TBD, at least trace what is wrong
        return false;
    }
    if( info.dli_sname != 0 ) {
        result.function = info.dli_sname;
        return true;
    }
    return false;
}
#endif

bool get_debug_info(void* address, debug_info& result)
{
#ifdef linux
    if( get_debug_info_addr2line(address,result) )
        return true
#endif
#ifdef HAVE_DLADDR
    if( get_debug_info_dladdr(address,result) )
        return true;
#endif
    return false;
}
static std::vector<const char*> sig_names;

static void my_signal(int signo, const char* name, void (*handler)(int, siginfo_t*, void*))
{
    struct sigaction act;
    memset (&act, '\0', sizeof(act));
 
	/* Use the sa_sigaction field because the handles has two additional parameters */
	act.sa_sigaction = handler;
 
	/* The SA_SIGINFO flag tells sigaction() to use the sa_sigaction field, not sa_handler. */
	act.sa_flags = SA_SIGINFO;
	if( sigaction(signo, &act, 0) == -1) {
	    TINFRA_LOG_ERROR(fmt("unable to install signal(%i) handler: %s") % signo % tinfra::errno_to_string(errno));
	}
	
	if( sig_names.size() <= signo ) {
	    sig_names.resize(signo+1);
	} 
	sig_names[signo] = name;
	// ensure that those pages are really loaded !
	strlen(name);
}
void initialize_platform_runtime()
{
    my_signal(SIGSEGV, "segmentation fault (SIGSEGV)", &tinfra_fatal_sighandler);
    my_signal(SIGBUS,  "bus error (SIGBUS)",           &tinfra_fatal_sighandler);
    my_signal(SIGABRT, "abort program (SIGABRT)", &tinfra_fatal_sighandler);
    my_signal(SIGILL,  "illegal instruction (SIGILL)", &tinfra_fatal_sighandler);
    
    my_signal(SIGINT,  "interrupt program (SIGINT)", &tinfra_interrupt_sighandler);
    my_signal(SIGTERM, "termination request (SIGTERM)", &tinfra_interrupt_sighandler);
}

void uninstall_abort_handler()
{
    signal(SIGABRT, SIG_DFL);
}

} // end of namespace tinfra

extern "C" void tinfra_fatal_sighandler(int signo, siginfo_t *, void *)
{
    char buf[64];
    const char* signame = "unknown signal";
    if( signo < tinfra::sig_names.size() )
        signame = tinfra::sig_names[signo];
    snprintf(buf, sizeof(buf), "fatal signal received: %s (signo=%i)", signame, signo);
    tinfra::fatal_exit(buf);
}

extern "C" void tinfra_interrupt_sighandler(int signo, siginfo_t *, void *)
{
    tinfra::interrupt();
}

#endif // TINFRA_POSIX

