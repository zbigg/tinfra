//
// Copyright (c) 2009, Zbigniew Zagorski
// This software licensed under terms described in LICENSE.txt
//

#ifndef tinfra_exception_h_included
#define tinfra_exception_h_included

#include "tinfra/runtime.h"

#include <stdexcept>
#include <string>

namespace tinfra {

class generic_exception: public std::exception {
public:
    generic_exception(std::string const& message);
    virtual ~generic_exception() throw() {}
    
    virtual const char* what() const throw() { return _message.c_str(); }

    stacktrace_t const& stacktrace() const { return _stacktrace; }

protected:
    std::string   _message;
    stacktrace_t  _stacktrace;
};

} // end of namespace tinfra

#endif
