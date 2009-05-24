//
// Copyright (C) 2009 Zbigniew Zagorski <z.zagorski@gmail.com>,
// licensed to the public under the terms of the GNU GPL (>= 2)
// see the file COPYING for details
// I.e., do what you like, but keep copyright and there's NO WARRANTY.
//

#ifndef __tinfra_shared_ptr_h__
#define __tinfra_shared_ptr_h__

#include <tinfra/trace.h>

namespace tinfra {

struct reference_count {
    long* ref_;    
        
    explicit reference_count(long* x) 
        : ref_(x) 
    {}
    
    reference_count(reference_count const& other)
        : ref_(other.ref_)
    {
    }
    
    reference_count& operator=(reference_count const& other)
    {
        if( this != &other ) {
            ref_ = other.ref_;
        }
        return *this;
    }
    
    void attach() {
        if( ref_ ) {
            *ref_ += 1;
        }
    }
    
    bool detach();
    
    bool unique() const { return *ref_ == 1; } // never throws
    long use_count() const { return *ref_; } // never throws
};

template <typename T>
class shared_ptr {
public:
    typedef T element_type;
    
    shared_ptr(): ptr_(0), refcount_(0) {}
        
    template <typename Y>
    explicit shared_ptr(Y* p);
        
    shared_ptr(shared_ptr const& p);    
    template <typename Y> explicit shared_ptr(shared_ptr<Y> const& p);
    
    shared_ptr<T>& operator=(shared_ptr const& p);    
    template <typename Y> shared_ptr<T>& operator=(shared_ptr<Y> const& p);
    
    T & operator*() const  { return *ptr_; } // never throws
    T * operator->() const { return ptr_; } // never throws
    T * get() const        { return ptr_; } // never throws

    ~shared_ptr() {
        release();
    }
public:
    reference_count const& _tinfra_priv_refcount_() const {
        return refcount_;
    }
private:
    void release() {
        if( refcount_.detach() ) {
            TINFRA_TRACE_MSG("tinfra::shared_ptr: releasing");
            delete ptr_;
            ptr_ = 0;
        }
    }
    
    T*    ptr_;
    reference_count refcount_;
};

//
// implementation
//

template <typename T>
template <typename Y>
shared_ptr<T>::shared_ptr(Y* p)
    : ptr_(p), 
      refcount_( new long(1))
{ 
    TINFRA_TRACE_MSG("tinfra::shared_ptr: new instance");
}
    
template <typename T>
shared_ptr<T>::shared_ptr(shared_ptr const& p)
    : ptr_(p.ptr_),
      refcount_(p.refcount_)
{
    TINFRA_TRACE_MSG("tinfra::shared_ptr: copy_const");
    refcount_.attach();
}

template <typename T>
template <typename Y>
shared_ptr<T>::shared_ptr(shared_ptr<Y> const& p)
    : ptr_(p.get()),
      refcount_(p._tinfra_priv_refcount_())
{
    TINFRA_TRACE_MSG("tinfra::shared_ptr: copy_const template");
    refcount_.attach();
}

template <typename T>
shared_ptr<T>& shared_ptr<T>::operator=(shared_ptr<T> const& p)
{
    TINFRA_TRACE_MSG("tinfra::shared_ptr: operator=");
    if( this == &p )
        return *this;
    
    release();
    
    ptr_ = p.ptr_;
    refcount_ = p.refcount_;
    
    refcount_.attach();
    
    return *this;
}
template <typename T>
template <typename Y>
shared_ptr<T>& shared_ptr<T>::operator=(shared_ptr<Y> const& p)
{
    TINFRA_TRACE_MSG("tinfra::shared_ptr: operator= template");
    if( this == &p )
        return *this;
    
    release();
    
    ptr_ = p.get();
    refcount_ = p._tinfra_priv_refcount_;
    
    refcount_.attach();
    
    return *this;
}

inline bool reference_count::detach() {
    if( ref_ == 0 )
        return false;
    *ref_ -= 1;
    if( use_count() == 0 ) {
        delete ref_;
        ref_ = 0;
        return true;
    }
    ref_ = 0;
    return false;
}

} // end namespace tinfra

#endif // __tinfra_shared_ptr_h__

