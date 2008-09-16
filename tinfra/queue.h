//
// Copyright (C) Zbigniew Zagorski <z.zagorski@gmail.com>,
// licensed to the public under the terms of the GNU GPL (>= 2)
// see the file COPYING for details
// I.e., do what you like, but keep copyright and there's NO WARRANTY.
//

#ifndef __tinfra_queue_h__
#define __tinfra_queue_h__

#include <list>
#include "tinfra/thread.h"

namespace tinfra {

template<typename T>
class Queue: public std::list<T>  {
    Monitor monitor_;
public:
    void put(T const& v)
    {
        Synhronizator s(monitor_);
        this->push_back(v);
        if( this->size() == 1 ) 
            s.broadcast();
        
    }
    
    T get() 
    {
        Synhronizator s(monitor_);
        
        while( this->size() == 0 ) {
            s.wait();
        }
        T result = this->front();
        this->pop_front();
        return result;
    }
    
    T peek(T const& def = T())
    {
        Synhronizator s(monitor_);
        
        if( this->size() == 0 ) {
            return def;
        } else {
            T result = this->front();
            this->pop_front();
            return result;
        }
    }
};
}

#endif
