//
// Copyright (C) Zbigniew Zagorski <z.zagorski@gmail.com>,
// licensed to the public under the terms of the GNU GPL (>= 2)
// see the file COPYING for details
// I.e., do what you like, but keep copyright and there's NO WARRANTY.
//

#ifndef __tinfra_aio_h__
#define __tinfra_aio_h__

#include <string>
#include <vector>
#include <map>
#include <memory>

#include "tinfra/io/stream.h"

namespace tinfra {
namespace aio {
    
typedef tinfra::io::stream stream;

class Dispatcher;
    
class Listener {
public:
    /// Normal operation event.
    ///
    /// Information about normal operation event. 
    ///
    /// @param event type of event ( Dispatcher::event_type )
    virtual void event(Dispatcher& d, stream* c, int event) = 0;

    /// Failure notification.
    /// 
    /// Called by dispatcher gives an error.
    ///
    /// TODO: describe/remove/specify what @param error means
    /// Memory:
    /// Dispatcher removes stream after this ntification.
    ///
    /// Stream lifecycle becomes user responsiblity.
    virtual void failure(Dispatcher& d, stream* c, int error) = 0;
    
    virtual ~Listener() {}
};

class Dispatcher {
public:
    enum event_type {
        READ = 1, 
        WRITE = 2
    };
    
    /// Create instance.
    ///
    /// Create new independent from others instance of Dispatcher.
    ///
    /// Thread: 
    ///    - safe to call concurently
    ///    - created instance are independent but not thread safe
    static  std::auto_ptr<Dispatcher> create();
    
    /// Add a stream 
    ///
    /// The new stream is added or it's parameters are replaced (listener, flags)
    ///
    /// In the following invocations of step() Dispatcher will wait 
    /// for events specified in flags.
    /// 
    /// Stream and Listener instances must be available up to
    /// Dispatcher destruction or call to remove()
    virtual void add(stream* c, Listener* l, int initial_flags) = 0;
    
    /// Remove a stream
    ///
    /// Remove stream from set of managed streams.
    ///
    /// Memory:
    /// Called can freely destroy stream after this call.
    ///
    /// Dispatcher MUST NOT dereference stream pointer after this call as 
    /// it's storage and object may be destroyed.
    ///
    /// Stream lifecycle becomes user responsiblity.
    virtual void remove(stream* c) = 0;
    
    /// Change wait state.
    ///
    /// Change wait state of stream. flags given by mask parameter
    /// are disabled/enabled according to enable parameter.
    virtual void wait(stream* c, int mask, bool enable) = 0;

    /// Perform on dispatching step.
    ///
    /// This call will wait until at least one event occurs.
    /// 
    /// TODO. There should be timeout parameter.
    /// TODO. There should be feedback about what dispatcher has
    ///       done during this one step.
    virtual void step() = 0;
    
    virtual ~Dispatcher() {}
};

//deprecated, use Dispatcher::create()
std::auto_ptr<Dispatcher> create_dispatcher();

}} // end namespace tinfra::aio

#endif // __tinfra_aio_h__

// jedit: :tabSize=8:indentSize=4:noTabs=true:mode=c++:
