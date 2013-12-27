//
// Copyright (c) 2011, Zbigniew Zagorski
// This software licensed under terms described in LICENSE.txt
//

#include "assert.h"
#include "runtime.h" // for fatal_exit

#include <cstdlib> // for std::getenv

namespace tinfra {
    
void report_assertion_failure(tinfra::source_location const& location, const char* message)
{
    const std::string file_name = location.filename ? location.filename : "<unknown>";
    const std::string function_name = location.name ? location.name : "<unknown>";
    const std::string exit_message = tinfra::fmt("%s at %s(%s:%i)") 
        % message % 
        function_name % file_name % location.line;
    
    if( std::getenv("TINFRA_STRICT_ASSERT") != 0 ) {
        tinfra::fatal_exit(exit_message.c_str());
    } else {
        throw std::logic_error(exit_message);
    }
}

void assert_failed(tinfra::source_location const& location, const char* expression)
{

    const std::string message = tinfra::fmt("assertion '%s' failed") % expression; 
    report_assertion_failure(location, message.c_str());
}

}

// jedit: :tabSize=8:indentSize=4:noTabs=true:mode=c++:

