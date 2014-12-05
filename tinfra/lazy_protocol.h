//
// Copyright (c) 2009, Zbigniew Zagorski
// This software licensed under terms described in LICENSE.txt
//

#ifndef tinfra_lazy_protocol_h_included
#define tinfra_lazy_protocol_h_included

#include <stdexcept>
#include <string>

#include <tinfra/tstring.h>
//#include <tinfra/interruptible.h>

#include "interruptible.h"

namespace tinfra {

class lazy_protocol: public interruptible<int, tstring> {
public:
    lazy_protocol():
        waiter_count_(0)
    {
    }
protected:
    void wait_for_bytes(size_t count, step_method method);
    void wait_for_delimiter(tstring const& delim, step_method method);
private:
    step_method waiter_method_;
    size_t      waiter_count_;
    std::string waiter_delim_;

    int maybe_have_delim(tstring const& input);
    
    int maybe_have_enough_bytes(tstring const& input);
};

} // end namespace tinfra

#endif // tinfra_lazy_protocol_h_included

