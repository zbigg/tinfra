//
// Copyright (C) 2008 Zbigniew Zagorski <z.zagorski@gmail.com>,
// licensed to the public under the terms of the GNU GPL (>= 2)
// see the file COPYING for details
// I.e., do what you like, but keep copyright and there's NO WARRANTY.
//

#include "lazy_protocol.h"

namespace tinfra {

/*
private:
step_method waiter_method_;
size_t      waiter_count_;
string      waiter_delim_;
*/
    
void lazy_protocol::wait_for_bytes(size_t count, step_method method)
{
    waiter_count_ = count;
    
    waiter_method_ = method;        
    next(make_step_method(&lazy_protocol::maybe_have_enough_bytes));
}

void lazy_protocol::wait_for_delimiter(tstring const& delim, step_method method)
{
    waiter_delim_.assign(delim.data(), delim.size());
    waiter_method_ = method;
    next(make_step_method(&lazy_protocol::maybe_have_delim));
}


int lazy_protocol::maybe_have_delim(tstring const& input)
{
    size_t pos =  input.find_first_of(waiter_delim_.data(), waiter_delim_.size());
    if( pos == tstring::npos ) {
        again();
        return 0;
    }
    return call(waiter_method_, tstring(input.data(), pos));
}

int lazy_protocol::maybe_have_enough_bytes(tstring const& input)
{
    if( input.size() < waiter_count_ ) {
        again();
        return 0;
    }
    
    return call(waiter_method_, tstring(input.data(), waiter_count_));
}

} // end namespace tinfra
