#ifndef tinfra_callfwd_h__
#define tinfra_callfwd_h__

#include <tinfra/symbol.h>
#include <tinfra/queue.h>

#include <memory>

namespace callfwd {

typedef std::string message_serial_id;
    
}

#include "callfwd_detail.h"

namespace callfwd {

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

/// Serializer of calls.
///
/// This objects acts as functor which serializes all
/// it's calls using writer W.
template <typename W>
class call_sender {
    W& writer_;
public:
    call_sender(W& w)
        : writer_(w) 
    {}

    void operator() ();
    
    template <typename P1>
    void operator() (P1 const& p1);
    
    template <typename P1, typename P2>
    void operator() (P1 const& p1, P2 const& p2);
};

template <typename IMPL, typename R>
class call_receiver {    
    R& reader_;
    IMPL& impl_;
    
    std::map<message_serial_id, detail::any_consumer_base*> dispatch_map;
    std::map<message_serial_id, detail::any_parser_base<R>*> parser_map;
public:
    /// Construct call_receiver.
    ///
    /// Both parameters will be remembered and used during
    /// call_receiver life.
    ///
    /// Before using pull() to process messages one should
    /// register message types than can be received by IMPL.
    /// 
    /// @param i   instance of message receiver
    /// @param r   instance of reader R
    
    call_receiver(IMPL& i, R& r): reader_(r), impl_(i) {}    
    ~call_receiver();
    
    /// register unparametrized parameter message (a signal)
    void register_signal();
    
    /// register one parameter message
    template <typename P1>
    void register_message()
    {
        typedef detail::message1<P1> local_message_type;
        message_serial_id mid = local_message_type::serial_id;
        dispatch_map[mid] = new detail::basic_message_consumer<IMPL, local_message_type>(impl_);
        parser_map[mid]   = new detail::basic_message_parser<local_message_type, R>();
    }
    /// register two parameter message
    template <typename P1, typename P2>
    void register_message() 
    {
        typedef detail::message2<P1,P2> local_message_type;
        message_serial_id mid = local_message_type::serial_id;
        dispatch_map[mid] = new detail::basic_message_consumer<IMPL, local_message_type>(impl_);
        parser_map[mid]   = new detail::basic_message_parser<local_message_type, R>();
    }
    
    /// process one call
    ///
    /// Read and deserialize one message from reader R.
    /// Then invoke this message on IMPL instaance
    void pull();

};

//
// implementation of call_sender
//

template <typename W>
void call_sender<W>::operator() () {
    tinfra::TypeTraitsProcessCaller<W> writer(writer_);
    
    writer(detail::S::message_id, detail::message0::serial_id);
    writer(detail::S::arguments, detail::message0());
}

template <typename W>
template <typename P1>
void call_sender<W>::operator() (P1 const& p1) {
    tinfra::TypeTraitsProcessCaller<W> writer(writer_);
    
    writer(detail::S::message_id, detail::message1<P1>::serial_id);
    writer(detail::S::arguments, detail::message1<P1>(p1));
}

template <typename W>
template <typename P1, typename P2>
void call_sender<W>::operator() (P1 const& p1, P2 const& p2) {
    tinfra::TypeTraitsProcessCaller<W> writer(writer_);
        
    writer(detail::S::message_id, detail::message2<P1,P2>::serial_id);
    writer(detail::S::arguments, detail::message2<P1,P2>(p1,p2));
}

//
// implementation of call_receiver
//

template <typename IMPL, typename R>
call_receiver<IMPL,R>::~call_receiver() {
    detail::clear_pointer_map(dispatch_map);
    detail::clear_pointer_map(parser_map);
}

template <typename IMPL, typename R>
void call_receiver<IMPL,R>::register_signal()
{
    typedef detail::message0 local_message_type;
    message_serial_id mid = local_message_type::serial_id;
    dispatch_map[mid] = new detail::basic_message_consumer<IMPL, local_message_type>(impl_);
    parser_map[mid]   = new detail::basic_message_parser<local_message_type, R>();
}
/*
template <typename IMPL, typename R>
template <typename P1>
void call_receiver<IMPL,R>::register_message<P1>()
{
    typedef detail::message1<P1> local_message_type;
    message_serial_id mid = local_message_type::serial_id;
    dispatch_map[mid] = new detail::basic_message_consumer<IMPL, local_message_type>(impl_);
    parser_map[mid]   = new detail::basic_message_parser<local_message_type, R>();
}

template <typename IMPL, typename R>
template <typename P1, typename P2>
void call_receiver<IMPL,R>::register_message<P1,P2>()
{
    typedef detail::message2<P1,P2> local_message_type;
    message_serial_id mid = local_message_type::serial_id;
    dispatch_map[mid] = new detail::basic_message_consumer<IMPL, local_message_type>(impl_);
    parser_map[mid]   = new detail::basic_message_parser<local_message_type, R>();
}
*/
template <typename IMPL, typename R>
void call_receiver<IMPL,R>::pull()
{
    message_serial_id mid;
    reader_(detail::S::message_id, mid);
    
    // check if mid is registered here
    assert(dispatch_map.find(mid) != dispatch_map.end());
    assert(parser_map.find(mid) != parser_map.end());
    
    // parse it
    detail::any_parser_base<R>* parser = parser_map[mid];
    std::auto_ptr<detail::dynamic_any_container> mptr = parser->parse(reader_);
    
    // call it
    detail::any_consumer_base* mc = dispatch_map[mid];
    mc->invoke(mptr->get());
    
    // *mptr is released
}

} // end namespace callfwd

#endif // tinfra_callfwd_h__
