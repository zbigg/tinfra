//
// Copyright (C) Zbigniew Zagorski <z.zagorski@gmail.com>,
// licensed to the public under the terms of the GNU GPL (>= 2)
// see the file COPYING for details
// I.e., do what you like, but keep copyright and there's NO WARRANTY.
//

#include "tinfra/fmt.h"
#include "tinfra/exception.h"
#include <string>
#include <cctype>
#include <iterator>
#include <algorithm>

namespace tinfra {

struct fmt_command {
    char command;
};
    
template <typename T>
size_t process_fmt(std::string fmt, size_t start, fmt_command& result, T& output)
{
    std::string::const_iterator istart = fmt.begin() + start;
    std::string::const_iterator c = istart;
    
    const std::string::const_iterator end = fmt.end();
    
    //std::cout << "pf: " << fmt << " at " << start << std::endl;
    while( c != end ) 
    {
        
        if( *c == '%' ) {
            if( c+1 == end ) 
                // THROW_ANALYSIS: assertion, programmer error
                // THROW_ANALYSIS: (format string are static string or should be checked prior to execution)
                throw format_exception("bad format: '%' at the end");
            output.append(istart, c);            
            ++c;
            if( *c == '%' ) {
                ++c;
                istart = c;
                output.append(1,'%');
                continue; // %% sequence is an escape
            } else if( std::isalpha(*c) ) {
                result.command = *c;
                ++c;
                //std::cout << "pf: " << (c - fmt.begin()) << std::endl;
                return c - fmt.begin();
            } else {
                // THROW_ANALYSIS: assertion, programmer error
                // THROW_ANALYSIS: (format string are static string or should be checked prior to execution)
                throw format_exception("simple_fmt: bad format command");
            }
        } else {
            ++c;
        }
    }
    //std::cout << "pf: " << -1 << std::endl;
    output.append(istart, end);
    return (size_t)-1;
}

class ostream_string_output {
    std::ostream& out;
    
public:
    ostream_string_output(std::ostream& out): out(out) {}
        
    void append(int n, char c)
    {
        for( int i = 0; i < n; ++i ) {
            out << c;
        }
    }
    void append(std::string::const_iterator begin, std::string::const_iterator end)
    {
        /*if( end != begin ) {
            const char* pb = & (*begin);
            size_t size = distance(begin,end);
            out.write(pb, size);
        }*/
        std::copy(begin, end, std::ostream_iterator<char>(out));
    }
};

///
/// simple_fmt
///
void simple_fmt::reset() {
    output_.clear();
    pos_ = 0;
}
size_t simple_fmt::check_command()
{
    fmt_command cmd;
    ostream_string_output output(formatter_);
    
    size_t cmd_pos = process_fmt(fmt_, pos_, cmd, output);
    // THROW_ANALYSIS: assertion, programmer error
    // THROW_ANALYSIS: (format string are static string or should be checked prior to execution)
    if( cmd_pos == (size_t)-1 ) throw format_exception("simple_fmt: too many actual arguments");
    switch( cmd.command ) {
    case 's':
    case 'i':
    case 'd':
        break;
    default:
        // THROW_ANALYSIS: assertion, programmer error
        // THROW_ANALYSIS: (format string are static string or should be checked prior to execution)
        throw format_exception(simple_fmt("simple_fmt: bad format command %s") % cmd.command);
    }
    return cmd_pos;
}
void simple_fmt::realize()
{
    fmt_command cmd;
    ostream_string_output output(formatter_);
    if( process_fmt(fmt_, pos_, cmd, output) != (size_t)-1 ) 
        // THROW_ANALYSIS: assertion, programmer error
        throw format_exception("simple_fmt: not all arguments realized");
    output_ = formatter_.str();
    pos_ = fmt_.size();
}

} // end namespace tinfra

std::ostream& operator << (std::ostream& out, tinfra::simple_fmt& fmt)
{
    return out << fmt.str();
}
