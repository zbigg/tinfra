//
// Copyright (C) 2008 Zbigniew Zagorski <z.zagorski@gmail.com>,
// licensed to the public under the terms of the GNU GPL (>= 2)
// see the file COPYING for details
// I.e., do what you like, but keep copyright and there's NO WARRANTY.
//

#ifndef __tinfra__lazy_protocol_h__
#define __tinfra__lazy_protocol_h__

#include <stdexcept>
#include <string>

#include <tinfra/tstring.h>
//#include <tinfra/interruptible.h>

#include "interruptible.h"

namespace tinfra {

class lazy_protocol: public interruptible<int, tstring> {
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

#endif // __tinfra__lazy_protocol_h__

