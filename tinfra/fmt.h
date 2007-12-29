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

class format_exception: public generic_exception {
public:
    format_exception(const std::string& message): generic_exception("format exception: " + message) {}
};
class simple_fmt {
public:
    simple_fmt(char const* format): fmt_(format), pos_(0) {}
    simple_fmt(std::string const& format): fmt_(format), pos_(0) {}
    
    template <typename T>
    simple_fmt& push(T const& value) {
        int cmd_pos = check_command();
        std::string tmp;
        to_string(value, tmp);
        output_.append(tmp);
        pos_= cmd_pos;        
        return *this;
    }
    
    template <typename T>
    simple_fmt& operator <<(T const& value) { return push(value); }
    
    template <typename T>
    simple_fmt& operator %(T const& value) { return push(value); }
    
    void reset();
    
    operator std::string() {
        return str();
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
    int check_command();
    void realize();

    std::string fmt_;
    int pos_;
    std::string output_;
};

///
/// The default formatter supported by tinfra.
///
typedef simple_fmt fmt;

} // end namespace tinfra

std::ostream& operator << (std::ostream& out, tinfra::simple_fmt& fmt);

#endif

