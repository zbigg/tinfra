#ifndef __tinfra_fmt_h__
#define __tinfra_fmt_h__

#include "tinfra/exception.h"
#include "tinfra/tinfra_lex.h"
#include <string>
#include <ostream>

namespace tinfra {
    
/// simple_fmt
/// formats everyting as string using tinfra::to_string()
/// Use case:
///     fmt("Hello %s. Nice to %s you. Count %i") << "zbyszek" << "opryszek" << 2;
///     fmt("Hello %s. Nice to %s you. Count %i") % "zbyszek" % "opryszek" % 2;
///

namespace format {

struct fmt_command {
    char command;
};
    
template <typename T>
int process_fmt(std::string fmt, int start, fmt_command& result, T& output)
{
    std::string::const_iterator istart = fmt.begin() + start;
    std::string::const_iterator c = istart;
    
    const std::string::const_iterator end = fmt.end();
    
    while( c != end ) 
    {
        if( *c == '%' ) {
            if( c+1 == end ) throw generic_exception("simple_fmt: bad format: '%' at the end");
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
                return c - fmt.begin();
            } else {
                throw generic_exception("simple_fmt: bad format command");
            }
        } else {
            ++c;
        }
    }
    output.append(istart, end);
    return -1;
}

} // end namespace tinfra::format

class simple_fmt {
public:
    simple_fmt(char const* format): fmt_(format), pos_(0) {}
    simple_fmt(std::string const& format): fmt_(format), pos_(0) {}
    
    template <typename T>
    void push(T const& value) {
        format::fmt_command cmd;
        int cmd_pos = format::process_fmt(fmt_, pos_, cmd, output_);
        if( cmd_pos == -1 ) throw generic_exception("simple_fmt: too many arguments");        
        std::string tmp;
        to_string(value, tmp);
        output_.append(tmp);
    }
    
    template <typename T>
    simple_fmt& operator <<(T const& value) { push(value); return *this; }
    
    template <typename T>
    simple_fmt& operator %(T const& value) { push(value); return *this; }
    
    void reset() {
        output_.clear();
        pos_ = 0;
    }
    
    void realize()
    {
        format::fmt_command cmd;        
        if( format::process_fmt(fmt_, pos_, cmd, output_) != -1 ) 
            throw generic_exception("simple_fmt: not all arguments realized");        
        pos_ = fmt_.size();
    }
    
    std::string const& str() {
        realize();
        return output_;
    }
    char const* c_str() {
        realize();
        return output_.c_str();
    }
    
private:    
    std::string fmt_;
    int pos_;
    std::string output_;
};


} // end namespace tinfra

std::ostream& operator << (std::ostream& out, tinfra::simple_fmt& fmt);

#endif

