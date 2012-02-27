//
// Copyright (c) 2009, Zbigniew Zagorski
// This software licensed under terms described in LICENSE.txt
//

#ifndef tinfra_shared_ptr_h_included_
#define tinfra_shared_ptr_h_included_

#include <tinfra/trace.h>

//#define TINFRA_HAVE_GCC_ATOMIC_BUILTINS
//#define TINFRA_SHARED_PTR_USE_BOOST

#ifdef TINFRA_SHARED_PTR_USE_BOOST
#include <boost/shared_ptr.hpp>
#elif defined(TINFRA_HAVE_GCC_ATOMIC_BUILTINS)
// use GCC builtin atomic ops
#else
#include "tinfra/mutex.h"
#include "tinfra/guard.h"
#endif

#include <memory>

namespace tinfra {

extern tinfra::module_tracer shared_ptr_tracer;

#ifdef TINFRA_SHARED_PTR_USE_BOOST

using boost::shared_ptr;

#else // no TINFRA_SHARED_PTR_USE_BOOST

namespace shared_ptr_atomic {

#ifdef TINFRA_HAVE_GCC_ATOMIC_BUILTINS
    typedef long atomic_long;
    
    inline atomic_long atomic_increment(atomic_long* a) {
        return __sync_add_and_fetch(a,1);
    }
    
    inline atomic_long atomic_decrement(atomic_long* a) {
        return __sync_sub_and_fetch(a,1);
    }
#else
    class atomic_long {
        long value_;
        tinfra::mutex mtx_;
    public:
        atomic_long(long a):
            value_(a)
        {
        }
        operator long() {
            tinfra::guard g(mtx_);
            return value_;
        }
        
        long inc() {
            tinfra::guard g(mtx_);
            return (value_ += 1);
        }
        
        long dec() {
            tinfra::guard g(mtx_);
            return (value_ -= 1);
        }
    private:
        atomic_long(atomic_long const&);
        atomic_long& operator=(atomic_long const&);
    };
    
    inline long atomic_increment(atomic_long* a) {
        return a->inc();
    }
    
    inline long atomic_decrement(atomic_long* a) {
        return a->dec();
    }
#endif
}

struct reference_count {
    shared_ptr_atomic::atomic_long* ref_;    
        
    explicit reference_count(shared_ptr_atomic::atomic_long* x) 
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
    
    void attach();
    bool detach();
    
    //bool unique() const { return *ref_ == 1; } // never throws
    //long use_count() const { return *ref_; } // never throws
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
    template <typename Y> explicit shared_ptr(std::auto_ptr<Y>& p);
    
    shared_ptr<T>& operator=(shared_ptr const& p);
    template <typename Y> shared_ptr<T>& operator=(shared_ptr<Y> const& p);
    
    T & operator*() const  { return *ptr_; } // never throws
    T * operator->() const { return ptr_; } // never throws
    T * get() const        { return ptr_; } // never throws

    ~shared_ptr() {
        release();
    }
    void swap(shared_ptr<T>& other)
    {
        //TINFRA_TRACE(shared_ptr_tracer, "tinfra::shared_ptr: swap");
        using std::swap;
        swap(ptr_, other.ptr_);
        swap(refcount_.ref_, other.refcount_.ref_);
    }
    
    void reset(shared_ptr<T> const& other) {
        (*this) = other;
    }
    
    void reset() {
        release();      
    }
    
public:
    reference_count const& _tinfra_priv_refcount_() const {
        return refcount_;
    }
private:
    void release() {
        if( refcount_.detach() ) {
            //TINFRA_TRACE(shared_ptr_tracer, "tinfra::shared_ptr: releasing");
            //TINFRA_TRACE_VAR(shared_ptr_tracer, this->ptr_);
            delete ptr_;
            ptr_ = 0;
        }
    }
    
    
    
    T*    ptr_;
    reference_count refcount_;
};

} // end namespace tinfra

