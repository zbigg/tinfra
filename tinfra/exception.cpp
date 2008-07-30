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
