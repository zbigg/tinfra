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

typedef tinfra::queue<detail::partial_invoker_base*> call_queue;

template <typename T>
class delegator {
    call_queue& q;
public:
    delegator(call_queue& pq): q(pq) {}

    void operator()() {
        detail::message0 m;
        q.put( new detail::message_invoker<T, detail::message0 >(m) );
    }
    
    template <typename P1>
    void operator() (P1 const& p1) {
        typedef detail::message1<P1> lmt;
        q.put( new detail::message_invoker<T, lmt >(lmt(p1) ));
    }
    
    template <typename P1, typename P2>
    void operator() (P1 const& p1, P2 const& p2) {
        typedef detail::message2<P1,P2> lmt;
        q.put( new detail::message_invoker<T, lmt >(lmt(p1, p2) ));
    }
    
    bool empty() { return q.empty(); }
};

template <typename T>
void process(call_queue& q, T& impl)
{
    std::auto_ptr<detail::partial_invoker_base> p( q.get() );
    detail::partial_invoker_base& cb = *p.get();
    cb.invoke(&impl);
}

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
class dispatch_map {
public:
    typedef detail::partial_invoker_base invoker_type;
    typedef detail::any_parser_base<R>   parser_type;
    
    invoker_type* get_invoker(message_serial_id const& mid) const
    {
        const typename std::map<message_serial_id, detail::partial_invoker_base*>::const_iterator i = dispatch_map.find(mid);
        
        if( i == dispatch_map.end() )
            return 0;
        return i->second;
    }
    
    parser_type* get_parser(message_serial_id const& mid) const
    {
        const typename std::map<message_serial_id, detail::any_parser_base<R>* >::const_iterator i = parser_map.find(mid);
        
        if( i == parser_map.end() )
            return 0;
        return i->second;
    }
    
    ~dispatch_map();
    
    /// register unparametrized parameter message (a signal)
    void register_signal();
    
    /// register one parameter message
    template <typename P1>
    void register_message()
    {
        typedef detail::message1<P1> local_message_type;
        message_serial_id mid = local_message_type::serial_id;
        dispatch_map[mid] = new detail::functor_message_invoker<IMPL, local_message_type>();
        parser_map[mid]   = new detail::basic_message_parser<local_message_type, R>();
    }
    /// register two parameter message
    template <typename P1, typename P2>
    void register_message() 
    {
        typedef detail::message2<P1,P2> local_message_type;
        message_serial_id mid = local_message_type::serial_id;
        dispatch_map[mid] = new detail::functor_message_invoker<IMPL, local_message_type>();
        parser_map[mid]   = new detail::basic_message_parser<local_message_type, R>();
    }
private:
    std::map<message_serial_id, detail::partial_invoker_base*> dispatch_map;
    std::map<message_serial_id, detail::any_parser_base<R>*> parser_map;
};

template <typename IMPL, typename R>
void process(R& reader, dispatch_map<IMPL, R> const& meta, IMPL& target)

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
// implementation of dispatch_map
//

template <typename IMPL, typename R>
dispatch_map<IMPL,R>::~dispatch_map() {
    detail::clear_pointer_map(dispatch_map);
    detail::clear_pointer_map(parser_map);
}

template <typename IMPL, typename R>
void dispatch_map<IMPL,R>::register_signal()
{
    typedef detail::message0 local_message_type;
    message_serial_id mid = local_message_type::serial_id;
    dispatch_map[mid] = new detail::functor_message_invoker<IMPL, local_message_type>();
    parser_map[mid]   = new detail::basic_message_parser<local_message_type, R>();
}
/*
template <typename IMPL, typename R>
template <typename P1>
void dispatch_map<IMPL,R>::register_message<P1>()
{
    typedef detail::message1<P1> local_message_type;
    message_serial_id mid = local_message_type::serial_id;
    dispatch_map[mid] = new detail::functor_invoker<IMPL, local_message_type>(impl_);
    parser_map[mid]   = new detail::basic_message_parser<local_message_type, R>();
}

template <typename IMPL, typename R>
template <typename P1, typename P2>
void dispatch_map<IMPL,R>::register_message<P1,P2>()
{
    typedef detail::message2<P1,P2> local_message_type;
    message_serial_id mid = local_message_type::serial_id;
    dispatch_map[mid] = new detail::functor_invoker<IMPL, local_message_type>(impl_);
    parser_map[mid]   = new detail::basic_message_parser<local_message_type, R>();
}
*/
template <typename IMPL, typename R>
void process(R& reader, dispatch_map<IMPL, R> const& meta, IMPL& target)
{
    message_serial_id mid;
    reader(detail::S::message_id, mid);
    
    // check if mid is registered here
    detail::any_parser_base<R>* parser = meta.get_parser(mid);
    detail::partial_invoker_base* invoker = meta.get_invoker(mid);
    
    assert(parser != 0);
    assert(invoker != 0);
    
    // parse it -> creates new message <mid> and reads it's content from reader
    std::auto_ptr<detail::dynamic_any_container> mptr = parser->parse(reader);
    
    // type aware call "target(*mptr)"
    void* params[2] = { & (target), mptr->get() };
    invoker->invoke(params);
    
    // *mptr is released
}

} // end namespace callfwd

#endif // tinfra_callfwd_h__
