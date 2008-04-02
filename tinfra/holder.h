#ifndef __tinfra__holder_h__
#define __tinfra__holder_h__

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
