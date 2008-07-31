#include <ostream>
#include <iomanip>
#include <string.h>

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

char* safe_strdup(const char* a)
{
    if( a ) {
        return ::strdup(a);
    } else
        return 0;
}

stackframe::stackframe()
    : address(0), symbol(""), file_name(""), line_number(0)
{
}

/*
stackframe::stackframe(stackframe const& v)

    : address(v.address),
      symbol(safe_strdup(v.symbol)), 
      file_name(safe_strdup(v.file_name)),
      line_number(v.line_number)
{      
}

stackframe& stackframe::operator=(stackframe const& x)
{
    address = x.address;
    
    ::free(symbol);
    symbol = safe_strdup(x.symbol);
    
    ::free(file_name);
    file_name = safe_strdup(x.file_name);
    
    line_number = x.line_number;
}

stackframe::~stackframe() 
{
    if( symbol ) ::free(symbol);
    if( file_name) ::free(file_name);
}
*/
}
