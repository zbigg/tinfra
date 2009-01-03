//
// Copyright (C) 2008 Zbigniew Zagorski <z.zagorski@gmail.com>,
// licensed to the public under the terms of the GNU GPL (>= 2)
// see the file COPYING for details
// I.e., do what you like, but keep copyright and there's NO WARRANTY.
//

#ifndef __tinfra_lazy_byte_consumer_h__
#define __tinfra_lazy_byte_consumer_h__

#include <stdexcept>
#include <string>

#include <tinfra/tstring.h>
//#include <tinfra/interruptible.h>

#include "interruptible.h"

namespace tinfra {

template <typename IMPL>
class lazy_byte_consumer: public interruptible<IMPL, int, tstring> {
    typedef typename interruptible<IMPL,int,tstring>::step_method step_method;
public:
    lazy_byte_consumer(IMPL& i, step_method s = 0):
    	interruptible<IMPL,int,tstring>(i,s)
    {}
    
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

template<typename IMPL>
void lazy_byte_consumer<IMPL>::wait_for_bytes(size_t count, step_method method)
{
    waiter_count_ = count;
    
    waiter_method_ = method;        
    next(make_step_method(&lazy_byte_consumer::maybe_have_enough_bytes));
}

template<typename IMPL>
void lazy_byte_consumer<IMPL>::wait_for_delimiter(tstring const& delim, step_method method)
{
    waiter_delim_.assign(delim.data(), delim.size());
    waiter_method_ = method;
    next(make_step_method(&lazy_byte_consumer::maybe_have_delim));
}

template<typename IMPL>
int lazy_byte_consumer<IMPL>::maybe_have_delim(tstring const& input)
{
    size_t pos =  input.find_first_of(waiter_delim_.data(), waiter_delim_.size());
    if( pos == tstring::npos ) {
        this->again();
        return 0;
    }
    return call(waiter_method_, tstring(input.data(), pos+1));
}

template<typename IMPL>
int lazy_byte_consumer<IMPL>::maybe_have_enough_bytes(tstring const& input)
{
    if( input.size() < waiter_count_ ) {
        this->again();
        return 0;
    }
    
    return call(waiter_method_, tstring(input.data(), waiter_count_));
}
} // end namespace tinfra

#endif // __tinfra__lazy_byte_consumer_h__

