//
// Copyright (c) 2009, Zbigniew Zagorski
// This software licensed under terms described in LICENSE.txt
//

//
// thread.h
//   common definition for tinfra threads
//

#ifndef tinfra_thread_h_included
#define tinfra_thread_h_included

#include "config-pub.h"

#include "platform.h"
#include "runner.h"
#include "time.h" // for deadline

#include <vector>


#if   defined( _WIN32)
// on win32 we must use pthread-win32 because we need condition variables
// #       include <tinfra/win32/thread.h>
#       define TINFRA_THREADS 1
#include <tinfra/win32/thread.h>
#elif defined(TINFRA_HAVE_PTHREAD_H)
#       include <tinfra/posix/thread.h>
#       define TINFRA_THREADS 1
#else
#error "tinfra: no threading support on this platform"
#       define TINFRA_THREADS 0
#endif

#include "mutex.h"
#include "guard.h"

namespace tinfra {
namespace thread {

// import DEPRECATED symbols:
//    tinfra::thread::mutex
//    tinfra::thread::guard

using tinfra::mutex;
using tinfra::guard;

class monitor {
    mutex      m;
    condition  c;
public:
    // mutex access
    void lock()      { m.lock(); }    
    void unlock()    { m.unlock(); }
    
    // condition access
    void wait()      { c.wait(m); }
    bool timed_wait(deadline const& t) { return c.timed_wait(m, t); }
    
    void signal()    { c.signal(); }
    void broadcast() { c.broadcast(); }
};

class synchronizator {
    monitor& m;
    
public:
    synchronizator(monitor& m): m(m) { m.lock(); }
    ~synchronizator() { m.unlock(); }
    
    void wait()      { m.wait(); }
    bool timed_wait(deadline const& t)      { return m.timed_wait(t); }
    void signal()    { m.signal(); }
    void broadcast() { m.broadcast(); }
};

class thread_set {
    std::vector<thread> threads_;

public:
    ~thread_set();

    thread start( runnable what);
    thread start( thread::thread_entry entry, void* param);

    template <typename T>
        thread start(void* (*entry)(T), T param) {
            return start(reinterpret_cast<thread::thread_entry>(entry), (void*)param);
        }
    
    void   add(thread t);

    /// Join all threads
    /// 
    /// Join all threads in the thread_set in reverse order reverse to
    /// addition/creation. Successfully joined threads are removed
    /// from thread_set.
    /// result vector pointer may be null if result is to be ignored.
    void join(std::vector<void*>* result = 0);
    
};

} } // end namespace tinfra::thread

#endif // #ifdef tinfra_thread_h_included

// jedit: :tabSize=8:indentSize=4:noTabs=true:mode=c++:

