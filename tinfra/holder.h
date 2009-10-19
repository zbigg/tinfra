//
// Copyright (C) Zbigniew Zagorski <z.zagorski@gmail.com>,
// licensed to the public under the terms of the GNU GPL (>= 2)
// see the file COPYING for details
// I.e., do what you like, but keep copyright and there's NO WARRANTY.
//

#ifndef tinfra_holder_h_included
#define tinfra_holder_h_included

namespace tinfra {

template <typename T>
class holder {
    T value_;
public:    
    holder(T const& value): value_(value) {}
    ~holder() { release(); }
    
    void release();
        
    operator T& () { return value_; }
    operator T const& () const { return value_; }
};

}

#endif
