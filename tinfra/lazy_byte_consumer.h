//
// Copyright (C) 2008 Zbigniew Zagorski <z.zagorski@gmail.com>,
// licensed to the public under the terms of the GNU GPL (>= 2)
// see the file COPYING for details
// I.e., do what you like, but keep copyright and there's NO WARRANTY.
//

#ifndef tinfra_lazy_byte_consumer_h_included__
#define tinfra_lazy_byte_consumer_h_included__

#include <stdexcept>
#include <string>

#include <tinfra/tstring.h>
#include "tinfra/trace.h"
//#include <tinfra/interruptible.h>

#include "interruptible.h"

namespace tinfra {

template <typename IMPL>
class lazy_byte_consumer: public interruptible<IMPL, int, tstring> {
    typedef typename interruptible<IMPL,int,tstring>::step_method step_method;
public:
    lazy_byte_consumer(IMPL& i, step_method s = 0):
    	interruptible<IMPL,int,tstring>(i,s),
        waiting_for_(NOTHING)
    {}
    

    void wait_for_anything(step_method method) {
        waiting_for_ = ANYTHING;
        next(method);
    }
    
    void wait_for_bytes(size_t count, step_method method);
    
    void wait_for_delimiter(tstring const& delim, step_method method);
protected:
    int call(step_method m, tstring const& input);
private:
    size_t      waiter_count_;
    std::string waiter_delim_;
    enum wait_type {
        NOTHING,
        ANYTHING,
        COUNT,
        DELIMITER,
    }           waiting_for_;
    int maybe_have_delim(tstring const& input);
    
    int maybe_have_enough_bytes(tstring const& input);
};

template <typename IMPL>
int lazy_byte_consumer<IMPL>::call(step_method m, tstring const& input)
{
    switch( waiting_for_ ) {
    case NOTHING:
        TINFRA_TRACE_VAR(input);
        throw std::logic_error("lazy_byte_consumer doesn't expect input now");
    case ANYTHING:
        break;
        
    case COUNT:
        if( input.size() < waiter_count_ ) {
            this->again();
            return 0;
        }
        break;
    case DELIMITER:
        {
            size_t pos =  input.find(waiter_delim_);
            if( pos == tstring::npos ) {
                this->again();
                return 0;
            }
        }
        break;
    }
    waiting_for_ = NOTHING;
    //TINFRA_TRACE_VAR(input);
    return interruptible<IMPL, int, tstring>::call(m, input);
}

template<typename IMPL>
void lazy_byte_consumer<IMPL>::wait_for_bytes(size_t count, step_method method)
{
    waiting_for_ = COUNT;
    waiter_count_ = count;    
    next(method);
}

template<typename IMPL>
void lazy_byte_consumer<IMPL>::wait_for_delimiter(tstring const& delim, step_method method)
{
    waiting_for_ = DELIMITER;
    waiter_delim_.assign(delim.data(), delim.size());
    next(method);
}

} // end namespace tinfra

#endif // tinfra_lazy_byte_consumer_h_included__

// jedit: :tabSize=8:indentSize=4:noTabs=true:mode=c++

