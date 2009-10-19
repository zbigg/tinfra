//
// Copyright (c) 2009, Zbigniew Zagorski
// This software licensed under terms described in LICENSE.txt
//

#include <tinfra/thread.h> // implements (thread_set)

#include <tinfra/fmt.h>    // impl dependecies
#include <iostream>

namespace tinfra {
namespace thread {

//
// thread_set implementation
//
    
thread_set::~thread_set()
{
    join(0);
}

void   thread_set::add(thread t)
{
    threads_.push_back(t);
}

thread thread_set::start(thread::thread_entry entry, void* param)
{
    thread t = thread::start(entry, param);
    threads_.push_back(t);
    return t;
}

thread thread_set::start(runnable_ptr runnable)
{
    thread t =  thread::start(runnable);
    threads_.push_back(t);
    return t;
}

void thread_set::join(std::vector<void*>* result)
{
    while( threads_.size() > 0 ) {
        std::vector<thread>::iterator i = threads_.end()-1;
        void* r = i->join();
        threads_.erase(i);
        if( result )
            result->push_back(r);
    }
}

} } // end namespace tinfra::thread

// jedit: :tabSize=8:indentSize=4:noTabs=true:mode=c++:

