#ifndef tinfra_callfwd_h__
#define tinfra_callfwd_h__

#include <tinfra/symbol.h>
#include <tinfra/queue.h>

#include <memory>

namespace callfwd {

typedef std::string message_serial_id;

namespace detail {

class caller_base {
public:
    virtual ~caller_base() {};        
    virtual void operator()() = 0;
};

namespace S {
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

} // end namespace detail

template <typename T>
class call_forwarder {
    tinfra::queue<detail::caller_base*> q;
    T* impl;
public:
    call_forwarder(T& i): impl(&i) {}

    void operator()() {
        q.put( new detail::functor_caller<T, detail::message0 >(impl) );
    }
    
    template <typename P1>
    void operator() (P1 const& p1) {
        typedef detail::message1<P1> lmt;
        q.put( new detail::functor_caller<T, lmt >(impl, lmt(p1) ));
    }
    
    template <typename P1, typename P2>
    void operator() (P1 const& p1, P2 const& p2) {
        typedef detail::message2<P1,P2> lmt;
        q.put( new detail::functor_caller<T, lmt >(impl, lmt(p1, p2) ));
    }
    
    void pull() {
        std::auto_ptr<detail::caller_base> p(q.get());
        detail::caller_base& cb = *p.get();
        cb();
        // p is deleted using auto_ptr
    }
    bool empty() { return q.empty(); }
};

template <typename W>
class call_sender {
    W& writer_;
public:
    call_sender(W& w)
        : writer_(w) 
    {}

    void operator() () {
        serialize_message(detail::message0(), writer_);
    }
    
    template <typename P1>
    void operator() (P1 const& p1) {
        serialize_message(detail::message1<P1>(p1), writer_);
    }
    
    template <typename P1, typename P2>
    void operator() (P1 const& p1, P2 const& p2) {
        serialize_message(detail::message2<P1,P2>(p1,p2), writer_);
    }
};

namespace detail {

struct dynamic_any_container {
    virtual ~dynamic_any_container() {}
    virtual void* get();
};

template<typename T>
class dynamic_any_container_impl: public dynamic_any_container {
    T value;
public:
    dynamic_any_container_impl(T const& v): value(v) {}
    
    virtual void* get()       { return &value; }    
    T&            typed_get() { return value; }
};

struct any_consumer_base {
    virtual ~any_consumer_base() = 0;
    virtual void operator()(void* pmt) = 0;
};

template <typename R>
struct any_parser_base {
    virtual ~any_parser_base() = 0;
    virtual std::auto_ptr<dynamic_any_container> parse(R&) = 0;
};

template <typename IMPL, typename MT>
class basic_message_consumer: public any_consumer_base {
    IMPL* impl;
    
public:
    basic_message_consumer(IMPL* impl): impl(impl) {}
        
    void invoke(void* pmt) {
        MT* mt = reinterpret_cast<MT*>(pmt);
        mt->call(*impl);
    }
};

template <typename MT, typename R>
class basic_message_parser: any_parser_base<R> {
    virtual std::auto_ptr<dynamic_any_container> parse(R& reader)
    {
        std::auto_ptr<dynamic_any_container_impl<MT> > pmsg( new dynamic_any_container_impl<MT>() );
        
        deserialize(pmsg->typed_get(), reader);
        
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

template <typename IMPL, typename R>
class call_receiver {    
    R& reader_;
    IMPL& impl_;
    
    std::map<message_serial_id, detail::dynamic_any_container*> dispatch_map;
    std::map<message_serial_id, detail::any_parser_base<R>*> parser_map;
public:
    call_receiver(IMPL& i, R& r): reader_(r), impl_(i) {}
    
    ~call_receiver() {
        using detail::clear_pointer_map;
        clear_pointer_map(dispatch_map);
        clear_pointer_map(parser_map);
    }
    
    void register_signal()
    {
        typedef detail::message0 local_message_type;
        message_serial_id mid = local_message_type::serial_id;
        dispatch_map[mid] = new detail::basic_message_consumer<IMPL, local_message_type>(impl_);
        parser_map[mid]   = new detail::basic_message_parser<local_message_type, R>();
    }
    
    template <typename P1>
    void register_message()
    {     
        typedef detail::message1<P1> local_message_type;
        message_serial_id mid = local_message_type::serial_id;
        dispatch_map[mid] = new detail::basic_message_consumer<IMPL, local_message_type>(impl_);
        parser_map[mid]   = new detail::basic_message_parser<local_message_type, R>();
    }
    
    template <typename P1, typename P2>
    void register_message()
    {
        typedef detail::message2<P1,P2> local_message_type;
        message_serial_id mid = local_message_type::serial_id;
        dispatch_map[mid] = new detail::basic_message_consumer<IMPL, local_message_type>(impl_);
        parser_map[mid]   = new detail::basic_message_parser<local_message_type, R>();
    }
    
    void pull()
    {
        message_serial_id mid;
        deserialize(mid, reader_);
        
        // check if mid is registered here
        assert(dispatch_map.find(mid) != dispatch_map.end());
        assert(parser_map.find(mid) != parser_map.end());
        
        // parse it
        detail::any_parser_base<R>* parser = parser_map[mid];
        std::auto_ptr<detail::dynamic_any_container> mptr = parser->parse(reader_);
        
        // call it
        detail::any_consumer_base& mc = *(dispatch_map[mid]);
        mc(mptr->get());
        
        // *mptr is released
    }

};

} // end namespace callfwd

#endif // tinfra_callfwd_h__
