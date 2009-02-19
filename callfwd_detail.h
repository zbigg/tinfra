#ifndef tinfra_callfwd_detail_h__
#define tinfra_callfwd_detail_h__

#include "tinfra/symbol.h"
#include "tinfra/queue.h"
#include "tinfra/tinfra.h"

#include <memory>

namespace callfwd {
namespace detail {

class caller_base {
public:
    virtual ~caller_base() {};
    virtual void operator()() = 0;
};

namespace S {
    extern tinfra::symbol message_id;
    extern tinfra::symbol arguments;
    extern tinfra::symbol p1;
    extern tinfra::symbol p2;
    extern tinfra::symbol p3;
}

struct message0 {
    template <typename F>
    void apply(F& f) const {}
        
    template <typename IMPL>
    void call(IMPL& i) {
        i();
    }
    
    static const message_serial_id  serial_id;
};

template <typename P1>
struct message1 {
    P1 p1;
    
    message1() {}
    message1(P1 const& p1): p1(p1) {}
        
    template <typename F>
    void apply(F& f) const {
        f(S::p1, p1);
    }
    
    template <typename IMPL>
    void call(IMPL& i) {
        i(p1);
    }
    
    static const message_serial_id  serial_id;
};

template <typename P1>
const message_serial_id message1<P1>::serial_id(std::string("1") + typeid(P1).name());

template <typename P1, typename P2> 
struct message2 {
    P1 p1;
    P2 p2;
    message2() {}
    message2(P1 const& p1, P2 const& p2): p1(p1), p2(p2) {}
    
    template <typename F>
    void apply(F& f) const {
        f(S::p1, p1);
        f(S::p2, p2);
    }
    
    template <typename IMPL>
    void call(IMPL& i) {
        i(p1,p2);
    }
    
    static const message_serial_id  serial_id;
};

template <typename P1, typename P2>
const message_serial_id message2<P1,P2>::serial_id(std::string("2") + typeid(P1).name() + typeid(P2).name());

}}

namespace tinfra {   
    template <>
    struct TypeTraits<callfwd::detail::message0>: public ManagedStruct<callfwd::detail::message0> {};
        
    template <>
    template <typename P1>
    struct TypeTraits< callfwd::detail::message1<P1> >: public ManagedStruct<callfwd::detail::message1<P1> > {};
        
    template <>
    template <typename P1, typename P2>
    struct TypeTraits< callfwd::detail::message2<P1, P2> >: public ManagedStruct<callfwd::detail::message2<P1, P2> > {};
}

namespace callfwd { 
namespace detail {

template <typename IMPL, typename MT>
class functor_caller: public caller_base {
    IMPL* impl;
    MT    mt;
public:
    functor_caller(IMPL* impl, MT mt): impl(impl), mt(mt) {}
        
    void operator()() {
        mt.call(*impl);
    }
};

/// dynamic container of any object
///
/// Implementation contains object of any type and
/// is responsible for its lifecycle.
///
/// Provides opaque access to this object.
///
/// C++ equivalent of void*
struct dynamic_any_container {
    virtual ~dynamic_any_container();
    virtual void* get() = 0;
};

/// dynamic consumer of any object
///
/// Used as base class for consumer of objects
/// which type is known only to implementation.
///
/// C++ equivalent of void (void*)
struct any_consumer_base {
    virtual ~any_consumer_base();
    virtual void invoke(void* pmt) = 0;
};

/// base for any parser of reader R
///
/// Base type of message specific parsers/deserializers
/// that interact with one type of reader -> R.
template <typename R>
struct any_parser_base {
    virtual ~any_parser_base() {}
    virtual std::auto_ptr<dynamic_any_container> parse(R&) = 0;
};

template<typename T>
class dynamic_any_container_impl: public dynamic_any_container {
    T value;
public:
    dynamic_any_container_impl() {}
    dynamic_any_container_impl(T const& v): value(v) {}
    
    virtual void* get()       { return &value; }    
    T&            typed_get() { return value; }
};

template <typename IMPL, typename MT>
class basic_message_consumer: public any_consumer_base {
    IMPL& impl;
    
public:
    basic_message_consumer(IMPL& impl): impl(impl) {}
        
    void invoke(void* pmt) {
        MT* mt = reinterpret_cast<MT*>(pmt);
        mt->call(impl);
    }
};

template <typename MT, typename R>
class basic_message_parser: public any_parser_base<R> {
    virtual std::auto_ptr<dynamic_any_container> parse(R& reader)
    {
        std::auto_ptr<dynamic_any_container_impl<MT> > pmsg( new dynamic_any_container_impl<MT>() );
        
        tinfra::TypeTraitsProcessCaller<R> tt_reader(reader);
        tt_reader(S::message_id, pmsg->typed_get());
        
        return std::auto_ptr<dynamic_any_container>( pmsg.release() );
    }
};

template <typename MapType>
void clear_pointer_map(MapType& m)
{
    for( typename MapType::iterator i = m.begin(); i != m.end(); ++i ) {
        delete i->second;
    }
    m.clear();
}

} // end namespace detail
} // end namespace callfwd

#endif // tinfra_callfwd_detail_h__
