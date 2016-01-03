//
// Copyright (c) 2009, Zbigniew Zagorski
// This software licensed under terms described in LICENSE.txt
//

#ifndef tinfra_any_h_included
#define tinfra_any_h_included

#include "shared_ptr.h"
#include "assert.h"

#include <cassert>
#include <typeinfo>
#include <memory>

namespace tinfra {

struct any_container_base;

/// container of any object
///
/// Implementation contains object of any type and
/// is responsible for its lifecycle.
///
/// Provides opaque access to this object.
///
/// C++ equivalent of void*

class any {
public:
    /// create any value holding copy of object
    template <typename T>
    static any from_copy(T const& v);

    /// create any value holding reference to object
    template <typename T>
    static any by_ref(T& v);

    /// create any value owning instance of object
    template <typename T>
    static any from_new(T* v);

    void* get_raw();
    const void* get_raw() const;

    std::type_info const& type() const;

    /// typesafe get
    /// precondition:
    ///         will check in runtime of any
    ///         holds instance of T (or reference to it)
    /// uses TINFRA_ASSERT to check precondition
    template <typename T>
    T& get();

    /// typesafe const get
    /// precondition:
    ///         will check in runtime of any
    ///         holds instance of T (or reference to it)
    /// uses TINFRA_ASSERT to check precondition
    template <typename T>
    T const& get() const;
private:

    any(any_container_base* ptr);
    shared_ptr<any_container_base> ref_;
};

//
// implementation detail
//

//
// any internal storage
//
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

//
// any itself
//

template <typename T>
any any::from_copy(T const& v) {
    return any(new storing_any_container<T>(v));
}

template <typename T>
any any::by_ref(T& v) {
    return any(new ref_any_container<T>(v));
}

template <typename T>
any any::from_new(T* v) {
    return any(new auto_ptr_any_container<T>(v));
}

template <typename T>
T& any::get() {
    TINFRA_ASSERT(this->type() == typeid(T));
    T* result = reinterpret_cast<T*>( this->get_raw() );
    return *result;
}

template <typename T>
T const& any::get() const {
    TINFRA_ASSERT(this->type() == typeid(T));
    T const* result = reinterpret_cast<T const*>( this->get_raw() );
    return *result;
}

} // end namespace tinfra

#endif // tinfra_any_h_included
