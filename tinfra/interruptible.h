//
// Copyright (C) 2008 Zbigniew Zagorski <z.zagorski@gmail.com>,
// licensed to the public under the terms of the GNU GPL (>= 2)
// see the file COPYING for details
// I.e., do what you like, but keep copyright and there's NO WARRANTY.
//

#ifndef __tinfra_interruptible_h__
#define __tinfra_interruptible_h__

#include <stdexcept>

namespace tinfra {
    
template<typename IMPL, typename R, typename T>
class interruptible {
public:
    typedef R (IMPL::*step_method)(T const& e);
    
    interruptible(IMPL& impl, step_method m = 0):
    	implementation_(impl),
        next_method_(m),
        again_(false),
        finished_(false)
    {
    }
    
    R process(T const& input)
    {
        if( finished_ ) {
            throw std::logic_error("already finished");
        }
        step_method current_method = next_method_;
        next_method_ = 0;
        again_ = false;
        
        R output = call(current_method, input);
        
        if( again_ ) {
            next_method_ = current_method;
        } else if( !next_method_ ) {
            finished_  = true;
        }
        return output;
    }
    
    bool is_finished() const {
        return finished_;
    }
protected:
    
    R call(step_method m, T const& a)
    {
        return (implementation_.*m)(a);
    }
    void next(step_method s) {
        next_method_ = s;
    }
        
    // deprecated
    step_method make_step_method(R (IMPL::*m)(T const&)) {
        return m;
    }
    void again() {
        again_ = true;
    }
    
    void finish() {
        // nothing needed
    }
private:
    IMPL&       implementation_;
    step_method next_method_;
    bool        again_;
    bool        finished_;
};

} // end namespace tinfra

#endif // __tinfra_interruptible_h__
