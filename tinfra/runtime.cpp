#include <ostream>
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

}
