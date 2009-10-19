//
// Copyright (c) 2009, Zbigniew Zagorski
// This software licensed under terms described in LICENSE.txt
//

//
// win32/thread.h
//   win32 based implementation of threads
//   STILL NOT READY

#ifndef tinfra_win32_thread_h_included
#define tinfra_win32_thread_h_included

#define WIN32_LEAN_AND_MEAN

#ifndef NOMINMAX
#define NOMINMAX
#endif

#define _WIN32_WINNT 0x0500 // Windows 2000
#include <windows.h>

namespace tinfra {
namespace thread {

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

    explicit thread(handle_type thread, DWORD thread_id_);
    ~thread();

    static thread current();
    
    static void sleep(long milliseconds);
    
    typedef void* (thread_entry)(void*);

    static thread start( runnable_ptr runnable);
    
    /// Start a detached thread
    /// runnable will be deleted after thread end
    static void   start_detached( runnable_ptr runnable);    
    
    static thread start( thread_entry entry, void* param );
    static void   start_detached( thread_entry entry, void* param );
    
    void* join();
    
    size_t to_number() const;
    
    HANDLE native_handle() const { return thread_handle_; }
    DWORD  native_id() const { return thread_id_; }
    
private:
    handle_type thread_handle_;
    DWORD       thread_id_;
};

} } // end namespace tinfra::thread

#endif

// jedit: :tabSize=8:indentSize=4:noTabs=true:mode=c++:

