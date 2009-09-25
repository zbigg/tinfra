#ifndef tinfra_any_h_included
#define tinfra_any_h_included

#include "shared_ptr.h"
#include <cassert>
#include <typeinfo>

namespace tinfra {

/// container of any object
///
/// Implementation contains object of any type and
/// is responsible for its lifecycle.
///
/// Provides opaque access to this object.
///
/// C++ equivalent of void*
struct any_container_base {
    virtual ~any_container_base();

    virtual void* get() = 0;
    virtual std::type_info const& type() const = 0;
};

template<typename T>
class storing_any_container: public any_container_base {
    T value;
public:
    storing_any_container(T const& v): value(v) {}
    
    virtual void* get()                        { return &value; }
    virtual std::type_info const& type() const { return typeid(T); }
    
    T&            typed_get() { return value; }
};


template<typename T>
class ref_any_container: public any_container_base {
    T& value_ref;
public:
    ref_any_container(T& v): value_ref(v) {}
    
    virtual void* get()                        { return &value_ref; }
    virtual std::type_info const& type() const { return typeid(T); }
    
    T&            typed_get() { return value_ref; }
};

template<typename T>
class auto_ptr_any_container: public any_container_base {
    std::auto_ptr<T> value_holder;
public:
    auto_ptr_any_container(T* v): value_holder(v) {}
    
    virtual void* get()                        { return value_holder.get(); }
    virtual std::type_info const& type() const { return typeid(T); }
    
    T&            typed_get() { return value_holder.get(); }
};

class any {
public:
    template <typename T>
    static any from_copy(T const& v) {
        return any(new storing_any_container<T>(v));
    }
    
    template <typename T>
    static any by_ref(T& v) {
        return any(new ref_any_container<T>(v));
    }
    
    template <typename T>
    static any from_new(T* v) {
        return any(new auto_ptr_any_container<T>(v));
    }
    
    any(any_container_base* ptr);
    
    void* get_raw();
    
    const void* get_raw() const;
    
    std::type_info const& type() const;
    
    template <typename T>
    T& get();
    
    template <typename T>
    T const& get() const;
private:
    shared_ptr<any_container_base> ref_;
};

template <typename T>
T& any::get() {
    assert(this->type() == typeid(T));
    T* result = reinterpret_cast<T*>( this->get_raw() );
    return *result;
}

template <typename T>
T const& any::get() const {
    assert(this->type() == typeid(T));
    T const* result = reinterpret_cast<T const*>( this->get_raw() );
    return *result;
}

} // end namespace tinfra

#endif // tinfra_any_h_included
