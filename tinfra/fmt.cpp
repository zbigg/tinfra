#include "tinfra/fmt.h"
#include "tinfra/exception.h"
#include <string>
#include <cctype>
namespace tinfra {

struct fmt_command {
    char command;
};
    
template <typename T>
int process_fmt(std::string fmt, int start, fmt_command& result, T& output)
{
    std::string::const_iterator istart = fmt.begin() + start;
    std::string::const_iterator c = istart;
    
    const std::string::const_iterator end = fmt.end();
    
    //std::cout << "pf: " << fmt << " at " << start << std::endl;
    while( c != end ) 
    {
        
        if( *c == '%' ) {
            if( c+1 == end ) throw format_exception("bad format: '%' at the end");
            output.append(istart, c);            
            ++c;
            if( *c == '%' ) {
                ++c;
                istart = c;
                output.append("%");
                continue; // %% sequence is an escape
            } else if( std::isalpha(*c) ) {
                result.command = *c;
                ++c;
                //std::cout << "pf: " << (c - fmt.begin()) << std::endl;
                return c - fmt.begin();
            } else {
                throw format_exception("simple_fmt: bad format command");
            }
        } else {
            ++c;
        }
    }
    //std::cout << "pf: " << -1 << std::endl;
    output.append(istart, end);
    return -1;
}

///
/// simple_fmt
///
void simple_fmt::reset() {
    output_.clear();
    pos_ = 0;
}
int simple_fmt::check_command()
{
    fmt_command cmd;
    int cmd_pos = process_fmt(fmt_, pos_, cmd, output_);
    if( cmd_pos == -1 ) throw format_exception("simple_fmt: too many actual arguments");        
    switch( cmd.command ) {
    case 's':
    case 'i':
    case 'd':
        break;
    default:
        throw format_exception(simple_fmt("simple_fmt: bad format command %s") % cmd.command);
    }
    return cmd_pos;
}
void simple_fmt::realize()
{
    fmt_command cmd;        
    if( process_fmt(fmt_, pos_, cmd, output_) != -1 ) 
        throw format_exception("simple_fmt: not all arguments realized");        
    pos_ = fmt_.size();
}

} // end namespace tinfra

std::ostream& operator << (std::ostream& out, tinfra::simple_fmt& fmt)
{
    return out << fmt.str();
}
