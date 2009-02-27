//
// Copyright (C) 2008,2009  Zbigniew Zagorski <z.zagorski@gmail.com>,
// licensed to the public under the terms of the GNU GPL (>= 2)
// see the file COPYING for details
// I.e., do what you like, but keep copyright and there's NO WARRANTY.
//

//
// win32/thread.h
//   win32 based implementation of threads
//   STILL NOT READY

#ifndef __tinfra_win32_thread_h__
#define __tinfra_win32_thread_h__

#define WIN32_LEAN_AND_MEAN
#define NOMINMAX
#define _WIN32_WINNT _WIN32_WINNT_WINXP
#include <windows.h>

namespace tinfra {
namespace thread {

class mutex {
public:    
    mutex();
    ~mutex();

    void lock();
    void unlock();

    CRITICAL_SECTION* get_native() { return &mutex_; };
private:
    CRITICAL_SECTION mutex_;
};

class condition {    
public:
    typedef void* handle_type;

    condition();
    ~condition();
    
    void signal();
    void broadcast();
    void wait(mutex& mutex);

    //CONDITION_VARIABLE* get_native() { return &cond_; }
private:
    mutex            internal_lock;
    unsigned long    current_generation;

    unsigned long    signal_count;
    HANDLE           signal_sem;
    unsigned long    signal_generation;

    unsigned long    waiters_count;

    // broadcast related
    bool             was_broadcast;
    HANDLE           broadcast_ended;
};

class thread {
public:
    typedef HANDLE handle_type;

    explicit thread(handle_type thread);
    ~thread();

    static thread current();
    
    static void sleep(long milliseconds);
    
    typedef void* (thread_entry)(void*);

    static thread start( Runnable& runnable);
    
    /// Start a detached thread
    /// runnable will be deleted after thread end
    static void start_detached( Runnable* runnable);    
    
    static thread start( thread_entry entry, void* param );
    static void   start_detached( thread_entry entry, void* param );
    
    void* join();
    
    size_t to_number() const;
private:
    handle_type thread_handle_;
};

} } // end namespace tinfra::thread

#endif

// jedit: :tabSize=8:indentSize=4:noTabs=true:mode=c++:

