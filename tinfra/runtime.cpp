#include <ostream>
#include <iomanip>

#include "tinfra/symbol.h"
#include "tinfra/runtime.h"

namespace tinfra {
    
void print_stacktrace(stacktrace_t const& st, std::ostream& out)
{    

    for( stacktrace_t::const_iterator i = st.begin(); i != st.end(); ++i ) {
        out << "0x" << std::setfill('0') << std::setw(sizeof(i->address)) << std::hex << i->address 
	    << "(" << i->symbol << ")" << std::endl;
    }
}

stackframe::~stackframe() 
{
    if( symbol ) ::free(symbol);
    if( file_name) ::free(file_name);
}

}