namespace std {
template <typename T>
void swap(tinfra::shared_ptr<T>& a, tinfra::shared_ptr<T>& b)
{
    a.swap(b);
}
} // end namespace std

namespace tinfra {

//
// implementation
//

template <typename T>
template <typename Y>
shared_ptr<T>::shared_ptr(Y* p)
    : ptr_(p), 
      refcount_( new shared_ptr_atomic::atomic_long(1))
{ 
    //TINFRA_TRACE(shared_ptr_tracer, "tinfra::shared_ptr: new instance");
}
    
template <typename T>
shared_ptr<T>::shared_ptr(shared_ptr const& p)
    : ptr_(p.ptr_),
      refcount_(p.refcount_)
{
    //TINFRA_TRACE(shared_ptr_tracer, "tinfra::shared_ptr: copy_const");
    refcount_.attach();
}

template <typename T>
template <typename Y>
shared_ptr<T>::shared_ptr(shared_ptr<Y> const& p)
    : ptr_(p.get()),
      refcount_(p._tinfra_priv_refcount_())
{
    //TINFRA_TRACE(shared_ptr_tracer, "tinfra::shared_ptr: copy_const template");
    refcount_.attach();
}
template <typename T>
template <typename Y> 
shared_ptr<T>::shared_ptr(std::auto_ptr<Y>& p)
    : ptr_(p.get()),
      refcount_( new shared_ptr_atomic::atomic_long(1))
{
    p.release();
}

template <typename T>
shared_ptr<T>& shared_ptr<T>::operator=(shared_ptr<T> const& p)
{
    //TINFRA_TRACE(shared_ptr_tracer, "tinfra::shared_ptr: operator=");
    //TINFRA_TRACE_VAR(shared_ptr_tracer, this->ptr_);
    int rc = refcount_.ref_ ? (long)(*refcount_.ref_) : -1;
    //TINFRA_TRACE_VAR(shared_ptr_tracer, rc);
    if( this == &p )
        return *this;
    
    // protect this value so it's deleted AFTER p
    // is correctly referenced
    // [ http://www.open-std.org/jtc1/sc22/wg21/docs/papers/2003/n1450.html, section D, example 3 ]
    shared_ptr<T> tmp; 
    swap(tmp);
    
    ptr_ = p.ptr_;
    refcount_ = p.refcount_;
    
    refcount_.attach();
    //TINFRA_TRACE(shared_ptr_tracer, "tinfra::shared_ptr: operator=, attached:");
    //TINFRA_TRACE_VAR(shared_ptr_tracer, this->ptr_);
    
    return *this;
}
template <typename T>
template <typename Y>
shared_ptr<T>& shared_ptr<T>::operator=(shared_ptr<Y> const& p)
{
    //TINFRA_TRACE(shared_ptr_tracer, "tinfra::shared_ptr: operator= template");
    if( this == &p )
        return *this;
    
    shared_ptr<T> tmp; 
    swap(tmp);
    
    ptr_ = p.get();
    refcount_ = p._tinfra_priv_refcount_;
    refcount_.attach();
    
    return *this;
}

inline void reference_count::attach() {
    if( ref_ ) {
        shared_ptr_atomic::atomic_increment(ref_);
        //TINFRA_TRACE_VAR(shared_ptr_tracer, ref_);
    }
}

inline bool reference_count::detach() {
    if( ref_ == 0 )
        return false;
    //TINFRA_TRACE_VAR(shared_ptr_tracer, ref_);
    int current_use_count = shared_ptr_atomic::atomic_decrement(ref_);
    if( current_use_count == 0 ) {
        delete ref_;
        ref_ = 0;
        return true;
    }
    ref_ = 0;
    return false;
}

#endif // not TINFRA_SHARED_PTR_USE_BOOST

} // end namespace tinfra

#endif // tinfra_shared_ptr_h_included_

