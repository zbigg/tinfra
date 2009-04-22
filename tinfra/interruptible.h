//
// Copyright (C) 2008 Zbigniew Zagorski <z.zagorski@gmail.com>,
// licensed to the public under the terms of the GNU GPL (>= 2)
// see the file COPYING for details
// I.e., do what you like, but keep copyright and there's NO WARRANTY.
//

#ifndef tinfra_interruptible_h_included__
#define tinfra_interruptible_h_included__

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
    
    virtual ~interruptible() {}
    
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
    void next(step_method s) {
        next_method_ = s;
    }
    void again() {
        again_ = true;
    }
    
    void finish() {
        // nothing needed
    }
protected:
    
    virtual R call(step_method m, T const& a)
    {
        return (implementation_.*m)(a);
    }

private:
    IMPL&       implementation_;
    step_method next_method_;
    bool        again_;
    bool        finished_;
};

} // end namespace tinfra

#endif // tinfra_interruptible_h_included__

// jedit: :tabSize=8:indentSize=4:noTabs=true:mode=c++

