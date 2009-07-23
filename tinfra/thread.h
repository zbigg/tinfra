//
// Copyright (C) 2008,2009 Zbigniew Zagorski <z.zagorski@gmail.com>,
// licensed to the public under the terms of the GNU GPL (>= 2)
// see the file COPYING for details
// I.e., do what you like, but keep copyright and there's NO WARRANTY.
//

//
// thread.h
//   common definition for tinfra threads
//

#ifndef tinfra_thread_h_included
#define tinfra_thread_h_included

#include <tinfra/platform.h>
#include <tinfra/runner.h>

#include <vector>

#if   defined( _WIN32)
// on win32 we must use pthread-win32 because we need condition variables
// #       include <tinfra/win32/thread.h>
#       define TINFRA_THREADS 1
#include <tinfra/win32/thread.h>
#elif defined(HAVE_PTHREAD_H)
#       include <tinfra/posix/thread.h>
#       define TINFRA_THREADS 1
#else
#error "tinfra: no threading support on this platform"
#       define TINFRA_THREADS 0
#endif

namespace tinfra {
namespace thread {

class guard {
    mutex& m;
public:
    guard(mutex& pm): m(pm)
    {
        m.lock();
    }
    ~guard()
    {
	m.unlock();
    }
};

class monitor {
    mutex      m;
    condition  c;
public:
    void lock()      { m.lock(); }
    void unlock()    { m.unlock(); }
    
    void wait()      { c.wait(m); }
    void signal()    { c.signal(); }
    void broadcast() { c.broadcast(); }
};

class synchronizator {
    monitor& m;
    
public:
    synchronizator(monitor& m): m(m) { m.lock(); }
    ~synchronizator() { m.unlock(); }
    
    void wait()      { m.wait(); }
    void signal()    { m.signal(); }
    void broadcast() { m.broadcast(); }
};

class thread_set {
    std::vector<thread> threads_;

public:
    ~thread_set();

    thread start( Runnable& runnable);
    thread start(thread::thread_entry entry, void* param);

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

