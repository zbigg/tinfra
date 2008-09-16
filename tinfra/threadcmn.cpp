//
// Copyright (C) Zbigniew Zagorski <z.zagorski@gmail.com>,
// licensed to the public under the terms of the GNU GPL (>= 2)
// see the file COPYING for details
// I.e., do what you like, but keep copyright and there's NO WARRANTY.
//

#include <tinfra/thread.h>
#include <tinfra/fmt.h>
#include <iostream>



namespace tinfra {

//
// ThreadSet implementation
//
    
ThreadSet::~ThreadSet()
{
    join(0);
}

void   ThreadSet::add(Thread t)
{
    threads_.push_back(t);
}

Thread ThreadSet::start(Thread::thread_entry entry, void* param)
{
    Thread t = Thread::start(entry, param);
    threads_.push_back(t);
    return t;
}

Thread ThreadSet::start(Runnable& runnable)
{
    Thread t =  Thread::start(runnable);
    threads_.push_back(t);
    return t;
}

void ThreadSet::join(std::vector<void*>* result)
{
    while( threads_.size() > 0 ) {
        std::vector<Thread>::iterator i = threads_.end()-1;
        void* r = i->join();
        threads_.erase(i);
        if( result )
            result->push_back(r);
    }
}

}

