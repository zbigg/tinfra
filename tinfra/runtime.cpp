//
// Copyright (C) Zbigniew Zagorski <z.zagorski@gmail.com>,
// licensed to the public under the terms of the GNU GPL (>= 2)
// see the file COPYING for details
// I.e., do what you like, but keep copyright and there's NO WARRANTY.
//

#include <ostream>
#include <iostream>
#include <iomanip>
#include <string.h>

#include "tinfra/fmt.h"
#include "tinfra/runtime.h"

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
    std::cerr << get_exepath() << ": message" << std::endl;
    if( stacktrace.size() > 0 ) 
        print_stacktrace(stacktrace, std::cerr);
    if( fatal_exception_handler ) {
	fatal_exception_handler();
    }
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

} // end of namespace tinfra

