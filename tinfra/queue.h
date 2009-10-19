//
// Copyright (c) 2009, Zbigniew Zagorski
// This software licensed under terms described in LICENSE.txt
//

#ifndef tinfra_queue_h_included
#define tinfra_queue_h_included

#include <list>
#include "tinfra/thread.h"

namespace tinfra {

template<typename T>
class queue: public std::list<T>  {
    tinfra::thread::monitor monitor_;
public:
    void put(T const& v)
    {
        tinfra::thread::synchronizator s(monitor_);
        this->push_back(v);
        if( this->size() == 1 ) 
            s.broadcast();
        
    }
    
    T get() 
    {
        tinfra::thread::synchronizator s(monitor_);
        
        while( this->size() == 0 ) {
            s.wait();
        }
        T result = this->front();
        this->pop_front();
        return result;
    }
    
    T peek(T const& def = T())
    {
        tinfra::thread::synchronizator s(monitor_);
        
        if( this->size() == 0 ) {
            return def;
        } else {
            T result = this->front();
            this->pop_front();
            return result;
        }
    }
};

} // end namespace tinfra

#endif // tinfra_queue_h_included

// jedit: :tabSize=8:indentSize=4:noTabs=true:mode=c++:

