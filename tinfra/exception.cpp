//
// Copyright (C) Zbigniew Zagorski <z.zagorski@gmail.com>,
// licensed to the public under the terms of the GNU GPL (>= 2)
// see the file COPYING for details
// I.e., do what you like, but keep copyright and there's NO WARRANTY.
//

#include <ostream>
#include <iomanip>

#include "tinfra/symbol.h"
#include "tinfra/exception.h"
#include "tinfra/runtime.h"

namespace tinfra {
    
//
// generic_exception implementation
//

generic_exception::generic_exception(std::string const& message): _message(message) {
    get_stacktrace(_stacktrace);
    /*
    std::cerr << "exception: " << message << std::endl;
    for( stacktrace_t::const_iterator i = _stacktrace.begin(); i != _stacktrace.end(); ++i ) {
	std::cerr << "    at 0x" << std::setfill('0') << std::setw(8) << std::hex << (long)i->address 
	          << "(" << i->symbol << ")" << std::endl;
    }
    */
}

}
