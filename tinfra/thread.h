//
// thread.h
//   common definition for tinfra threads
//

#ifndef __tinfra_thread_h__
#define __tinfra_thread_h__

#include <tinfra/platform.h>
#include <tinfra/runner.h>

#include <vector>

#if   defined( _WIN32)
// on win32 we must use pthread-win32 because we need condition variables
// #       include <tinfra/win32/thread.h>
#include <tinfra/posix/thread.h>
#elif defined(HAVE_PTHREAD_H)
#       include <tinfra/posix/thread.h>
#else
#error "tinfra: no threading support on this platform"
#endif

namespace tinfra {

class Monitor {
    Mutex      m;
    Condition  c;
public:
    void lock()      { m.lock(); }
    void unlock()    { m.unlock(); }
    
    void wait()      { c.wait(m); }
    void signal()    { c.signal(); }
    void broadcast() { c.broadcast(); }
};

class Synhronizator {
    Monitor& m;
    
public:
    Synhronizator(Monitor& m): m(m) { m.lock(); }
    ~Synhronizator() { m.unlock(); }
    
    void wait()      { m.wait(); }
    void signal()    { m.signal(); }
    void broadcast() { m.broadcast(); }
};

class ThreadSet {
    std::vector<Thread> threads_;

public:
    ~ThreadSet();

    Thread start( Runnable& runnable);
    Thread start(Thread::thread_entry entry, void* param);

    template <typename T>
        Thread start(void* (*entry)(T), T param) {
            return start(reinterpret_cast<Thread::thread_entry>(entry), (void*)param);
        }
    
    void   add(Thread t);

    /// Join all threads
    /// 
    /// Join all threads in the ThreadSet in reverse order reverse to
    /// addition/creation. Successfully joined threads are removed
    /// from ThreadSet.
    /// result vector pointer may be null if result is to be ignored.
    void join(std::vector<void*>* result = 0);
    
};

} // end namespace tinfra

#endif // #ifdef __tinfra_thread_h__
