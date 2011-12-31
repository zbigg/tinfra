//
// Copyright (c) 2009, Zbigniew Zagorski
// This software licensed under terms described in LICENSE.txt
//

#ifndef tinfra_fmt_h_included
#define tinfra_fmt_h_included

#include "tinfra/platform.h"

#include "tinfra/tstring.h"

#include <string>  // for std::string
#include <iosfwd>  // for std::ostream
#include <sstream> // for std::ostringstream 
#include <stdexcept> // for std::logic_error
namespace tinfra {
    
/// simple_fmt
/// formats everyting as string using tinfra::to_string()
/// Use case:
///     fmt("Hello %s. Nice to %s you. Count %i") << "zbyszek" << "opryszek" << 2;
///     fmt("Hello %s. Nice to %s you. Count %i") % "zbyszek" % "opryszek" % 2;
///

class format_exception: public std::logic_error {
public:
    format_exception(const std::string& message): std::logic_error("format exception: " + message) {}
};

class simple_fmt {
public:
    simple_fmt(tstring const& format): fmt_(format), pos_(0) {}
    
    template <typename T>
    simple_fmt& push(T const& value) {
        std::ostringstream formatter(&this->format_buf_);
        std::size_t cmd_pos = check_command(formatter);
        formatter << value;
        pos_= cmd_pos;
        return *this;
    }
    
    template <typename T>
    simple_fmt& operator <<(T const& value) { return push(value); }
    
    template <typename T>
    simple_fmt& operator %(T const& value) { return push(value); }
    
    void reset();
    
    
    operator std::string () {
    	    return str();
    }
    /*
    operator std::string const&() {
        return str();
    }
    */
    
    operator tinfra::tstring() {
        realize();
        return tinfra::tstring(output_);
    }
    
    operator const char*() {
        return c_str();
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

    std::size_t check_command(std::ostream&);
    void realize();

    std::string fmt_;
    std::size_t pos_;
    std::string output_;
    std::stringbuf format_buf_;
};

inline simple_fmt fmt(tstring const& fmt) { return simple_fmt(fmt); }

template <typename T1>
simple_fmt fmt(tstring const& fmt, T1 const& v1) { return simple_fmt(fmt) % v1; }

template <typename T1, typename T2>
simple_fmt fmt(tstring const& fmt, T1 const& v1, T2 const& v2) { return simple_fmt(fmt) % v1 % v2; } 
///
/// The default formatter supported by tinfra.
///
//typedef simple_fmt fmt;

std::ostream& operator << (std::ostream& out, simple_fmt& fmt);

} // end namespace tinfra


#endif

