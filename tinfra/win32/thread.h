//
// win32/thread.h
//   win32 based implementation of threads
//   STILL NOT READY

#ifndef __tinfra_win32_thread_h__
#define __tinfra_win32_thread_h__

#define WIN32_LEAN_AND_MEAN
#include <windows.h>

namespace tinfra {

class Mutex {
public:    
    Mutex();
    ~Mutex();

    void lock();
    void unlock();

    CRITICAL_SECTION* get_native() { return &mutex_; };
private:
    CRITICAL_SECTION mutex_;
};

class Condition {    
public:
    typedef void* handle_type;

    Condition();
    ~Condition();
    
    void signal();
    void broadcast();
    void wait(void* mutex);
    void wait(Mutex& mutex);

    CONDITION_VARIABLE* get_native() { return &cond_; }
private:
    CONDITION_VARIABLE cond_;
};

class Thread {
public:
    typedef unsigned long handle_type;

    explicit Thread(handle_type thread): thread_id_(thread) {}
        
    static Thread current();
    
    static void sleep(long milliseconds);
    
    typedef void* (thread_entry)(void*);

    static Thread start( Runnable& runnable);
    
    /// Start a detached thread
    /// runnable will be deleted after thread end
    static Thread start_detached( Runnable* runnable);    
    
    static Thread start( thread_entry entry, void* param );
    static Thread start_detached( thread_entry entry, void* param );
    
    void* join();
    
    size_t to_number() const;
private:
    handle_type thread_id_;
};

} // end namespace tinfra

#endif
