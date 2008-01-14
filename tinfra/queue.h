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
};
}

#endif
