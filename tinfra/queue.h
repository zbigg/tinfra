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
class queue {
    tinfra::thread::monitor monitor_;
    std::list<T> container;
public:
    void put(T const& v)
    {
        tinfra::thread::synchronizator s(monitor_);
        this->container.push_back(v);
        if( this->container.size() == 1 ) 
            s.broadcast();
    }

    T get() 
    {
        tinfra::thread::synchronizator s(monitor_);

        while( this->container.size() == 0 ) {
            s.wait();
        }
        T result = this->container.front();
        this->container.pop_front();
        return result;
    }

    T peek(T const& def = T())
    {
        tinfra::thread::synchronizator s(monitor_);

        if( this->container.size() == 0 ) {
            return def;
        } else {
            T result = this->container.front();
            this->container.pop_front();
            return result;
        }
    }
};

} // end namespace tinfra

#endif // tinfra_queue_h_included

// jedit: :tabSize=8:indentSize=4:noTabs=true:mode=c++:

