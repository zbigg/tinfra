//
// Copyright (c) 2009, Zbigniew Zagorski
// This software licensed under terms described in LICENSE.txt
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
