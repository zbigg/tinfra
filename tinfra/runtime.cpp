//
// Copyright (C) Zbigniew Zagorski <z.zagorski@gmail.com>,
// licensed to the public under the terms of the GNU GPL (>= 2)
// see the file COPYING for details
// I.e., do what you like, but keep copyright and there's NO WARRANTY.
//

#include <ostream>
#include <iostream>
#include <iomanip>

#include <cstring>
#include <cstdlib>

#include "tinfra/fmt.h"
#include "tinfra/runtime.h"

#if 0 // TODO woe32 part of this
#include <csignal>
using std::sigatomic_t;
#else
typedef int sigatomic_t;
#endif

namespace tinfra {
    
void print_stacktrace(stacktrace_t const& st, std::ostream& out)
{    

    for( stacktrace_t::const_iterator i = st.begin(); i != st.end(); ++i ) {
        void* address = *i;
        debug_info di;
        out << "\tat ";
        if( get_debug_info(address, di) ) {
            // func(file:line)
            out << di.function;
            if( di.source_file.size() > 0 ) {
                out << "(" << di.source_file;
                if( di.source_line != 0 )
                    out << ":" << std::dec << di.source_line;
                out << ")";
            }
        } 
        //  0xabcdef123
        out << "[" << std::setfill('0') << std::setw(sizeof(address)) << std::hex << address << "]";
        out << std::endl;
    }
}

void (*fatal_exception_handler) (void) = 0;

void set_fatal_exception_handler(void (*handler) (void))
{
    fatal_exception_handler = handler;
}

// initialize platform specific fatal error
// handling
void initialize_platform_runtime(); 
void terminate_handler();

void initialize_fatal_exception_handler()
{
    static bool initialized = false;
    if( initialized ) 
        return;

    initialize_platform_runtime();
    
    std::set_terminate(terminate_handler);
}

void terminate_handler()
{
    fatal_exit("terminate called");
}

void fatal_exit(const char* message, stacktrace_t& stacktrace)
{
    if( fatal_exception_handler ) {
	fatal_exception_handler();
    }
    std::cerr << get_exepath() << ": " << message << std::endl;
    if( stacktrace.size() > 0 ) 
        print_stacktrace(stacktrace, std::cerr);    
    std::cerr << "aborting" << std::endl;
    abort();
}

void fatal_exit(const char* message)
{
    stacktrace_t stacktrace;
    if( is_stacktrace_supported() ) {
        get_stacktrace(stacktrace);
    }
    
    fatal_exit(message, stacktrace);
}

//
// *interrupt* here is about high level user interrupt
// it's not about interrupt by ANY signal it's about
// interrupt caused by closing program or reqursting termination
// by Ctrl+C, Ctrl+Break, SIGINT, SIGTERN
//
void interrupt_exit(const char* message)
{
    std::cerr << get_exepath() << ": " << message << std::endl;
    exit(1);
}

interrupted_exception::interrupted_exception()
{}

const char* interrupted_exception::what() const throw()
{
	return "interrupted";
}

// TODO:    interrupt should be thread local somehow
         // ???
interrupt_policy current_interrupt_policy = IMMEDIATE_ABORT;
static volatile sigatomic_t interrupted = 0;

void interrupt()
{
    if( current_interrupt_policy == IMMEDIATE_ABORT ) {
        interrupt_exit("interrupted");
    } else {
        interrupted = 1;
    }
}

void test_interrupt()
{
    if( interrupted ) {
        interrupted = 0;
        throw interrupted_exception();
    }
}

void set_interrupt_policy(interrupt_policy p)
{
    current_interrupt_policy = p;
}

} // end of namespace tinfra

